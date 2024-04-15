#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>

#include <semaphore.h>


//int priority_pass[1];


sem_t sem; 
sem_t stop;
sem_t *rIn;
sem_t *rOut;
sem_t *wIn;
sem_t *wOut;
sem_t mutex;

sem_t sem_w;
int writerN=0; // Number of writers in queue
int readerN=0;
int wp=0;
int rp=0;
int win=0;

int rapier=0;

void dgt(int maxnum){

    rIn=realloc(rIn, maxnum*sizeof(sem_t));
    for(int i=0; i<maxnum; i++){
        sem_init(&rIn[i], 0, 0);
    }

    rOut=realloc(rOut, maxnum*sizeof(sem_t));
    for(int i=0; i<maxnum; i++){
        sem_init(&rOut[i], 0, 0);
    }

    wIn=realloc(wIn, maxnum*sizeof(sem_t));
    for(int i=0; i<maxnum; i++){
        sem_init(&wIn[i], 0, 0);
    }

    wOut=realloc(wOut, maxnum*sizeof(sem_t));
    for(int i=0; i<maxnum; i++){
        sem_init(&wOut[i], 0, 0);
    }

    

    return;
}

void *lector(void *arg){
    int *mypt=(int*)arg;
    
    while(1){
        printf("[Lector %d] -> Esperando a intentar leer... \n", *mypt); fflush(stdout);
        sleep(1);
        sem_wait(&rIn[*mypt]); // Espera señal del user
        
        printf("[Lector %d] -> Intentando leer... \n", *mypt); fflush(stdout);
        readerN++;
        while(1){
            if((writerN!=0)) {sem_post(&sem_w); }
            if((writerN==0) && (win==0)) {break;}
            printf("Me duermo...\n");
            if(win!=0){
                printf("Me dormí\n"); rapier++;
                sem_wait(&stop);
            }
            
        }
        printf("Me despierto\n"); rapier--;
        readerN--;
        rp++;
        sem_wait(&sem); 
        
        printf("[Lector %d] -> Leyendo \n", *mypt); fflush(stdout);
        
        sem_wait(&rOut[*mypt]); // Ahora debe esperar por la luz verde para salir 
        printf("[Lector %d] -> Fin lectura\n", *mypt); fflush(stdout);
        sem_post(&sem); // Sal y deja paso libre
        rp--;
        if(wp!=0) sem_post(&mutex);
        // rapier??
        if(readerN!=0) sem_post(&stop);
        
        sleep(1);
    }

    pthread_exit((void*)NULL);
}

void *escritor(void *arg){
    int *mypt=(int*)arg;
    
    
    while(1){
        printf("[escritor %d] -> Esperando a intentar escribir... \n", *mypt); fflush(stdout);
        sleep(1);
        sem_wait(&wIn[*mypt]); writerN++;

        
        printf("[escritor %d] -> Intentando escribir... \n", *mypt); fflush(stdout);
        sem_wait(&sem_w); 
        if(rp){
            wp++;
            sem_wait(&mutex);
        }
        wp--;
        sem_wait(&sem); win++; writerN--;
        
        printf("[escritor %d] -> Escribiendo \n", *mypt); fflush(stdout);
        

        sem_wait(&wOut[*mypt]); // Ahora debe esperar por la luz verde para salir 


        printf("[escritor %d] -> Fin escritura\n", *mypt); fflush(stdout);
        
        sem_post(&sem); // Sal y deja paso libre
        win--;
        if(writerN!=0){
            sem_post(&sem_w);
        }
        if(readerN!=0) sem_post(&stop);
        if(rapier!=0){
            printf("Hay alguien dormido!\n");
            sem_post(&stop);
        }
        if(wp!=0) sem_post(&mutex);
        

        sleep(1);
    }

    pthread_exit((void*)NULL);
}

