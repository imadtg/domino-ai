#include "hands.h"

// initialize a hands struct
void init_hands(Hands *hands){
    memset(hands, 0, sizeof(*hands));
}

int calc_solid_weight(int player, Hands *hands){
    int w = 0;
    for(int i = 0; i < PIPS; i++){
        for(int j = 0; j <= i; j++){
            if(sole_possession(player, hands, i, j)){
                w += i + j;
            }
        }
    }
    return w;
}

int calc_liquid_weight(int player, Hands *hands, int *size){
    int w = 0;
    *size = 0;
    for(int i = 0; i < PIPS; i++){
        for(int j = 0; j <= i; j++){
            if(possible_possession(player, hands, i, j) && !certain(hands, i, j)){
                w += i + j;
                (*size)++;
            }
        }
    }
    return w;
}

float calc_collapsed_liquid_weight(int player, Hands *hands, int collapsing_size){
    int size, w;
    w = calc_liquid_weight(player, hands, &size);
    if(!size)
        return 0.0;
    return (float) (w * collapsing_size) / size;   
}

float calc_true_liquid_weight(int player, Hands *hands){
    return calc_collapsed_liquid_weight(player, hands, hands->hand_sizes[player] - hands->solid_hand_sizes[player]);
}

float calc_weight(int player, Hands *hands){
    return calc_solid_weight(player, hands) + calc_true_liquid_weight(player, hands);
}

void increment_solid_player(int player, Hands *hands, int i, int j){
    hands->solid_hand_sizes[player]++;
    hands->solid_weights[player] += i + j;
}

void decrement_solid_player(int player, Hands *hands, int i, int j){
    hands->solid_hand_sizes[player]--;
    hands->solid_weights[player] -= i + j;
}

int boneyard_info_sound(Hands *hands){
    int c = 0;
    for(int i = 0; i < PIPS; i++){
        c += hands->boneyard_solid_group_sizes[i];
    }
    if(c != hands->solid_hand_sizes[NP]) return 0;
    return 1;
}

void increment_solid_boneyard(Hands *hands, int i, int j){
    hands->boneyard_solid_group_sizes[i]++;
    hands->solid_hand_sizes[NP]++;
    hands->boneyard_solid_group_weights[i] += i + j;
    hands->boneyard_solid_group_weights[j] += (i != j) * (i + j);
    hands->solid_weights[NP] += i + j;
}

void decrement_solid_boneyard(Hands *hands, int i, int j){
    hands->boneyard_solid_group_sizes[i]--;
    hands->solid_hand_sizes[NP]--;
    hands->boneyard_solid_group_weights[i] -= i + j;
    hands->boneyard_solid_group_weights[j] -= (i != j) * (i + j);
    hands->solid_weights[NP] -= i + j;
}

void increment_solid(int player, Hands *hands, int i, int j){
    if(player == NP)
        increment_solid_boneyard(hands, i, j);
    else
        increment_solid_player(player, hands, i, j);
}

void decrement_solid(int player, Hands *hands, int i, int j){
    if(player == NP)
        decrement_solid_boneyard(hands, i, j);
    else
        decrement_solid_player(player, hands, i, j);
}

void increment_liquid(int player, Hands *hands, int i, int j){
    hands->liquid_hand_sizes[player]++;
    hands->liquid_weights[player] += i + j;
}

void decrement_liquid(int player, Hands *hands, int i, int j){
    hands->liquid_hand_sizes[player]--;
    hands->liquid_weights[player] -= i + j;
}

void convert_to_liquid_player(int player, Hands *hands, int i, int j){
    decrement_solid_player(player, hands, i, j);
    increment_liquid(player, hands, i, j);
}

void convert_to_solid_player(int player, Hands *hands, int i, int j){
    decrement_liquid(player, hands, i, j);
    increment_solid_player(player, hands, i, j);
}

void convert_to_solid_boneyard(Hands *hands, int i, int j){
    decrement_liquid(NP, hands, i, j);
    increment_solid_boneyard(hands, i, j);
}

