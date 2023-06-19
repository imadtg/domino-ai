#include "hands.h"

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
    return calc_collapsed_liquid_weight(player, hands, hands->hand_sizes[player] - hands->solid_groups[player].size);
}

float calc_weight(int player, Hands *hands){
    return calc_solid_weight(player, hands) + calc_true_liquid_weight(player, hands);
}

void increment_solid_player(int player, Hands *hands, int i, int j){
    hands->solid_groups[player].size++;
    hands->solid_groups[player].weight += i + j;
}

void decrement_solid_player(int player, Hands *hands, int i, int j){
    hands->solid_groups[player].size--;
    hands->solid_groups[player].weight -= i + j;
}

void increment_solid_boneyard(Hands *hands, int i, int j){
    hands->boneyard_groups_grouped_by_pip[i].size++;
    hands->boneyard_groups_grouped_by_pip[j].size += (i != j);
    hands->solid_groups[NP].size++;
    hands->boneyard_groups_grouped_by_pip[i].weight += i + j;
    hands->boneyard_groups_grouped_by_pip[j].weight += (i != j) * (i + j);
    hands->solid_groups[NP].weight += i + j;
}

void decrement_solid_boneyard(Hands *hands, int i, int j){
    hands->boneyard_groups_grouped_by_pip[i].size--;
    hands->boneyard_groups_grouped_by_pip[j].size -= (i != j);
    hands->solid_groups[NP].size--;
    hands->boneyard_groups_grouped_by_pip[i].weight -= i + j;
    hands->boneyard_groups_grouped_by_pip[j].weight -= (i != j) * (i + j);
    hands->solid_groups[NP].weight -= i + j;
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
    /*hands->liquid_hand_sizes[player]++;
    hands->liquid_weights[player] += i + j;*/
    //hands->liquid_hand_sizes[player]++;
    hands->liquid_groups[player].size++;
    hands->liquid_groups[player].weight += i + j;
}

