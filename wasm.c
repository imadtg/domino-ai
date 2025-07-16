// this is a file only used to expose some utility functions to be able to use with wasm output of emcc
// this file should only appear if compiled with emcc
#ifdef __EMSCRIPTEN__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include "game.h"
#include "minimax.h"
#include <emscripten/emscripten.h>
#define KEEPALIVE EMSCRIPTEN_KEEPALIVE

KEEPALIVE
int get_number_of_players(){
    return NP;
}

KEEPALIVE
int get_pips(){
    return PIPS;
}

KEEPALIVE
int get_current_player(){
    return 1;
}

KEEPALIVE
Game *alloc_game(){ // TODO: handle failures on this side of the interface
    return malloc(sizeof(Game));
}

KEEPALIVE
Hands *get_hands(Game *game){
    return &game->hands;
}

KEEPALIVE
void set_hand_size(Game *game, int player, int size){
    game->hands.hand_sizes[player] = size;
}

KEEPALIVE
void set_turn(Game *game, int player){
    game->turn = player;
}

KEEPALIVE
int get_turn(Game *game){
    return game->turn;
}

KEEPALIVE
int *alloc_int(){ // yes, i don't know of a better way...
    return malloc(sizeof(int));
}

KEEPALIVE
int deref_int(int *ptr){
    return *ptr;
}

KEEPALIVE
Move *alloc_max_move_arr(){
    return malloc(sizeof(Move) * DCOUNT);
}

KEEPALIVE
Move *alloc_move(){
    return malloc(sizeof(Move));
}

KEEPALIVE
void pick_all_boneyard(Game *game){
    Move pick_all_of_boneyard;
    pick_all_of_boneyard.type = IMPERFECT_PICK;
    pick_all_of_boneyard.imperfect_pick.count = game->hands.hand_sizes[NP];
    imperfect_pick(game, pick_all_of_boneyard);
}

KEEPALIVE
int get_hand_size(Game *game, int player){
    return game->hands.hand_sizes[player];
}

KEEPALIVE
Move *get_move_by_index(Move move_array[], int index){
    return &move_array[index];
}

KEEPALIVE
void play_move_by_pointer(Game *game, Move *move){
    play_move(game, *move);
}

KEEPALIVE
void perfect_pick_by_pointer(Game *game, Move *move){
    perfect_pick(game, *move);
}

KEEPALIVE
void imperfect_pick_by_pointer(Game *game, Move *move){
    imperfect_pick(game, *move);
}

KEEPALIVE
Snake *get_snake(Game *game){
    return &game->snake;
}

KEEPALIVE
void populate_move_from_components(Move *move, enum Type type, int left, int right){
    move->type = type;
    move->play.left = left;
    move->play.right = right;
}

KEEPALIVE
void populate_imperfect_picking_move(Move *move, int amount){
    move->type = IMPERFECT_PICK;
    move->imperfect_pick.count = amount;
}

KEEPALIVE
void populate_move_by_ai(Game *game, Move *move, Move moves[], int n, int depth, float *score, int *nodes){
    printf("DEBUGGING PRINTS:\n");
    print_game(game);
    print_playing_moves(moves, n);
    printf("depth: %d, movecount: %d\n", depth, n);
    printf("move pointer: %p\n", move);
    float scores[n];
    *nodes = 0;
    // TODO: delegate choice of AI function and the skip flag to this wrapper's parameters instead of hardcoding them...
    Move best = best_move(game, moves, scores, n, depth, 1, nodes, expectiminimax);
    for(int i = 0; i < n; i++){
        if(best.play.left == moves[i].play.left && best.play.right == moves[i].play.right && best.type == moves[i].type){
            printf('best move: ');
            *score = scores[i];
        }
        printf("score of move [%d|%d] %d: %f\n", moves[i].play.left, moves[i].play.right, moves[i].type, scores[i]);
    }
    printf("best move found: [%d|%d] %d: %f\n", best.play.left, best.play.right, best.type, *score);
    *move = best;
    printf("END OF DEBUGGING PRINTS.\n");
}

KEEPALIVE
int get_left_of_move(Move *move){
    return move->play.left;
}

KEEPALIVE
int get_right_of_move(Move *move){
    return move->play.right;
}

KEEPALIVE
int get_type_of_move(Move *move){
    return move->type;
}

KEEPALIVE
enum Type get_LEFT(){
    return LEFT;
}

KEEPALIVE
enum Type get_RIGHT(){
    return RIGHT;
}

KEEPALIVE
enum Type get_PERFECT_PICK(){
    return PERFECT_PICK;
}

KEEPALIVE
enum Type get_IMPERFECT_PICK(){
    return IMPERFECT_PICK;
}

KEEPALIVE
enum Type get_PASS(){
    return PASS;
}

#endif