/**
 * Limit operations
 *
 * Provides basic functions to add, remove and replace a limit struct inside
 * a Limit tree.
 *
 * A Limit struct is a a Binary-Search-Tree Node implementation. Functions to
 * modify and search this BST are found below.
 *
 * Functions defined in this module are considered PRIVATE and are generally
 * not intended to be used by the end-user.
 *
 */

#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "hftlob.h"


/**
 * API for Limit struct-based Binary Search Tree Implementation.
 */

Limit*
newRoot(void){
    /**
     * Create a Limit struct as root and return its pointer.
     */
    Limit *ptr_limit = malloc(sizeof(Limit));
    initLimit(ptr_limit);
    ptr_limit->limitPrice = -INFINITY;
    assert(isRoot(ptr_limit));
    return ptr_limit;
}

void
deleteRoot(Limit *ptr_root){
    /**
     * Delete the Limit struct acting as a BST root at the given pointer.
     *
     * We will traverse
     */
    assert(isRoot(ptr_root));
    Limit *ptr_foundLimit = getMinimumLimit(ptr_root);
    while (ptr_foundLimit != ptr_root){
        removeLimit(ptr_foundLimit);
        deleteLimit(ptr_foundLimit);
        ptr_foundLimit = getMinimumLimit(ptr_root);
    }
    deleteLimit(ptr_root);
}

Limit*
newLimit(double value){
    /**
     * Create a new Limit struct and return a pointer to it.
     */
    Limit *ptr_limit = malloc(sizeof(Limit));
    ptr_limit->limitPrice = value;
    return ptr_limit;
}

void
deleteLimit(Limit *ptr_limit){
    /**
     * Delete the Limit struct at the given pointer.
     *
     * This first asserts that the limit has no children left, before deleting
     * any orders that are still stored in the Limit struct.
     *
     */
    assert(ptr_limit->leftChild == NULL && ptr_limit->rightChild == NULL);
    while(ptr_limit->headOrder != NULL){
        deleteOrder(popOrder(ptr_limit));
    }
    free(ptr_limit);
}

Limit*
addLimit(Limit *ptr_root, Limit *ptr_limit){
    /**
     * Add a new Limit struct to the given limit tree.
     * Its left and right child will be set to NULL.
     * 
     * If it already exists, we return the existing limit. Otherwise, we
     * return the given limit after we added it to the tree.
     */
    Limit* existingLimit = limitExists(ptr_root, ptr_limit->limitPrice);
    if(existingLimit != NULL){
        return existingLimit;
    }
    ptr_limit->leftChild = NULL;
    ptr_limit->rightChild = NULL;


    Limit *ptr_currentLimit = ptr_root;
    Limit *child;
    while(1){
        if(ptr_currentLimit->limitPrice < ptr_limit->limitPrice){
            if(ptr_currentLimit->rightChild == NULL){
                ptr_currentLimit->rightChild = limit;
                ptr_limit->parent = ptr_currentLimit;
                return limit;
            }
            else{
                ptr_currentLimit = ptr_currentLimit->rightChild;
            }
        }
        else if (ptr_currentLimit->limitPrice > ptr_limit->limitPrice){
            if(ptr_currentLimit->leftChild == NULL){
                ptr_currentLimit->leftChild = limit;
                ptr_limit->parent = ptr_currentLimit;
                return limit;
            }
            else{
                ptr_currentLimit = ptr_currentLimit->leftChild;
            }
        }
        else{ 
        /*
         * If its neither greater than nor smaller then it must be equal, 
         * and therefore already exist. We shouldn't reach this point, due to 
         * the previous check up top, but just in case, we handle this here.
         */
         break;
        }
        continue;

    }
    return ptr_currentLimit;
}

Limit*
removeLimit(Limit *ptr_limit){
    /**
     * Remove the given limit from the tree it belongs to.
     *
     * This assumes it IS part of a limit tree.
     *
     * Python Reference code here:
     *     https://en.wikipedia.org/wiki/Binary_search_tree#Deletion
     */
    if(!hasGrandpa(ptr_limit) && limitIsRoot(ptr_limit)){
        return NULL;
    }

    Limit *ptr_successor = ptr_limit;
    if(ptr_limit->leftChild != NULL && ptr_limit->rightChild != NULL){
        /*Limit has two children*/
        ptr_successor = getMinimumLimit(ptr_limit->rightChild);
        Limit *parent = ptr_successor->parent;
        Limit *leftChild = ptr_successor->rightChild;
        Limit *rightChild = ptr_successor->leftChild;


        if(ptr_limit->leftChild != ptr_successor){
            ptr_successor->leftChild = ptr_limit->leftChild;
        }
        else{
            ptr_successor->leftChild = NULL;
        }

        if(ptr_limit->rightChild != ptr_successor){
            ptr_successor->rightChild = ptr_limit->rightChild;
        }
        else{
            ptr_successor->rightChild = NULL;
        }
        ptr_limit->leftChild = leftChild;
        ptr_limit->rightChild = rightChild;
        ptr_successor->parent = ptr_limit->parent;
        if(ptr_successor->parent->rightChild==ptr_limit){
            ptr_successor->parent->rightChild = ptr_successor;
        }
        else if(ptr_successor->parent->leftChild==ptr_limit){
            ptr_successor->parent->leftChild = ptr_successor;
        }
        ptr_limit->parent = parent;

        removeLimit(ptr_limit);
    }
    else if(ptr_limit->leftChild != NULL && ptr_limit->rightChild == NULL){
        /*Limit has only left child*/
        replaceLimitInParent(ptr_limit, ptr_limit->leftChild);
    }
    else if(ptr_limit->leftChild != NULL && ptr_limit->rightChild == NULL){
        /*Limit has only left child*/
        replaceLimitInParent(ptr_limit, ptr_limit->rightChild);
    }
    else{
        /*Limit has no children*/
        replaceLimitInParent(ptr_limit, NULL);
    }
    return ptr_limit;
}

