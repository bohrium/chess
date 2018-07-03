#include "Board.h"
#include "Search.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>

int main(int argc, char** argv)
{
    srand(time(NULL));
    std::cout << "Welcome!" << std::endl; 

    Board B;
    init_board(&B);
    MoveList ML;

    for (int p=0; p!=200; ++p) {
        if (p%1==0) { print_board(&B); }
        Move m = get_best_move(&B, (rand()%5==0) ? 6 : 6);
        std::cout << "Played "; print_move(&B, m);
        std::cout << " " << (float)((int)(alpha_beta(&B, 5, -500.0, +500.0)*100))/100 << std::endl;  
        apply_move(&B, m);
        if (m.taken.species == Species::king) {
            std::cout << "CHECKMATE!" << std::endl;
            break; 
        }
    }
    print_board(&B);    

    return 0;
}

