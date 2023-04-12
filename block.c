#include "cst.h"
#include "block.h"

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