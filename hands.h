#ifndef HANDS_H
#define HANDS_H
#include "cst.h"

typedef struct {
    int ownership[PIPS][PIPS];
    int hand_sizes[NP+1];
    // this is the sum of weights of all the dominoes that the player can have,
    // not to be confused with the sum of weights of the dominoes in the player's hand
    int liquid_weights[NP+1];
    int liquid_hand_sizes[NP+1]; // this is the number of distinct dominoes the player can have, not to be confused with the number of dominoes in the player's hand
    int solid_weights[NP+1]; // this is the sum of weights of all the dominoes that we are sure the player has
    int solid_hand_sizes[NP+1]; // this is the number of distinct dominoes that we are sure the player has
    int boneyard_solid_group_weights[PIPS]; // this is the weight of each group of dominoes by pip value that we are sure is in the boneyard, used to calculate average hand weight after picking up from a boneyard.
    int boneyard_solid_group_sizes[PIPS]; // this is the number of dominoes in each group of dominoes by pip value that we are sure is in the boneyard, used to calculate average hand weight after picking up from a boneyard.
} Hands;


void init_hands(Hands *hands);

// Weight calculation
int calc_solid_weight(int player, Hands *hands);
int calc_liquid_weight(int player, Hands *hands, int *size);
float calc_collapsed_liquid_weight(int player, Hands *hands, int collapsing_size);
float calc_true_liquid_weight(int player, Hands *hands);
float calc_weight(int player, Hands *hands);

// Solid weight management
void increment_solid_player(int player, Hands *hands, int i, int j);
void decrement_solid_player(int player, Hands *hands, int i, int j);
void increment_solid_boneyard(Hands *hands, int i, int j);
void decrement_solid_boneyard(Hands *hands, int i, int j);
void increment_solid(int player, Hands *hands, int i, int j);
void decrement_solid(int player, Hands *hands, int i, int j);

// Liquid weight management
void increment_liquid(int player, Hands *hands, int i, int j);
void decrement_liquid(int player, Hands *hands, int i, int j);

// Conversions
void convert_to_liquid_player(int player, Hands *hands, int i, int j);
void convert_to_solid_player(int player, Hands *hands, int i, int j);
void convert_to_solid_boneyard(Hands *hands, int i, int j);
void convert_to_liquid_boneyard(Hands *hands, int i, int j);
void convert_to_solid(int player, Hands *hands, int i, int j);
void convert_to_liquid(int player, Hands *hands, int i, int j);

// Ownership management
void set_ownership(int value, Hands *hands, int i, int j);
void or_ownership(int value, Hands *hands, int i, int j);
void and_ownership(int value, Hands *hands, int i, int j);

// Ownership queries
int possible_possession(int player, Hands *hands, int i, int j);
int sole_possession(int player, Hands *hands, int i, int j);
int no_possession(Hands *hands, int i, int j);
int certain(Hands *hands, int i, int j);
int sole_owner(Hands *hands, int i, int j);

// Ownership modifications after moves/start
void set_sole_owner_pick(int player, Hands *hands, int i, int j);
void collapse_piece(int player, Hands *hands, int i, int j);
void absent_piece(int player, Hands *hands, int i, int j);
void set_sole_owner_start(int player, Hands *hands, int i, int j);
void set_outside_owner_start(int player, Hands *hands, int i, int j);
void set_possible_owner_pick(int player, Hands *hands, int i, int j);
void clear_owner_pass(int player, Hands *hands, int i, int j);
void clear_owner_play(int player, Hands *hands, int i, int j);
void collapse_hand_evaporate(int player, Hands *hands);
void collapse_hand_solidify(int player, Hands *hands);
void cascade_collapse(int player, Hands *hands);
void emit_collapse(Hands *hands);

// Weight queries
int solid_weight(Hands *hands, int player);
float collapsed_liquid_weight(Hands *hands, int player);
float weight(Hands *hands, int player);

// Hand print
void print_hand(Hands *hands, int player);

// Hand read
void get_hand_sizes(Hands *hands);
void get_hands(Hands *hands);

#endif