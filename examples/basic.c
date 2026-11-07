/**
 * A minimal walk-through of the C Book API.
 *
 * Build & run (from the repo root):
 *     cmake -S . -B build && cmake --build build && ./build/hftlob_example
 */
#include <stdio.h>
#include "hftlob.h"

static void
printTop(const Book *book){
    Limit *bid = getBestBid(book);
    Limit *ask = getBestAsk(book);
    if(bid != NULL){
        printf("  best bid: %.2f x %.0f (%d orders)\n",
               bid->limitPrice, bid->size, bid->orderCount);
    }
    else{
        printf("  best bid: -\n");
    }
    if(ask != NULL){
        printf("  best ask: %.2f x %.0f (%d orders)\n",
               ask->limitPrice, ask->size, ask->orderCount);
    }
    else{
        printf("  best ask: -\n");
    }
}

int
main(void){
    Book *book = createBook();
    if(book == NULL){
        return 1;
    }

    /* The order sequence from issue #17. */
    printf("add 5879: sell 550 @ 26.00\n");
    addOrderToBook(book, "5879", HFTLOB_SELL, 550, 26.0);
    printTop(book);

    printf("cancel 5879\n");
    removeOrderFromBook(book, "5879");
    printTop(book);

    printf("add 5880: sell 80 @ 38.00, add 5881: sell 100 @ 22.70\n");
    addOrderToBook(book, "5880", HFTLOB_SELL, 80, 38.0);
    addOrderToBook(book, "5881", HFTLOB_SELL, 100, 22.7);
    printTop(book);

    /* Some bids, two at the same price: they queue first-in, first-out. */
    printf("add bids: 1 x 50 @ 21.00, 2 x 20 @ 22.00, 3 x 30 @ 22.00\n");
    addOrderToBook(book, "1", HFTLOB_BUY, 50, 21.0);
    addOrderToBook(book, "2", HFTLOB_BUY, 20, 22.0);
    addOrderToBook(book, "3", HFTLOB_BUY, 30, 22.0);
    printTop(book);
    printf("  oldest order at best bid: %s\n", getBestBid(book)->tailOrder->tid);

    /* A size change keeps queue position; a price change re-queues. */
    printf("replace 2: 15 @ 22.00 (size only), then 3: 30 @ 22.50 (new price)\n");
    replaceOrderInBook(book, "2", 15, 22.0);
    replaceOrderInBook(book, "3", 30, 22.5);
    printTop(book);

    printf("volume resting at 22.00: %.0f\n",
           getLimitFromBook(book, HFTLOB_BUY, 22.0)->size);

    freeBook(book);
    return 0;
}
