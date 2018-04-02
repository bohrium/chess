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
        if (p%2==0) { print_board(&B); }
        Move m = get_best_move(&B, (rand()%10==0) ? 2 : 4);
        apply_move(&B, m);
        std::cout << "played "; print_move(m);
        if (m.taken.species == Species::king) {
            std::cout << "CHECKMATE!" << std::endl;
            break; 
        }
    }
    print_board(&B);    

    return 0;
}