Limit*
limitExists(Limit *root, int limitPrice){
    /**
     * Check if the given price level (value) exists in the
     * given limit tree (root).
     */
    if(root->parent == NULL && root->rightChild == NULL){
        return NULL;
    }
    Limit *currentLimit = root;
    while(currentLimit->limitPrice != limitPrice){
        if(currentLimit->leftChild == NULL && currentLimit->rightChild == NULL){
            return NULL;
        }
        else {
            if(currentLimit->rightChild != NULL && currentLimit->limitPrice < limitPrice){
                currentLimit = currentLimit->rightChild;
            }
            else if(currentLimit->leftChild != NULL && currentLimit->limitPrice > limitPrice){
                currentLimit = currentLimit->leftChild;
            }
            else{
                /*
                 * An Error occurred here. Indicate with a negative.
                 */
                return -1;
            }
            continue;
        }
    }
    return currentLimit;
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


/**
 * Lower-lever functions to balance a branch of a Binary Search Tree made up
 * of Limit structs.
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
     * Rotate tree nodes for LL Case.
     *
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
    Limit* tmp_ptr = child->rightChild;
    child->rightChild = limit;
    limit->leftChild = tmp_ptr;
    return;
}

void
rotateLeftRight(Limit *limit) {
    /**
     * Rotate tree nodes for LR Case.
     *
     * Reference:
     *     https://en.wikipedia.org/wiki/File:Tree_Rebalancing.gif
     */
    Limit *child = limit->leftChild;
    Limit *grandChild = limit->leftChild->rightChild;
    child->parent = grandChild;
    grandChild->parent = limit;

    Limit* tmp_b_ptr = child->leftChild;
    Limit* tmp_c_ptr = grandChild->leftChild;
    Limit* tmp_d_ptr = grandChild->rightChild;

    child->rightChild = grandChild->leftChild;
    limit->leftChild = grandChild;
    grandChild->leftChild = child;
    grandChild->rightChild = tmp_d_ptr;
    child->leftChild = tmp_b_ptr;
    child->rightChild = tmp_c_ptr;
    rotateLeftLeft(limit);
    return;
}

void
rotateRightRight(Limit *limit){
    /**
     * Rotate tree nodes for RR Case.
     *
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
    Limit* tmp_ptr = child->leftChild;
    child->leftChild = limit;
    limit->rightChild = tmp_ptr;
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
    Limit* tmp_b_ptr = child->rightChild;
    Limit* tmp_c_ptr = grandChild->rightChild;
    Limit* tmp_d_ptr = grandChild->leftChild;
    limit->rightChild = grandChild;
    grandChild->rightChild = child;
    grandChild->leftChild = tmp_d_ptr;
    child->leftChild = tmp_c_ptr;
    child->rightChild = tmp_b_ptr;

    rotateRightRight(limit);
    return;
}


/**
 * Lower-level functions to detect states, balance factors and heights of the
 * Limit BST's nodes and their children.
 */

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
replaceLimitInParent(Limit *ptr_limit, Limit *ptr_newLimit) {
    /**
     * Pop out the given limit and replace all pointers to it from limit->parent
     * to point to the newLimit.
     *
     * Serve all cases : Node has no children, node has one child, node has two children.
     *
     * Python Reference code here:
     *     https://en.wikipedia.org/wiki/Binary_search_tree#Deletion
     */

    if(!limitIsRoot(ptr_limit)){
        if(ptr_limit==ptr_limit->parent->leftChild && !limitIsRoot(ptr_limit->parent)){
            ptr_limit->parent->leftChild = ptr_newLimit;
        }
        else{
            ptr_limit->parent->rightChild = ptr_newLimit;
        }
    }
    if(ptr_newLimit!=NULL){
        ptr_newLimit->parent = ptr_limit->parent;
    }
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