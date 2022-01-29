#include "Search.h"
#include "Board.h"
#include "Helpers.h"
#include "EvalParams.h"
#include <iomanip>
#include <iostream>
#include <algorithm>

/*=============================================================================
====  0. SEARCH FUNCTION  =====================================================
=============================================================================*/

/* important to parenthesize macro args!  one hour bug traced to this */
#define BARK(VERBOSE,STMNT)                             \
    if (0<(VERBOSE)) {                                  \
        GO_RIGHT(15 * (MAX_VERBOSE-(VERBOSE)));         \
        {STMNT;}                                        \
        CLEAR_LINE(30);                                 \
    }                                                    

ScoredMove get_best_move(Board* B, int const depth, int alpha, int beta, bool stable, bool null_move_okay, int verbose, PVTable parent)
{
    /* TODO: handle checkmate / draw leaf termination!! */

    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ~~~~  0.0. Base Case  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    if (depth<=0) {
        int score = stable ? stable_eval(B, QUIESCE_DEPTH, alpha, beta)
                           : evaluate(B);
        return {unk_move, score, 0};
    }

    const int orig_alpha = alpha;
    const int orig_beta = beta;
    const bool is_white = B->next_to_move==Color::white;

    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ~~~~  0.1. Hash Read  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    if (stable) {
        for (int i=0; i!=2; ++i) { /* NEW FEATURE: look up deepers ! */
            PVRecord pvr = parent[depth+i][B->hash % PV_TABLE_SIZE];
            if (pvr.hash != B->hash) { continue; }
            if ((pvr.sm.score > pvr.alpha || pvr.alpha <= alpha) &&
                (pvr.sm.score < pvr.beta  || beta <= pvr.beta)) {
                return pvr.sm;
            }
            break;
        }
    }

    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ~~~~  0.2. Generate Moves  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    MoveList ML;  
    generate_moves(B, &ML, false);
    int nb_candidates = ML.length; 
    if (2<=depth) {
        BARK(verbose,std::cout<<COLORIZE(GRAY,"sort'n"));
        order_moves(B, &ML, ordering_depths[depth], 6, parent);//branching_factors[depth]);
        nb_candidates = MIN(ML.length, branching_factors[depth]);
    }

    const int worst_case = is_white ? -KING_POINTS : +KING_POINTS;
    if (!nb_candidates) {
        /* NEW! handle no pieces */
        return {unk_move, worst_case, 0};
    }
    ScoredMove best, next_best;
    best.m = ML.moves[0]; 
    next_best.m = ML.moves[0]; 
    best.score = worst_case;
    next_best.score = worst_case;

    bool any_triumphant = false;  
    bool trigger_lmr = false;  

    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ~~~~  0.3. Null Move Reduction  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    #if ALLOW_NMR
    if ((RED_DEPTH_SOFT_FLOOR<=depth && null_move_okay)) { 
        int pass = stable_eval(B, QUIESCE_DEPTH, alpha-2, beta+2);
        if (is_white && beta<pass || !is_white && pass<alpha) {
            BARK(verbose,std::cout << "\033[3D" << COLORIZE(GREEN, "NUL"));
            apply_null(B);
            int alpha_hi = is_white ? beta-1+NMR_THRESH : alpha  -NMR_THRESH; 
            int beta_hi  = is_white ? beta  +NMR_THRESH : alpha+1-NMR_THRESH; 
            int child_depth = NMR_AMOUNT*(depth-RED_DEPTH_HARD_FLOOR)+RED_DEPTH_HARD_FLOOR-1; 
            ScoredMove child = get_best_move(B, child_depth, alpha_hi, beta_hi, stable, false, verbose, parent);
            bool skip = false;
            if (( is_white && beta +NMR_THRESH<=child.score) ||
                (!is_white && child.score<=alpha-NMR_THRESH)) {
                best = {unk_move, child.score, 0};//child.height+1}; 
                skip = true;
            }
            undo_null(B);
            BARK(verbose,std::cout << "\033[3D" << COLORIZE(GREEN, "   "));
            if (skip) { goto END; }
        }
    }
    #endif//ALLOW_NMR

    ScoredMove results[MAX_NB_MOVES];

    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ~~~~  0.4. Main Loop  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    for (int l=0; l!=nb_candidates; ++l) {
        /*--------  0.4.0. check for game termination  ----------------------*/
        Move m = ML.moves[l];
        if (m.taken.species == Species::king) { 
            best = {m, (is_white ? +KING_POINTS : -KING_POINTS), 0};
            goto END;
        }

        /*--------  0.4.1. compute default reduction  -----------------------*/
        float reduction_factor = 1.0;
        #if ALLOW_CSR
        if (RED_DEPTH_SOFT_FLOOR<=depth && 2<=ML.length) {
           reduction_factor *= CSR_AMOUNT;
        } 
        #endif//ALLOW_CSR
        #if ALLOW_AR
        if (RED_DEPTH_SOFT_FLOOR<=depth && AR_THRESH<=l) {
            //trigger_lmr = true;
           reduction_factor *= AR_AMOUNT;
        }
        #endif//ALLOW_AR 

        /*--------  0.4.2. display move under consideration  ----------------*/
        BARK(verbose,std::cout<<COLORIZE(GRAY,FLUSH_RIGHT(2,l)<<"/"<<FLUSH_RIGHT(2,nb_candidates))<<" ";print_move(B,m));

        /*--------  0.4.3. late move reduction  -----------------------------*/
        #if ALLOW_LMR
        bool skip = false; 
        /* scout / late move reduction */
        if ((RED_DEPTH_SOFT_FLOOR <=depth && 1<=l) && 
            (SCOUT_THRESH<=beta-alpha || (trigger_lmr && !is_capture(m)))) {
            BARK(verbose-1,std::cout << "\033[3D" << COLORIZE(RED, "LMR"));
            apply_move(B, m);
            int alpha_lo = is_white ? alpha : beta-1; 
            int beta_lo  = is_white ? alpha+1 : beta; 

            int local_rf = reduction_factor;
            if (trigger_lmr && !is_capture(m)) { local_rf *= LMR_AMOUNT; }

            int child_depth = local_rf*(depth-RED_DEPTH_HARD_FLOOR)+RED_DEPTH_HARD_FLOOR-1; 

            int child = get_best_move(B, child_depth, alpha_lo, beta_lo, stable, true, verbose-1, parent).score;
            if ( is_white && child<=alpha ||
                !is_white && beta <=child) {
                skip = true;
            }
            undo_move(B, m);
            BARK(verbose-1,std::cout << "\033[3D" << COLORIZE(RED, "   "));
        }
        if (skip) { continue; }
        #endif//ALLOW_LMR

        /*--------  0.4.4. full search  -------------------------------------*/
        {
            apply_move(B, m);
            int child_depth = reduction_factor*(depth-RED_DEPTH_HARD_FLOOR)+RED_DEPTH_HARD_FLOOR-1; 
            ScoredMove child = get_best_move(B, child_depth, alpha, beta, stable, true, verbose-1, parent);
            undo_move(B, m);

            int new_score = is_white ? MAX(child.score, best.score) : MIN(child.score, best.score);
            if ( is_white && alpha<=child.score ||
                !is_white && beta <=child.score) {
                any_triumphant = true;
            }
            if (l==LMR_THRESH && !any_triumphant) {
                trigger_lmr = true;
            }
            if (new_score != best.score) {
                next_best = best;
                best = {m, child.score, child.height+1}; 
            } else {
                int new_next_score = is_white ? MAX(child.score, next_best.score) : MIN(child.score, next_best.score);
                if (new_next_score != next_best.score) {
                    next_best = {m, child.score, child.height+1};
                }
            }
        }

        /*--------  0.4.5. alpha-beta updates and cutoffs  ------------------*/
        /* NB: at top level, cutoff won't happen except when taking king. */
        if (is_white) { if (best.score >= beta ) break; alpha = MAX(alpha, best.score); } 
        else          { if (best.score <= alpha) break; beta  = MIN(beta , best.score); } 
    }

    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ~~~~  0.4. Co-Singular Reduction  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    #if ALLOW_CSR
    if (RED_DEPTH_SOFT_FLOOR<=depth && 2<=ML.length &&
        ( is_white && next_best.score + CSR_THRESH < best.score ||    
         !is_white && next_best.score - CSR_THRESH > best.score)) {
        BARK(verbose,std::cout<<COLORIZE(YELLOW,"cosing");print_move(B,best.m));
        apply_move(B, best.m);
        ScoredMove child = get_best_move(B, depth-1, alpha, beta, true, true, verbose-1, parent);
        undo_move(B, best.m);

        if ( is_white && child.score>=next_best.score ||
            !is_white && child.score<=next_best.score) {
            best = {best.m, child.score, child.height+1}; 
        } else {
            best = next_best;
        }
    }
    #endif//ALLOW_CSR
 
END:
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ~~~~  0.5. Hash Write  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    if (stable) {
        parent[depth][(B->hash)%PV_TABLE_SIZE] = {B->hash, orig_alpha, orig_beta, best};
    }
    return best;
} 

