#ifndef SEARCH_H
#define SEARCH_H

#include "Board.h"

int alpha_beta(Board* B, int nb_plies, int alpha, int beta);
void order_moves(Board* B, MoveList* ML);

struct ScoredMove {
    Move m;
    int score;
};
ScoredMove get_best_move(Board* B, int nb_plies, int alpha, int beta, int verbose);
void print_pv(Board* B, int nb_plies, int verbose);

#endif//SEARCH_H
