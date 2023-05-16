#include "minimax.h"

float endgame_evaluation(Game *g){
    int weights[NP] = {0};
    for(int i = 0; i < NP; i++) {
        weights[i] = weight(&g->hands, i);
    }
    return weights[0] * (weights[0] > weights[1]) - weights[1] * (weights[1] > weights[0]);
}

float heuristic_evaluation(Game *g){
    return weight(&g->hands, 0) - weight(&g->hands, 1);
}

float pick_loss_evaluation(Game *g){ // the loss of the player who's turn it is if he starts picking from the bonyard.
    int left = g->snake.tail->domino.left;
    int right = g->snake.head->domino.right;
    int n = (g->hands.boneyard_groups_grouped_by_pip[left].size + g->hands.boneyard_groups_grouped_by_pip[right].size); // number of dominoes that can be played in the boneyard
    n >>= (left == right); // divide by two if the snake ends are identical to remove duplicates
    n -= (left != right) && sole_possession(NP, &g->hands, left, right); // substract the duplicate domino that's both in the left and right group if they are different and is in the boneyard.
    int wn = (g->hands.boneyard_groups_grouped_by_pip[left].weight + g->hands.boneyard_groups_grouped_by_pip[right].weight); // their weight
    wn >>= (left == right); // same logic as above
    wn -= (left != right) && sole_possession(NP, &g->hands, left, right) * (left + right); // same logic as above
    return heuristic_evaluation(g) + (1 - 2 * g->turn) * (weight(&g->hands, NP) - wn) / (n + 1);
}

float max(float a, float b) {
    if (a > b)
        return a;
    return b;
}

float min(float a, float b) {
    if (a < b) 
        return a;
    return b;
}

void process_absence(Game *g, Hands *anchor, float *pass_score, float *prob, int n, int depth, float (*ai_function)(Game *, int)){
    *prob = pass_probability_from_num_moves(g, n);
    if(*prob){
        if(!hand_is_empty(NP, &g->hands)){
            if(is_passing(g, NP)){
                Move pick_all_of_boneyard;
                pick_all_of_boneyard.type = IMPERFECT_PICK;
                pick_all_of_boneyard.imperfect_pick.count = g->hands.hand_sizes[NP];
                imperfect_pick(g, pick_all_of_boneyard);
                *pass_score = ai_function(g, depth);
                undo_imperfect_pick(g, anchor);
            } else {
                absence_event(g);
                *pass_score = pick_loss_evaluation(g);
                undo_absence_event(g, anchor);
            }
        } else {
            pass(g);
            *pass_score = ai_function(g, depth);
            undo_pass(g, anchor);
        }
    }
}

float minimax(Game *g, int depth){
    if(over(g))
        return endgame_evaluation(g);
    if(depth == 0)
        return heuristic_evaluation(g);
    int n, cant_pass, pc = g->pass_counter;
    Move moves[MAX];
    Hands anchor = g->hands;
    float pass_score = 0, prob = 0, score, best_score;
    get_moves(g, moves, &n, &cant_pass);
    if(!cant_pass){
        process_absence(g, &anchor, &pass_score, &prob, n, depth, minimax);
    }
    if(g->turn){
        best_score = -FLT_MAX;
        for(int i = 0; i < n; i++){
            play_move(g, moves[i]);
            score = minimax(g, depth - 1);
            unplay_move(g, moves[i], &anchor, pc);
            best_score = max(best_score, score);
        }
    } else {
        best_score = FLT_MAX;
        for(int i = 0; i < n; i++){
            play_move(g, moves[i]);
            score = minimax(g, depth - 1);
            unplay_move(g, moves[i], &anchor, pc);
            best_score = min(best_score, score);
        }
    }
    return prob * pass_score + (1 - prob) * best_score; 
}

float expected_score_from_heap(Game *g, Heap *h, int liquid_size, int collapsing_size){
    HeapElt e = heap_extract(h);
    if(certain(&g->hands, e.move.play.left, e.move.play.right) || liquid_size == collapsing_size || !can_extract(h))
        return e.score;
    float probability_of_ownership = (float) collapsing_size / liquid_size;
    return probability_of_ownership * e.score + (1 - probability_of_ownership) * expected_score_from_heap(g, h, liquid_size - 1, collapsing_size);
}

float expectiminimax(Game *g, int depth){
    if(over(g))
        return endgame_evaluation(g);
    if(depth == 0)
        return heuristic_evaluation(g);
    int n, cant_pass, pc = g->pass_counter;
    Move moves[MAX];
    Heap move_heap;
    Hands anchor = g->hands;
    float pass_score = 0, prob = 0, score, best_score;
    get_moves(g, moves, &n, &cant_pass);
    if(!cant_pass){
        process_absence(g, &anchor, &pass_score, &prob, n, depth, expectiminimax);
    }
    if(n == 0) return pass_score;
    init_heap(&move_heap, n, g->turn ? greater_than : less_than);
    if(g->turn){
        for(int i = 0; i < n; i++){
            play_move(g, moves[i]);
            score = expectiminimax(g, depth - 1);
            unplay_move(g, moves[i], &anchor, pc);
            heap_insert(&move_heap, moves[i], i, score, symmetric_non_double_move(&g->snake, moves[i]));
        }
        best_score = expected_score_from_heap(g, &move_heap, g->hands.liquid_groups[g->turn].size, g->hands.hand_sizes[g->turn] - g->hands.solid_groups[g->turn].size);
    } else {
        for(int i = 0; i < n; i++){
            play_move(g, moves[i]);
            score = expectiminimax(g, depth - 1);
            unplay_move(g, moves[i], &anchor, pc);
            heap_insert(&move_heap, moves[i], i, score, symmetric_non_double_move(&g->snake, moves[i]));
        }
        best_score = expected_score_from_heap(g, &move_heap, g->hands.liquid_groups[g->turn].size, g->hands.hand_sizes[g->turn] - g->hands.solid_groups[g->turn].size);
    }
    free_heap(&move_heap);
    return prob * pass_score + (1 - prob) * best_score; 
}

Move best_move(Game *g, int depth, float (*ai_function)(Game *, int)){
    Move moves[DCOUNT], best_move;
    int n = 0, cant_pass, pc = g->pass_counter;
    Hands anchor = g->hands;
    float best_score, score;
    get_moves(g, moves, &n, &cant_pass);
    if(g->turn){
        best_score = -FLT_MAX;
        for(int i = 0; i < n; i++){
            play_move(g, moves[i]);
            score = ai_function(g, depth - 1);
            printf("score of move [%d|%d] %d: %f\n", moves[i].play.left, moves[i].play.right, moves[i].type, score);
            unplay_move(g, moves[i], &anchor, pc);
            if(score > best_score){
                best_score = score;
                best_move = moves[i];
            }
        }
    } else {
        best_score = FLT_MAX;
        for(int i = 0; i < n; i++){
            play_move(g, moves[i]);
            score = ai_function(g, depth - 1);
            printf("score of move [%d|%d] %d: %f\n", moves[i].play.left, moves[i].play.right, moves[i].type, score);
            unplay_move(g, moves[i], &anchor, pc);
            if(score < best_score){
                best_score = score;
                best_move = moves[i];
            }
        }
    }
    printf("best score: %f, best move: [%d|%d] %d\n", best_score, best_move.play.left, best_move.play.right, best_move.type);
    return best_move;
}