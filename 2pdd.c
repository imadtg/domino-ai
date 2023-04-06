#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#define PIPS 5
#define MAX (PIPS*2+1)
#define NP 2
#define TOTAL (PIPS*(PIPS+1)*(PIPS+2)/2)
#define DCOUNT (PIPS*(PIPS+1)/2)

enum Type {LEFT, RIGHT, PERFECT_PICK, IMPERFECT_PICK, PASS}; 

int FACTORIAL[DCOUNT];

typedef struct {
    int left;
    int right;
} Domino;

typedef struct Block{
    Domino domino;
    struct Block *left_block, *right_block;
} Block;

typedef struct {
    Block* head;
    Block* tail;
} Snake;

typedef struct {
    int ownership[PIPS][PIPS];
    int hand_sizes[NP+1];
    // this is the sum of weights of all the dominoes that the player can have,
    // not to be confused with the sum of weights of the dominoes in the player's hand
    int liquid_weights[NP+1];
    int liquid_hand_sizes[NP+1]; // this is the number of distinct dominoes the player can have, not to be confused with the number of dominoes in the player's hand
    int solid_weights[NP+1]; // this is the sum of weights of all the dominoes that we are sure the player has
    int solid_hand_sizes[NP+1]; // this is the number of distinct dominoes that we are sure the player has
    int boneyard_solid_group_weights[PIPS]; // this is the weight of each group of dominoes by pip value that we are sure is in the boneyard, used to calculate average hand weight after picking up from a boneyard.
    int boneyard_solid_group_sizes[PIPS]; // this is the number of dominoes in each group of dominoes by pip value that we are sure is in the boneyard, used to calculate average hand weight after picking up from a boneyard.
} Hands;

typedef struct Game {
    Snake snake;
    Hands hands;
    //int outed[PIPS]; // this is the number of dominoes that have a pip value of i that are out of the game
    int turn;
    int pass_counter;
    Block *block_set[PIPS][PIPS];
} Game;

typedef Domino playing_move;
typedef Domino perfect_picking_move;

typedef struct {
} passing_move;

typedef struct {
    int count;
} imperfect_picking_move;

typedef struct {
    enum Type type;
    union {
        playing_move play;
        passing_move pass;
        imperfect_picking_move imperfect_pick;
        perfect_picking_move perfect_pick;
    };
} Move;

int calc_solid_weight(int player, Hands *hands){
    int w = 0;
    for(int i = 0; i < PIPS; i++){
        for(int j = 0; j <= i; j++){
            if(sole_possession(player, hands, i, j)){
                w += i + j;
            }
        }
    }
    return w;
}

int calc_liquid_weight(int player, Hands *hands, int *size){
    int w = 0;
    *size = 0;
    for(int i = 0; i < PIPS; i++){
        for(int j = 0; j <= i; j++){
            if(possible_possession(player, hands, i, j) && !certain(hands, i, j)){
                w += i + j;
                (*size)++;
            }
        }
    }
    return w;
}

float calc_collapsed_liquid_weight(int player, Hands *hands, int collapsing_size){
    int size, w;
    w = calc_liquid_weight(player, hands, &size);
    if(!size)
        return 0.0;
    return (float) (w * collapsing_size) / size;   
}

float calc_true_liquid_weight(int player, Hands *hands){
    return calc_collapsed_liquid_weight(player, hands, hands->hand_sizes[player] - hands->solid_hand_sizes[player]);
}

float calc_weight(int player, Hands *hands){
    return calc_solid_weight(player, hands) + calc_true_liquid_weight(player, hands);
}

void increment_solid_player(int player, Hands *hands, int i, int j){
    hands->solid_hand_sizes[player]++;
    hands->solid_weights[player] += i + j;
    if(hands->liquid_hand_sizes[player] && hands->hand_sizes[player] == \
                                            hands->solid_hand_sizes[player] + hands->liquid_hand_sizes[player])
        collapse_hand(player, hands, i, j);
}

void decrement_solid_player(int player, Hands *hands, int i, int j){
    hands->solid_hand_sizes[player]--;
    hands->solid_weights[player] -= i + j;
}

