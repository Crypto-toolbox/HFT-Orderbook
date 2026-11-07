/**
 * Book operations: the public API tying limit trees, order lists and
 * hash maps together.
 *
 *   Add     - O(log M) for the first order at a price, O(1) otherwise
 *   Remove  - O(1), plus O(log M) when it empties a price level
 *   Replace - O(1) for a size change, remove + add for a price change
 *   Best bid/ask, order and limit lookups - O(1)
 *
 * where M is the number of price levels on that side.
 */
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "hftlob.h"

static double
normalisePrice(double price){
    /* -0.0 and 0.0 must map to the same Limit. */
    return price + 0.0;
}

static HashMap*
limitMapFor(Book *book, unsigned buyOrSell){
    return buyOrSell == HFTLOB_BUY ? &book->buyLimits : &book->sellLimits;
}

static Limit*
treeFor(Book *book, unsigned buyOrSell){
    return buyOrSell == HFTLOB_BUY ? book->buyTree : book->sellTree;
}

static Limit*
getOrCreateLimit(Book *book, unsigned buyOrSell, double price){
    HashMap *limits = limitMapFor(book, buyOrSell);
    Limit *limit = mapGet(limits, &price, sizeof(price));
    if(limit != NULL){
        return limit;
    }

    limit = malloc(sizeof(Limit));
    if(limit == NULL){
        return NULL;
    }
    initLimit(limit);
    limit->limitPrice = price;
    if(!mapPut(limits, &price, sizeof(price), limit)){
        free(limit);
        return NULL;
    }
    addNewLimit(treeFor(book, buyOrSell), limit);
    rebalanceUpwards(limit->parent);

    if(buyOrSell == HFTLOB_BUY){
        if(book->highestBuy == NULL || price > book->highestBuy->limitPrice){
            book->highestBuy = limit;
        }
    }
    else if(book->lowestSell == NULL || price < book->lowestSell->limitPrice){
        book->lowestSell = limit;
    }
    return limit;
}

static void
dropLimitIfEmpty(Book *book, unsigned buyOrSell, Limit *limit){
    Limit *tree = treeFor(book, buyOrSell);
    double price = limit->limitPrice;
    if(limit->orderCount > 0){
        return;
    }

    mapRemove(limitMapFor(book, buyOrSell), &price, sizeof(price));
    rebalanceUpwards(detachLimit(limit));

    if(book->highestBuy == limit){
        book->highestBuy = tree->rightChild ? getMaximumLimit(tree->rightChild) : NULL;
    }
    if(book->lowestSell == limit){
        book->lowestSell = tree->rightChild ? getMinimumLimit(tree->rightChild) : NULL;
    }
    free(limit);
}

static void
freeOrder(Order *order){
    free(order->tid);
    free(order);
}

Book*
createBook(void){
    /**
     * Allocate an empty book. Returns NULL if out of memory.
     */
    Book *book = calloc(1, sizeof(Book));
    if(book == NULL){
        return NULL;
    }
    book->buyTree = createRoot();
    book->sellTree = createRoot();
    if(book->buyTree == NULL || book->sellTree == NULL
            || !initMap(&book->orders, 0)
            || !initMap(&book->buyLimits, 0)
            || !initMap(&book->sellLimits, 0)){
        freeBook(book);
        return NULL;
    }
    return book;
}

void
freeBook(Book *book){
    /**
     * Free the book and every Order and Limit in it.
     */
    size_t i;
    HashMap *limitMaps[2];
    int m;
    if(book == NULL){
        return;
    }

    for(i = 0; i < book->orders.capacity; ++i){
        MapEntry *entry;
        for(entry = book->orders.buckets[i]; entry != NULL; entry = entry->next){
            freeOrder(entry->value);
        }
    }
    limitMaps[0] = &book->buyLimits;
    limitMaps[1] = &book->sellLimits;
    for(m = 0; m < 2; ++m){
        for(i = 0; i < limitMaps[m]->capacity; ++i){
            MapEntry *entry;
            for(entry = limitMaps[m]->buckets[i]; entry != NULL; entry = entry->next){
                free(entry->value);
            }
        }
    }
    freeMap(&book->orders);
    freeMap(&book->buyLimits);
    freeMap(&book->sellLimits);
    free(book->buyTree);
    free(book->sellTree);
    free(book);
}

