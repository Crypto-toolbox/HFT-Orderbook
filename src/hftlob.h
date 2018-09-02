#ifndef HFTLOB_H_
#define HFTLOB_H_
/**
 * CUSTOM STRUCTS
 */

typedef struct Order{
    char *oid;
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

typedef struct Book{
    Limit *bids;
    Limit *asks;
} Book;


/**
 * Book API Functions
 */

void
addOrderToBook(Book* ptr_book, Order* ptr_order);

int
removeOrderFromBook(Book* ptr_book, Order* ptr_order);


/**
 * INIT FUNCTIONS
 */

void
initBook(Book *book);

void
initOrder(Order *order);

void
initLimit(Limit *limit);


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

Limit*
addLimit(Limit *root, Limit *limit);

Limit*
removeLimit(Limit *limit);

int
getLimit(Limit *root, Limit *limit)

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

Limit*
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
replaceLimitInParent(Limit *limit, Limit *newLimit);

void
copyLimit(Limit *ptr_src, Limit *ptr_tar);

/**
 * CuTest Functions
 */

void RunAllTests(void);

#endif