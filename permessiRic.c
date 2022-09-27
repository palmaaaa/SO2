#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "arrayDinamico.h"  //Implementazione di uno pseudo dizionario per il conteggio di file uguali [Descrizione nel file stesso]

//Author: Michele Palma 1849661


//             [DESCRIZIONI FUNZIONI E GESTIONE ERRORI]                       
// Le descrizioni generali e dettagliate si trovano nelle funzioni stesse, mentre
// la descrizione della gestione degli errori si trova prima della funzioni.

//------------------------PROTOTIPI FUNZIONI---------------------------------
void visitaRicorsivaDirectory(char *, int, int,char **,struct arrayDin *,int);
char ** creaPathCartelle(long unsigned int, char *);
void creaCartelle(char * pathPadre, char * pathCorrente);
char * creaPathSuccessivo(char *, char *, struct stat *,int);
void copiaFile(char *,char **, char *, struct stat, int, struct arrayDin *);
void liberaMemoria(char **, struct arrayDin *);

//------------MAIN----------------
int main (int argc, char *argv[]){

	//____________CONTROLLI SULL'INPUT___________________________________________________________________________________________________________________________________________________________

	if (argc!=4){  						   	   //                   [CONTROLLO DEL NUMERO DI ARGOMENTI]                                                              
		puts("Inserire Path,Profondita' e Processi \nRiprovare");  //  Gli argomenti devono essere esattamente 4 anche se l'utente ne inserisce 3.                                       
		exit(EXIT_FAILURE);					   //  Bisogna considerare anche argv[0] ovvero il nome del programma che si esegue.                                     
	}																							     
																								
	if (argv[1][0] != '/' || argv[1][strlen(argv[1])-1] != '/'){                  	         	 //                [CONTROLLO DELLA FORMATTAZIONE DELLA STRINGA]             
		puts("Inserire un path con il seguente formato\n-> /home/cartella/DIR/ <-\nRiprovare");  //   La stringa inserita deve avere esattamente questo formato: /cartella1/DIR/  
		exit(EXIT_FAILURE);									 // si controllano quindi primo ed ultimo carattere per evitare errori di segmentazione.
	}                                                                                        	 //    Anche inserendo dei numeri (o solo numeri) questi controlli ci tutelerebbero. 
	
	if (access(argv[1],R_OK|X_OK) == -1){   //                      [CONTROLLO ACCESSO PATH]
		perror("Path Error: ");         //    Controlliamo se l'accesso alla directory non dia nessun errore.
		exit(EXIT_FAILURE);             // In caso di errore potrebbe non esistere la directory o non essere accesibile.
	}
	
	if (atoi(argv[2]) < 0 || (atoi(argv[2])==0 && argv[2][0]!= '0')){   	     //                                       [CONTROLLO PROFONDITA'] 
		puts("Inserire un numero naturale per la Profondita'\nRiprovare");   //    Viene controllato se il valore di profondita' inserito e' positivo. Se viene inserita una stringa la
		exit(EXIT_FAILURE);						     //   conversione da atoi darà zero. Siccome anche la stringa "0" con atoi restituisce zero, viene effettuato
	}                                                                    	     //   il secondo controllo. Cioe' controlliamo il primo carattere di argv[2], se e' diverso da zero e la funzione
                                                                                     //       restituisce zero, significa che l'utente ha inserito sicuramente una stringa e non un numero.
	 
	if (atoi(argv[3]) <= 0){                                                         	//                                   [CONTROLLO PROCESSI]
		puts("Inserire un numero naturale maggiore di zero per i Processi\nRiprovare"); //  Usiamo lo stesso ragionamento fatto per la profondita', con l'unica differenza che il valore di P>0.
		exit(EXIT_FAILURE);                                                             //   Eliminando quindi il caso in cui ci potesse essere l'ambiguità con l'inserimento di una stringa.
	}                                                          
	//____________________________________________________________________________________________________________________________________________________________________________________________

	char ** pathCartelle = creaPathCartelle(strlen(argv[1]),argv[1]); // Avremo il path padre e il path delle cartelle eseguibili/regolari in questo puntatore a puntatori

	struct arrayDin pseudoDiz = newArray(); // Questo pseudo-dizionario sara' utilizzato per tener traccia dei file con nome uguale (per la copia).

	creaCartelle(pathCartelle[0],argv[1]); // Si creano le cartelle eseguibili" e "regolari" al di fuori della directory inserita dall'utente
	
	visitaRicorsivaDirectory(argv[1],0, atoi(argv[2])+1,pathCartelle,&pseudoDiz,atoi(argv[3])); // Si esploreranno tutte le cartelle/file all'interno della directory dell'utente.

	liberaMemoria(pathCartelle,&pseudoDiz); // Si liberano tutte le variabili con memoria allocata dinamicamente
}