void increment_liquid(int player, Hands *hands, int i, int j){
    hands->liquid_hand_sizes[player]++;
    hands->liquid_weights[player] += i + j;
}

void decrement_liquid(int player, Hands *hands, int i, int j){
    hands->liquid_hand_sizes[player]--;
    hands->liquid_weights[player] -= i + j;
    if(hands->liquid_hand_sizes[player] && hands->hand_sizes[player] == \
                                            hands->solid_hand_sizes[player] + hands->liquid_hand_sizes[player])
        collapse_hand(player, hands, i, j);
}

void increment_solid_boneyard(Hands *hands, int i, int j){
    hands->boneyard_solid_group_sizes[i]++;
    hands->solid_hand_sizes[NP]++;
    hands->boneyard_solid_group_weights[i] += i + j;
    hands->boneyard_solid_group_weights[j] += (i != j) * (i + j);
    hands->solid_weights[NP] += i + j;
    if(hands->liquid_hand_sizes[NP] && hands->hand_sizes[NP] == \
                                            hands->solid_hand_sizes[NP] + hands->liquid_hand_sizes[NP])
        collapse_hand(NP, hands, i, j);
}

void decrement_solid_boneyard(Hands *hands, int i, int j){
    hands->boneyard_solid_group_sizes[i]--;
    hands->solid_hand_sizes[NP]--;
    hands->boneyard_solid_group_weights[i] -= i + j;
    hands->boneyard_solid_group_weights[j] -= (i != j) * (i + j);
    hands->solid_weights[NP] -= i + j;
}

void increment_solid(int player, Hands *hands, int i, int j){
    if(player == NP)
        increment_solid_boneyard(hands, i, j);
    else
        increment_solid_player(player, hands, i, j);
}

void decrement_solid(int player, Hands *hands, int i, int j){
    if(player == NP)
        decrement_solid_boneyard(hands, i, j);
    else
        decrement_solid_player(player, hands, i, j);
}

void convert_to_liquid_player(int player, Hands *hands, int i, int j){
    decrement_solid_player(player, hands, i, j);
    increment_liquid(player, hands, i, j);
}

void convert_to_solid_player(int player, Hands *hands, int i, int j){
    decrement_liquid(player, hands, i, j);
    increment_solid_player(player, hands, i, j);
}

void convert_to_solid_boneyard(Hands *hands, int i, int j){
    decrement_liquid(NP, hands, i, j);
    increment_solid_boneyard(hands, i, j);
}

void convert_to_liquid_boneyard(Hands *hands, int i, int j){
    decrement_solid_boneyard(hands, i, j);
    increment_liquid(NP, hands, i, j);
}

void convert_to_solid(int player, Hands *hands, int i, int j){
    if(player == NP)
        convert_to_solid_boneyard(hands, i, j);
    else
        convert_to_solid_player(player, hands, i, j);
}

void convert_to_liquid(int player, Hands *hands, int i, int j){
    if(player == NP)
        convert_to_liquid_boneyard(hands, i, j);
    else
        convert_to_liquid_player(player, hands, i, j);
}

void set_ownership(Hands *hands, int v, int i, int j){
    hands->ownership[i][j] = v;
    hands->ownership[j][i] = v;
}

void or_ownership(Hands *hands, int v, int i, int j){
    hands->ownership[i][j] |= v;
    hands->ownership[j][i] |= v;
}

void and_ownerhip(Hands *hands, int v, int i, int j){
    hands->ownership[i][j] &= v;
    hands->ownership[j][i] &= v;
}

int possible_possession(int player, Hands *hands, int i, int j){
    return hands->ownership[i][j] & (1 << player);
}

int sole_possession(int player, Hands *hands, int i, int j){
    return hands->ownership[i][j] == (1 << player);
}

int no_possession(Hands *hands, int i, int j){
    return !hands->ownership[i][j];
}

// essentially tests if we know for certain where a piece is, 
//whether in a player's hand, the boneyard, or already played.
int certain(Hands *hands, int i, int j){ 
    return !(hands->ownership[i][j] & (hands->ownership[i][j] - 1));
}

