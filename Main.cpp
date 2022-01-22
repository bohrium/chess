#include "Board.h"
#include "Search.h"
#include "Helpers.h"
#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <time.h>
#include <thread>

/* WARNING: if NB_PLIES too small, also should update verbose in main.c */
#define NB_WHITE_PLIES      10
#define NB_BLACK_PLIES       8
#define NB_COMMENTARY_PLIES  8

PVTable pv_table;

int main(int argc, char** argv)
{
    srand(time(NULL));
    std::cout << ANSI_YELLOW;
    std::cout << "Welcome!" << std::endl; 

    Board B;
    init_board(&B);

    zero_table(pv_table);
    
    for (int t=0; ; ++t) {
        print_board_fancy(&B);
        //char c; std::cin >> c;
        //std::cout << "\033[15A" << std::flush; /* go up clear */
        std::cout << "\033[33A" << std::flush; /* go up clear */

        std::cout << std::endl;
        int alpha=-KING_POINTS/2, beta=+KING_POINTS/2;
        int nb_plies = t%2==0 ? NB_WHITE_PLIES : NB_BLACK_PLIES; 
        int layers   = 3;
        for (int k=0; k!=50; ++k) { std::cout << std::endl; }
        //std::cout << "\033[20B" << std::flush;
        //ScoredMove sm = get_best_move_multithreaded(&B, nb_plies, alpha, beta, layers, pv_table);
        ScoredMove sm = get_best_move(&B, nb_plies, alpha, beta, true, false, MAX_VERBOSE, pv_table); 
        std::cout << "\033[50A" << std::flush;

        for (int k=0; k!=200; ++k) { std::cout << " "; }
        std::cout << "\33[200D";
        std::cout << (t%2==0 ? 'W' : 'B');
        std::cout << " plays (height " << ANSI_GREEN << std::right << std::setw(2) << sm.height << ANSI_YELLOW << ") ";
        print_pv(&B, nb_plies, MAX_VERBOSE, pv_table);
        int score = get_best_move(&B, NB_COMMENTARY_PLIES, -KING_POINTS/2, +KING_POINTS/2, true, true, 0, pv_table).score;
        std::cout << " " << ANSI_RED << std::showpos << std::right << std::setw(5) << score << std::noshowpos << ANSI_YELLOW; 
        std::cout << "            " << std::endl;

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

