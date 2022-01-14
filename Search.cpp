#include "Search.h"
#include "Board.h"
#include <iostream>
#include <algorithm>

#define MIN(X,Y) (((X)<(Y))?(X):(Y))
#define MAX(X,Y) (((X)>(Y))?(X):(Y))

Move shallow_greedy_move(Board* B);
void order_moves(Board* B, MoveList* ML, int depth, int k);
int stable_eval(Board* B, int max_plies, int alpha, int beta);

typedef struct PVRecord {
    unsigned int hash;
    int alpha;
    int beta;
    ScoredMove sm;
} PVRecord;
#define PV_TABLE_SIZE 25000
PVRecord pv_table[2][20][PV_TABLE_SIZE];

/*=============================================================================
====  0. SEARCH FUNCTION  =====================================================
=============================================================================*/

#define BARK(STMNT)                                     \
    if (0<verbose) {                                    \
        std::cout << "  ";                              \
        for (int t=0; t!=MAX_VERBOSE-verbose; ++t) {    \
            std::cout << "\033[6C";                     \
        }                                               \
        STMNT;                                          \
        std::cout << "\033[200D" << std::flush;         \
    }                                                    

ScoredMove get_best_move(Board* B, const int depth, int alpha, int beta, bool stable, bool null_move_okay, int verbose)
{
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ~~~~  0.0. Base Case  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    if (depth<=0) {
        int score = stable ? stable_eval(B, QUIESCE_DEPTH, alpha, beta)
                           : evaluate(B);
        return {unk_move, score};
    }

    const int orig_alpha = alpha;
    const int orig_beta = beta;
    const bool is_white = B->next_to_move==Color::white;

    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ~~~~  0.1. Hash Read  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    if (stable || depth) {
        PVRecord pvr = pv_table[stable?1:0][depth][B->hash % PV_TABLE_SIZE];
        if (pvr.hash == B->hash) {
          if ((pvr.sm.score > pvr.alpha || pvr.alpha <= alpha) &&
              (pvr.sm.score < pvr.beta  || beta <= pvr.beta)) {
            return pvr.sm;
          }
        };
    }

    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ~~~~  0.2. Generate Moves  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    MoveList ML;  
    generate_moves(B, &ML);
    int nb_candidates = ML.length; 
    if (2<=depth) {
        BARK(std::cout<<"Sorting Moves      ");
        order_moves(B, &ML, ordering_depths[depth], 6);//branching_factors[depth]);
        nb_candidates = MIN(ML.length, branching_factors[depth]);
    }

    int score = is_white ? -KING_POINTS : +KING_POINTS; 
    Move best_move;
    bool any_triumphant = false;  
    bool trigger_lmr = false;  
    bool trigger_csr = false;  

    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ~~~~  0.3. Pre-Loop Processing  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    /*--------  0.3.0. null move reduction  ---------------------------------*/
    #if ALLOW_NMR
    {
        int pass = evaluate(B); /* TODO: replace by quiescent? */
        if ((MIN_FILTER_DEPTH<=depth && null_move_okay) && 
            (is_white && beta<pass || !is_white && pass<alpha)) {
            BARK(std::cout<<"Trying Null      ");
            apply_null(B);
            int alpha_hi = is_white ? beta-1+NMR_THRESH : alpha  -NMR_THRESH; 
            int beta_hi  = is_white ? beta  +NMR_THRESH : alpha+1-NMR_THRESH; 
            ScoredMove child = get_best_move(B, depth-1-NMR_AMOUNT, alpha_hi, beta_hi, stable, false, 0);
            bool skip = false;
            if (( is_white && beta +NMR_THRESH<=child.score) ||
                (!is_white && child.score<=alpha-NMR_THRESH)) {
                score = child.score; 
                best_move = child.m; 
                skip = true;
            }
            undo_null(B);
            if (skip) { goto END; }
        }
    }
    #endif//ALLOW_NMR

    /*--------  0.3.1. co-singular reduction  -------------------------------*/
    #if ALLOW_CSR
    if (MIN_FILTER_DEPTH<=depth && 2<=ML.length) {
        BARK(std::cout<<"Trying Singular      ");
        int score_fst, score_snd;
        {
            // TODO: avoid this egregious repetition of order_moves()'s work
            apply_move(B, ML.moves[0]);
            score_fst = get_best_move(B, ordering_depths[depth], alpha, beta, true, true, 0).score;
            undo_move(B, ML.moves[0]);
        }
        int alpha_lo = is_white ? score_fst   - CSR_THRESH : score_fst-1 + CSR_THRESH; 
        int beta_lo  = is_white ? score_fst+1 - CSR_THRESH : score_fst   + CSR_THRESH;
        {
            apply_move(B, ML.moves[1]);
            score_snd = get_best_move(B, ordering_depths[depth], alpha_lo, beta_lo, true, true, 0).score;
            undo_move(B, ML.moves[1]);
        }
        if (( is_white && score_snd<=alpha_lo) || 
            (!is_white && beta_lo <=score_snd)) {
            trigger_csr = true;
        }
    } else if (1==ML.length) {
        trigger_csr = true;
    }
    #endif//ALLOW_CSR

    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ~~~~  0.4. Main Loop  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    for (int l=0; l!=nb_candidates; ++l) {
        /*--------  0.4.0. check for game termination  ----------------------*/
        Move m = ML.moves[l];
        if (m.taken.species == Species::king) { 
            best_move = m;
            score = is_white ? +KING_POINTS : -KING_POINTS;
            break;
        }

        /*--------  0.4.1. compute default reduction  -----------------------*/
        int reduction=0;
        #if ALLOW_CSR
        if (trigger_csr&&l==1 || !trigger_csr&&l==0) {
            reduction += CSR_AMOUNT;
        } 
        #endif//ALLOW_CSR
        #if ALLOW_AR
        if (MIN_FILTER_DEPTH<=depth && AR_THRESH<=l) {
            reduction += AR_AMOUNT;
        }
        #endif//ALLOW_AR

        /*--------  0.4.2. display move under consideration  ----------------*/
        BARK(print_move(B,m);std::cout<<"            ");

        /*--------  0.4.3. late move reduction  -----------------------------*/
        #if ALLOW_LMR
        bool skip = false; 
        /* scout / late move reduction */
        if ((MIN_FILTER_DEPTH <=depth && 1<=l) && 
            (SCOUT_THRESH<=beta-alpha || (trigger_lmr && !is_capture(m)))) {
            apply_move(B, m);
            int alpha_lo = is_white ? alpha : beta-1; 
            int beta_lo  = is_white ? alpha+1 : beta; 
            int child_depth = depth-reduction;
            if (trigger_lmr && !is_capture(m)) { child_depth -= LMR_AMOUNT; }
            int child = get_best_move(B, child_depth-1, alpha_lo, beta_lo, stable, true, 0).score;
            if (( is_white && child<=alpha) ||
                (!is_white && beta <=child)) {
                skip = true;
            }
            undo_move(B, m);
        }
        if (skip) { continue; }
        #endif//ALLOW_LMR

        /*--------  0.4.4. full search  -------------------------------------*/
        {
            apply_move(B, m);
            int child = get_best_move(B, depth-1-reduction, alpha, beta, stable, true, verbose-1).score;
            int new_score = is_white ? MAX(child, score) : MIN(child, score);
            if (is_white&&alpha<=child || !is_white&&beta<=child) { any_triumphant = true; }
            if (l==LMR_THRESH && !any_triumphant) { trigger_lmr = true; }
            if (new_score != score) {
                score = new_score; 
                best_move = m;
            }
            undo_move(B, m);
        }

        /*--------  0.4.5. alpha-beta updates and cutoffs  ------------------*/
        /* note: cutoff will never happen at top level (except in king-taking
         * case)                                                             */
        if (is_white) { if (score >= beta ) break; alpha = MAX(alpha, score); } 
        else          { if (score <= alpha) break; beta  = MIN(beta , score); } 
    }

END:
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ~~~~  0.4. Hash Write  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    if (stable || depth) {
        pv_table[stable?1:0][depth][(B->hash)%PV_TABLE_SIZE] = {B->hash, orig_alpha, orig_beta, {best_move, score}};
    }
    return {best_move, score};
} 

