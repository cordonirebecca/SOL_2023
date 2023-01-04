//
// Created by rebecca on 02/01/23.
//
#include "list.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


void print_listEl(struct ListEl* head){
    struct ListEl* current = head;
    while (current!=NULL){
        printf("\nlist: %s->\n",current->name);
        current = current -> next;
    }
}