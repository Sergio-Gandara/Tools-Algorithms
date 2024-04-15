
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h> 
#include <sys/msg.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

struct mesg_buffer { 
    long mesg_type; 
    char mesg_text[100]; 
} message; 


void putm(int queue_id, char m){
    //strcpy(message.mesg_text, m); // Replace with your data
    message.mesg_text[0]=m;
    int result = msgsnd(queue_id, &message, sizeof(message.mesg_text), 0);
    if (result == -1) {
        perror("msgsnd");
        exit(1);
    }
    printf("Message (%s) sent successfully to %d.\n", message.mesg_text, queue_id);
}


int main(int argc, char *argv[]){

    key_t key_smoke[4]={123401, 123402, 123403, 123404};
    //              tobacco | Phosphorus  | Paper | VIP Queue
    int queue_ids[4];  // Array to store message queue IDs


    for (int i = 0; i < 4; i++) {
        key_t key = key_smoke[i];  // Use appropriate key based on index
        queue_ids[i] = msgget(key, 0666 | IPC_CREAT); 
        if (queue_ids[i] < 0) {
            perror("msgget");
            exit(1);
        }
        printf("Queue created with ID: %d\n", queue_ids[i]);
    }

    if(argc!=1){
        printf("Special behaviour triggered: CLEANSING QUEUES\n");
        for(int j=0; j<4; j++){
            printf("Cleansing queue...\n"); fflush(stdout);

            while(msgrcv(queue_ids[j], &message, sizeof(message), 1, IPC_NOWAIT)!=-1);
            
        }
    }

    printf("Bienvenido a Tabacalera!\n");

    srand(time(NULL));
    char set[3][10]={"tabaco", "fosforo", "papel"}; // 0, 1, 2
    char chosen[2][10];
    int codes[2];
    message.mesg_type=1;
    while(1){
        codes[0]=0; codes[1]=0;
        printf("Producir productos para fumadores? (PULSA ENTER)");
        getchar();
        codes[0]=rand() % 3;
        strcpy(chosen[0], set[codes[0]]);
        do{
            codes[1]=rand() % 3;
            strcpy(chosen[1], set[codes[1]]);
        }while(codes[0]==codes[1]);
        printf("\t(Se han producido: [%s] y [%s] (%d|%d)\n", chosen[0], chosen[1], codes[0], codes[1]);


        putm(queue_ids[codes[0]], '1');
        putm(queue_ids[codes[1]], '1');
        // Pistoletazo de salida
        putm(queue_ids[3], '1'); 
        putm(queue_ids[3], '1');
        putm(queue_ids[3], '1');
        sleep(1);


    }


}