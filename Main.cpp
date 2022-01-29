#include "Search.h"
#include "Helpers.h"
#include "EvalParams.h"
#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <time.h>
#include <thread>

#define NB_WHITE_DEPTH      16 
#define NB_BLACK_DEPTH      13
#define NB_COMMENTARY_DEPTH 10

#define LINE_REPORT_PLIES 6 

#define MULTITHREADED 0 
#define THREAD_LAYERS 2
#define MAX_NB_PLIES 360

PVTable pv_table;

/*=============================================================================
====  0. SEARCH FUNCTION  =====================================================
=============================================================================*/

int main(int argc, char** argv)
{
    std::cout << COLORIZE(GRAY, "Welcome!") << std::endl; 

    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ~~~~  0.0. Initialize Board  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    Board B;
    init_board(&B);
    zero_table(pv_table);

    srand(time(NULL));
    
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ~~~~  0.1. Main Loop  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    for (int t=0; t!=MAX_NB_PLIES; ++t) {
        /*--------  0.1.0. display game state  ------------------------------*/
        print_board_fancy(&B);
        GO_UP(33);

        /*--------  0.1.1. compute best move  -------------------------------*/
        std::cout << std::endl;
        int alpha=-KING_POINTS/2, beta=+KING_POINTS/2;
        int nb_plies = t%2==0 ? NB_WHITE_DEPTH : NB_BLACK_DEPTH; 
        ScoredMove sm;
        {
            GO_DOWN(40);
            #if MULTITHREADED
                sm = get_best_move_multithreaded(&B, nb_plies, alpha, beta, THREAD_LAYERS, pv_table);
            #else
                sm = get_best_move(&B, nb_plies, alpha, beta, true, false, MAX_VERBOSE, pv_table); 
            #endif
            GO_UP(40);
        }

        /*--------  0.1.2. announce move (and predicted line)  --------------*/
        CLEAR_REST_OF_LINE;
        std::cout << COLORIZE(t%2?CYAN:MAGENTA, (t%2?'B':'W')) << "plays";
        std::cout << "(height " << COLORIZE(GREEN, FLUSH_RIGHT(2, sm.height)) << ")";
        if (sm.m.taken.species == Species::king) {
            std::cout << std::endl;
            std::cout << "\t" << COLORIZE(GRAY,  "CHECKMATE!") << std::endl;
            std::cout << "\t" << COLORIZE(RED,   "CHECKMATE!") << std::endl;
            std::cout << "\t" << COLORIZE(YELLOW,"CHECKMATE!") << std::endl;
            std::cout << "\t" << COLORIZE(BLUE,  "CHECKMATE!") << std::endl;
            std::cout << "\t" << COLORIZE(GRAY,  "CHECKMATE!") << std::endl;
            break; 
        }
        std::cout << " ";
        print_pv(&B, nb_plies, LINE_REPORT_PLIES, pv_table);

        /*--------  0.1.3. make move (update game state)  -------------------*/
        apply_move(&B, sm.m);

        /*--------  0.1.4. evaluate and display move quality  ---------------*/
        std::cout << " ";
        ScoredMove commentary = get_best_move(&B, NB_COMMENTARY_DEPTH, -KING_POINTS/2, +KING_POINTS/2, true, true, 0, pv_table);
        std::cout << COLORIZE(RED, FLUSH_RIGHT_POS(4, commentary.score)); 
        CLEAR_REST_OF_LINE;
        std::cout << std::endl;
    }

    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ~~~~  0.2. Print Checkmate Compactly  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    print_board(&B);

    return 0;
}

