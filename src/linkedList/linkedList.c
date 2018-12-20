#include "linkedList.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Crée une liste vide
 */
list_t createList(){
    list_t l;
    l = (list_t) malloc(sizeof(cell_t));
    l->fileName = NULL;
    l->sizeFile = -1;
    l->nbrBlocks = -1;
    l->firstServer = -1;
    l->next = NULL;
    l->end = NULL;
    return l;
}

/*
 * Retourne 1 si une liste est vide, o sinon
 */
int listIsEmpty(list_t l){

    if (l->next == NULL && l->end == NULL)
	return 1;
    else
	return 0;
}

/*
 * Retourne une liste remplie par un utilisateur
 */
list_t readList(){

    list_t l = createList();

    return l;
}

/*
 * Ecrit sur fichier le contenu de la liste
 */
void writeList(list_t l){
  
}

/*
 * Affiche le contenu d'une liste
 */
void printList(list_t l){
	
    if (listIsEmpty(l)){
	printf("La liste est vide.\n");
	return;
    }

    cell_t * tmp = l;

    while(tmp->next != NULL){
	tmp = tmp->next;
	printf("%s : %d octet(s)\n\t%d bloc(s)\n\tLe premier bloc est sur le serveur \
%d\n", tmp->fileName, tmp->sizeFile, tmp->nbrBlocks, tmp->firstServer);
    }
}

/*
 * Ajoute le nouvel élément après la dernière cellule
 */
list_t addCell(char * fileName, int sizeFile, int nbrBlocks, int firstServer, list_t l){

    //Création de la nouvelle cellule à insérer
    cell_t * new;
    new = (cell_t*) malloc(sizeof(cell_t));
    new->fileName = fileName;
    new->sizeFile = sizeFile;
    new->nbrBlocks = nbrBlocks;
    new->firstServer = firstServer;
    new->next = NULL;
    new->end = NULL;
    
    if(l->end == NULL){
	l->next = new;
	l->end = new;
    }
    else
	l->end->next = new;

    return l;
}

/*
 * Supprime l'élément après la cellule preced
 */
list_t deleteCell(char * fileName, list_t l){

    cell_t * tmp = l->next;
    cell_t * prev = l;
    int match = match = strcmp(tmp->fileName, fileName);
    
    while(tmp->next != NULL &&  match != 0){
	prev = tmp;
	tmp = tmp->next;
	match = strcmp(tmp->fileName, fileName);
    }
        
    if(match != 0)
	return NULL;
    
    prev->next = tmp->next;
    prev->end = tmp->end;
    free(tmp);

    return l;
}

/*
 * Réaliser un free de la liste
 */
void freeList(list_t l){
 
    cell_t * tmp;
    while (l->next != NULL){
	tmp = l->next;
	l->next = l->next->next;
	free(tmp);
    }
    free(l);
}