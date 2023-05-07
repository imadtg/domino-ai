#include "heap.h"
#include <stdlib.h>

void init_heap(Heap *h, int capacity){
    h->array = malloc(capacity * sizeof(HeapElt));
    h->size = 0;
    h->capacity = capacity;
}

void free_heap(Heap *h){
    free(h->array);
}

void minheapify(Heap *h){
    for(int i = h->size / 2 - 1; i >= 0; i--){
        minheapify_down(h, i);
    }
}

void maxheapify(Heap *h){
    for(int i = h->size / 2 - 1; i >= 0; i--){
        maxheapify_down(h, i);
    }
}

void minheapify_down(Heap *h, int i){
    int left = 2 * i + 1;
    int right = 2 * i + 2;
    int smallest = i;
    if(left < h->size && h->array[left].score < h->array[smallest].score)
        smallest = left;
    if(right < h->size && h->array[right].score < h->array[smallest].score)
        smallest = right;
    if(smallest != i){
        HeapElt temp = h->array[i];
        h->array[i] = h->array[smallest];
        h->array[smallest] = temp;
        minheapify_down(h, smallest);
    }
}

void maxheapify_down(Heap *h, int i){
    int left = 2 * i + 1;
    int right = 2 * i + 2;
    int largest = i;
    if(left < h->size && h->array[left].score > h->array[largest].score)
        largest = left;
    if(right < h->size && h->array[right].score > h->array[largest].score)
        largest = right;
    if(largest != i){
        HeapElt temp = h->array[i];
        h->array[i] = h->array[largest];
        h->array[largest] = temp;
        maxheapify_down(h, largest);
    }
}

int minheapify_up(Heap *h, int i){
    int parent = (i - 1) / 2;
    if(parent >= 0 && h->array[i].score < h->array[parent].score){
        HeapElt temp = h->array[i];
        h->array[i] = h->array[parent];
        h->array[parent] = temp;
        return minheapify_up(h, parent);
    }
    return i;
}

int maxheapify_up(Heap *h, int i){
    int parent = (i - 1) / 2;
    if(parent >= 0 && h->array[i].score > h->array[parent].score){
        HeapElt temp = h->array[i];
        h->array[i] = h->array[parent];
        h->array[parent] = temp;
        return maxheapify_up(h, parent);
    }
    return i;
}

int minheap_insert(Heap *h, Move move, int index, float score){
    HeapElt elt;
    elt.move = move;
    elt.move_index = index;
    elt.score = score;
    if(h->size == h->capacity){
        h->capacity *= 2;
        h->array = realloc(h->array, h->capacity * sizeof(HeapElt));
    }
    h->array[h->size] = elt;
    int i = minheapify_up(h, h->size);
    h->size++;
    return i;
}

int maxheap_insert(Heap *h, Move move, int index, float score){
    HeapElt elt;
    elt.move = move;
    elt.move_index = index;
    elt.score = score;
    if(h->size == h->capacity){
        h->capacity *= 2;   
        h->array = realloc(h->array, h->capacity * sizeof(HeapElt));
    }
    h->array[h->size] = elt;
    int i = maxheapify_up(h, h->size);
    h->size++;
    return i;
}

HeapElt minheap_extract(Heap *h){
    HeapElt elt = h->array[0];
    h->array[0] = h->array[h->size - 1];
    h->size--;
    minheapify_down(h, 0);
    return elt;
}

HeapElt maxheap_extract(Heap *h){
    HeapElt elt = h->array[0];
    h->array[0] = h->array[h->size - 1];
    h->size--;
    maxheapify_down(h, 0);
    return elt;
}

void minheap_update(Heap *h, int heap_index, Move move, int move_index, float score){
    h->array[heap_index].move = move;
    h->array[heap_index].move_index = move_index;
    h->array[heap_index].score = score;
}

void maxheap_update(Heap *h, int heap_index, Move move, int move_index, float score){
    h->array[heap_index].move = move;
    h->array[heap_index].move_index = move_index;
    h->array[heap_index].score = score;    
}