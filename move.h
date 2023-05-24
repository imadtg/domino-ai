#ifndef MOVE_H
#define MOVE_H
#include "domino.h"

enum Type {LEFT, RIGHT, PERFECT_PICK, IMPERFECT_PICK, PASS};

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

void print_playing_moves(Move moves[], int n);
void print_picking_moves(Move moves[], int n);

#endif