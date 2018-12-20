#ifndef LINKED_LIST_H
#define LINKED_LIST_H

struct Cell{
    char * fileName;
    int sizeFile;
    int nbrBlocks;
    int firstServer;
    struct Cell * next;
    struct Cell * end;
};

typedef struct Cell cell_t;
typedef cell_t* list_t;

/*
 * Crée une liste vide
 */
list_t createList();

/*
 * Retourne 1 si une liste est vide, o sinon
 */
int listIsEmpty(list_t l);

/*
 * Retourne une liste remplie à partir d'un fichier
 */
list_t readList();

/*
 * Ecrit sur fichier le contenu de la liste
 */
void writeList(list_t l);

/*
 * Affiche le contenu d'une liste
 */
void printList(list_t l);

/*
 * Ajoute le nouvel élément après la dernière cellule
 */
list_t addCell(char * fileName, int sizeFile, int nbrBlocks, int firstServer, list_t l);

/*
 * Supprime l'élément après la cellule preced
 */
list_t deleteCell(char * fileName, list_t l);

/*
 * Retourne la "position" du premier élément de valeur v dans la liste l
 * et NULL sinon
 * Version itérative et récursive
 */
cell_t* search(int v, list_t l);

/*
 * Réaliser un free de la liste
 */
void freeList(list_t l);

#endif
