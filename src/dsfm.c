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
    printf("Rang : %d\nMachine : %s\n----------\n", k, name);

	//Le client (rang = 0) envoie le nombre de blocs, la taille du fichier et le nom du fichier au serveur LB ( rang = 1)
	//sendInfoToLB();
	printf("\n----------\n");
	//Le client (rang = 0) envoie le fichier en block aux serveurs de stockage ( rang = 1 , rang = 2, rang = 3)
    sendFileToServers();

	//test readBloc
	getBlocsFromServers(4);

    MPI_Finalize();                                 //End MPI
    
    return EXIT_SUCCESS;
}
