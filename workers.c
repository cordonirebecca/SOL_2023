//
// Created by rebecca on 23/12/22.
//
#include "workers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/stat.h>
#include "auxiliaryMW.h"
#include <ctype.h>
#include <limits.h>
#include<dirent.h>
#include <assert.h>
#include <stdbool.h> // for bool
#include <ctype.h> // for isspace
#define UNIX_PATH_MAX 255

#define NUM_ITEMS 1000

int array[NUM_ITEMS];

static bool readLine (const char* data, const char** beginPtr, const char** endPtr) {
    static const char* nextStart;
    if (data) {
        nextStart = data;
        return true;
    }
    if (*nextStart == '\0') return false;
    *beginPtr = nextStart;

    // Find next delimiter.
    do {
        nextStart++;
    } while (*nextStart != '\0' && *nextStart != '\n');

    // Trim whitespace.
    *endPtr = nextStart - 1;
    while (isspace(**beginPtr) && *beginPtr < *endPtr)
        (*beginPtr)++;
    while (isspace(**endPtr) && *endPtr >= *beginPtr)
        (*endPtr)--;
    (*endPtr)++;

    return true;
}

//funzione che conta il numero di numeri dentro al file
int lenght_file(char *nome_del_file){
    FILE* file;
    int i, count = 0;

    //printf("NOME DEL FILE: %s\n\n\n",nome_del_file);
    file = fopen(nome_del_file, "r");

    if (!file) {
        printf("Problem with opening file\n");
        return 0; // or some error code
    }

    while(count < NUM_ITEMS && fscanf(file, "%d", &array[count]) == 1){
        count++;
    }
    fclose(file);
    //stampo contenuto del file
 /*   for(i=0; i<count; i++){
        printf("a[%d] = %d\n", i, array[i]);
    }*/
    return count;
}


//mi prende un file e ne fa la sommatoria degli elementi al suo interno
int sommatoria(char *nome_del_file){
    FILE *myFile;
    int sum=0;
    printf("nome del file nella sommatoria:%s\n\n",nome_del_file);
    char *str= nome_del_file;
    int init_size = strlen(str);
   /* char delim[] = "";

    char *ptr = strtok(str, delim);

    while (ptr != NULL)
    {
        printf("ptr: %s\n", ptr);
        ptr = strtok(NULL, delim);
    }*/
    myFile = fopen(nome_del_file, "r");
    int lenght_of_files= lenght_file(nome_del_file);
    //read file into array
    int numberArray[lenght_of_files];
    int i;
    if (myFile == NULL){
        printf("Error Reading File\n");
        exit (0);
    }
    for (i = 0; i < lenght_of_files; i++){
        fscanf(myFile, "%d,", &numberArray[i] );
    }
    for (i = 0; i < lenght_of_files; i++){
            sum=sum+(numberArray[i]*i);
    }

    fclose(myFile);
    return sum;
}




