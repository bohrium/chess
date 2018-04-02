#ifndef SEARCH_H
#define SEARCH_H

#include "Board.h"
#include <iostream>
#include <algorithm>

auto const max_accumulator = [](float a, float b) {return a<b ? b : a;};
auto const min_accumulator = [](float a, float b) {return a>b ? b : a;};


float alpha_beta(Board* B, int nb_plies, float alpha, float beta);
void order_moves(Board* B, MoveList* ML);

void order_moves(Board* B, MoveList* ML, int nb_plies)
{
    float shallow_scores[MAX_NB_MOVES]; 
    int sorted_indices[MAX_NB_MOVES];
    for (int m=0; m!=ML->length; ++m) {
        apply_move(B, ML->moves[m]);
        shallow_scores[m] = alpha_beta(B, nb_plies, -10000.0, +10000.0);
        undo_move(B, ML->moves[m]);
        sorted_indices[m] = m;
    }
    auto scorer = B->next_to_move==Color::white
        ? [](int a, int b) { return a>b; } /* white wants bigger scores on left */
        : [](int a, int b) { return a<b; };/* black wants smaller scores on left */
    std::sort(sorted_indices, sorted_indices+ML->length, scorer); 
    Move sorted_moves[MAX_NB_MOVES]; 
    for (int m=0; m!=ML->length; ++m) {
        sorted_moves[m] = ML->moves[sorted_indices[m]];
    }
    for (int m=0; m!=ML->length; ++m) {
        ML->moves[m] = sorted_moves[m];
    }
} 

float alpha_beta(Board* B, int nb_plies, float alpha, float beta)
{
    if (nb_plies==0) { return evaluate(B); }
    MoveList ML;  
    generate_moves(B, &ML);
    if (3<=nb_plies) {
        order_moves(B, &ML, nb_plies/3);
    }

    bool is_white = B->next_to_move==Color::white;

    auto const accumulator = is_white ? max_accumulator : min_accumulator; 
    float score = is_white ? -10000.0 : +10000.0; 
    for (int l=0; l!=ML.length; ++l) {
        Move m = ML.moves[l];
        if (m.taken.species == Species::king) { return B->next_to_move==Color::white ? +10000.0 : -10000.0;}
        apply_move(B, m);
        float child = alpha_beta(B, nb_plies-1, alpha, beta);
        score = accumulator(child, score);
        undo_move(B, m);

        if (is_white) {
            alpha = accumulator(alpha, score); 
        } else {
            beta = accumulator(beta, score); 
        } 

        if (! (alpha <= beta)) {
            break;
        }
    }
    return score;
}

Move get_best_move(Board* B, int nb_plies)
{
    MoveList ML;  
    generate_moves(B, &ML);

    auto const accumulator = B->next_to_move==Color::white ? max_accumulator : min_accumulator; 
    float score = B->next_to_move==Color::white ? -10000.0 : +10000.0; 
    Move best_move;
    for (int l=0; l!=ML.length; ++l) {
        std::cout << "." << std::flush;
        Move m = ML.moves[l];
        if (m.taken.species == Species::king) { return m; }
        apply_move(B, m);
        //float child = evaluate_at_depth(B, nb_plies);
        float child = alpha_beta(B, nb_plies, -10000.0, +10000.0);
        if (score != accumulator(child, score)) {
            score = accumulator(child, score);
            best_move = m;
        }
        undo_move(B, m);
    }
    return best_move;
}  

#endif//SEARCH_H
