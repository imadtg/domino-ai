#include "heap.h"
#include <stdlib.h>

int greater_than(HeapElt a, HeapElt b){
    return a.score > b.score;
}

int less_than(HeapElt a, HeapElt b){
    return a.score < b.score;
}

void init_heap(Heap *h, int capacity, int (*compare)(HeapElt, HeapElt)){
    h->array = malloc(capacity * sizeof(HeapElt));
    h->size = 0;
    h->capacity = capacity;
    h->compare = compare;
    h->symmetric_count = 0;
}

void free_heap(Heap *h){
    free(h->array);
}

void heapify(Heap *h){
    for(int i = h->size / 2 - 1; i >= 0; i--){
        heapify_down(h, i);
    }
}

void heapify_down(Heap *h, int i){
    int left = 2 * i + 1;
    int right = 2 * i + 2;
    int p = i;
    if(left < h->size && h->compare(h->array[left], h->array[p]))
        p = left;
    if(right < h->size && h->compare(h->array[right], h->array[p]))
        p = right;
    if(p != i){
        HeapElt temp = h->array[i];
        h->array[i] = h->array[p];
        h->array[p] = temp;
        heapify_down(h, p);
    }
}

int heapify_up(Heap *h, int i){
    int parent = (i - 1) / 2;
    if(parent >= 0 && h->compare(h->array[i], h->array[parent])){
        HeapElt temp = h->array[i];
        h->array[i] = h->array[parent];
        h->array[parent] = temp;
        return heapify_up(h, parent);
    }
    return i;
}

int heap_insert(Heap *h, Move move, int index, float score, int symmetric){
    HeapElt elt;
    elt.move = move;
    elt.move_index = index;
    elt.score = score;
    elt.symmetric = symmetric;
    if(symmetric) h->symmetric_count++;
    if(h->size == h->capacity){
        h->capacity *= 2;
        h->array = realloc(h->array, h->capacity * sizeof(HeapElt));
    }
    h->array[h->size] = elt;
    int i = heapify_up(h, h->size);
    h->size++;
    return i;
}

HeapElt heap_extract(Heap *h){
    HeapElt elt = h->array[0];
    h->array[0] = h->array[h->size - 1];
    h->size--;
    heapify_down(h, 0);
    if(elt.symmetric){
        h->symmetric_count--;
        if(h->symmetric_count == 0) return heap_extract(h);// they come in pairs, so if it's 0 after decrementing it then the first pair has already been removed.
    }
    return elt;
}

int can_extract(Heap *h){// test if the heap isn't empty OR the last element isn't symmetric with no other symmetric elements in the heap
    if(h->size == 0) return 0;
    if(h->size == 1 && h->symmetric_count == 1) return 0;
    return 1;
}