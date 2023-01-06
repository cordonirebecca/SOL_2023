//
// Created by rebecca on 02/01/23.
//
#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <pthread.h>

static inline void freeNode(llist *node)           { free((void*)node); }

Queue_t *initQueue() {
    Queue_t *q = malloc(sizeof(Queue_t));
    if (!q) return NULL;
    q->head = malloc(sizeof(llist));
    if (!q->head) return NULL;
    q->head->opzione = NULL;
    q->head->next = NULL;
    q->tail = q->head;
    q->qlen = 0;
    if (pthread_mutex_init(&q->qlock, NULL) != 0) {
        perror("mutex init");
        return NULL;
    }
    if (pthread_cond_init(&q->qcond, NULL) != 0) {
        perror("mutex cond");
        if (&q->qlock) pthread_mutex_destroy(&q->qlock);
        return NULL;
    }
    return q;
}

void deleteQueue(Queue_t *q) {
    while(q->head != q->tail) {
        llist *p = (llist*)q->head;
        q->head = q->head->next;
        freeNode(p);
    }
    if (q->head) freeNode((void*)q->head);
    if (&q->qlock)  pthread_mutex_destroy(&q->qlock);
    if (&q->qcond)  pthread_cond_destroy(&q->qcond);
    free(q);
}

int is_empty_list(struct llist* head){
    return head->opzione == NULL;
}

int is_valid_list(struct llist* head){
    return head != NULL;
}

void StampaLista(Queue_t *q){
    llist *temp = q->head;
    printf("Lista:");
    while (temp != NULL){
        printf(" %s -> ", temp->opzione);
        temp = temp->next;
    }
    printf("\n");

}

int push(Queue_t *q, char *data) {
    if ((q == NULL) || (data == NULL)) { errno= EINVAL; return -1;}
    llist *n = malloc(sizeof(llist));
    if (!n)
        return -1;
    n->opzione = data;
    n->next = NULL;
    //lock queue
    if (pthread_mutex_lock(&q->qlock)!=0){
        fprintf(stderr, "ERRORE FATALE lock\n");
        pthread_exit((void*)EXIT_FAILURE);
    }
    q->tail->next = n;
    q->tail       = n;
    q->qlen+= 1;
    //sblocco queue
    if (pthread_mutex_unlock(&q->qlock)!=0){
        fprintf(stderr, "ERRORE FATALE unlock\n");
        pthread_exit((void*)EXIT_FAILURE);
    }
    //mando segnale che c'è un valore in lista
    if (pthread_cond_signal(&q->qcond)!=0){
        fprintf(stderr, "ERRORE FATALE signal\n");
        pthread_exit((void*)EXIT_FAILURE);
    }
    return 0;
}

//funzione che apre tutte le cartelle e mi stampa i file in ognuna
/*void listdir(const char *name, int indent,struct llist *l){
    DIR *dir;
    struct dirent *entry;
    if (!(dir = opendir(name)))
        return;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            char path[1024];
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            //path è il percorso directory senza i file
            snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
            // printf("%*s[%s]\n", indent, "", entry->d_name);
            listdir(path, indent + 2,l);
        } else {
            // sono uno o più file nella directory
            //  printf("%*s- %s\n", indent, "", entry->d_name);
            enqueue(l,entry->d_name);
        }
    }
    closedir(dir);
}*/

//mi elimina il primo elemento della lista e me lo ritorna, così il worker se lo prende
void *dequeue(Queue_t *q) {
    if (q == NULL) {
        errno= EINVAL;
    }
    //lock queue
    if (pthread_mutex_lock(&q->qlock)!=0){
        fprintf(stderr, "ERRORE FATALE lock\n");
        pthread_exit((void*)EXIT_FAILURE);
    }
    while(q->head == q->tail) {
        //unlock queue and wait
        if (pthread_cond_wait(&q->qcond, &q->qlock) != 0) {
            fprintf(stderr, "ERRORE FATALE wait\n");
            pthread_exit((void *) EXIT_FAILURE);
        }
    }
    assert(q->head->next);
    llist *n  = (llist *)q->head;
    void *data = (q->head->next)->opzione;
    q->head    = q->head->next;
    q->qlen   -= 1;
    assert(q->qlen>=0);
    //unlock queue
    if (pthread_mutex_unlock(&q->qlock)!=0){
        fprintf(stderr, "ERRORE FATALE unlock\n");
        pthread_exit((void*)EXIT_FAILURE);
    }
    freeNode(n);
    return data;
}

unsigned long length(Queue_t *q) {
    //lockqueue
    if (pthread_mutex_lock(&q->qlock)!=0){
        fprintf(stderr, "ERRORE FATALE lock\n");
        pthread_exit((void*)EXIT_FAILURE);
    }
    unsigned long len = q->qlen;
   // UnlockQueue
    if (pthread_mutex_unlock(&q->qlock)!=0){
        fprintf(stderr, "ERRORE FATALE unlock\n");
        pthread_exit((void*)EXIT_FAILURE);
    }
    return len;
}