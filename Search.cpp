#include "Search.h"
#include "Board.h"
#include <iostream>
#include <algorithm>

#define QUIESCE_DEPTH 2
#define STABLE_ORDER_DEPTH 8

#define MIN(X,Y) (((X)<(Y))?(X):(Y))
#define MAX(X,Y) (((X)>(Y))?(X):(Y))
#define SUM(X,Y) ((X)+(Y))

#define NULL_THRESHOLD  5 // centipawns
#define MIN_FILTER_DEPTH 3
#define LATE_MOVE_REDUCTION 2
#define NULL_MOVE_REDUCTION 2

int alpha_beta_inner(Board* B, int nb_plies, int alpha, int beta, bool stable, bool null_move_okay);

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

// ensures first k elements of ML coincide with top k moves as determined by
// search with nb_plies many plies, and that these elements are sorted.  our
// routine avoids computing all scores exactly via alpha-beta style pruning:
// we expect special gains for small k
void order_moves(Board* B, MoveList* ML, int nb_plies, int k)
{
    /* TODO: check best-of-k logic! */

    if (4 <= nb_plies) {
        order_moves(B, ML, 2, 1000);
    }

    int shallow_scores[MAX_NB_MOVES]; 
    int sorted_indices[MAX_NB_MOVES];
    bool stable = (STABLE_ORDER_DEPTH <= nb_plies); 

    bool is_white = (B->next_to_move==Color::white);

    // worst_of_best_of_k 
    int wobok = is_white ? +KING_POINTS : -KING_POINTS;

    for (int m=0; m!=ML->length; ++m) {

        /* less than king value to avoid king trade */
        int alpha_ = is_white ? MAX(-KING_POINTS/2, wobok) : -KING_POINTS/2;
        int beta_  = is_white ? +KING_POINTS/2             : MIN(+KING_POINTS/2, wobok);
        apply_move(B, ML->moves[m]);
        int score = m==0 ?
            alpha_beta_inner(B, nb_plies, -KING_POINTS/2, +KING_POINTS/2, stable, true) :
            alpha_beta_inner(B, nb_plies, alpha_, beta_, stable, true);
        undo_move(B, ML->moves[m]);

        if (m < k) {
            sorted_indices[m] = m;
            shallow_scores[m] = score; 
            wobok = is_white ? MIN(score, wobok) : MAX(score, wobok);
        } else if (is_white && wobok<score || !is_white && score<wobok) {
            // update to make sure list of best-k moves are maintained
            int new_wobok = is_white ? +KING_POINTS : -KING_POINTS;
            bool replaced = false;
            for (int j=0; j!=k; ++j) { /* by control flow, we may assume k<=m */
                new_wobok = is_white ? MIN(shallow_scores[sorted_indices[j]], new_wobok) : MAX(shallow_scores[sorted_indices[j]], new_wobok);
                if (!replaced && shallow_scores[sorted_indices[j]]==wobok) {
                    //shallow_scores[j] = score; 
                    sorted_indices[j] = m; /* TODO: put move here!! */
                    replaced = true;
                }
            }
            wobok = new_wobok;
        }
    }
    auto scorer = [shallow_scores, is_white](int a, int b) {
        return is_white ?
            (shallow_scores[a]) > (shallow_scores[b]) :/* white wants bigger scores on left */
            (shallow_scores[a]) < (shallow_scores[b]); /* black wants smaller scores on left */
    };
    std::sort(sorted_indices, sorted_indices+MIN(k,ML->length), scorer); 
    Move sorted_moves[MAX_NB_MOVES]; 
    for (int m=0; m!=MIN(k,ML->length); ++m) {
        sorted_moves[m] = ML->moves[sorted_indices[m]];
    }
    for (int m=0; m!=MIN(k,ML->length); ++m) {
        ML->moves[m] = sorted_moves[m];
    }
} 

typedef struct ABRecord {
    unsigned int hash;
    int alpha;
    int beta;
    int score;
} ABRecord;
#define AB_TABLE_SIZE 10000
ABRecord ab_table[15][AB_TABLE_SIZE]; /* todo: zero out */

