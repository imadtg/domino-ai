#include "move.h"

void print_moves(Move moves[], int n){
    printf("Possible moves: ");
    for(int i = 0; i < n; i++) {
        printf("[%d|%d] ", moves[i].play.left, moves[i].play.right);
    }
    printf("\n");
}