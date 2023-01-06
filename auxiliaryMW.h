//
// Created by rebecca on 07/11/22.
//

#ifndef SOL_AUXILIARYMW_H
#define SOL_AUXILIARYMW_H
#include "list.h"

// tipo di dato usato per passare gli argomenti al thread
typedef struct threadArgs {
    int      thid;
    Queue_t *q;
    int      start;
    int      stop;
} threadArgs_t;


extern void signalMask();

char* getPathAssoluto(char* directoryName);

#endif //SOL_AUXILIARYMW_H
