//
// Created by rebecca on 16/09/22.
//

#ifndef SOL_LIST_H
#define SOL_LIST_H

#include<stdio.h>
#include<stdlib.h>
#include <stdbool.h>
/*
extern struct node{
    int info;
    struct node *next;
};

typedef struct node NodeList;
typedef NodeList *ListNodes;

extern void printList(NodeList *l);

extern void appendFileHead(NodeList **l, int el);

extern void appendFileTail(NodeList **l, int el);

extern void delete(NodeList**l, int el);



*/

typedef struct file_node{            // è la coda dove inserisco i files
    char* file;
    struct file_node*next;
}file_node;

typedef struct {
    file_node *front;
    file_node *rear;
    pthread_mutex_t mutex;
} Queue_r;

void StampaLista(struct file_node* head);

void appendFile(struct file_node** head, char* file);

bool dequeue(Queue_r* queue, char* value);

void enqueue(Queue_r* queue, char* value);

Queue_r* qcreate();



#endif //SOL_LIST_H
