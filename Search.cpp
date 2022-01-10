#include "Search.h"
#include "Board.h"
#include <iostream>
#include <algorithm>

#define STABLE_DEPTH 2

#define MIN(X,Y) (((X)<(Y))?(X):(Y))
#define MAX(X,Y) (((X)>(Y))?(X):(Y))

int alpha_beta_inner(Board* B, int nb_plies, int alpha, int beta, bool stable);

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
        shallow_scores[m] = alpha_beta_inner(B, nb_plies, -KING_POINTS/2, +KING_POINTS/2, false); /* less than king value to avoid king trade */
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
    return alpha_beta_inner(B, nb_plies, alpha, beta, true);
}

                             /*    0   1   2   3   4   5   6   7   8   9  10 */
const int branching_factors[] = { -1, 64, 64, 64, 64, 16, 16,  4,  4,  4,  4}; 
const int ordering_depths[]   = { -1,  0,  0,  1,  2,  3,  4,  4,  4,  4,  4}; 

int alpha_beta_inner(Board* B, int nb_plies, int alpha, int beta, bool stable)
{
    if (nb_plies<=0) {
        if (stable) { return stable_eval(B, STABLE_DEPTH); }
        else        { return evaluate(B);                  }
    }

    //if (nb_plies == AB_TABLE_DEPTH) {
    //  ABRecord rec = ab_table[B->hash % AB_TABLE_SIZE];
    //  if (rec.hash == B->hash) {
    //    std::cout << "moo!" << std::endl;
    //    if ((rec.score > rec.alpha || rec.alpha <= alpha) &&
    //        (rec.score < rec.beta  || beta <= rec.beta)) {
    //      std::cout << "hit!" << std::endl;
    //      return rec.score; 
    //    }
    //  };
    //}

    MoveList ML;  
    generate_moves(B, &ML);
    //if (2<=nb_plies) {
      order_moves(B, &ML, ordering_depths[nb_plies]);
    //}

    bool is_white = B->next_to_move==Color::white;

    int score = is_white ? -KING_POINTS : +KING_POINTS; 

    int nb_candidates = MIN(ML.length, branching_factors[nb_plies]);

    int nb_triumphs = 0; 

    for (int l=0; l!=nb_candidates; ++l) {
        Move m = ML.moves[l];
        if (m.taken.species == Species::king) { return is_white ? +KING_POINTS : -KING_POINTS;}

        bool skip = false; 

        /* scout / late move reduction */
        if (4<=nb_plies && 1<=l) {
            apply_move(B, m);
            int alpha_ = is_white ? alpha : beta-1; 
            int beta_  = is_white ? alpha+1 : beta; 
            int depth = nb_triumphs==0 ? nb_plies-3 : nb_plies-1; 
            int child = alpha_beta_inner(B, depth, alpha_, beta_, stable);
            if (( is_white && child<=alpha) ||
                (!is_white && beta <=child)) {
                skip = true;
            }
            undo_move(B, m);
        }
        if (skip) { continue; }

        /* full search */
        {
            apply_move(B, m);
            int child = alpha_beta_inner(B, nb_plies-1, alpha, beta, stable);
            score = is_white ? MAX(child, score) : MIN(child, score);
            if (child == (is_white ? MAX(child, alpha) : MIN(child, beta))) {
                nb_triumphs += 1;
            }
            undo_move(B, m);
        }

        if (is_white) { if (score >= beta ) break; alpha = MAX(alpha, score); } 
        else          { if (score <= alpha) break; beta  = MIN(beta , score); } 
    }

    //if (nb_plies == AB_TABLE_DEPTH) {
    //  ABRecord* rec = &(ab_table[B->hash % AB_TABLE_SIZE]);
    //  rec->hash = B->hash;
    //  rec->alpha = alpha;
    //  rec->beta = beta;
    //  rec->score = score;
    //}
    return score;
}

ScoredMove get_best_move(Board* B, int nb_plies, int alpha, int beta, int verbose)
{
    MoveList ML;  
    generate_moves(B, &ML);

    //if (2<=nb_plies) {
      order_moves(B, &ML, ordering_depths[nb_plies]);
    //}

    bool is_white = B->next_to_move==Color::white;

    int score = is_white ? -KING_POINTS : +KING_POINTS; 
    Move best_move;

    int nb_candidates = MIN(ML.length, branching_factors[nb_plies]);

    int nb_triumphs = 0; 

    for (int l=0; l!=nb_candidates; ++l) {
        Move m = ML.moves[l];
        if (m.taken.species == Species::king) { return {m, is_white ? +KING_POINTS : -KING_POINTS}; }

        {
            for (int t=0; t!=3-verbose; ++t) {
                std::cout << "\033[6C";
            }
            print_move(B, m);
            std::cout << "\033[100D" << std::flush;
        }

        bool skip = false; 

        /* scout / late move reduction */
        if (4<=nb_plies && 1<=l) {
            apply_move(B, m);
            int alpha_ = is_white ? alpha : beta-1; 
            int beta_  = is_white ? alpha+1 : beta; 
            int depth = nb_triumphs==0 ? nb_plies-3 : nb_plies-1; 
            int child = alpha_beta(B, depth, alpha_, beta_);
            if (( is_white && child<=alpha) ||
                (!is_white && beta <=child)) {
                skip = true;
            }
            undo_move(B, m);
        }
        if (skip) { continue; }

        /* full search */
        {
            apply_move(B, m);
            int child = verbose==0 ? alpha_beta(B, nb_plies-1, alpha, beta) :
                                     get_best_move(B, nb_plies-1, alpha, beta, verbose-1).score;
            int new_score = is_white ? MAX(child, score) : MIN(child, score);
            if (child == (is_white ? MAX(child, alpha) : MIN(child, beta))) {
                nb_triumphs += 1;
            }
            if (new_score != score) {
                score = new_score; 
                best_move = m;
            }
            undo_move(B, m);
        }

        /* cutoff will never happen at top level (except in king-taking case) */
        if (is_white) { if (score >= beta ) break; alpha = MAX(alpha, score); } 
        else          { if (score <= alpha) break; beta  = MIN(beta , score); } 
    }
    return {best_move, score};
} 



        //int depth = (
        //    1==branching_factors[nb_plies] ? nb_plies-1 :
        //    l < branching_factors[nb_plies] ? nb_plies-2 : nb_plies-3);  

