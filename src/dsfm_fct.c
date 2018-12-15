#include "dsfm_fct.h"
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void sendFileToServers(){

	int numprocs, rank, i;
	int nbrBlocs;

    MPI_File fh;
    MPI_Offset offset;

	//Nom du fichier
    char fileName[FILENAME_LENGTH] = "test.txt";

	//Le tableau qui va contenir les morceaux du fichiers ( la taille du bloc est de 8K)
	char buffer[8];

	//Le rang des ps
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	//Le nombre de ps
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);

	//Ouverture du fichier
    MPI_File_open(MPI_COMM_WORLD, fileName, MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
	
	//La taille du fichier
    MPI_File_get_size(fh, &offset);

	//Cette fonction va permetre de mettre un pointeur afin de divier le fichier en bloc
	MPI_File_set_view(fh, 0, MPI_CHAR, MPI_CHAR, "native", MPI_INFO_NULL);

	//Tester si la taille du fichier est <= 8 => on envoi tout le contenu du fichier à un seul serveur
	if(offset <= 8){
		if(rank==0)
		{
			MPI_File_read(fh, buffer, offset, MPI_CHAR, MPI_STATUS_IGNORE);
			MPI_Send(buffer, offset, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
		}
		MPI_File_set_view(fh, offset, MPI_CHAR, MPI_CHAR, "native", MPI_INFO_NULL);
		if(rank == 1) {
			MPI_Recv(buffer, offset, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			printf("Je suis %d et j'ai reçu %s\n", rank, buffer);
		}
	//Sinon, il faut diviser la taille du fichier par 8 et envoyer le fichier par tranche a nbBlocs serveurs
	} else {
		nbrBlocs = offset/8;
		for(i=1; i<=nbrBlocs; i++){
			if(rank==0)
			{
				MPI_File_read(fh, buffer, 8, MPI_CHAR, MPI_STATUS_IGNORE);
				MPI_Send(buffer, 8, MPI_CHAR, i, 0, MPI_COMM_WORLD);
			}
			MPI_File_set_view(fh, i*8, MPI_CHAR, MPI_CHAR, "native", MPI_INFO_NULL);

		/*if(i >nbrBlocs)
			i = 0;
		*/
		}
		if(rank != 0) {
			MPI_Recv(buffer, 8, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			printf("Je suis %d et j'ai reçu %s\n", rank, buffer);
		}
	}
 }

	void sendInfoToLB(){
		MPI_File fh;
	    MPI_Offset offset, offsetRecv;
		MPI_Request request;

		//Le rang des ps
		int rank;
   		MPI_Comm_rank(MPI_COMM_WORLD, &rank);

		 char fileName[FILENAME_LENGTH] = "test.txt";
		//fileName = "test.txt";

		char fileNameRecv [FILENAME_LENGTH];
		//Ouverture du fichier
    	MPI_File_open(MPI_COMM_WORLD, fileName, MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
	
		//La taille du fichier
    	MPI_File_get_size(fh, &offset);

		if( rank == 0){
			MPI_Isend(&offset, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &request);
			MPI_Wait ( &request, MPI_STATUS_IGNORE );
	
			MPI_Isend(fileName, FILENAME_LENGTH, MPI_CHAR, 1, 0, MPI_COMM_WORLD, &request);
			MPI_Wait ( &request, MPI_STATUS_IGNORE );
		}else if (rank == 1){
			MPI_Irecv(&offsetRecv, 1, MPI_INT, 0, 0, MPI_COMM_WORLD,&request);
			MPI_Wait ( &request, MPI_STATUS_IGNORE );
			printf("Je suis %d et j'ai reçu %lld\n", rank, offsetRecv);
		
			MPI_Irecv(fileNameRecv, FILENAME_LENGTH, MPI_CHAR, 0, 0, MPI_COMM_WORLD,&request);
			MPI_Wait ( &request, MPI_STATUS_IGNORE );			
			printf("Je suis %d et j'ai reçu %s\n", rank, fileNameRecv);
		}
	}