void decrement_liquid(int player, Hands *hands, int i, int j){
    /*hands->liquid_hand_sizes[player]--;
    hands->liquid_weights[player] -= i + j;*/
    hands->liquid_groups[player].size--;
    hands->liquid_groups[player].weight -= i + j;
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

int hand_is_solid(int player, Hands *hands){
    return hands->solid_groups[player].size == hands->hand_sizes[player];
}

int hand_is_liquid(int player, Hands *hands){
    return hands->solid_groups[player].size == 0;
}

int hand_is_empty(int player, Hands *hands){
    return hands->hand_sizes[player] == 0;
}

int boneyard_is_pickable(Hands *hands){
#if BLOCK
    return 0;
#else
    return !hand_is_empty(NP, hands);
#endif
}

// perfect boneyard picks
void set_sole_owner_pick(int player, Hands *hands, int i, int j){ // needs not be optimized
    if(certain(hands, i, j)){
        decrement_solid_boneyard(hands, i, j);
    } else {
        for(int p = 0; p <= NP; p++){
            if(possible_possession(p, hands, i, j))
                decrement_liquid(p, hands, i, j);
        }
    }
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
    if(!possible_possession(player, hands, i, j) || certain(hands, i, j))
        return;
    and_ownership(~(1 << player), hands, i, j);
    decrement_liquid(player, hands, i, j);
    if(certain(hands, i, j))
        convert_to_solid(sole_owner(hands, i, j), hands, i, j);
}

// game start, sets the piece to be owned by the player
void set_sole_owner_start(int player, Hands *hands, int i, int j){ // unused
    increment_solid_player(player, hands, i, j);
    set_ownership((1 << player), hands, i, j);
}

// game start, sets the piece to not be owned by this person in the game start,
// essentially sets it as possible to be owned by everyone other than this player,
// never used on a boneyard. assumes that there is atleast two possible owners outside of this player.
void set_outside_owner_start(int player, Hands *hands, int i, int j){ // unused
    for(int p = 0; p <= NP; p++){
        if(p != player)
            increment_liquid(p, hands, i, j);
    }
    set_ownership((1 << (NP + 1)) - 1 - (1 << player), hands, i, j);
}

// set this domino to be in every player's liquid group.
void set_everywhere_start(Hands *hands, int i, int j){ // needs not be optimized
    for(int p = 0; p <= NP; p++){
        increment_liquid(p, hands, i, j);
    }
    set_ownership((1 << (NP + 1)) - 1, hands, i, j);
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

void clear_owner_play(int player, Hands *hands, int i, int j){ // plays, (can't be used to erase a piece because it assumes the player has it in a liquid group or a solid group).
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
    if(!collapse_hand(player, hands)) return;
    int y = 2;
    int x = *(int *)((hands->liquid_groups[0].size >= 0) * (unsigned long long) &y);
    for(int p = 0; p <= NP; p++){
        if(p != player)
            cascade_collapse(p, hands);
    }
}

int collapse_hand(int player, Hands *hands){
    if(hand_is_solid(player, hands))
        return 0;
    if(hands->hand_sizes[player] - hands->solid_groups[player].size == hands->liquid_groups[player].size){
        collapse_hand_solidify(player, hands);
        return 1;
    } else if(hands->hand_sizes[player] - hands->solid_groups[player].size == 0){
        collapse_hand_evaporate(player, hands);
        return 1;
    }
    return 0;
}

void emit_collapse(Hands *hands){
    for(int p = 0; p <= NP; p++){
        cascade_collapse(p, hands);
    }
}

int solid_weight(Hands *hands, int player){
    return hands->solid_groups[player].weight;
}

float collapsed_liquid_weight(Hands *hands, int player){
    if(hands->liquid_groups[player].size == 0)
        return 0.0f;
    return (float) (hands->liquid_groups[player].weight * (hands->hand_sizes[player] - hands->solid_groups[player].size)) / hands->liquid_groups[player].size;
}

float weight(Hands *hands, int player){
    return collapsed_liquid_weight(hands, player) + solid_weight(hands, player);
}

void print_hand(Hands *hands, int player) {
    int max = 0, min = 0;
    if(player == NP) printf("Boneyard:");
    else printf("Player %d:", player);
    for(int i = 0; i < PIPS; i++) {
        for(int j = 0; j <= i; j++) {
            if(possible_possession(player, hands, i, j)){
                max++;
                printf(" [%d|%d]", i, j);
                if(certain(hands, i, j)){
                    min++;
                    printf("*");
                }
            }
        }
    }
    int sw = solid_weight(hands, player);
    float lw = collapsed_liquid_weight(hands, player);
    int tsw = calc_solid_weight(player, hands);
    float tlw = calc_true_liquid_weight(player, hands);
    printf("\nsizes: liquid = %d, solid = %d, true = %d, weights: %d (%d)+ %f (%f)= %f\n", hands->liquid_groups[player].size, hands->solid_groups[player].size, hands->hand_sizes[player], tsw, sw, tlw, lw, weight(hands, player));
}

void print_hands(Hands *hands){
    for(int i = 0; i <= NP; i++) {
        print_hand(hands, i);
    }
}

void get_hand_sizes(Hands *hands) {
    // TODO: allow silent games
    for(int i = 0; i < NP; i++){
        printf("Player %d: ", i);
        scanf("%d", &hands->hand_sizes[i]);
    }
    printf("Boneyard: ");
    scanf("%d", &hands->hand_sizes[NP]);
}

// initialize a hands struct
void init_hands(Hands *hands){
    memset(hands, 0, sizeof(*hands));
    for(int i = 0; i < PIPS; i++){
        for(int j = 0; j <= i; j++){
            set_everywhere_start(hands, i, j);
        }
    }
}

void get_hand(Hands *hands, int player) { // reads the dominoes of one player, while eliminating all others from this hand.
    // TODO: make a version that allows silent games
    int left, right;
    int mask[PIPS][PIPS];
    memset(mask, 0, PIPS * PIPS * sizeof(int));
    if(player == NP) printf("Give boneyard: \n");
    else printf("Give player %d's dominoes: \n", player);
    for(int i = 0; i < hands->hand_sizes[player]; i++){
        printf("Give domino %d: ", i);
        do{
            scanf("%d %d", &left, &right);
        }while(left < 0 || left >= PIPS || right < 0 || right > PIPS || mask[left][right]);
        mask[left][right] = 1;
        mask[right][left] = 1;
    }
    for(int i = 0; i < PIPS; i++){
        for(int j = 0; j <= i; j++){
            if(mask[i][j]){
                collapse_piece(player, hands, i, j);
            } else {
                absent_piece(player, hands, i, j);
            }
        }
    }
    emit_collapse(hands);
}