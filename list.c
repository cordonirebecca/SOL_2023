//
// Created by rebecca on 16/09/22.
//
#include <stdio.h>
#include <stdlib.h>
#include"list.h"
/*
void printList(NodeList *l){
    while(l != NULL){
        printf("%d -> ", l->info);
        l=l->next;
    }
    printf("NULL");
}

void insertHead(NodeList **l, int el){
    NodeList *newList=malloc(sizeof(NodeList));
    newList->info= el;
    newList->next=*l;
    *l=newList;
}

//cancello primo elemento uguale ad el.
void delete(NodeList**l, int el){
    if(*l!=NULL){
        if((*l)->info == el){
            NodeList *aus=*l;
            *l=(*l)->next;
            free(aus);
        }
        else{
            delete(&(*l)->next, el);
        }
    }
}

*/





//REBECCA; LAVORA SU QUESTO QUI SOTTO; NON SOPRA! TESTONA ;)


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include <math.h>
#include "list.h"

//Gestione di una coda di stringhe

int is_empty_list(struct struttura_coda* head)
{
    return head->opzione == NULL;
}

int is_valid_list(struct struttura_coda* head)
{
    return head != NULL;
}

int enqueue(struct struttura_coda* head, char * opzione)
{
    if (!is_valid_list(head))
    {
        //printf("Errore, testa della lista uguale a null");
        return 0;
    }

    if(is_empty_list(head))
    {
        head->opzione=(char*)malloc(80*sizeof(char));
        strcpy(head->opzione,opzione);
        head->prec = NULL;
        head->next = NULL;
        return 1;
    }
    else
    {
        struct struttura_coda *nodoCorrente=head;
        while(nodoCorrente->next != NULL)
        {
            nodoCorrente=nodoCorrente->next;
        }
        struct struttura_coda* nodo_successivo = malloc(sizeof(struct struttura_coda));
        nodo_successivo->opzione=malloc(80*sizeof(char));
        //nodo_successivo->opzione = opzione;
        strcpy(nodo_successivo->opzione,opzione);
        nodo_successivo->next = NULL;
        nodoCorrente->next=nodo_successivo;
        nodo_successivo->prec = nodoCorrente;


        return 1;
    }
}


char * dequeue(struct struttura_coda* head)
{
    if (!is_valid_list(head))
    {
        perror("Errore! head == NULL");
        exit(1);
    }
    if(is_empty_list(head))
    {
        ////printf("LISTA VUOTA!");
        return NULL;
    }
    else
    {
        char *comando_scodato =(char*)malloc(80*sizeof(char));
        strcpy(comando_scodato,head->opzione);
        if (head->next == NULL)
        {
            /* se la lista � fatta da un solo elemento, allora la lista diventa una lista vuota */
            head->opzione=NULL;
        }
        else
        {
            /* se invece devo eliminare il primo elemento della lista, ma la lista � formata da pi� elementi.
             * Allora il secondo elemento della lista diventa il primo
             */
            struct struttura_coda *tmp = head->next;
            strcpy(head->opzione,tmp->opzione);
            head->prec = NULL;
            head->next = tmp->next;
            /* nota che il puntatore al primo elemento della lista non � cambiato. � cambiato solamente il suo contenuto
             * (che adesso � il secondo cliente della lista). Quindi nel caso in cui la lista sia di 3 o pi� elementi devo anche aggiustare
             * il puntatore prec del terzo elemento della lista, per farlo puntare a head (perch� prima puntava al secondo elemento della lista).
             */
            if(head->next != NULL)
            {
                head->next->prec = head;
            }
            free(tmp);
        }
        ////printf("ho eliminato il comando dalla lista, ritorno la stringa:%s\n",comando_scodato);
        //return comando_scodato;
        return comando_scodato;
    }
}

char * rimuovi(struct struttura_coda* head, char * opzione)
{
    struct struttura_coda* temp;
    struct struttura_coda* nodo_da_rimuovere = NULL;

    if (!is_valid_list(head))
    {
        perror("Errore! head == NULL");
        exit(1);
    }

    if(is_empty_list(head))
    {
        ////printf("Tentativo di eliminazione testa da lista vuota\n");
        return NULL;
    }
    else
    {
        temp=head;
        while(temp != NULL && temp->opzione != 0 && nodo_da_rimuovere == NULL)
        {
            if (temp != NULL && temp->opzione != 0)
            {
                if ( (temp->opzione) == opzione )
                {
                    nodo_da_rimuovere = temp;
                }
            }
            temp=temp->next;
        }
        if(nodo_da_rimuovere != NULL) /* se ho trovato il nodo da rimuovere */
        {
            if (nodo_da_rimuovere == head)
            { /* se il nodo da rimuovere � il primo della lista in pratica � come fare una dequeue */
                ////printf("rimuovo nodo in testa\n");
                dequeue(head);
                return opzione;
                ////printf("Il cliente con id: %d si è spostato di cassa\n", nodo_da_rimuovere->cliente->idCliente);
            }
            else
            {
                /* se il nodo da rimuovere � nel mezzo della lista, allora per rimuoverlo devo cortocircuitare il nodo precedente a quello successivo
                 */
                struct struttura_coda *nodo_prec = nodo_da_rimuovere->prec;
                nodo_prec->next=nodo_da_rimuovere->next;
                if (nodo_da_rimuovere->next != NULL)
                {
                    nodo_da_rimuovere->next->prec = nodo_prec;
                }
                free(nodo_da_rimuovere);
                return opzione;
                ////printf("Il cliente con id: %d si è spostato di cassa\n", nodo_da_rimuovere->cliente->idCliente);


            }
        }
        else
        {
            return NULL;
        }
    }

}


void StampaLista(struct struttura_coda* head)
{
    struct struttura_coda *temp = head;
    if (!is_valid_list(head))
    {
        // tirare errore se head = null
        //printf("Errore Stampa Lista");
        exit(1);
    }

    if(is_empty_list(head))
    {
        //printf("Lista vuota\n");
    }
    else
    {
        //printf("Lista:");
        while (temp != NULL)
        {
            //printf(" %s", temp->opzione);
            temp = temp->next;
        }
        //printf("\n");
    }
}

int size(struct struttura_coda* head)
{
    int size = 0;
    struct struttura_coda *temp = head;
    if (!is_valid_list(head))
    {
        // tirare errore se head = null
        //printf("Errore Size Coda\n");
        exit(1);
    }

    if(is_empty_list(head))
    {
        size = 0;
    }
    else
    {
        while (temp != NULL)
        {
            size+=1;
            temp = temp->next;
        }
    }
    return size;
}