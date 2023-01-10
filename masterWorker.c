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
#include <ctype.h>
#include <semaphore.h>
#define MAXPATHLEN 100
#define ec_meno1(s,m) \
    if((s)==-1) {perror(m); exit(EXIT_FAILURE); \
    }
#define ec_null(s,m) \
    if((s)==NULL) {perror(m); exit(EXIT_FAILURE); \
    }
typedef int buffer_item;
#define BUFFER_SIZE 5
// Global variables
buffer_item buffer[BUFFER_SIZE];
pthread_mutex_t mutex;
sem_t full, empty;
int count, in, out;
// Function prototypes
int insert_item(buffer_item item);
int remove_item(buffer_item *item);
void *consumer(void *param);
void *producer(void *param);
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

void *job_of_masterWorker();
void *job_of_worker();

// funzione eseguita dal thread produttore
void *Producer(void *arg) {
    Queue_t *q  = ((threadArgs_t*)arg)->q;
    int   myid  = ((threadArgs_t*)arg)->thid;
    int   start = ((threadArgs_t*)arg)->start;
    int   stop  = ((threadArgs_t*)arg)->stop;
    llist *lista_piena=((threadArgs_t*)arg)->list_file_MasterWorker;//è la lista con tutti i file piena ammodo

    for(int k=0;k<lenghtTail;k++){
        char *data = malloc(sizeof(char));
        if (data == NULL) {
            perror("Producer malloc");
            pthread_exit(NULL);
        }
        //dalla lista piena copio gli argomenti nel data
        //printf("inserisco in data\n\n");
        data = file_singolo_da_inserire(lista_piena);
        //print_list(lista_piena);
        //insrisco nella coda condivisa
        if (push(q, data) == -1) {
            fprintf(stderr, "Errore: push\n");
            pthread_exit(NULL);
        }
        //elimino la testa
        delete_head_lista_piena(&lista_piena,data);
        printf("Producer %d pushed <%s>\n", myid, data);
    }
    //stampo la codona da inviare al worker
    //printf("CODONA NEL MASTER\n\n\n");
    //ho un null all'inizio non si sa come mai
    //StampaLista(q);
    printf("Producer: %d exits\n", myid);
    return NULL;
}

// funzione eseguita dal thread consumatore
void *Consumer(void *arg) {
    //gli passo la codona piena
    file_structure *aus= malloc(sizeof(file_structure));
    Queue_t *q = ((threadArgs_t *) arg)->q;
    int myid = ((threadArgs_t *) arg)->thid;
    char *path_completo=malloc(sizeof(char)*100);
    size_t consumed = 0;
    int risultato = 0;
    while (true) {
        char *data;
        data = dequeue(q);
        assert(data);
        if (data == NULL) {
            free(data);
            break;
        }
        ++consumed;
        printf("Consumer:  %d, dato estratto: <%s>\n", myid, data);
        aus->nomeFile=path_completo;
        printf("AUS: %s\n\n",aus->nomeFile);
        //prendo il path di data; e lo salvo il path_completo
        //si corrompe pathcompleto
        Look_for_file(data, "pluto", 0, aus);
        //prendiamo il data, ovvero il file e lo apriamo per fare la sommatoria
        //ritorniamo l'intero
        //printf("path_completo nel consumer %s\n\n",path_completo);
        risultato = sommatoria(aus->nomeFile);
        printf("RISULTATO: %d\n\n\n", risultato);
        free(data);
        free(aus);
    }
    printf("Consumer %d, consumed <%ld> messages, now it exits\n", myid, consumed);
}

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
    pthread_t tid;
    pthread_t ptid;
    char* file_preso;
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


    // gestione parser
    parser(argc, argv);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //faccio il parsing degli argomenti
    /*   extern char *optarg;
    int p=1,c=numberThreads, n=lenghtTail;//è il numero dei messaggi/dei file da mandare
    printf("num producers =%d, num consumers =%d\n", p, c);
    pthread_t    *th;
    threadArgs_t *thARGS;
    th     = malloc((p+c)*sizeof(pthread_t));
    thARGS = malloc((p+c)*sizeof(threadArgs_t));
    if (!th || !thARGS) {
        fprintf(stderr, "malloc fallita\n");
        exit(EXIT_FAILURE);
    }
    //è la lista che invio al producer per inserire i vari elementi
    llist *lista_da_inviare= malloc((sizeof(llist)));
    Queue_t *q = initQueue();
    //ho riempito la lista da inviare al consumer
    listdir("pluto",0,lista_da_inviare);
    //print_list(lista_da_inviare);
    if (!q) {
        fprintf(stderr, "initQueue fallita\n");
        exit(errno);
    }
    int chunk = n/p, r= n%p;
    int start = 0;
    //inizializzo la struttura per il producer
    for(int i=0;i<p; ++i) {
        thARGS[i].thid = i;
        thARGS[i].q    = q;
        thARGS[i].start= start;
        thARGS[i].stop = start+chunk + ((i<r)?1:0);
        start = thARGS[i].stop;
        thARGS[i].list_file_MasterWorker=lista_da_inviare;
    }
    //inizializzo la struttura per il worker
    for(int i=p;i<(p+c); ++i) {
        thARGS[i].thid = i-p;
        thARGS[i].q    = q;
        thARGS[i].start= 0;
        thARGS[i].stop = 0;
        thARGS[i].list_file_MasterWorker=lista_da_inviare;
    }
    // è il produttore con la coda già piena
    if (pthread_create(&th, NULL, Producer, (void*)&thARGS[i]) != 0) {
        fprintf(stderr, "pthread_create failed (Producer)\n");
        exit(EXIT_FAILURE);
    }
    //sono i workers
    for(int i=0;i<c; ++i){
        if (pthread_create(&th[p+i], NULL, Consumer, (void*)&thARGS[p+i]) != 0) {
            fprintf(stderr, "pthread_create failed (Consumer)\n");
            exit(EXIT_FAILURE);
            }
    }
    // aspetto prima il produttore
    /*  printf("join produttore\n\n");
    pthread_join(th, NULL);
    printf("join workers");
    //qui ho un problemone da guardare
    // aspetto la terminazione di tutti i consumatori
    for(int i=0;i<c; ++i){
    pthread_join(th[p+i], NULL);
    }
    // libero memoria
    deleteQueue(q);
    free(th);
    free(thARGS);
    */
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