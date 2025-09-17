#ifndef MINIMAX_H
#define MINIMAX_H
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
#include "move.h"
#include "game.h"
#include "heap.h"


enum Mode {PESSIMIST, EXPECT};


#ifdef _WIN32
extern volatile int FALLBACK;
WINBOOL interrupt_search(DWORD ctrl_type);
#endif
float max(float x, float y);
float min(float x, float y);
float endgame_evaluation(Game *g);
float heuristic_evaluation(Game *g);
float pick_loss_evaluation(Game *g);
void process_absence(Game *g, Hands *anchor, float *pass_score, float *prob, int n, int depth, int skip, int *nodes, float (*ai_function)(Game *, int, int, int *));
float minimax(Game *g, int depth, int skip, int *nodes);
float expected_score_from_heap(Game *g, Heap *h, int liquid_size, int collapsing_size);
float expectiminimax(Game *g, int depth, int skip, int *nodes);
Move best_move(Game *g, Move moves[], float scores[], int n, int depth, int skip, int *nodes, float (*ai_function)(Game *, int, int, int *));

#ifdef _WIN32
Move iterative_deepening(Game *g, Move moves[], int n, int skip, float (*ai_function)(Game *, int, int, int *));
#endif

#endif