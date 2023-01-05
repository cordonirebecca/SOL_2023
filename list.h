//
// Created by rebecca on 02/01/23.
//

#ifndef SOL_LIST_H
#define SOL_LIST_H

#include <pthread.h>

typedef struct {
    char *name;
} file_name;

typedef struct llist
{
    char* opzione;
    struct llist *prec;
    struct llist *next;
    pthread_mutex_t head_lock;
    pthread_mutex_t tail_lock;
} list;


int is_empty_list(struct llist* head);

int is_valid_list(struct llist* head);

void StampaLista(struct llist* head);

void enqueue(struct llist* head, char * opzione);

void listdir(const char *name, int indent,struct llist *l);

#endif //SOL_LIST_H
