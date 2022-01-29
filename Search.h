#ifndef SEARCH_H
#define SEARCH_H

#include "Board.h"

#define MAX_VERBOSE 4

/* BASIC SEARCH PARAMETERS */

#define SCOUT_THRESH       3 // centipawns
#define QUIESCE_DEPTH      3 // plies    
#define STABLE_ORDER_DEPTH 2 // plies

                             /*    0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21 */
const int branching_factors[] = { -1, -1, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36}; // nb siblings
const int ordering_depths[]   = { -1, -1,  0,  1,  1,  1,  2,  2,  2,  3,  3,  3,  4,  4,  4,  5,  5,  5,  6,  6,  6,  7}; // plies

/* REDUCTION PARAMETERS */

#define RED_DEPTH_SOFT_FLOOR 6 // plies 
#define RED_DEPTH_HARD_FLOOR 4 // plies 

#define ALLOW_NMR 1
//#define ALLOW_AR  1
#define ALLOW_LMR 1
#define ALLOW_CSR 1

#define NMR_THRESH 50  // centipawns
#define NMR_AMOUNT 0.9 // (fracplies

#define AR_THRESH  8  // nb siblings
#define AR_AMOUNT 0.9 // fracplies 

#define LMR_THRESH 2  // nb siblings
#define LMR_AMOUNT 0.8 // fracplies 

#define CSR_THRESH 50 // centipawns
#define CSR_AMOUNT 0.8 // fracplies

/* SEARCH METHODS */

struct ScoredMove {
    Move m;
    int score;
    int height;
};

typedef struct PVRecord {
    unsigned int hash;
    int alpha;
    int beta;
    ScoredMove sm;
} PVRecord;
#define PV_TABLE_SIZE 10000
typedef PVRecord PVTable[22][PV_TABLE_SIZE]; 

void zero_table(PVTable table);
void update_table(PVTable parent, PVTable update);
void print_pv(Board* B, int depth, int verbose, PVTable parent);

Move shallow_greedy_move(Board* B);
void order_moves(Board* B, MoveList* ML, int depth, int k, PVTable table);
int stable_eval(Board* B, int max_plies, int alpha, int beta);

ScoredMove get_best_move(Board* B, int depth, int alpha, int beta, bool stable, bool null_move_okay, int verbose, PVTable parent);
ScoredMove get_best_move_multithreaded(Board* B, int depth, int alpha, int beta, int layers, PVTable parent);

#endif//SEARCH_H
