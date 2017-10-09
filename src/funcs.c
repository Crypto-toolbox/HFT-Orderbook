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
*/

int
limitExists(Limit *root, float value){
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
    if(limit->parent==NULL){
        return 1;
    }
    else{
        return 0;
    }
}

Node*
getMinimumLimit(Limit *limit){
    Limit *minimum = limit;
    while(minimum->leftChild != NULL){
        minimum = minimum->leftChild;
    }
    return minimum;
}

Node*
getMaximumLimit(Limit *limit){
    Limit *maximum = limit;
    while(maximum->rightChild != NULL){
        maximum = maximum->rightChild;
    }
    return minimum;
}

int
getHeight(Limit *limit){

    int leftHeight = 0;
    int rightHeight = 0;

    if(tmpChild->leftChild!=NULL)
        leftHeight = getHeight(limit->leftChild);
    }

    if(tmpChild->rightChild!=NULL){
        rightHeight = getHeight(limit->rightChild);
    }
    return leftHeight+1 ? leftHeight>rightHeight: rightHeight+1;
}

int
getBalanceFactor(Limit *limit){
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
pushOrder(Limit *limit, Order *new_order)
/* function to add an Order to a Limit struct at head*/
{
    new_order->parent_limit = limit;
    new_order->next = limit->headOrder;
    new_order->previous = NULL;

    // Check if our head is NULL (if the list is empty)
    if (limit->headOrder != NULL){
        // It isn't ! Push the new order
        limit->headOrder->previous = new_order;
    }
    else{
        // It is! Set the new Order as tail
        limit->tail = new_order;
    };

    // update head and increment counter of list
    limit->headOrder = new_order;
    limit->orderCount++;
    limit->size += new_order.size;
    limit->totalVolume += new_order.size * limit->limitPrice

    return;
}

int
popOrder(Limit *limit)
/* Pop function to remove tail from a Limit struct*/
{
    // Check if our list is empty (tail is Null)
    if (limit->tail == NULL){
        return 0;
    }
    // Pop the tail
    Order* oldTail = limit->tailOrder;

    if (limit->tailOrder->previous != NULL){  // If tail isn't also head
        limit->tailOrder = limit->tailOrder->previous;
        limit->tailOrder->next = NULL;
        limit->orderCount--;
        limit->size -= oldTail.size
        limit->totalVolume -= oldTail.size * limit->price
    }
    else{  // if tail IS also head
        limit->headOrder = NULL;
        limit->tailOrder = NULL
        limit->orderCount = 0;
        limit->size = 0;
        limit->totalVolume = 0;
    }


    return 1;
}