//==================================================================================================================================================================================================================
//                                                                                       [GESTIONE DEGLI ERRORI]
//  	       Il programma gestisce i controlli su input ed eventuali errori delle funzioni utilizzate. L'attento controllo che viene effettuato per i valori di input permette di eliminare molti
//	     errori che potrebbero verificarsi durante l'esecuzione. In particolare si controlla la formattazione della stringa e la sua validita', non permettendo all'utente di inserire una directory
//	       mal formattata o addirittura non esistente/non accessibile. Esistono pochi casi di errori che non minerebbero la corretta esecuzione del programma, per i quali si avviserebbe l'utente
//                con una stampa a video senza terminare il programma. Nei casi non gestiti sono presenti errori riguardanti la memoria (es. memoria insufficiente o simili).
//==================================================================================================================================================================================================================
//   |                                                                             [FUNZIONE visitaRicorsivaDirectory]                                                                                           |
//   | E' la funzione principale del programma. Permettere di visitare ricorsivamente tutte le directory analizzate e nel caso di incontro di file ne fara' la copia. Questo comportamento avviene nel caso in   |
//   |    cui i Processi inseriti in input siano minori di 2. Nel caso in cui i processi siano maggiori di 1, il processo padre esplorera' tutte le directory salvando i path dei file da copiare, mentre i      |
//   |   figli effettueranno la copia. Nella prima chiamata a questa funzione si avra' il path dell'utente (che cambiera',entrando in ricorsione, nel path delle sotto Dir incontrate durante l'esplorazione)    |
//   |     il livello corrente (che sara' 0 ,ovvero la radice, ed aumentera' ad ogni visita ricorsiva), il livello di profondita' massimo desiderato dall'utente, i path delle cartelle dove verrano copiati i   |
//   |       files, lo pseudo-dizionario nel quale salveremo tutte le occorenze dei file trovati (con lo stesso nome) e il numero dei Processi (in base al quale si decidera' quale tipo di operazioni fare).    |
//   |___________________________________________________________________________________________________________________________________________________________________________________________________________|


