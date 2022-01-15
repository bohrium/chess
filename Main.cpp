#include "Board.h"
#include "Search.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <thread>

/* WARNING: if NB_PLIES too small, also should update verbose in main.c */
#define NB_WHITE_PLIES       12
#define NB_BLACK_PLIES       11
#define NB_COMMENTARY_PLIES  11


int main(int argc, char** argv)
{
    srand(time(NULL));
    std::cout << "\033[0;33m"; /* yellow */
    std::cout << "Welcome!" << std::endl; 

    Board B;
    init_board(&B);
    MoveList ML;

    PVTable pv_table;
    zero_table(pv_table);

    for (int t=0; ; ++t) {
        print_board(&B);
        std::cout << "\033[15A" << std::flush; /* go up clear */

        std::cout << std::endl;
        int alpha=-KING_POINTS/2, beta=+KING_POINTS/2;
        int nb_plies = t%2==0 ? NB_WHITE_PLIES : NB_BLACK_PLIES; 
        int verbose  = 4;
        int layers   = 3;
        std::cout << "\033[20B" << std::flush;

        ScoredMove sm = get_best_move_multithreaded(&B, nb_plies, alpha, beta, layers, pv_table);
        //ScoredMove sm = get_best_move(&B, nb_plies, alpha, beta, true, true, verbose, pv_table); 
        std::cout << "\033[20A" << std::flush;

        std::cout << (t%2==0 ? 'W' : 'B');
        std::cout << " plays "; print_pv(&B, nb_plies, verbose, pv_table);
        std::cout << "\033[0;34m"; /* blue */
        std::cout << " " << get_best_move(&B, NB_COMMENTARY_PLIES, -KING_POINTS/2, +KING_POINTS/2, true, true, 0, pv_table).score; 
        std::cout << "            ";
        std::cout << "\033[0;33m" << std::endl; /* yellow */

        apply_move(&B, sm.m);
        if (sm.m.taken.species == Species::king) {
            std::cout << "CHECKMATE!" << std::endl;
            break; 
        }

        //char c;  std::cin >> c;
    }
    print_board(&B);    

    return 0;
}

