typedef struct Order{
    int tid;
    unsigned buyOrSell;
    float shares;
    int limit;
    int entryTime;
    int eventTime;
    struct Order *nextOrder;
    struct Order *prevOrder;
    struct Limit *parentLimit;
} Order;

typedef struct Limit{
    float limitPrice;
    float size;
    float totalVolume;
    int orderCount;
    struct Limit *parent;
    struct Limit *leftChild;
    struct Limit *rightChild;
    struct Order *headOrder;
    struct Order *tailOrder;
} Limit;

void
pushOrder(Limit *limit, Order *new_order);

int
popOrder(Limit *limit);

Limit*
createRoot(void);

int
addNewLimit(Limit *root, Limit *limit);

void
replaceLimitInParent(Limit *limit, Limit *newLimit);

int
removeLimit(Limit *limit);

/*
Limit-related BST rotation functions.
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


/*
Limit-related convenience functions to query attributes
about a Limit struct.

These are mainly used to make important code parts more readable,
by being more descriptive.
*/

int
limitExists(Limit *root, float value);

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


/**
 * CuTest Functions
 * */

void RunAllTests(void);