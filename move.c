#include "move.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#define KEEPALIVE EMSCRIPTEN_KEEPALIVE
#else
#define KEEPALIVE
#endif

KEEPALIVE
void print_playing_moves(Move moves[], int n){
    printf("Possible playing moves: ");
    for(int i = 0; i < n; i++) {
        printf("[%d|%d] ", moves[i].play.left, moves[i].play.right);
    }
    printf("\n");
}

KEEPALIVE
void print_picking_moves(Move moves[], int n){
    printf("Possible picking moves: ");
    for(int i = 0; i < n; i++) {
        printf("[%d|%d] ", moves[i].perfect_pick.left, moves[i].perfect_pick.right);
    }
    printf("\n");
}