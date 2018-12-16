#include "dsfm_fct.h"
#include <unistd.h>
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
    char fileName[FILENAME_LENGTH] = FILENAME;

	//Le tableau qui va contenir les morceaux du fichiers ( la taille du bloc est de 4096 octets)
	char buffer[TAILLE_BUFFER];

	//Le rang des ps
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	//Le nombre de ps
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);

	//Ouverture du fichier
    MPI_File_open(MPI_COMM_WORLD, fileName, MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
	
	//La taille du fichier
    MPI_File_get_size(fh, &offset);

	//Tester si la taille du fichier est <= TAILLE_BUFFER => on envoi tout le contenu du fichier à un seul serveur
	if(offset <= TAILLE_BUFFER){
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
	//Sinon, il faut diviser la taille du fichier par TAILLE_BUFFER et envoyer le fichier par tranche a nbBlocs serveurs
	} else {
		nbrBlocs = offset/TAILLE_BUFFER;
		int server = 1;
		char bufferRecv[TAILLE_BUFFER];
		for(i=0; i<=nbrBlocs; i++){	
			//Cette fonction va permetre de mettre un pointeur afin de divier le fichier en bloc
			MPI_File_set_view(fh, i*TAILLE_BUFFER, MPI_CHAR, MPI_CHAR, "native", MPI_INFO_NULL);

			server = roundRobbin(server);	
			if(rank==0)
			{
				MPI_File_read(fh, buffer, TAILLE_BUFFER - 1, MPI_CHAR, MPI_STATUS_IGNORE);
				//printf("buffer before send %s\n",buffer);
				MPI_Send(buffer, TAILLE_BUFFER, MPI_CHAR, server, 0, MPI_COMM_WORLD);
				printf("Je suis %d et j'ai envoye %s au serveur %d\n", rank, buffer,server);
			}		
			if(rank == server) {
				MPI_Recv(bufferRecv, TAILLE_BUFFER, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				printf("Je suis %d et j'ai reçu %s\n", rank, bufferRecv);
			}
		}
	}
	MPI_File_close(&fh);
 }

	int roundRobbin(int server){
		int nbrProcs;
		//Le nombre de ps
		MPI_Comm_size(MPI_COMM_WORLD, &nbrProcs);

		server = (server + 1)%nbrProcs;
		if(server == 0)
			server ++;
		return server;
	}

	void sendInfoToLB(){
		MPI_File fh;
	    MPI_Offset offset, offsetRecv, nbrBlocs, nbrBlocsRecv;
		MPI_Request request;

		//Le rang des ps
		int rank;
   		MPI_Comm_rank(MPI_COMM_WORLD, &rank);

		char fileName[FILENAME_LENGTH] = FILENAME;

		char fileNameRecv [FILENAME_LENGTH];
		//Ouverture du fichier
    	MPI_File_open(MPI_COMM_WORLD, fileName, MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
	
		//La taille du fichier
    	MPI_File_get_size(fh, &offset);
		nbrBlocs = offset/TAILLE_BUFFER;
		offsetRecv = 0;

		if( rank == 0){
			MPI_Isend(&offset, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &request);
			MPI_Wait (&request, MPI_STATUS_IGNORE );
			//printf("Je suis %d et j'ai envoye un fichier de taille %lld \n", rank,offset);			
	
			MPI_Isend(fileName, FILENAME_LENGTH, MPI_CHAR, 1, 0, MPI_COMM_WORLD, &request);
			MPI_Wait (&request, MPI_STATUS_IGNORE );

			MPI_Isend(&nbrBlocs, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &request);
			MPI_Wait ( &request, MPI_STATUS_IGNORE );
			//printf("Je suis %d et j'ai envoye %lld blocs\n", rank, nbrBlocs);
		}else if (rank == 1){
			MPI_Irecv(&offsetRecv, 1, MPI_INT, 0, 0, MPI_COMM_WORLD,&request);
			MPI_Wait (&request, MPI_STATUS_IGNORE );
			printf("Je suis %d et j'ai reçu un fichier de taille %lld \n", rank, offsetRecv);
		
			MPI_Irecv(fileNameRecv, FILENAME_LENGTH, MPI_CHAR, 0, 0, MPI_COMM_WORLD,&request);
			MPI_Wait ( &request, MPI_STATUS_IGNORE );			
			printf("Je suis %d et j'ai reçu un fichier nommé %s\n", rank, fileNameRecv);

			MPI_Irecv(&nbrBlocsRecv, 1, MPI_INT, 0, 0, MPI_COMM_WORLD,&request);
			MPI_Wait (&request, MPI_STATUS_IGNORE );			
			printf("Je suis %d et j'ai reçu %lld blocs\n", rank, nbrBlocsRecv);
		}
	}

/*void writeFileToDisk1(const char* nomFichier, char* buffer)
{
    int res_fprintf;
    FILE *fichier = fopen(nomFichier,"a");
    if(fichier == NULL)
        printf("Erreur dans la creation du fichier, fonction ecrire_fichier_sur_disque\n");
    else{
        res_fprintf = fprintf(fichier,buffer);
        if(res_fprintf < 0){
            printf("Erreur lors de l'écriture du fichier sur disque, fonction ecrire_fichier_sur_disque\n");
        }
    }
    
    fclose(fichier);
}*/

void writeFileToDisk(const char* nomFichier, char* buffer)
{
    int res_fputc;
    FILE *fichier = fopen(nomFichier,"a");
    if(fichier == NULL)
        printf("Erreur dans la creation du fichier, fonction ecrire_fichier_sur_disque\n");
    
    else{
        int i = 0;
        do{
            if(buffer[i] != EOF) // on fait ce test au cas où le bloc n'est pas rempli au max car c'est le dernier bloc du fichier. on n'ecrit pas le caractère EOF.
                res_fputc = fputc(buffer[i],fichier); 
            else
                res_fputc = EOF; // on n'arrete d'écrire le bloc dans le fichier dès qu'on lit EOF
            if(res_fputc == EOF){ // on test si l'écriture sur le fichier c'est bien passée sinon on sort de la boucle et on affiche le message d'erreur suivant
                printf("Erreur lors de l'écriture du fichier sur disque ou ecriture d'un bloc pas rempli au max, fonction ecrire_fichier_sur_disque\n");
            }
            i++;
        }
        while(res_fputc != EOF && i<TAILLE_BUFFER);
    }
    
    fclose(fichier);
}


 void readBloc(const char* nom_fichier, char* bloc, int* curseur, int* status)
 {
     
     FILE* fichier = fopen(nom_fichier,"r");
     if (fichier == NULL){
         printf("Erreur dans la lecture du fichier, celui-ci est absent, fonction lire_bloc\n");
        *status = -1;
        bloc = NULL;
     }
     int nb_char_lu = 0;
     char char_lu;
     if(fseek(fichier, *curseur, SEEK_SET)){
         printf("Erreur lors du deplacement du curseur dans le fichier, fonction lire_bloc}\n");
         *status = -1;
     }
         
     do{
        char_lu=fgetc(fichier);
        //if(char_lu != EOF)
            bloc[nb_char_lu] = char_lu;
        nb_char_lu++;
     }
     while( (nb_char_lu < TAILLE_BUFFER) && (char_lu != EOF) );
     *curseur = *curseur + nb_char_lu;
     if(char_lu == EOF)
         *status = 1;
     else
        *status = 0;
     
     fclose(fichier);
 }
 
 void readFileFromDisk(const char* nom_fichier, char* bloc,  int* status)
 {
    int status_lire_bloc;
    int curseur=0;
     do{
        readBloc(nom_fichier,bloc,&curseur,&status_lire_bloc);
        if(bloc != NULL )
            displayBlocs(bloc);
        printf("status_lire_bloc: %d\n", status_lire_bloc);
    }
    while( status_lire_bloc == 0 );
    
    *status = status_lire_bloc;
    
    
     
 }
 
 void displayBlocs(const char* bloc)
 {
     printf("Bloc vaut: ");//
    for(int i=0; i<TAILLE_BUFFER; i++)
    {
         printf("%c" , bloc[i]);
    }
    printf("\n");
}




