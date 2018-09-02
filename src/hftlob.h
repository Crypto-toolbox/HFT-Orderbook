#ifndef HFTLOB_H_
#define HFTLOB_H_

/**
 * Constants
 */
extern const uuid_t BINUUID;

/**
 * HFT Custom Struct Type definitions and functions.
 *
 * This sections includes initializer, constructor and desctructor functions
 * for the defined structs. Other functions using these structs can be found
 * further below, in the relevant API sections of this file.
 *
 *
 * Initializer functions set the struct's values at the given pointer to 0
 * - or equivalent - * on all fields, explicitly. This will override any
 * previously set values.
 *
 * Constructor functions (those starting with "new*"), create a pointer for
 * the relevant struct, allocate memory using malloc(), and return it.
 *
 * Destructor functions (those starting with "delete*"), free any memory
 * allocated by the constructor methods.
 *
 * .. Note::
 *
 *        We only free() resources that were explicitly malloc'd by us. Any
 *        other memory allocation must be free'd by you beforehand, otherwise
 *        this will cause a memory leak.
 *
 * .. Note::
 *
 *      While destructor functions assert that the struct's references to other
 *      structs have been NULL'd, they do NOT check other structs for references
 *      to the struct that is being deleted.
 *
 *      We assume it has been unlinked from the remaining structs properly,
 *      if its struct references are NULL!
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

void
initOrder(Order *order);

Order*
newOrder(void);

void
deleteOrder(Order *ptr_order);


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

void
initLimit(Limit *limit);

Limit*
newLimit(void);

void
deleteLimit(Limit *ptr_limit);

Limit*
newRoot(void);

void
deleteRoot(Limit *ptr_limit);


typedef struct Book{
    Limit *bids;
    Limit *asks;
} Book;

void
initBook(Book *book);

Book*
newBook(void);

void
deleteBook(Book *ptr_book);


/**
 * Public API Functions for the HFT-Orderbook library.
 */

void
addOrderToBook(Book* ptr_book, Order* ptr_order);

int
removeOrderFromBook(Book* ptr_book, Order* ptr_order);





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