/**
 * Contains complimentary data structures needed to run the LOB.
 */
#include <stdlib.h>
#include "hftlob.h"
#include <string.h>


void
pushToQueue(Queue *q, Limit *limit){
    QueueItem *ptr_newItem = malloc(sizeof(QueueItem));
    ptr_newItem->limit = limit;

    if(q->head != NULL){
        q->head->previous = ptr_newItem;
    }
    q->head = ptr_newItem;
    ptr_newItem->previous = q->tail;
    q->tail = ptr_newItem;
    q->size++;
}

Limit*
popFromQueue(Queue *q){
    if(q->tail == NULL){
        return NULL;
    }
    QueueItem *poppedItem = q->tail;
    Limit *poppedLimit = q->tail->limit;
    q->tail = q->tail->previous;
    if(q->tail==NULL){
        q->head = NULL;
    }
    q->size--;
    free(poppedItem);
    return poppedLimit;
}

int
queueIsEmpty(Queue *q){
    if(q->tail == NULL){
        return 1;
    }
    return 0;
}


Book* createBook() {
    // Allocate memory for the book
    Book* new_book = (Book*)malloc(sizeof(Book));
    
    if (new_book == NULL) {
        return NULL; // Memory allocation failed
    }

    // Initialize trees to empty
    new_book->bids = NULL;
    new_book->asks = NULL;

    // Initialize the order map to all NULLs
    // This ensures we don't have "garbage" data in our map
    memset(new_book->order_map, 0, sizeof(new_book->order_map));

    return new_book;
}
