#include <stdio.h>
#include <stdlib.h>
#include "linkedList.h"

int main(){

    list_t l = createList();
    printf("Yooooooooupi 1\n");
    addCell("Fichier_1", 18, 3, 1, l);
    printf("Yooooooooupi 2\n");
    addCell("Fichier_2", 24, 3, 2, l);
    printList(l);
    printf("-------------------------\n");
    if (deleteCell("testnull", l) == NULL) printf("testnull n'existe pas dans l\n");
    printf("-------------------------\n");
    deleteCell("Fichier_1", l);
    printList(l);
    printf("-------------------------\n");
    
    freeList(l);
    
    return 0;
}
