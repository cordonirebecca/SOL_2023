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
char buf[PATH_MAX+1];
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
////Funzione per leggere la directory e le sottodirectory

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
        {
            perror("ERRORE nella funzione chdir\n");
        }
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
            printf("Ho preso una directory\n");
            chiamataRicorsivaReturnValue = leggiNFileDaDirectory( numFile2,filename, arrayPath,posizioneArray,bitConteggio, numeroFileLetti);
            if (chiamataRicorsivaReturnValue == -1)
            {
                closeDirReturnValue=closedir(dir);
                if(closeDirReturnValue==-1 && errno!=0)
                {
                    {
                        perror("ERRORE nella funzione closedir\n");
                        return -1;
                    }
                }
                printf("chiusa directory\n\n\n\n\n");
            }
            else
            {

printf("devo salire\n");
                chdirReturnValue=chdir("..");
                if(chdirReturnValue==-1 && errno!=0)
                {
                    {
                        perror("ERRORE nella funzione closedir\n");
                    }
                }
                printf("sono salito\n");
            }
        }
        else if (isFileRegolare)
        {
printf("ho preso un file\n");
            if(bitConteggio == 0)
            {
                (*numeroFileLetti)++;
            }
            else
            {
                lunghezza=strlen(getPathAssoluto(filename))+1;
                path=malloc(sizeof(char)*lunghezza);
                path=getPathAssoluto(filename);
                printf("Adesso lavoro nel path%s\n",path);
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

printf("lascio il file\n");
        }
    }


    closedir(dir);
    printf("chiusa dir\n");

    return 0;
}







////////
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

    /////////leggere file in directory
    char ** arrayPath ;
int numFile2=0;
int numFileLetti=0;
char * dirName="pluto";
char * directoryPartenza;

    directoryPartenza=malloc(sizeof(char)*800);
    directoryPartenza=getcwd(directoryPartenza,800);
    printf("CLIENT: lavoro nella directory:%s\n",directoryPartenza);

    /*Conto quanti file sono effettivamente presenti all' interno della direcotry richiesta*/
    int bitConteggio=0;

    //printf("prima volta--->\n numFile2:%d\ndirname:%s\ni:%d\nbitconteggio:%d\nnumfileletti:%d",numFile2,dirName,i,bitConteggio,numFileLetti);
    int letturaDirectoryReturnValue=leggiNFileDaDirectory(&numFile2,dirName,arrayPath,i,bitConteggio,&numFileLetti);
printf("letti %d file\n",numFileLetti);

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

   int chdirReturnValue=chdir(directoryPartenza);
    if(chdirReturnValue != 0)
    {
        perror("Errore nell' utilizzo di chdir\n");
        return -1;
    }

    //printf("seconda volta--->\n numFile2:%d\ndirname:%s\ni:%d\nbitconteggio:%d\nnumfileletti:%d",numFile2,dirName,i,bitConteggio,numFileLetti);
    letturaDirectoryReturnValue=leggiNFileDaDirectory(&numFile2,dirName,arrayPath,i,bitConteggio,&numFileLetti);


    chdirReturnValue=0;
    chdirReturnValue=chdir(directoryPartenza);
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
    char * rest;
    char * token;
    char * pathRelativo;
    printf("SE STAMPO QUESTO SONO FELICE ------\n------------");
    printf("salvanumfile%d\n\n\n\n\n\n\n\n\n",salvaNumFile);

    for(i = 0; i < salvaNumFile; i++)
    {
        rest = arrayPath[i];

        while ((token = strtok_r(rest, "/", &rest))!=NULL)
        {
            pathRelativo=token;
        }
        printf("Lavoro sul file: %s",pathRelativo);
    }












    ////////////////////////////////////////////



    // apro la directory corrente per contare i miei file
  /*  d = opendir(directoryName);

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
*/
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
