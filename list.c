//
// Created by rebecca on 02/01/23.
//
#include "list.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>

int is_empty_list(struct llist* head){
    return head->opzione == NULL;
}

int is_valid_list(struct llist* head){
    return head != NULL;
}

void StampaLista(struct llist* head){
    struct llist *temp = head;
    if (!is_valid_list(head)){
        // tirare errore se head = null
        printf("Errore Stampa Lista");
        exit(1);
    }

    if(is_empty_list(head)){
        printf("Lista vuota\n");
    }
    else{
        printf("Lista:");
        while (temp != NULL){
            printf(" %s -> ", temp->opzione);
            temp = temp->next;
        }
        printf("\n");
    }
}

void enqueue(struct llist* head, char * opzione){
    struct llist *new= malloc(sizeof(struct llist));
    struct llist *nodoCorrente= malloc(sizeof(struct llist));
    new->opzione=malloc(80* sizeof(char));
    strcpy(new->opzione,opzione);
    new->next=NULL;
    if((head->opzione)==NULL) {
        (head->opzione) = (new->opzione);
    }else{
        nodoCorrente=head;
        while(nodoCorrente->next != NULL){
            nodoCorrente=nodoCorrente->next;
        }
    }
    nodoCorrente->next=new;
}

//funzione che apre tutte le cartelle e mi stampa i file in ognuna
void listdir(const char *name, int indent,struct llist *l){
    DIR *dir;
    struct dirent *entry;
    if (!(dir = opendir(name)))
        return;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            char path[1024];
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            //path è il percorso directory senza i file
            snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
            // printf("%*s[%s]\n", indent, "", entry->d_name);
            listdir(path, indent + 2,l);
        } else {
            // sono uno o più file nella directory
            //  printf("%*s- %s\n", indent, "", entry->d_name);
            enqueue(l,entry->d_name);
        }
    }
    closedir(dir);
}