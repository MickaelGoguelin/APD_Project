#include "dsfm_fct.h"
#include <unistd.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

void put(char fileName[FILENAME_LENGTH]){

	int numprocs, rank, i;
	int nbrBlocs;

    MPI_File fh;
    MPI_Offset offset;

	int sizeLastBloc = 0;

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

	//Nombre de blocs
	nbrBlocs = (offset/SIZE_BUFFER);
	
	//Appel de la fonction qui permet d'envoyer les information qui correspondent au fichier au serveur LB
	sendInfoToLB(nbrBlocs, offset, FILENAME);

	//Tester si la taille du fichier est <= SIZE_BUFFER => on envoi tout le contenu du fichier à un seul serveur

		int server = getFirstServerFromLB();
		char bufferRecv[SIZE_BUFFER];
		char str[FILENAME_LENGTH];
		char strFileName[200] = "";
		//int l = strlen(ROOT_DIR);
		mkdir(ROOT_DIR,0777);
		for(i=1; i<MAX_MACHINES; i++){
			if(rank == i){
				sprintf(str, "%d", i);
				strcat(strFileName,ROOT_DIR);
				//Attention !! il faut changer la ligne suivante avec la ligne en commentaire pour faire la distribution des fichiers dans
				//les machines de l ecole !!!!!!!!!
				//strcat(strFileName,FILENAME);				
				strcat(strFileName,str);
				//printf("file directory 1 %s\n",strFileName);
			}
		}
		for(i=0; i<nbrBlocs; i++){	
			//Cette fonction va permetre de mettre un pointeur afin de diviser le fichier en bloc
			MPI_File_set_view(fh, i*SIZE_BUFFER, MPI_CHAR, MPI_CHAR, "native", MPI_INFO_NULL);
			//cette fonction permet d'incrementer les serveurs en tourniquet
			//server = roundRobbin(server);	
			MPI_Barrier(MPI_COMM_WORLD);
			if(rank==CUSTOMER) {
				MPI_File_read(fh, buffer, SIZE_BUFFER, MPI_CHAR, MPI_STATUS_IGNORE);

				MPI_Send(buffer, SIZE_BUFFER, MPI_CHAR, server, 0, MPI_COMM_WORLD);
				//printf("Je suis %d et j'ai envoye au serveur %d : \n", rank,server);
			}		
			if(rank == server) {
				MPI_Recv(bufferRecv, SIZE_BUFFER, MPI_CHAR, CUSTOMER, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				//Inserer dans chaque serveur des fichiers qui contiennent le contenu des blocs
				//sprintf(str, "%d", server);
				//printf("file directory 2 %s\n",strFileName);
				writeFileToDisk(strFileName, bufferRecv,SIZE_BUFFER);
				//printf("Je suis %d et j'ai reçu %s\n", rank, bufferRecv);
			}
			server = roundRobbin(server);	
		}
		if(offset%SIZE_BUFFER != 0){
		//Retourner la taille du dernier bloc (il peut être inférieur à la taille des blocs fixés )
			sizeLastBloc = offset - (SIZE_BUFFER*nbrBlocs);

			if(sizeLastBloc != 0 ){
			char bufferRecv2[sizeLastBloc];
			//Cette fonction va permetre de mettre un pointeur afin de divier le fichier en bloc
				MPI_File_set_view(fh, i*SIZE_BUFFER, MPI_CHAR, MPI_CHAR, "native", MPI_INFO_NULL);

				//server = roundRobbin(server);	
				if(rank==CUSTOMER)
				{
					MPI_File_read(fh, buffer, sizeLastBloc, MPI_CHAR, MPI_STATUS_IGNORE);
					MPI_Send(buffer, sizeLastBloc, MPI_CHAR, server, 0, MPI_COMM_WORLD);
					//printf("Je suis %d et j'ai envoye au serveur %d : \n", rank,server);
				}		
				if(rank == server) {
					MPI_Recv(bufferRecv2, sizeLastBloc, MPI_CHAR, CUSTOMER, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
					//Inserer dans chaque serveur des fichiers qui contiennent le contenu des blocs
					//sprintf(str, "%d", server);
					//printf("file directory 3 %s\n",strFileName);
					writeFileToDisk(strFileName, bufferRecv2, sizeLastBloc);
					//printf("Je suis %d et j'ai reçu %s\n", rank, bufferRecv);
				}
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
//Le client (rang = 0) envoie le nombre de blocs, la taille du fichier et le nom du fichier au serveur LB ( rang = 1)
void sendInfoToLB(int nbrBlocs, MPI_Offset offset, char fileName[FILENAME_LENGTH]){
	MPI_Offset offsetRecv;
	MPI_Request request;
	int rank, nbrBlocsRecv;
	//Le rang des ps
   	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	char fileNameRecv [FILENAME_LENGTH];
	offsetRecv = 0;

	if( rank == CUSTOMER){
		MPI_Isend(&offset, 1, MPI_INT, LB, 0, MPI_COMM_WORLD, &request);
		MPI_Wait (&request, MPI_STATUS_IGNORE );
	
		MPI_Isend(fileName, FILENAME_LENGTH, MPI_CHAR, LB, 0, MPI_COMM_WORLD, &request);
		MPI_Wait (&request, MPI_STATUS_IGNORE );

		MPI_Isend(&nbrBlocs, 1, MPI_INT, LB, 0, MPI_COMM_WORLD, &request);
		MPI_Wait ( &request, MPI_STATUS_IGNORE );
	}else if (rank == LB){
		MPI_Irecv(&offsetRecv, 1, MPI_INT, CUSTOMER, 0, MPI_COMM_WORLD,&request);
		MPI_Wait (&request, MPI_STATUS_IGNORE );
		//printf("Je suis %d et j'ai reçu un fichier de taille %lld \n", rank, offsetRecv);
		
		MPI_Irecv(fileNameRecv, FILENAME_LENGTH, MPI_CHAR, CUSTOMER, 0, MPI_COMM_WORLD,&request);
		MPI_Wait ( &request, MPI_STATUS_IGNORE );			
		//printf("Je suis %d et j'ai reçu un fichier nommé %s\n", rank, fileNameRecv);

		MPI_Irecv(&nbrBlocsRecv, 1, MPI_INT, CUSTOMER, 0, MPI_COMM_WORLD,&request);
		MPI_Wait (&request, MPI_STATUS_IGNORE );			
		//printf("Je suis %d et j'ai reçu %d blocs\n", rank, nbrBlocsRecv);
	}
}

//Cette fonction va permettre au serveur LB d'envoyer au client l'indice du premier serveur où va être stocké le premier bloc du fichier
int getFirstServerFromLB(){
	/*int rank, firstServerRecv, firstServerSend;
	firstServerSend = SERVER_1;
	MPI_Request request;

	//Le rang des ps
   	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	if(rank == LB){
		MPI_Isend(&firstServerSend, 1, MPI_INT, CUSTOMER, 0, MPI_COMM_WORLD,&request);
		MPI_Wait (&request, MPI_STATUS_IGNORE );
	}else if( rank == CUSTOMER){
		MPI_Irecv(&firstServerRecv, 1, MPI_INT, LB, 0, MPI_COMM_WORLD, &request);
		MPI_Wait (&request, MPI_STATUS_IGNORE );
	}*/
return SERVER_1;
}

void get(char fileName[FILENAME_LENGTH]){
	char str[FILENAME_LENGTH];
	char bloc[SIZE_BUFFER];
	int status, rank, i, curseur, nbrBlocs, firstServer;
	//A recupérer de la chaine
	firstServer = FIRST_SERVER;
	int server = firstServer;
	//A recupérer de la chaine
	int sizeFile = 34;
	
	//A recupérer de la chaine
	//Nombre de blocs
	nbrBlocs = 5;
	//Le rang des ps
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
		
	int j = 0;
	int sizeBuffer = SIZE_BUFFER;
	char strFileName[200] = "";
	for(i=1; i<MAX_MACHINES; i++){
		if(rank == i){
			sprintf(str, "%d", i);
			strcat(strFileName,ROOT_DIR);
			//Attention !! il faut changer la ligne suivante avec la ligne en commentaire pour faire la distribution des fichiers dans
			//les machines de l ecole !!!!!!!!!
			//strcat(strFileName,FILENAME);
			strcat(strFileName,str);
			//printf("file directory 1 %s\n",strFileName);
		}
	}
	for(i=0; i<nbrBlocs; i++){	
		//sprintf(str, "%d", server);
		char blocRecv[sizeBuffer];
		MPI_Request request;
		if(i == nbrBlocs-1 && sizeFile%SIZE_BUFFER != 0){
			sizeBuffer = sizeFile%SIZE_BUFFER;
		}
		if(rank == server){
				//Ce curseur va permettre de positionner un pointeur, soit à 0 si c'est le début du fichier soit à la position du second bloc
				curseur = SIZE_BUFFER * j;
				//Ce if permet de vérifier si on atteint le dernier bloc qui contient par exemple que 2 caractères, de retourner seulement 2 					et pas SIZE_BUFFER
				readBloc(strFileName,bloc, &curseur, &status, sizeBuffer);
				MPI_Isend(bloc, sizeBuffer, MPI_CHAR, CUSTOMER, 0, MPI_COMM_WORLD, &request);
				MPI_Wait (&request, MPI_STATUS_IGNORE );			
				//printf("Je suis %d et j'ai envoye %s au serveur %d\n", rank, bloc,server);
		}else if(rank==CUSTOMER){
				MPI_Irecv(blocRecv, sizeBuffer, MPI_CHAR, server, 0, MPI_COMM_WORLD, &request);
				MPI_Wait (&request, MPI_STATUS_IGNORE );	
				//printf("Je suis %d et j'ai recu %s du serveur %d\n", rank, blocRecv,server);	
				writeFileToDisk(FILENAME_GET,blocRecv, sizeBuffer);	
		}
			MPI_Barrier(MPI_COMM_WORLD);
			server = roundRobbin(server);
			MPI_Barrier(MPI_COMM_WORLD);
			if(server == firstServer){
				j++;
			}
	}
}

//Test en attendant la table de référence
int getCountBlocs(char fileName[FILENAME_LENGTH]){
	MPI_File fh;
    MPI_Offset offset;
	MPI_File_open(MPI_COMM_WORLD, fileName, MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
	MPI_File_get_size(fh, &offset);

	return offset/SIZE_BUFFER;
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
                /*printf("Erreur lors de l'écriture du fichier sur disque ou ecriture d'un bloc pas rempli au max, fonction ecrire_fichier_sur_disque\n");*/
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

int insertToDictionnary(char* nom_fichier, int nbrB, int tailleF, int roundR){
    
   	 FILE* fichier;
    	fichier = fopen("Dictionnaire.txt", "a");
    
    	if( fichier == NULL){
        	printf("Erreur dans la creation ou l'ouverture du fichier dictionnaire.txt\n");
        	return 0;
    	}
    
    	if( 0 > fprintf(fichier,"%s %d %d %d\n", nom_fichier, nbrB, tailleF, roundR)){
        	printf("Erreur dans l'insertion d'une ligne dans le dictionnaire\n");
		return 0;
	}
    	return 1;
}

int loadDictionnary(){

	FILE * dict = fopen("Dictionnaire.txt", "r");
	char file_name[FILENAME_LENGTH];
	int nbr_blocs, taille_fichier, round_robin;
	if(dict == NULL)
		exit(EXIT_FAILURE);
	while(4 == fscanf(dict,"%s %d %d %d\n",file_name,&nbr_blocs, &taille_fichier,&round_robin)){
		printf("%s %d %d %d\n", file_name, nbr_blocs, taille_fichier, round_robin);
	}

	if(fclose(dict) != 0){
		printf("Fermeture du dictionnaire a echoue\n");
		exit(EXIT_FAILURE);
	}
	dict = fopen("Dictionnaire.txt", "w");
	if( dict == NULL){
		printf("Ouverture du fichier Dictionnaire pour effacer contenu a echoue\n");
		exit(EXIT_FAILURE);
	}
	
	if(fclose(dict) != 0){
		printf("Fermeture du dictionnaire a echoue\n");
		exit(EXIT_FAILURE);
	}

	return 0;
}

