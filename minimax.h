#ifndef MINIMAX_H
#define MINIMAX_H
#include "move.h"
#include "game.h"
#include "heap.h"

enum Mode {PESSIMIST, AVG, EXPECT};

float endgame_evaluation(Game *g);
float heuristic_evaluation(Game *g);
float pick_loss_evaluation(Game *g);
float max(float a, float b);
float min(float a, float b);
float minimax(Game *g, int depth, int maximizing_player);
float expected_score_from_heap(Game *g, Heap *h, int liquid_size, int collapsing_size);
float expectiminimax(Game *g, int depth, int maximizing_player);
float avgminimax(Game *g, int depth, int maximizing_player);
Move best_move(Game *g, int depth);
Move best_move_expect(Game *g, int depth);
Move best_move_avg(Game *g, int depth);

#endif