//
// Created by rebecca on 02/01/23.
//
#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <dirent.h>
#include <string.h>

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
    printf("Lista della codona :");
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


void delete_head_lista_piena(struct llist** head,char* data){
    if(*head != NULL){
        if((*head)->opzione == data){
            //cancello primo elemento uguale a data
            llist * aus=*head;
            *head=(*head)->next;
            free(aus);
        }
        else{
            delete_head_lista_piena(&(*head)->next,data);
        }
    }
}

char* file_singolo_da_inserire(struct llist* head){
    char* File_singolo;
    if(head == NULL){
        printf("lista vuota\n");
        return NULL;
    }
    return File_singolo=head->opzione;
}

//stampa la lista che passa al masterworker, che poi spezza  e inserisce nella coda grande
void print_list(struct llist* head){
    struct llist *temp = head;
    if (!is_valid_list(head)){
        // tirare errore se head = null
        printf("Errore Stampa Lista");
        exit(1);
    }

    if(is_empty_list(head)){
        printf("Lista vuota\n");
    }
    else{
        printf("Lista da inserire: ");
        while (temp != NULL){
            printf(" %s -> ", temp->opzione);
            temp = temp->next;
        }
        printf("\n");
    }
}

void add_list(struct llist* head, char * opzione){
    struct llist *new= malloc(sizeof(struct llist));
    struct llist *nodoCorrente= malloc(sizeof(struct llist));
    new->opzione=malloc(80* sizeof(char));
    strcpy(new->opzione,opzione);
    new->next=NULL;
    if((head->opzione)==NULL) {
        (head->opzione) = (new->opzione);
    }else{
        nodoCorrente=head;
        while(nodoCorrente->next != NULL){
            nodoCorrente=nodoCorrente->next;
        }
    }
    nodoCorrente->next=new;
}

//funzione che apre tutte le cartelle e mi stampa i file in ognuna
void listdir(const char *name, int indent,struct llist *l){
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
            add_list(l,entry->d_name);
        }
    }
    closedir(dir);
}


//mi elimina il primo elemento della lista e me lo ritorna, così il worker se lo prende
char *dequeue(Queue_t *q) {
    if (q == NULL) {
        errno= EINVAL;
    }
    //lock queue
    if (pthread_mutex_lock(&q->qlock)!=0){
        fprintf(stderr, "ERRORE FATALE lock\n");
        pthread_exit((void*)EXIT_FAILURE);
    }
    //finchè è vuota
    while(q->head == q->tail) {
        //unlock queue and wait
        if (pthread_cond_wait(&q->qcond, &q->qlock) != 0) {
            fprintf(stderr, "ERRORE FATALE wait\n");
            pthread_exit((void *) EXIT_FAILURE);
        }
    }
    //qui è tutto bloccato

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
    //libero lo spazio
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