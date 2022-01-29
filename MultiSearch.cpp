#include "Search.h"
#include "Board.h"
#include "Helpers.h"
#include "EvalParams.h"
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <thread>

/*=============================================================================
====  2. MULTITHREADING  ======================================================
=============================================================================*/

int const THREADS_WIDTH = 4; /* TODO: alpha beta prune after bundle of threads (DONE but UNTUNED) */
int const ELDERS = 1;
int const MAX_MULTI_MOVES = 36;

ScoredMove get_best_move_multithreaded(Board* B, int const depth, int alpha, int beta, int layers, PVTable parent)
{
    if (layers<=0 || depth<=RED_DEPTH_SOFT_FLOOR) {
        return get_best_move(B, depth, alpha, beta, true, true, 0, parent);
    }

    const int orig_alpha = alpha;
    const int orig_beta = beta;
    const bool is_white = B->next_to_move==Color::white;

    bool stable = true;
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

    MoveList ML; 
    generate_moves(B, &ML, false);
    ML.length = MIN(ML.length, MAX_MULTI_MOVES);
    order_moves(B, &ML, ordering_depths[depth], 6, parent);

    ScoredMove sms[MAX_NB_MOVES];
    const int worst_case = is_white ? -KING_POINTS : +KING_POINTS;
    if (!ML.length) {
        /* NEW! handle no pieces */
        return {unk_move, worst_case, 0};
    }

    for (int l=0; l!=ELDERS; ++l) {
        Move m = ML.moves[l];
        int reduced_depth = 0.9*(depth-RED_DEPTH_HARD_FLOOR)+RED_DEPTH_HARD_FLOOR-1;

        GO_LEFT(120); GO_RIGHT((5-layers) * 8);
        print_move(B, m);
        GO_LEFT(120);

        {
            apply_move(B, m);
            ScoredMove sm = get_best_move_multithreaded(B, reduced_depth, alpha, beta, layers-1, parent);
            sms[l] = {m, sm.score, sm.height+1};
            undo_move(B, m);
        }

        if (is_white) { alpha = MAX(alpha, sms[l].score); } 
        else          { beta  = MIN(beta , sms[l].score); } 
        if ( beta <= alpha ) { ML.length = l+1; goto END; }
    }

    PVTable* my_pv_tables[MAX_MULTI_MOVES];
    int new_alphas[MAX_MULTI_MOVES];
    int new_betas [MAX_MULTI_MOVES];
    for (int t=0; t!=ML.length; ++t) {
        my_pv_tables[t] = (PVTable*)malloc(2*20*PV_TABLE_SIZE * sizeof(PVRecord));
        update_table(*(my_pv_tables[t]), parent);
    }

    for (int L=ELDERS; L<ML.length; L+=THREADS_WIDTH) {
        std::vector<std::thread> threads;
        int l;
        for (l=L; l<ML.length && l<L+THREADS_WIDTH; ++l) {
            new_alphas[l] = alpha;
            new_betas[l] = beta;

            Board by_val = copy_board(*B);
            threads.push_back(std::thread(
                [is_white, &ML,&sms,l,alpha,beta,depth,layers, &my_pv_tables, &new_alphas, &new_betas](Board by_val){
                    Move m = ML.moves[l];
                    int reduced_depth = 0.8*(depth-RED_DEPTH_HARD_FLOOR)+RED_DEPTH_HARD_FLOOR-1;
                    {
                        apply_move(&by_val, m);
                        ScoredMove sm = get_best_move_multithreaded(&by_val, reduced_depth, alpha, beta, layers-2, *(my_pv_tables[l]));
                        sms[l] = {m, sm.score, sm.height+1};
                        undo_move(&by_val, m);
                    }
                    if (is_white) { new_alphas[l] = MAX(new_alphas[l], sms[l].score); } 
                    else          { new_betas[l]  = MIN(new_betas[l] , sms[l].score); } 
                }, copy_board(*B))
            );
        }
        for (l=L; l<ML.length && l<L+THREADS_WIDTH; ++l) {
            /* TODO: for smoother printing: iterate backward ? */

            GO_LEFT(120); GO_RIGHT((5-layers) * 8);
            std::cout << COLORIZE(GREEN, "$"); print_move(B, ML.moves[l]);
            GO_LEFT(120); 
            {
                threads[l-L].join();
            }
            GO_LEFT(120); GO_RIGHT((5-layers) * 8);
            CLEAR_LINE(30);

            update_table(parent, *(my_pv_tables[l]));
            free(my_pv_tables[l]);

            alpha = MAX(alpha, new_alphas[l]);
            beta  = MIN(beta , new_betas [l]);
            if (is_white) { alpha = MAX(alpha, new_alphas[l]); } 
            else          { beta  = MIN(beta , new_betas[l] ); } 
        }
        if ( beta <= alpha ) { ML.length = l+1; goto END; }
    }

END:

    ScoredMove best = {ML.moves[0], worst_case, -1};
    ScoredMove next_best = {ML.moves[0], worst_case, -1};
    for (int l=0; l!=ML.length; ++l) {
        if ( is_white && best.score < sms[l].score ||
            !is_white && sms[l].score < best.score) {
            next_best = best;
            best = sms[l];
        } else if ( is_white && next_best.score < sms[l].score ||
                   !is_white && sms[l].score < next_best.score) {
            next_best = sms[l];
        }
    }

    #if ALLOW_CSR
    if (RED_DEPTH_SOFT_FLOOR<=depth && 2<=ML.length &&
        ( is_white && next_best.score + CSR_THRESH < best.score ||    
         !is_white && next_best.score - CSR_THRESH > best.score)) {
        int reduced_depth = depth-1;
        ScoredMove child;
        {
            apply_move(B, best.m);
            child = get_best_move_multithreaded(B, reduced_depth, alpha, beta, layers-1, parent);
            undo_move(B, best.m);
        }

        if ( is_white && child.score>=next_best.score ||
            !is_white && child.score<=next_best.score) {
            best = {best.m, child.score, child.height+1}; 
        } else {
            best = next_best;
        }
    }
    #endif//ALLOW_CSR
 
    if (stable) {
        parent[depth][(B->hash)%PV_TABLE_SIZE] = {B->hash, orig_alpha, orig_beta, best};
    }

    return best;
}