void convert_to_liquid_boneyard(Hands *hands, int i, int j){
    decrement_solid_boneyard(hands, i, j);
    increment_liquid(NP, hands, i, j);
}

void convert_to_solid(int player, Hands *hands, int i, int j){
    if(player == NP)
        convert_to_solid_boneyard(hands, i, j);
    else
        convert_to_solid_player(player, hands, i, j);
}

void convert_to_liquid(int player, Hands *hands, int i, int j){
    if(player == NP)
        convert_to_liquid_boneyard(hands, i, j);
    else
        convert_to_liquid_player(player, hands, i, j);
}

void set_ownership(int value, Hands *hands, int i, int j){
    hands->ownership[i][j] = value;
    hands->ownership[j][i] = value;
}

void or_ownership(int value, Hands *hands, int i, int j){
    hands->ownership[i][j] |= value;
    hands->ownership[j][i] |= value;
}

void and_ownership(int value, Hands *hands, int i, int j){
    hands->ownership[i][j] &= value;
    hands->ownership[j][i] &= value;
}

int possible_possession(int player, Hands *hands, int i, int j){
    return hands->ownership[i][j] & (1 << player);
}

int sole_possession(int player, Hands *hands, int i, int j){
    return hands->ownership[i][j] == (1 << player);
}

int no_possession(Hands *hands, int i, int j){
    return !hands->ownership[i][j];
}

// essentially tests if we know for certain where a piece is, 
// whether in a player's hand, the boneyard, or already played.
int certain(Hands *hands, int i, int j){ 
    return !(hands->ownership[i][j] & (hands->ownership[i][j] - 1));
}

// returns the sole owner of a piece, assuming he exists.
int sole_owner(Hands *hands, int i, int j){
    return __builtin_ctz(hands->ownership[i][j]);
}

// perfect boneyard picks
void set_sole_owner_pick(int player, Hands *hands, int i, int j){ // needs not be optimized
    if(certain(hands, i, j))
        decrement_solid_boneyard(hands, i, j);
    else
        decrement_liquid(NP, hands, i, j);
    increment_solid_player(player, hands, i, j);
    set_ownership((1 << player), hands, i, j);
}

void collapse_piece(int player, Hands *hands, int i, int j){ // collapse piece to the player
    if(certain(hands, i, j) || !possible_possession(player, hands, i, j))
        return;
    for(int p = 0; p <= NP; p++){
        if(possible_possession(p, hands, i, j) && p != player)
            decrement_liquid(p, hands, i, j);
    }
    convert_to_solid(player, hands, i, j);
    set_ownership((1 << player), hands, i, j);
}

void absent_piece(int player, Hands *hands, int i, int j){ // piece is not in the player's hand
    /*if(!possible_possession(player, hands, i, j))
        return;
    if(certain(hands, i, j))
        decrement_solid_player(player, hands, i, j);
    else 
        decrement_liquid(player, hands, i, j);*/
    if(!possible_possession(player, hands, i, j))
        return;
    and_ownership(~(1 << player), hands, i, j);
    decrement_liquid(player, hands, i, j);
    if(certain(hands, i, j))
        convert_to_solid(sole_owner(hands, i, j), hands, i, j);
}

// game start, sets the piece to be owned by the player
void set_sole_owner_start(int player, Hands *hands, int i, int j){
    increment_solid_player(player, hands, i, j);
    set_ownership((1 << player), hands, i, j);
}

// game start, sets the piece to not be owned by this person in the game start,
// essentially sets it as possible to be owned by everyone other than this player,
// never used on a boneyard. assumes that there is atleast two possible owners outside of this player.
void set_outside_owner_start(int player, Hands *hands, int i, int j){ // needs not be optimized
    for(int p = 0; p <= NP; p++){
        if(p != player)
            increment_liquid(p, hands, i, j);
    }
    set_ownership((1 << (NP + 1)) - 1 - (1 << player), hands, i, j);
}

