#include "Board.h"
#include "Search.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>

#define NB_PLIES 6

int main(int argc, char** argv)
{
    srand(time(NULL));
    std::cout << "\033[0;33m"; /* yellow */
    std::cout << "Welcome!" << std::endl; 

    Board B;
    init_board(&B);
    MoveList ML;

    for (int t=0; ; ++t) {
        std::cout << std::endl;
        print_board(&B);
        Move m = get_best_move(&B, NB_PLIES);
        std::cout << "Played "; print_move(&B, m);
        std::cout << "\033[0;34m"; /* blue */
        std::cout << " " << alpha_beta(&B, NB_PLIES, -KING_POINTS/2, +KING_POINTS/2) << std::endl;  
        std::cout << "\033[0;33m"; /* yellow */
        apply_move(&B, m);
        if (m.taken.species == Species::king) {
            std::cout << "CHECKMATE!" << std::endl;
            break; 
        }
        char c;  std::cin >> c;
    }
    print_board(&B);    

    return 0;
}

