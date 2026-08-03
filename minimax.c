#include "minimax.h"
#include <stdatomic.h>
#include <stdint.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#define KEEPALIVE EMSCRIPTEN_KEEPALIVE
#else
#define KEEPALIVE
#endif

volatile _Atomic int32_t FALLBACK = 0;

KEEPALIVE
volatile uintptr_t get_fallback_ptr(){
    return (uintptr_t)&FALLBACK;
}

KEEPALIVE
int get_fallback(){
    return atomic_load_explicit(&FALLBACK, memory_order_seq_cst);;
}

KEEPALIVE
void set_fallback(){
    atomic_store_explicit(&FALLBACK, 1, memory_order_seq_cst);
}

KEEPALIVE
void reset_fallback(){
    atomic_store_explicit(&FALLBACK, 0, memory_order_seq_cst);
}

#ifdef _WIN32
WINBOOL interrupt_search(DWORD ctrl_type){
    if (ctrl_type == CTRL_C_EVENT) {
        set_fallback();
        SetConsoleCtrlHandler((PHANDLER_ROUTINE)interrupt_search, FALSE);
        return TRUE;
    }
    return FALSE;
}
#endif

float max(float x, float y){
    if(x > y)
        return x;
    return y;
}

float min(float x, float y){
    if(x < y)
        return x;
    return y;
}

KEEPALIVE
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

void process_absence(Game *g, Hands *anchor, float *pass_score, float *prob, int n, int depth, int skip, int *nodes, float (*ai_function)(Game *, int, int, int *)){
    *prob = pass_probability_from_num_moves(g, n);
    if(*prob != 0){
        if(boneyard_is_pickable(&g->hands)){ // we have to handle picking from the boneyard
            if(is_passing(g, NP)){ // there is no playable domino left in the boneyard, we will pick it all.
                Move pick_all_of_boneyard;
                pick_all_of_boneyard.type = IMPERFECT_PICK;
                pick_all_of_boneyard.imperfect_pick.count = g->hands.hand_sizes[NP];
                imperfect_pick(g, pick_all_of_boneyard);
                *pass_score = ai_function(g, depth, skip, nodes);
                undo_imperfect_pick(g, anchor);
            } else {
                absence_event(g);
                if(skip){ // to reduce the branching factor, we can estimate the score by the average weight difference after picking from the boneyard
                    *pass_score = pick_loss_evaluation(g);
                } else {
                    if(hand_is_solid(NP, &g->hands) || hand_is_solid(g->turn, &g->hands)){ // do not introduce uncertainty by doing imperfect picks if either boneyard or hand is known.
                        Move perfect_picking_moves[g->hands.liquid_groups[NP].size + g->hands.solid_groups[NP].size];
                        int nperf;
                        get_perfect_picking_moves(g, perfect_picking_moves, &nperf);
                        *pass_score = 0;
                        if(nperf == 0){
                            printf("no perfect move\n");
                        }
                        for(int i = 0; i < nperf; i++){
                            perfect_pick(g, perfect_picking_moves[i]);
                            *pass_score += ai_function(g, depth - 1, skip, nodes);
                            undo_perfect_pick(g, anchor);
                        }
                        *pass_score /= nperf;
                    } else { // TODO: explore imperfect picks of all sizes followed by all playable perfect picks.
                        Move playable_perfect_picking_moves[MAX_NUM_PLY_MOVE];
                        int nplayperf;
                        get_playable_perfect_picking_moves(g, playable_perfect_picking_moves, &nplayperf);
                        float unplayable_prob = pick_unplayable_domino_probability_from_moves(g, playable_perfect_picking_moves, nplayperf);
                        //print_game(g);
                        //print_picking_moves(playable_perfect_picking_moves, nplayperf);
                        //printf("Probability of picking unplayable move : %f", unplayable_prob);
                        float pick_playable_score = 0, pick_unplayable_score = 0;
                        for(int i = 0; i < nplayperf; i++){
                            perfect_pick(g, playable_perfect_picking_moves[i]);
                            pick_playable_score += ai_function(g, depth - 1, skip, nodes);
                            undo_perfect_pick(g, anchor);
                        }
                        if(!nplayperf) printf("uh oh");
                        pick_playable_score /= nplayperf;
                        if(unplayable_prob != 0){
                            Move pick_unplayable;
                            pick_unplayable.type = IMPERFECT_PICK;
                            pick_unplayable.imperfect_pick.count = 1;
                            imperfect_pick(g, pick_unplayable);
                            pick_unplayable_score = ai_function(g, depth - 1 ,skip, nodes);
                            undo_imperfect_pick(g, anchor);
                        }
                        *pass_score = unplayable_prob * pick_unplayable_score + (1 - unplayable_prob) * pick_playable_score;
                    }
                }
                undo_absence_event(g, anchor);
            }
        } else {
            pass(g);
            *pass_score = ai_function(g, depth, skip, nodes);
            undo_pass(g, anchor);
        }
    }
}

