#include "Board.h"
#include "Search.h"
#include <iostream>

int main(int argc, char** argv)
{
    std::cout << "Welcome!" << std::endl; 

    Board B;
    init_board(&B);
    MoveList ML;

    for (int p=0; p!=100; ++p) {
        print_board(&B);    
        Move m = get_best_move(&B, 3);
        std::cout << "suggested move: "; 
        print_move(m);
        apply_move(&B, m);
        if (m.taken.species == Species::king) {
            std::cout << "CHECKMATE!" << std::endl;
            break; 
        }
    }

    return 0;
}

