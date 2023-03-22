#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define PIPS 4
#define MAX (PIPS*2+1)
#define NP 2
#define TOTAL (PIPS*(PIPS+1)*(PIPS+2)/2)

enum Type {LEFT, RIGHT, PERFECT_PICK, IMPERFECT_PICK, PASS};

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
    int liquid_weights[NP+1];  // this is the sum of weights of all the dominoes that the player can have, not to be confused with the sum of weights of the dominoes in the player's hand
    int liquid_hand_sizes[NP+1]; // this is the number of distinct dominoes the player can have, not to be confused with the number of dominoes in the player's hand
    int solid_weights[NP+1]; // this is the sum of weights of all the dominoes that we are sure the player has
    int boneyard_solid_sizes[PIPS]; // this is the weight of each group of dominoes by pip value that we are sure is in the boneyard, used to calculate average hand weight after picking up from a boneyard.
} Hands;

typedef struct Game {
    Snake snake;
    Hands *hands;
    int hand_sizes[NP+1];
    int outed[PIPS]; // this is the number of dominoes that have a pip value of i that are out of the game
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
    int player;
    union {
        playing_move play;
        passing_move pass;
        imperfect_picking_move imperfect_pick;
        perfect_picking_move perfect_pick;
    };
} Move;

int can_play(int player, Hands *hands, int i, int j){
    return hands->ownership[i][j] & (1 << player);
}

int owns(int player, Hands *hands, int i, int j){
    return hands->ownership[i][j] == (1 << player);
}

int out(Hands *hands, int i, int j){
    return !hands->ownership[i][j];
}

int sole_owner(Hands *hands, int i, int j){
    return !((hands->ownership[i][j] & (hands->ownership[i][j] - 1)) || out(hands, i, j));
}

void set_owner(int player, Hands *hands, int i, int j){
    int mask = 1;
    int hands_mask = hands->ownership[i][j];
    for (int p = 0; p <= NP; p++) {
        hands->liquid_weights[p] -= ((hands_mask & mask) != 0) * (i + j);
        hands->liquid_hand_sizes[p] -= ((hands_mask & mask) != 0);
        mask <<= 1;
    }
    hands->solid_weights[player] += i + j;
    hands->ownership[i][j] = (1 << player);
    hands->ownership[j][i] = (1 << player);
}

void set_possible_owner(int player, Hands *hands, int i, int j){
    int mask = 1 << player;
    int hands_mask = hands->ownership[i][j];
    hands->liquid_weights[player] += ((hands_mask & mask) == 0) * (i + j);
    hands->liquid_hand_sizes[player] += ((hands_mask & mask) == 0);
    hands->ownership[i][j] |= mask * ((hands_mask & mask) == 0);
    hands->ownership[j][i] |= mask * ((hands_mask & mask) == 0);
}

void clear_owner(int player, Hands *hands, int i, int j){
    int mask = 1 << player;
    int hands_mask = hands->ownership[i][j];
    hands->liquid_weights[player] -= ((hands_mask & mask) != 0) * (i + j);
    hands->liquid_hand_sizes[player] -= ((hands_mask & mask) != 0);
    hands->ownership[i][j] &= ~(hands_mask & mask);
    hands->ownership[j][i] &= ~(hands_mask & mask);
}


float weight(Game *g, int player){
    return (g->hands->liquid_weights[player] / (float) g->hands->liquid_hand_sizes[player]) * (g->hand_sizes[player]);
}

// initialize a game
void init_game(Game *g) {
    int i, j;
    for (i = 0; i < PIPS; i++) {
        for (j = 0; j < PIPS; j++) {
            g->block_set[i][j] = (block*) malloc(sizeof(block));
            g->block_set[i][j]->left = i;
            g->block_set[i][j]->right = j;
            g->block_set[i][j]->left_block = NULL;
            g->block_set[i][j]->right_block = NULL;
        }
    }
    g->snake.head = NULL;
    g->snake.tail = NULL;
    g->pass_counter = 0;
    g->hands = (Hands*) malloc(sizeof(Hands));
    memset(g->hands->ownership, 0, sizeof(g->hands->ownership));
    memset(g->hands->liquid_weights, 0, sizeof(g->hands->liquid_weights));
    memset(g->hands->liquid_hand_sizes, 0, sizeof(g->hands->liquid_hand_sizes));
}

