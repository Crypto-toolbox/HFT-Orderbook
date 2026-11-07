/*
Binary Search Tree Operations
*/

#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "hftlob.h"

void
balanceBranch(Limit *limit) {
    /**
     * Balance the nodes of the given branch of Limit structs.
     *
     * Branches with a balance factor within [-1, 1] are left untouched.
     */

    int balanceFactor = getBalanceFactor(limit);
    if(balanceFactor > 1){
        /*Right is heavier.*/
        balanceFactor = getBalanceFactor(limit->rightChild);
        if(balanceFactor < 0){
            rotateRightLeft(limit);
        }
        else{
            rotateRightRight(limit);
        }
    }
    else if(balanceFactor < -1){
        /*Left is heavier.*/
        balanceFactor = getBalanceFactor(limit->leftChild);
        if(balanceFactor > 0){
            rotateLeftRight(limit);
        }
        else{
            rotateLeftLeft(limit);
        }
    }
    else{/*Everything is fine, do nothing*/}
}

static void
rotateLeft(Limit *limit){
    /* Our right child takes our place; we become its left child. */
    Limit *child = limit->rightChild;
    replaceLimitInParent(limit, child);
    limit->rightChild = child->leftChild;
    if(limit->rightChild != NULL){
        limit->rightChild->parent = limit;
    }
    child->leftChild = limit;
    limit->parent = child;
    updateHeight(limit);
    updateHeight(child);
}

static void
rotateRight(Limit *limit){
    /* Our left child takes our place; we become its right child. */
    Limit *child = limit->leftChild;
    replaceLimitInParent(limit, child);
    limit->leftChild = child->rightChild;
    if(limit->leftChild != NULL){
        limit->leftChild->parent = limit;
    }
    child->rightChild = limit;
    limit->parent = child;
    updateHeight(limit);
    updateHeight(child);
}

void
rotateLeftLeft(Limit *limit) {
    /**
     * Rotate tree nodes for LL Case
     * Reference:
     *     https://en.wikipedia.org/wiki/File:Tree_Rebalancing.gif
     */
    rotateRight(limit);
}

void
rotateLeftRight(Limit *limit) {
    /**
     * Rotate tree nodes for LR Case
     * Reference:
     *     https://en.wikipedia.org/wiki/File:Tree_Rebalancing.gif
     */
    rotateLeft(limit->leftChild);
    rotateRight(limit);
}

void
rotateRightRight(Limit *limit){
    /**
     * Rotate tree nodes for RR Case
     * Reference:
     *     https://en.wikipedia.org/wiki/File:Tree_Rebalancing.gif
     */
    rotateLeft(limit);
}

void
rotateRightLeft(Limit *limit){
    /**
     * Rotate tree nodes for RL Case.
     *
     * Reference:
     *     https://en.wikipedia.org/wiki/File:Tree_Rebalancing.gif
     */
    rotateRight(limit->rightChild);
    rotateLeft(limit);
}

void
rebalanceUpwards(Limit *limit){
    /**
     * Refresh cached heights and restore the AVL property from the given
     * limit up to the tree's root sentinel. Call it on the lowest limit
     * whose subtree changed after an insert or removal.
     */
    while(limit != NULL && !limitIsRoot(limit)){
        updateHeight(limit);
        balanceBranch(limit);
        limit = limit->parent;
    }
}