Order*
addOrderToBook(Book *book, const char *tid, unsigned buyOrSell,
               double shares, double price){
    /**
     * Add a new order at the back of its price level's queue.
     *
     * The book copies tid and owns the returned Order; it stays valid until
     * the order is removed. Returns NULL if tid is NULL or already in the
     * book, buyOrSell is not HFTLOB_BUY/HFTLOB_SELL, shares is not positive,
     * price is NaN, or memory runs out.
     */
    size_t tidLen;
    Order *order;
    Limit *limit;

    if(tid == NULL || (buyOrSell != HFTLOB_BUY && buyOrSell != HFTLOB_SELL)
            || !(shares > 0) || isnan(price)){
        return NULL;
    }
    tidLen = strlen(tid);
    if(mapGet(&book->orders, tid, tidLen) != NULL){
        return NULL;
    }

    order = malloc(sizeof(Order));
    if(order == NULL){
        return NULL;
    }
    initOrder(order);
    order->tid = malloc(tidLen + 1);
    if(order->tid == NULL){
        free(order);
        return NULL;
    }
    memcpy(order->tid, tid, tidLen + 1);
    order->buyOrSell = buyOrSell;
    order->shares = shares;
    order->limit = normalisePrice(price);

    limit = getOrCreateLimit(book, buyOrSell, order->limit);
    if(limit == NULL || !mapPut(&book->orders, tid, tidLen, order)){
        if(limit != NULL){
            dropLimitIfEmpty(book, buyOrSell, limit);
        }
        freeOrder(order);
        return NULL;
    }
    pushOrder(limit, order);
    return order;
}

int
removeOrderFromBook(Book *book, const char *tid){
    /**
     * Cancel an order and free it. Empty price levels are removed.
     * Returns 1 on success, 0 if the order is not in the book.
     */
    Order *order;
    Limit *limit;
    if(tid == NULL){
        return 0;
    }
    order = mapRemove(&book->orders, tid, strlen(tid));
    if(order == NULL){
        return 0;
    }
    limit = order->parentLimit;
    removeOrder(order);
    dropLimitIfEmpty(book, order->buyOrSell, limit);
    freeOrder(order);
    return 1;
}

int
replaceOrderInBook(Book *book, const char *tid, double shares, double price){
    /**
     * Change an order's size and/or price.
     *
     * - shares <= 0 cancels the order (like removeOrderFromBook).
     * - A size-only change is applied in place; the order keeps its spot
     *   in the queue.
     * - A price change moves the order to the back of its new price level.
     *   The Order pointer stays the same.
     *
     * Returns 1 on success, 0 if the order is not in the book, -1 if the
     * price is NaN or memory runs out (the order is then left unchanged).
     */
    Order *order = getOrderFromBook(book, tid);
    Limit *oldLimit;
    Limit *newLimit;

    if(order == NULL){
        return 0;
    }
    if(!(shares > 0)){
        return removeOrderFromBook(book, tid);
    }
    if(isnan(price)){
        return -1;
    }
    price = normalisePrice(price);
    oldLimit = order->parentLimit;

    if(price == order->limit){
        double diff = shares - order->shares;
        order->shares = shares;
        oldLimit->size += diff;
        oldLimit->totalVolume += diff * oldLimit->limitPrice;
        return 1;
    }

    newLimit = getOrCreateLimit(book, order->buyOrSell, price);
    if(newLimit == NULL){
        return -1;
    }
    removeOrder(order);
    dropLimitIfEmpty(book, order->buyOrSell, oldLimit);
    order->shares = shares;
    order->limit = price;
    pushOrder(newLimit, order);
    return 1;
}

Order*
getOrderFromBook(const Book *book, const char *tid){
    if(tid == NULL){
        return NULL;
    }
    return mapGet(&book->orders, tid, strlen(tid));
}

Limit*
getLimitFromBook(const Book *book, unsigned buyOrSell, double price){
    /**
     * Return the price level at the given price, or NULL.
     * Its size, totalVolume and orderCount answer "volume at limit" in O(1).
     */
    const HashMap *limits = buyOrSell == HFTLOB_BUY ? &book->buyLimits : &book->sellLimits;
    price = normalisePrice(price);
    return mapGet(limits, &price, sizeof(price));
}

Limit*
getBestBid(const Book *book){
    return book->highestBuy;
}

Limit*
getBestAsk(const Book *book){
    return book->lowestSell;
}
