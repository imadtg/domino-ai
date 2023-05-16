#ifndef MINIMAX_H
#define MINIMAX_H
#include "move.h"
#include "game.h"
#include "heap.h"

enum Mode {PESSIMIST, EXPECT};

float endgame_evaluation(Game *g);
float heuristic_evaluation(Game *g);
float pick_loss_evaluation(Game *g);
float max(float a, float b);
float min(float a, float b);
void process_absence(Game *g, Hands *anchor, float *pass_score, float *prob, int n, int depth, float (*ai_function)(Game *, int));
float minimax(Game *g, int depth);
float expected_score_from_heap(Game *g, Heap *h, int liquid_size, int collapsing_size);
float expectiminimax(Game *g, int depth);
//float avgminimax(Game *g, int depth);
Move best_move(Game *g, int depth, float (*ai_function)(Game *, int));

#endif