// perfect boneyard picks
void set_sole_owner_pick(int player, Hands *hands, int i, int j){ // needs not be optimized
    if(certain(hands, i, j))
        decrement_solid_boneyard(hands, i, j);
    else
        decrement_liquid(NP, hands, i, j);
    increment_solid_player(player, hands, i, j);
    set_ownership(hands, (1 << player), i, j);
}

void collapse_piece(int player, Hands *hands, int i, int j){ // collapse piece to the player
    if(certain(hands, i, j) || !possible_possession(player, hands, i, j))
        return;
    for(int p = 0; p <= NP; p++){
        if(possible_possession(p, hands, i, j) && p != player)
            decrement_liquid(p, hands, i, j);
    }
    convert_to_solid(player, hands, i, j);
}

void absent_piece(int player, Hands *hands, int i, int j){ // piece is not in the player's hand
    if(!possible_possession(player, hands, i, j))
        return;
    if(certain(hands, i, j))
        decrement_solid_player(player, hands, i, j);
    else 
        decrement_liquid(player, hands, i, j);
}

// game start, sets the piece to be owned by the player
void set_sole_owner_start(int player, Hands *hands, int i, int j){
    increment_solid_player(player, hands, i, j);
    set_ownership(hands, (1 << player), i, j);
}

// game start, sets the piece to not be owned by this person in the game start,
// essentially sets it as possible to be owned by everyone other than this player,
// never used on a boneyard. assumes that there is atleast two possible owners outside of this player.
void set_outside_owner_start(int player, Hands *hands, int i, int j){ // needs not be optimized
    for(int p = 0; p <= NP; p++){
        if(p != player)
            increment_liquid(p, hands, i, j);
    }
    set_ownership(hands, (1 << (NP + 1)) - 1 - (1 << player), i, j);
}

void set_possible_owner_pick(int player, Hands *hands, int i, int j){ // imperfect picks, needs not be optimized
    if(possible_possession(player, hands, i, j))
        return;
    if(certain(hands, i, j))
        convert_to_liquid_boneyard(hands, i, j);
    increment_liquid(player, hands, i, j);
    or_ownership(hands, (1 << player), i, j);
}

void clear_owner_pass(int player, Hands *hands, int i, int j){ // passes, and hand collapses.
    if(!possible_possession(player, hands, i, j))
        return;
    and_ownerhip(hands, ~(1 << player), i, j);
    decrement_liquid(player, hands, i, j);
    if(certain(hands, i, j)){
        for(int p = 0; p < NP; p++){
            if(possible_possession(p, hands, i, j)){
                convert_to_solid_player(p, hands, i, j);
                return;
            }
        }
        convert_to_solid_boneyard(hands, i, j);
    }
}

void clear_owner_play(int player, Hands *hands, int i, int j){ // plays
    if(certain(hands, i, j)){
        decrement_solid_player(player, hands, i, j);
    }
    else {
        for(int p = 0; p <= NP; p++){
            if(possible_possession(p, hands, i, j))
                decrement_liquid(p, hands, i, j);
        }
    }
    set_ownership(hands, 0, i, j);
}

void destroy_boneyard(Hands *hands){
    for(int i = 0; i < PIPS; i++){
        for(int j = 0; j <= i; j++){
            if(possible_possession(NP, hands, i, j)){
                and_ownerhip(hands, ~(1 << NP), i, j);
                if(certain(hands, i, j)){
                    for(int p = 0; p < NP; p++){
                        if(possible_possession(p, hands, i, j)){
                            convert_to_solid_player(p, hands, i, j);
                            break;
                        }
                    }
                }
            }
        }
    }
}

void collapse_hand(int player, Hands *hands, int i, int j){
    int less, more;
    if(i < j){
        less = i;
        more = j;
    } else {
        less = j;
        more = i;
    }
    for(int k = 0; k < PIPS; k++){
        for(int l = 0; l <= i; l++){
            if(possible_possession(player, hands, i, j) && k != more && l != less && l != more){
                collapse_piece(player, hands, i, j);
            }
        }
    }
}

int solid_weight(Game *g, int player){
    return g->hands.solid_weights[player];
}