int possible_move(Snake *s, Move move) {
    switch(move.type) {
        case LEFT:
            return s->tail == NULL || s->tail->left == move.right;
        case RIGHT:
            return s->head == NULL || s->head->right == move.left;
        default:
            return 1;
    }
}

int playable_domino(snake *s, int left, int right) {
    return s->head == NULL || left == s->head->left || left == s->tail->right || right == s->head->left || right == s->tail->right;
}

void print_snake(Game *g) {
    Block *b = g->snake.tail;
    printf("Snake: ");
    while(b!=NULL){
        printf("[%d|%d]", b->left, b->right);
        b = b->right_block;
    }
    printf("\n");
}

void print_hand(Game *g, int i) {
    printf("Player %d: ", i);
    for(int j = 0; j < PIPS; j++) {
        for(int k = 0; k <= j; k++) {
            if(owns(i, g->hands, k, j))
                printf("[%d|%d] ", k, j);
        }
    }
    printf("\n");
}

void print_game(Game *g){
    printf("--------------------\n");
    // print the hands then the snake.
    for(int i = 0; i < NP; i++) {
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

void get_moves(Game *g, Move moves[MAX], int *n, int *can_pass){ // doesn't work on empty snake
    *n = 0;
    *can_pass = 0;
    int left = g->snake.tail->left;
    int right = g->snake.head->right;
    for(int i = 0; i < PIPS; i++){
        if(can_play(g->turn, g->hands, i, left) && possible_move(&g->snake, (Move){LEFT, g->turn, i, left})){
            *can_pass ||= owns(g->turn, g->hands, i, left);
            moves[*n].play.left = i;
            moves[*n].play.right = left;
            moves[*n].type = LEFT;
            (*n)++;
        }
    }
    if(left != right){
        for(int i = 0; i < PIPS; i++){
            if(can_play(g->turn, g->hands, i, left) && possible_move(&g->snake, (Move){RIGHT, g->turn, right, i})){
                *can_pass ||= owns(g->turn, g->hands, i, left);
                moves[*n].play.left = right;
                moves[*n].play.right = i;
                moves[*n].type = RIGHT;
                (*n)++;
            }
        }
    }
}

void add_block(Snake* s, Block *block, enum Type type) {
    if(s->head == NULL){
        s->head = block;
        s->tail = block;
        return;
    }
    switch(type) {
        case LEFT:
            block->left_block = s->tail;
            block->right_block = NULL;
            s->tail->right_block = block;
            s->tail = block;
            break;
        case RIGHT:
            block->right_block = s->head;
            block->left_block = NULL;
            s->head->left_block = block;
            s->head = block;
            break;
        default:
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
            s->tail->left_block->left_block = NULL;
            s->tail->left_block = NULL;
            break;
        case RIGHT:
            s->head = s->head->left_block;
            s->head->right_block->left_block = NULL;
            s->head->right_block->right_block = NULL;
            s->head->right_block = NULL;
            break;
        default:
            break;
    }
}

int max(int a, int b) {
    if (a > b) {
        return a;
    }
    return b;
}

int min(int a, int b) {
    if (a < b) {
        return a;
    }
    return b;
}

void play_move(Game *g, Move move){
    add_block(&g->snake, g->block_set[move.play.left][move.play.right], move.type);
    clear_owner(g->hands, move.play.left, move.play.right);
    g->hands.weight[g->turn] -= move.play.left + move.play.right;
    g->hand_sizes[g->turn]--;
    g->turn = g->turn + 1 % NP;
    g->pass_counter = 0;
}

void unplay_move(Game *g, Move move){
    remove_block(&g->snake, move.type);
    g->turn = g->turn - 1 % NP;
    g->hands = hands;
    g->hand_sizes[g->turn]++;
}

void pass(game *g) {
    g->turn = g->turn + 1 % NP;
    g->pass_counter = g->pass_counter + 1;
}

void do_move(Game *g, Move move) {
    switch(move.type) {
        case LEFT:
        case RIGHT:
            play_move(g, move);
            break;
        case PASS:
            pass(g);
            break;
        case PERFECT_PICK:
            perfect_pick(g, move);
            break;
        case IMPERFECT_PICK:
            imperfect_pick(g, move);
            break;
    }
}

void undo_move(Game *g, Move move) {
    switch(move.type) {
        case LEFT:
        case RIGHT:
            unplay_move(g, move);
            break;
        case PASS:
            undo_pass(g);
            break;
        case PERFECT_PICK:
            undo_perfect_pick(g, move);
            break;
        case IMPERFECT_PICK:
            undo_imperfect_pick(g, move);
            break;
    }

}

void undo_pass(game *g){
    g->turn = g->turn - 1 % NP;
    g->pass_counter = g->pass_counter - 1;
}

int over(game *g) {
    return g->pass_counter == 2 || g->hand_sizes[0] == 0 || g->hand_sizes[1] == 0;
}

float endgame_evaluation(game *g){
    int weights[NP] = {0};
    for(int i = 0; i < NP; i++) {
        weights[i] = weight(g, i);
    }
    return weights[0] * (weights[0] > weights[1]) - weights[1] * (weights[1] > weights[0]);
}

float heuristic_evaluation(game *g){
    int weights[NP] = {0};
    for(int i = 0; i < NP; i++) {
        weights[i] = weight(g, i);
    }
    return weights[0] - weights[1];
}



/*node *minmax_tree(game *g, int depth, int alpha, int beta, int maximizing_player, move move){
    if(over(g))
        return new_node(move, endgame_evaluation(g) * 1000);
    if(depth == 0)
        return new_node(move, heuristic_evaluation(g));
    struct move moves[MAX];
    int n = 0;
    get_moves(g, moves, &n);
    sort_moves(moves, n);
    node *move_node = new_node(move, maximizing_player ? INT_MIN : INT_MAX);
    switch(n){
    case 0:
        pass(g);
        move_node->children[0] = minmax_tree(g, depth, alpha, beta, !maximizing_player, (struct move){-1, -1, -1});
        move_node->score = move_node->children[0]->score;
        move_node->num_children = 1;
        unpass(g);
        return move_node;
    case 1:
        domove(g, moves[0]);
        move_node->children[0] = minmax_tree(g, depth-1, alpha, beta, !maximizing_player, moves[0]);
        move_node->score = move_node->children[0]->score;
        move_node->num_children = 1;
        unmove(g, moves[0]);
        return move_node;
    default:
        if(maximizing_player){
            move_node->num_children = n;
            for(int i = 0; i < n; i++){
                domove(g, moves[i]);
                move_node->children[i] = minmax_tree(g, depth-1, alpha, beta, 0, moves[i]);
                unmove(g, moves[i]);
                move_node->score = max(move_node->score, move_node->children[i]->score);
                alpha = max(alpha, move_node->score);
                if(beta <= alpha){
                    move_node->num_children = i+1;
                    break;
                }
            }
        } else {
            move_node->num_children = n;
            for(int i = 0; i < n; i++){
                domove(g, moves[i]);
                move_node->children[i] = minmax_tree(g, depth-1, alpha, beta, 1, moves[i]);
                unmove(g, moves[i]);
                move_node->score = min(move_node->score, move_node->children[i]->score);
                beta = min(beta, move_node->score);
                if(beta <= alpha){
                    move_node->num_children = i+1;
                    break;
                }
            }
        }
        return move_node;   
    }
}*/

// implement the same function as above, but only return the score of the given move instead of the move tree.
int negamax(game *g, int depth, int alpha, int beta){
    /*if(over(g))
        return endgame_evaluation(g) * 1000;
    if(depth == 0)
        return heuristic_evaluation(g);
    struct move moves[MAX];
    int n = 0, score;
    get_moves(g, moves, &n);
    sort_moves(moves, n);
    switch(n){
    case 0:
        pass(g);
        score = minmax(g, depth, alpha, beta, !maximizing_player);
        unpass(g);
        return score;
    case 1:
        domove(g, moves[0]);
        score = minmax(g, depth-1, alpha, beta, !maximizing_player);
        unmove(g, moves[0]);
        return score;
    default:
        if(maximizing_player){
            for(int i = 0; i < n; i++){
                domove(g, moves[i]);
                score = minmax(g, depth-1, alpha, beta, 0);
                unmove(g, moves[i]);
                alpha = max(alpha, score);
                if(beta <= alpha)
                    break;
            }
            return alpha;
        } else {
            for(int i = 0; i < n; i++){
                domove(g, moves[i]);
                score = minmax(g, depth-1, alpha, beta, 1);
                unmove(g, moves[i]);
                beta = min(beta, score);
                if(beta <= alpha)
                    break;
            }
            return beta;
        }
    }*/
    if(over(g))
        return endgame_evaluation(g) * 1000;
    if(depth == 0)
        return heuristic_evaluation(g)
    Move moves[MAX];
    Hands previous_hands = g->hands;
    int n = 0, score;
}


void get_hands(int hands[PIPS][PIPS], int hand_sizes[2]) {
    for(int i = 0; i < PIPS; i++){
        for(int j = 0; j <= i; j++){
            printf("[%d|%d]: ", i, j);
            scanf("%d", &hands[i][j]);
            hands[j][i] = hands[i][j];
            hand_sizes[hands[i][j]]++;
        }
    }
}

move best_move(game *g, int depth){
    struct move moves[MAX], best_move;
    int n = 0, best_score;
    get_moves(g, moves, &n);
    best_score = g->turn ? INT_MIN : INT_MAX;
    for(int i = 0; i < n; i++){
        domove(g, moves[i]);
        int score = minmax(g, depth-1, INT_MIN, INT_MAX, g->turn);
        unmove(g, moves[i]);
        if((g->turn && score > best_score) || (!g->turn && score < best_score)){
            best_score = score;
            best_move = moves[i];
        }
    }
    printf("best score: %d, best move: %d %d %d\n", best_score, best_move.left, best_move.right, best_move.head);
    return best_move;
}

/*node *game_theoretic_tree(int depth, game *g, move *first, int write){
    if(g == NULL){
        g = malloc(sizeof(game));
        init_game(g);
        get_hands(g->hands, g->hand_sizes);
        first = malloc(sizeof(move));
        *first = (struct move){PIPS-1, PIPS-1, 0};
        g->turn = g->hands[PIPS-1][PIPS-1];
        domove(g, *first);
    }
    node *root = minmax_tree(g, depth, INT_MIN, INT_MAX, g->turn, *first);
    if(write) print_wrapper(root, "C:\\Users\\PC\\Desktop\\FL\\tree.txt");
    return root;
}*/

void start(){
    game *g = malloc(sizeof(game));
    init_game(g);
    get_hands(g->hands, g->hand_sizes);
    // the first move has to be the biggest double, which is just [PIPS-1|PIPS-1]. so we initialize the turn with who has it and play it right away.
    g->turn = g->hands[PIPS-1][PIPS-1];
    domove(g, (struct move){PIPS-1, PIPS-1, 0});
    int n;
    move moves[MAX];
    do {
        print_game(g);
        get_moves(g, moves, &n);
        print_moves(moves, n);
        printf("%d moves\n", n);
        switch(n){
        case 0:
            pass(g);
            break;
        case 1:
            domove(g, moves[0]);
            break;
        default:
            move move;
            while(1){
                scanf("%d %d %d", &move.left, &move.right, &move.head);
                if(move.left == -1){
                    if(move.head == -1){
                        move = best_move(g, /*move.right*/20); // using move.right as depth when move.left is -1.
                        continue;
                    }
                    move = best_move(g, /*move.right*/20); // this isn't the best way to do this, but it works.
                }
                domove(g, move);
                break;
            }
        }
    } while(!over(g));
    print_game(g);
    printf("game over, score: %d\n", endgame_evaluation(g));
}

int main(){
    start();
    //game_theoretic_tree(13, NULL, NULL, 1);
    getch();
    return 0;
}