/*
Limit-related convenience functions.

These are mainly used to make important code parts more readable,
by being more descriptive.
*/

#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "hftlob.h"


void
initOrder(Order *order){
    order->tid = NULL;
    order->buyOrSell = -1;
    order->shares = 0;
    order->limit = 0;
    order->entryTime = 0;
    order->eventTime = 0;
    order->nextOrder = NULL;
    order->prevOrder = NULL;
    order->parentLimit = NULL;
    order->exchangeId = 0;
};

void
initLimit(Limit *limit){
    limit->limitPrice = 0;
    limit->size = 0;
    limit->totalVolume = 0;
    limit->orderCount = 0;
    limit->parent = NULL;
    limit->leftChild = NULL;
    limit->rightChild = NULL;
    limit->headOrder = NULL;
    limit->headOrder = NULL;
};

void
initQueueItem(QueueItem *item){
    item->limit = NULL;
    item->previous = NULL;
};

void
initQueue(Queue *q){
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
};

int
limitExists(Limit *root, Limit *limit){
    /**
     * Check if the given price level (value) exists in the
     * given limit tree (root).
     */
    if(root->parent == NULL && root->rightChild == NULL){
        return 0;
    }
    Limit *currentLimit = root;
    while(currentLimit->limitPrice != limit->limitPrice){
        if(currentLimit->leftChild == NULL && currentLimit->rightChild==NULL){
            return 0;
        }
        else {
            if(currentLimit->rightChild != NULL && currentLimit->limitPrice < limit->limitPrice){
                currentLimit = currentLimit->rightChild;
            }
            else if(currentLimit->leftChild != NULL && currentLimit->limitPrice > limit->limitPrice){
                currentLimit = currentLimit->leftChild;
            }
            else{
                return -1;
            }
            continue;
        }
    }
    return 1;
}

int
limitIsRoot(Limit *limit){
    /**
     * Check if the given limit is the root of the limit tree.
     */
    if(limit->parent==NULL){
        return 1;
    }
    else{
        return 0;
    }
}

int
hasGrandpa(Limit *limit){
    /**
     * Check if there is a parent to the passed limit's parent.
     */
    if(limit->parent != NULL && limit->parent->parent != NULL){
        return 1;
    }
    else{
        return 0;
    }
}

Limit*
getGrandpa(Limit *limit){
    /**
     * Return the limit's parent parent.
     */
    if(hasGrandpa(limit)){
        return limit->parent->parent;
    }
    return NULL;
}

Limit*
getMinimumLimit(Limit *limit){
    /**
     * Return the left-most limit struct for the given limit
     * tree / branch.
     */
    Limit *ptr_minimum;
    if(limitIsRoot(limit)){
        ptr_minimum = limit->rightChild;
    }
    else{
        ptr_minimum = limit;
    }

    while(ptr_minimum->leftChild != NULL){
        ptr_minimum = ptr_minimum->leftChild;
    }
    return (ptr_minimum);
}

Limit*
getMaximumLimit(Limit *limit){
    /**
     * Return the right-most limit struct for the given limit
     * tree / branch.
     */
    Limit *ptr_maximum = limit;
    while(ptr_maximum->rightChild != NULL){
        ptr_maximum = ptr_maximum->rightChild;
    }
    return (ptr_maximum);
}

int
getHeight(Limit *limit){
    /**
     * Calculate the height of the limits under the passed limit non-recursively.
     */

     if(limit == NULL){
        return -1;
     }

    int height = -1; /*Set to -1; if limit has no children, this will end up being 0*/
    int qsize = 0;

    Queue *ptr_queue = malloc(sizeof(Queue));
    initQueue(ptr_queue);
    Limit *ptr_current;
    pushToQueue(ptr_queue, limit);
    while(1){
        qsize = ptr_queue->size;
        if(qsize == 0){
            break;
        }
        height++;
        while(qsize > 0){
            ptr_current = popFromQueue(ptr_queue);
            if(ptr_current->leftChild != NULL){
                pushToQueue(ptr_queue, ptr_current->leftChild);
            }
            if(ptr_current->rightChild != NULL){
                pushToQueue(ptr_queue, ptr_current->rightChild);
            }
            qsize--;
        }
    }
    free(ptr_queue);
    return height;
}

int
getBalanceFactor(Limit *limit){
    /**
     * Calculate the balance factor of the passed limit, by
     * subtracting the left children's height from the right children's
     * height.
     */
    int leftHeight = -1;
    int rightHeight = -1;
    if(limit->rightChild!=NULL){
        rightHeight = getHeight(limit->rightChild);
    }

    if(limit->leftChild!=NULL){
        leftHeight = getHeight(limit->leftChild);
    }
    return rightHeight - leftHeight;
}

void
copyLimit(Limit *ptr_src, Limit *ptr_tar){
    /**
     * Copy the values of src limit to tar limit.
     */
    ptr_tar->limitPrice = ptr_src->limitPrice;
    ptr_tar->size = ptr_src->size;
    ptr_tar->totalVolume = ptr_src->totalVolume;
    ptr_tar->orderCount = ptr_src->orderCount;
    ptr_tar->headOrder = ptr_src->headOrder;
    ptr_tar->tailOrder = ptr_src->tailOrder;
    Order *ptr_order = ptr_tar->headOrder;

    while(ptr_order != NULL){
        ptr_order->parentLimit = ptr_tar;
        if(ptr_order != NULL){
            ptr_order = ptr_order->nextOrder;
        }
    }
}