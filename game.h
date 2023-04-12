#ifndef GAME_H
#define GAME_H
#include "domino.h"
#include "block.h"
#include "move.h"
#include "snake.h"
#include "hands.h"

typedef struct Game {
    Snake snake;
    Hands hands;
    int turn;
    int pass_counter;
    Block *block_set[PIPS][PIPS];
} Game;

// Factorial function, to be used in pass_probability function
int init_fact();
extern int FACTORIAL[DCOUNT];

// Game initialization
void init_game(Game *g);

// Functions for checking if a domino can be played
int playable_move(Snake *s, enum Type type, int left, int right);
int playable_domino(Snake *s, int left, int right);

// Functions for printing the game and getting possible moves
void print_game(Game *g);
void get_moves(Game *g, Move moves[MAX], int *n, int *cant_pass);

// Function for calculating pass probability
float pass_probability(Game *g, int n);

// Functions for playing and unplaying a move
void play_move(Game *g, Move move);
void unplay_move(Game *g, Move move, Hands *prev);

void absent(Game *g);
void unabsent(Game *g);

void pass(Game *g);
void undo_pass(Game *g, Hands *prev);

void perfect_pick(Game *g, Move move);
void imperfect_pick(Game *g, Move move);

int over(Game *g);

#endif