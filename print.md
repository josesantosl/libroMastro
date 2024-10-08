# 7.Prints

Questa sezione contiene tutto il codice che collega con le funzioni che servono per stampare le funzioni. 
Le principali funzioni sono:

__printStatus__: mostra una tabella con la informazione degli utenti piu attivi e i nodi. Si usa per mostrare i dati aggiornati in ogni secodo de la simulazione.

__finalPrint__: mostra una tabella con tutti gli utenti ,utti i nodi, e più dati, come il nome indica, si usa per stampare tutti i dati alla fine della simulazione. 

## Macros

Lista di macros che ci servono per stampare tutti i valori:

- __clear__: pulisce lo schermo.
- __MAX__: ritorna il numero maggiore tra i due.
- __MIN__: invia il numero minore tra i due. 
- __boolString__: fa la funzione di %b in altri linguagi di programmmazione.

```c print.c
/*macros per il print*/
#define clear() printf("\033[H\033[J") /*clear the screen*/
#define MAX(x,y) ((x>y)?x:y) /*max between to parameters*/
#define MIN(z,w) ((z<w)?z:w) /*min between to parameters*/
#define boolString(b) ((b) ? "True":"False")/*make the %b*/

```



## memoria condivise

Tutte le variabili che devono stampare le funzioni di stampa

```c print.c
/*variabili degli utenti*/
extern int *budgetlist;
extern bool *checkUser;

/*variabili degli utenti*/
extern int *rewardlist;
extern int *rewardlist;
extern int *poolsizelist;
extern bool *checkNode;

```
## Compare Function
Metodo che compara due valori e restituisce un numero positivo, se b è piu
grande di a ,e negativo, se b è piu piccolo di a.
```c print.c
int cmpfunc(const void  *a, const void *b) {
    return(budgetlist[*((int*)b)]-budgetlist[*((int*)a)]);
}
```


## Sort risults
Metodo di ordinamento dei processi in modo decrescente (dal piu grande al piu piccolo).

```c print.c
int * sort(){
    int dim=configurazione.SO_USERS_NUM;
    int *r=malloc(sizeof(int)*dim);
    int i;
    for(i=0; i<dim; i++)
        r[i]=i;
    
    qsort(r, dim, sizeof(int), cmpfunc);
    return r;
}
```


## PrintStatus Nodes and Users
Questo metodo non solo mostra lo stato di tutti gli utenti e i nodi, ritorna anche una variabile boolean per identificare se ci sono ancora utenti disponibili.

```c print.c
bool printStatus(int nstamp){
    /*User var*/
    int activeUsers=0;
    int inactiveUsers=0;
    int sommaBudget=0;
    bool ActiveU;
    /*Node var*/
    int activeNodes=0;
    int inactiveNodes=0;
    int sommaRewards=0;
    bool ActiveN;
    /*Share var*/
    int i=0;
    int *pa;
    int dim=MIN(MAX(configurazione.SO_USERS_NUM,configurazione.SO_NODES_NUM), nstamp);
    pa=sort();
    printf("\n\n");
    printf("------------------------------------------------------------------------\n");
    printf("||  User_ID |  Budget  |  Status |##|  Node_ID  |  Rewards  |  Status ||\n");
    printf("||===============================|##|=================================||\n");
    
    /*Stampa risultati*/
    for(i=0; i<MAX(configurazione.SO_USERS_NUM,configurazione.SO_NODES_NUM); i++){
        if(i<configurazione.SO_USERS_NUM){
            sommaBudget += budgetlist[*(pa+i)];
            checkUser[*(pa+i)] ? activeUsers++ : inactiveUsers++;
            if(i<dim){
                printf("||%10d|%10d|%9s|#",pa[i],budgetlist[*(pa+i)], boolString(checkUser[*(pa+i)]));
            }
        }else if(i<dim){
            printf("||          |          |         |#");
        }


        if(i < configurazione.SO_NODES_NUM){
            sommaRewards+=rewardlist[i];
            checkNode[i] ? activeNodes++ : inactiveNodes++;
            if(i<dim){
                printf("#|%11d|%11d|%9s||\n", i, rewardlist[i],boolString(checkNode[i]));
            }

        }else if(i<dim){
            printf("#|           |           |         ||\n");
        }
    }

    printf("------------------------------------------------------------------------\n");
    printf("||  Active Users  |  Tot Budget  |##|  Active Nodes  |   Tot Rewards  ||\n");
    printf("||%16d|%14d|##|%16d|%16d||\n",activeUsers,sommaBudget,activeNodes, sommaRewards);
    printf("\n");
    
    return activeUsers>0;
}

```

## final print

Questo metodo fa l'utima stampa del proggetto. Mostrando tutti gli utenti e mostrando anche la grandezza de la Transaction Pool. Serve come riassunto della simulazione.

```c print.c
void finalprint(){
    /*User var*/
    int activeUsers=0;
    int inactiveUsers=0;
    int sommaBudget=0;
    bool ActiveU;
    /*Node var*/
    int activeNodes=0;
    int inactiveNodes=0;
    int sommaRewards=0;
    bool ActiveN;
    /*Share var*/
    int i=0;
    int dim = MAX(configurazione.SO_USERS_NUM, configurazione.SO_NODES_NUM);
 
    printf("\n\n");
    printf("---------------------------------------------------------------------------------\n");
    printf("||  User_ID |  Budget  |  Status |##|  Node_ID  |  Rewards  |  p_size | Status ||\n");
    printf("||===============================|##|==========================================||\n");
    for(i=0; i< dim; i++){
        if( i < configurazione.SO_USERS_NUM){
            sommaBudget += budgetlist[i];

            checkUser[i] ? activeUsers++ : inactiveUsers++;

            printf("||%10d|%10d|%9s|#",i,budgetlist[i], boolString(checkUser[i]));
        }else{
            printf("||          |          |         |#");
        }

        if(i< configurazione.SO_NODES_NUM){
            sommaRewards+=rewardlist[i];

            checkNode[i] ? activeNodes++ : inactiveNodes++;
            printf("#|%11d|%11d|%9d|%8s||\n", i, rewardlist[i],poolsizelist[i],boolString(checkNode[i]));
        }else{
            printf("#|           |           |         |        ||\n");
        }
    }
    printf("---------------------------------------------------------------------------------\n");
    printf("||   Active Users  |   Inactive Users  |##|   Active Nodes  |  Inactive Nodes  ||\n");
    printf("||%17d|%19d|##|%17d|%18d||\n",activeUsers,inactiveUsers, activeNodes, inactiveNodes);
    printf("||-----------------------------------------------------------------------------||\n");
    printf("||    Tot Rewards  |%59d||\n",sommaRewards);
    printf("||-----------------------------------------------------------------------------||\n");
    printf("||    Tot Budgets  |%59d||\n",sommaBudget);
    printf("||-----------------------------------------------------------------------------||\n");
    printf("||    Tot Block    |%59d||\n", libroCounter);
    printf("---------------------------------------------------------------------------------\n");

    /*motivo del termine*/
    if(activeUsers==0){
        printf("Motivo di chiusura:tutti gli utenti sono disattivati.\n");
    }else if(libroCounter >= SO_REGISTRY_SIZE){
        printf("Motivo di chiusura: libroMastro pieno.\n");
    }else{
        printf("Simulazione finita perfettamente.\n");
    }
}
```
