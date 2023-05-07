#ifndef HEAP_H
#define HEAP_H
#include "domino.h"
#include "move.h"

typedef struct {
    Move move;
    int move_index;
    float score;
} HeapElt;

typedef struct {
    HeapElt *array;
    int size;
    int capacity;
} Heap;

void init_heap(Heap *h, int capacity);
void free_heap(Heap *h);
void minheapify(Heap *h);
void maxheapify(Heap *h);
void minheapify_down(Heap *h, int heap_index);
void maxheapify_down(Heap *h, int heap_index);
int minheapify_up(Heap *h, int heap_index);
int maxheapify_up(Heap *h, int heap_index);
int minheap_insert(Heap *h, Move move, int move_index, float score);
int maxheap_insert(Heap *h, Move move, int move_index, float score);
void minheap_update(Heap *h, int heap_index, Move move, int move_index, float score);
void maxheap_update(Heap *h, int heap_index, Move move, int move_index, float score);
HeapElt minheap_extract(Heap *h);
HeapElt maxheap_extract(Heap *h);

#endif