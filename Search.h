#ifndef SEARCH_H
#define SEARCH_H

#include "Board.h"

#define MAX_VERBOSE 4

/* BASIC SEARCH PARAMETERS */

#define SCOUT_THRESH       5 // centipawns
#define QUIESCE_DEPTH      4 // plies    
#define STABLE_ORDER_DEPTH 3 // plies

                             /*    0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24 */
const int branching_factors[] = { -1, -1, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36}; // nb siblings
const int ordering_depths[]   = { -1, -1,  0,  1,  1,  2,  2,  3,  3,  4,  4,  5,  5,  6,  6,  7,  7,  8,  8,  9,  9, 10, 10, 11, 11}; // plies

/* REDUCTION PARAMETERS */

#define MIN_FILTER_DEPTH 3 // plies 

#define ALLOW_AR  1
#define ALLOW_LMR 1
#define ALLOW_NMR 1
#define ALLOW_CSR 1

#define AR_THRESH   6 // nb siblings
#define AR_AMOUNT   1 // fracplies 
#define LMR_THRESH  2 // nb siblings
#define LMR_AMOUNT  2 // fracplies 
#define NMR_THRESH 50 // centipawns
#define NMR_AMOUNT  2 // fracplies
#define CSR_THRESH 50 // centipawns
#define CSR_AMOUNT  1 // fracplies

/* SEARCH METHODS */

struct ScoredMove {
    Move m;
    int score;
};

typedef struct PVRecord {
    unsigned int hash;
    int alpha;
    int beta;
    ScoredMove sm;
} PVRecord;
#define PV_TABLE_SIZE 1000
typedef PVRecord PVTable[20][PV_TABLE_SIZE]; 

void zero_table(PVTable table);
ScoredMove get_best_move_multithreaded(Board* B, int depth, int alpha, int beta, int layers, PVTable parent);
ScoredMove get_best_move(Board* B, int depth, int alpha, int beta, bool stable, bool null_move_okay, int verbose, PVTable parent);
void print_pv(Board* B, int depth, int verbose, PVTable table); 

#endif//SEARCH_H
