#include "Board.h"
#include <iostream>

int main(int argc, char** argv)
{
    std::cout << "Welcome!" << std::endl; 

    Board B;
    init_board(&B);
    print_board(&B);

    Move M = {{1,1}, {3,1}, empty_piece};
    apply_move(&B, M);
    print_board(&B);
    undo_move(&B, M);
    print_board(&B);

    //apply_move(&B,Move{{1,0},{0,0},empty_piece,MoveType::promote_to_queen});
    //undo_move (&B,Move{{1,0},{0,0},empty_piece,MoveType::promote_to_queen});
    //apply_move(&B,Move{{1,0},{0,0},empty_piece,MoveType::promote_to_queen});


    return 0;
}

