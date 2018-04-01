#include "Board.h"
#include <iostream>

int main(int argc, char** argv)
{
    std::cout << "Welcome!" << std::endl; 

    Board B;
    init_board(&B);
    MoveList ML;

    /* Ruy Lopez, Exchange Variation */
    Move Ruy[7] = {
        {{6,4}, {4,4}, empty_piece}, // e4

        {{1,4}, {3,4}, empty_piece}, // e5
        {{7,6}, {5,5}, empty_piece}, // Nf3
        {{0,1}, {2,2}, empty_piece}, // Nc6
        {{7,5}, {3,1}, empty_piece}, // Bb5
        {{1,0}, {2,0}, empty_piece}, // a6
        {{3,1}, {2,2}, {Color::black, Species::knight}} // Bxc6
    };

    for (int i=0; i!=7; ++i) {
        apply_move(&B, Ruy[i]);
        print_board(&B);    
        std::cout << evaluate(&B) << std::endl;
    }

    return 0;
}