void set_possible_owner_pick(int player, Hands *hands, int i, int j){ // imperfect picks, needs not be optimized
    if(possible_possession(player, hands, i, j))
        return;
    if(certain(hands, i, j))
        convert_to_liquid_boneyard(hands, i, j);
    increment_liquid(player, hands, i, j);
    or_ownership((1 << player), hands, i, j);
}

void clear_owner_pass(int player, Hands *hands, int i, int j){ // passes, (works for hand collapses).
    absent_piece(player, hands, i, j);
}

void clear_owner_play(int player, Hands *hands, int i, int j){ // plays, (can be used to erase a piece).
    if(certain(hands, i, j)){
        decrement_solid_player(player, hands, i, j);
    } else {
        for(int p = 0; p <= NP; p++){
            if(possible_possession(p, hands, i, j))
                decrement_liquid(p, hands, i, j);
        }
    }
    set_ownership(0, hands, i, j);
}

// collapse hand, assumes that part of the hand is liquid.
void collapse_hand_solidify(int player, Hands *hands){
    for(int i = 0; i < PIPS; i++){
        for(int j = 0; j <= i; j++){
            if(possible_possession(player, hands, i, j) && !certain(hands, i, j))
                collapse_piece(player, hands, i, j);
        }
    }
}

void collapse_hand_evaporate(int player, Hands *hands){
    for(int i = 0; i < PIPS; i++){
        for(int j = 0; j <= i; j++){
            if(possible_possession(player, hands, i, j) && !certain(hands, i, j))
                absent_piece(player, hands, i, j);
        }
    }
}

void cascade_collapse(int player, Hands *hands){
    if(!hands->liquid_hand_sizes[player])
        return;
    if(hands->hand_sizes[player] - hands->solid_hand_sizes[player] == hands->liquid_hand_sizes[player])
        collapse_hand_solidify(player, hands);
    else if(hands->hand_sizes[player] - hands->solid_hand_sizes[player] == 0)
        collapse_hand_evaporate(player, hands);
    else return;
    for(int p = 0; p <= NP; p++){
        if(p != player)
            cascade_collapse(p, hands);
    }
}

void emit_collapse(Hands *hands){
    for(int p = 0; p <= NP; p++){
        cascade_collapse(p, hands);
    }
}

int solid_weight(Hands *hands, int player){
    return hands->solid_weights[player];
}

float collapsed_liquid_weight(Hands *hands, int player){
    if(hands->liquid_hand_sizes[player] == 0)
        return 0.0f;
    return (float) (hands->liquid_weights[player] * (hands->hand_sizes[player] - hands->solid_hand_sizes[player])) / hands->liquid_hand_sizes[player];
}

float weight(Hands *hands, int player){
    return collapsed_liquid_weight(hands, player) + solid_weight(hands, player);
}

void print_hand(Hands *hands, int player) {
    if(player == NP) printf("Boneyard:");
    else printf("Player %d:", player);
    for(int i = 0; i < PIPS; i++) {
        for(int j = 0; j <= i; j++) {
            if(possible_possession(player, hands, i, j)){
                printf(" [%d|%d]", i, j);
                if(certain(hands, i, j))
                    printf("*");
            }
        }
    }
    int sw = solid_weight(hands, player);
    float lw = collapsed_liquid_weight(hands, player);
    int tsw = calc_solid_weight(player, hands);
    float tlw = calc_true_liquid_weight(player, hands);
    printf(" \nweights: %d (%d)+ %f (%f)= %f\n", tsw, sw, tlw, lw, weight(hands, player));
}

void get_hand_sizes(Hands *hands) {
    for(int i = 0; i < NP; i++){
        printf("Player %d: ", i);
        scanf("%d", &hands->hand_sizes[i]);
    }
    printf("Boneyard: ");
    scanf("%d", &hands->hand_sizes[NP]);
}

void get_hands(Hands *hands) {
    int a;
    for(int i = 0; i < PIPS; i++){
        for(int j = 0; j <= i; j++){
            printf("[%d|%d]: ", i, j);
            scanf("%d", &a);
            if(a){
                set_sole_owner_start(1, hands, i, j);
            } else {
                set_outside_owner_start(1, hands, i, j);
            }
        }
    }
}