/*=============================================================================
====  1. SEARCH HELPERS  ======================================================
=============================================================================*/

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
// search with depth many plies, and that these elements are sorted. 
void order_moves(Board* B, MoveList* ML, int depth, int k)
{
    //if (1<=depth) {
    //    order_moves(B, ML, 0, k);
    //}

    int shallow_scores[MAX_NB_MOVES]; 
    int sorted_indices[MAX_NB_MOVES];
    bool stable = (STABLE_ORDER_DEPTH <= depth); 

    bool is_white = (B->next_to_move==Color::white);

    for (int m=0; m!=ML->length; ++m) {
        apply_move(B, ML->moves[m]);
        int score = get_best_move(B, depth, -KING_POINTS/2, +KING_POINTS/2, stable, true, 0).score;
        undo_move(B, ML->moves[m]);
        shallow_scores[m] = score; 
        sorted_indices[m] = m;
    }

    if (is_white) {
        /* black want higher scores on left */
        auto scorer = [shallow_scores](int a, int b){return (shallow_scores[a]) > (shallow_scores[b]);};
        std::sort(sorted_indices, sorted_indices+ML->length, scorer); 
    } else {
        /* black want lower scores on left */
        auto scorer = [shallow_scores](int a, int b){return (shallow_scores[a]) < (shallow_scores[b]);};
        std::sort(sorted_indices, sorted_indices+ML->length, scorer); 
    }

    Move sorted_moves[MAX_NB_MOVES]; 
    for (int m=0; m!=MIN(k,ML->length); ++m) {
        sorted_moves[m] = ML->moves[sorted_indices[m]];
    }
    for (int m=0; m!=MIN(k,ML->length); ++m) {
        ML->moves[m] = sorted_moves[m];
    }
} 

/*=============================================================================
====  2. PV TABLE HELPERS  ====================================================
=============================================================================*/

void zero_tables()
{
    for (int s=0; s!=2; ++s) {
        for (int d=0; d!=15; ++d) {
            for (int i=0; i!=PV_TABLE_SIZE; ++i) {
                pv_table[s][d][i].hash = 42;
                pv_table[s][d][i].alpha = +1;
                pv_table[s][d][i].beta = -1;
            }
        }
    }
}

void print_pv(Board* B, int depth, int verbose)
{
    if (verbose<=0) { return; } 
    PVRecord pvr = pv_table[1][depth][(B->hash)%PV_TABLE_SIZE];
    if (B->hash != pvr.hash) { return; }
    print_move(B, pvr.sm.m);
    {
        apply_move(B, pvr.sm.m);
        print_pv(B, depth-1, verbose-1); 
        undo_move(B, pvr.sm.m);
    }
}


