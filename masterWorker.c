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
#include <limits.h>
#include "auxiliaryMW.h"
#include <ctype.h>
#define UNIX_PATH_MAX 255

int numberThreads=4;
int lenghtTail =8;
const char* directoryName;
char path_assoluto [UNIX_PATH_MAX+1];
int tempo=0;



#define ec_meno1(s,m) \
    if((s)==-1) {perror(m); exit(EXIT_FAILURE); \
    }

#define ec_null(s,m) \
    if((s)==NULL) {perror(m); exit(EXIT_FAILURE); \
    }



int isCurrentDirOrParentDir(char *nomeDirectory)
{
    if (strcmp(nomeDirectory, ".") == 0)
    {
        return 1;
    }
    else if(strcmp(nomeDirectory, "..") == 0)
    {
        return 2;
    }
    else
    {
        return 0;
    }
}

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
    else if((path== NULL) && (errno==ENOENT)){
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
    opterr=0;
// manca il caso di default da analizzare
   while((c = getopt(argc, argv, "n:q:d:t:")) != -1){
       switch(c){
           case 'n':
               //printf (" è arrivato -n \n");
               ec_null(c,"main");
               //printf("dentro\n");
               //printf("optarg: %s\n",optarg);
               //printf("c: %d\n",c);
               //printf("optopt: %c\n",optopt);
               if(strcmp(optarg,"-q")!= 0){
                   numberThreads = atoi(optarg);
               }else{
                   numberThreads= 4;
               }
              // printf("numberThreads: %d\n", numberThreads);
               break;

           case 'q':
               //printf (" è arrivato -q \n");
               ec_null(c,"main");
               //printf( " c: %d\n", c);
               if(optopt == 0){
                   lenghtTail = atoi(optarg);
               }
               //printf("lenghtail: %d\n", lenghtTail);
               break;

           case 'd':
               //printf (" è arrivato -d \n");
               ec_null(c,"main");
               //printf( " argv: %c\n", c);
               directoryName= optarg;
               directoryName=malloc(sizeof(char)*500);
               directoryName=getcwd(directoryName,500);
               printf("CLIENT: lavoro nella directory:%s\n",directoryName);
               //directoryName=getPathAssoluto(directoryName);
               printf("pathAssoluto: %s\n",directoryName);
               break;

           case 't':
               //printf (" è arrivato -t \n");
               ec_null(c,"main");
               //printf( " argv: %c\n", c);
               if(optopt == 0){
                   tempo= atoi(optarg);
               }
               //printf("tempo: %d\n", tempo);
               break;

           case '?':
              // printf("optopt: %d\n",optopt);
               //printf("optarg: %s\n",optarg);

               printf("n: %d\n",'n');
               return 1;

       }
   }
}

void* startWorker(void* tizio){

    int indiceWorker=(intptr_t) tizio;
    //printf("sono lo schiavo numero %d\n",tizio);

}

/*Funzione che legge N file da una directory*/
/*nel caso in cui siano presenti sottoDirectory, le visita ricorsivamente fino al raggiungimento di N*/
int leggiNFileDaDirectory(int *numFile2,const char *dirName, char** arrayPath, int posizioneArray, short bitConteggio, int *numeroFileLetti)
{
    int chDirReturnValue=0;
    int leggiTuttiIFile=0;
    char *filename;
    int closeDirReturnValue=0;
    struct stat s;
    int isFileCurrentDir = 0;
    int chiamataRicorsivaReturnValue=0;
    int isDirectory = 0;
    int isFileRegolare = 0;
    int chdirReturnValue=0;
    int lunghezza=0;
    char *path =NULL;
    struct dirent *file = NULL;

    if(numFile2 == NULL)
    {
        perror("ERRORE è stato passato alla funzione un valore non valido");
        return -1;
    }


    leggiTuttiIFile = *numFile2 <= 0;
    printf("dirName in funzione: %s\n",dirName);
    DIR *dir = opendir(dirName);
    if(errno==EACCES)
    {
        printf("ERRNO=EACCES");
    }
    if(errno==EBADF)
    {
        printf("ERRNO=EBADF");
    }
    if(errno==EMFILE)
    {
        printf("ERRNO=EMFILE");
    }
    if(errno==ENFILE)
    {
        printf("ERRNO=ENFILE");
    }
    if(errno==ENOENT)
    {
        printf("ERRNO=ENOENT");
    }
    if(errno==ENOMEM)
    {
        printf("ERRNO=ENOMEM");
    }
    if(errno==ENOTDIR)
    {
        printf("ERRNO=ENOTDIR");
    }
    if(dir==NULL)
    {
        perror("ERRORE nella funzione opendir\n");
        return -1;
    }

    /* Eseguo operazione cd nella directory selezionata*/
    chDirReturnValue=chdir(dirName);
    if(chDirReturnValue==-1 && errno!=0)
    {
        perror("ERRORE nella funzione chdir\n");
        return -1;
    }


    /*Leggo ogni entry presente nella directory fino a che non ho letto tutti i file oppure ho raggiunto il limite*/
    while ((leggiTuttiIFile || *numFile2) && (file = readdir(dir)) != NULL)
    {
        filename = file->d_name;

        stat(filename, &s);



        isFileCurrentDir = isCurrentDirOrParentDir(filename);

        isDirectory = S_ISDIR(s.st_mode);
        isFileRegolare = S_ISREG(s.st_mode);

        /*Tramite questi tre if, se ho selezionato un file*/
        /*speciale lo salto, non considerandolo nel conteggio*/
        if (isFileCurrentDir == 1 || isFileCurrentDir == 2)
        {
            continue;
        }
        else if (!isDirectory && !isFileRegolare)
        {
            /*se entro dentro questo if significato che*/
            /*grazie all' utilizzo della struttura stat,*/
            /*sono riuscito ad identificare che il file considerato in questo momento non*/
            /*risulta un file regolare e nemmeno una directory*/
            continue;
        }
        else if (isDirectory)
        {
            chiamataRicorsivaReturnValue = leggiNFileDaDirectory( numFile2,filename, arrayPath,posizioneArray,bitConteggio, numeroFileLetti);
            if (chiamataRicorsivaReturnValue == -1)
            {
                closeDirReturnValue=closedir(dir);
                if(closeDirReturnValue==-1 && errno!=0)
                {
                        perror("ERRORE nella funzione closedir\n");
                        return -1;
                }
                printf("chiusa directory\n\n\n\n\n");
            }
            else
            {


                chdirReturnValue=chdir("..");
                if(chdirReturnValue==-1 && errno!=0)
                {
                        perror("ERRORE nella funzione closedir\n");
                }
            }
        }
        else if (isFileRegolare)
        {

            if(bitConteggio == 0)
            {
                (*numeroFileLetti)++;
            }
            else
            {
                lunghezza=strlen(getPathAssoluto(filename))+1;
                path=malloc(sizeof(char)*lunghezza);
                path=getPathAssoluto(filename);
                if(path==NULL)
                {
                    errno=EINVAL;
                    return -1;
                }

                if(strcmp(arrayPath[posizioneArray],"")!=0)
                {
                    posizioneArray++;
                }
                strcpy(arrayPath[posizioneArray],path);
            }
            if (!leggiTuttiIFile)
            {
                (*numFile2)--;
            }


        }
    }


    closedir(dir);
    printf("chiusa dir\n");

    return 0;
}

