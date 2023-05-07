#include "game.h"

unsigned long long FACTORIAL[DCOUNT];

void init_fact(){
    FACTORIAL[0] = 1;
    for(int i = 1; i < DCOUNT; i++){
        FACTORIAL[i] = FACTORIAL[i-1] * i;
    }
}

// initialize a game
void init_game(Game *g) {
    init_blocks(g->block_set);
    g->snake.head = NULL;
    g->snake.tail = NULL;
    g->pass_counter = 0;
    init_hands(&g->hands);
}

int playable_move(Snake *s, enum Type type, int left, int right) {
    if(s->head == NULL)
        return 1;
    switch(type) {
        case LEFT:
            return s->tail->domino.left == right;
        case RIGHT:
            return s->head->domino.right == left;
        default:
            return 1;
    }
}

int symmetric_move(Snake *s, enum Type type, int left, int right) { // tests whether this move can be played on both sides as is.
    if(s->head == NULL)
        return 1;
    switch(type) {
        case LEFT:
            return s->head->domino.right == left;
        case RIGHT:
            return s->tail->domino.left == right;
        default:
            return 1;
    }
}

int playable_domino(Snake *s, int left, int right) {
    return s->head == NULL || left == s->head->domino.left || left == s->tail->domino.right \
            || right == s->head->domino.left || right == s->tail->domino.right;
}

int is_passing(Game *g, int player){ // test whether a player will pass if given the turn for certain
    int left = g->snake.tail->domino.left;
    int right = g->snake.head->domino.right;
    for(int i = 0; i < PIPS; i++){
        if(possible_possession(player, &g->hands, i, left) || possible_possession(player, &g->hands, i, right))
            return 0;
    }
    return 1;
}

void print_game(Game *g){
    printf("-------------------------------------------------------------\n");
    // print the hands then the snake.
    for(int i = 0; i <= NP; i++) {
        print_hand(&g->hands, i);
    }
    print_snake(&g->snake);
    // print whose turn it is and the pass counter. also print possible_move moves for the current player.
    printf("Turn: %d\n", g->turn);
    printf("Pass Counter: %d\n", g->pass_counter);
}

void get_moves(Game *g, Move moves[MAX], int *n, int *cant_pass){
    *n = 0;
    *cant_pass = 0;
    if(g->snake.head == NULL){
        for(int i = 0; i < PIPS; i++){
            for(int j = 0; j <= i; j++){
                if(possible_possession(g->turn, &g->hands, i, j)){
                    *cant_pass = 1;
                    moves[*n].play.left = i;
                    moves[*n].play.right = j;
                    moves[*n].type = LEFT;
                    (*n)++;
                }
            }
        }
        return;
    }
    int left = g->snake.tail->domino.left;
    int right = g->snake.head->domino.right;
    for(int i = 0; i < PIPS; i++){
        if(possible_possession(g->turn, &g->hands, i, left) && playable_move(&g->snake, LEFT, i, left)){
            *cant_pass = *cant_pass || certain(&g->hands, i, left);
            moves[*n].play.left = i;
            moves[*n].play.right = left;
            moves[*n].type = LEFT;
            (*n)++;
        }
    }
    if(left != right){
        for(int i = 0; i < PIPS; i++){
            if(possible_possession(g->turn, &g->hands, right, i) && playable_move(&g->snake, RIGHT, right, i)){
                *cant_pass = *cant_pass || certain(&g->hands, right, i);
                moves[*n].play.left = right;
                moves[*n].play.right = i;
                moves[*n].type = RIGHT;
                (*n)++;
            }
        }
    }
}

float pass_probability(Game *g, int n){// n is the number of distinct playable dominoes by the player
    if(n <= 0)
        return 1.0f;
    int k = g->hands.hand_sizes[g->turn] - g->hands.solid_hand_sizes[g->turn]; // number of unknown dominoes
    int m = g->hands.liquid_hand_sizes[g->turn] - n; // number of unknown dominoes that are not playable
    if(k > m)
        return 0.0f;
    return  ((float) FACTORIAL[m] / FACTORIAL[m-k]) * ((float) FACTORIAL[m + n - k] / FACTORIAL[m + n]);
}


