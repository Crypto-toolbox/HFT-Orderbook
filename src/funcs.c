/**
 * Contains all functions required to run a HFT limit order book.
 */
#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "hftlob.h"

/**
 * Init functions for structs
 */

void 
initOrder(Order *order){
    order->tid = "NULL";
    order->buyOrSell = 0;
    order->shares = 0.0;
    order->limit = 0.0;
    order->entryTime = 0.0;
    order->eventTime = 0.0;
    order->Order = NULL;
    order->Order = NULL;
    order->Limit = NULL;
};

void
initLimit(Limit *limit){
    limit->limitPrice = 0.0;
    limit->size = 0.0;
    limit->totalVolume = 0.0;
    limit->orderCount = 0;
    limit->parent = NULL;
    limit->leftChild = NULL;
    limit->rightChild = NULL;
    limit->headOrder = NULL;
    limit->tailOrder = NULL;
};

void
initQueueItem(QueueItem *item){
    item->limit = NULL;
    item->previous = NULL;
}

/*
Functions for Order related operations
*/

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

int
popOrder(Limit *limit){
    /**
     * Pop the order at the tail of a Limit structure.
     */
    if (limit->tailOrder == NULL){
        return 0;
    }

    Order* oldTail = limit->tailOrder;

    if (limit->tailOrder->prevOrder!= NULL){
        limit->tailOrder = limit->tailOrder->prevOrder;
        limit->tailOrder->nextOrder = NULL;
        limit->orderCount--;
        limit->size -= oldTail->shares;
        limit->totalVolume -= oldTail->shares * limit->limitPrice;
    }
    else{
        limit->headOrder = NULL;
        limit->tailOrder = NULL;
        limit->orderCount = 0;
        limit->size = 0;
        limit->totalVolume = 0;
    }


    return 1;
}

/*
Limit-related data operations
*/

Limit*
createRoot(void){
    /**
     * Create a Limit structure as root and return a ptr to it.
     */
    Limit *ptr_limit = malloc(sizeof(Limit));
    ptr_limit->parent = NULL;
    ptr_limit->leftChild = NULL;
    ptr_limit->rightChild = NULL;
    ptr_limit->limitPrice = -INFINITY;
    return ptr_limit;
}

int
addNewLimit(Limit *root, Limit *limit){
    /**
     * Add a new Limit struct to the given limit tree.
     *
     * Asserts that the limit does not yet exist.
     * Also sets left and right child to NULL.
     */
    if(limitExists(root, limit) == 1){
        return 0;
    }
    limit->leftChild = NULL;
    limit->rightChild = NULL;


    Limit *currentLimit = root;
    Limit *child;
    while(1){
        if(currentLimit->limitPrice < limit->limitPrice){
            if(currentLimit->rightChild == NULL){
                currentLimit->rightChild = limit;
                limit->parent = currentLimit;
                return 1;
            }
            else{
                currentLimit = currentLimit->rightChild;
            }
        }
        else if (currentLimit->limitPrice < limit->limitPrice){
            if(currentLimit->leftChild == NULL){
                currentLimit->leftChild = limit;
                limit->parent = currentLimit;
                return 1;
            }
            else{
                currentLimit = currentLimit->leftChild;
            }
        }
        continue;

    }
    return 0;
}

void
replaceLimitInParent(Limit *limit, Limit *newLimit) {
    /**
     * Pop out the given limit and replace all pointers to it from limit->parent
     * to point to the newLimit.
     *
     * Python Reference code here:
     *     https://en.wikipedia.org/wiki/Binary_search_tree#Deletion
     */
    if(!limitIsRoot(limit)){
        if(limit==limit->parent->leftChild){
            limit->parent->leftChild = newLimit;
        }
        else{
            limit->parent->rightChild = newLimit;
        }
    }
    if(newLimit!=NULL){
        newLimit->parent = limit->parent;
    }
}

int
removeLimit(Limit *limit){
    /**
     * Remove the given limit from the tree it belongs to.
     *
     * This assumes it IS part of a limit tree.
     *
     * Python Reference code here:
     *     https://en.wikipedia.org/wiki/Binary_search_tree#Deletion
     */
    if(!hasGrandpa(limit) && limitIsRoot(limit)){
        return 0;
    }

    Limit *ptr_successor = limit;
    while(limit->leftChild!=NULL && limit->rightChild!=NULL){
        ptr_successor = getMinimumLimit(limit->rightChild);
    }

    if(limit->leftChild!=NULL){
        replaceLimitInParent(limit, ptr_successor->leftChild);
    }
    else if(limit->rightChild!=NULL){
        replaceLimitInParent(limit, ptr_successor->rightChild);
    }
    else{
        replaceLimitInParent(limit, NULL);
    }

    free(limit);
    return 1;
}


/*
Limit-related BST rotation functions.
*/