int main(int argc, char* argv []){
    int pid;
    int status;
    struct struttura_coda *comandi = malloc(sizeof(struct struttura_coda));
    comandi->opzione=NULL;
    comandi->next = NULL;
    comandi->prec = NULL;
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
            //printf("ho aspettato il figlio sto morendo anch'io\n\n");
        }
    }//fine creazione collector

    //gestione parser
    parser(argc, argv);

    //test directory










    char **arrayPath;
    char* rest;
    printf("CLIENT: lavoro nella directory:%s\n",directoryName);

    /*Conto quanti file sono effettivamente presenti all' interno della direcotry richiesta*/
    int bitConteggio=0,i=0,numFileLetti=0,numFile2=0;

    int letturaDirectoryReturnValue=leggiNFileDaDirectory(&numFile2,directoryName,arrayPath,i,bitConteggio,&numFileLetti);


    if(letturaDirectoryReturnValue != 0)
    {
        perror("Errore nella lettura dei file dalla directory\n");
    }

    if(numFile2<=0 || numFile2>numFileLetti)
    {
        numFile2=numFileLetti;

    }
    bitConteggio=1;


    arrayPath = malloc(numFile2 * sizeof(char *));
    for(i=0; i<numFile2; i++)
    {
        arrayPath[i] = malloc(200 * sizeof(char));
        strncpy(arrayPath[i],"",2);
    }

    i=0;

    int salvaNumFile=numFile2;


    /*Dopo essermi memorizzato la directory corrente da cui partivo, utilizzo la funzione chdir per ritornarci.
    Questo risulta essere necessario perchè al procedura leggiNFileDaDirectory mi ha modificato al directory in cui sto lavorando.*/

    int chdirReturnValue=chdir(directoryName);
    if(chdirReturnValue != 0)
    {
        /*è stato settato errno*/
        perror("Errore nell' utilizzo di chdir\n");
        return -1;
    }


    letturaDirectoryReturnValue=leggiNFileDaDirectory(&numFile2,directoryName,arrayPath,i,bitConteggio,&numFileLetti);


    chdirReturnValue=0;
    chdirReturnValue=chdir(directoryName);
    if(chdirReturnValue != 0)
    {
        /*è stato settato errno*/
        perror("Errore nell' utilizzo di chdir\n");
        return -1;
    }

    if(letturaDirectoryReturnValue != 0)
    {
        perror("Errore nella lettura dei file dalla directory\n");
    }
    char * token=NULL;
    char * pathRelativo;
    for(i = 0; i < salvaNumFile; i++)
    {
        rest = arrayPath[i];

        while ((token = strtok_r(rest, "/", &rest))!=NULL)
        {
            pathRelativo=token;
        }
        printf("Lavoro sul file: %s",pathRelativo);
    }
















    //fine test directory
    //inserisco in modo concorrente i task in coda
    //vorrei inserire i task con la inserTail ma in mutua eslusione come faccio


    //creo threadWorkers

    int j=0;
    for(j=0;j<numberThreads;j++)
    {
        if(err = pthread_create(&tid,NULL,startWorker, (void*)(intptr_t)i ))
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
