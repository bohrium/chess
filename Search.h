#ifndef SEARCH_H
#define SEARCH_H

#include "Board.h"

/* BASIC SEARCH PARAMETERS */

/* TODO: implement fracplies, especially in conjunction with CSR */
#define FRACPLIES_PER_PLY 2 // for fractional depth 

#define SCOUT_THRESH 5       // centipawns
#define QUIESCE_DEPTH (FRACPLIES_PER_PLY*4) // fracplies
#define STABLE_ORDER_DEPTH (FRACPLIES_PER_PLY*3) // fracplies

                             /*    0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18 */
const int branching_factors[] = { -1, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36}; // nb siblings
const int ordering_depths[]   = { -1,  0,  0,  1,  1,  2,  2,  3,  3,  4,  4,  5,  5,  6,  6,  7,  7,  8,  8}; // plies

void zero_tables();
int alpha_beta(Board* B, int nb_fracplies, int alpha, int beta);
//void order_moves(Board* B, MoveList* ML);
void order_moves(Board* B, MoveList* ML, int nb_fracplies, int k);

struct ScoredMove {
    Move m;
    int score;
};
ScoredMove get_best_move(Board* B, int nb_fracplies, int alpha, int beta, int verbose, bool null_move_okay);
void print_pv(Board* B, int nb_plies, int verbose);

#endif//SEARCH_H
