/**
 * Book operations: the public API tying limit trees, order lists and
 * hash maps together.
 *
 *   Add     - O(log M) for the first order at a price, O(1) otherwise
 *   Remove  - O(1), plus O(log M) when it empties a price level
 *   Replace - O(1) for a size change, remove + add for a price change
 *   Execute - O(1) per resting order filled, plus O(log M) per emptied level
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

static uint64_t
priceKey(double price){
    /* Prices are keyed by their (normalised) bit pattern. */
    uint64_t key;
    memcpy(&key, &price, sizeof(key));
    return key;
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
    Limit *limit = mapGet(limits, priceKey(price));
    if(limit != NULL){
        return limit;
    }

    limit = poolAlloc(&book->limitPool);
    if(limit == NULL){
        return NULL;
    }
    initLimit(limit);
    limit->limitPrice = price;
    if(!mapPut(limits, priceKey(price), limit)){
        poolFree(&book->limitPool, limit);
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

    mapRemove(limitMapFor(book, buyOrSell), priceKey(price));
    rebalanceUpwards(detachLimit(limit));

    if(book->highestBuy == limit){
        book->highestBuy = tree->rightChild ? getMaximumLimit(tree->rightChild) : NULL;
    }
    if(book->lowestSell == limit){
        book->lowestSell = tree->rightChild ? getMinimumLimit(tree->rightChild) : NULL;
    }
    poolFree(&book->limitPool, limit);
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
    initPool(&book->orderPool, sizeof(Order));
    initPool(&book->limitPool, sizeof(Limit));
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
    if(book == NULL){
        return;
    }
    destroyPool(&book->orderPool);
    destroyPool(&book->limitPool);
    freeMap(&book->orders);
    freeMap(&book->buyLimits);
    freeMap(&book->sellLimits);
    free(book->buyTree);
    free(book->sellTree);
    free(book);
}

Order*
addOrderToBook(Book *book, OrderId id, unsigned buyOrSell,
               double shares, double price){
    /**
     * Add a new order at the back of its price level's queue.
     *
     * The book owns the returned Order; it stays valid until the order is
     * removed or fully executed. Returns NULL if id is already in the book,
     * buyOrSell is not HFTLOB_BUY/HFTLOB_SELL, shares is not positive, price
     * is NaN, or memory runs out.
     */
    Order *order;
    Limit *limit;

    if((buyOrSell != HFTLOB_BUY && buyOrSell != HFTLOB_SELL)
            || !(shares > 0) || isnan(price)){
        return NULL;
    }
    if(mapGet(&book->orders, id) != NULL){
        return NULL;
    }

    order = poolAlloc(&book->orderPool);
    if(order == NULL){
        return NULL;
    }
    initOrder(order);
    order->id = id;
    order->buyOrSell = buyOrSell;
    order->shares = shares;
    order->limit = normalisePrice(price);

    limit = getOrCreateLimit(book, buyOrSell, order->limit);
    if(limit == NULL || !mapPut(&book->orders, id, order)){
        if(limit != NULL){
            dropLimitIfEmpty(book, buyOrSell, limit);
        }
        poolFree(&book->orderPool, order);
        return NULL;
    }
    pushOrder(limit, order);
    return order;
}

int
removeOrderFromBook(Book *book, OrderId id){
    /**
     * Cancel an order and free it. Empty price levels are removed.
     * Returns 1 on success, 0 if the order is not in the book.
     */
    Order *order;
    Limit *limit;
    order = mapRemove(&book->orders, id);
    if(order == NULL){
        return 0;
    }
    limit = order->parentLimit;
    removeOrder(order);
    dropLimitIfEmpty(book, order->buyOrSell, limit);
    poolFree(&book->orderPool, order);
    return 1;
}

int
replaceOrderInBook(Book *book, OrderId id, double shares, double price){
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
    Order *order = getOrderFromBook(book, id);
    Limit *oldLimit;
    Limit *newLimit;

    if(order == NULL){
        return 0;
    }
    if(!(shares > 0)){
        return removeOrderFromBook(book, id);
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

double
executeInBook(Book *book, unsigned aggressorSide, double shares,
              double limitPrice, FillCallback onFill, void *ctx){
    /**
     * Execute an incoming (aggressive) order against the opposite side of
     * the book, in price-time priority: best price first, and the oldest
     * order first within a price level.
     *
     * A buy fills against asks priced at or below limitPrice; a sell fills
     * against bids priced at or above it. Pass INFINITY (buy) or -INFINITY
     * (sell) for a market order. Matching stops when the order is filled or
     * no eligible liquidity is left.
     *
     * Resting orders are reduced or, when fully filled, removed and freed;
     * emptied price levels are removed. onFill (may be NULL) is called for
     * every fill, at the resting order's price.
     *
     * The unfilled remainder is NOT added to the book (immediate-or-cancel);
     * add it with addOrderToBook() to rest a limit order.
     *
     * Returns the quantity filled, or 0 for invalid arguments.
     */
    double remaining = shares;

    if((aggressorSide != HFTLOB_BUY && aggressorSide != HFTLOB_SELL)
            || !(shares > 0) || isnan(limitPrice)){
        return 0;
    }

    while(remaining > 0){
        Limit *best = aggressorSide == HFTLOB_BUY ? book->lowestSell : book->highestBuy;
        Order *resting;
        double fill;

        if(best == NULL
                || (aggressorSide == HFTLOB_BUY && best->limitPrice > limitPrice)
                || (aggressorSide == HFTLOB_SELL && best->limitPrice < limitPrice)){
            break;
        }

        resting = best->tailOrder;  /* oldest order at this level */
        fill = resting->shares < remaining ? resting->shares : remaining;
        if(onFill != NULL){
            onFill(resting, fill, best->limitPrice, ctx);
        }
        remaining -= fill;

        if(fill == resting->shares){
            removeOrderFromBook(book, resting->id);
        }
        else{
            resting->shares -= fill;
            best->size -= fill;
            best->totalVolume -= fill * best->limitPrice;
        }
    }
    return shares - remaining;
}

Order*
getOrderFromBook(const Book *book, OrderId id){
    return mapGet(&book->orders, id);
}

Limit*
getLimitFromBook(const Book *book, unsigned buyOrSell, double price){
    /**
     * Return the price level at the given price, or NULL.
     * Its size, totalVolume and orderCount answer "volume at limit" in O(1).
     */
    const HashMap *limits = buyOrSell == HFTLOB_BUY ? &book->buyLimits : &book->sellLimits;
    price = normalisePrice(price);
    return mapGet(limits, priceKey(price));
}

Limit*
getBestBid(const Book *book){
    return book->highestBuy;
}

Limit*
getBestAsk(const Book *book){
    return book->lowestSell;
}
