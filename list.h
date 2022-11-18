//
// Created by rebecca on 16/09/22.
//

#ifndef SOL_LIST_H
#define SOL_LIST_H

#include<stdio.h>
#include<stdlib.h>
/*
extern struct node{
    int info;
    struct node *next;
};

typedef struct node NodeList;
typedef NodeList *ListNodes;

extern void printList(NodeList *l);

extern void insertHead(NodeList **l, int el);

extern void insertTail(NodeList **l, int el);

extern void delete(NodeList**l, int el);



*/

extern struct tail_of_files{            // è la coda dove inserisco i files
    char* file;
    struct tail_of_files*prec;
    struct tail_of_files*next;
}tail_of_files;

void StampaLista(struct tail_of_files* head);
void insert(struct tail_of_files** head, char* file);

#endif //SOL_LIST_H
