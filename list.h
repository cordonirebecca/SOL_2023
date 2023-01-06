//
// Created by rebecca on 02/01/23.
//

#ifndef SOL_LIST_H
#define SOL_LIST_H

#include <pthread.h>

typedef struct llist{
    char* opzione;
    struct llist *next;
} llist;

typedef struct Queue {
    llist       *head;    // elemento di testa
    llist       *tail;    // elemento di coda
    unsigned long  qlen;    // lunghezza
    pthread_mutex_t qlock;
    pthread_cond_t  qcond;
} Queue_t;

Queue_t *initQueue();

void deleteQueue(Queue_t *q);

int is_empty_list(struct llist* head);

int is_valid_list(struct llist* head);

void StampaLista(Queue_t *q);

int push(Queue_t *q, char *data);

void listdir(const char *name, int indent,struct llist *l);

void *dequeue(Queue_t *q);

unsigned long length(Queue_t *q);

#endif //SOL_LIST_H
