#include "Board.h"
#include "Search.h"
#include <iostream>

int main(int argc, char** argv)
{
    std::cout << "Welcome!" << std::endl; 

    Board B;
    /* TODO: test mating ability using following 8 examples */

    /* Checkmate in 1 (Rc8#) Back Rank Mate */
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

    read_board(&B, backrank);
    print_board(&B);

    /* Checkmate in 1 (Qxf7#) --- Scholar's Mate */
/*
        White to move
           a b c d e f g h
        8 |r| |b|q|k|b|n|r|
        7 |p|p|p|p| |p|p|p|
        6 | | |n| | |n| | |
        5 | | | | |p| | |Q|
        4 | | |B| |P| | | |
        3 | | | | | | | | |
        2 |P|P|P|P| |P|P|P|
        1 |R|N|B| |K| |N|R|
           ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
*/

    /* Checkmate in 1 (Nf2#) --- Smothered Mate */
/*
        Black to move
           a b c d e f g h
        8 | | | | | | | |k|
        7 |p|p|p| | | |p|p|
        6 | | | |R| | | | |
        5 | | | | | | | | |
        4 | | | | |n| | | |
        3 | | | | | | | | |
        2 |P|P|P| | | |P|P|
        1 | | | | | | |R|K|
           ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
*/

    /* Checkmate in 2 (Bd7+) --- Evergreen Game */
/*
        White to move
           a b c d e f g h
        8 | |r| | |k| |r| |
        7 |p|b|p| | |p| |p|
        6 | |b| | | |P| | |
        5 | | | | | |B| | |
        4 | | | | | | | | |
        3 |B| |p| | |Q| | |
        2 |p| | | | |p|p|p|
        1 | | | |R| | |K| |
           ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
*/

    /* https://thechessworld.com/articles/problems/3-most-tricky-mate-in-1-positions-ever */

    /* Checkmate in 2 (Qf6+) --- Immortal Game */
/*
        White to move
           a b c d e f g h
        8 |r| |b|k| | |n|r|
        7 |p| | |p| |p|N|p|
        6 |n| | |B| | | | |
        5 | |p| |N|P| | |P|
        4 | | | | | | |P| |
        3 | | | |P| |Q| | |
        2 |P| |P| |K| | | |
        1 |q| | | | | | | |
           ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
*/

    /* Checkmate in 2 (Nc3+) --- Game of the Century */
/*
        Black to move
           a b c d e f g h
        8 | |Q| | | | | | |
        7 | | | | | |p|k| |
        6 | | |p| | | |p| |
        5 | |p| | | | | |p|
        4 | |b| | | | | |P|
        3 | |b| | | | | | |
        2 |r| | | |n| |P| |
        1 | |K| | | | | | |
           ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
*/

    /* Checkmate in 2 (Nc3+) --- The Opera Game (black version) */
   
    return 0;
}

