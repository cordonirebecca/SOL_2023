//
// Created by rebecca on 16/09/22.
//
#include <stdio.h>
#include <stdlib.h>
#include"list.h"

void printList(NodeList *l){
    while(l != NULL){
        printf("%d -> ", l->info);
        l=l->next;
    }
    printf("NULL");
}

void insertHead(NodeList **l, int el){
    NodeList *newList=malloc(sizeof(NodeList));
    newList->info= el;
    newList->next=*l;
    *l=newList;
}

void insertTail(NodeList **l, int el){
    if(*l==NULL){
        *l= malloc(sizeof(NodeList));
        (*l)->info=el;
        (*l)->next=NULL;
    }
    else{
        insertTail(&(*l)->next,el);
    }
}


//cancello primo elemento uguale ad el.
void delete(NodeList**l, int el){
    if(*l!=NULL){
        if((*l)->info == el){
            NodeList *aus=*l;
            *l=(*l)->next;
            free(aus);
        }
        else{
            delete(&(*l)->next, el);
        }
    }
}