float minimax(Game *g, int depth, int skip, int *nodes){
    if(get_fallback())
        return 0;
    (*nodes)++;
    if(over(g))
        return endgame_evaluation(g);
    if(depth == 0)
        return heuristic_evaluation(g);
    int n, cant_pass, pc = g->pass_counter;
    Move moves[MAX_NUM_PLY_MOVE];
    Hands anchor = g->hands;
    float pass_score = 0, prob = 0, score, best_score;
    get_playing_moves(g, moves, &n, &cant_pass);
    if(!cant_pass){
        process_absence(g, &anchor, &pass_score, &prob, n, depth, skip, nodes, minimax);
    }
    if(g->turn){
        best_score = -FLT_MAX;
        for(int i = 0; i < n; i++){
            play_move(g, moves[i]);
            score = minimax(g, depth - 1, skip, nodes);
            unplay_move(g, moves[i], &anchor, pc);
            best_score = max(best_score, score);
        }
    } else {
        best_score = FLT_MAX;
        for(int i = 0; i < n; i++){
            play_move(g, moves[i]);
            score = minimax(g, depth - 1, skip, nodes);
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

float expectiminimax(Game *g, int depth, int skip, int *nodes){
    if(get_fallback())
        return 0;
    (*nodes)++;
    if(over(g))
        return endgame_evaluation(g);
    if(depth == 0)
        return heuristic_evaluation(g);
    int n, cant_pass, pc = g->pass_counter;
    Move moves[MAX_NUM_PLY_MOVE];
    Heap move_heap;
    Hands anchor = g->hands;
    float pass_score = 0, prob = 0, score, best_score;
    get_playing_moves(g, moves, &n, &cant_pass);
    if(!cant_pass){
        process_absence(g, &anchor, &pass_score, &prob, n, depth, skip, nodes, expectiminimax);
    }
    if(n == 0) return pass_score;
    init_heap(&move_heap, n, g->turn ? greater_than : less_than);
    if(g->turn){
        for(int i = 0; i < n; i++){
            play_move(g, moves[i]);
            score = expectiminimax(g, depth - 1, skip, nodes);
            unplay_move(g, moves[i], &anchor, pc);
            heap_insert(&move_heap, moves[i], i, score, symmetric_non_double_move(&g->snake, moves[i]));
        }
        best_score = expected_score_from_heap(g, &move_heap, g->hands.liquid_groups[g->turn].size, g->hands.hand_sizes[g->turn] - g->hands.solid_groups[g->turn].size);
    } else {
        for(int i = 0; i < n; i++){
            play_move(g, moves[i]);
            score = expectiminimax(g, depth - 1, skip, nodes);
            unplay_move(g, moves[i], &anchor, pc);
            heap_insert(&move_heap, moves[i], i, score, symmetric_non_double_move(&g->snake, moves[i]));
        }
        best_score = expected_score_from_heap(g, &move_heap, g->hands.liquid_groups[g->turn].size, g->hands.hand_sizes[g->turn] - g->hands.solid_groups[g->turn].size);
    }
    free_heap(&move_heap);
    return prob * pass_score + (1 - prob) * best_score; 
}

Move best_move(Game *g, Move moves[], float scores[], int n, int depth, int skip, int *nodes, float (*ai_function)(Game *, int, int, int *)){
    float dummy_scores[n];
    int dummy_nodes;
    if(scores == NULL)
        scores = dummy_scores;
    if(nodes == NULL)
        nodes = &dummy_nodes;
    Move best_move;
    int pc = g->pass_counter;
    Hands anchor = g->hands;
    float best_score, score;
    if(g->turn){
        best_score = -FLT_MAX;
        for(int i = 0; i < n; i++){
            play_move(g, moves[i]);
            score = ai_function(g, depth - 1, skip, nodes);
            scores[i] = score;
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
            score = ai_function(g, depth - 1, skip, nodes);
            scores[i] = score;
            unplay_move(g, moves[i], &anchor, pc);
            if(score < best_score){
                best_score = score;
                best_move = moves[i];
            }
        }
    }
    return best_move;
}

#ifdef _WIN32
Move iterative_deepening(Game *g, Move moves[], int n, int skip, float (*ai_function)(Game *, int, int, int *)){
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)interrupt_search, TRUE);
    int depth = 1, nodes = 0, prev_nodes = 0;
    float scores[n], last_scores[n];
    Move best = best_move(g, moves, scores, n, depth, skip, &nodes, ai_function), last_best;
    while(!get_fallback() && prev_nodes < nodes){
        last_best = best;
        prev_nodes = nodes;
        for(int i = 0; i < n; i++)
            last_scores[i] = scores[i];
        for(int i = 0; i < n; i++)
            printf("score of move [%d|%d] %d: %f\n", moves[i].play.left, moves[i].play.right, moves[i].type, last_scores[i]);
        printf("depth: %d, best move = [%d|%d] %d, nodes: %d\n", depth, last_best.play.left, last_best.play.right, last_best.type, prev_nodes);
        depth++;
        nodes = 0;
        best = best_move(g, moves, scores, n, depth, skip, &nodes, ai_function);
    }
    reset_fallback();
    for(int i = 0; i < n; i++)
        printf("score of move [%d|%d] %d: %f\n", moves[i].play.left, moves[i].play.right, moves[i].type, last_scores[i]);
    printf("best move: [%d|%d] %d\n", last_best.play.left, last_best.play.right, best.type);
    return last_best;
}
#endif