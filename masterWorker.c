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
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>

#define SOCKNAME     "./cs_sock"
#define MAXBACKLOG   32


#define MAXPATHLEN 100
#define CHECKNULL(r,c,e) CHECK_EQ_EXIT(e, (r=c), NULL,e,"")


#define ec_meno1(s,m) \
    if((s)==-1) {perror(m); exit(EXIT_FAILURE); \
    }

#define ec_null(s,m) \
    if((s)==NULL) {perror(m); exit(EXIT_FAILURE); \
    }

typedef int buffer_item;
#define BUFFER_SIZE 5


/** Evita letture parziali
 *
 *   \retval -1   errore (errno settato)
 *   \retval  0   se durante la lettura da fd leggo EOF
 *   \retval size se termina con successo
 */
static inline int readn(long fd, void *buf, size_t size) {
    size_t left = size;
    int r;
    char *bufptr = (char*)buf;
    while(left>0) {
        if ((r=read((int)fd ,bufptr,left)) == -1) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (r == 0) return 0;   // EOF
        left    -= r;
        bufptr  += r;
    }
    return size;
}

/** Evita scritture parziali
 *
 *   \retval -1   errore (errno settato)
 *   \retval  0   se durante la scrittura la write ritorna 0
 *   \retval  1   se la scrittura termina con successo
 */
static inline int writen(long fd, void *buf, size_t size) {
    size_t left = size;
    int r;
    char *bufptr = (char*)buf;
    while(left>0) {
        if ((r=write((int)fd ,bufptr,left)) == -1) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (r == 0) return 0;
        left    -= r;
        bufptr  += r;
    }
    return 1;
}



#if !defined(BUFSIZE)
#define BUFSIZE 256
#endif

#if !defined(EXTRA_LEN_PRINT_ERROR)
#define EXTRA_LEN_PRINT_ERROR   512
#endif

#define SYSCALL_EXIT(name, r, sc, str, ...)	\
    if ((r=sc) == -1) {				\
	perror(#name);				\
	int errno_copy = errno;			\
	print_error(str, __VA_ARGS__);		\
	exit(errno_copy);			\
    }

#define SYSCALL_PRINT(name, r, sc, str, ...)	\
    if ((r=sc) == -1) {				\
	perror(#name);				\
	int errno_copy = errno;			\
	print_error(str, __VA_ARGS__);		\
	errno = errno_copy;			\
    }

#define SYSCALL_RETURN(name, r, sc, str, ...)	\
    if ((r=sc) == -1) {				\
	perror(#name);				\
	int errno_copy = errno;			\
	print_error(str, __VA_ARGS__);		\
	errno = errno_copy;			\
	return r;                               \
    }

#define CHECK_EQ_EXIT(name, X, val, str, ...)	\
    if ((X)==val) {				\
        perror(#name);				\
	int errno_copy = errno;			\
	print_error(str, __VA_ARGS__);		\
	exit(errno_copy);			\
    }

#define CHECK_NEQ_EXIT(name, X, val, str, ...)	\
    if ((X)!=val) {				\
        perror(#name);				\
	int errno_copy = errno;			\
	print_error(str, __VA_ARGS__);		\
	exit(errno_copy);			\
    }

/**
 * \brief Procedura di utilita' per la stampa degli errori
 *
 */
static inline void print_error(const char * str, ...) {
    const char err[]="ERROR: ";
    va_list argp;
    char * p=(char *)malloc(strlen(str)+strlen(err)+EXTRA_LEN_PRINT_ERROR);
    if (!p) {
        perror("malloc");
        fprintf(stderr,"FATAL ERROR nella funzione 'print_error'\n");
        return;
    }
    strcpy(p,err);
    strcpy(p+strlen(err), str);
    va_start(argp, str);
    vfprintf(stderr, p, argp);
    va_end(argp);
    free(p);
}


/**
 * \brief Controlla se la stringa passata come primo argomento e' un numero.
 * \return  0 ok  1 non e' un numbero   2 overflow/underflow
 */
static inline int isNumber(const char* s, long* n) {
    if (s==NULL) return 1;
    if (strlen(s)==0) return 1;
    char* e = NULL;
    errno=0;
    long val = strtol(s, &e, 10);
    if (errno == ERANGE) return 2;    // overflow/underflow
    if (e != NULL && *e == (char)0) {
        *n = val;
        return 0;   // successo
    }
    return 1;   // non e' un numero
}

#define LOCK(l)      if (pthread_mutex_lock(l)!=0)        { \
    fprintf(stderr, "ERRORE FATALE lock\n");		    \
    pthread_exit((void*)EXIT_FAILURE);			    \
  }
#define UNLOCK(l)    if (pthread_mutex_unlock(l)!=0)      { \
  fprintf(stderr, "ERRORE FATALE unlock\n");		    \
  pthread_exit((void*)EXIT_FAILURE);				    \
  }
#define WAIT(c,l)    if (pthread_cond_wait(c,l)!=0)       { \
    fprintf(stderr, "ERRORE FATALE wait\n");		    \
    pthread_exit((void*)EXIT_FAILURE);				    \
}
/* ATTENZIONE: t e' un tempo assoluto! */
#define TWAIT(c,l,t) {							\
    int r=0;								\
    if ((r=pthread_cond_timedwait(c,l,t))!=0 && r!=ETIMEDOUT) {		\
      fprintf(stderr, "ERRORE FATALE timed wait\n");			\
      pthread_exit((void*)EXIT_FAILURE);					\
    }									\
  }
#define SIGNAL(c)    if (pthread_cond_signal(c)!=0)       {	\
    fprintf(stderr, "ERRORE FATALE signal\n");			\
    pthread_exit((void*)EXIT_FAILURE);					\
  }