void
balanceBranch(Limit *limit) {
    /**
     * Balance the nodes of the given branch of Limit structs.
     *
     * Asserts that limit has a height of at least 2.
     */
    assert(getHeight(limit) < 2);

    int balanceFactor = getBalanceFactor(limit);
    if(balanceFactor > 1){
        /*Right is heavier.*/
        balanceFactor = getBalanceFactor(limit->rightChild);
        if(balanceFactor < 0){
            rotateRightLeft(limit);
        }
        else if(balanceFactor > 0){
            rotateRightRight(limit);
        }
    }
    else if(balanceFactor < -1){
        /*Left is heavier.*/
        balanceFactor = getBalanceFactor(limit->leftChild);
        if(balanceFactor < 0){
            rotateLeftLeft(limit);
        }
        else if(balanceFactor > 0){
            rotateLeftRight(limit);
        }
    }
    else{/*Everything is fine, do nothing*/}
}

void
rotateLeftLeft(Limit *limit) {
    /**
     * Rotate tree nodes for LL Case
     * Reference:
     *     https://en.wikipedia.org/wiki/File:Tree_Rebalancing.gif
     */
    Limit *child = limit->leftChild;
    if(limitIsRoot(limit->parent)==1 || limit->limitPrice > limit->parent->limitPrice){
        limit->parent->rightChild = child;
    }
    else{
        limit->parent->leftChild = child;
    }
    child->parent = limit->parent;
    limit->parent = child;
    child->rightChild = limit;
    limit->leftChild = child->rightChild;
    return;
}

void
rotateLeftRight(Limit *limit) {
    /**
     * Rotate tree nodes for LR Case
     * Reference:
     *     https://en.wikipedia.org/wiki/File:Tree_Rebalancing.gif
     */
    Limit *child = limit->leftChild;
    Limit *grandChild = limit->leftChild->rightChild;
    child->parent = grandChild;
    grandChild->parent = limit;
    child->rightChild = grandChild->leftChild;
    limit->leftChild = grandChild;
    grandChild->leftChild = child;
    rotateLeftLeft(limit);
    return;
}

void
rotateRightRight(Limit *limit){
    /**
     * Rotate tree nodes for RR Case
     * Reference:
     *     https://en.wikipedia.org/wiki/File:Tree_Rebalancing.gif
     */
    Limit *child = limit->rightChild;
    if(limitIsRoot(limit->parent)==1 || limit->limitPrice > limit->parent->limitPrice){
        limit->parent->rightChild = child;
    }
    else{
        limit->parent->leftChild = child;
    }
    child->parent = limit->parent;
    limit->parent = child;
    child->leftChild = limit;
    limit->rightChild = child->leftChild;
    return;
}

void
rotateRightLeft(Limit *limit){
    /**
     * Rotate tree nodes for RL Case.
     *
     * Reference:
     *     https://en.wikipedia.org/wiki/File:Tree_Rebalancing.gif
     */
    Limit *child = limit->rightChild;
    Limit *grandChild = limit->rightChild->leftChild;
    child->parent = grandChild;
    grandChild->parent = limit;
    child->leftChild = grandChild->rightChild;
    limit->rightChild = grandChild;
    grandChild->rightChild = child;
    rotateRightRight(limit);
    return;
}


/*
Limit-related convenience functions to query attributes
about a Limit struct.

These are mainly used to make important code parts more readable,
by being more descriptive.
*/

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
    Limit *minimum = limit;
    while(minimum->leftChild != NULL){
        minimum = minimum->leftChild;
    }
    return minimum;
}

Limit*
getMaximumLimit(Limit *limit){
    /**
     * Return the right-most limit struct for the given limit
     * tree / branch.
     */
    Limit *maximum = limit;
    while(maximum->rightChild != NULL){
        maximum = maximum->rightChild;
    }
    return maximum;
}

int
getHeight(Limit *limit){
    /**
     * Calculate the height of the limits under the passed limit non-recursively.
     */
    Queue queue;
    Queue *ptr_queue = &queue;
    int height = 0;
    Limit *ptr_current;
    pushToQueue(ptr_queue, limit);
    pushToQueue(ptr_queue, NULL);
    while(!queueIsEmpty(ptr_queue)){
        ptr_current = popFromQueue(ptr_queue);
        if(ptr_current == NULL){
            if(!queueIsEmpty(ptr_queue)){
                pushToQueue(ptr_queue, NULL);
            }
            height++;
        }
        else{
            if(ptr_current->leftChild!=NULL){
                pushToQueue(ptr_queue, ptr_current->leftChild);
            }
            if(ptr_current->rightChild!=NULL){
                pushToQueue(ptr_queue, ptr_current->rightChild);
            }
        }
    }
    free(ptr_current);
    return height;
}

int
getBalanceFactor(Limit *limit){
    /**
     * Calculate the balance factor of the passed limit, by
     * subtracting the left children's height from the right children's
     * height.
     */
    int leftHeight = 0;
    int rightHeight = 0;
    if(limit->rightChild!=NULL){
        rightHeight = getHeight(limit->rightChild);
    }

    if(limit->leftChild!=NULL){
        leftHeight = getHeight(limit->leftChild);
    }
    return rightHeight - leftHeight;
}



