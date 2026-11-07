/**
 * Micro-benchmark for the Book API.
 *
 *     cmake -S . -B build-rel -DCMAKE_BUILD_TYPE=Release
 *     cmake --build build-rel && ./build-rel/hftlob_bench [orders] [levels]
 *
 * Adds `orders` orders spread over `levels` price levels per side, then
 * resizes half of them, cancels half, and sweeps the bid side with sells.
 * Prints the best of three runs per phase.
 */
#define _POSIX_C_SOURCE 199309L  /* clock_gettime */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "hftlob.h"

#define RUNS 3

static double
now(void){
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec / 1e9;
}

static void
keepBest(double *best, double value){
    if(*best == 0 || value < *best){
        *best = value;
    }
}

int
main(int argc, char **argv){
    int n = argc > 1 ? atoi(argv[1]) : 1000000;
    int levels = argc > 2 ? atoi(argv[2]) : 5000;
    unsigned *side = malloc(n * sizeof(*side));
    double *price = malloc(n * sizeof(*price));
    double *qty = malloc(n * sizeof(*qty));
    double add = 0, resize = 0, cancel = 0, execute = 0, teardown = 0;
    long executes = 0;
    int run, i;

    if(side == NULL || price == NULL || qty == NULL){
        return 1;
    }
    srand(1);
    for(i = 0; i < n; ++i){
        side[i] = (i & 1) ? HFTLOB_BUY : HFTLOB_SELL;
        qty[i] = 1 + rand() % 100;
        price[i] = (rand() % levels) + (side[i] == HFTLOB_BUY ? 0 : levels);
    }

    for(run = 0; run < RUNS; ++run){
        Book *book = createBook();
        double t;
        long calls = 0;

        t = now();
        for(i = 0; i < n; ++i){
            addOrderToBook(book, (OrderId)i, side[i], qty[i], price[i]);
        }
        keepBest(&add, (now() - t) * 1e9 / n);

        t = now();
        for(i = 0; i < n; i += 2){
            replaceOrderInBook(book, (OrderId)i, qty[i] + 1, price[i]);
        }
        keepBest(&resize, (now() - t) * 1e9 / (n / 2));

        t = now();
        for(i = 0; i < n; i += 4){
            removeOrderFromBook(book, (OrderId)i);
            removeOrderFromBook(book, (OrderId)i + 1);
        }
        keepBest(&cancel, (now() - t) * 1e9 / (n / 2));

        t = now();
        while(getBestBid(book) != NULL){
            executeInBook(book, HFTLOB_SELL, 50, -1e18, NULL, NULL);
            calls++;
        }
        keepBest(&execute, (now() - t) * 1e9 / calls);
        executes = calls;

        t = now();
        freeBook(book);
        keepBest(&teardown, (now() - t) * 1e3);
    }

    printf("%d orders, %d levels per side, best of %d runs\n", n, levels, RUNS);
    printf("  add      %7.1f ns/op\n", add);
    printf("  resize   %7.1f ns/op\n", resize);
    printf("  cancel   %7.1f ns/op\n", cancel);
    printf("  execute  %7.1f ns/call (%ld calls)\n", execute, executes);
    printf("  freeBook %7.1f ms\n", teardown);
    free(side);
    free(price);
    free(qty);
    return 0;
}
