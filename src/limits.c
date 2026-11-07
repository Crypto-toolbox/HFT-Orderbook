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
    if(ptr_limit == NULL){
        return NULL;
    }
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
    limit->height = 0;

    Limit *currentLimit = root;
    while(1){
        if(currentLimit->limitPrice < limit->limitPrice){
            if(currentLimit->rightChild == NULL){
                currentLimit->rightChild = limit;
                limit->parent = currentLimit;
                updateHeightsUpwards(currentLimit);
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
                updateHeightsUpwards(currentLimit);
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

Limit*
detachLimit(Limit *limit){
    /**
     * Remove the given limit from the tree it belongs to, without
     * rebalancing.
     *
     * Returns the lowest limit whose subtree changed (possibly the tree's
     * root sentinel), so callers can rebalance from there; or NULL if the
     * limit is a root sentinel and cannot be removed.
     *
     * Python Reference code here:
     *     https://en.wikipedia.org/wiki/Binary_search_tree#Deletion
     */
    Limit *changed;
    if(limitIsRoot(limit)){
        return NULL;
    }

    if(limit->leftChild != NULL && limit->rightChild != NULL){
        /*Limit has two children: splice in its in-order successor (see #34).*/
        Limit *ptr_successor = getMinimumLimit(limit->rightChild);

        if(limit->rightChild != ptr_successor){
            changed = ptr_successor->parent;
            replaceLimitInParent(ptr_successor, ptr_successor->rightChild);
            ptr_successor->rightChild = limit->rightChild;
            ptr_successor->rightChild->parent = ptr_successor;
        }
        else{
            changed = ptr_successor;
        }

        replaceLimitInParent(limit, ptr_successor);
        ptr_successor->leftChild = limit->leftChild;
        ptr_successor->leftChild->parent = ptr_successor;
    }
    else{
        /*Limit has at most one child*/
        changed = limit->parent;
        replaceLimitInParent(limit,
                limit->leftChild != NULL ? limit->leftChild : limit->rightChild);
    }

    limit->parent = NULL;
    limit->leftChild = NULL;
    limit->rightChild = NULL;
    limit->height = 0;
    updateHeightsUpwards(changed);
    return changed;
}

int
removeLimit(Limit *limit){
    /**
     * Remove the given limit from the tree it belongs to, without
     * rebalancing. Returns 1 on success, 0 if limit is a root sentinel.
     */
    return detachLimit(limit) != NULL;
}