/*=============================================================================
====  1. SEARCH HELPERS  ======================================================
=============================================================================*/

bool shallow_greedy_move(Board* B, Move* m)
{
    MoveList ML;  
    generate_moves(B, &ML, true);
    if (!ML.length) { return false; }

    int move_acc = 0;

    int sign = B->next_to_move == Color::white ? +1 : -1; 

    int score_acc = -KING_POINTS/2; 
    for (int l=0; l!=ML.length; ++l) {
        apply_move(B, ML.moves[l]);
        int score = sign * evaluate(B);
        undo_move(B, ML.moves[l]);
        if (score_acc < score) {
            score_acc = score;
            move_acc = l;
        }
    }
    //score_acc *= sign; /* useless but goes with computation spirit */

    *m = ML.moves[move_acc];
    return true;
}

int stable_eval(Board* B, int max_plies, int alpha, int beta)
{
    /* TODO: check for taken king at very beginning and during loop */

    int pass = evaluate(B);
    if (max_plies<=0) { return pass; }
    bool is_white = B->next_to_move==Color::white;
    if (is_white && pass>=beta || !is_white && pass<alpha) { return pass; } 
    if ( is_white && alpha<pass ) { alpha = pass; }
    if (!is_white && pass< beta ) { beta  = pass; }

    Move m;
    bool found_capture = shallow_greedy_move(B, &m);
    if (!found_capture) { return pass; }
    //if (!is_capture(m)) { return pass; }

    apply_move(B, m);
    int go = stable_eval(B, max_plies-1, alpha, beta); 
    undo_move(B, m);
    return is_white ? MAX(pass, go) : MIN(pass, go);
}

