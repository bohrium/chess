#include "Board.h"
#include "Search.h"
#include <iostream>

void print_best_move_of(char const* board_string)
{
    Board B;
    read_board(&B, board_string);
    Move m = get_best_move(&B, 4);
    print_move(&B, m); std::cout << std::endl;
}

int main(int argc, char** argv)
{
    std::cout << "Welcome!" << std::endl; 

    Board B;
    /* TODO: test mating ability using following 8 examples */

    /*******************
     * CHECKMATES in 1 *
     *******************/

    /* (Rc8#) --- Back Rank Mate */
    char backrank[] =
        " White to move       \n"
        "    a b c d e f g h  \n"
        " 8 | | | | | | |k| | \n"
        " 7 | | | | | |p|p|p| \n"
        " 6 | | | | | | | | | \n"
        " 5 | | | | | | | | | \n"
        " 4 | | | | | | | | | \n"
        " 3 | | | | | | | | | \n"
        " 2 | | |R| |K| | | | \n"
        " 1 | | | | | | | | | \n"
        "    ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾  \n";

    /* (Qxf7#) --- Scholar's Mate */
    char scholars[] =
        " White to move       \n"
        "    a b c d e f g h  \n"
        " 8 |r| |b|q|k|b|n|r| \n"
        " 7 |p|p|p|p| |p|p|p| \n"
        " 6 | | |n| | |n| | | \n"
        " 5 | | | | |p| | |Q| \n"
        " 4 | | |B| |P| | | | \n"
        " 3 | | | | | | | | | \n"
        " 2 |P|P|P|P| |P|P|P| \n"
        " 1 |R|N|B| |K| |N|R| \n"
        "    ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾  \n";

    /* (Nf2#) --- Smothered Mate */
    char smothered[] =
        " Black to move       \n"
        "    a b c d e f g h  \n"
        " 8 | | | | | | | |k| \n"
        " 7 |p|p|p| | | |p|p| \n"
        " 6 | | | |R| | | | | \n"
        " 5 | | | | | | | | | \n"
        " 4 | | | | |n| | | | \n"
        " 3 | | | | | | | | | \n"
        " 2 |P|P|P| | | |P|P| \n"
        " 1 | | | | | | |R|K| \n"
        "    ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾  \n";

    /*******************
     * CHECKMATES in 2 *
     *******************/

    /* (Bd7+) --- Evergreen Game */
    char evergreen[] =
        " White to move       \n"
        "    a b c d e f g h  \n"
        " 8 | |r| | | | |r| | \n"
        " 7 |p|b|p|k|n|p| |p| \n"
        " 6 | |b| | | |P| | | \n"
        " 5 | | | | | | | | | \n"
        " 4 | | | | | | | | | \n"
        " 3 |B| |P|B| |q| | | \n"
        " 2 |P| | | | |P|P|P| \n"
        " 1 | | | |R| | |K| | \n"
        "    ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾  \n";

    /* (Qf6+) --- Immortal Game */
    char immortal[] =
        " White to move       \n"
        "    a b c d e f g h  \n"
        " 8 |r| |b|k| | |n|r| \n"
        " 7 |p| | |p| |p|N|p| \n"
        " 6 |n| | |B| | | | | \n"
        " 5 | |p| |N|P| | |P| \n"
        " 4 | | | | | | |P| | \n"
        " 3 | | | |P| |Q| | | \n"
        " 2 |P| |P| |K| | | | \n"
        " 1 |q| | | | | | | | \n"
        "    ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾  \n";

    /* (Nc3+) --- Game of the Century */
    char century[] =
        " Black to move       \n"
        "    a b c d e f g h  \n"
        " 8 | |Q| | | | | | | \n"
        " 7 | | | | | |p|k| | \n"
        " 6 | | |p| | | |p| | \n"
        " 5 | |p| | | | | |p| \n"
        " 4 | |b| | | | | |P| \n"
        " 3 | |b| | | | | | | \n"
        " 2 |r| | | |n| |P| | \n"
        " 1 | |K| | | | | | | \n"
        "    ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾  \n";

    print_best_move_of(backrank);  // Rc8 
    print_best_move_of(scholars);  // Qxf7
    print_best_move_of(smothered); // Nf2
    print_best_move_of(evergreen); // Bd7
    print_best_move_of(immortal);  // Qf6
    print_best_move_of(century);   // Nc3
   
    return 0;
}

