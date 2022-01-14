#include "Search.h"
#include "Board.h"
#include <iostream>
#include <algorithm>

#define MIN(X,Y) (((X)<(Y))?(X):(Y))
#define MAX(X,Y) (((X)>(Y))?(X):(Y))
#define SUM(X,Y) ((X)+(Y))

void zero_tables();

int alpha_beta_inner(Board* B, int depth, int alpha, int beta, bool stable, bool null_move_okay);

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
    score_acc *= sign; /* useless but goes with computation spirit */
    return ML.moves[move_acc]; 
}

int stable_eval(Board* B, int max_plies, int alpha, int beta)
{
    /* TODO: check for taken king at very beginning and during loop */
    /* TODO: implement alpha beta style cutoffs */

    if (max_plies<=0) { return evaluate(B); }
    bool is_white = B->next_to_move==Color::white;
    int pass = evaluate(B);
    if (is_white && pass>=beta || !is_white && pass<alpha) { return pass; } 

    Move m = shallow_greedy_move(B);
    if (!is_capture(m)) { return evaluate(B); }

    apply_move(B, m);
    int go = stable_eval(B, max_plies-1, alpha, beta); /* TODO: update alpha beta */ 
    undo_move(B, m);
    return is_white ? MAX(pass, go) : MIN(pass, go);
}

// ensures first k elements of ML coincide with top k moves as determined by
// search with depth many plies, and that these elements are sorted.  our
// routine avoids computing all scores exactly via alpha-beta style pruning:
// we expect special gains for small k

