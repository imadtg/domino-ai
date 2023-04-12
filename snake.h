#ifndef SNAKE_H
#define SNAKE_H
#include "block.h"
#include "move.h"

typedef struct {
    Block* head;
    Block* tail;
} Snake;

void print_snake(Snake *snake);
void add_block(Snake *s, Block *block, enum Type type);
void remove_block(Snake *s, enum Type type);

#endif