#include "dsfm_fct.h"
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

/*
 * Implémentation de la diffusion en étoile
 * en utilisant uniquement des opérations
 * d'envoi et de réception
 */

int main( int argc, char **argv ) {
    int k, kmax;                                    //Rang du processus et nb processus
    char name[MPI_MAX_PROCESSOR_NAME];              //Récupération du nom de la 
    int size;                                       //machine host
    
    MPI_Init( &argc, &argv );                       //Init MPI
    MPI_Comm_rank(MPI_COMM_WORLD, &k);              //Récupération du rang
    MPI_Comm_size(MPI_COMM_WORLD, &kmax);           //Récupération du nombre de processus
    MPI_Get_processor_name(name, &size);            //Récupération du hostname
    /*if(k==0){
	int test = assertHostfile();
	printf("%d\n", test);
    }*/
	if(k== CUSTOMER)
    	printf("Rang : %d\n Machine : %s\n Machine du client \n----------\n", k, name);
	else if(k == LB)
    	printf("Rang : %d\n Machine : %s\n Serveur Load Balancer / stockage \n----------\n", k, name);
	else if(k == SERVER_1)
    	printf("Rang : %d\n Machine : %s\n Serveur de stockage 1 \n----------\n", k, name);
	else if(k == SERVER_2)
    	printf("Rang : %d\n Machine : %s\n Serveur de stockage 2 \n----------\n", k, name);	
	else if(k == SERVER_3)
    	printf("Rang : %d\n Machine : %s\n Serveur de stockage 3 \n----------\n", k, name); 

	//Le client (rang = 0) envoie le fichier en block aux serveurs de stockage ( rang = 1 , rang = 2, rang = 3)
    put(FILENAME);
	if(k==0)
		printf("Votre fichier a bien été stocké côté serveur \n----------\n");
	//Récupérer les blocs depuis les serveurs
	char buffer[SIZE_BUFFER];
	MPI_Barrier(MPI_COMM_WORLD);
	get(getCountBlocs(FILENAME) + 1, FIRST_SERVER,buffer);
	if(k==0)
		printf("L'opération de récupération a été effectué avec succès: \n Votre fichier se trouve sous le répértoire %s \n----------\n",ROOT_DIR);
    MPI_Finalize();                                 //End MPI
    
    return EXIT_SUCCESS;
}
