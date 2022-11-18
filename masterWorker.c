// 17/09/2022
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
//#include "auxiliaryMW.h"
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



int numberThreads=4;
int lenghtTail =8;
const char* directoryName;
char path_assoluto [UNIX_PATH_MAX+1];
char buf[PATH_MAX];
char *res;
int tempo=0;

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

void getPathAssoluto(char* directoryName){
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
}

int parser(int argc, char*argv[]){
    int c;
    int i=1;
    char* option = NULL;
    char*ptr;
    opterr=0;
   while((c = getopt(argc, argv, "n:q:d:t:")) != -1){
       switch(c){
           case 'n':
               printf (" è arrivato -n \n");
               ec_null(c,"main");
               numberThreads = atoi(optarg);
               printf("numberThreads: %d\n", numberThreads);
               break;

           case 'q':
               printf (" è arrivato -q \n");
               ec_null(c,"main");
               lenghtTail = atoi(optarg);
               printf("lenghtail: %d\n", lenghtTail);
               break;

           case 'd':
               printf (" è arrivato -d \n");
               ec_null(c,"main");
               directoryName= optarg;
               printf("pathrelativoVero: %s\n",directoryName);
               getPathAssoluto(directoryName);
               printf("pathAsssolutoFUORI : %s\n", buf);
               break;

           case 't':
               printf (" è arrivato -t \n");
               ec_null(c,"main");
               //printf( " argv: %c\n", c);
               tempo= atoi(optarg);
               printf("tempo: %d\n", tempo);
               break;
       }
   }
}

void* startWorker(void* tizio){
    int indiceWorker=(intptr_t) tizio;
}

int main(int argc, char* argv []){
    int pid;
    int status;
    struct tail_of_files* element = malloc(sizeof(struct tail_of_files));
    element->file=NULL;
    element->next = NULL;
    element->prec = NULL;
    pthread_t tid;
    pthread_t maskProducer;
    int sum;
    int tot_files;
    int err;
    int fd_skt, fd_c;
    struct sockaddr_un sa;
    sa.sun_family=AF_UNIX;
    int n=0, i=0;
    DIR *d;
    struct dirent *dir;

    //creo thread che si occupa della maschera segnali
    if(	pthread_create(&maskProducer, NULL, signalMask, NULL)!=0){
        perror(" error in function pthread_create of signalMask");
        exit(EXIT_FAILURE);
    }
    signalMask();

    //creo collector, figlio del masterWorker
    switch( pid = fork() ) {
        case -1: {
            perror("Errore: fork");//errore masterWorker
            break;
        }
        case 0:  { //figlio collector
           // execvp(argv[0], argv);
           perror("Errore: exec");//errore collector
           //printf("sono il figlio con il pid : %d\n\n", getpid());
            sleep(3);
            //printf("sono il figlio sto morendo");
            exit(EXIT_SUCCESS);
        }
        default: { //padre masterWorker
            /*attende il figlio o ritorna*/
            //printf("sono il padre con il pid: %d\n\n",getpid());
            //printf("aspetto mio figlio\n\n");
            if ( waitpid(pid,&status,0) == -1 ) {
                perror("Errore: waitpid");
                exit(EXIT_FAILURE);
            }
            if (WIFEXITED(status)) {
                /* il figlio terminato con exit o return */
                //printf("stato %d\n", WEXITSTATUS(status));
            }
        }
    }//fine creazione collector

    //gestione parser
    parser(argc, argv);

    // apro la directory corrente per contare i miei file
    d = opendir(directoryName);

    // settiamo errno per opendir
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
    
    //conto tutti gli elementi per avere la dimensione per l'array
    while((dir = readdir(d)) != NULL) {
        if ( !strcmp(dir->d_name, ".") || !strcmp(dir->d_name, "..") ){
            continue;
        } else {
            n++;
        }
    }
    rewinddir(d); // dato che abbiamo sceso tutto l'albero, si torna alla radice

    char *filesList[n];

    //metto i file nel mio array
    while((dir = readdir(d)) != NULL) {
        if ( !strcmp(dir->d_name, ".") || !strcmp(dir->d_name, "..") ){
            continue;
        }
        else {
            filesList[i] = (char*) malloc (strlen(dir->d_name)+1);
            strncpy (filesList[i],dir->d_name, strlen(dir->d_name) );
            i++;
        }
    }
    closedir(d);

    //stampo i file
    for(int k=0; k<=n-1; k++)
        printf("%s\n", filesList[k]);

    //creo threadWorkers
    int j=0;
    for(j=0;j<numberThreads;j++)
    {
        if(err = pthread_create(&tid,NULL,startWorker, (void*)(intptr_t)j ))
        {
            perror("errore creazione threadWorkers");
            exit(EXIT_FAILURE);
        }
    }


    //comunicazione con collector
    if (fork()!=0) { /* padre, server */
        fd_skt = socket(AF_UNIX, SOCK_STREAM, 0);
        if((bind(fd_skt, (struct sockaddr *) &sa, sizeof(sa))) == -1 || errno == EINTR){
            perror(" errore nella funzione bind");
            exit(EXIT_FAILURE);
        }
        if((listen(fd_skt,SOMAXCONN)) == -1 || errno == EINTR){
            perror(" errore nella funzione listen");
            exit(EXIT_FAILURE);
        }
    }
    else{ // figlio
        fd_skt=socket(AF_UNIX,SOCK_STREAM,0);
        while (connect(fd_skt,(struct sockaddr*)&sa,sizeof(sa)) == -1 ) {
            if ( errno == ENOENT ){
                sleep(1); /* socket non esiste */
            }
            else {
                exit(EXIT_FAILURE);
            }
        }
    }
    return 0;
}
