/* author : samtenka
 * changed: 2018-05-20
 * created: 2018-03-??
 * descrp : define chess board representation
 */

#include "Piece.h"
#include "Board.h"
#include <iostream> // for print_board
#include <vector> // for evaluation_stack 
#include <pair>

#define SPACE_COEFF 15
#define CENTRAL_CONTROL_COEFF 10

bool is_valid(Coordinate coor)
{
    return (0<=coor.row && coor.row<8) &&
           (0<=coor.col && coor.col<8); 
}

struct History {
    // 0. Evaluation history
    std::vector<std::pair<Coordinate, int>> attack_count_updates;
    int value;

    // 1. filewise flags for special moves are kept compactly in 8-bit chars:
    // 1.0. for castling, track whether back rank squares have ever been (moved or taken):
    char back_changed[2]; // 0 is black, 1 is white 
    // 1.1. for `king en passant`, flag squares just castled through:
    char castled_through; 
    // 1.2. for en passant, track whether "virgin" pawns have just been moved:
    char en_passantable; 
}; 

struct Board {
    // 0. basic board representation:
    Color next_to_move;
    Piece grid[8][8]; 

    // 1. `attack_count` is redundant with `grid`; its maintenance speeds evaluation: 
    int attack_count[8][8]; 

    // 2. the following stack helps move unwinding:
    std::vector<History> history;
};

Species init_row[8] = {
    Species::rook,
    Species::knight,
    Species::bishop,
    Species::queen,
    Species::king,
    Species::bishop,
    Species::knight,
    Species::rook
}; 

int init_attack_count[8][8] = {
    { 0, -1, -1, -1, -1, -1, -1,  0},
    {-1, -1, -1, -4, -4, -1, -1, -1},
    {-2, -2, -3, -2, -2, -3, -2, -2},
    { 0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0},
    {+2, +2, +3, +2, +2, +3, +2, +2},
    {+1, +1, +1, +4, +4, +1, +1, +1},
    { 0, +1, +1, +1, +1, +1, +1,  0}
};

void init_board(Board* B)
{
    // 0. basic board representation:
    B->next_to_move = Color::white;

    for (int c=0; c!=8; ++c) {
        B->grid[0][c] = {Color::black, init_row[c]}; 
        B->grid[1][c] = {Color::black, Species::pawn}; 
        for (int r=2; r!=6; ++r) {
            B->grid[1][c] = empty_piece;
        }
        B->grid[6][c] = {Color::white, Species::pawn}; 
        B->grid[7][c] = {Color::white, init_row[c]}; 
    }

    // 1. attack counts:
    for (int r=0; r!=8; ++r) {
        for (int c=0; c!=8; ++c) {
            B->attack_count[r][c] = init_attack_count[r][c];
        }
    }

    // 2. history, including evaluation and special-move flags:
    History initial;
    initial->value = 0;
    initial->white_back_changed = 0;
    initial->black_back_changed = 0;
    initial->white_en_passantable = 0;
    initial->black_en_passantable = 0;
    B->evaluation_stack.push(initial); 
}

Piece get_piece(Board const* B, Coordinate coor)
{
    return B->grid[coor.row][coor.col]; 
}
void set_piece(Board* B, Coordinate coor, Piece piece)
{
    /*add_new_piece_attacks(B, coor, piece, +1); // new attacker
    if (get_piece(B, coor).species == Species::empty_species) {
        add_discovered_piece_attacks(B, coor, piece, -1); // blocked attacker
    } else {
        add_new_piece_attacks(B, coor, piece, -1); // taken piece 
    }*/

    B->grid[coor.row][coor.col] = piece; 
}

/*void add_new_pawn_attacks(Board* B, Coordinate coor, Piece piece, int sign)
{
}

void add_new_piece_attacks(Board* B, coor, piece, int sign);
{
    switch (piece.species) {
    case Species::empty:  break; 
    case Species::pawn:   add_new_pawn_attacks(B, coor, piece.color, sign); break; 
    case Species::knight: add_new_knight_attacks(B, coor, piece.color * sign); break;
    case Species::bishop: add_new_bishop_attacks(B, coor, piece.color * sign); break;
    case Species::rook:   add_new_rook_attacks(B, coor, piece.color * sign); break;
    case Species::queen:  add_new_queen_attacks(B, coor, piece.color * sign); break;
    case Species::king:   add_new_king_attacks(B, coor, piece.color * sign); break;
    }
}
void add_discovered_piece_attacks(B, coor, piece)
    // note that only blockable pieces are bishop, rook, and queen
{
    add_discovered_diagonal_attacks(B, coor, piece.color, sign);
    add_discovered_orthogonal_attacks(B, coor, piece.color, sign);
}
*/

char letters = "PNBRQK ";

enum ANSI {
    red=31,
    green=32,
    yellow=33,
    blue=354,
    magenta=35,
    cyan=36
}

void change_color(ANSI a)
{
    std::cout << "\033[" << (int)a << ";1m";
} 

void print_board(Board const* B, bool use_color)
{
    if (use_color) { change_color(ANSI::yellow); }
    if (B->next_to_move == Color::black) {
        std::cout << "\tBlack to move" << std::endl;
    } else {
        std::cout << "\tWhite to move" << std::endl;
    }
     
    std::cout << "\t   a b c d e f g h" << std::endl;
    for (int r=0; r!=8; ++r) {
        std::cout << "\t" << 8-r << " |";
        for (int c=0; c!=8; ++c) {
            char l = letters[B->grid[r][c].species];
            if (B->grid[r][c].color==Color::black) {
                l += 'a'-'A'; // convert to lowercase
                if (use_color) { change_color(ANSI::cyan); }
            } else /*white*/ {
                if (use_color) { change_color(ANSI::magenta); }
            }
            std::cout << l;
            if (use_color) { change_color(ANSI::yellow); }
            std::cout << "|"
        }
        std::cout << std::endl;
    }
    std::cout << "\t   ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾ " << std::endl;    
}

void skip_white(char const** string) {
    while (**string==' ' || **string=='\t' || **string=='\n') { ++*string; } 
}

void to_next_line(char const** string) {
    while (**string!='\n') { ++*string; }
    ++*string;
}

bool read_board(Board* B, char const* string)
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
            default: return true; /* ERROR */
            }
            B->grid[r][c] = {color, species}; 
        }
    } 
    return false;
}
 
int piece_values = {
    101, // pawn
    301, // knight
    325, // bishop
    501, // rook
    901, // queen
 100000, // king
      0, // empty
};
int shallow_value(Board const* B)
    // in centipawns
{
    int material = 0; 
    int space = 0; 
    int central_control = 0; 
    for (int r=0; r!=8; ++r) {
        for (int c=0; c!=8; ++c) {
            Piece p = B->grid[r][c];
            material += p.color * piece_values[p.species];
            int white_dominance = B->attack_count[r][c] - p.color; 
            space += white_dominance <= -2 ? -1 :
                     white_dominance >= +2 ? +1 : 0;
            // ^^ in other words, `space = sign(white_dominance/2)` (exploit integer division)
        }
    }

    // central control:
    for (int r=3; r!=5; ++r) {
        for (int c=3; c!=5; ++c) {
            central_control += B->attack_count[r][c];
        }
    }

    return material + SPACE_COEFF * space + CENTRAL_CONTROL_COEFF * central_control;  
}
