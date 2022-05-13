#include <stdio.h>  /*Standard input-output header*/
#include <stdlib.h> /*Libreria Standard*/  
#include <time.h>   /*Acquisizione e manipolazione del tempo*/
#include <stdbool.h>/*Aggiunge i boolean var*/
#include <string.h>/*Standar library for string type*/

#include <unistd.h>      /*Header per sleep()*/
#include <pthread.h>     /*Creazione/Modifica thread*/
#include <semaphore.h>   /*Aggiunge i semafori*/

#include "User.c"
Transazione libroMastro[SO_REGISTRY_SIZE * SO_BLOCK_SIZE];/*libro mastro dove si scrivono tutte le transazioni.*/
int libroCounter=0;/*Counter controlla la quantitta di blocchi*/
sem_t libroluck;/*Luchetto per accedere solo a un nodo alla volta*/

/*variabili condivise tra diversi thread.*/
int *retrylist;      /*numero di tentativi di ogni utente*/
int *budgetlist;     /*un registro del budget di ogni utente*/
int *rewardlist;     /*un registro pubblico del reward totale di ogni nodo.*/
int *poolsizelist;   /*un registro del dimensioni occupate pool transaction*/
sem_t *semafori;     /*semafori per accedere/bloccare un nodo*/
Transazione *mailbox;/*struttura per condividere */
time_t startSimulation;
pthread_t *utenti_id;     /*lista id di processi utenti*/
pthread_t *nodi_id;     /*lista id di processi nodi  */
Configurazione configurazione;
int *userStatus(int i, int bdgt, int actv, int inctv){

  int *r=malloc(4*sizeof(int));
  if(!r)
    return NULL;

  r[0]=bdgt+budgetlist[i];
  r[1] = retrylist[i]<configurazione.SO_RETRY;
  if(r[1])
    r[2]=actv+1;
  else
    r[3]=inctv+1;

  return r;
}
int *nodeStatus(int i, int rwrds, int actv, int inctv){

  int *r= malloc(4*sizeof(int));
  if(!r)
    return NULL;

  r[0]=rwrds+rewardlist[i];
  r[1] = poolsizelist[i] < configurazione.SO_TP_SIZE;
  if(r[1])
    r[2]=actv+1;
  else
    r[3]=inctv+1;

  return r;
}
bool printStatus(){
    int i;
    int *rU=userStatus(0,0,0,0);
    int *rN=nodeStatus(0,0,0,0);;
    
    if(rU && rN){
        /*Attributi*/
        printf("\n\n");
        printf("|| User_ID | Budget | Status |##| Node_ID | Rewards | Status ||\n");

        /*Stampa risultati*/
        for(i=0; i<MAX(configurazione.SO_NODES_NUM,configurazione.SO_USERS_NUM); i++){
            if(i<configurazione.SO_USERS_NUM && i<configurazione.SO_NODES_NUM){
                rU=userStatus(i,*(rU+0),*(rU+2), *(rU+3));
                rN=nodeStatus(i,*(rN+0),*(rN+2), *(rN+3));
                printf("||%9d|%8d|%8s|##|%9d|%9d|%8s||\n", i , *(rU+0) , *(rU+1)?"True":"False" , i,*(rN+0) , *(rN+1)?"True":"False");
            }else if(i<configurazione.SO_USERS_NUM && i>=configurazione.SO_NODES_NUM){
                rU=userStatus(i,*(rU+0),*(rU+2), *(rU+3));
                printf("||%9d|%8d|%8s|##|         |         |        ||\n", i , *(rU+0) , *(rU+1)?"True":"False");
            }else if(i>=configurazione.SO_USERS_NUM && i<configurazione.SO_NODES_NUM){
                rN=nodeStatus(i,*(rN+0),*(rN+2), *(rN+3));
                printf("||         |        |        |##|%9d|%9d|%8s||\n", i, *(rN+0) , *(rN+1)?"True":"False");
            }
        }
        printf("\n\n");
        free(rU);
        free(rN);
    }
    return *(rU+2)!=0;
}

int main(int argc,char *argv[]){
    int i;
	float now;
    bool test;
    int counterAttivi;

    srand((unsigned) time(0)); /*aleatorio*/

    if(argc<2){
	    printf("si aspettava un file con la configurazione o il commando 'manual'.\n");
        exit(EXIT_FAILURE);
    }else if(argc>2){
		printf("troppi argomenti.\n");
		exit(EXIT_FAILURE);
    }else{
		/*in caso di voler inserire i valori a mano*/
		if( strcmp(argv[1],"mano")==0 || strcmp(argv[1],"manual")==0 ){
			writeConf();
		}else{
	    	 readconf(argv[1]);/*lettura del file*/
        }
    
        /*now that we have all the variables we can start the process
        master*/
    
        startSimulation = time(0);/* el tiempo de ahora*/
        sem_init(&libroluck,0,1);/*inizia il semaforo del libromastro*/
    
        /*generatore dei nodi*/
        poolsizelist=malloc(configurazione.SO_TP_SIZE * sizeof(int));
        rewardlist=malloc(configurazione.SO_NODES_NUM * sizeof(int));
        semafori=malloc(configurazione.SO_NODES_NUM * sizeof(sem_t));
        mailbox=malloc(configurazione.SO_NODES_NUM * ((4 * sizeof(int)) + sizeof(double)));
        nodi_id = malloc(configurazione.SO_NODES_NUM * sizeof(pthread_t));
        for(i=0;i<configurazione.SO_NODES_NUM;i++){
			pthread_create(&nodi_id[i],NULL,nodo,NULL);
        }

        /*generatore dei utenti*/
        retrylist =malloc(configurazione.SO_USERS_NUM * sizeof(int));
        budgetlist=malloc(configurazione.SO_USERS_NUM * sizeof(int));
        utenti_id = malloc(configurazione.SO_USERS_NUM * sizeof(pthread_t));
        for(i=0;i<configurazione.SO_USERS_NUM;i++){
			pthread_create(&utenti_id[i],NULL,utente,NULL);
        }
    
		/*now start the master process*/
		now = difftime(time(0), startSimulation);

		while(now < configurazione.SO_SIM_SEC){
			sleep(1);
			clear();
    
			/*show last update*/
	    	printf("ultimo aggiornamento: %.2f/%d\n",difftime(time(0),startSimulation),configurazione.SO_SIM_SEC);

	    	now = difftime(time(0), startSimulation);
            
            if(libroCounter > SO_REGISTRY_SIZE){
                printf("%f: libro mastro pieno\n",now);
                break;
            }

            if(!printStatus()){
                printf("tutti gli utenti sono disattivati");
                break;
            }

        }
    
        /*kill all the threads*/
        for(i=0; i<configurazione.SO_NODES_NUM ; i++){
			pthread_cancel(nodi_id[i]);
		}
        for(i=0; i<configurazione.SO_USERS_NUM; i++){
            pthread_cancel(utenti_id[i]);
        }
    
		/*printf("numero di blocchi: %d\n\n",libroCounter);*/
		/*solo por confirmar al final*/
		for(i=0;i<libroCounter*SO_BLOCK_SIZE;i++){
			/*prinTrans(libroMastro[i]); per ora non mostro tutte transazioni*/
        }
    
	}
	return 0;
}
