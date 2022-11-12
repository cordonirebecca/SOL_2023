//
// Created by rebecca on 07/11/22.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <errno.h>
#include "list.h"
#include <limits.h>
#define UNIX_PATH_MAX 255

int numberThreads = 4;
int lenghtTail = 8;
const char* directoryName;
char path_assoluto [UNIX_PATH_MAX+1];
int tempo=0;

char* getPathAssoluto(const char *path_relativo){
    char *path;
    printf("path relativo nella funzione:%s\n",path_relativo);
    path = realpath(path_relativo, path_assoluto);

    if((path== NULL) && (errno==EACCES)){
        perror("Permesso di lettura o ricerca negato per un componente del prefisso del percorso.\n");
        return NULL;
    }

    else if((path== NULL) && (errno==EINVAL)){
        perror("il percorso è NULL\n");
        return NULL;
    }

    else if((path== NULL) && (errno==EIO)){
        perror("Si è verificato un errore di I/O durante la lettura dal filesystem\n");
        return NULL;
    }

    else if((path== NULL) && (errno==ELOOP)){
        perror("sono stati rilevati troppi collegamenti simbolici durante la traduzione del percorso.\n");
        return NULL;
    }

    else if((path== NULL) && (errno==ENAMETOOLONG)){
        perror("Un componente di un percorso ha superato i caratteri NAME_MAX o un intero percorso ha superato i caratteri PATH_MAX. \n");
        return NULL;
    }

    else if((path== NULL) && (errno==ENOENT))
    {
        perror("Il file denominato non esiste.\n");
        return NULL;
    }

    else if((path== NULL) && (errno==ENOMEM)){
        perror("Memoria esaurita.\n");
        return NULL;
    }

    else if((path== NULL) && (errno==ENOTDIR)){
        perror("Un componente del prefisso del percorso non è una directory.\n");
        return NULL;
    }
    return path_assoluto;
}
