#include "Search.h"
#include "Board.h"
#include <iostream>
#include <algorithm>
#include <thread>

#define MIN(X,Y) (((X)<(Y))?(X):(Y))
#define MAX(X,Y) (((X)>(Y))?(X):(Y))

void update_table(PVTable parent, PVTable update);

Move shallow_greedy_move(Board* B);
void order_moves(Board* B, MoveList* ML, int depth, int k, PVTable table);
int stable_eval(Board* B, int max_plies, int alpha, int beta);


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

#define ELDERS 2
#define COYOUTHS AR_THRESH 
ScoredMove get_best_move_multithreaded(Board* B, const int depth, int alpha, int beta, int layers, PVTable parent)
{
    if (layers<=0) { return get_best_move(B, depth, alpha, beta, true, true, 0, parent); };

    const int orig_alpha = alpha;
    const int orig_beta = beta;
    const bool is_white = B->next_to_move==Color::white;

    /* TODO: protect hash writes as atomic? */
    bool stable = true;
    if (stable) {
        PVRecord pvr = parent[depth][B->hash % PV_TABLE_SIZE];
        if (pvr.hash == B->hash) {
            if ((pvr.sm.score > pvr.alpha || pvr.alpha <= alpha) &&
                (pvr.sm.score < pvr.beta  || beta <= pvr.beta)) {
              return pvr.sm;
            }
        };
    }

    MoveList ML; 
    generate_moves(B, &ML);
    ML.length = MIN(ML.length, 36);
    PVTable* my_pv_tables[36];
    for (int t=0; t!=ML.length; ++t) {
        my_pv_tables[t] = (PVTable*)malloc(2*20*PV_TABLE_SIZE * sizeof(PVRecord));
        zero_table(*(my_pv_tables[t]));
    }

    order_moves(B, &ML, ordering_depths[depth], 6, parent);
    ScoredMove sms[MAX_NB_MOVES];

    ScoredMove best = {unk_move, is_white ? -KING_POINTS : +KING_POINTS};
    for (int l=0; l!=ELDERS; ++l) {
        int reduced_depth = depth-1; /* assume cosingular is triggered */
        if (1 <= l) { reduced_depth -= 1; }
        if (COYOUTHS <= l) { reduced_depth -= 1; }

        Move m = ML.moves[l];
        for (int i=0; i!=5-layers; ++i) { std::cout << "\033[2C"; }
        print_move(B, m); std::cout << "\n\033[100D" << std::flush;
        apply_move(B, m);
        sms[l] = {m, get_best_move_multithreaded(B, reduced_depth, alpha, beta, layers-1, parent).score};
        undo_move(B, m);
        std::cout << "\033[1A" << std::flush;

        if ( is_white && best.score < sms[l].score ||
            !is_white && sms[l].score < best.score) {
            best = sms[l];
        }

        if (is_white) { if (sms[l].score >= beta ) goto END; alpha = MAX(alpha, sms[l].score); } 
        else          { if (sms[l].score <= alpha) goto END; beta  = MIN(beta , sms[l].score); } 
    }

    {
        std::vector<std::thread> threads;
        for (int l=ELDERS; l<ML.length; ++l) {
            Board by_val = copy_board(*B);
            threads.push_back(
                std::thread([&best, is_white, &ML,&sms,l,alpha,beta,depth,layers, &my_pv_tables](Board by_val){
                  Move m = ML.moves[l];
                  apply_move(&by_val, m);
                  int reduced_depth = depth-1; /* assume cosingular is triggered */
                  if (1 <= l) { reduced_depth -= 1; }
                  if (COYOUTHS <= l) { reduced_depth -= 1; }
                  sms[l] = {m, get_best_move_multithreaded(&by_val, reduced_depth, alpha, beta, layers-2,  *(my_pv_tables[l])).score};
                  undo_move(&by_val, m);
                  if ( is_white && best.score < sms[l].score ||
                      !is_white && sms[l].score < best.score) {
                      best = sms[l];
                  }
                }, copy_board(*B)));
        }
        for (int l=ML.length-1; l>=ELDERS; --l) {
            for (int i=0; i!=5-layers; ++i) { std::cout << "  "; }
            std::cout << "waiting on "; print_move(B, ML.moves[l]); std::cout << std::flush; 
            std::cout << "\n\033[100D" << std::flush;
            threads[l-ELDERS].join();
            std::cout << "\033[1A                              \n\033[1A" << std::flush;
            update_table(parent, *(my_pv_tables[l]));
        }
    }

END:

    if (stable) {
        parent[depth][(B->hash)%PV_TABLE_SIZE] = {B->hash, orig_alpha, orig_beta, best};
    }
    for (int t=0; t!=ML.length; ++t) {
        free(my_pv_tables[t]);
    }

    return best;
}

