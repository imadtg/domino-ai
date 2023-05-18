#include "move.h"

void print_playing_moves(Move moves[], int n){
    printf("Possible playing moves: ");
    for(int i = 0; i < n; i++) {
        printf("[%d|%d] ", moves[i].play.left, moves[i].play.right);
    }
    printf("\n");
}

void print_picking_moves(Move moves[], int n){
    printf("Possible picking moves: ");
    for(int i = 0; i < n; i++) {
        printf("[%d|%d] ", moves[i].perfect_pick.left, moves[i].perfect_pick.right);
    }
    printf("\n");
}