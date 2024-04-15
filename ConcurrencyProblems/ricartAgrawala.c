#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <pthread.h>

#include <semaphore.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdbool.h>


sem_t comprueba__quiero_entrar; //Limita la concurrencia al acceso al entero 'quiero_entrar'
sem_t seccion_critica;
sem_t dentro_seccion;

int quiero_entrar=0; //Indica si el nodo quiere entrar a la sección crítica, sirve para que 'process_mesage' haga una cosa u otra
int estoy_dentro =0; //Indica si el nodo está dentro de la sección crítica, sirve para no enviar ACK's

int total_nodes=0; //Numero total de nodos
int contador_procesos_ACK=0; //Cuenta el numero de nodos que enviaron el ACK para poder entrar a la sección crítica

char myID[10]; //myID en string
int myID_int=0; //myID en entero

int myticket=0; //Ticket

bool *response_mask; //Array de booleanos para saber que nodo envío ACK y quien no

key_t eth_key=12345; //Key de la cola de mensajes
int eth0;  //ID de la cola de mensajes


//convierte un entero a cadena de caracteres
char *itoa(int num) {
    // Reservar memoria para la cadena de caracteres
    char *str = (char *)malloc(12); // Suficiente para almacenar cualquier número entero

    // Verificar si malloc tuvo éxito
    if (str == NULL) {
        fprintf(stderr, "Error: No se pudo asignar memoria.\n");
        exit(EXIT_FAILURE);
    }

    // Convertir el número a una cadena
    sprintf(str, "%d", num);

    // Devolver la cadena convertida
    return str;
}

struct mesg_buffer { 
    long mesg_type; 
    char mesg_text[100]; 
} message; 

/*
    Envía un mensaje a la cola de mensajes que se indica
    el char 'm' sirve para indicar que tipo de mensaje es 
    el target es el 'tipo' de mensaje para distinguir los nodos
*/
char* send(int queue_id, char m, int target){
    
    /*PAYLOAD SPECS
        -A-BBBB-CCCC.    :     -CODE-myID-myTicket.

        CODES:

            R -> REQUEST | request access to CRITSEC
            A -> ACK     | acknowledge message correctly processed 
            D -> DROP    | drop access of CRITSEC
    */

    strcpy(message.mesg_text,"");

    char payload[100]=""; //Inicializamos a array vacío para no detectar "basura"
    payload[0]='-'; 
    payload[1]=m;
    payload[2]='-';
    strcat(payload, itoa(myID_int));
    strcat(payload, "-");
    strcat(payload, itoa(myticket));
    strcat(payload, ".");

    message.mesg_type=target;

    strcpy(message.mesg_text,payload);
    
    int result = msgsnd(queue_id, &message, sizeof(message), 0);
    if (result == -1) {
        perror("msgsnd");
        exit(1);
    }
    printf("[SND: (%s) (200)]\n", message.mesg_text);
    return "def";
}

//procesa los mensajes recibidos
void process_message(){
    //-A-BBBB-CCCC.    :     -CODE-myID-myTicket.

    char CODE=' ';
    char ID_mensaje_recibido[4]=""; //Hay que crear espacio para 5 caracteres debido al caracter de final de cadena
    char ticket_recibido[4]="";
    int ID_mensaje_recibido_int;

    int contador_caracteres_ID = 0;
    int contador_caracteres_ticket = 0;
    int contador_separador = 0;

    for(int i = 0; i < strlen(message.mesg_text); i++){

        char comparar = message.mesg_text[i];

        //Si es un '.' es el final de la cadena y sale del bucle
        //Si es un '-' aumenta en '1' el contador de separadores
        if(comparar == '.'){
            break;
        }else if(comparar == '-'){
            contador_separador++;
        }else{

            if(contador_separador == 1){
                CODE = comparar;
                
            }else if(contador_separador == 2){
                ID_mensaje_recibido[contador_caracteres_ID] = comparar;
                contador_caracteres_ID++;
                
            }else if(contador_separador == 3){
                ticket_recibido[contador_caracteres_ticket] = comparar;
                contador_caracteres_ticket++;
            }
        }
    }


    ID_mensaje_recibido_int = atoi(ID_mensaje_recibido);

    switch (CODE){
    case 'R': 
        //Si está dentro de la sección y recibe un 'Request' el nodo no envía ACK
        sem_wait(&dentro_seccion);
        if(estoy_dentro){
            sem_post(&dentro_seccion);
            break;
        }
        sem_post(&dentro_seccion);

        //Si el nodo quiere entrar y recibe 'R' -----> COMPRUEBA LOS TICKETS
        sem_wait(&comprueba__quiero_entrar);
        if(quiero_entrar){
            //Si el ticket recibido es menor => envía ACK, en caso contrario no envía nada
            if(atoi(ticket_recibido) < myticket){
                //Compruebo si ese nodo me había dado permiso en el pasado, en caso de que sea así me "autoquito" su permiso ya que su ticket es menor
                if(response_mask[ID_mensaje_recibido_int] == 1){
                    response_mask[ID_mensaje_recibido_int] = 0;
                    contador_procesos_ACK--;
                } 
                send(eth0,'A',ID_mensaje_recibido_int);
                printf("[%d] envio ACK - \n", myID_int);
            }else{
                sem_post(&comprueba__quiero_entrar);
                break;
            }
        }
        sem_post(&comprueba__quiero_entrar);
        
        //Si mi ticket = 0 => No quiero entrar => Envío ACK
        if(myticket == 0){
            send(eth0,'A',ID_mensaje_recibido_int);
            printf("[%d] envio ACK - \n", myID_int);
        }

        break;
    case 'A':
        sem_wait(&comprueba__quiero_entrar);
        if(quiero_entrar){
            response_mask[ID_mensaje_recibido_int] = 1;
            contador_procesos_ACK++;
            if(contador_procesos_ACK == total_nodes-1){ // el -1 es porque el ID del proceso propio no cuenta
                sem_post(&seccion_critica);
            }
        }
        sem_post(&comprueba__quiero_entrar);
        break;
    case 'D':
        break;
    default:
        break;
    }

    return;
}



