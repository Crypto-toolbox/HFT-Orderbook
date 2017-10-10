#include 'funcs.c'

typedef struct Order{
    int tid;
    unsigned buyOrSell;
    int shares;
    int limit;
    int entryTime;
    int eventTime;
    struct Order *nextOrder;
    struct Order *prevOrder;
    struct Limit *parentLimit;
};

void
pushOrder(Limit *limit, Order *new_order);

int
popOrder(Limit *limit);

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
};

int
treeInsertOrder(Limit *limit, Order *order);

int
treePopOrder(Limit *limit, Order *order);

void
rotateLeftLeft(Limit *limit);


void
rotateLeftRight(Limit *limit);

void
rotateRightRight(Limit *limit);

void
rotateRightLeft(Limit *limit);



/*
Limit-related convenience fucntions to query attributes
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

Node*
getGrandpa(Limit *limit);

Node*
getMinimumLimit(Limit *limit);

Node*
getMaximumLimit(Limit *limit);

int
getHeight(Limit *limit);

int
getBalanceFactor(Limit *limit);


