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
#include "auxiliaryMW.h"
#include <ctype.h>
#include <limits.h>
#include<dirent.h>
#include "workers.h"
#define UNIX_PATH_MAX 255
#include <arpa/inet.h>

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
int tempo=0;

//comandi del parser
int parser(int argc, char*argv[]){
    int c;
    int i=1;
    char* option = NULL;
    char*ptr;
    opterr=0;
    while((c = getopt(argc, argv, "n:q:d:t:")) != -1){
       switch(c){
           case 'n':
               ec_null(c,"main");
               numberThreads = atoi(optarg);
               printf("numberThreads: %d\n", numberThreads);
               break;

           case 'q':
               ec_null(c,"main");
               lenghtTail = atoi(optarg);
               printf("lenghtail: %d\n", lenghtTail);
               break;

           case 'd':
               ec_null(c,"main");
               directoryName= optarg;
               getPathAssoluto(directoryName);
               break;

           case 't':
               ec_null(c,"main");
               tempo= atoi(optarg);
               break;
       }
   }
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
    int i=0;
    char ** arrayPath ;
    int numFile2=0;
    int numFileLetti=0;
    char * dirName="pluto";
    char * directoryPartenza;
    char * rest;
    char * token;
    char * pathRelativo;

    int socket_desc, client_sock, client_size;
    struct sockaddr_un server_addr, client_addr;
    char server_message[2000], client_message[2000];

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

    //leggo i file da tutte le directory

    directoryPartenza=malloc(sizeof(char)*800);
    directoryPartenza=getcwd(directoryPartenza,800);
    printf("CLIENT: lavoro nella directory:%s\n",directoryPartenza);

    /*Conto quanti file sono effettivamente presenti all' interno della direcotry richiesta*/
   int bitConteggio=0;

    //printf("prima volta--->\n numFile2:%d\ndirname:%s\ni:%d\nbitconteggio:%d\nnumfileletti:%d",numFile2,dirName,i,bitConteggio,numFileLetti);
    int letturaDirectoryReturnValue=leggiNFileDaDirectory(&numFile2,dirName,arrayPath,i,bitConteggio,&numFileLetti);
    printf("letti %d file\n",numFileLetti);

    if(letturaDirectoryReturnValue != 0){
        perror("Errore nella lettura dei file dalla directory\n");
    }

    if(numFile2<=0 || numFile2>numFileLetti){
        numFile2=numFileLetti;

    }
    bitConteggio=1;

    arrayPath = malloc(numFile2 * sizeof(char *));
    for(i=0; i<numFile2; i++){
        arrayPath[i] = malloc(200 * sizeof(char));
        strncpy(arrayPath[i],"",2);
    }

    i=0;
    int salvaNumFile=numFile2;
    /*Dopo essermi memorizzato la directory corrente da cui partivo, utilizzo la funzione chdir per ritornarci.
    Questo risulta essere necessario perchè al procedura leggiNFileDaDirectory mi ha modificato al directory in cui sto lavorando.*/

   int chdirReturnValue=chdir(directoryPartenza);
    if(chdirReturnValue != 0){
        perror("Errore nell' utilizzo di chdir\n");
        return -1;
    }

    letturaDirectoryReturnValue=leggiNFileDaDirectory(&numFile2,dirName,arrayPath,i,bitConteggio,&numFileLetti);

    chdirReturnValue=0;
    chdirReturnValue=chdir(directoryPartenza);
    if(chdirReturnValue != 0){
        /*è stato settato errno*/
        perror("Errore nell' utilizzo di chdir\n");
        return -1;
    }

    if(letturaDirectoryReturnValue != 0){
        perror("Errore nella lettura dei file dalla directory\n");
    }

    for(i = 0; i < salvaNumFile; i++){
        rest = arrayPath[i];
        while ((token = strtok_r(rest, "/", &rest))!=NULL){
            pathRelativo=token;
        }
        printf("Lavoro sul file: %s\n",pathRelativo);
    }


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
        server_addr.sun_family=AF_UNIX;

        // Clean buffers:
        memset(server_message, '\0', sizeof(server_message));
        memset(client_message, '\0', sizeof(client_message));

        // Create socket:

        socket_desc = socket(AF_UNIX, SOCK_STREAM, 0);

        if(socket_desc == -1 && errno == EINTR){
            printf("Error while creating socket\n");
            return -1;
        }
        printf("Socket created successfully\n");

        // Bind to the set port and IP:
        if(bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr))== -1 && errno==EINTR){
            printf("Couldn't bind to the port\n");
            return -1;
        }
        printf("Done with binding\n");

        // Listen for clients:
        if(listen(socket_desc, SOMAXCONN)== -1 && errno == EINTR ){
            printf("Error while listening\n");
            return -1;
        }
        printf("\nListening for incoming connections.....\n");

        // Accept an incoming connection:
        client_size = sizeof(client_addr);
        client_sock = accept(socket_desc, (struct sockaddr*)&client_addr, &client_size);

        if (client_sock < 0){
            printf("Can't accept\n");
            return -1;
        }
        //printf("Client connected at IP: %s and port: %i\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Receive client's message:
        if (recv(client_sock, client_message, sizeof(client_message), 0) < 0){
            printf("Couldn't receive\n");
            return -1;
        }
        printf("Msg from client: %s\n", client_message);

        // Respond to client:
        strcpy(server_message, "This is the server's message.");

        if (send(client_sock, server_message, strlen(server_message), 0) < 0){
            printf("Can't send\n");
            return -1;
        }

        // Closing the socket:
        close(client_sock);
        close(socket_desc);


    return 0;
}
