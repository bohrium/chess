#include "Board.h"
#include "EvalParams.h"
#include "Search.h"
#include "Helpers.h"
#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <time.h>
#include <thread>

/* WARNING: if NB_DEPTH too small, also should update verbose in main.c */
#define NB_WHITE_DEPTH      14
#define NB_BLACK_DEPTH      12
#define NB_COMMENTARY_DEPTH 10

#define LINE_REPORT_PLIES 4 

#define MULTITHREADED 0 

PVTable pv_table;

int main(int argc, char** argv)
{
    srand(time(NULL));
    std::cout << YELLOW;
    std::cout << "Welcome!" << std::endl; 

    Board B;
    init_board(&B);

    zero_table(pv_table);
    
    for (int t=0; t!=360; ++t) {
    //for (int t=0; t!=1; ++t) {
        // DISPLAY
        print_board_fancy(&B);
        GO_UP(33);

        // CALCULATION 
        std::cout << std::endl;
        int alpha=-KING_POINTS/2, beta=+KING_POINTS/2;
        int nb_plies = t%2==0 ? NB_WHITE_DEPTH : NB_BLACK_DEPTH; 

        GO_DOWN(40);
#if MULTITHREADED
        int thread_layers = 2;
        ScoredMove sm = get_best_move_multithreaded(&B, nb_plies, alpha, beta, thread_layers, pv_table);
#else
        ScoredMove sm = get_best_move(&B, nb_plies, alpha, beta, true, false, MAX_VERBOSE, pv_table); 
#endif
        GO_UP(40);

        // ANNOUNCE MOVE 
        CLEAR_REST_OF_LINE;
        if (t%2==0) { std::cout << COLORIZE(MAGENTA, 'W'); }
        else        { std::cout << COLORIZE(CYAN   , 'B'); }
        std::cout << " plays ";
        std::cout << "(height " << COLORIZE(GREEN, FLUSH_RIGHT(2, sm.height)) << ") ";
        print_pv(&B, nb_plies, LINE_REPORT_PLIES, pv_table);

        // GAME DYNAMICS
        apply_move(&B, sm.m);
        if (sm.m.taken.species == Species::king) {
            std::cout << "CHECKMATE!" << std::endl;
            break; 
        }

        // COMMENTARY
        std::cout << " ";
        ScoredMove commentary = get_best_move(&B, NB_COMMENTARY_DEPTH, -KING_POINTS/2, +KING_POINTS/2, true, true, 0, pv_table);
        std::cout << COLORIZE(RED, SHOW_SIGN(FLUSH_RIGHT(4, commentary.score))); 
        CLEAR_REST_OF_LINE;
        std::cout << std::endl;
    }
    print_board(&B);    

    return 0;
}

