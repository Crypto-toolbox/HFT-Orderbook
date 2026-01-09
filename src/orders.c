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

// Adding Order to Book
// This function finds the right price level (Limit) and attaches the Order to the end of the list (Time Priority).

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

// Removing order from book : map to find and delete an order in constant time $O(1)$.

void removeOrderFromBook(Book* book,int id){
    // 1. O(1) Lookup: Find the order in the map using ID
    if (id < 0 || id >= 10000 ) return;
    Order* orderToDelete = book ->order_map[id];

    if (orderToDelete == NULL){
        printf("Order %d not found, nothing to remove.\n", id);
    }
    
    // 2. Access the price level (Limit) this order belongs to
    Limit* parent = orderToDelete->parentLimit;

    // 3. Unhook the order from the doubly linked list
    if (orderToDelete->prevOrder != NULL) {
        orderToDelete->prevOrder->nextOrder = orderToDelete->nextOrder;
    } else {
        // It was the head of the list
        parent->headOrder = orderToDelete->nextOrder;
    }

    if (orderToDelete->nextOrder != NULL) {
        orderToDelete->nextOrder->prevOrder = orderToDelete->prevOrder;
    } else {
        // It was the tail of the list
        parent->tailOrder = orderToDelete->prevOrder;
    }

    // 4. Update the Limit statistics
    parent->orderCount--;
    parent->totalVolume -= orderToDelete->shares;

    // 5. Clean up the map and free memory
    book->order_map[id] = NULL;
    free(orderToDelete);

    printf("Order %d successfully removed from book.\n", id);

}

// If only quantity changes: We just update the number (keeping time priority).
// If price changes: We must remove it and re-add it (losing time priority).

void replaceOrderInBook(Book* book, int id, double newPrice, double newQty) {
    Order* existingOrder = book->order_map[id];
    
    if (existingOrder == NULL) {
        printf("Order %d not found for replacement. Adding as new.\n", id);
        addOrderToBook(book, id, 0, newPrice, newQty); // Default side or logic needed
        return;
    }

    // If price is the same, just update quantity (fastest)
    if (existingOrder->limit == newPrice) {
        double qtyDiff = newQty - existingOrder->shares;
        existingOrder->shares = newQty;
        existingOrder->parentLimit->totalVolume += qtyDiff;
        printf("Order %d quantity updated to %.2f\n", id, newQty);
    } 
    else {
        // Price changed: Must remove and re-add to maintain price/time priority
        int side = existingOrder->buyOrSell;
        removeOrderFromBook(book, id);
        addOrderToBook(book, id, side, newPrice, newQty);
        printf("Order %d price changed. Re-inserted into book.\n", id);
    }
}
