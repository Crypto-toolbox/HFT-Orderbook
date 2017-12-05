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
    Limit* tmp_ptr = child->rightChild;
    child->rightChild = limit;
    limit->leftChild = tmp_ptr;
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