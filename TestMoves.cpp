#include "Board.h"
#include <iostream>

int main(int argc, char** argv)
{
    std::cout << "Welcome!" << std::endl; 

    Board B;
    init_board(&B);
    MoveList ML;

    /* Print whites's opening choices */
    print_board(&B);    
    generate_moves(&B, &ML);
    std::cout << ML.length << " moves" << std::endl;
    print_movelist(&ML);

    /* Ruy Lopez, Exchange Variation */
    apply_move(&B, {{6,4}, {4,4}, empty_piece}); // e4
    apply_move(&B, {{1,4}, {3,4}, empty_piece}); // e5
    apply_move(&B, {{7,6}, {5,5}, empty_piece}); // Nf3
    apply_move(&B, {{0,1}, {2,2}, empty_piece}); // Nc6
    apply_move(&B, {{7,5}, {3,1}, empty_piece}); // Bb5
    apply_move(&B, {{1,0}, {2,0}, empty_piece}); // a6
    apply_move(&B, {{3,1}, {2,2}, {Color::black, Species::knight}}); // Bxc6

    /* Print black's responses to RLEV */
    print_board(&B);
    generate_moves(&B, &ML);
    std::cout << ML.length << " moves" << std::endl;
    print_movelist(&ML);

    return 0;
}

