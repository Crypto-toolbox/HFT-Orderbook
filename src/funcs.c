#include 'hftlob.h'
/*
Limit-related data operations
*/

int
treeInsertOrder(Limit *limit, Order *order){
    /*
    Insert the given order into a Limit tree.
    if the Limit doesn't exist yet, create it and append
    the given order to it.
    */
    return 0;
}

int
treePopOrder(Limit *limit, Order *order){
    /*
    Remove an order from the given Limit struct.

    If the Limit struct would have no orders left, remove the Limit
    from the tree.
    */
    return 0;
}

/*
Limit-related BST rotation functions.
*/
void
rotateLeftLeft(Limit *limit) {
    /*
    Rotate tree nodes for LL Case
    Reference:
        https://en.wikipedia.org/wiki/File:Tree_Rebalancing.gif
    */
    Limit *child = limit->leftChild;
    if(limitIsRoot(limit->parent)==1 || limit->price > limit->parent->price){
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
    /*
    Rotate tree nodes for LR Case
    Reference:
        https://en.wikipedia.org/wiki/File:Tree_Rebalancing.gif
    */
    return;
}

void
rotateRightRight(Limit *limit){
    /*
    Rotate tree nodes for RR Case
    Reference:
        https://en.wikipedia.org/wiki/File:Tree_Rebalancing.gif
    */
    Limit *child = limit->rightChild;
    if(limitIsRoot(limit->parent)==1 || limit->price > limit->parent->price){
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
    /*
    Rotate tree nodes for RL Case
    Reference:
        https://en.wikipedia.org/wiki/File:Tree_Rebalancing.gif
    */
    return;
}

/*
Limit-related convenience fucntions to query attributes
about a Limit struct.

These are mainly used to make important code parts more readable,
by being more descriptive.
*/

int
limitExists(Limit *root, float value){
    /*
    Check if the given price level (value) exists in the
    given limit tree (root).
    */
    float current;
    Limit *currentLimit = root;
    while(1){
        if(currentLimit->price == value){
            return 1;
        }
        else if(currentLimit->leftChild==NULL && currentLimit->rightChild==NULL){
            return 0;
        }
        else if(currentLimit->price < value){
            currentLimit = currentLimit->leftChild;
        }
        else if(currentLimit->price > value){
            currentLimit = currentLimit->rightChild
        }
    }
}

int
limitIsRoot(Limit *limit){
    /*
    Check if the given limit is the root of the limit tree.
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
    /*
    Check if there is a parent to the passed limit's parent.
    */
    if(limit->parent != NULL && limit->parent->parent != NULL){
        return 1;
    }
    else{
        return 0;
    }
}

Node*
getGrandpa(Limit *limit){
    /*
    Return the limit's parent parent.
    */
    return limit->parent->parent;
}

Node*
getMinimumLimit(Limit *limit){
    /*
    Return the left-most limit struct for the given limit
    tree / branch.
    */
    Limit *minimum = limit;
    while(minimum->leftChild != NULL){
        minimum = minimum->leftChild;
    }
    return minimum;
}

Node*
getMaximumLimit(Limit *limit){
    /*
    Return the right-most limit struct for the given limit
    tree / branch.
    */
    Limit *maximum = limit;
    while(maximum->rightChild != NULL){
        maximum = maximum->rightChild;
    }
    return minimum;
}

int
getHeight(Limit *limit){
    /*
    Calculate the height of the limits under the passed limit.
    */
    int leftHeight = 0;
    int rightHeight = 0;

    if(tmpChild->leftChild!=NULL){
        leftHeight = getHeight(limit->leftChild);
    }

    if(tmpChild->rightChild!=NULL){
        rightHeight = getHeight(limit->rightChild);
    }
    return leftHeight+1 ? leftHeight>rightHeight: rightHeight+1;
}

int
getBalanceFactor(Limit *limit){
    /*
    Calculate the balance factor of the passed limit, by
    subtracting the left children's height from the right children's
    height.
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


/*
Functions for Order related operations
*/
void
pushOrder(Limit *limit, Order *new_order){
    /*
    function to add an Order to a Limit struct at head
    */
    new_order->parent_limit = limit;
    new_order->next = limit->headOrder;
    new_order->previous = NULL;


    if (limit->headOrder != NULL){
        limit->headOrder->previous = new_order;
    }
    else{
        limit->tail = new_order;
    };

    limit->headOrder = new_order;
    limit->orderCount++;
    limit->size += new_order.size;
    limit->totalVolume += new_order.size * limit->limitPrice

    return;
}

int
popOrder(Limit *limit){
    /*
    Pop function to remove tail from a Limit struct.
    */

    if (limit->tail == NULL){
        return 0;
    }

    Order* oldTail = limit->tailOrder;

    if (limit->tailOrder->previous != NULL){
        limit->tailOrder = limit->tailOrder->previous;
        limit->tailOrder->next = NULL;
        limit->orderCount--;
        limit->size -= oldTail.size
        limit->totalVolume -= oldTail.size * limit->price
    }
    else{
        limit->headOrder = NULL;
        limit->tailOrder = NULL
        limit->orderCount = 0;
        limit->size = 0;
        limit->totalVolume = 0;
    }


    return 1;
}
