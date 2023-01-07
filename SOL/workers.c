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
#define UNIX_PATH_MAX 255

#define NUM_ITEMS 1000

int array[NUM_ITEMS];

//funzione che conta il numero di numeri dentro al file
int lenght_file(char* nome_del_file){
    FILE* file;
    int i, count = 0;
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
int sommatoria(char* nome_del_file){
    FILE *myFile;
    int sum=0;
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