void visitaRicorsivaDirectory(char * pathInputUtente, int livelloCorrente, int livelloInputUtente,char ** pathCartellePerCopia, struct arrayDin * diz, int processi){

	if (livelloCorrente==livelloInputUtente) return; // [CASO BASE RICORSIONE] Se il livello di profondita' inserito dall'utente viene raggiunto, si interrompe la ricorsione.
	
	DIR * pathStream;  //variabile che verrà utilizzata per contenere lo stream del path dell'utente

	pathStream = opendir(pathInputUtente); // con la funzione assegnamo a pathStream lo stream della Directory in input dall'utente
	
	struct dirent * puntatoreStruttura = NULL; // struttura tipo Dirent per informazioni su cartelle e su file

	struct stat infoFile; //struttura di tipo stat utilizzata per ottenere le informazioni sui permessi/nome del file attraverso la funzione stat
	
	char * pathTemporaneo = NULL; //variabile che puntera' a tutti i path che varrano creati (sia Directory che file). 

	

	while ((puntatoreStruttura = readdir(pathStream)) != NULL){   // Si estrae la struttura dati con le informazioni del path ad ogni ciclo finche' non diventera' NULL, ovvero 
                                                                      // non ci saranno altri file/directory da analizzare.

		switch(puntatoreStruttura->d_type){ // Si analizzeranno Directory e File

			case (DT_DIR):

				if( puntatoreStruttura->d_name[0]=='.'|| puntatoreStruttura->d_name[1]=='.') break; // Non si vogliono analizzare le DIR speciali "." e ".."

				printf("%s ->DIR Livello: %d\n========\n",(*puntatoreStruttura).d_name,livelloCorrente);	

				pathTemporaneo = creaPathSuccessivo(pathInputUtente,puntatoreStruttura->d_name,NULL,1);	// Path costruito per la directory (entrera' nella 
															// funz. ricorsiva per esplorare quest'ultima)

				visitaRicorsivaDirectory(pathTemporaneo,livelloCorrente+1,livelloInputUtente,pathCartellePerCopia,diz,processi); // Si continuano ad esplorare le directory incontrate, aumentando 
																	         // il livello di profondita'.
				free(pathTemporaneo);
				break;

			
			case(DT_REG):	

				pathTemporaneo = creaPathSuccessivo(pathInputUtente,puntatoreStruttura->d_name,&infoFile,0); // Path costruito per il file

				if( (infoFile.st_mode & S_IXUSR)  || (infoFile.st_mode & S_IXGRP) || (infoFile.st_mode & S_IXOTH)){ // Si controlla se il file e' eseguibile 
																    // [ovvero se uno tra user/group/other ha il permesso di esecuzione]

					printf("%s ->ESEGUIBILE Livello: %d\n========\n",(*puntatoreStruttura).d_name,livelloCorrente);	
					copiaFile(pathTemporaneo,pathCartellePerCopia,puntatoreStruttura->d_name,infoFile,1,diz); // Si copia il file nella cartella eseguibili

				}else{
					printf("%s ->REGULAR Livello: %d\n========\n",(*puntatoreStruttura).d_name,livelloCorrente);
					copiaFile(pathTemporaneo,pathCartellePerCopia,puntatoreStruttura->d_name,infoFile,2,diz); // Si copia il file nella cartella regolari
				}
			
				free(pathTemporaneo);
				break;
		}
	}
	closedir(pathStream);  // La directory viene chiusa (ogni scope della ricorsione chiudera' le directory analizzate)	
	return; // Nel caso in cui l'utente inserisca una Profondita' maggiore di quella effettiva, il programma interrompera' comunque la ricorsione arrivati all'ultimo livello.
}
//==================================================================================================================================================================================================================
//                                             |                                            [FUNZIONE creaPathCartelle]                                                 |  
//                                             |              Prende come input il path dell'utente e la lunghezza di quest'ultimo. Ritorna il path padre               |
//                                             |     e quello cartelle che conterranno i file eseguibili/regolari (posti al di fuori della DIR immessa dall'utente)     |                                    
//                                             |________________________________________________________________________________________________________________________|

