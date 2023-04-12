#ifndef MINIMAX_H
#define MINIMAX_H
#include "move.h"
#include "game.h"

float endgame_evaluation(Game *g);
float heuristic_evaluation(Game *g);
float pick_loss_evaluation(Game *g);
float max(float a, float b);
float min(float a, float b);
float minimax(Game *g, int depth, int maximizing_player);
Move best_move(Game *g, int depth);

#endif