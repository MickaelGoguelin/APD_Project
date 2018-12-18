#include <mpi.h>
#ifndef DSFM_FCT_H
#define DSFM_FCT_H
#define ROOT_DIR "/tmp/AIR3_G5_APD/"
#define FILENAME_LENGTH 100
#define SIZE_BUFFER 8
#define FILENAME "test.txt"
#define FILENAME_GET "test1.txt" 
#define CUSTOMER 0
#define LB 1
#define SERVER_1 2
#define SERVER_2 3
#define SERVER_3 4
#define MAX_MACHINES 4
#define FIRST_SERVER 2

/*
 * Définition des fonctions
*/
	void sendBlocToServers(char fileName[FILENAME_LENGTH]);
	void sendInfoToLB(MPI_File fh, int rank, MPI_Offset offset, char fileName[FILENAME_LENGTH]);
	int roundRobbin(int server);
	void writeFileToDisk(const char* nomFichier, char* buffer, int sizeExactOfBuffer);
	/* cette fonction ecrit un fichier sur disque. si le fichier n'existe pas, il le cree. Sinon il rajoute buffer à la fin du fichier.
	 * param nomFichier : cette chaine contient le chemin vers le repertoir où sera stocké le   fichier et le nom du fichier ex: ./APD/toto.txt
	 *param buffer : les données qui vont etre ecrite
	 */
 	//void writeFileToDisk1(const char* nomFichier, char* buffer);
	void readBloc(const char* nom_fichier, char* bloc, int* curseur, int* status, int sizeExactOfBuffer);
	void readFileFromDisk(const char* nom_fichier, char* bloc,  int* status);
	void displayBlocs(const char* bloc);
	int assertHostfile();
	void get(int nbrBlocs, int firstServer, char bloc[SIZE_BUFFER]);
	void putBlocInServer(int sizeLastBLoc, int i, int server);
	int getCountBlocs(char fileName[FILENAME_LENGTH]);

#endif
