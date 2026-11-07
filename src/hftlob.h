#ifndef HFTLOB_H_
#define HFTLOB_H_

#include <stddef.h>
#include <stdint.h>

/* Order ids, as assigned by the exchange or the caller. */
typedef uint64_t OrderId;

/* Values for Order.buyOrSell */
#define HFTLOB_SELL 0u
#define HFTLOB_BUY  1u
/**
 * CUSTOM STRUCTS
 */

typedef struct Order{
    OrderId id;
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
    int height;             /* cached AVL height; a leaf is 0 */
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

typedef struct MapEntry{
    void *key;
    size_t keyLen;
    uint64_t hash;
    void *value;
    struct MapEntry *next;
} MapEntry;

typedef struct HashMap{
    MapEntry **buckets;
    size_t capacity;
    size_t size;
} HashMap;

/**
 * Called once per fill by executeInBook(), before the resting order is
 * updated: resting->shares is still its size before this fill. If shares
 * equals resting->shares, the resting order is removed and freed once the
 * callback returns. The callback must not modify the book.
 */
typedef void (*FillCallback)(const Order *resting, double shares,
                             double price, void *ctx);

/**
 * A limit order book: one AVL tree of Limits per side, plus hash maps
 * for O(1) lookups of orders by id and limits by price.
 *
 * The book owns every Order and Limit in it; use the *Book functions
 * below instead of manipulating the trees directly.
 */
typedef struct Book{
    Limit *buyTree;         /* root sentinels; the tree hangs off rightChild */
    Limit *sellTree;
    Limit *highestBuy;      /* best bid, or NULL */
    Limit *lowestSell;      /* best ask, or NULL */
    HashMap orders;         /* id -> Order* */
    HashMap buyLimits;      /* price -> Limit* */
    HashMap sellLimits;     /* price -> Limit* */
} Book;

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
 * HASH MAP FUNCTIONS
 */

int
initMap(HashMap *map, size_t capacity);

void
freeMap(HashMap *map);

void*
mapGet(const HashMap *map, const void *key, size_t keyLen);

int
mapPut(HashMap *map, const void *key, size_t keyLen, void *value);

void*
mapRemove(HashMap *map, const void *key, size_t keyLen);

/**
 * BOOK FUNCTIONS
 */

Book*
createBook(void);

void
freeBook(Book *book);

Order*
addOrderToBook(Book *book, OrderId id, unsigned buyOrSell,
               double shares, double price);

int
removeOrderFromBook(Book *book, OrderId id);

int
replaceOrderInBook(Book *book, OrderId id, double shares, double price);

double
executeInBook(Book *book, unsigned aggressorSide, double shares,
              double limitPrice, FillCallback onFill, void *ctx);

Order*
getOrderFromBook(const Book *book, OrderId id);

Limit*
getLimitFromBook(const Book *book, unsigned buyOrSell, double price);

Limit*
getBestBid(const Book *book);

Limit*
getBestAsk(const Book *book);

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

Limit*
detachLimit(Limit *limit);

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

void
rebalanceUpwards(Limit *limit);


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
updateHeight(Limit *limit);

void
updateHeightsUpwards(Limit *limit);

void
copyLimit(Limit *ptr_src, Limit *ptr_tar);

/**
 * CuTest Functions
 * */

int RunAllTests(void);

#endif