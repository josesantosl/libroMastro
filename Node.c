#include "Structs.c"
extern Transazione libroMastro[SO_REGISTRY_SIZE * SO_BLOCK_SIZE];/*libro mastro dove si scrivono tutte le transazioni.*/
extern int libroCounter;/*Counter controlla la quantitta di blocchi*/
extern sem_t libroluck;/*luchetto per accedere solo un nodo alla volta*/

/*variabili condivise tra diversi thread.*/
extern nodeStruct *nodeList;
extern Configurazione configurazione;
extern time_t startSimulation;
extern pthread_t *nodi_id;       /*lista dei processi nodi*/

/*Trova thread id in nodi_id*/
int trovaNid(){
    int i;
    for(i=0;i<configurazione.SO_NODES_NUM;i++){
        if(nodi_id[i] == pthread_self()){
            return i;
        }
    }
}

/*funzione dell'ultima transazione del blocco.*/
Transazione riasunto(int id, int somma){
    Transazione transaction;
    transaction.sender    = -1;
    transaction.receiver  = id; /*id del nodo*/
    transaction.quantita  = somma; /*la somma di tutto il reward generato*/
    transaction.timestamp = difftime(time(0),startSimulation);/*quanto tempo ha passato dal inizio della simulazione.*/
    return transaction;
}
void* nodo(void *conf){
	/*creazioni dei dati del nodo*/
    int id = trovaNid();
    int i;
    int counterBlock=0;/*contatore della quantita di transazioni nel blocco*/
    int sommaBlocco=0; /*somma delle transazioni del blocco atuale*/
    Transazione blocco[SO_BLOCK_SIZE];
    Transazione pool[1000];/*stabilisce 1000 come la grandezza massima del pool, cmq si ferma in configurazione.SO_TP_SIZE*/
    int semvalue;/*valore del semaforo*/

    /*rellena la estructura del nodo*/
    nodeList[id].poolsize = 0;
    nodeList[id].reward   = 0;
    sem_init(&nodeList[id].semaforo,configurazione.SO_USERS_NUM,1);

    
    /*inizio del funzionamento*/
    while(nodeList[id].poolsize < configurazione.SO_TP_SIZE){
    
		/*aggiorno il valore del semaforo*/
        sem_getvalue(&nodeList[id].semaforo,&semvalue);
        if(semvalue <= 0){
            /*printf("hay algo en el mailbox #%d\n",id);*/
			/*scrivo la nuova transazione nel blocco e nella pool*/
	    	 pool[nodeList[id].poolsize]=nodeList[id].mailbox;
	    	 blocco[counterBlock]=nodeList[id].mailbox;
    
	    	 /*somma il reward*/
	    	 sommaBlocco    += blocco[counterBlock].reward;
	    	 nodeList[id].reward += blocco[counterBlock].reward;/*si mette al registro publico totale*/
    
	    	 /*incremento i contatori di posizione di pool e block*/
	    	 counterBlock++;
	    	 nodeList[id].poolsize++;

	    	 if(counterBlock == SO_BLOCK_SIZE - 1){
	    	    /*si aggiunge una nuova transazione come chiusura del blocco*/
	    	    blocco[counterBlock]=riasunto(id, sommaBlocco);/*aggiunge la transazione al blocco.*/
    
	    	    sem_wait(&libroluck);
	    	    for(i=0;i< SO_BLOCK_SIZE;i++){
	    	       libroMastro[(libroCounter * SO_BLOCK_SIZE) + i] = blocco[i];
	    	    }
	    	    /*si spostano i contatori*/
	    	    libroCounter++;
	    	    sem_post(&libroluck);
	    	    counterBlock=0;
	    	    sommaBlocco=0;
				randomSleep(configurazione.SO_MIN_TRANS_PROC_NSEC,configurazione.SO_MAX_TRANS_PROC_NSEC);
    
	    	      
	    	}
            if(nodeList[id].poolsize < configurazione.SO_TP_SIZE){
                sem_post(&nodeList[id].semaforo);/*stabilisco il semaforo come di nuovo disponibile*/
	        }
    
		}
    
	}
}