#define BCAST(c)     if (pthread_cond_broadcast(c)!=0)    {		\
    fprintf(stderr, "ERRORE FATALE broadcast\n");			\
    pthread_exit((void*)EXIT_FAILURE);						\
  }
static inline int TRYLOCK(pthread_mutex_t* l) {
    int r=0;
    if ((r=pthread_mutex_trylock(l))!=0 && r!=EBUSY) {
        fprintf(stderr, "ERRORE FATALE unlock\n");
        pthread_exit((void*)EXIT_FAILURE);
    }
    return r;
}

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


#define CHECKNULL(r,c,e) CHECK_EQ_EXIT(e, (r=c), NULL,e,"")

/**
 * tipo del messaggio
 */
typedef struct msg {
    int len;
    char *str;
} msg_t;


void cleanup() {
    unlink(SOCKNAME);
}
int cmd(const char str[], char *buf) {
    int tobc[2];
    int frombc[2];

    int notused;
    SYSCALL_EXIT("pipe", notused, pipe(tobc), "pipe1", "");
    SYSCALL_EXIT("pipe", notused, pipe(frombc), "pipe2","");

    if (fork() == 0) {
        // chiudo i descrittori che non uso
        SYSCALL_EXIT("close", notused, close(tobc[1]), "close", "");
        SYSCALL_EXIT("close", notused, close(frombc[0]), "close", "");

        SYSCALL_EXIT("dup2", notused, dup2(tobc[0],0),   "dup2 child (1)", "");  // stdin
        SYSCALL_EXIT("dup2", notused, dup2(frombc[1],1), "dup2 child (2)", "");  // stdout
        SYSCALL_EXIT("dup2", notused, dup2(frombc[1],2), "dup2 child (3)", "");  // stderr

        execl("/usr/bin/bc", "bc", "-l", NULL);
        return -1;
    }
    // chiudo i descrittori che non uso
    SYSCALL_EXIT("close", notused, close(tobc[0]), "close","");
    SYSCALL_EXIT("close", notused, close(frombc[1]), "close","");
    int n;
    SYSCALL_EXIT("write", n, write(tobc[1], (char*)str,strlen(str)), "writen","");
    SYSCALL_EXIT("read",  n, read(frombc[0], buf, BUFSIZE), "readn","");  // leggo il risultato o l'errore
    SYSCALL_EXIT("close", notused, close(tobc[1]), "close","");  // si chiude lo standard input di bc cosi' da farlo terminare
    SYSCALL_EXIT("wait", notused, wait(NULL), "wait","");
    return n;
}

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

