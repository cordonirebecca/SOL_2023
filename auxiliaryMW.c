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
#include <dirent.h>
#include <pthread.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/stat.h>
#include "list.h"
#include "auxiliaryMW.h"
#include <ctype.h>
#include <limits.h>
#include<dirent.h>

#define UNIX_PATH_MAX 255

#define ec_meno1(s,m) \
    if((s)==-1) {perror(m); exit(EXIT_FAILURE); \
    }

#define ec_null(s,m) \
    if((s)==NULL) {perror(m); exit(EXIT_FAILURE); \
    }

char buf[PATH_MAX+1];
char *res;

void signalMask(){
    struct sigaction sa;
    sigset_t set;
    //maschero tutti i segnali
    ec_meno1(sigfillset(&set),"main");
    memset(&sa,0,sizeof(sa));
    sa.sa_handler = SIG_IGN;

    ec_meno1(sigaction(SIGHUP,&sa,NULL),"main");
    ec_meno1(sigaction(SIGINT,&sa,NULL),"main");
    ec_meno1(sigaction(SIGQUIT,&sa,NULL),"main");
    ec_meno1(sigaction(SIGTERM,&sa,NULL),"main");
    ec_meno1(sigaction(SIGUSR1,&sa,NULL),"main");
    ec_meno1(sigaction(SIGPIPE,&sa,NULL),"main");

    ec_meno1(sigemptyset(&set),"main");

    ec_meno1(sigaddset(&set, SIGHUP),"main");
    ec_meno1(sigaddset(&set, SIGINT),"main");
    ec_meno1(sigaddset(&set, SIGQUIT),"main");
    ec_meno1(sigaddset(&set, SIGTERM),"main");
    ec_meno1(sigaddset(&set, SIGUSR1),"main");
    ec_meno1(sigaddset(&set, SIGPIPE),"main");

    ec_meno1(pthread_sigmask(SIG_SETMASK,&set,NULL),"main");

    int sigwaitReturnValue;
    int indiceSegnale=0;
    sigwaitReturnValue=sigwait(&set, &indiceSegnale);
    printf("arrivato segnale:%d\n",indiceSegnale);
}


char* getPathAssoluto(char* directoryName){
    char*res=realpath(directoryName, buf); // buf ha il path assoluto
    if(res == NULL ){
        if(errno == EACCES){
            perror("Read or search permission was denied for a component of the path prefix.");
        }else if(errno == EINVAL){
            perror("path is NULL.");
        }else if(errno == EIO){
            perror("An I/O error occurred while reading from the filesystem.");
        }else if(errno == ELOOP){
            perror("Too many symbolic links were encountered in translating the pathname.");
        }else if(errno == ENAMETOOLONG){
            perror("A component of a pathname exceeded NAME_MAX characters, or an entire pathname exceeded PATH_MAX characters.");
        }else if(errno == ENOENT){
            perror("The named file does not exist.");
        }else if(errno == ENOMEM){
            perror("Out of memory.");
        }else if(errno == ENOTDIR){
            perror(" A component of the path prefix is not a directory.");
        }
    }
    return buf;
}

/*Funzione che legge N file da una directory*/
/*nel caso in cui siano presenti sottoDirectory, le visita ricorsivamente fino al raggiungimento di N*/
int leggiNFileDaDirectory(int *numFile2,const char *dirName, char** arrayPath, int posizioneArray, short bitConteggio, int *numeroFileLetti){
    int chDirReturnValue=0;
    int leggiTuttiIFile=0;
    char *filename;
    int closeDirReturnValue=0;
    struct stat s;
    int chiamataRicorsivaReturnValue=0;
    int isDirectory = 0;
    int isFileRegolare = 0;
    int chdirReturnValue=0;
    int lunghezza=0;
    char *path =NULL;
    struct dirent *dir = NULL;
    DIR *d;

    if(numFile2 == NULL){
        perror("ERRORE è stato passato alla funzione un valore non valido");
        return -1;
    }

    leggiTuttiIFile = *numFile2 <= 0;

    printf("dirName in funzione: %s\n",dirName);
    d = opendir(dirName);

    //setto errno per opendir
    if(errno == EACCES){
        perror("Permission denied.");
    }
    else if(errno == EBADF){
        perror("fd is not a valid file descriptor opened for reading.");
    }
    else if(errno == EMFILE){
        perror("The per-process limit on the number of open file descriptors has been reached.");
    }
    else if(errno == ENFILE){
        perror("The system-wide limit on the total number of open files has been reached.");
    }
    else if(errno == ENOENT){
        perror("Directory does not exist, or name is an empty string.");
    }
    else if(errno == ENOMEM){
        perror("Insufficient memory to complete the operation.");
    }
    else if(errno == ENOTDIR){
        perror("name is not a directory.");
    }

    if(d==NULL){    // se non apro directory
        perror("Could not open current directory\n");
        return -1;
    }

    /* Eseguo operazione cd nella directory selezionata*/
    if((chdir(dirName))==-1 && errno!=0){
        perror("error in function chdir\n");
        return -1;
    }

    /*Leggo ogni entry presente nella directory fino a che non ho letto tutti i file oppure ho raggiunto il limite*/
    while ((leggiTuttiIFile || *numFile2) && (dir = readdir(d)) != NULL){
        filename = dir->d_name;
        stat(filename, &s);

        /*Tramite questi tre if, se ho selezionato un file*/
        /*speciale lo salto, non considerandolo nel conteggio*/
        if((strcmp(filename, ".") == 0) || (strcmp(filename, "..") == 0)){
            continue;
        }
        else if (!(S_ISDIR(s.st_mode)) && !(S_ISREG(s.st_mode))){
            /*se entro dentro questo if significato che*/
            /*grazie all' utilizzo della struttura stat,*/
            /*sono riuscito ad identificare che il file considerato in questo momento non*/
            /*risulta un file regolare e nemmeno una directory*/
            continue;
        }
        else if (S_ISDIR(s.st_mode)){
            printf("Ho preso una directory\n");
            chiamataRicorsivaReturnValue = leggiNFileDaDirectory( numFile2,filename, arrayPath,posizioneArray,bitConteggio, numeroFileLetti);
            if (chiamataRicorsivaReturnValue == -1){
                if((closedir(d))==-1 && errno!=0){{
                        perror("ERRORE nella funzione closedir\n");
                        return -1;
                    }
                }
                printf("chiusa directory\n\n\n\n\n");
            }
            else{
                if((chdir(".."))==-1 && errno!=0){{
                        perror("ERRORE nella funzione closedir\n");
                    }
                }
            }
        }
        else if (S_ISREG(s.st_mode)){
            if(bitConteggio == 0){
                (*numeroFileLetti)++;
            }
            else{
                lunghezza=strlen(getPathAssoluto(filename))+1;
                path=malloc(sizeof(char)*lunghezza);
                path=getPathAssoluto(filename);
                printf("Adesso lavoro nel path%s\n",path);
                if(path==NULL){
                    errno=EINVAL;
                    return -1;
                }
                if(strcmp(arrayPath[posizioneArray],"")!=0){
                    posizioneArray++;
                }
                strcpy(arrayPath[posizioneArray],path);
            }
            if (!leggiTuttiIFile){
                (*numFile2)--;
            }
        }
    }
    closedir(d);
    return 0;
}

