#include <inttypes.h>
#include <pthread.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <zmq.h>

#include "arg.h"

volatile sig_atomic_t flag = 0;
int im_left = 0;

void sig_check(int sig){
	switch (sig){
		case SIGINT: {
			if (flag == 0){
				flag = sig;
				im_left++;
				puts(":(");
			} else if (flag == sig){
				flag = 0;
				puts(" Server unblocked!");
			}
			break;
		}
		case SIGQUIT:
			flag = sig;
	}
}

void* SendMessage(void* arg, Message_t* p){
    Message_t* data = (Message_t*)arg;
    zmq_msg_t message;

    zmq_msg_init_size(&message, sizeof(Message_t));
    memcpy(zmq_msg_data(&message), data, sizeof(Message_t));
    zmq_msg_send(&message, data->requester, 0);
    zmq_msg_close(&message);

    zmq_msg_init(&message);
    zmq_msg_recv(&message, data->requester, 0);
    data = (Message_t*)zmq_msg_data(&message);

    printf("%s\n", data->message);
    p->id = data->id;
    p->playground_id = data->playground_id;
    p->game_is_ready = data->game_is_ready;
    //p->playzone = data->playzone;
    p->playzone.status = data->playzone.status;
    p->playzone.current_figure = data->playzone.current_figure;
    p->playzone.game_end = data->playzone.game_end;
    for(int i = 0; i < DIM; ++i) {
        for(int j = 0; j < DIM; ++j) {
            p->playzone.coord[i][j] = data->playzone.coord[i][j];
        }
    }
    p->ready_notification = data->ready_notification;
    p->coords_sended = data->coords_sended;

    p->playzone.who_left = data->playzone.who_left;
    p->playzone.count_left =  data->playzone.count_left;

    zmq_msg_close(&message);
    return 0;
}

