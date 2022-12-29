//
// Created by rebecca on 16/09/22.
//
#include <stdio.h>
#include <stdlib.h>
#include"list.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include "list.h"

Queue_r* qcreate(){
    // crea la coda
    Queue_r *queue = malloc(sizeof(Queue_r));

    // inizializza la coda
    queue->front = NULL;
    queue->rear  = NULL;
    pthread_mutex_init(&queue->mutex, NULL);
    return queue;
}

// enqueue() - aggiunge un elemento alla coda
void enqueue(Queue_r* queue, char* value){
    // crea un nuovo nodo
    file_node *temp = malloc(sizeof (struct file_node));
    temp->file = value;
    temp->next  = NULL;
    // blocco l'accesso
    pthread_mutex_lock(&queue->mutex);
    // test se la coda è vuota
    if (queue->front == NULL) {
        // con la coda vuota front e rear coincidono
        queue->front = temp;
        queue->rear  = temp;
    }
    else {
        // aggiungo un elemento
        file_node *old_rear = queue->rear;
        old_rear->next = temp;
        queue->rear    = temp;
    }
    // sblocco l'accesso ed esco
    pthread_mutex_unlock(&queue->mutex);
}

// dequeue() - toglie un elemento dalla coda
bool dequeue(Queue_r* queue, char* value){
    // blocco l'accesso
    pthread_mutex_lock(&queue->mutex);
    // test se la coda è vuota
    file_node *front = queue->front;
    if (front == NULL) {
        // sblocco l'accesso ed esco
        pthread_mutex_unlock(&queue->mutex);
        return false;
    }
    // leggo il valore ed elimino l'elemento dalla coda
    *value = front->file;
    queue->front = front->next;
    free(front);
    // sblocco l'accesso ed esco
    pthread_mutex_unlock(&queue->mutex);
    return true;
}


void StampaLista(struct file_node* head){
    struct file_node *temp = head;
    while (temp != NULL){
        printf(" %s", temp->file);
        temp = temp->next;
    }
    printf("\n");
}

void appendFile(struct file_node** head, char* file){
    if ( head == NULL ) {
        //head is invalid, do nothing
        return;
    }
    struct file_node* newEl =calloc(1, sizeof(file_node));
    strncpy(newEl->file, file, 3982);
    //No need to do this now...calloc will initialise it to NULL
    //newEl->next = NULL;

    if ( *head == NULL ) {
        //No nodes in list yet, this is the first
        *head = newEl;
    } else if ( *head != NULL ) {
        //Find the end of the list
        while((*head)->next!=NULL) {
            *head = (*head)->next;
        }
    }
    //Add the new node to the list
    *head = newEl;
}