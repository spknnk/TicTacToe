#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stropts.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#include <zmq.h>
#include "arg.h"

volatile sig_atomic_t flag = 0;

#define Send_msg(DATA, MESSAGE, INFO, SERVER)                \
    memcpy(DATA->message, INFO, strlen(INFO) + 1);           \
    zmq_msg_init_size(&MESSAGE, sizeof(Message_t));          \
    memcpy(zmq_msg_data(&MESSAGE), DATA, sizeof(Message_t)); \
    zmq_msg_send(&MESSAGE, SERVER, 0);                       \
    zmq_msg_close(&MESSAGE);

void PlayzoneInit(Playzone* p){
	for(int i = 0; i < DIM; i++){
		for(int j = 0; j < DIM; j++){
			p->coord[i][j] = nan;
		}
	}
}

void TCPString(int* currPort, char* sckt){
	char* tcp = "tcp://localhost:";
	char s_currPort[5];
	sprintf(s_currPort, "%d", *currPort);

	snprintf(sckt, 22, "%s%s", tcp, s_currPort);

	printf("Status: %s\n", sckt);
}

void sig_check(int sig){
	switch (sig){
		case SIGINT: {
			if (flag == 0){
				flag = sig;
				puts(" Server blocked!");
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

int main(int argc, char **argv) {
	printf("Server is starting...\n");

	int currPort = 5000;
	char adress[13];
	sprintf(adress, "%s%d", "tcp://*:", currPort);
	printf("Status: %s\n", adress);

	size_t playersInQueue = 0;
	size_t player_id = 0;

	Playzone p;
	PlayzoneInit(&p);
	PlayzonePrint(&p);

	void *context = zmq_ctx_new();
	void *server = zmq_socket(context, ZMQ_REP);

	if (zmq_bind(server, adress)) {
		puts("ERROR: Bind. Server is invalid\n");
	}


	Playzone playground[50];
	char info[90]; // info to client

	for (int i = 0; i < 50; ++i) {
		playground[i].status = 0;
		playground[i].current_figure = 1;
		playground[i].game_end = 0;
		playground[i].who_left = 0;
	}

	int playgroud_iterator = 0;

	while (1) {
		signal(SIGINT, sig_check);
		signal(SIGQUIT, sig_check);

		if (flag == SIGQUIT) {
			zmq_close(server);
			zmq_ctx_destroy(context);
			printf("Server is down\n");
			return 0;
		}

		if (!flag) {
			zmq_msg_t message;

			zmq_msg_init(&message);
			zmq_msg_recv(&message, server, 0);
			Message_t *data = (Message_t *) zmq_msg_data(&message);
			zmq_msg_close(&message);

			if((playground[data->playground_id].current_figure != 0) && (data->playzone.current_figure != 0)){ // F5
			 	 data->playzone.current_figure = playground[data->playground_id].current_figure;
			 	 //data->playzone.game_end = playground[data->playground_id].game_end;
			}
			for(int i = 0; i < DIM; ++i) {
        		for(int j = 0; j < DIM; ++j) {
		            data->playzone.coord[i][j] = playground[data->playground_id].coord[i][j];
		        }
		    }

		    if(data->playzone.who_left != 0){ // F5
			 	playground[data->playground_id].who_left = data->playzone.who_left;
			 	data->playzone.who_left = playground[data->playground_id].who_left; // fix later
			}

			if(data->playzone.game_end != 0)
				playground[data->playground_id].game_end = data->playzone.game_end;

			if(data->playzone.count_left != 0){
				playground[data->playground_id].count_left = data->playzone.count_left;
			}

			printf(">Playzone status: %d\n", data->playzone.status);			
			printf(">ID: %d\n", data->id);
			printf(">FIG: %d\n", data->figure);
			printf(">Game ready status: %d\n", data->game_is_ready);
			printf(">Game ready notific: %d\n", data->ready_notification);
			printf(">Coords: %d %d\n", data->x_coord, data->y_coord);
			printf(">Coords sended: %d\n", data->coords_sended);
			printf("PG ID: %d\n", data->playground_id);
			printf("mdF: %d\n", data->figure);
			printf("dF: %d\n", data->playzone.current_figure);
			printf("pF: %d\n", playground[data->playground_id].current_figure);
			printf("Game_end_data: %d\n", data->playzone.game_end);
			printf("Game_end_playg: %d\n", playground[data->playground_id].game_end);
			printf("\n\nWHO_LEFT: dt%d   pz%d\n\n", data->playzone.who_left, playground[data->playground_id].who_left);
			printf(">-----------------------\n\n");

			if (data->coords_sended == 2) {
				if (data->figure == data->playzone.current_figure) {
					printf("COORDS SENDED CHANGE\n");
					data->playzone.status = playground[data->playground_id].status;
					Send_msg(data, message, "Your turn now!(0)\n", server);
				} else {
					data->playzone.who_left = playground[data->playground_id].who_left;
					data->playzone.status = playground[data->playground_id].status;
					Send_msg(data, message, "Waiting for enemy turn!\n", server);
				}
			}

			if (data->coords_sended == 1) {
				if(playground[data->playground_id].game_end == 4){
					data->playzone.game_end = 4;
	                Send_msg(data, message, "OTHER PLAYERS HAS LEFT THE GAME", server);
	                printf("Game finished!\n");
				}
				if (data->figure == playground[data->playground_id].current_figure) {
					// if (playground[data->playground_id].current_figure < 3) {
					// 	++playground[data->playground_id].current_figure;
					// 	++data->playzone.current_figure;
					// } else {
					// 	playground[data->playground_id].current_figure = 1;
					// 	data->playzone.current_figure = 1;

					if (playground[data->playground_id].current_figure == 1) {
						playground[data->playground_id].current_figure = 2;
						data->playzone.current_figure = 2;
						if(playground[data->playground_id].who_left == 2){
							playground[data->playground_id].current_figure = 3;
							data->playzone.current_figure = 3;
						}

					} else {

						if (playground[data->playground_id].current_figure == 2) {
							playground[data->playground_id].current_figure = 3;
							data->playzone.current_figure = 3;
							if(playground[data->playground_id].who_left == 3){
								playground[data->playground_id].current_figure = 1;
								data->playzone.current_figure = 1;
							}

						} else {
							if (playground[data->playground_id].current_figure == 3) {
								playground[data->playground_id].current_figure = 1;
								data->playzone.current_figure = 1;
								if(playground[data->playground_id].who_left == 1){
									playground[data->playground_id].current_figure = 2;
									data->playzone.current_figure = 2;
								}
							}
						}
						
					}

					if(playground[data->playground_id].who_left != 0){
						Send_msg(data, message, ">>>WARNING:PLAYER LEFT THE GAME", server);
					}

					printf("\n\n\n>>>>>>>>>>>>>>>>dF: %d\n", data->playzone.current_figure);
					printf(">>>>>>>>>>>>>>>>pF: %d\n", playground[data->playground_id].current_figure);
					printf("Client ID: %d\n", data->id);
					printf("Coordinates: %d %d\n", data->x_coord, data->y_coord);
					if ((data->x_coord >= 0) && (data->y_coord >= 0)) {

						data->playzone.who_left = playground[data->playground_id].who_left;
						data->playzone.count_left = playground[data->playground_id].count_left;
						
						if(PlayzoneIsFull(&playground[data->playground_id]) == 0){
							playground[data->playground_id].coord[data->x_coord][data->y_coord] = data->figure;
							data->playzone.coord[data->x_coord][data->y_coord] = data->figure;
							if(PlayzoneCheckForWinner(&playground[data->playground_id], data->x_coord, data->y_coord) == 1){
	                            data->playzone.status = 4;
	                            playground[data->playground_id].status = 4;
	                            Send_msg(data, message, "", server);
                        	}
						} else {
							//TODO
							printf("Game finished!\n");
							//return 0;
						}
					}

					for (int i = 0; i < DIM; ++i) {
						for (int j = 0; j < DIM; ++j) {
							data->playzone.coord[i][j] = playground[data->playground_id].coord[i][j];
						}
					}

					PlayzonePrint(&playground[data->playground_id]);
					printf("\n\n1MY PG STATUS IS: %d\n", playground[data->playground_id].status);
					printf("2MY PG STATUS IS: %d\n\n", data->playzone.status);
					printf("!!!!MSG0\n");
					Send_msg(data, message, "Nice turn!\n", server);
				} else {
					printf("~~~~~~~~~~~~~~~~~~\n");
					Send_msg(data, message, "It's not your turn now!\n", server);
				}
			} else {
				if (data->id == 0) {

					printf("New user!\n");
					player_id++;
					playersInQueue++;
					data->id = player_id;
					data->playground_id = playgroud_iterator;
					printf(">Number of players: %ld\n", player_id);

					if ((playersInQueue != 3) && (data->game_is_ready == 0)) {
						sprintf(info, "%s%d%c", "Sorry, you wasn't registred. Now you're allowed to play!\nYour ID:",
								data->id, '\0');
						printf("!!!!MSG1\n");
						Send_msg(data, message, info, server);
					}

				}
				// Checking if game is ready (for all players)
				if ((data->playzone.status == 1) && (data->ready_notification == 0)) {

					data->game_is_ready = 1;
					data->ready_notification = 1;
					printf("!!!!MSG2\n");
					Send_msg(data, message, "Your game is ready!(1)\0", server);

				}

				if ((playersInQueue != 3) && (data->game_is_ready == 0)) {

					if(data->playzone.who_left != 0){ // F5
					 	playground[data->playground_id].who_left = data->playzone.who_left;
					 	data->playzone.who_left = playground[data->playground_id].who_left; // fix later
					}

					if(data->playzone.game_end != 0)
						playground[data->playground_id].game_end = data->playzone.game_end;

					if(data->playzone.count_left != 0){
						playground[data->playground_id].count_left = data->playzone.count_left;
					}

					data->playzone = playground[playgroud_iterator];
					data->playzone.status = playground[data->playground_id].status; ///////////////////////////////////////////////////////////////
					printf(">Players in queue: %ld\n", playersInQueue);
					printf("!!!!MSG3\n");
					Send_msg(data, message, "Waiting for other players...\0", server);

				}

				if (playersInQueue == 3) {

					if ((data->coords_sended == 3) || (playground[data->playground_id].status == 5)){
						playground[data->playground_id].status = 5;
						data->playzone.status = playground[data->playground_id].status;
						Send_msg(data, message, "Somebody left the game(2)!\n", server);
					}

					printf("Game %d is ready!\n", playgroud_iterator);
					PlayzoneInit(&playground[playgroud_iterator]);
					PlayzonePrint(&playground[playgroud_iterator]);

					playground[playgroud_iterator].status = 1;
					data->playzone = playground[playgroud_iterator];
					data->game_is_ready = 1;

					playgroud_iterator++;
					playersInQueue = 0;
					data->ready_notification = 1;
					printf("!!!!MSG4\n");
					Send_msg(data, message, "Your game is ready!(2)\0", server);
				}
			}
			data->coords_sended = 0;
		}
			sleep(1);
	}

		zmq_close(server);
		zmq_ctx_destroy(context);
		printf("Server is down!\n");
		return 0;
}
