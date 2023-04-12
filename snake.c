#include "snake.h"

void print_snake(Snake *snake) {
    Block *b = snake->tail;
    printf("Snake: ");
    while(b!=NULL){
        printf("[%d|%d]", b->domino.left, b->domino.right);
        b = b->right_block;
    }
    printf("\n");
}

void add_block(Snake* s, Block *block, enum Type type) {
    if(s->head == NULL){
        s->head = block;
        s->tail = block;
        return;
    }
    switch(type) {
    case LEFT:
        block->right_block = s->tail;
        block->left_block = NULL;
        s->tail->left_block = block;
        s->tail = block;
        break;
    case RIGHT:
        block->left_block = s->head;
        block->right_block = NULL;
        s->head->right_block = block;
        s->head = block;
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
        s->tail->left_block = NULL;
        break;
    case RIGHT:
        s->head = s->head->left_block;
        s->head->right_block->left_block = NULL;
        s->head->right_block = NULL;
        break;
    }
}