ScoredMove get_best_move(Board* B, const int depth, int alpha, int beta, bool stable, bool null_move_okay, int verbose, PVTable parent)
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
    if (stable) {
        PVRecord pvr = parent[depth][B->hash % PV_TABLE_SIZE];
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
        order_moves(B, &ML, ordering_depths[depth], 6, parent);//branching_factors[depth]);
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
            ScoredMove child = get_best_move(B, depth-1-NMR_AMOUNT, alpha_hi, beta_hi, stable, false, 0, parent);
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
            score_fst = get_best_move(B, ordering_depths[depth], alpha, beta, true, true, 0, parent).score;
            undo_move(B, ML.moves[0]);
        }
        int alpha_lo = is_white ? score_fst   - CSR_THRESH : score_fst-1 + CSR_THRESH; 
        int beta_lo  = is_white ? score_fst+1 - CSR_THRESH : score_fst   + CSR_THRESH;
        {
            apply_move(B, ML.moves[1]);
            score_snd = get_best_move(B, ordering_depths[depth], alpha_lo, beta_lo, true, true, 0, parent).score;
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
            int child = get_best_move(B, child_depth-1, alpha_lo, beta_lo, stable, true, 0, parent).score;
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
            int child = get_best_move(B, depth-1-reduction, alpha, beta, stable, true, verbose-1, parent).score;
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
    if (stable) {
        parent[depth][(B->hash)%PV_TABLE_SIZE] = {B->hash, orig_alpha, orig_beta, {best_move, score}};
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
void order_moves(Board* B, MoveList* ML, int depth, int k, PVTable parent)
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
    //for (int s=0; s!=2; ++s) {
        for (int d=0; d!=15; ++d) {
            for (int i=0; i!=PV_TABLE_SIZE; ++i) {
                table[d][i].hash = 42;
                table[d][i].alpha = +1;
                table[d][i].beta = -1;
            }
        }
    //}
}

void update_table(PVTable parent, PVTable update)
{
    //for (int s=0; s!=2; ++s) {
        for (int d=0; d!=20; ++d) {
            for (int i=0; i!=PV_TABLE_SIZE; ++i) {
                if (update[d][i].hash%PV_TABLE_SIZE != i) { continue; }
                if (parent[d][i].hash%PV_TABLE_SIZE == i) { continue; }
                parent[d][i] = update[d][i]; 
            }
        }
    //}
}

void print_pv(Board* B, int depth, int verbose, PVTable parent)
{
    if (verbose<=0) { return; } 
    PVRecord pvr;
    int dd;
    for (dd=0; ; ++dd) {
        if (depth-dd<=MIN_FILTER_DEPTH) { return; } 
        pvr = parent[depth-dd][(B->hash)%PV_TABLE_SIZE];
        if (B->hash == pvr.hash) { break; } 
    }
    print_move(B, pvr.sm.m);
    {
        apply_move(B, pvr.sm.m);
        print_pv(B, depth-dd, verbose-1, parent); 
        undo_move(B, pvr.sm.m);
    }
}