void play_move(Game *g, Move move){
    add_block(&g->snake, g->block_set[move.play.left][move.play.right], move.type);
    //clear_owner(g->hands, move.play.left, move.play.right);
    clear_owner_play(g->turn, &g->hands, move.play.left, move.play.right);
    g->hands.hand_sizes[g->turn]--;
    emit_collapse(&g->hands);
    g->turn = (g->turn + 1) % NP;
    g->pass_counter = 0;
}

void unplay_move(Game *g, Move move, Hands *prev, int prev_pc){
    remove_block(&g->snake, move.type);
    g->turn = (g->turn - 1 + NP) % NP;
    g->hands = *prev;
    g->pass_counter = prev_pc;
}

// this is a bad name
void absent(Game *g){ // this makes all playable dominoes not in this hand, used in passes and imperfect picks
    int left = g->snake.tail->domino.left;
    int right = g->snake.head->domino.right;
    if(left == right){
        for(int i = 0; i < PIPS; i++){
            //printf("clearing %d %d\n", i, left);
            absent_piece(g->turn, &g->hands, i, left);
            //print_hand(&g->hands, g->turn);
        }
    } else {
        for(int i = 0; i < PIPS; i++){
            //printf("clearing %d %d and %d %d\n", i, left, right, i);
            absent_piece(g->turn, &g->hands, i, left);
            absent_piece(g->turn, &g->hands, i, right);
            //print_hand(&g->hands, g->turn);
        }
    }
}

void absence_event(Game *g){
    absent(g);
    emit_collapse(&g->hands);
}

void undo_absence_event(Game *g, Hands *prev){ // this needs an anchor to undo the function above, but kept here if implementation changes.
    g->hands = *prev;
}

// this is also a really bad name
void unabsent(Game *g){ // this isn't the inverse of the function above, this function just makes dominoes that are in boneyard possible in this hand except for playable ones, only used in imperfect picks    
    int right = g->snake.head->domino.right, left = g->snake.tail->domino.left;
    for(int i = 0; i < PIPS; i++){
        for(int j = 0; j <= i; j++){
            if(i != left && i != right && j != left && j != right && possible_possession(NP, &g->hands, i, j)){
                //printf("setting %d %d\n", i, j);
                set_possible_owner_pick(g->turn, &g->hands, i, j);
                //print_hand(&g->hands, g->turn);
            }
        }
    }
}

void pick_liquid_update(Game *g){
    absent(g);
    unabsent(g);
}

void pass(Game *g){
    absent(g);
    emit_collapse(&g->hands);
    g->turn = (g->turn + 1) % NP;
    g->pass_counter++;
}

void undo_pass(Game *g, Hands *prev){
    g->turn = (g->turn - 1) % NP;
    g->pass_counter--;
    g->hands = *prev;
}

void perfect_pick(Game *g, Move move){
    if(g->hands.liquid_hand_sizes[g->turn])
        absent(g);
    set_sole_owner_pick(g->turn, &g->hands, move.perfect_pick.left, move.perfect_pick.right);
    g->hands.hand_sizes[g->turn]++;
    g->hands.hand_sizes[NP]--;
    emit_collapse(&g->hands);
}

void undo_perfect_pick(Game *g, Hands *prev){
    g->hands = *prev;
}

void imperfect_pick(Game *g, Move move){
    pick_liquid_update(g);
    g->hands.hand_sizes[g->turn] += move.imperfect_pick.count;
    g->hands.hand_sizes[NP] -= move.imperfect_pick.count;
    emit_collapse(&g->hands);
}

void undo_imperfect_pick(Game *g, Hands *prev){
    g->hands = *prev;
}

int over(Game *g){
    if(g->pass_counter == NP)
        return 1;
    for(int i = 0; i < NP; i++){
        if(g->hands.hand_sizes[i] == 0)
            return 1;
    }
    return 0;
}
