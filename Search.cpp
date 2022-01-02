#include "Search.h"
#include "Board.h"
#include <iostream>
#include <algorithm>

#define MIN(X,Y) (((X)<(Y))?(X):(Y))
#define MAX(X,Y) (((X)>(Y))?(X):(Y))

/*TODO: get rid of / simplify lambda usage*/
auto const max_accumulator = [](int a, int b) {return a<b ? b : a;};
auto const min_accumulator = [](int a, int b) {return a>b ? b : a;};

Move shallow_greedy_move(Board* B)
{
    /* TODO: check for empty move list?? */
    MoveList ML;  
    generate_moves(B, &ML);
    int move_acc = 0;

    int sign = B->next_to_move == Color::white ? +1 : -1; 

    int score_acc = -KING_POINTS/2; 
    for (int m=0; m!=ML.length; ++m) {
        apply_move(B, ML.moves[m]);
        int score = sign * evaluate(B);
        undo_move(B, ML.moves[m]);
        score_acc = MAX(score_acc, score);
        if (score_acc == score) { move_acc = m; }
    }
    //score_acc *= sign;
    return ML.moves[move_acc];
}

int stable_eval(Board* B, int max_plies)
{
    /* TODO: check for taken king at very beginning and during loop */
    /* TODO: implement alpha beta style cutoffs */

    if (max_plies==0) { return evaluate(B); }
    Move m = shallow_greedy_move(B);
    if (!is_capture(m)) { return evaluate(B); }

    int pass = evaluate(B);
    apply_move(B, m);
    int go = stable_eval(B, max_plies-1); 
    undo_move(B, m);
    return B->next_to_move==Color::white ? MAX(pass, go) : MIN(pass, go);
}

void order_moves(Board* B, MoveList* ML, int nb_plies)
{
    int shallow_scores[MAX_NB_MOVES]; 
    int sorted_indices[MAX_NB_MOVES];
    for (int m=0; m!=ML->length; ++m) {
        apply_move(B, ML->moves[m]);
        shallow_scores[m] = alpha_beta(B, nb_plies, -KING_POINTS/2, +KING_POINTS/2); /* less than king value to avoid king trade */
        undo_move(B, ML->moves[m]);
        sorted_indices[m] = m;
    }
    auto scorer = [shallow_scores, B](int a, int b) {
        return B->next_to_move==Color::white ?
            (shallow_scores[a]) > (shallow_scores[b]) :/* white wants bigger scores on left */
            (shallow_scores[a]) < (shallow_scores[b]); /* black wants smaller scores on left */
    };
    std::sort(sorted_indices, sorted_indices+ML->length, scorer); 
    Move sorted_moves[MAX_NB_MOVES]; 
    for (int m=0; m!=ML->length; ++m) {
        sorted_moves[m] = ML->moves[sorted_indices[m]];
    }
    for (int m=0; m!=ML->length; ++m) {
        ML->moves[m] = sorted_moves[m];
    }
} 

typedef struct ABRecord {
    unsigned int hash;
    int alpha;
    int beta;
    int score;
} ABRecord;
#define AB_TABLE_SIZE 100000
#define AB_TABLE_DEPTH 5
ABRecord ab_table[AB_TABLE_SIZE]; /* todo: zero out */

int alpha_beta(Board* B, int nb_plies, int alpha, int beta)
{
    //if (nb_plies==0) { return evaluate(B); }
    if (nb_plies==0) { return stable_eval(B, 4); }

    if (nb_plies == AB_TABLE_DEPTH) {
      ABRecord rec = ab_table[B->hash % AB_TABLE_SIZE];
      if (rec.hash == B->hash) {
        std::cout << "moo!" << std::endl;
        if ((rec.score > rec.alpha || rec.alpha <= alpha) &&
            (rec.score < rec.beta  || beta <= rec.beta)) {
          std::cout << "hit!" << std::endl;
          return rec.score; 
        }
      };
    }

    MoveList ML;  
    generate_moves(B, &ML);
    if (2<=nb_plies) {
        order_moves(B, &ML, nb_plies-2);
    }

    bool is_white = B->next_to_move==Color::white;

    auto const accumulator = is_white ? max_accumulator : min_accumulator; 
    int score = is_white ? -KING_POINTS : +KING_POINTS; 

                                 /*  0   1  2    3    4   5   6   7  8   */
    const int branching_factors[] = {-1, 64, 64, 16, 16, 8, 8,  4,  4}; 
    //const int branching_factors[] = {-1, 64, 64, 64, 64, 64, 64, 64, 64}; 
    int nb_candidates = MIN(ML.length, branching_factors[nb_plies]);
    //int nb_candidates = ML.length;

    for (int l=0; l!=nb_candidates; ++l) {
        Move m = ML.moves[l];
        if (m.taken.species == Species::king) { return is_white ? +KING_POINTS : -KING_POINTS;}

        /* stack push/pop */
        {
            apply_move(B, m);
            int child = alpha_beta(B, nb_plies-1, alpha, beta);
            score = accumulator(child, score);
            undo_move(B, m);
        }

        if (is_white) {
            if (score >= beta) break;
            alpha = accumulator(alpha, score); 
        } else {
            if (score <= alpha) break;
            beta = accumulator(beta, score); 
        } 

        ///* cutoff! */
        //if (! (alpha <= beta)) { break; }
    }

    if (nb_plies == AB_TABLE_DEPTH) {
      ABRecord* rec = &(ab_table[B->hash % AB_TABLE_SIZE]);
      rec->hash = B->hash;
      rec->alpha = alpha;
      rec->beta = beta;
      rec->score = score;
    }
    return score;
}

Move get_best_move(Board* B, int nb_plies)
{
    MoveList ML;  
    generate_moves(B, &ML);
    if (2<=nb_plies) {
        order_moves(B, &ML, nb_plies-2);
    }
    bool is_white = B->next_to_move==Color::white;

    int alpha=-KING_POINTS/2, beta=+KING_POINTS/2;

    auto const accumulator = is_white ? max_accumulator : min_accumulator; 
    int score = is_white ? -KING_POINTS : +KING_POINTS; 
    Move best_move;

                                 /*  0   1  2    3    4   5   6   7  8   */
    const int branching_factors[] = {-1, 64, 64, 16, 16, 8, 8,  4,  4}; 
    //const int branching_factors[] = {-1, 64, 64, 64, 64, 64, 64, 64, 64}; 
    int nb_candidates = MIN(ML.length, branching_factors[nb_plies]);
    //int nb_candidates = ML.length;

    for (int l=0; l!=nb_candidates; ++l) {
        Move m = ML.moves[l];
        print_move(B, m);
        std::cout << "\033[6D" << std::flush;
        if (m.taken.species == Species::king) { return m; }
        apply_move(B, m);
        int child = alpha_beta(B, nb_plies-1, alpha, beta);
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