int main(int argc, char *argv[]){
    if(argc!=4){
        printf("ERROR: ./lectores N1 N2\n>N1: MAX_LECT (HOW MANY)\n>N2: SIZE_LECT (HOW MANY AT THE SAME TIME)\n");
        printf(">N3: MAX_ESCR (HOW MANY WRITERS)\n");
        return 1;
    }
    
    // PRIORITY
    int K=0;
    int MAX_ESCR=atoi(argv[3]);
    
    

    int MAX_LECT=atoi(argv[1]);
    int SIZE_LECT=atoi(argv[2]);
    
    int SIZE_ESCR=atoi(argv[3]);
    printf("[!] MAX: %d, SIZE LECT: %d , MAX ESCR: %d\n", MAX_LECT, SIZE_LECT, MAX_ESCR);

    // sem_init(&sem, COMPARTICION?, VALOR_MAXIMO); 
    sem_init(&sem, 0, SIZE_LECT);
    sem_init(&sem_w, 0, 1);
    sem_init(&stop, 0, 1); // 1?
    sem_init(&mutex, 0, 0);
    dgt(MAX_LECT); // Esto parará a todos los threads que intenten entrar hasta que se les dé luz verde


    // CREATE READERS
    pthread_t silk[MAX_LECT];
    int matrix[MAX_LECT];
    for(int i=0; i<MAX_LECT; i++) matrix[i]=i;
    for(int i=0; i<MAX_LECT; i++) pthread_create(&silk[i], NULL, lector, (void*)&matrix[i]);
    // CREATE WRITERS
    pthread_t silk_w[MAX_ESCR];
    int matrix_w[MAX_ESCR];
    for(int i=0; i<MAX_ESCR; i++) matrix_w[i]=i;
    for(int i=0; i<MAX_ESCR; i++) pthread_create(&silk_w[i], NULL, escritor, (void*)&matrix_w[i]);
    // STABLISH GAPS
    

    char option='0';
    int out=0;
    int selector=0;
    while(1){
        if(out==1) break;
        sleep(1);
        printf("Escritores (E) o Lectores (L)? : ");
        option=getchar();
        while(getchar()!='\n');
        if(option=='L'){ // READER -------------------------------------------------
                printf("1. Intentar leer\n2. Finalizar leer\n3. Salir\n$>  ");
            option=getchar();
            while(getchar()!='\n');
            switch(option){
                case '1':
                    printf("> LEER (%c)  > introduzca el numero del lector (de 1 a %d): ", option, SIZE_LECT + 1);
                    selector=getchar() - '0' - 1;
                    printf("lector Nº %d selccionado ...\n", selector);
                    sem_post(&rIn[selector]);
                    // TODO : READERS IN

                    break;
                case '2':
                    printf("> FINALIZAR (%c)  > introduzca el número del lector (de 1 a %d): ", option, SIZE_LECT +1) ;
                    selector=getchar() - '0' - 1;
                    printf("lector Nº %d selccionado ...\n", selector + 1);
                    sem_post(&rOut[selector]);
                    // TODO: End Read
                    break;
                case '3':
                    printf("> SALIR (%c)\n", option);
                    out=1;
                    break;
                default:
                    printf("Unknown option");
                    break;
            }
            while(getchar()!='\n');
        } // END LECTORES
        else if(option=='E'){ // WRITERS ----------------------------------------------------------------
            if(out==1) break;
            printf("1. Intentar escribir\n2. Finalizar escribir\n3. Salir\n$>  ");
            option=getchar();
            while(getchar()!='\n');
            switch(option){
                case '1':
                    printf("> escribir (%c)  > introduzca el numero del escritor (de 1 a %d): ", option, SIZE_ESCR + 1);
                    selector=getchar() - '0' - 1;
                    printf("escritor Nº %d selccionado ...\n", selector + 1);
                    sem_post(&wIn[selector]);
                    // TODO : WRITERS IN
                    
                    break;
                case '2':
                    printf("> FINALIZAR (%c)  > introduzca el número del escritor (de 1 a %d): ", option, SIZE_ESCR  + 1);
                    selector=getchar() - '0' - 1;
                    printf("escritor Nº %d selccionado ...\n", selector  + 1);
                    sem_post(&wOut[selector]);
                    break;
                case '3':
                    printf("> SALIR (%c)\n", option);
                    out=1;
                    break;
                default:
                    printf("Unknown option");
                    break;
            }
            while(getchar()!='\n');
        }

        
    }

    return 0;
}