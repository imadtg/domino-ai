#ifndef HANDS_H
#define HANDS_H
#include "cst.h"

typedef struct {
    int size; // the span of the group
    int weight; // the weight of the group
} GroupInfo;

typedef GroupInfo LiquidGroup;

typedef GroupInfo SolidGroup;

typedef struct {
    int ownership[PIPS][PIPS]; // the mask of ownership of every domino, is 1 in the bit in the position of the player if the player can have the domino, 0 otherwise.
    int hand_sizes[NP+1];
    LiquidGroup liquid_groups[NP+1]; // this is the table that stores the liquid groups of each player's hand.
    SolidGroup solid_groups[NP+1]; // this is the table that stores the solid group of each player's hand.
    SolidGroup boneyard_groups_grouped_by_pip[PIPS]; // this table stores the weight and number of solid dominoes grouped by pip in the boneyard, used for pick loss evaluation.
} Hands;

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

// Hand testing
int hand_is_solid(int player, Hands *hands);
int hand_is_liquid(int player, Hands *hands);
int hand_is_empty(int player, Hands *hands);
int boneyard_is_pickable(Hands *hands);

// Ownership modifications after moves/start
void set_sole_owner_pick(int player, Hands *hands, int i, int j);
void collapse_piece(int player, Hands *hands, int i, int j);
void absent_piece(int player, Hands *hands, int i, int j);
void set_sole_owner_start(int player, Hands *hands, int i, int j);
void set_outside_owner_start(int player, Hands *hands, int i, int j);
void set_everywhere_start(Hands *hands, int i, int j);
void set_possible_owner_pick(int player, Hands *hands, int i, int j);
void clear_owner_pass(int player, Hands *hands, int i, int j);
void clear_owner_play(int player, Hands *hands, int i, int j);
void collapse_hand_solidify(int player, Hands *hands);
void collapse_hand_evaporate(int player, Hands *hands);
void cascade_collapse(int player, Hands *hands);
int collapse_hand(int player, Hands *hands);
void emit_collapse(Hands *hands);

// Weight queries
int solid_weight(Hands *hands, int player);
float collapsed_liquid_weight(Hands *hands, int player);
float weight(Hands *hands, int player);

// Hand print
void print_hand(Hands *hands, int player);
void print_hands(Hands *hands);

// Hand initialization
void get_hand_sizes(Hands *hands);
void init_hands(Hands *hands);
void get_hand(Hands *hands, int player);

#endif