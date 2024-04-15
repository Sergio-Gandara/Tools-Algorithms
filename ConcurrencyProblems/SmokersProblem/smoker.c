
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h> 
#include <sys/msg.h>

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

void putm2(int queue_id, char m, int typ){
    //strcpy(message.mesg_text, m); // Replace with your data
    message.mesg_text[0]=m;
    int cache=message.mesg_type; message.mesg_type=typ;
    int result = msgsnd(queue_id, &message, sizeof(message.mesg_text), 0);
    if (result == -1) {
        perror("msgsnd");
        exit(1);
    }
    message.mesg_type=cache;
    printf("Message (%s) sent successfully to %d.\n", message.mesg_text, queue_id);
}


int main(int argc, char* argv[]){

    if(argc!=2){
        printf("Error! Not enough arguments!\n./smoker (A|B|C)\n");
        return -1;
    }

    key_t key_smoke[4]={123401, 123402, 123403, 123404};
    //              tobacco | Phosphorus  | Paper | VIP Queue
    int queue_ids[4];  // Array to store message queue IDs



    for (int i = 0; i < 4; i++) {
        key_t key = key_smoke[i];  // Use appropriate key based on index
        queue_ids[i] = msgget(key, 0666); 
        if (queue_ids[i] < 0) {
            perror("msgget");
            exit(1);
        }
        printf("Queue acquired with ID: %d\n", queue_ids[i]);
    }

    char identity=argv[1][0];

    message.mesg_type=1;
    int bias=0;
    switch(identity){
        case 'A': // Papel y fosforo (Tengo tabaco)

            while(1){
                message.mesg_text[0]='0';
                printf("[O] Voy a ver si hay tabaco! (*cof *cof)\n");
                msgrcv(queue_ids[3], &message, sizeof(message), 1, 0);
                message.mesg_text[0]='0';
                bias=0;
                while(1){
                    msgrcv(queue_ids[3], &message, sizeof(message), 9, IPC_NOWAIT);
                    if(message.mesg_text[0]=='C') bias++;
                    printf("[REC] %c", message.mesg_text[0]);
                    message.mesg_text[0]='0';
                    msgrcv(queue_ids[3], &message, sizeof(message), 8, IPC_NOWAIT);
                    if(message.mesg_text[0]=='B') bias++;
                    printf("[REC] %c", message.mesg_text[0]);
                    message.mesg_text[0]='0';
                    if(bias==2){
                        printf("FED UP\n");
                        break;
                    }
                    printf("¡"); fflush(stdout); sleep(1);
                    msgrcv(queue_ids[0], &message, sizeof(message), 1, IPC_NOWAIT);
                    if(message.mesg_text[0]=='1') break;
                }
                
                
                if(message.mesg_text[0]=='1'){
                    printf("Hay tabaco! A mi no me toca fumar!\n");
                    putm2(queue_ids[3], 'A', 7);
                    putm(queue_ids[0], '1'); // Devuelvo lo otro
                    sleep(1);
                    continue;
                }
                

                printf("[O] Quiero papel! (*cof *cof)\n");
                msgrcv(queue_ids[2], &message, sizeof(message), 1, 0); 
                printf("[1/2] Tengo papel!!!\n ");
                
                printf("[O] Quiero fosforo! (*cof *cof)\n");
                msgrcv(queue_ids[1], &message, sizeof(message), 1, 0); 
                printf("[2/2] Tengo fosforo!!!\n ");


                printf("[FUMANDO] Fumador A fumando...\n");
                //putm(queue_ids[3], '1');
                //putm(queue_ids[3], '1');
                sleep(1);
            }

        break;

        case 'B': // Papel y tabaco (Tengo fosforo)

            while(1){
                message.mesg_text[0]='0';
                printf("[O] Voy a ver si hay fosforo! (*cof *cof)\n");
                msgrcv(queue_ids[3], &message, sizeof(message), 1, 0);
                message.mesg_text[0]='0';
                bias=0;
                while(1){
                    msgrcv(queue_ids[3], &message, sizeof(message), 7, IPC_NOWAIT);
                    if(message.mesg_text[0]=='A') bias++;
                    printf("[REC] %c", message.mesg_text[0]);
                    message.mesg_text[0]='0';
                    msgrcv(queue_ids[3], &message, sizeof(message), 9, IPC_NOWAIT);
                    if(message.mesg_text[0]=='C') bias++;
                    printf("[REC] %c", message.mesg_text[0]);
                    message.mesg_text[0]='0';
                    if(bias==2){
                        printf("FED UP\n");
                        break;
                    }
                    printf("¡"); fflush(stdout); sleep(1);
                    msgrcv(queue_ids[1], &message, sizeof(message), 1, IPC_NOWAIT);
                    if(message.mesg_text[0]=='1') break;
                }
                
                
                if(message.mesg_text[0]=='1'){
                    printf("Hay fosforo! A mi no me toca fumar!\n");
                    putm2(queue_ids[3], 'B', 8);
                    putm(queue_ids[1], '1'); // Devuelvo lo otro
                    sleep(1);
                    continue;
                }
                
                
                
                printf("[SYNC]\n");
                

                printf("[O] Quiero papel! (*cof *cof)\n");
                msgrcv(queue_ids[2], &message, sizeof(message), 1, 0); 
                printf("[1/2] Tengo papel!!!\n ");
                
                printf("[O] Quiero tabaco! (*cof *cof)\n");
                msgrcv(queue_ids[0], &message, sizeof(message), 1, 0); 
                printf("[2/2] Tengo tabaco!!!\n ");


                printf("[FUMANDO] Fumador B fumando...\n");
                //putm(queue_ids[3], '1');
                //putm(queue_ids[3], '1');
                sleep(1);
            }

        break;

        case 'C': // Tabaco y fósforo (Tengo papel)
            
            while(1){
                message.mesg_text[0]='0';
                printf("[O] Voy a ver si hay papel! (*cof *cof)\n");
                
                msgrcv(queue_ids[3], &message, sizeof(message), 1, 0);
                message.mesg_text[0]='0';
                bias=0;
                while(1){
                    msgrcv(queue_ids[3], &message, sizeof(message), 7, IPC_NOWAIT);
                    if(message.mesg_text[0]=='A') bias++;
                    printf("[REC] %c", message.mesg_text[0]);
                    message.mesg_text[0]='0';
                    msgrcv(queue_ids[3], &message, sizeof(message), 8, IPC_NOWAIT);
                    if(message.mesg_text[0]=='B') bias++;
                    printf("[REC] %c", message.mesg_text[0]);
                    message.mesg_text[0]='0';
                    if(bias==2){
                        printf("FED UP\n");
                        break;
                    }
                    printf("¡"); fflush(stdout); sleep(1);
                    msgrcv(queue_ids[2], &message, sizeof(message), 1, IPC_NOWAIT);
                    if(message.mesg_text[0]=='1') break;
                }
                
                
                if(message.mesg_text[0]=='1'){
                    printf("Hay papel! A mi no me toca fumar!\n");
                    putm2(queue_ids[3], 'C', 9);
                    putm(queue_ids[2], '1'); // Devuelvo lo otro
                    sleep(1);
                    continue;
                }

                printf("[O] Quiero tabaco! (*cof *cof)\n");
                msgrcv(queue_ids[0], &message, sizeof(message), 1, 0); 
                printf("[1/2] Tengo tabaco!!!\n ");
                
                printf("[O] Quiero fosforo! (*cof *cof)\n");
                msgrcv(queue_ids[1], &message, sizeof(message), 1, 0); 
                printf("[2/2] Tengo fosforo!!!\n ");


                printf("[FUMANDO] Fumador A fumando...\n");
                //putm(queue_ids[3], '1');
                //putm(queue_ids[3], '1');
                sleep(1);
            }

        break;

        default:
            printf("Unknown option\n");
            return -1;
        break;
    }
/*
    while(1){
        message.mesg_text[0]='0';
        printf("[O] Quiero ");
        msgrcv(queue_id, &message, sizeof(message), 1, 0);


        printf("[O] Quiero ");
        msgrcv(queue_id, &message, sizeof(message), 1, 0);
    }
*/

}
