#ifndef HFTLOB_H_
#define HFTLOB_H_
/**
 * CUSTOM STRUCTS
 */

typedef struct Order{
    char *tid;
    unsigned buyOrSell;
    double shares;
    double limit;
    double entryTime;
    double eventTime;
    struct Order *nextOrder;
    struct Order *prevOrder;
    struct Limit *parentLimit;
    int exchangeId;
} Order;

typedef struct Limit{
    double limitPrice;
    double size;
    double totalVolume;
    int orderCount;
    struct Limit *parent;
    struct Limit *leftChild;
    struct Limit *rightChild;
    struct Order *headOrder;
    struct Order *tailOrder;
} Limit;

typedef struct QueueItem{
    Limit *limit;
    struct QueueItem *previous;
} QueueItem;

typedef struct Queue{
    int size;
    QueueItem *head;
    QueueItem *tail;
} Queue;

/**
 * INIT FUNCTIONS
 */
void
initOrder(Order *order);

void
initLimit(Limit *limit);

void
initQueueItem(QueueItem *item);

void
initQueue(Queue *q);

/**
 * QUEUE FUNCTIONS
 */

void
pushToQueue(Queue *q, Limit *limit);

Limit*
popFromQueue(Queue *q);

int
queueIsEmpty(Queue *q);

/**
 * ORDER FUNCTIONS
 */

int
pushOrder(Limit *limit, Order *new_order);

Order*
popOrder(Limit *limit);

int
removeOrder(Order *order);

/**
 * BINARY SEARCH TREE FUNCTIONS
 */

Limit*
createRoot(void);

int
addNewLimit(Limit *root, Limit *limit);

void
replaceLimitInParent(Limit *limit, Limit *newLimit);

int
removeLimit(Limit *limit);

/**
 * BINARY SEARCH TREE BALANCING FUNCTIONS
 */

void
balanceBranch(Limit *limit);

void
rotateLeftLeft(Limit *limit);

void
rotateLeftRight(Limit *limit);

void
rotateRightRight(Limit *limit);

void
rotateRightLeft(Limit *limit);


/**
 * CONVENIENCE FUNCTIONS FOR BST OPERATIONS
 */

int
limitExists(Limit *root, Limit *limit);

int
limitIsRoot(Limit *limit);

int
hasGrandpa(Limit *limit);

Limit*
getGrandpa(Limit *limit);

Limit*
getMinimumLimit(Limit *limit);

Limit*
getMaximumLimit(Limit *limit);

int
getHeight(Limit *limit);

int
getBalanceFactor(Limit *limit);

void
copyLimit(Limit *ptr_src, Limit *ptr_tar);

/**
 * CuTest Functions
 * */

void RunAllTests(void);

#endif