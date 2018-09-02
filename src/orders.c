/**
 * Order Operations
 */

#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <uuid/uuid.h>  // Requires compilation with '-luuid' flag.
#include "hftlob.h"

/**
 * API for Order Structs.
 */

void
initOrder(Order *ptr_order){
    /**
     * Initialize an Order struct at the given pointer.
     *
     * .. Note::
     *
     *      This initializer function differs slightly from the others, as it
     *      allocates a char* to ptr_order->oid and assigns a UUID to it, which
     *      acts as its unique order ID.
     *
     */
    ptr_order->oid = NULL;
    addOrderID(ptr_order);
    ptr_order->buyOrSell = -1;
    ptr_order->shares = 0;
    ptr_order->limit = 0;
    ptr_order->entryTime = 0;
    ptr_order->eventTime = 0;
    ptr_order->nextOrder = NULL;
    ptr_order->prevOrder = NULL;
    ptr_order->parentLimit = NULL;
    ptr_order->exchangeId = 0;
};

Order*
newOrder(int isBuy, double limit, double shares){
    /**
     * Create an Order struct and return a pointer to it.
     */
    Order *ptr_order = malloc(sizeof(Order));
    initOrder(ptr_order);
    assert(isBuy == 0 || isBuy == 1);
    ptr_order->buyOrSell = isBuy;
    ptr_order->limit = limit;
    ptr_order->shares;
    return ptr_order;
}

void
deleteOrder(Order *ptr_order){
    /**
     * Delete the Order struct at the given pointer.
     *
     * We only execute this function if the order is not part of a Limit
     * anymore.
     *
     * We free the order_id field (ptr_order->oid) first, then free the ptr
     * itself.
     */
    assert(ptr_order->parentLimit == NULL)
    assert(ptr_order->prevOrder == NULL)
    assert(ptr_order->nextOrder == NULL)
    free(ptr_order->oid);
    free(ptr_order);
}

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

int
addOrderID(Order *ptr_order){
    /**
     * Add an UUID-based Order ID to the Order struct at the given pointer.
     *
     * We safeguard against overrides if the given order already has an OID
     * assigned to it.
     */
    if (ptr_order->oid != NULL){return 0}
    ptr_order->oid = malloc(sizeof(char) * 37);
    uuid_generate_random(BINUUID);
    uuid_unparse(binuuid, ptr_order->oid);
    return 1;
}
