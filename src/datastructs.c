/**
 * Contains complimentary data structures needed to run the LOB.
 */
#include <stdlib.h>
#include "hftlob.h"

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
