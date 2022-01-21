#include "Board.h"
#include "Display.h"
#include <iostream>

Board copy_board(Board B)
{
    Board rtrn = B; 
    rtrn.evaluation_stack = B.evaluation_stack; 
    rtrn.plies_since_irreversible = B.plies_since_irreversible; 
    return rtrn;
}

Species init_row[] = {
    Species::rook,
    Species::knight,
    Species::bishop,
    Species::queen,
    Species::king,
    Species::bishop,
    Species::knight,
    Species::rook
};
void init_board(Board* B)
{
    B->next_to_move = Color::white;
    B->plies_since_irreversible.push_back(0);

    B->nb_majors[0] = 3;
    B->nb_majors[1] = 3;

    for (int r=0; r!=8; ++r) {
        for (int c=0; c!=8; ++c) {
            B->grid[r][c] = empty_piece;
            B->attacks_by_pawn[Color::black][r][c] = 0;
            B->attacks_by_pawn[Color::white][r][c] = 0;
        }
    }

    for (int c=0; c!=8; ++c) {
        B->grid[0][c] = {Color::black, init_row[c]};
        B->grid[1][c] = {Color::black, Species::pawn};
        B->attacks_by_pawn[Color::black][2][c] = c==0||c==7 ? 1 : 2;
        B->attacks_by_pawn[Color::white][5][c] = c==0||c==7 ? 1 : 2;
        B->grid[6][c] = {Color::white, Species::pawn};
        B->grid[7][c] = {Color::white, init_row[c]};
    }

    B->evaluation_stack.push_back(0); /* initial evaluation */ 
    B->hash = 0;

    B->king_loc[0] = {0, 4};
    B->king_loc[1] = {7, 4};

    for (int color=0; color!=2; ++color) {
        for (int c=0; c!=8; ++c) {
            B->nb_pawns_by_file[color][c] = 1;
            B->nb_rooks_by_file[color][c] = 0;
            B->least_advanced[color][c] = color==Color::white ? 6 : 1;
        }
        B->nb_rooks_by_file[color][0] = 1;
        B->nb_rooks_by_file[color][7] = 1;
        for (int q=0; q!=4; ++q) {
            B->nb_pieces_by_quadrant[color][q] = (q&2)^(color<<1) ? 0 : 8;
        }
        for (int p=0; p!=2; ++p) {
            B->nb_pawns_by_square_parity[color][p] = 4;
            B->nb_bishops_by_square_parity[color][p] = 1;
        }
    }
}

void print_board(Board const* B)
{
    auto new_line = [](){
        std::cout << "                              " << std::endl << "\t";
    };

    new_line();
    std::cout << "   a b c d e f g h ";
    new_line();
    for (int r=0; r!=8; ++r) {
        std::cout << 8-r << " |";
        for (int c=0; c!=8; ++c) {
            char l = species_names[B->grid[r][c].species];
            if (B->grid[r][c].color==Color::black) {
                l += 'a'-'A'; // lowercase
                std::cout << ANSI_CYAN; /* black pieces are cyan*/
            } else {
                std::cout << ANSI_MAGENTA; /* white pieces are magenta */
            }
            std::cout << l;
            std::cout << ANSI_YELLOW; /* yellow */
            std::cout << "|";
        }
        new_line();
    }
    std::cout << "   ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾ ";
    new_line();

    if (B->next_to_move == Color::white) {
        std::cout << ANSI_MAGENTA << "White" << ANSI_YELLOW << " to move";
    } else {                                                
        std::cout << ANSI_CYAN << "Black" << ANSI_YELLOW << " to move";
    }
    std::cout << "; ";
    std::cout << ANSI_BLUE << B->plies_since_irreversible.back() << ANSI_YELLOW << " plies";
    new_line();
    std::cout << ANSI_BLUE << B->hash << ANSI_YELLOW;
    new_line();
    std::cout << ANSI_YELLOW <<  "mtr" << ANSI_RED << B->evaluation_stack.back() + bishop_adjustment(B) + redundant_majors(B)
              << ANSI_YELLOW << " kng" << ANSI_RED << king_tropism(B) + king_shelter(B)
              << ANSI_YELLOW << " pwn" << ANSI_RED << pawn_connectivity(B) + weak_square_malus(B)
              << ANSI_YELLOW << " out" << ANSI_RED << rook_placement(B)
              << ANSI_YELLOW << " sqr" << ANSI_RED << 0 
              << ANSI_YELLOW;
    new_line();
} 
/*
        White to move
           a b c d e f g h
        8 | | | | | | |k| |
        7 | | | | | |p|p|p|
        6 | | | | | | | | |
        5 | | | | | | | | |
        4 | | | | | | | | |
        3 | | | | | | | | |
        2 | | |R| |K| | | |
        1 | | | | | | | | |
           ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
*/
void skip_white(char const** string) {
    while (**string==' ' || **string=='\t' || **string=='\n') { ++*string; } 
}
void to_next_line(char const** string) {
    while (**string!='\n') { ++*string; }
    ++*string;
}
bool read_board(Board* B, char const* string) /* returns TRUE if error */
{
    skip_white(&string);
    if (*string=='W') { B->next_to_move = Color::white; }
    else if (*string=='B') { B->next_to_move = Color::black; }
    else { return true; /* ERROR */ }
    to_next_line(&string); /* go to "   a b c" etc */
    for (int r=0; r!=8; ++r) {
        to_next_line(&string); /* go to "8 |r|n|b" etc */
        skip_white(&string);
        string += 1; /* skip "8" etc */ 
        for (int c=0; c!=8; ++c) {
            string += 2; /*skip " |" etc */ 
            if (*string==' ') {
                B->grid[r][c] = empty_piece;
                continue;
            }  
            Color color = (*string<'a') ? Color::white : Color::black;
            Species species;
            switch ((char)(*string + ((*string<'a') ? 0 : 'A'-'a'))) {
            case 'P': species = Species::pawn;   break;
            case 'N': species = Species::knight; break;
            case 'B': species = Species::bishop; break;
            case 'R': species = Species::rook;   break;
            case 'Q': species = Species::queen;  break;
            case 'K': species = Species::king;   break;
            }
            B->grid[r][c] = {color, species}; 
        }
    } 
    /* TODO: compute hash, plies_since_irreversible!! (and other associated precomputed vals!)  */
    return false;
}


Piece get_piece(Board const* B, Coordinate coor)
{
    return B->grid[coor.row][coor.col]; 
} 


