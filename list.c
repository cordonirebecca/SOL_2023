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
#include "list.h"

void StampaLista(struct tail_of_files* head)
{
    struct tail_of_files *temp = head;
    printf("Lista:");
    while (temp != NULL)
    {
        printf(" %s", temp->file);
        temp = temp->next;
    }
    printf("\n");
}

void insert(struct tail_of_files** head, char* file){
    if ( head == NULL ) {
        //head is invalid, do nothing
        return;
    }
    struct tail_of_files* newEl =calloc(1, sizeof(tail_of_files));
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