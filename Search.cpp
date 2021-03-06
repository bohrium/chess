#include "Search.h"
#include "Board.h"
#include <iostream>
#include <algorithm>

auto const max_accumulator = [](float a, float b) {return a<b ? b : a;};
auto const min_accumulator = [](float a, float b) {return a>b ? b : a;};

void order_moves(Board* B, MoveList* ML, int nb_plies)
{
    float shallow_scores[MAX_NB_MOVES]; 
    int sorted_indices[MAX_NB_MOVES];
    for (int m=0; m!=ML->length; ++m) {
        apply_move(B, ML->moves[m]);
        shallow_scores[m] = alpha_beta(B, nb_plies, -500.0, +500.0); /* less than king value to avoid king trade */
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
    if (4<=nb_plies) {
        order_moves(B, &ML, nb_plies/4);
    }

    bool is_white = B->next_to_move==Color::white;

    auto const accumulator = is_white ? max_accumulator : min_accumulator; 
    float score = is_white ? -1000.0 : +1000.0; 
    for (int l=0; l!=ML.length; ++l) {
        Move m = ML.moves[l];
        if (m.taken.species == Species::king) { return is_white ? +1000.0 : -1000.0;}
        apply_move(B, m);
        float child = alpha_beta(B, nb_plies-1, alpha, beta);
        score = accumulator(child, score);
        undo_move(B, m);

        if (is_white) {
            alpha = accumulator(alpha, score); 
        } else {
            beta = accumulator(beta, score); 
        } 

        if (! (alpha <= beta)) { /* cutoff! */
            break;
        }
    }
    return score;
}

Move get_best_move(Board* B, int nb_plies)
{
    MoveList ML;  
    generate_moves(B, &ML);
    if (4<=nb_plies) {
        order_moves(B, &ML, nb_plies/4);
    }
    bool is_white = B->next_to_move==Color::white;

    float alpha=-500.0, beta=+500.0;

    auto const accumulator = is_white ? max_accumulator : min_accumulator; 
    float score = is_white ? -1000.0 : +1000.0; 
    Move best_move;
    for (int l=0; l!=ML.length; ++l) {
        Move m = ML.moves[l];
        print_move(B, m);
        std::cout << "\033[6D" << std::flush;
        if (m.taken.species == Species::king) { return m; }
        apply_move(B, m);
        float child = alpha_beta(B, nb_plies-1, alpha, beta);
        if (score != accumulator(child, score)) {
            score = accumulator(child, score);
            best_move = m;
        }
        undo_move(B, m);

        if (is_white) {
            alpha = accumulator(alpha, score); 
        } else {
            beta = accumulator(beta, score); 
        } 
        /* cutoff will never happen at top level (except in king-taking case) */
    }
    return best_move;
} 