void *recv(){ 
    
    while(1){

        strcpy(message.mesg_text, "");
        msgrcv(eth0, &message, sizeof(message), myID_int, 0); //Bloqueante para no consumir CPU
        printf("[RECV (%s) (200)]\n", message.mesg_text);
        process_message();
        printf("-------------------\n");
    }

}


int main(int argc, char * argv[]){

    if(argc<3){
        printf("ERROR - Incorrect call\n./RAnode ID NUMBER_OF_NODES\n");
        return 1;
    }

    //Crea o Elimina la cola de mensajes
    if(strcmp(argv[1], "S")==0){ // spawn queue
        eth0 = msgget(eth_key, 0666 | IPC_CREAT); 
        printf("Special behaviour triggered: spawning queue...\n");
        return 0;
    }else if(strcmp(argv[1], "C")==0) { // cleanse queue
        eth0 = msgget(eth_key, 0666 | IPC_CREAT);
        msgctl(eth0,IPC_RMID,0);
        printf("Special behaviour triggered: cleansing queue...\n");
        return 0;
    }

    eth0 = msgget(eth_key, 0666 | IPC_CREAT); //Obtiene la cola de mensajes

    strcpy(myID, argv[1]); // Stablish myID
    myID_int=atoi(myID);
    if(myID_int == 0){ //Es un error pq el tipo de mensaje del struct (que para nosotros es el ID del nodo) no puede ser 0
        printf("ERROR - Incorrect node ID (has to be > 0)");
        return 1;
    }

    sem_init(&comprueba__quiero_entrar, 0, 1);
    sem_init(&seccion_critica, 0, 0);
    sem_init(&dentro_seccion, 0, 1);

    total_nodes=atoi(argv[2]); // Stablish number of total nodes
    response_mask=malloc(sizeof(bool)*total_nodes); //Se inicializa el array de booleanos que comprueba que nodos enviaron ACK


    // 3- Init async RECV thread
    pthread_t recv_th;
    pthread_create(&recv_th, NULL, recv, NULL); // NULL param for thread OK ?

    srand(time(NULL)); //"INICIALIZA" LA FUNCION rand()

    while(1){

        myticket = 0; //Siempre que se vuelve a entrar al bucle se "desecha" el ticket anterior
        response_mask[myID_int]=1; //Se inicializa a '1' el booleano con el ID del nodo propio ("yo me doy permiso a mi mismo siempre")

        printf("[%d] WORKING - \n", myID_int); 
        getchar();
        
        // 4- CRITSEC control
        printf("[%d] solving a CRITSEC contest...- \n", myID_int);
        getchar();

        sem_wait(&comprueba__quiero_entrar);
        quiero_entrar=1;
        myticket = rand() % 9000; //esta linea no seria necesaria meterla en el semaforo (?)
        sem_post(&comprueba__quiero_entrar);
        
        //Envía 'Request' a todos los nodos
        for(int i=1; i<=total_nodes; i++){
            if(i != myID_int){
                send(eth0, 'R', i);
            }
        }
        
        //Espera a que todos los nodos envíen ACK
        sem_wait(&seccion_critica);

        sem_wait(&dentro_seccion);
        estoy_dentro = 1;
        sem_post(&dentro_seccion);

        //CRITSEC


        // 5- CRITSEC access        
        printf("[%d] I am in the critic section my friend \n", myID_int);
        getchar();


        // 6- CRITSEC exit
        printf("[%d] I am out \n", myID_int);
        getchar();
        
        
        sem_wait(&dentro_seccion);
        estoy_dentro = 0;
        sem_post(&dentro_seccion);
            
        sem_wait(&comprueba__quiero_entrar);
        quiero_entrar=0;
        sem_post(&comprueba__quiero_entrar);

        contador_procesos_ACK = 0; //"Reiniciamos" esta variable para que cuando vuelva a solicitar entrar a la sección crítica esté a '0'

        //Reiniciamos el array de booleanos
        for(int i=1; i < total_nodes; i++){
            response_mask[i] = 0;
        }
        
        //Enviamos ACK's a todos los nodos => Indica que hemos abandonado la sección crítica
        for(int i=1; i<=total_nodes; i++){
            if(i != myID_int){
                send(eth0, 'A', i);
            }
        }

    }
    return 0;
}