char ** creaPathCartelle(long unsigned int len_pathInput, char * pathInput){                                       

	int i=0; //Contatore For (utilizzato anche al di fuori di esso)

	char** puntatorePathCartelle= malloc(sizeof(char *)*3); // Puntatore a puntatori che conterrà il path padre e quello delle cartelle per i file eseguibili e regolari
        if(!puntatorePathCartelle){									            
       		perror("[Function creaPathCartelle]\nError with malloc():"); 
		exit(EXIT_FAILURE);
	}
	
	//------CREAZIONE PATH PADRE------
	for (i=len_pathInput-2;i>=0; i--){        // Si scorre il path in input al contrario, partendo dal terz'ultimo elemento evitando di controllare                                  
		if (pathInput[i]== '/') break;    // gli ultimi due elementi noti (slash finale e terminatore stringa). Non appena si trova uno slash si sapra'
	}                                         // che quello sarà il path padre, si interrompe il ciclo, tenendo in "i" il valore posizionale di quest'ultimo.

	char * pathPadre= (char *)malloc(i+2);        		             // Si crea la stringa che conterra' il path padre. La dimensione sarà: la i (data dallo scorrimento) 
	if(!pathPadre){                                                      // +2 (siccome il vettore parte da zero, bisogna considerarlo,quindi +1, in più bisogna considerare
       		perror("[Function creaPathCartelle]\nError with malloc():"); // lo \0, ovvero un altro +1). Si copia una parte del path di input (ovvero il path padre, che sappiam
		exit(EXIT_FAILURE);                                          // sia memorizzato posizionalmente in i) e si inserisce lo '\0'.
	}                                        
	strncpy(pathPadre,pathInput,i+1);        
	pathPadre[i+2]= '\0';                    

	//----------CREAZIONE PATH CARTELLE----------
	char * eseguibili=malloc(strlen(pathPadre)+12); 		     // Dopo aver ottenuto il path padre creiamo due aree di memoria che					  
	if(!eseguibili){			                             // conterranno i path delle cartelle eseguibili e regolari.
       		perror("[Function creaPathCartelle]\nError with malloc():"); // Esse avranno dimensione del path padre piu' una dimensione fissata.
		exit(EXIT_FAILURE);					     // Lunghezza del nome della cartella + \0 [es. "regolari/" Len: 9 + \0 = 10]
	} 

	char * regolari=malloc(strlen(pathPadre)+10);
	if(!regolari){			               
       		perror("[Function creaPathCartelle]\nError with malloc():"); 
		exit(EXIT_FAILURE);
	}  
	
	strcat(eseguibili,pathPadre);     //Con una serie di concatenazioni si creano i path 
	strcat(eseguibili,"eseguibili/");
	strcat(regolari,pathPadre);
	strcat(regolari,"regolari/");
	
	puntatorePathCartelle[0]=pathPadre;  // Si assegnano a "puntatorePathCartelle" i puntatori di path padre,eseguibili e regolari con dimensione allocata dinamicamente nella funzione (sapendo che 
	puntatorePathCartelle[1]=eseguibili; // anche al di fuori di essa si avra' comunque quell'area di memoria riservata e quindi i dati disponibili).
	puntatorePathCartelle[2]=regolari;

	return puntatorePathCartelle;
}
//==================================================================================================================================================================================================================
//                                                               |                              [FUNZIONE creaCartelle]                                 |
//                                                               |      Si cambia directory (da corrente a directory padre). Si creano le directory     |
//                                                               |    Eseguibili e Regolari (al di fuori della DIR analizzata) e si cambia nuovamente   |
//                                                               |                     il path in quello immesso dall'utente.                           |
//                                                               |______________________________________________________________________________________|

void creaCartelle(char * pathPadre, char * pathCorrente){                           
                                                                                           
	if ( chdir(pathPadre) == -1){   				 // Se la chdir dovesse dare errore si interrompera' l'esecuzione del programma.
		perror("[Function creaCartelle]\nError with chdir():");
		exit(EXIT_FAILURE);
	}
                                                               
	if (mkdir("eseguibili",S_IRWXU |S_IRWXG |S_IRWXO) == -1){                // Se la mkdir non andasse a buon fine non si  interrompera' direttamente il programma.
		if(errno != EEXIST){				                 // Si controllera' se l'errore e' causato dall'esistenza della cartella, in caso
			perror("[Function creaCartelle]\nError with mkdir():");  // positivo si continuera' con l'esecuzione, mentre in qualsiasi altro caso si interrompera'.
			exit(EXIT_FAILURE);							    
		}
		puts("Folder 'eseguibili' already exists, won't be created");
	}

	if (mkdir("regolari",S_IRWXU |S_IRWXG |S_IRWXO) == -1){
		if(errno != EEXIST){
			perror("[Function creaCartelle]\nError with mkdir():");
			exit(EXIT_FAILURE);
		}
		puts("Folder 'eseguibili' already exists, won't be created");
	}

	if ( chdir(pathCorrente) == -1){
		perror("[Function creaCartelle]\nError with chdir():");
		exit(EXIT_FAILURE);
	}
}
//==================================================================================================================================================================================================================
//                                                            |                             [FUNZIONE creaPathSuccessivo]                                   | FUNZIONALITA' AGGIUNTIVA: se si vuole solo costruire
//                                                            |    La funzione crea un path successivo per una directory (MODE 1) o per un file (MODE 0).   | un path di un file senza estrarne i permessi e'
//                                                            |   Oltre che creare un path per il file, ne vengono anche estratte le informazioni (stat).   | necessario inserire NULL nel terzo argomento.
//                                                            |_____________________________________________________________________________________________|_______________________________________________________
													    
                                                                                                            

