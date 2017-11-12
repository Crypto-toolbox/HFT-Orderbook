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
        else if (currentLimit->limitPrice > limit->limitPrice){
            if(currentLimit->leftChild == NULL){
                currentLimit->leftChild = limit;
                limit->parent = currentLimit;
                return 1;
            }
            else{
                currentLimit = currentLimit->leftChild;
            }
        }
        else{ /*If its neither greater than nor smalle then it must be equal, and hence exist.*/
            break;
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
     * Serve all cases : Node has no children, node has one child, node has two children.
     *
     * Python Reference code here:
     *     https://en.wikipedia.org/wiki/Binary_search_tree#Deletion
     */

    if(!limitIsRoot(limit)){
        if(limit==limit->parent->leftChild && !limitIsRoot(limit->parent)){
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
    if(limit->leftChild != NULL && limit->rightChild != NULL){
        /*Limit has two children*/
        ptr_successor = getMinimumLimit(limit->rightChild);
        Limit *parent = ptr_successor->parent;
        Limit *leftChild = ptr_successor->rightChild;
        Limit *rightChild = ptr_successor->leftChild;


        if(limit->leftChild != ptr_successor){
            ptr_successor->leftChild = limit->leftChild;
        }
        else{
            ptr_successor->leftChild = NULL;
        }

        if(limit->rightChild != ptr_successor){
            ptr_successor->rightChild = limit->rightChild;
        }
        else{
            ptr_successor->rightChild = NULL;
        }
        limit->leftChild = leftChild;
        limit->rightChild = rightChild;
        ptr_successor->parent = limit->parent;
        if(ptr_successor->parent->rightChild==limit){
            ptr_successor->parent->rightChild = ptr_successor;
        }
        else if(ptr_successor->parent->leftChild==limit){
            ptr_successor->parent->leftChild = ptr_successor;
        }
        limit->parent = parent;

        removeLimit(limit);
    }
    else if(limit->leftChild != NULL && limit->rightChild == NULL){
        /*Limit has only left child*/
        replaceLimitInParent(limit, limit->leftChild);
    }
    else if(limit->leftChild != NULL && limit->rightChild == NULL){
        /*Limit has only left child*/
        replaceLimitInParent(limit, limit->rightChild);
    }
    else{
        /*Limit has no children*/
        replaceLimitInParent(limit, NULL);
    }
    return 1;
}