// ensures first k elements of ML coincide with top k moves as determined by
// search with depth many plies, and that these elements are sorted. 
void order_moves(Board* B, MoveList* ML, int depth, int k, PVTable parent)
{
    k = 100; /* NEW */

    int shallow_scores[MAX_NB_MOVES]; 
    int sorted_indices[MAX_NB_MOVES];
    bool stable = (STABLE_ORDER_DEPTH <= depth); 

    bool is_white = (B->next_to_move==Color::white);

    for (int m=0; m!=ML->length; ++m) {
        apply_move(B, ML->moves[m]);
        int score = get_best_move(B, depth, -KING_POINTS/2, +KING_POINTS/2, stable, true, 0, parent).score;
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

void zero_table(PVTable table)
{
    for (int d=0; d!=15; ++d) {
        for (int i=0; i!=PV_TABLE_SIZE; ++i) {
            table[d][i].hash = 42;
            table[d][i].alpha = +1;
            table[d][i].beta = -1;
        }
    }
}

void update_table(PVTable parent, PVTable update)
{
    for (int d=0; d!=20; ++d) {
        for (int i=0; i!=PV_TABLE_SIZE; ++i) {
            if (update[d][i].hash%PV_TABLE_SIZE != i) { continue; }
            if (parent[d][i].hash%PV_TABLE_SIZE == i) { continue; }
            parent[d][i] = update[d][i]; 
        }
    }
}

void print_pv(Board* B, int depth, int verbose, PVTable parent)
{
    if (verbose<=0) { return; } 
    PVRecord pvr;
    int dd;
    for (dd=0; ; ++dd) {
        if (depth-dd<=0) { return; } 
        pvr = parent[depth-dd][(B->hash)%PV_TABLE_SIZE];
        if (B->hash == pvr.hash) { break; } 
    }
    print_move(B, pvr.sm.m);
    if (pvr.sm.m.type != MoveType::extra_legal) {
        apply_move(B, pvr.sm.m);
        print_pv(B, depth-dd, verbose-1, parent); 
        undo_move(B, pvr.sm.m);
    }
}

