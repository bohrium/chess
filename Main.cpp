#include "Board.h"
#include "Search.h"
#include "Display.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <thread>

/* WARNING: if NB_PLIES too small, also should update verbose in main.c */
#define NB_WHITE_PLIES       6  
#define NB_BLACK_PLIES       6
#define NB_COMMENTARY_PLIES  6


int main(int argc, char** argv)
{
    srand(time(NULL));
    std::cout << ANSI_YELLOW;
    std::cout << "Welcome!" << std::endl; 

    Board B;
    init_board(&B);

    PVTable pv_table;
    zero_table(pv_table);
    
    for (int t=0; ; ++t) {
        print_board(&B);
        //char c; std::cin >> c;
        std::cout << "\033[15A" << std::flush; /* go up clear */

        std::cout << std::endl;
        int alpha=-KING_POINTS/2, beta=+KING_POINTS/2;
        int nb_plies = t%2==0 ? NB_WHITE_PLIES : NB_BLACK_PLIES; 
        int verbose  = 3;
        int layers   = 3;
        for (int k=0; k!=20; ++k) { std::cout << std::endl; }
        //std::cout << "\033[20B" << std::flush;
        //ScoredMove sm = get_best_move_multithreaded(&B, nb_plies, alpha, beta, layers, pv_table);
        ScoredMove sm = get_best_move(&B, nb_plies, alpha, beta, true, true, verbose, pv_table); 
        std::cout << "\033[20A" << std::flush;

        std::cout << (t%2==0 ? 'W' : 'B');
        std::cout << " plays "; print_pv(&B, nb_plies, verbose, pv_table);
        std::cout << ANSI_BLUE;
        std::cout << " " << get_best_move(&B, NB_COMMENTARY_PLIES, -KING_POINTS/2, +KING_POINTS/2, true, true, 0, pv_table).score; 
        std::cout << "            ";
        std::cout << ANSI_YELLOW << std::endl; /* yellow */

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

