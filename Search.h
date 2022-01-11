#ifndef SEARCH_H
#define SEARCH_H

#include "Board.h"

void zero_tables();
int alpha_beta(Board* B, int nb_plies, int alpha, int beta);
//void order_moves(Board* B, MoveList* ML);
void order_moves(Board* B, MoveList* ML, int nb_plies, int k);

struct ScoredMove {
    Move m;
    int score;
};
ScoredMove get_best_move(Board* B, int nb_plies, int alpha, int beta, int verbose, bool null_move_okay);
void print_pv(Board* B, int nb_plies, int verbose);

#endif//SEARCH_H
