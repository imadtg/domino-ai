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
    get_hand(&g->hands, 1);
    printf("Game starts with turn : ");
    fflush(stdout);
    scanf("%d", &g->turn);
    int n, cant_pass, ai_play, skip, depth;
    float (*ai_function)(Game *, int, int, int *);
    enum Mode ai_mode;
    Move moves[MAX_NUM_PLY_MOVE], move;
    do {
        print_game(g);
        get_playing_moves(g, moves, &n, &cant_pass);
        print_playing_moves(moves, n);
        printf("%d moves\n", n);
        if(cant_pass) printf("cant pass\n");
        if(n == 0){
            if(g->hands.hand_sizes[NP]){
                if(is_passing(g, NP)){
                    Move pick_all_of_boneyard;
                    pick_all_of_boneyard.type = IMPERFECT_PICK;
                    pick_all_of_boneyard.imperfect_pick.count = g->hands.hand_sizes[NP];
                    imperfect_pick(g, pick_all_of_boneyard);
                    continue;
                }
                else if(g->hands.hand_sizes[NP] == 1){
                    Move pick_one_of_boneyard;
                    int dummy;
                    pick_one_of_boneyard.type = PERFECT_PICK;
                    get_perfect_picking_moves(g, &pick_one_of_boneyard, &dummy); // VERY UNSAFE
                    perfect_pick(g, pick_one_of_boneyard);
                    continue;
                }
            } else {
                pass(g);
                continue;
            }
        } else if (cant_pass && n == 1){
            play_move(g, moves[0]);
            continue;
        }
        if(!cant_pass && n == 0)
            printf("give move type (perf pick = %d, imp pick = %d, pass = %d): ", PERFECT_PICK, IMPERFECT_PICK, PASS);
        else if(!cant_pass)
            printf("give move type (left = %d, right = %d, perf pick = %d, imp pick = %d, pass = %d)(AI def): ", LEFT, RIGHT, PERFECT_PICK, IMPERFECT_PICK, PASS);
        else
            printf("give move type (left = %d, right = %d)(AI def): ", LEFT, RIGHT);
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
            if(cant_pass){
                printf("invalid move\n");
                break;
            }
            pass(g);
            break;
        default: // AI move
            if(n == 0){
                printf("reading 4 ints for consistency:");
                scanf("%*d %*d %*d %*d");
                printf("no moves to choose AI move from\n");
                break;
            }
            printf("give ai mode (pessimist = %d, expect = %d): ", PESSIMIST, EXPECT);
            scanf("%d", &ai_mode);
            printf("skip exploring picking positions?: ");
            scanf("%d", &skip);
            printf("give depth of search (0 is iterative deepening, negative is infinite): ");
            scanf("%d", &depth);
            switch(ai_mode){
            case PESSIMIST:
                ai_function = minimax;
                break;
            case EXPECT:
                ai_function = expectiminimax;
                break;
            }
            if(depth != 0) move = best_move(g, moves, NULL, n, depth, skip, NULL, ai_function);
            else move = iterative_deepening(g, moves, n, skip, ai_function);
            printf("play the move?: ");
            scanf("%d", &ai_play);
            printf("\n");
            if(ai_play){
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