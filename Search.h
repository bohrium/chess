#ifndef SEARCH_H
#define SEARCH_H

#include "Board.h"

#define MAX_VERBOSE 4

/* BASIC SEARCH PARAMETERS */

#define SCOUT_THRESH 5       // centipawns
#define QUIESCE_DEPTH 4      // plies    
#define STABLE_ORDER_DEPTH 3 // plies

                             /*    0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24 */
const int branching_factors[] = { -1, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36}; // nb siblings
const int ordering_depths[]   = { -1,  0,  0,  1,  1,  2,  2,  3,  3,  4,  4,  5,  5,  6,  6,  7,  7,  8,  8,  9,  9, 10, 10, 11, 11}; // plies

/* REDUCTION PARAMETERS */

#define MIN_FILTER_DEPTH 4 // plies 

#define ALLOW_AR  1
#define ALLOW_LMR 1
#define ALLOW_NMR 1
#define ALLOW_CSR 1 /* TODO: turn back on */ 

#define AR_THRESH   6 // nb siblings
#define AR_AMOUNT   2 // fracplies 
#define LMR_THRESH  2 // nb siblings
#define LMR_AMOUNT  4 // fracplies 
#define NMR_THRESH  0 // centipawns
#define NMR_AMOUNT  4 // fracplies
#define CSR_THRESH 50 // centipawns
#define CSR_AMOUNT  1 // fracplies

//#define ALLOW_GBR 1
//#define GBR_AMOUNT 1 // plies  

/* SEARCH METHODS */

void zero_tables();

struct ScoredMove {
    Move m;
    int score;
};
ScoredMove get_best_move(Board* B, int depth, int alpha, int beta, bool stable, bool null_move_okay, int verbose);
void print_pv(Board* B, int depth, int verbose); 

#endif//SEARCH_H
