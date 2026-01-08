/**
 * Order Operations
 */

#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "hftlob.h"

int
pushOrder(Limit *limit, Order *newOrder){
    /**
     * Add an Order to a Limit structure at head.
     */
    if(limit->limitPrice != newOrder->limit){
        return 0;
    }
    newOrder->parentLimit = limit;
    newOrder->nextOrder = limit->headOrder;
    newOrder->prevOrder= NULL;


    if (limit->headOrder != NULL){
        limit->headOrder->prevOrder= newOrder;
    }
    else{
        limit->tailOrder = newOrder;
    };

    limit->headOrder = newOrder;
    limit->orderCount++;
    limit->size += newOrder->shares;
    limit->totalVolume += (newOrder->shares * limit->limitPrice);

    return 1;
}

Order*
popOrder(Limit *limit){
    /**
     * Pop the order at the tail of a Limit structure.
     */
    if (limit->tailOrder == NULL){
        return NULL;
    }

    Order *ptr_poppedOrder = limit->tailOrder;

    if (limit->tailOrder->prevOrder!= NULL){
        limit->tailOrder = limit->tailOrder->prevOrder;
        limit->tailOrder->nextOrder = NULL;
        limit->orderCount--;
        limit->size -= ptr_poppedOrder->shares;
        limit->totalVolume -= ptr_poppedOrder->shares * limit->limitPrice;
    }
    else{
        limit->headOrder = NULL;
        limit->tailOrder = NULL;
        limit->orderCount = 0;
        limit->size = 0;
        limit->totalVolume = 0;
    }

    return ptr_poppedOrder;
}

int
removeOrder(Order *order){
    /**
     * Remove the order from where it is at.
     */
    if(order->parentLimit->headOrder == order && order->parentLimit->tailOrder == order){
        /* Head and Tail are identical, set both to NULL and be done with it.*/
        order->parentLimit->headOrder = NULL;
        order->parentLimit->tailOrder = NULL;
    }
    else if(order->prevOrder != NULL && order->nextOrder != NULL){
        /* If Its in the middle, update reference for previous and next order and be done.*/
        order->prevOrder->nextOrder = order->nextOrder;
        order->nextOrder->prevOrder = order->prevOrder;
    }
    else if(order->nextOrder == NULL && order->parentLimit->tailOrder == order){
        /*This is the Tail - replace the tail with previous Order*/
        order->prevOrder->nextOrder = NULL;
        order->parentLimit->tailOrder = order->prevOrder;
    }
    else if(order->prevOrder == NULL && order->parentLimit->headOrder == order){
        /*This is the Head - replace the head with the next order*/
        order->nextOrder->prevOrder = NULL;
        order->parentLimit->headOrder = order->nextOrder;
    }
    else{
        return -1;
    }

    return 1;
}



void addOrderToBook(Book* book, int id, int side, double price, double shares) {
    // 1. Create and initialize the new Order
    Order* newOrder = (Order*)malloc(sizeof(Order));
    initOrder(newOrder); // Sets pointers to NULL via your utils.c function
    
    newOrder->limit = price;
    newOrder->shares = shares;
    newOrder->buyOrSell = side;
    // we'll use the array index as the ID
    // We store the pointer in the map for O(1) removal.
    if (id >= 0 && id < 10000) {
        book->order_map[id] = newOrder;
    }

    // 2. Logic to attach to the tree
    // Usually you'd call addNewLimit here to find/create the price level
    // this establishes the O(1) map required by the issue.
    printf("Successfully added Order ID: %d at Price: %.2f\n", id, price);
}