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
	


	int choice=0;
	int c;
	char nomFichier [10];

	//if(k == CUSTOMER){
		while(!choice){
			if(k == CUSTOMER){
				printf("1. put \n");
				printf("2. get \n");
				printf("3. quitter \n");
				scanf("%d",&c);
				getchar();
				MPI_Bcast(&c, 1, MPI_INT, CUSTOMER, MPI_COMM_WORLD);
			}
			MPI_Barrier(MPI_COMM_WORLD);
		switch(c)
		{
			case 1: 
				if(k == CUSTOMER){
					printf("entrer un nom de fichier pour put\n");
					//scanf("%s",nomFichier);
					fgets(nomFichier, sizeof nomFichier, stdin);
					//getchar();
					printf("LE fichier a put : %s",nomFichier);
				}
				put(nomFichier);
				MPI_Barrier(MPI_COMM_WORLD);
				if(k == CUSTOMER)			
					printf("Votre fichier a bien été stocké côté serveur \n----------\n");
				break;
			case 2:printf("entrer un nom de fichier pour get\n");
				fgets(nomFichier, sizeof nomFichier, stdin);
				//getchar();
				get(nomFichier);
				printf("L'opération de récupération a été effectué avec succès: \n Votre fichier se trouve sous le répértoire %s \n----------\n",ROOT_DIR);
				break;
			case 3: printf("Quitter \n");
				choice = 1;
				break;
			default: if(k==CUSTOMER) printf("Wrong choice \n"); break;
		}
	}
	//}

	//Le client (rang = 0) envoie le fichier en block aux serveurs de stockage ( rang = 1 , rang = 2, rang = 3)
  /*  put(FILENAME);
	if(k==0)
		printf("Votre fichier a bien été stocké côté serveur \n----------\n");*/
	//Récupérer les blocs depuis les serveurs
	/*char buffer[SIZE_BUFFER];
	MPI_Barrier(MPI_COMM_WORLD);
	get(getCountBlocs(FILENAME) + 1, FIRST_SERVER,buffer);*/
	//MPI_Barrier(MPI_COMM_WORLD);
/*	get(FILENAME);
	if(k==0)
		printf("L'opération de récupération a été effectué avec succès: \n Votre fichier se trouve sous le répértoire %s \n----------\n",ROOT_DIR);*/
    MPI_Finalize();                                 //End MPI
    
    return EXIT_SUCCESS;
}