char * creaPathSuccessivo(char * pathUtente, char * pathSuccessivo, struct stat * tempPermessi, int mode){  

	char * nextPath = (char *)malloc(strlen(pathUtente)+strlen(pathSuccessivo)+mode+1);     // nextPath conterra' il path successivo, la mode indicherà se il path sara' per una directory o per un file.
        if(!nextPath){									        // La sua lunghezza e' data da: lunghezza del path padre + lunghezza path successivo + / (in caso di directory
       		perror("[Function creaPathSuccessivo]\nError with malloc():");                  // [mode=1]) + \0 oppure senza slash e con solo \0 (in caso di file [mode=0]).
		exit(EXIT_FAILURE);
	}

	switch(mode){

		case 0: //CASE FILE
			strcpy(nextPath,pathUtente);     //Costruzione del path per file
			strcat(nextPath,pathSuccessivo);	 
			nextPath[strlen(nextPath)]='\0';	 // Si utilizza poi il path (del file) con la funzione stat per estrarne le informazioni. Alla funzione  creaPathSuccessivo passiamo l'indirizzo
			if (tempPermessi == NULL) break;         // della variabile di tipo struct stat che si trova nel main, cosi' da ritrovarci i dati disponibili al di fuori.
			if (stat(nextPath,tempPermessi) == -1){ 
				perror("[Function creaPathSuccessivo]\nError with stat():");
				exit(EXIT_FAILURE);
			}
			break;                             

		case 1: //CASE DIRECTORY
			strcpy(nextPath,pathUtente);       //Costruzione del path per le directory
			strcat(nextPath,pathSuccessivo);
			strcat(nextPath,"/");
			nextPath[strlen(nextPath)]='\0';
			break;
	}
	return nextPath; //Si restituisce il path per essere utilizzato - Costruisce in maniera "dinamica" il path successivo
}
//==================================================================================================================================================================================================================
//                                             |                                                [FUNZIONE copiaFile]                                                            |
//                                             | La funzione prende in input il path del file che si vuole copiare, quello della cartella dove verrà copiato, il nome del file, |
//                                             |                  permessi/dimensione (con la struct stat) e il tipo (2=regolare | 1=eseguibile).                               |
//                                             |________________________________________________________________________________________________________________________________|

