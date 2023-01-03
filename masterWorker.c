// 17/09/2022
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>
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
#include <stdbool.h>
#include "workers.h"
#define UNIX_PATH_MAX 255
#include <arpa/inet.h>
#include<semaphore.h>
#define SHARED 1
#include "list.h"
#include <dirent.h>


#define MAXPATHLEN 100


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
pthread_t maskProducer;
pthread_t thread_workers;
pthread_t Creatore_thread_workers;

sem_t empty, full,sm;
int data;

int socket_desc, client_sock, client_size;
struct sockaddr_un server_addr, client_addr;
char server_message[2000], client_message[2000];
int i=0;

//mutex e cond variable
pthread_mutex_t mtx=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond=PTHREAD_COND_INITIALIZER;

extern struct listEl *queue;

void *job_of_masterWorker();
void *job_of_worker();


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

struct file_name* estrai(){

}

int main(int argc, char* argv []){
    int pid;
    int status;
    pthread_t tid;
    pthread_t ptid;

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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //inizio a creare i thread
    sem_init(&empty, SHARED, 1);
    sem_init(&full, SHARED, 0);
    sem_init(&sm,SHARED,1);

    //creo producer
    pthread_create(&ptid,NULL,job_of_masterWorker,NULL);


    //creo i vari workers
    for(i=0;i<numberThreads; i++){
        if((pthread_create(&thread_workers, NULL, job_of_worker, (void*)(intptr_t)i))!=0){
            perror("SERVER-> Errore nella funzione pthread_create\n");
            exit(EXIT_FAILURE);
        }
    }

    //eseguo le join
    pthread_join(ptid,NULL);
    for(int j=0;j<numberThreads;j++){
        pthread_join(thread_workers,NULL);

    }

    printf("fine main\n");

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

void listdir(const char *name, int indent)
{
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(name)))
        return;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            char path[1024];
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
            printf("%*s[%s]\n", indent, "", entry->d_name);
            listdir(path, indent + 2);
        } else {
            printf("%*s- %s\n", indent, "", entry->d_name);
        }
    }
    closedir(dir);
}


void *job_of_masterWorker(void* arg){
    int produced;
    printf("Producer created\n");
    listdir("pluto",1);

    pthread_mutex_lock(&mtx);
    pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mtx);


      return(void*)0;
}


void *job_of_worker(void *arg){
    int consumed, total=0;
    struct file_node* p;
    int *thread = (int*)arg;

    printf("worker: %d\n",thread);
   // printf("head: %s\n\n\n",head->name);
    /*   while(true){
           pthread_mutex_lock(&mtx);
           while(head == NULL){
               pthread_cond_wait(&cond, &mtx);
               printf("svegliati amico thread\n");
               fflush(stdout);
           }/*
           //il consumatore prende quel nodo
           p=estrai();
           // elimina quel nodo
           //delete();*/
   //        pthread_mutex_unlock(&mtx);
           // elaborazione di p il file estratto, la somma dei numeri binari
           return(void*)0;
     //  }
}