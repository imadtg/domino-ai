#ifndef CST_H
#define CST_H
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

#define BLOCK 0 // block picking from the boneyard, like in the blocking game.
#define PIPS 7 // the number of possible pip numbers, counts from 0 to PIPS-1.
#define MAX_NUM_PLY_MOVE (PIPS*2+1) // max number of possible moves assuming snake is nonempty
#define NP 2 // the number of players, it is 2 for now.
#define DCOUNT ((PIPS*(PIPS+1))/2) // total number of distinct dominoes
#define LGCOUNT (DCOUNT/2+1) // this is embarassing


#endif