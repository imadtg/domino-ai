#ifndef HEAP_H
#define HEAP_H
#include "domino.h"
#include "move.h"

typedef struct {
    Move move;
    int move_index;
    float score;
    int symmetric; // symmetric elements are ignored after the first one is extracted
} HeapElt;

typedef struct {
    HeapElt *array;
    int size;
    int capacity;
    int (*compare)(HeapElt, HeapElt);
    int symmetric_count; // number of symmetric elements in the heap
} Heap;

int greater_than(HeapElt a, HeapElt b);
int less_than(HeapElt a, HeapElt b);
void init_heap(Heap *h, int capacity, int (*compare)(HeapElt, HeapElt));
void free_heap(Heap *h);
void heapify(Heap *h);
void heapify_down(Heap *h, int heap_index);
int heapify_up(Heap *h, int heap_index);
int heap_insert(Heap *h, Move move, int move_index, float score, int symmetric);
HeapElt heap_extract(Heap *h);
int can_extract(Heap *h);

#endif