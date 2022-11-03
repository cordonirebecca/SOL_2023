// 17/09/2022
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
#define UNIX_PATH_MAX 108

#define ec_meno1(s,m) \
    if((s)==-1) {perror(m); exit(EXIT_FAILURE); \
    }

#define ec_null(s,m) \
    if((s)==NULL) {perror(m); exit(EXIT_FAILURE); \
    }

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


int parser(int argc, char*argv[]){
    int c;
    int i=1;
    char* option = NULL;
    int numberThreads = 0;
    int lenghtTail = 0;
    char* directoryName;
    int time;
   while((c = getopt(argc, argv, "n:q:d:t:")) != -1){
       switch(c){
           case 'n':
               printf (" è arrivato -n \n");
               ec_null(c,"main");
               printf( " argv : %c\n",c);
               numberThreads = atoi(optarg);
               printf("numberThreads: %d\n", numberThreads);
               break;

           case 'q':
               printf (" è arrivato -q \n");
               ec_null(c,"main");
               printf( " argv: %c\n", c);
               lenghtTail = atoi(optarg);
               printf("lenghtail: %d\n", lenghtTail);
               break;

           case 'd':
               printf (" è arrivato -d \n");
               ec_null(c,"main");
               printf( " argv: %c\n", c);
               directoryName= optarg;
               printf("directoryName: %s\n", directoryName);
               break;

           case 't':
               printf (" è arrivato -t \n");
               ec_null(c,"main");
               printf( " argv: %c\n", c);
               time= atoi(optarg);
               printf("time: %d\n", time);
               break;
       }
   }
}

void* startWorker(void* idWorker){
    //fare tutto
}

int main(int argc, char* argv []){
    int pid;
    int status;
    NodeList *l = NULL;//inizializzo lista
    l=malloc(sizeof(NodeList));
    pthread_t tid;
    pthread_t maskProducer;
    int sum;
    int err;
    int fd_skt, fd_c;
    struct sockaddr_un sa;
    sa.sun_family=AF_UNIX;

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
           //perror("Errore: exec");//errore collector
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
            //printf("ho aspettato il figlio sto morendo anch'io\n\n");
        }
    }//fine creazione collector

    //gestione parser
    parser(argc, argv);

    //inserisco in modo concorrente i task in coda
    //vorrei inserire i task con la inserTail ma in mutua eslusione come faccio


    //creo threadWorkers
    /*if(err = pthread_create(&tid,NULL,startWorker,NULL )){
        perror("errore creazione threadWorkers");
        exit(EXIT_FAILURE);
    }*/

    //comunicazione con collector
    /*if (fork()!=0) { /* padre, server */
      /*  fd_skt = socket(AF_UNIX, SOCK_STREAM, 0);
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
            /*}
            else {
                exit(EXIT_FAILURE);
            }
        }
    }
*/
        return 0;
}
