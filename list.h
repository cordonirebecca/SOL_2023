//
// Created by rebecca on 02/01/23.
//

#ifndef SOL_LIST_H
#define SOL_LIST_H

typedef struct ListEl{
    char name[1000];
    struct ListEl* next;
}listEl;

void add_bottom_listEl (struct ListEl *head, char *name);

void print_listEl(struct ListEl* head);

#endif //SOL_LIST_H
