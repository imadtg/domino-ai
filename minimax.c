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

float pick_loss_evaluation(Game *g){
    int left = g->snake.tail->domino.left;
    int right = g->snake.head->domino.right;
    int n = (g->hands.boneyard_solid_group_sizes[left] + g->hands.boneyard_solid_group_sizes[right]) >> (left == right) \
            - (left != right) * sole_possession(NP, &g->hands, left, right); // number of dominoes that can be played in the boneyard
    int wn = (g->hands.boneyard_solid_group_weights[left] + g->hands.boneyard_solid_group_weights[right]) >> (left == right) \
            - (left != right) * sole_possession(NP, &g->hands, left, right) * (left + right); // their weight
    float x = (float) (g->hands.hand_sizes[NP] - n) / (n + 1); // average number of dominoes picked before picking a playable domino
    return heuristic_evaluation(g) + (1 - 2 * g->turn) * (weight(&g->hands, NP) - wn) * x;
}

float max(float a, float b) {
    if (a > b) {
        return a;
    }
    return b;
}

float min(float a, float b) {
    if (a < b) {
        return a;
    }
    return b;
}

float minimax(Game *g, int depth, int maximizing_player){
    if(over(g))
        return endgame_evaluation(g);
    if(depth == 0)
        return heuristic_evaluation(g);
    int n, cant_pass;
    Move moves[MAX];
    Hands anchor = g->hands;
    float pass_score = 0, prob = 0, score, best_score;
    get_moves(g, moves, &n, &cant_pass);
    if(!cant_pass){
        prob = pass_probability(g, n - possible_possession(g->turn, &g->hands, g->snake.head->domino.left, g->snake.head->domino.right));
        if(g->hands.hand_sizes[NP]){
            pass_score = pick_loss_evaluation(g);
        } else {
            pass(g);
            pass_score = minimax(g, depth, !maximizing_player);
            undo_pass(g, &anchor);
        }
    }
    if(maximizing_player){
        best_score = -FLT_MAX;
        for(int i = 0; i < n; i++){
            play_move(g, moves[i]);
            score = minimax(g, depth - 1, !maximizing_player);
            unplay_move(g, moves[i], &anchor);
            best_score = max(best_score, score);
        }
    } else {
        best_score = FLT_MAX;
        for(int i = 0; i < n; i++){
            play_move(g, moves[i]);
            score = minimax(g, depth - 1, !maximizing_player);
            unplay_move(g, moves[i], &anchor);
            best_score = min(best_score, score);
        }
    }
    return prob * pass_score + (1 - prob) * best_score; 
}

Move best_move(Game *g, int depth){
    Move moves[DCOUNT], best_move;
    int n = 0, cant_pass;
    Hands anchor = g->hands;
    float best_score, score;
    get_moves(g, moves, &n, &cant_pass);
    if(g->turn){
        best_score = -FLT_MAX;
        for(int i = 0; i < n; i++){
            play_move(g, moves[i]);
            score = minimax(g, depth - 1, g->turn);
            unplay_move(g, moves[i], &anchor);
            if(score > best_score){
                best_score = score;
                best_move = moves[i];
            }
        }
    } else {
        best_score = FLT_MAX;
        for(int i = 0; i < n; i++){
            play_move(g, moves[i]);
            score = minimax(g, depth - 1, g->turn);
            unplay_move(g, moves[i], &anchor);
            if(score < best_score){
                best_score = score;
                best_move = moves[i];
            }
        }
    }
    printf("best score: %f, best move: [%d|%d] %d\n", best_score, best_move.play.left, best_move.play.right, best_move.type);
    return best_move;
}