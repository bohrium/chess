#ifndef SEARCH_H
#define SEARCH_H

#include "Board.h"

float alpha_beta(Board* B, int nb_plies, float alpha, float beta);
void order_moves(Board* B, MoveList* ML);
Move get_best_move(Board* B, int nb_plies);

#endif//SEARCH_H
