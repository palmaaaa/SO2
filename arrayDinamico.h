/* Header che contiene la struttura che permettera' di tener conto dei file copiati.
   Si sono consultate diverse fonti per poter implementare l'array dinamico. Le principale nozioni sono state
   apprese con le videolezioni, mentre il resto è stato frutto ricerca sul web e di test.
   Nel codice viene chiamato come pseudo-Dizionario ma è difatti un array dinamico, cioe' qualcosa di ben diverso.
   Si e' definito come tale per l'uso che se ne fa, ovvero un array a struct coppia che contiene due campi, Nome e Occorrenze del nome.  */
	
struct coppia {	
	char * nomeFile;
	int occorrenze;
};

struct arrayDin{                
	struct coppia * array;	// Puntatore alla struct coppia
	int primoIndiceLibero;  // Primo indice libero all'interno dell'array
	int n;                  // Numero di celle effettive dell'array
};

//--[FUNZIONE nuovoArray()]--
struct arrayDin nuovoArray(){               // Funzione per inizializzare in maniera elegante la variabile di tipo arrayDin.

	struct arrayDin temp = {NULL,0,0};
	return temp;
}

//---------------[FUNZIONE trova]------------------
int trova (struct arrayDin * var,char * daTrovare){         // Funzione che restituira' l'indice al quale si trova la stringa messa in input nel nostro "dizionario". 
							    // Se la stringa non e' presente verra' restituito -1.
	int indice = -1;				   
	for(int i=0;i < var->primoIndiceLibero; i++){
		if(strcmp(var->array[i].nomeFile, daTrovare) == 0){
			indice = i;
		}
	}
	return indice;
}

//------[FUNZIONE stampa_array]----------
void stampa_array(struct arrayDin * var){                   // Funzione usata principalmente per il debug. Stampa semplicemente tutti i nomi presenti nella struttura con le rispettive occorrenze.

	for(int i = 0; i < var->primoIndiceLibero; i++){
		printf("%s = %d\n",var->array[i].nomeFile, var->array[i].occorrenze);
	}
}

//-----------------[FUNZIONE aggiungiElemento]---------------
bool aggiungiElemento(struct arrayDin * var, char * nomeFile){   // Funzione principale della struttura. 

	char * temp = malloc(strlen(nomeFile)+ 1);
	for(int i = 0; i < strlen(nomeFile) + 1; i++){
		temp[i] = nomeFile[i];
	}
	struct coppia daAggiungere = {temp,1};
	int indice =  trova(var,nomeFile);

	if(indice == -1){                               //Se non e' presente la stringa nel "dizionario" si controllera' se il primo indice libero e' minore della capienza massima, ovvero l'array non 
		if(var->primoIndiceLibero < var->n){                                                // e' stato riempito totalmente, quindi si aggiungera' la stringa con occorrenza 1. Altrimenti lo 
			var->array[var->primoIndiceLibero++] = daAggiungere;                        // spazio non e' sufficiente e si  dovra' riallocare memoria. Si raddoppiera' la memoria +1 
		}else{                                                                              // (nel caso di zero anche raddoppiando lo spazio allocato rimarrebbe 0) e aggiornera' la nuova
			var->array = realloc(var->array, (2 * var->n + 1) * sizeof(struct coppia)); // capienza massima dell'array.
			var->array[var->primoIndiceLibero++] = daAggiungere;
			var->n = 2 * var->n + 1; 
		}
		return true;  // Si ritorna true se si aggiunge un nuovo elemento.
	}else{
		var->array[indice].occorrenze++;  // Se viene ritornato l'indice del nome (gia' presente nel dizionario ) allora si aumenteranno le sue occorrenze.
		return false; // Si ritorna false se effettivamente non si e' aggiunto un nuovo nome.
	}
}
