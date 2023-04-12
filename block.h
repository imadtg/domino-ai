#ifndef BLOCK_H
#define BLOCK_H
#include "cst.h"
#include "domino.h"

typedef struct Block{
    Domino domino;
    struct Block *left_block, *right_block;
} Block;

void init_blocks(Block *block_set[PIPS][PIPS]);
#endif