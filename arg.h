#ifndef ARG_H
#define ARG_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "zmq.h"

#define SIZE_LOG 100
#define MSG_SIZE 2048

#define DIM 5 //dimension of playgroud

#define nan 0
#define x 1
#define o 2
#define n 3

#define error(msg)                             \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

typedef struct{
    int game_end; // 0 by deflault
                  // 1,2,3 - players
                  // 4 - playzone is full
    int current_figure;
    int status;
    int coord[DIM][DIM];
    int who_left;
    int count_left;
}Playzone;

typedef struct _msg{
    int id;
    Playzone playzone;
    int playground_id;
    int game_is_ready;
    int ready_notification;
    int coords_sended;
    int x_coord;
    int y_coord;
    int figure;
    void* requester;
    char message[50];
    int debmsg[100];
} Message_t;

void PlayzonePrint(Playzone* p){
    printf("    0   1   2   3   4\n");
    for (int i = 0; i < DIM; ++i) {
        printf("%d ", i);
        for (int j = 0; j < DIM; ++j) {
            if(p->coord[i][j] == 0) printf("| _ ");
            if(p->coord[i][j] == 1) printf("| x ");
            if(p->coord[i][j] == 2) printf("| o ");
            if(p->coord[i][j] == 3) printf("| □ ");
        }
        printf("\n");
    }
}

int PlayzoneIsFull(Playzone* p){
    int null_counter = 0;
    for(int i = 0; i < DIM; i++){
        for(int j = 0; j < DIM; j++){
            if(p->coord[i][j] == nan) null_counter++;
        }
    }
    if(null_counter > 0){
        printf("~~~ Null counter: %d\n", null_counter);
        return 0; 
    }
    printf("~~~Zone is full\n");
    return 1;
}

int PlayzoneCheckForWinner(Playzone* p, int x_coord, int y_coord){
    int curr_fig = p->coord[x_coord][y_coord];
    int bingo = 1;
    // Right diag
    if( ((x_coord-1) >= 0) && ((y_coord+1) < DIM) ){
        if(p->coord[x_coord-1][y_coord+1] == curr_fig){
            bingo++;
            if( ((x_coord-2) < DIM) && ((y_coord+2) >= 0) ){
                if(p->coord[x_coord-2][y_coord+2] == curr_fig){
                    bingo++;
                }
            }
        }
    }

    if( ((x_coord+1) < DIM) && ((y_coord-1) >= 0) ){
        if(p->coord[x_coord+1][y_coord-1] == curr_fig){
            bingo++;
            if( ((x_coord+2) < DIM) && ((y_coord-2) >= 0) ){
                if(p->coord[x_coord+2][y_coord-2] == curr_fig){
                    bingo++;
                }
            }
        }
    }

    if(bingo >= 3){
        printf("WINNER WINNER CHICKEN DINNER!!!\n");
        return 1;
    } else bingo = 1;

    // Left diag
    if( ((x_coord-1) >= 0) && ((y_coord-1) >= 0) ){
        if(p->coord[x_coord-1][y_coord-1] == curr_fig){
            bingo++;
            if( ((x_coord-2) >= 0) && ((y_coord-2) >= 0) ){
                if(p->coord[x_coord-2][y_coord-2] == curr_fig){
                    bingo++;
                }
            }
        }
    }
    if( ((x_coord+1) < DIM) && ((y_coord+1) < DIM) ){
        if(p->coord[x_coord+1][y_coord+1] == curr_fig){
            bingo++;
            if( ((x_coord+2) >= 0) && ((y_coord+2) >= 0) ){
                if(p->coord[x_coord+2][y_coord+2] == curr_fig){
                    bingo++;
                }
            }
        }
    }
    if(bingo >= 3){
        printf("WINNER WINNER CHICKEN DINNER!!!\n");
        return 1;
    } else bingo = 1;
    // Gor
    if((x_coord-1) >= 0){
        if(p->coord[x_coord-1][y_coord] == curr_fig){
            bingo++;
            if((x_coord-2) >= 0){
                if(p->coord[x_coord-2][y_coord] == curr_fig){
                    bingo++;
                }
            }
        }
    }

    if((x_coord+1) < DIM){
        if(p->coord[x_coord+1][y_coord] == curr_fig){
            bingo++;
            if((x_coord+2) < DIM){
                if(p->coord[x_coord-2][y_coord] == curr_fig){
                    bingo++;
                }
            }
        }
    }
    if(bingo >= 3){
        printf("WINNER WINNER CHICKEN DINNER!!!\n");
        return 1;
    } else bingo = 1;
    // Vert

    if((y_coord-1) >= 0){
        if(p->coord[x_coord][y_coord-1] == curr_fig){
            bingo++;
            if((y_coord-2) >= 0){
                if(p->coord[x_coord][y_coord-2] == curr_fig){
                    bingo++;
                }
            }
        }
    }

    if((y_coord+1) < DIM){
        if(p->coord[x_coord][y_coord+1] == curr_fig){
            bingo++;
            if((y_coord+2) < DIM){
                if(p->coord[x_coord][y_coord+2] == curr_fig){
                    bingo++;
                }
            }
        }
    }
    
    if(bingo >= 3){
        printf("WINNER WINNER CHICKEN DINNER!!!\n");
        return 1;
    } else bingo = 1;

    return 0;
}

#endif