void copiaFile(char * pathCorrenteFile, char ** pathExecReg, char * nomeFile, struct stat infoFileInput, int tipoFile, struct arrayDin * pseudoDiz){

	int fdFileOriginale=0; // file descriptor del file che verra' copiato
	int fdFileCopia=0;     // file descriptor della copia
	int rwControllo = 0;   // controllo del ritorno delle funzioni read/write


	char * nomeFileDefinitivo = malloc(strlen(nomeFile)+1);       // Si crea preventivamente il nome del file copia uguale all'originale, dopodiche'
	if(!nomeFileDefinitivo){			              // si controlla se esiste gia' una copia con quel nome. Se esiste si riallochera' la 
       		perror("[Function copiaFile]\nError with malloc():"); // memoria puntata per poter apportare le opportune modifiche al nome.
		exit(EXIT_FAILURE);
	}  
	strcpy(nomeFileDefinitivo,nomeFile);  

                 
	//------CREAZIONE NOME DEL FILE COPIA-----						
	if(!aggiungiElemento(pseudoDiz,nomeFile)){  //Si controlla se il file e' stato gia' copiato. Se il ritorno della funzione è false allora esiste gia' un file con questo nome (si rinomina il file copia)
			
		int fileOccorrenze = pseudoDiz->array[trova(pseudoDiz,nomeFile)].occorrenze; // Con trova si otterra' l'indice al quale e' presente il numero di occorrenze del file
										             // (ovvero quante copie esistono). Avremo quindi nella variabile le occorrenze di esso.

		char dim [7];
		snprintf(dim,7,"%d",fileOccorrenze); // Si trasforma l'int delle occorenze in una stringa da poter concatenare al nome del file.
						     // Si utilizza una grandezza prefissata (7),sapendo che l'utente difficilmente copiera' 999'999 file identici

		int i;
		for (i=1;i<strlen(dim)+1;i++){    // Con questo for si trovera' il numero esatto di byte da riallocare per il nome del file copia.
			if (dim[i] == ' ') break; // Ovvero' a meno che il numero non sia a 6 cifre, occupando tutto il buffer, si avranno delle "celle vuote"
		}				  // che non verranno considerate (per la realloc). Si copieranno solo i "numeri effettivi".
		
		nomeFileDefinitivo = realloc(nomeFileDefinitivo,strlen(nomeFile)+i+3);   // Si rialloca la memoria per il file copia. Avra' la lunghezza del file originale + il numero di occorenze
		if (!nomeFileDefinitivo){					         // (il quale includera' uno spazio vuoto ' ', per cui non si fa il +1 per lo '\0' considerando di averlo gia' fatto cosi')
			perror("[Function copiaFile]\nError with realloc():");	         // +3 ovvero i caratteri che verranno utilizzati per formattare la copia ---> "copia.txt_(28372)"
			exit(EXIT_FAILURE);
		}  

		strcat(nomeFileDefinitivo,"_(");   // Costruzione del nome 
		strncat(nomeFileDefinitivo,dim,i); //---> Si arriva fino ad "i", ignorando tutti i spazi vuoti che occuperebbero celle inutilmente 
		strcat(nomeFileDefinitivo,")");
		nomeFileDefinitivo[strlen(nomeFileDefinitivo)] = '\0';
	}

	char * pathFileFinale = creaPathSuccessivo(pathExecReg[tipoFile],nomeFileDefinitivo,NULL,0); 
	 								 // In base al tipo di file si costruira' il path della sua copia utilizzando il path della cartella (regolare/eseguibile)
			                                                 // concatenata al nome del file originale. Cosicche' si avra' nell'open il path del file "copia". 
                                                                         // Si riutilizza la funzione pathSuccessivo. N.B. path cartella regolare = pathExecReg[2] | eseguibile = pathExecReg[1]
                                                                         // per chiarezza guardare la funzione creaPathCartelle (ultime 2 operazioni prima del return)

	void * sizeFile = malloc(infoFileInput.st_size);   // Si crea un buffer per le funzioni read e write, dove salveremo il contenuto del file. Avra' la dimensione del file (st_size di quest'ultimo).
	if (!sizeFile){
		perror("[Function copiaFile]\nError with malloc():");
		exit(EXIT_FAILURE);
	}

	//------------COPIA DEL FILE ORIGINALE-----------
	fdFileOriginale = open(pathCorrenteFile,O_RDONLY);      		     // Si apre il file da copiare e con la read si memorizzano
	if (fdFileOriginale == -1){						     // i dati su un buffer temporaneo (sizeFile). Si chiude il 
		perror("[Function copiaFile]\nError with open() (Original file):");  // file una volta terminata la copia nel buffer.
		exit(EXIT_FAILURE);
	}

	rwControllo = read(fdFileOriginale,sizeFile,infoFileInput.st_size);          // Si controlla se la read e' andata a buon fine. Nel caso in cui vengano
	if (rwControllo == -1){                                                      // letti meno byte di quelli del file originale si notifichera' l'utente.
		perror("[Function copiaFile]\nError with read() (Original file):");  // In caso di errore effettivo si terminera' il programma.
		exit(EXIT_FAILURE);

	}else if(rwControllo < infoFileInput.st_size){
		printf("%d Bytes were read instead of %lu Bytes from %s \n",rwControllo,infoFileInput.st_size,nomeFile);
	}

	if (close(fdFileOriginale) == -1){  
		perror("[Function copiaFile]\nError with close() (Original file):");
		exit(EXIT_FAILURE);
	}

	//-----------------------------------CREAZIONE DEL FILE COPIA--------------------------------------
	fdFileCopia = open(pathFileFinale,O_CREAT|O_WRONLY,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);  // Si crea la copia nella directory opportuna ( e con dei permessi temporanei
	if (fdFileCopia == -1){                                                                               // per un uso correttod della open). La funzione write copiera' il contenuto al suo interno. 
		perror("[Function copiaFile]\nError with open() (File duplicate):");
		exit(EXIT_FAILURE);
	}

	rwControllo = write(fdFileCopia,sizeFile,infoFileInput.st_size);	       // Si controlla se la write e' andata a buon fine. Nel caso in cui vengano
	if (rwControllo == -1){                                                        // scritti meno byte di quelli del file originale si notifichera' l'utente.
		perror("[Function copiaFile]\nError with write() (File duplicate):");  // In caso di errore effettivo si terminera' il programma.
		exit(EXIT_FAILURE);

	}else if(rwControllo < infoFileInput.st_size){
		printf("%d Bytes were written instead of %lu Bytes from %s \n",rwControllo,infoFileInput.st_size,nomeFile);
	}

	if( tipoFile == 1){                          // Se il file e' eseguibile [mode 1] si dovranno cambiare i permessi, rendendolo
		infoFileInput.st_mode &= ~(S_IXGRP); // eseguibile solo dal proprietario. Si eseguono quindi operazioni bit a bit
		infoFileInput.st_mode &= ~(S_IXOTH); // rendendo i bit di esecuzione di group e other 0, mentre quello del 
		infoFileInput.st_mode |= S_IXUSR;    // proprietario verrà messo a 1.
	}

	if (fchmod(fdFileCopia,infoFileInput.st_mode)== -1){            // Vengono applicati i nuovi permessi al file. Saranno uguali all'originale 
		perror("[Function copiaFile]\nError with fchmod(): ");  // in caso di file regolare oppure modificati (in caso di file eseguibile).
		exit(EXIT_FAILURE);                                     
	}
	if (close(fdFileCopia) == -1){                                               //Si chiude il file copia dopo le operazioni svolte.
		perror("[Function copiaFile]\nError with close() (File duplicate): ");
		exit(EXIT_FAILURE);	
	}

	
	free(sizeFile);           // Si libera la memoria 
	free(nomeFileDefinitivo); // allocata dinamicamente.
	free(pathFileFinale);
}
//==================================================================================================================================================================================================================
//                          |                                                         [FUNZIONE liberaMemoria]                                                                  |
//                          |       La funzione libera la memoria allocata dinamicamente per le variabili utilizzate nel main (anche utilizzate nelle chiamate ricorsive).      |
//                          |___________________________________________________________________________________________________________________________________________________|

void liberaMemoria(char ** puntatoriCartelle, struct arrayDin * pseudoDizionario){

	for(int i=0; i< pseudoDizionario->primoIndiceLibero; i++){    // Si "liberano" tutte le celle dello pseudoDizionario e con i primi tre cicli lo si fa 
		if (i<=2) free(puntatoriCartelle[i]) ;		      // anche per le celle del puntatore a puntatori. Infine si fa le free del puntatore a puntatori.
		free(pseudoDizionario->array[i].nomeFile);
	}
	free(puntatoriCartelle);
}