#define FANCY_ORDER 0 
void order_moves(Board* B, MoveList* ML, int depth, int k)
{
    /* TODO: check best-of-k logic! */
    k = MIN(k, MAX_NB_MOVES);

#if FANCY_ORDER
    if (3 <= depth) {
        order_moves(B, ML, 1, MAX_NB_MOVES);
    }
#endif

    int shallow_scores[MAX_NB_MOVES]; 
    int sorted_indices[MAX_NB_MOVES];
    bool stable = (STABLE_ORDER_DEPTH <= depth); 

    bool is_white = (B->next_to_move==Color::white);

#if FANCY_ORDER
    if (2 <= depth) {
        // worst_of_best_of_k 
        int wobok = is_white ? +KING_POINTS : -KING_POINTS;

        for (int m=0; m!=ML->length; ++m) {
            /* less than king value to avoid king trade */
            int alpha_ = is_white ? MAX(-KING_POINTS/2, wobok) : -KING_POINTS/2;
            int beta_  = is_white ? +KING_POINTS/2             : MIN(+KING_POINTS/2, wobok);
            apply_move(B, ML->moves[m]);
            int score = m==0 ?
                alpha_beta_inner(B, depth, -KING_POINTS/2, +KING_POINTS/2, stable, true) :
                alpha_beta_inner(B, depth, alpha_, beta_, stable, true);
            undo_move(B, ML->moves[m]);

            shallow_scores[m] = score; 
            if (m < k) {
                sorted_indices[m] = m;
                wobok = is_white ? MIN(score, wobok) : MAX(score, wobok);
            } else if (is_white && wobok<score || !is_white && score<wobok) {
                // update to make sure list of best-k moves are maintained
                int new_wobok = is_white ? +KING_POINTS : -KING_POINTS;
                bool replaced = false;
                for (int j=0; j!=k; ++j) { /* by control flow, we may assume k<=m */
                    if (!replaced && shallow_scores[sorted_indices[j]]==wobok) {
                        sorted_indices[j] = m; /* TODO: put move here!! */
                        replaced = true;
                    }
                    new_wobok = is_white ? MIN(shallow_scores[sorted_indices[j]], new_wobok) : MAX(shallow_scores[sorted_indices[j]], new_wobok);
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
    } else {
#else
    if (true) {
#endif
        for (int m=0; m!=ML->length; ++m) {
            apply_move(B, ML->moves[m]);
            int score = alpha_beta_inner(B, depth, -KING_POINTS/2, +KING_POINTS/2, stable, true);
            undo_move(B, ML->moves[m]);
            shallow_scores[m] = score; 
            sorted_indices[m] = m;
        }
        auto scorer = [shallow_scores, is_white](int a, int b) {
            return is_white ?
                (shallow_scores[a]) > (shallow_scores[b]) :/* white wants bigger scores on left */
                (shallow_scores[a]) < (shallow_scores[b]); /* black wants smaller scores on left */
        };
        std::sort(sorted_indices, sorted_indices+ML->length, scorer); 
    }

    Move sorted_moves[MAX_NB_MOVES]; 
    for (int m=0; m!=MIN(k,ML->length); ++m) {
        sorted_moves[m] = ML->moves[sorted_indices[m]];
    }
    for (int m=0; m!=MIN(k,ML->length); ++m) {
        ML->moves[m] = sorted_moves[m];
    }
    //
    ML->length = MIN(k,ML->length);
} 

typedef struct ABRecord {
    unsigned int hash;
    int alpha;
    int beta;
    int score;
} ABRecord;
#define AB_TABLE_SIZE 10000
ABRecord ab_table[20][AB_TABLE_SIZE]; /* todo: zero out */

int alpha_beta(Board* B, int depth, int alpha, int beta)
{
    return alpha_beta_inner(B, depth, alpha, beta, true, true);
}

int alpha_beta_inner(Board* B, const int depth, int alpha, int beta, bool stable, bool null_move_okay)
{
    /* BASE CASE */
    if (depth<=0) {
        if (stable) { return stable_eval(B, QUIESCE_DEPTH, alpha, beta); }
        else        { return evaluate(B);                                }
    }

    int orig_alpha = alpha;
    int orig_beta = beta;
    int reduction;

    /* HASH READ */
    if (stable) {
        ABRecord rec = ab_table[depth][B->hash % AB_TABLE_SIZE];
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
    order_moves(B, &ML, ordering_depths[depth], branching_factors[depth]);

    /* MAIN LOOP */
    bool is_white = B->next_to_move==Color::white;
    int score = is_white ? -KING_POINTS : +KING_POINTS; 
    int nb_candidates = MIN(ML.length, branching_factors[depth]);
    bool any_triumphant = false;  
    bool trigger_lmr = false;  
    bool trigger_csr = false;  

#if ALLOW_NMR
/**/{
/**/    /* null move reduction */
/**/    int pass = evaluate(B); /* TODO: replace by quiescent? */
/**/    if ((MIN_FILTER_DEPTH<=depth && null_move_okay) && 
/**/        (is_white && beta<pass || !is_white && pass<alpha)) {
/**/        apply_null(B);
/**/        int alpha_hi = is_white ? beta-1+NMR_THRESH : alpha  -NMR_THRESH; 
/**/        int beta_hi  = is_white ? beta  +NMR_THRESH : alpha+1-NMR_THRESH; 
/**/        reduction = NMR_AMOUNT;
/**/        int child = alpha_beta_inner(B, depth-1-reduction, alpha_hi, beta_hi, stable, false);
/**/        bool skip = false;
/**/        if (( is_white && beta +NMR_THRESH<=child) ||
/**/            (!is_white && child<=alpha-NMR_THRESH)) {
/**/            score = child; 
/**/            skip = true;
/**/        }
/**/        undo_null(B);
/**/        if (skip) { goto END; }
/**/    }
/**/}
#endif//ALLOW_NMR

#if ALLOW_CSR
    if (MIN_FILTER_DEPTH<=depth && 2<=ML.length) {
        /* TODO: think about alpha beta windows for CSR!*/
        /* TODO: use null window to test score_snd*/
        int score_fst, score_snd;
        {
            apply_move(B, ML.moves[0]);
            score_fst = alpha_beta_inner(B, ordering_depths[depth], alpha-CSR_AMOUNT, beta+CSR_AMOUNT, true, true);
            undo_move(B, ML.moves[0]);
        }
        {
            apply_move(B, ML.moves[1]);
            score_snd = alpha_beta_inner(B, ordering_depths[depth], alpha-CSR_AMOUNT, beta+CSR_AMOUNT, true, true);
            undo_move(B, ML.moves[1]);
        }
        if (( is_white && (score_snd + CSR_THRESH <= score_fst)) || 
            (!is_white && (score_fst <= score_snd - CSR_THRESH))) {
            trigger_csr = true;
        }
    } else if (1==ML.length) {
        trigger_csr = true;
    }
#endif//ALLOW_CSR

    for (int l=0; l!=nb_candidates; ++l) {
        reduction = 0;
#if ALLOW_CSR
        if (trigger_csr&&l==1 || !trigger_csr&&l==0) {
            reduction += CSR_AMOUNT;
        } 
#endif//ALLOW_CSR
#if ALLOW_AR
/**/    if (MIN_FILTER_DEPTH<=depth && AR_THRESH<=AR_THRESH) {
/**/        reduction += AR_AMOUNT;
/**/    }
#endif//ALLOW_AR

        Move m = ML.moves[l];
        if (m.taken.species == Species::king) { return is_white ? +KING_POINTS : -KING_POINTS;}

#if ALLOW_LMR
/**/    bool skip = false; 
/**/    /* scout / late move reduction */
/**/    if ((MIN_FILTER_DEPTH<=depth && 1<=l) && 
/**/        (SCOUT_THRESH<=beta-alpha || (trigger_lmr && !is_capture(m)))) {
/**/        apply_move(B, m);
/**/        int alpha_lo = is_white ? alpha : beta-1; 
/**/        int beta_lo = is_white ? alpha+1 : beta; 
/**/        int child_depth = depth-reduction;
/**/        if (trigger_lmr && !is_capture(m)) { child_depth -= LMR_AMOUNT; }
/**/        int child = alpha_beta_inner(B, child_depth-1, alpha_lo, beta_lo, stable, true);
/**/        if (( is_white && child<=alpha) ||
/**/            (!is_white && beta <=child)) {
/**/            skip = true;
/**/        }
/**/        undo_move(B, m);
/**/    }
/**/    if (skip) { continue; }
#endif//ALLOW_LMR

        /* full search */
        {
            apply_move(B, m);
            int child = alpha_beta_inner(B, depth-1-reduction, alpha, beta, stable, true);
            score = is_white ? MAX(child, score) : MIN(child, score);
            if (is_white&&alpha<=child || !is_white&&beta<=child) { any_triumphant = true; }
            if (l==LMR_THRESH && !any_triumphant) { trigger_lmr = true; }
            undo_move(B, m);
        }

        /* alpha-beta updates and cutoffs */
        if (is_white) { if (score >= beta ) break; alpha = MAX(alpha, score); } 
        else          { if (score <= alpha) break; beta  = MIN(beta , score); } 
    }

END:

    /* HASH WRITE */
    if (stable) {
        ABRecord* rec = &(ab_table[depth][B->hash % AB_TABLE_SIZE]);
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
PVRecord pv_table[20][PV_TABLE_SIZE]; /* todo: zero out */

void zero_tables()
{
    for (int d=0; d!=15; ++d) {
        for (int i=0; i!=AB_TABLE_SIZE; ++i) {
            ab_table[d][i].hash = 0;
        }
        for (int i=0; i!=PV_TABLE_SIZE; ++i) {
            pv_table[d][i].hash = 0;
        }
    }
}

void print_pv(Board* B, int depth, int verbose)
{
    PVRecord pvr = pv_table[depth][(B->hash)%PV_TABLE_SIZE];
    if (B->hash != pvr.hash) { return; }
    print_move(B, pvr.sm.m);
    apply_move(B, pvr.sm.m);
    if (verbose) { print_pv(B, depth-1, verbose-1); } 
    undo_move(B, pvr.sm.m);
}

ScoredMove get_best_move(Board* B, const int depth, int alpha, int beta, int verbose, bool null_move_okay)
{
    if (depth < 1) {
        return get_best_move(B, 1, alpha, beta, 0, null_move_okay);
    }

    /* fix several bugs with these three record keepers 
     * (merit of "const"!) --- could have caused segfault!
     */
    int reduction;
    const int orig_alpha = alpha;
    const int orig_beta = beta;

    PVRecord pvr = pv_table[depth][(B->hash)%PV_TABLE_SIZE];
    if (pvr.hash == B->hash) {
        if (pvr.alpha<=alpha && beta<=pvr.beta) { // record's window contains ours 
            return pvr.sm;
        }
    }

    MoveList ML;  
    generate_moves(B, &ML);
    order_moves(B, &ML, ordering_depths[depth], branching_factors[depth]);

    bool is_white = B->next_to_move==Color::white;
    int score = is_white ? -KING_POINTS : +KING_POINTS; 
    Move best_move;
    int nb_candidates = MIN(ML.length, branching_factors[depth]);
    bool any_triumphant = false;  
    bool trigger_lmr = false;  
    bool trigger_csr = false;  

#if ALLOW_NMR
/**/{
/**/    /* null move reduction */
/**/    int pass = evaluate(B); /* TODO: replace by quiescent? */
/**/    if ((MIN_FILTER_DEPTH<=depth && null_move_okay) && 
/**/        (is_white && beta<pass || !is_white && pass<alpha)) {
/**/        apply_null(B);
/**/        int alpha_hi = is_white ? beta-1+NMR_THRESH : alpha  -NMR_THRESH; 
/**/        int beta_hi  = is_white ? beta  +NMR_THRESH : alpha+1-NMR_THRESH; 
/**/        int reduction = NMR_AMOUNT;
/**/        ScoredMove child = get_best_move(B, depth-1-reduction, alpha_hi, beta_hi, verbose-1, false);
/**/        bool skip = false;
/**/        if (( is_white && beta +NMR_THRESH<=child.score) ||
/**/            (!is_white && child.score<=alpha-NMR_THRESH)) {
/**/            score = child.score; 
/**/            best_move = child.m; 
/**/            skip = true;
/**/        }
/**/        undo_null(B);
/**/        if (skip) { goto END; }
/**/    }
/**/}
#endif//ALLOW_NMR

#if ALLOW_CSR
    if (MIN_FILTER_DEPTH<=depth && 2<=ML.length) {
        /* TODO: think about alpha beta windows for CSR!*/
        /* TODO: use null window to test score_snd*/
        int score_fst, score_snd;
        {
            apply_move(B, ML.moves[0]);
            score_fst = alpha_beta_inner(B, ordering_depths[depth], alpha-CSR_AMOUNT, beta+CSR_AMOUNT, true, true);
            undo_move(B, ML.moves[0]);
        }
        {
            apply_move(B, ML.moves[1]);
            score_snd = alpha_beta_inner(B, ordering_depths[depth], alpha-CSR_AMOUNT, beta+CSR_AMOUNT, true, true);
            undo_move(B, ML.moves[1]);
        }
        if (( is_white && (score_snd + CSR_THRESH <= score_fst)) || 
            (!is_white && (score_fst <= score_snd - CSR_THRESH))) {
            trigger_csr = true;
        }
    } else if (1==ML.length) {
        trigger_csr = true;
    }
#endif//ALLOW_CSR

    for (int l=0; l!=nb_candidates; ++l) {
        reduction=0;
        /* since we later index using depth
         * ( pv_table[depth][(B->hash)%PV_TABLE_SIZE] = ... )
         * we need to keep track of a separate depth.
         * (merit of "const"!) --- could have caused segfault!
         */
#if ALLOW_CSR
        if (trigger_csr&&l==1 || !trigger_csr&&l==0) {
            reduction += CSR_AMOUNT;
        } 
#endif//ALLOW_CSR
#if ALLOW_AR
/**/    if (MIN_FILTER_DEPTH<=depth && AR_THRESH<=l) {
/**/        reduction += AR_AMOUNT;
/**/    }
#endif//ALLOW_AR

        Move m = ML.moves[l];
        if (m.taken.species == Species::king) { 
            best_move = m;
            score = is_white ? +KING_POINTS : -KING_POINTS;
            break;
        }

        if (verbose && 6<=depth) {
            std::cout << "  ";
            for (int t=0; t!=12-depth; ++t) {
                std::cout << "\033[6C";
            }
            print_move(B, m);
            std::cout << "         ";
            std::cout << "\033[200D" << std::flush;
        }

#if ALLOW_LMR
/**/    bool skip = false; 
/**/    /* scout / late move reduction */
/**/    if ((MIN_FILTER_DEPTH <=depth && 1<=l) && 
/**/        (SCOUT_THRESH<=beta-alpha || (trigger_lmr && !is_capture(m)))) {
/**/        apply_move(B, m);
/**/        int alpha_ = is_white ? alpha : beta-1; 
/**/        int beta_  = is_white ? alpha+1 : beta; 
/**/        int child_depth = depth-reduction;
/**/        if (trigger_lmr && !is_capture(m)) { child_depth -= LMR_AMOUNT; }
/**/        int child = alpha_beta(B, child_depth-1, alpha_, beta_);
/**/        if (( is_white && child<=alpha) ||
/**/            (!is_white && beta <=child)) {
/**/            skip = true;
/**/        }
/**/        undo_move(B, m);
/**/    }
/**/    if (skip) { continue; }
#endif//ALLOW_LMR

        /* full search */
        {
            apply_move(B, m);
            int child = verbose<=0 ? alpha_beta(B, depth-1-reduction, alpha, beta) :
                                     get_best_move(B, depth-1-reduction, alpha, beta, verbose-1, true).score;
            int new_score = is_white ? MAX(child, score) : MIN(child, score);
            if (is_white&&alpha<=child || !is_white&&beta<=child) { any_triumphant = true; }
            if (l==LMR_THRESH && !any_triumphant) { trigger_lmr = true; }
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

    /* since we changed depth above
     * ( if (MIN_FILTER_DEPTH<=depth && l==AR_THRESH ) ... )
     * we need to index into pv_table using the original depth.
     * (merit of "const"!) --- could have caused segfault!
     */
    pv_table[depth][(B->hash)%PV_TABLE_SIZE] = {B->hash, orig_alpha, orig_beta, {best_move, score}};
    return {best_move, score};
} 


