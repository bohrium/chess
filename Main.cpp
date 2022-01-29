#include "Search.h"
#include "Helpers.h"
#include "EvalParams.h"
#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <time.h>
#include <thread>

ply_t const WHITE_DEPTH      = 16; 
ply_t const BLACK_DEPTH      = 10;
ply_t const COMMENTARY_DEPTH = 13;

ply_t const LINE_REPORT_PLIES = 6;

bool const MULTITHREADED = 0;
int const THREAD_LAYERS = 2;

ply_t const MAX_NB_PLIES = 360;

/*=============================================================================
====  0. MAIN LOOP  ===========================================================
=============================================================================*/

int main(int argc, char** argv)
{
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ~~~~  0.0. Initialize Board  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    std::cout << COLORIZE(GRAY, "Welcome!") << std::endl; 

    Board B;
    init_board(&B);
    static PVTable pv_table; /* static : don't waste stack space */
    zero_table(pv_table);

    srand(time(NULL));
    
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ~~~~  0.1. Main Loop  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    for (int t=0; t!=MAX_NB_PLIES; ++t) {
        /*--------  0.1.0. display game state  ------------------------------*/
        print_board_fancy(&B);
        GO_UP(34);

        /*--------  0.1.1. compute best move  -------------------------------*/
        std::cout << std::endl;
        int alpha=-KING_POINTS/2, beta=+KING_POINTS/2;
        int nb_plies = t%2==0 ? WHITE_DEPTH : BLACK_DEPTH; 
        ScoredMove sm;
        {
            GO_DOWN(40);
            if (MULTITHREADED) {
                sm = get_best_move_multithreaded(&B, nb_plies, alpha, beta, THREAD_LAYERS, pv_table);
            } else {
                sm = get_best_move(&B, nb_plies, alpha, beta, true, false, MAX_VERBOSE, pv_table); 
            }
            GO_UP(40);
        }

        /*--------  0.1.2. announce move (and predicted line)  --------------*/
        CLEAR_LINE(120);
        std::cout << COLORIZE(t%2?CYAN:MAGENTA, (t%2?'B':'W')) << " plays";
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
        print_pv(&B, nb_plies, LINE_REPORT_PLIES, pv_table);
        std::cout << " \"";
        std::cout << COLORIZE(RED, FLUSH_RIGHT_POS(4, sm.score)); 
        std::cout << "\"";

        /*--------  0.1.3. make move (update game state)  -------------------*/
        apply_move(&B, sm.m);

        /*--------  0.1.4. evaluate and display move quality  ---------------*/
        std::cout << " (";
        ScoredMove commentary = get_best_move(&B, COMMENTARY_DEPTH, -KING_POINTS/2, +KING_POINTS/2, true, true, 0, pv_table);
        std::cout << COLORIZE(RED, FLUSH_RIGHT_POS(4, commentary.score)); 
        std::cout << ")";
        CLEAR_LINE(120);
        std::cout << std::endl;
    }

    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ~~~~  0.2. Print Checkmate Compactly  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    print_board(&B);

    return 0;
}