float collapsed_liquid_weight(Game *g, int player){
    if(g->hands.liquid_hand_sizes[player] == 0)
        return 0.0f;
    return (float) (g->hands.liquid_weights[player] * (g->hands.hand_sizes[player] - g->hands.solid_hand_sizes[player])) / g->hands.liquid_hand_sizes[player];
}

float weight(Game *g, int player){
    return collapsed_liquid_weight(g, player) + solid_weight(g, player);
}

void init_blocks(Block *block_set[PIPS][PIPS]){
    for (int i = 0; i < PIPS; i++) {
        for (int j = 0; j < PIPS; j++) {
            block_set[i][j] = (Block*) malloc(sizeof(Block));
            block_set[i][j]->domino.left = i;
            block_set[i][j]->domino.right = j;
            block_set[i][j]->left_block = NULL;
            block_set[i][j]->right_block = NULL;
        }
    }
}

// initialize a hands struct
void init_hands(Hands *hands){
    memset(hands, 0, sizeof(*hands));
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

int playable_domino(Snake *s, int left, int right) {
    return s->head == NULL || left == s->head->domino.left || left == s->tail->domino.right \
            || right == s->head->domino.left || right == s->tail->domino.right;
}

void print_snake(Game *g) {
    Block *b = g->snake.tail;
    printf("Snake: ");
    while(b!=NULL){
        printf("[%d|%d]", b->domino.left, b->domino.right);
        b = b->right_block;
    }
    printf("\n");
}

void print_hand(Game *g, int i) {
    if(i == NP) printf("Boneyard:");
    else printf("Player %d:", i);
    for(int j = 0; j < PIPS; j++) {
        for(int k = 0; k <= j; k++) {
            if(possible_possession(i, &g->hands, k, j)){
                printf(" [%d|%d]", k, j);
                if(certain(&g->hands, k, j))
                    printf("*");
            }
        }
    }
    int sw = solid_weight(g, i);
    float lw = collapsed_liquid_weight(g, i);
    int tsw = calc_solid_weight(i, &g->hands);
    float tlw = calc_true_liquid_weight(i, &g->hands);
    printf(" \nweights: %d + %f = %f\n", tsw, tlw, weight(g, i));
}

void print_game(Game *g){
    printf("--------------------\n");
    // print the hands then the snake.
    for(int i = 0; i <= NP; i++) {
        print_hand(g, i);
    }
    print_snake(g);
    // print whose turn it is and the pass counter. also print possible_move moves for the current player.
    printf("Turn: %d\n", g->turn);
    printf("Pass Counter: %d\n", g->pass_counter);
}

void print_moves(Move moves[], int n){
    printf("Possible moves: ");
    for(int i = 0; i < n; i++) {
        printf("[%d|%d] ", moves[i].play.left, moves[i].play.right);
    }
    printf("\n");
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

int init_fact(){
    FACTORIAL[0] = 1;
    for(int i = 1; i < DCOUNT; i++){
        FACTORIAL[i] = FACTORIAL[i-1] * i;
    }
}

float pass_probability(Game *g, int n){// n is the number of distinct playable dominoes by the player
    int k = g->hands.hand_sizes[g->turn] - g->hands.solid_hand_sizes[g->turn];
    int m = g->hands.liquid_hand_sizes[g->turn] - n;
    if(k > m)
        return 0;
    return (float) (FACTORIAL[m] * FACTORIAL[m + n - k]) / (FACTORIAL[m - k] * FACTORIAL[m + n]);
}

void add_block(Snake* s, Block *block, enum Type type) {
    if(s->head == NULL){
        s->head = block;
        s->tail = block;
        return;
    }
    switch(type) {
    case LEFT:
        block->right_block = s->tail;
        block->left_block = NULL;
        s->tail->left_block = block;
        s->tail = block;
        break;
    case RIGHT:
        block->left_block = s->head;
        block->right_block = NULL;
        s->head->right_block = block;
        s->head = block;
        break;
    }
}

// remove the block on the head or tail of the snake depending on the head parameter. isolate the block by setting it's own pointers to NULL. don't forget about the case when there is only one block.
void remove_block(Snake* s, enum Type type) {
    if(s->head == s->tail){
        s->head = NULL;
        s->tail = NULL;
        return;
    }
    switch(type) {
    case LEFT:
        s->tail = s->tail->right_block;
        s->tail->left_block->right_block = NULL;
        s->tail->left_block = NULL;
        break;
    case RIGHT:
        s->head = s->head->left_block;
        s->head->right_block->left_block = NULL;
        s->head->right_block = NULL;
        break;
    }
}

float max(float a, float b) {
    if (a > b) {
        return a;
    }
    return b;
}

float min(float a, float b) {
    if (a < b) {
        return a;
    }
    return b;
}

void play_move(Game *g, Move move){
    add_block(&g->snake, g->block_set[move.play.left][move.play.right], move.type);
    //clear_owner(g->hands, move.play.left, move.play.right);
    clear_owner_play(g->turn, &g->hands, move.play.left, move.play.right);
    g->hands.hand_sizes[g->turn]--;
    g->turn = (g->turn + 1) % NP;
    g->pass_counter = 0;
}

void unplay_move(Game *g, Move move, Hands *prev){
    remove_block(&g->snake, move.type);
    g->turn = (g->turn - 1 + NP) % NP;
    g->hands = *prev;
    g->hands.hand_sizes[g->turn]++;
}

// this is a bad name
void absent(Game *g){ // used in passes and imperfect picks
    int left = g->snake.tail->domino.left;
    int right = g->snake.head->domino.right;
    for(int i = 0; i < PIPS; i++){
        clear_owner_pass(g->turn, &g->hands, i, left);
        clear_owner_pass(g->turn, &g->hands, i, right);
    }
}

// this is also a really bad name
void unabsent(Game *g){ // this isn't the inverse of the function above, only used in imperfect picks
    int less, more;
    if(g->snake.tail->domino.left > g->snake.head->domino.right){
        less = g->snake.head->domino.right;
        more = g->snake.tail->domino.left;
    } else {
        more = g->snake.head->domino.right;
        less = g->snake.tail->domino.left;
    }
    for(int i = 0; i < PIPS; i++){
        for(int j = 0; j <= i; j++){
            if(i != more && j != less && j != more && possible_possession(NP, &g->hands, i, j))
                set_possible_owner_pick(g->turn, &g->hands, i, j);
        }
    }
}

void pass(Game *g) {
    absent(g);
    g->turn = (g->turn + 1) % NP;
    g->pass_counter++;

}

void undo_pass(Game *g, Hands *prev){
    g->turn = (g->turn - 1) % NP;
    g->pass_counter--;
    g->hands = *prev;
}

void perfect_pick(Game *g, Move move){
    set_sole_owner_pick(g->turn, &g->hands, move.perfect_pick.left, move.perfect_pick.right);
    g->hands.hand_sizes[g->turn]++;
    g->hands.hand_sizes[NP]--;
    if(!g->hands.hand_sizes[NP])
        destroy_boneyard(&g->hands);
}

void imperfect_pick(Game *g, Move move){
    absent(g);
    unabsent(g); // yes, this is very confusing, i'm rapidly losing the will to do this.
    g->hands.hand_sizes[g->turn] += move.imperfect_pick.count;
    g->hands.hand_sizes[NP] -= move.imperfect_pick.count;    
    if(!g->hands.hand_sizes[NP])
        destroy_boneyard(&g->hands);
}

int over(Game *g) {
    return g->pass_counter == 2 || g->hands.hand_sizes[0] == 0 || g->hands.hand_sizes[1] == 0;
}

float endgame_evaluation(Game *g){
    int weights[NP] = {0};
    for(int i = 0; i < NP; i++) {
        weights[i] = weight(g, i);
    }
    return weights[0] * (weights[0] > weights[1]) - weights[1] * (weights[1] > weights[0]);
}

float heuristic_evaluation(Game *g){
    return weight(g, 0) - weight(g, 1);
}

float pick_loss_evaluation(Game *g){
    int left = g->snake.tail->domino.left;
    int right = g->snake.head->domino.right;
    int n = (g->hands.boneyard_solid_group_sizes[left] + g->hands.boneyard_solid_group_sizes[right]) >> (left == right) \
            - (left != right) * sole_possession(NP, &g->hands, left, right); // number of dominoes that can be played in the boneyard
    int wn = (g->hands.boneyard_solid_group_weights[left] + g->hands.boneyard_solid_group_weights[right]) >> (left == right) \
            - (left != right) * sole_possession(NP, &g->hands, left, right) * (left + right); // their weight
    float x = (float) (g->hands.hand_sizes[NP] - n) / (n + 1); // average number of dominoes picked before picking a playable domino
    return heuristic_evaluation(g) + (1 - 2 * g->turn) * (weight(g, NP) - wn) * x;
}

float minimax(Game *g, int depth, int maximizing_player){
    if(over(g))
        return endgame_evaluation(g);
    if(depth == 0)
        return heuristic_evaluation(g);
    int n, cant_pass;
    Move moves[MAX];
    Hands anchor = g->hands;
    float pass_score = 0, prob = 0, score, best_score;
    get_moves(g, moves, &n, &cant_pass);
    if(!cant_pass){
        prob = pass_probability(g, n - possible_possession(g->turn, &g->hands, g->snake.head->domino.left, g->snake.head->domino.right));
        if(g->hands.hand_sizes[NP]){
            pass_score = pick_loss_evaluation(g);
        } else {
            pass(g);
            pass_score = minimax(g, depth, !maximizing_player);
            undo_pass(g, &anchor);
        }
    }
    if(maximizing_player){
        best_score = -FLT_MAX;
        for(int i = 0; i < n; i++){
            play_move(g, moves[i]);
            score = minimax(g, depth - 1, !maximizing_player);
            unplay_move(g, moves[i], &anchor);
            best_score = max(best_score, score);
        }
    } else {
        best_score = FLT_MAX;
        for(int i = 0; i < n; i++){
            play_move(g, moves[i]);
            score = minimax(g, depth - 1, !maximizing_player);
            unplay_move(g, moves[i], &anchor);
            best_score = min(best_score, score);
        }
    }
    return prob * pass_score + (1 - prob) * best_score; 
}

void get_hand_sizes(Game *g) {
    for(int i = 0; i < NP; i++){
        printf("Player %d: ", i);
        scanf("%d", &g->hands.hand_sizes[i]);
    }
    printf("Boneyard: ");
    scanf("%d", &g->hands.hand_sizes[NP]);
}

void get_hands(Game *g) {
    int a;
    for(int i = 0; i < PIPS; i++){
        for(int j = 0; j <= i; j++){
            printf("[%d|%d]: ", i, j);
            scanf("%d", &a);
            if(a){
                set_sole_owner_start(1, &g->hands, i, j);
            } else {
                set_outside_owner_start(1, &g->hands, i, j);
            }

        }
    }
}

Move best_move(Game *g, int depth){
    Move moves[DCOUNT], best_move;
    int n = 0, cant_pass;
    Hands anchor = g->hands;
    float best_score, score;
    get_moves(g, moves, &n, &cant_pass);
    if(g->turn){
        best_score = -FLT_MAX;
        for(int i = 0; i < n; i++){
            play_move(g, moves[i]);
            score = minimax(g, depth - 1, g->turn);
            unplay_move(g, moves[i], &anchor);
            if(score > best_score){
                best_score = score;
                best_move = moves[i];
            }
        }
    } else {
        best_score = FLT_MAX;
        for(int i = 0; i < n; i++){
            play_move(g, moves[i]);
            score = minimax(g, depth - 1, g->turn);
            unplay_move(g, moves[i], &anchor);
            if(score < best_score){
                best_score = score;
                best_move = moves[i];
            }
        }
    }
    printf("best score: %f, best move: [%d|%d] %d\n", best_score, best_move.play.left, best_move.play.right, best_move.type);
    return best_move;
}

void start(){
    Game *g = malloc(sizeof(Game));
    init_game(g);
    get_hand_sizes(g);
    get_hands(g);
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
