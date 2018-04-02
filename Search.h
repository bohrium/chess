#ifndef SEARCH_H
#define SEARCH_H

#include "Board.h"
#include <iostream>

auto const max_accumulator = [](float a, float b) {return a<b ? b : a;};
auto const min_accumulator = [](float a, float b) {return a>b ? b : a;};

float evaluate_at_depth(Board* B, int nb_plies)
{
    if (nb_plies==0) { return evaluate(B); }
    MoveList ML;  
    generate_moves(B, &ML);

    auto const accumulator = B->next_to_move==Color::white ? max_accumulator : min_accumulator; 
    float score = B->next_to_move==Color::white ? -10000.0 : +10000.0; 
    for (int l=0; l!=ML.length; ++l) {
        Move m = ML.moves[l];
        if (m.taken.species == Species::king) { return B->next_to_move==Color::white ? +10000.0 : -10000.0;}
        apply_move(B, m);
        float child = evaluate_at_depth(B, nb_plies-1);
        score = accumulator(child, score);
        undo_move(B, m);
    }
    return score;
}

float alpha_beta(Board* B, int nb_plies, float alpha, float beta)
{
    if (nb_plies==0) { return evaluate(B); }
    MoveList ML;  
    generate_moves(B, &ML);

    bool is_white = B->next_to_move==Color::white;

    auto const accumulator = is_white ? max_accumulator : min_accumulator; 
    float score = is_white ? -10000.0 : +10000.0; 
    for (int l=0; l!=ML.length; ++l) {
        Move m = ML.moves[l];
        if (m.taken.species == Species::king) { return B->next_to_move==Color::white ? +10000.0 : -10000.0;}
        apply_move(B, m);
        float child = evaluate_at_depth(B, nb_plies-1);
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
