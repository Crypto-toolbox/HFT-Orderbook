#ifndef HFTLOB_H_
#define HFTLOB_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Forward declarations */
typedef struct Order Order;
typedef struct Limit Limit;
typedef struct QueueItem QueueItem;
typedef struct Queue Queue;
typedef struct Book Book;

/* 1. The individual Order */
struct Order {
    char *tid;
    unsigned buyOrSell;
    double shares;
    double limit;
    double entryTime;
    double eventTime;
    Order *nextOrder;
    Order *prevOrder;
    Limit *parentLimit;
    int exchangeId;
};

/* 2. The Price Level (Limit) */
struct Limit {
    double limitPrice;
    double size;
    double totalVolume;
    int orderCount;
    Limit *parent;
    Limit *leftChild;
    Limit *rightChild;
    Order *headOrder;
    Order *tailOrder;
};

/* 3. The Order Book Container */
struct Book {
    Limit* bids;
    Limit* asks;
    Order* order_map[10000]; 
};

/* 4. Queue structures for BST operations */
struct QueueItem {
    Limit *limit;
    QueueItem *previous;
};

struct Queue {
    int size;
    QueueItem *head;
    QueueItem *tail;
};

/**
 * FUNCTION PROTOTYPES
 */

/* Core Book Logic */
Book* createBook();
void addOrderToBook(Book* book, int id, int side, double price, double shares);

/* Init functions */
void initOrder(Order *order);
void initLimit(Limit *limit);
void initQueueItem(QueueItem *item);
void initQueue(Queue *q);

/* BST & Order Management (Existing) */
void pushToQueue(Queue *q, Limit *limit);
Limit* popFromQueue(Queue *q);
int pushOrder(Limit *limit, Order *new_order);
Order* popOrder(Limit *limit);
int removeOrder(Order *order);
Limit* createRoot(void);
int addNewLimit(Limit *root, Limit *limit);
int removeLimit(Limit *limit);

/* BINARY SEARCH TREE FUNCTIONS */
Limit* createRoot(void);
int addNewLimit(Limit *root, Limit *limit);
void replaceLimitInParent(Limit *limit, Limit *newLimit);
int removeLimit(Limit *limit);

/* CONVENIENCE FUNCTIONS */
int limitExists(Limit *root, Limit *limit);
int limitIsRoot(Limit *limit);
int hasGrandpa(Limit *limit);
Limit* getGrandpa(Limit *limit);
Limit* getMinimumLimit(Limit *limit);
Limit* getMaximumLimit(Limit *limit);
int getHeight(Limit *limit);
int getBalanceFactor(Limit *limit);

/* ROTATION FUNCTIONS (Make sure these are void) */
void rotateLeftLeft(Limit *limit);
void rotateLeftRight(Limit *limit);
void rotateRightRight(Limit *limit);
void rotateRightLeft(Limit *limit);
void balanceBranch(Limit *limit);

void RunAllTests(void);

void printTree(Limit* node);

#endif