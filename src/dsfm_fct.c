#include "dsfm_fct.h"
#include <unistd.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

void sendFileToServers(){

	int numprocs, rank, i;
	int nbrBlocs;

    MPI_File fh;
    MPI_Offset offset;

	int sizeLastBloc = 0;

	//Nom du fichier
    char fileName[FILENAME_LENGTH] = FILENAME;

	//Le tableau qui va contenir les morceaux du fichiers ( la taille du bloc est de 4096 octets)
	char buffer [SIZE_BUFFER];

	//Le rang des ps
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	//Le nombre de ps
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);

	//Ouverture du fichier
    MPI_File_open(MPI_COMM_WORLD, fileName, MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
	
	//La taille du fichier
    MPI_File_get_size(fh, &offset);

	//Tester si la taille du fichier est <= SIZE_BUFFER => on envoi tout le contenu du fichier à un seul serveur
	if(offset <= SIZE_BUFFER){
		if(rank==CUSTOMER)
		{
			MPI_File_read(fh, buffer, offset, MPI_CHAR, MPI_STATUS_IGNORE);
			MPI_Send(buffer, offset, MPI_CHAR, LB, 0, MPI_COMM_WORLD);
		}
		MPI_File_set_view(fh, offset, MPI_CHAR, MPI_CHAR, "native", MPI_INFO_NULL);
		if(rank == LB) {
			MPI_Recv(buffer, offset, MPI_CHAR, CUSTOMER, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			printf("Je suis %d et j'ai reçu %s\n", rank, buffer);
		}
	//Sinon, il faut diviser la taille du fichier par SIZE_BUFFER et envoyer le fichier par tranche a nbBlocs serveurs
	} else {
		nbrBlocs = (offset/SIZE_BUFFER);
		int server = LB;
		char bufferRecv[SIZE_BUFFER];
		char str[FILENAME_LENGTH];
		//char a[5]="bloc";
		for(i=0; i<nbrBlocs; i++){	
			//Cette fonction va permetre de mettre un pointeur afin de divier le fichier en bloc
			MPI_File_set_view(fh, i*SIZE_BUFFER, MPI_CHAR, MPI_CHAR, "native", MPI_INFO_NULL);

			server = roundRobbin(server);	
			MPI_Barrier(MPI_COMM_WORLD);
			if(rank==CUSTOMER)
			{
				MPI_File_read(fh, buffer, SIZE_BUFFER, MPI_CHAR, MPI_STATUS_IGNORE);
				//buffer[SIZE_BUFFER]='\0';
				//printf("buffer before send %s\n",buffer);
				MPI_Send(buffer, SIZE_BUFFER, MPI_CHAR, server, 0, MPI_COMM_WORLD);
				//printf("Je suis %d et j'ai envoye %s au serveur %d\n", rank, buffer,server);
				printf("Je suis %d et j'ai envoye au serveur %d : \n", rank,server);
				//for(int j=0; j<SIZE_BUFFER; j++) printf("%c", buffer[j]);
				//printf("\n");
			}		
			if(rank == server) {
				MPI_Recv(bufferRecv, SIZE_BUFFER, MPI_CHAR, CUSTOMER, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				//Inserer dans chaque serveur des fichiers qui contiennent le contenu des blocs
				sprintf(str, "%d", server);
				writeFileToDisk(str, bufferRecv,SIZE_BUFFER);
				printf("Je suis %d et j'ai reçu %s\n", rank, bufferRecv);
			}
		}

		//A mettre dans une fct !!!!!!!!!!!
			sizeLastBloc = offset - (SIZE_BUFFER*nbrBlocs);

			if(sizeLastBloc != 0 ){
			char bufferRecv2[sizeLastBloc];
			//Cette fonction va permetre de mettre un pointeur afin de divier le fichier en bloc
				MPI_File_set_view(fh, i*SIZE_BUFFER, MPI_CHAR, MPI_CHAR, "native", MPI_INFO_NULL);

				server = roundRobbin(server);	
				if(rank==CUSTOMER)
				{
					MPI_File_read(fh, buffer, sizeLastBloc, MPI_CHAR, MPI_STATUS_IGNORE);
					//buffer[SIZE_BUFFER]='\0';
					//printf("buffer before send %s\n",buffer);
					MPI_Send(buffer, sizeLastBloc, MPI_CHAR, server, 0, MPI_COMM_WORLD);
					//printf("Je suis %d et j'ai envoye %s au serveur %d\n", rank, buffer,server);
					printf("Je suis %d et j'ai envoye au serveur %d : \n", rank,server);
					/*for(int j=0; j<sizeLastBloc; j++) {
						printf("chaaaaaaaar%c", buffer[j]);
						if( buffer[j] =="\0") printf("ennnnnnnd \n");
					}
					printf("\n");*/
				}		
				if(rank == server) {
					MPI_Recv(bufferRecv2, sizeLastBloc, MPI_CHAR, CUSTOMER, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
					//Inserer dans chaque serveur des fichiers qui contiennent le contenu des blocs
					sprintf(str, "%d", server);
					writeFileToDisk(str, bufferRecv2, sizeLastBloc);
					printf("Je suis %d et j'ai reçu %s\n", rank, bufferRecv);
				}

			//putBlocInServer(sizeLastBloc, nbrBlocs, server);
		}
	}
	MPI_File_close(&fh);
 }

int roundRobbin(int server){
    server = (server + 1)%MAX_MACHINES;
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
		nbrBlocs = offset/SIZE_BUFFER;
		offsetRecv = 0;

		if( rank == CUSTOMER){
			MPI_Isend(&offset, 1, MPI_INT, LB, 0, MPI_COMM_WORLD, &request);
			MPI_Wait (&request, MPI_STATUS_IGNORE );
			//printf("Je suis %d et j'ai envoye un fichier de taille %lld \n", rank,offset);			
	
			MPI_Isend(fileName, FILENAME_LENGTH, MPI_CHAR, LB, 0, MPI_COMM_WORLD, &request);
			MPI_Wait (&request, MPI_STATUS_IGNORE );

			MPI_Isend(&nbrBlocs, 1, MPI_INT, LB, 0, MPI_COMM_WORLD, &request);
			MPI_Wait ( &request, MPI_STATUS_IGNORE );
			//printf("Je suis %d et j'ai envoye %lld blocs\n", rank, nbrBlocs);
		}else if (rank == LB){
			MPI_Irecv(&offsetRecv, 1, MPI_INT, CUSTOMER, 0, MPI_COMM_WORLD,&request);
			MPI_Wait (&request, MPI_STATUS_IGNORE );
			printf("Je suis %d et j'ai reçu un fichier de taille %lld \n", rank, offsetRecv);
		
			MPI_Irecv(fileNameRecv, FILENAME_LENGTH, MPI_CHAR, CUSTOMER, 0, MPI_COMM_WORLD,&request);
			MPI_Wait ( &request, MPI_STATUS_IGNORE );			
			printf("Je suis %d et j'ai reçu un fichier nommé %s\n", rank, fileNameRecv);

			MPI_Irecv(&nbrBlocsRecv, 1, MPI_INT, CUSTOMER, 0, MPI_COMM_WORLD,&request);
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

void writeFileToDisk(const char* nomFichier, char* buffer, int sizeExactOfBuffer)
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
        while(res_fputc != EOF && i<sizeExactOfBuffer);
    }
    
    fclose(fichier);
}


 void readBloc(const char* nom_fichier, char* bloc, int* curseur, int* status, int sizeExactOfBuffer)
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
     while( (nb_char_lu < sizeExactOfBuffer) && (char_lu != EOF) );
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
        readBloc(nom_fichier,bloc,&curseur,&status_lire_bloc, SIZE_BUFFER);
        if(bloc != NULL )
            displayBlocs(bloc);
        //printf("status_lire_bloc: %d\n", status_lire_bloc);
    }
    while( status_lire_bloc == 0 );
    
    *status = status_lire_bloc;
 }
 
void displayBlocs(const char* bloc)
 {
     printf("Bloc vaut: ");
    for(int i=0; i<SIZE_BUFFER; i++)
    {
         printf("%c" , bloc[i]);
    }
    printf("\n");
}

int assertHostfile(){
    int kmax, i=0;
    FILE * f;
    char hostfile[] = "localhost slots=1\n\
g207-1 slots=1\n\
g207-2 slots=1\n\
g207-3 slots=1\n\
g207-5 slots=1\n";
    char * tmp;
    char c;
    struct stat s;
    
    MPI_Comm_size(MPI_COMM_WORLD, &kmax);
    if(kmax != MAX_MACHINES)
	return 0;
    else {
	f = fopen("hostf_univ", "r");
	stat("hostf_univ", &s);
	if(f == NULL) return 0;
	tmp = (char*) malloc(s.st_size*sizeof(char));
	c=(char)fgetc(f);
	while(c!=EOF&&i<s.st_size){
	    tmp[i]=c;
	    c=(char)fgetc(f);
	    i++;
	}
	if(strcmp(hostfile, tmp)) return 0;
	fclose(f);
	free(tmp);
    }
    return 1;
}

void putBlocInServer(int sizeLastBloc, int i, int server ){
	char bufferRecv[sizeLastBloc];
	char buffer[sizeLastBloc];
	int numprocs, rank;
	//int nbrBlocs;
	MPI_Offset offset;
	MPI_File fh;
	char str[FILENAME_LENGTH];
	//Le tableau qui va contenir les morceaux du fichiers ( la taille du bloc est de 4096 octets)
	//char buffer [SIZE_BUFFER];

	//Le rang des ps
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	//Le nombre de ps
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);

	//Nom du fichier
    char fileName[FILENAME_LENGTH] = FILENAME;

	//Ouverture du fichier
    MPI_File_open(MPI_COMM_WORLD, fileName, MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
	
	//La taille du fichier
    MPI_File_get_size(fh, &offset);

	//Cette fonction va permetre de mettre un pointeur afin de divier le fichier en bloc
	MPI_File_set_view(fh, i*SIZE_BUFFER, MPI_CHAR, MPI_CHAR, "native", MPI_INFO_NULL);
	//int server = LB;
	server = roundRobbin(server);	
	if(rank==CUSTOMER)
	{
		MPI_File_read(fh, buffer, sizeLastBloc, MPI_CHAR, MPI_STATUS_IGNORE);
		//buffer[SIZE_BUFFER]='\0';
		//printf("buffer before send %s\n",buffer);
		MPI_Send(buffer, sizeLastBloc, MPI_CHAR, server, 0, MPI_COMM_WORLD);
		//printf("Je suis %d et j'ai envoye %s au serveur %d\n", rank, buffer,server);
		printf("Je suis %d et j'ai envoye au serveur %d : \n", rank,server);
		for(int j=0; j<SIZE_BUFFER; j++) printf("%c", buffer[j]);
		printf("\n");
	}		
	if(rank == server) {
	 	MPI_Recv(bufferRecv, sizeLastBloc, MPI_CHAR, CUSTOMER, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		//Inserer dans chaque serveur des fichiers qui contiennent le contenu des blocs
		sprintf(str, "%d", server);
		writeFileToDisk(str, bufferRecv, sizeLastBloc);
		printf("Je suis %d et j'ai reçu %s\n", rank, bufferRecv);
	}
}


void getBlocsFromServers(int nbrBlocs, int firstServer,  char bloc[SIZE_BUFFER]){
	//char bloc [SIZE_BUFFER];
	char str[FILENAME_LENGTH];
	int status, rank, i, curseur;
	int server = firstServer;
	//Le rang des ps
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	int j = 0;
	int sizeBuffer = SIZE_BUFFER;
	for(i=0; i<nbrBlocs; i++){	
		sprintf(str, "%d", server);
		//printf("Server %s \n",str);
		char blocRecv[sizeBuffer];
		MPI_Request request;
		if(rank == server){
				//Ce curseur va permettre de positionner un pointeur, soit à 0 si c'est le début du fichier soit à la position du second bloc
				curseur = SIZE_BUFFER * j;
				//Ce if permet de vérifier si on atteint le dernier bloc qui contient par exemple que 2 caractères, de retourner seulement 2 					et pas SIZE_BUFFER
				if(i == nbrBlocs )
					{
						char fileName[FILENAME_LENGTH] = FILENAME;
						MPI_File fh;
						MPI_File_open(MPI_COMM_WORLD, fileName, MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
							
						//La taille du fichier
						MPI_Offset offset;
						MPI_File_get_size(fh, &offset);
						//size buffer permet de recupérer la taille du dernier fichier insérer afin de ne pas insérer des caratères spéciaux à 							la fin du fichier s'il ne contient pas SIZE_BUFFER caractères
						sizeBuffer = offset % SIZE_BUFFER;
					}
				readBloc(str,bloc, &curseur, &status, sizeBuffer);
				MPI_Isend(bloc, sizeBuffer, MPI_CHAR, CUSTOMER, 0, MPI_COMM_WORLD, &request);
				MPI_Wait (&request, MPI_STATUS_IGNORE );			
				printf("Je suis %d et j'ai envoye %s au serveur %d\n", rank, bloc,server);

					/*for(int j=0; j<sizeBuffer; j++) {
						printf("blooc  %c\n", bloc[j]);}*/
				//writeFileToDisk("test1.txt",bloc,sizeBuffer );
		}else if(rank==CUSTOMER){
				MPI_Irecv(blocRecv, sizeBuffer, MPI_CHAR, server, 0, MPI_COMM_WORLD, &request);
				MPI_Wait (&request, MPI_STATUS_IGNORE );	
				printf("Je suis %d et j'ai recu %s du serveur %d\n", rank, blocRecv,server);	
				writeFileToDisk("test1.txt",blocRecv, sizeBuffer);	
		}
			MPI_Barrier(MPI_COMM_WORLD);
			server = roundRobbin(server);
			MPI_Barrier(MPI_COMM_WORLD);
			//printf("Servers %d \n",server);
			if(server == firstServer){
				j++;
			}
	}
}