int alpha_beta(Board* B, int nb_plies, int alpha, int beta)
{
    return alpha_beta_inner(B, nb_plies, alpha, beta, true, true);
}

                             /*    0   1   2   3   4   5   6   7   8   9  10  11  12 */
const int branching_factors[] = { -1, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30}; 
const int ordering_depths[]   = { -1,  0,  0,  1,  2,  3,  3,  3,  3,  3,  3,  3,  3}; 

int alpha_beta_inner(Board* B, int nb_plies, int alpha, int beta, bool stable, bool null_move_okay)
{
    /* BASE CASE */
    if (nb_plies<=0) {
        if (stable) { return stable_eval(B, QUIESCE_DEPTH); }
        else        { return evaluate(B);                  }
    }

    int orig_alpha = alpha;
    int orig_beta = beta;

    /* HASH READ */
    if (stable) {
        ABRecord rec = ab_table[nb_plies][B->hash % AB_TABLE_SIZE];
        if (rec.hash == B->hash) {
          if ((rec.score > rec.alpha || rec.alpha <= alpha) &&
              (rec.score < rec.beta  || beta <= rec.beta)) {
            return rec.score; 
          }
        };
    }

    /* GENERATE MOVES */
    MoveList ML;  
    generate_moves(B, &ML);
    order_moves(B, &ML, ordering_depths[nb_plies], branching_factors[nb_plies]);

    /* MAIN LOOP */
    bool is_white = B->next_to_move==Color::white;
    int score = is_white ? -KING_POINTS : +KING_POINTS; 
    int nb_candidates = MIN(ML.length, branching_factors[nb_plies]);
    int nb_triumphs = 0; 

    /* null move reduction */
    if (MIN_FILTER_DEPTH<=nb_plies && null_move_okay && 
            (is_white && evaluate(B)>beta || !is_white && evaluate(B)<alpha)){
        apply_null(B);
        int alpha_hi = is_white ? beta-1 : alpha  ; 
        int beta_hi  = is_white ? beta   : alpha+1; 
        int child = alpha_beta_inner(B, nb_plies-1-NULL_MOVE_REDUCTION, alpha_hi, beta_hi, stable, false);
        if (( is_white && beta <=child) ||
            (!is_white && child<=alpha)) {
            score = child; 
            undo_null(B);
            goto END;
        }
        undo_null(B);
    }

    for (int l=0; l!=nb_candidates; ++l) {
        Move m = ML.moves[l];
        if (m.taken.species == Species::king) { return is_white ? +KING_POINTS : -KING_POINTS;}

        bool skip = false; 

        /* scout / late move reduction */
        if ((MIN_FILTER_DEPTH <=nb_plies && 1<=l) && 
            (NULL_THRESHOLD<=beta-alpha || (nb_triumphs==0 && !is_capture(m)))) {
            apply_move(B, m);
            int alpha_lo = is_white ? alpha : beta-1; 
            int beta_lo = is_white ? alpha+1 : beta; 
            int depth = nb_plies - 1 - (
                (nb_triumphs==0 && is_capture(m)) ? LATE_MOVE_REDUCTION : 0
            );
            int child = alpha_beta_inner(B, depth, alpha_lo, beta_lo, stable, true);
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
            int child = alpha_beta_inner(B, nb_plies-1, alpha, beta, stable, true);
            score = is_white ? MAX(child, score) : MIN(child, score);
            if (child == (is_white ? MAX(child, alpha) : MIN(child, beta))) {
                nb_triumphs += 1;
            }
            undo_move(B, m);
        }

        /* alpha-beta updates and cutoffs */
        if (is_white) { if (score >= beta ) break; alpha = MAX(alpha, score); } 
        else          { if (score <= alpha) break; beta  = MIN(beta , score); } 
    }

END:

    /* HASH WRITE */
    if (stable) {
        ABRecord* rec = &(ab_table[nb_plies][B->hash % AB_TABLE_SIZE]);
        rec->hash = B->hash;
        rec->alpha = orig_alpha;
        rec->beta = orig_beta;
        rec->score = score;
    }
    return score;
}