int main() {
    void* context = zmq_ctx_new();
    void* server = zmq_socket(context, ZMQ_REQ);

    //save id,sckt in file 
    // IS READY TO PLAY
    int sckt = 5000;
    // FILE *f = fopen("id.txt", "r"); 
    // fscanf(f, "%d %d", &id, &sckt);

    char adress[25];
    sprintf(adress, "%s%d", "tcp://localhost:", sckt);
    zmq_connect(server, adress);
    printf("\nConnected to %s\n\n", adress);


    /* ZMQ */
    Message_t data;
    data.id = 0; // not registred on server
    data.game_is_ready = 0;
    data.ready_notification = 0;
    data.requester = server;
    data.coords_sended = 0;

    data.x_coord = -1;
    data.y_coord = -1;
    data.figure = -1;
    int game_is_ready_flag = 0;
    int info_status = 0;

    data.playzone.status = 0; // 4 - winner found
    data.playzone.who_left = 0;
    data.playzone.game_end = 0;
    data.playzone.count_left = 0;

while(1) {
    //Debugging
    printf("%d.---------------------\n", info_status);
    printf("ID: %d\n", data.id);
    printf("Game ready status: %d\n", data.game_is_ready);
    printf("Game ready notific: %d\n", data.ready_notification);
    printf("Playzone status: %d\n", data.playzone.status);
    printf("Figure: %d\n", data.figure);
    printf("dF: %d\n", data.playzone.current_figure);
    printf("PG ID: %d\n", data.playground_id);
    printf("Game end: %d\n", data.playzone.game_end);
    printf("-----------------------\n");
    info_status++;

    	signal(SIGINT, sig_check);
		signal(SIGQUIT, sig_check);

        if((im_left == 1) && (data.playzone.who_left != 0)){
            data.playzone.game_end = 4;
            data.playzone.count_left++;
            SendMessage(&data, &data);
            printf("Disconnected(0.0)!\n");
            return 0;
        }

		if((im_left == 1) && (data.playzone.who_left == 0)){
			data.playzone.who_left = data.figure;
            data.playzone.count_left++;
			SendMessage(&data, &data);
			printf("Disconnected(0.1)!\n");
            return 0;
		}

        if(data.playzone.game_end == 4){
            printf("GAME END\n");
            return 0;
        }

        if((data.game_is_ready == 1) && (data.id != 0)) {
            data.x_coord = -1;
            data.y_coord = -1;

            if(data.figure == -1){
                if(data.id % 3 == 0) data.figure = x;
                if(data.id % 3 == 1) data.figure = o;
                if(data.id % 3 == 2) data.figure = n;
            }   

            int wrong_coords = 0;
coords:
                while((data.x_coord < 0) || (data.y_coord < 0) || (data.x_coord > 4) || (data.y_coord > 4)) {
                    printf("MYFIGURE: %d\n", data.figure);
                    if(wrong_coords == 1){
                        printf("Enter correct coordinates!\n");
                        wrong_coords = 0;
                    } 
                    
                    if(data.figure != data.playzone.current_figure){
                        //printf("Waiting for enemy turn(1)...\n");
                        //while(data.figure != data.playzone.current_figure){
                        printf("mdF: %d\n", data.figure);
                        printf("df: %d\n", data.playzone.current_figure);
                            sleep(3);
                            data.x_coord = -1;
                            data.y_coord = -1;
                            data.coords_sended = 2;

                            signal(SIGINT, sig_check);
                            signal(SIGQUIT, sig_check);

                            if((im_left == 1) && (data.playzone.who_left != 0)){
                                data.playzone.game_end = 4;
                                data.playzone.count_left++;
                                SendMessage(&data, &data);
                                printf("Disconnected(1.0)!\n");
                                return 0;
                            }

                            if((im_left == 1) && (data.playzone.who_left == 0)){
                                data.playzone.who_left = data.figure;
                                data.playzone.count_left++;
                                SendMessage(&data, &data);
                                printf("Disconnected(1.1)!\n");
                                return 0;
                            }

                            if((data.playzone.game_end == 4) || (data.playzone.count_left == 2)){
                                printf("GAME END\n");
                                return 0;
                            }

                            signal(SIGINT, sig_check);
                            signal(SIGQUIT, sig_check);

                            if((im_left == 1) && (data.playzone.who_left != 0)){
                                data.playzone.game_end = 4;
                                data.playzone.count_left++;
                                SendMessage(&data, &data);
                                printf("Disconnected(1.0)!\n");
                                return 0;
                            }

                            printf("!!!MESSAGE SENDED1.5!!!\n");
                            SendMessage(&data, &data);
                    }

                    if(data.figure == data.playzone.current_figure){
                        PlayzonePrint(&data.playzone);

                        if((PlayzoneIsFull(&data.playzone) == 1) || (data.playzone.game_end == 4)){
                            printf("\n\nPlayzone is full. Game finished!\n");
                            data.playzone.game_end = 4;
                            data.coords_sended = 1;
                            printf("!!!MESSAGE SENDED FULL!!!\n");
                            SendMessage(&data, &data);
                            return 0;
                         }

                        if(data.playzone.status == 4){
                            printf("BETTER LUCK NEXT TIME!\n");
                            data.coords_sended = 1;
                            printf("!!!MESSAGE SENDED LOSTGAME!!!\n");
                            SendMessage(&data, &data);
                            return 0;
                        }

                        if(data.playzone.game_end == 4){
                            printf("Other players left the game. Game finished!\n");
                            return 0;
                        }

                        printf("~Please, enter coordinates of your figure: ");
                        wrong_coords = 1;
                        scanf("%d %d", &data.x_coord, &data.y_coord);
                        if((data.playzone.coord[data.x_coord][data.y_coord] != 0) || (data.x_coord > 4) || (data.y_coord > 4)){
                            
                            data.x_coord = -1;
                            data.y_coord = -1;
                            goto coords;
                        }
                        data.playzone.coord[data.x_coord][data.y_coord] = data.figure;
                        PlayzonePrint(&data.playzone);
                        //CHECK FOR WINNER
                        if(PlayzoneCheckForWinner(&data.playzone, data.x_coord, data.y_coord) == 1){
                            //TODO
                            printf("IM SENDING INFO THAT OTHER PLAYERS LOSE\n");                         
                            data.playzone.status = 4;
                            data.coords_sended = 1;
                            printf("!!!MESSAGE SENDED WINNER!!!\n");
                            SendMessage(&data, &data);
                            return 0;
                        }
                        data.coords_sended = 1;
                        printf("!!!MESSAGE SENDED1!!!\n");
                        SendMessage(&data, &data);
                    }
            }


    } else {
        printf("IM IN ELSE\n");
        if((data.game_is_ready == 1) && (game_is_ready_flag == 0)) { // Start of the game
            game_is_ready_flag++;
            PlayzonePrint(&data.playzone);
        }  
        
        if(data.id == 0) {
            printf("Wait a moment...\n");
            printf("!!!MESSAGE SENDED2!!!\n");
            SendMessage(&data, &data);
        }
        printf("INFO STATUS: %d\n", info_status-1);

        while((data.game_is_ready == 0) && (data.id != 0)){
            sleep(2);
            data.x_coord = -1;
            data.y_coord = -1;

            signal(SIGINT, sig_check);
            signal(SIGQUIT, sig_check);

            if((im_left == 1) && (data.playzone.who_left != 0)){
                printf("....here1\n");
                data.coords_sended = 1;
                data.figure = 2;
                data.playzone.game_end = 4;
                data.playzone.count_left++;
                SendMessage(&data, &data);
                printf("Disconnected(2.0)!\n");
                return 0;
            }
            
            if((im_left == 1) && (data.playzone.who_left == 0)){
                printf("....here2\n");
                data.coords_sended = 1;
                data.figure = 1;
                data.playzone.who_left = data.figure;
                data.playzone.count_left++;
                SendMessage(&data, &data);
                printf("Disconnected(2.1)!\n");
                return 0;
            }

            printf("!!!MESSAGE SENDED3!!!\n");
            SendMessage(&data, &data);
        }
    }
    //sleep(1);
}
/////////
    zmq_close(server);
    zmq_ctx_destroy(context);
    return 0;
}