#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include "game.h"
#include "minimax.h"

void start(){
    Game *g = malloc(sizeof(Game));
    init_game(g);
    get_hand_sizes(&g->hands);
    get_hands(&g->hands);
    printf("Game starts with turn : ");
    scanf("%d", &g->turn);
    int n, cant_pass, ai, depth;
    Move moves[DCOUNT], move;
    do {
        print_game(g);
        get_moves(g, moves, &n, &cant_pass);
        print_moves(moves, n);
        printf("%d moves\n", n);
        if(cant_pass) printf("cant pass\n");
        if(n == 0 && g->hands.hand_sizes[NP] == 0){
            pass(g);
            continue;
        }
        if(cant_pass && n == 1){
            play_move(g, moves[0]);
            continue;
        }
        printf("give move type (left = %d, right = %d", LEFT, RIGHT);
        if(!cant_pass)
            printf(", perfect pick = %d, imperfect pick = %d, pass = %d", PERFECT_PICK, IMPERFECT_PICK, PASS);
        printf("): ");
        scanf("%d", &move.type);
        switch(move.type){
        case LEFT:
        case RIGHT:
            scanf("%d %d", &move.play.left, &move.play.right);
            if(playable_move(&g->snake, move.type, move.play.left, move.play.right) && possible_possession(g->turn, &g->hands, move.play.left, move.play.right)){
                play_move(g, move);
            } else {
                printf("invalid move\n");
            }
            break;
        case PERFECT_PICK:
            if(cant_pass){
                printf("invalid move\n");
                break;
            }
            scanf("%d %d", &move.perfect_pick.left, &move.perfect_pick.right);
            if(possible_possession(NP, &g->hands, move.perfect_pick.left, move.perfect_pick.right))
                perfect_pick(g, move);
            else
                printf("invalid move\n");
            break;
        case IMPERFECT_PICK:
            if(cant_pass){
                printf("invalid move\n");
                break;
            }
            scanf("%d", &move.imperfect_pick.count);
            if(move.imperfect_pick.count > 0 && move.imperfect_pick.count <= g->hands.hand_sizes[NP])
                imperfect_pick(g, move);
            else
                printf("invalid move\n");
            break;
        case PASS:
            if(!cant_pass){
                printf("invalid move\n");
                break;
            }
            pass(g);
            break;
        default: // play AI move
            scanf("%d", &depth);
            move = best_move(g, depth);
            printf("play the move? : ");
            scanf("%d", &ai);
            if(ai){
                play_move(g, move);
            }
        }
    } while(!over(g));
    print_game(g);
    printf("game over, score: %f\n", endgame_evaluation(g));
}

int main(){
    init_fact();
    start();
    getch();
    return 0;
}