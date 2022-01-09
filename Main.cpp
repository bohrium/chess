#include "Board.h"
#include "Search.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>

#define NB_WHITE_PLIES 6
#define NB_BLACK_PLIES 6
#define NB_COMMENTARY_PLIES 6

int main(int argc, char** argv)
{
    srand(time(NULL));
    std::cout << "\033[0;33m"; /* yellow */
    std::cout << "Welcome!" << std::endl; 

    Board B;
    init_board(&B);
    MoveList ML;

    //print_board(&B);
    for (int t=0; ; ++t) {
        std::cout << std::endl;
        Move m = t%2==0 ? get_best_move(&B, NB_WHITE_PLIES) 
                        : get_best_move(&B, NB_BLACK_PLIES);
        std::cout << "Played "; print_move(&B, m);
        std::cout << "\033[0;34m"; /* blue */
        std::cout << " " << alpha_beta(&B, NB_COMMENTARY_PLIES, -KING_POINTS/2, +KING_POINTS/2);  
        std::cout << "            \033[0;33m" << std::endl; /* yellow */
        apply_move(&B, m);
        print_board(&B);
        if (m.taken.species == Species::king) {
            std::cout << "CHECKMATE!" << std::endl;
            break; 
        }
        std::cout << "\033[15A" << std::flush; /* go up clear */
        //char c;  std::cin >> c;
    }
    print_board(&B);    

    return 0;
}