typedef struct PVRecord {
    unsigned int hash;
    int alpha;
    int beta;
    ScoredMove sm;
} PVRecord;
#define PV_TABLE_SIZE 10000
PVRecord pv_table[15][PV_TABLE_SIZE]; /* todo: zero out */

void print_pv(Board* B, int nb_plies, int verbose)
{
    PVRecord pvr = pv_table[nb_plies][(B->hash)%PV_TABLE_SIZE];
    print_move(B, pvr.sm.m);
    apply_move(B, pvr.sm.m);
    if (verbose) { print_pv(B, nb_plies-1, verbose-1); } 
    undo_move(B, pvr.sm.m);
}

ScoredMove get_best_move(Board* B, int nb_plies, int alpha, int beta, int verbose, bool null_move_okay)
{
    PVRecord pvr = pv_table[nb_plies][(B->hash)%PV_TABLE_SIZE];
    if (pvr.hash == B->hash) {
        if (pvr.alpha<=alpha && beta<=pvr.beta) { // record's window contains ours 
            //std::cout << "!\n";
            return pvr.sm;
        }
    }

    int orig_alpha = alpha;
    int orig_beta = beta;

    MoveList ML;  
    generate_moves(B, &ML);
    // TODO: what if due to recursion, inputted nb_plies is non-positive?!
    order_moves(B, &ML, ordering_depths[nb_plies], branching_factors[nb_plies]);

    bool is_white = B->next_to_move==Color::white;
    int score = is_white ? -KING_POINTS : +KING_POINTS; 
    Move best_move;
    int nb_candidates = MIN(ML.length, branching_factors[nb_plies]);
    int nb_triumphs = 0; 

    /* null move reduction */
    if (MIN_FILTER_DEPTH<=nb_plies && null_move_okay && 
            (is_white && evaluate(B)>beta || !is_white && evaluate(B)<alpha)){
        apply_null(B);
        int alpha_hi = is_white ? beta-1 : alpha  ; 
        int beta_hi  = is_white ? beta   : alpha+1; 
        ScoredMove child = get_best_move(B, nb_plies-1-NULL_MOVE_REDUCTION, alpha_hi, beta_hi, verbose-1, false);
        if (( is_white && beta <=child.score) ||
            (!is_white && child.score<=alpha)) {
            score = child.score;
            best_move = child.m;
            undo_null(B);
            goto END;
        }
        undo_null(B);
    }

    for (int l=0; l!=nb_candidates; ++l) {
        Move m = ML.moves[l];
        if (m.taken.species == Species::king) { 
            best_move = m;
            score = is_white ? +KING_POINTS : -KING_POINTS;
            break;
        }

        {
            std::cout << "  ";
            for (int t=0; t!=4-verbose; ++t) {
                std::cout << "\033[6C";
            }
            print_move(B, m);
            std::cout << "\033[100D"; //<< std::flush;
        }

        bool skip = false; 

        /* scout / late move reduction */
        if ((MIN_FILTER_DEPTH<=nb_plies && 1<=l) && 
            (NULL_THRESHOLD<=beta-alpha || (nb_triumphs==0 && !is_capture(m)))) {
            apply_move(B, m);
            int alpha_ = is_white ? alpha : beta-1; 
            int beta_  = is_white ? alpha+1 : beta; 
            int depth = nb_plies - 1 - (
                (nb_triumphs==0 && is_capture(m)) ? LATE_MOVE_REDUCTION : 0
            );
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
                                     get_best_move(B, nb_plies-1, alpha, beta, verbose-1, true).score;
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

END:

    pv_table[nb_plies][(B->hash)%PV_TABLE_SIZE] = {B->hash, orig_alpha, orig_beta, {best_move, score}};
    return {best_move, score};
} 



        //int depth = (
        //    1==branching_factors[nb_plies] ? nb_plies-1 :
        //    l < branching_factors[nb_plies] ? nb_plies-2 : nb_plies-3);  