struct file_name* estrai(){

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

    //gestione parser
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
       for(int i=0;i<c; ++i)
           if (pthread_create(&th[p+i], NULL, Consumer, (void*)&thARGS[p+i]) != 0) {
               fprintf(stderr, "pthread_create failed (Consumer)\n");
               exit(EXIT_FAILURE);
           }
       /* possibile protocollo di terminazione:
        * si aspettano prima tutti i produttori
        * quindi si inviano 'c' valori speciali (-1)
        * quindi si aspettano i consumatori
        */
    // aspetto prima il produttore
    /*   printf("join produttore\n\n");
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

    // cancello il socket file se esiste
    cleanup();
    // se qualcosa va storto ....
    atexit(cleanup);

    int listenfd;
    // creo il socket
    SYSCALL_EXIT("socket", listenfd, socket(AF_UNIX, SOCK_STREAM, 0), "socket","");

    // setto l'indirizzo
    struct sockaddr_un serv_addr;
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strncpy(serv_addr.sun_path, SOCKNAME, strlen(SOCKNAME)+1);

    int notused;
    // assegno l'indirizzo al socket
    SYSCALL_EXIT("bind", notused, bind(listenfd, (struct sockaddr*)&serv_addr,sizeof(serv_addr)), "bind", "");

    // setto il socket in modalita' passiva e definisco un n. massimo di connessioni pendenti
    SYSCALL_EXIT("listen", notused, listen(listenfd, 1), "listen","");

    int connfd, n;
    do {
        SYSCALL_EXIT("accept", connfd, accept(listenfd, (struct sockaddr*)NULL ,NULL), "accept","");

        msg_t str;
        CHECKNULL(str.str,malloc(BUFSIZE), "malloc");
        while(1) {
            char buffer[BUFSIZE];

            memset(str.str, '\0', BUFSIZE);
            // leggo la size della stringa
            SYSCALL_EXIT("readn", n, readn(connfd, &str.len, sizeof(int)), "readn1", "");
            // leggo la stringa
            SYSCALL_EXIT("readn", n, readn(connfd, str.str, str.len), "readn2", "");
            if (n==0) break;
            memset(buffer, '\0', BUFSIZE);
            if ((n = cmd(str.str, buffer)) < 0) {
                fprintf(stderr, "Errore nell'esecuzione del comando\n");
                break;
            }
            buffer[n] = '\0';

            // invio la risposta
#if 1
            SYSCALL_EXIT("writen", notused, writen(connfd, &n, sizeof(int)), "writen1", "");
            SYSCALL_EXIT("writen", notused, writen(connfd, buffer, n), "writen2", "");
#else
            // qui si puo' utilizzare anche writev (man 2 writev) invece che 2 write distinte
	// NOTA: andrebbe implementata una writevn per evitare le "scritture parziali",
	//       l'implementazione e' un po' piu' complessa della writen
	struct iovec iov[2] = { {&n, sizeof(int)}, {buffer, n} };
	SYSCALL_EXIT("writev", notused, writev(connfd, iov, 2), "writev", "");
#endif
        }
        close(connfd);
        printf("connection done\n");
        if (str.str) free(str.str);
    } while(1);
    return 0;
}


void *job_of_masterWorker(void* arg){

}


void *job_of_worker(void *arg){

}
