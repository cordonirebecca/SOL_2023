//
// Created by rebecca on 02/01/23.
//

#ifndef SOL_LIST_H
#define SOL_LIST_H

#include <pthread.h>

typedef struct ListEl{
    const char *name;
    struct ListEl* next;
}listEl;




void add_bottom_listEl (char ch);

void print_listEl(struct ListEl* head);

#endif //SOL_LIST_H
