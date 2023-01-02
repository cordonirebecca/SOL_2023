//
// Created by rebecca on 02/01/23.
//
#include "list.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void add_bottom_listEl (struct ListEl *head, char *name)
{
    struct ListEl *newEl;
    while(head->next!=NULL)
    {
        head=head->next;
    }
    newEl =malloc(sizeof(struct ListEl));
    strcpy(newEl->name, name);
    newEl->next = NULL;
}
void print_listEl(struct ListEl* head){
    struct ListEl* current = head;
    while (current!=NULL)
    {
        int i=1;
        printf("\nlist: %s->\n",current->name);
        ++i;
        current = current -> next;
    }
}
