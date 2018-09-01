/**
 * Limit operations
 */

 #include <math.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "hftlob.h"


Limit*
createRoot(void){
    /**
     * Create a Limit structure as root and return a ptr to it.
     */
    Limit *ptr_limit = malloc(sizeof(Limit));
    initLimit(ptr_limit);
    ptr_limit->limitPrice = -INFINITY;
    return ptr_limit;
}

Limit*
addNewLimit(Limit *ptr_root, Limit *ptr_limit){
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