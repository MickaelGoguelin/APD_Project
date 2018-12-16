#ifndef DSFM_FCT_H
#define DSFM_FCT_H
#define ROOT_DIR "/tmp/AIR3_G5_APD/"
#define FILENAME_LENGTH 100
#define TAILLE_NOM_FICHIER_MAX 100
#define TAILLE_BUFFER 8
#define FILENAME "test.txt" 




/*
 * Définition des fonctions
*/
	void sendFileToServers();
	void sendInfoToLB();
	int roundRobbin(int server);
	void writeFileToDisk(const char* nomFichier, char* buffer);
	/* cette fonction ecrit un fichier sur disque. si le fichier n'existe pas, il le cree. Sinon il rajoute buffer à la fin du fichier.
	 * param nomFichier : cette chaine contient le chemin vers le repertoir où sera stocké le   fichier et le nom du fichier ex: ./APD/toto.txt
	 *param buffer : les données qui vont etre ecrite
	 */
	 
 	//void writeFileToDisk1(const char* nomFichier, char* buffer);
	void readBloc(const char* nom_fichier, char* bloc, int* curseur, int* status);
	void readFileFromDisk(const char* nom_fichier, char* bloc,  int* status);
	void displayBlocs(const char* bloc);

#endif
