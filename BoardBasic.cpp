#include "Board.h"
#include "Helpers.h"
#include <iostream>
#include <iomanip>

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

    B->evaluation_stack.push_back({0,0,0,0}); /* initial evaluation */ 
    B->hash = 0;

    B->king_locs[0].push_back({0, 4});
    B->king_locs[1].push_back({7, 4});

    for (int side=0; side!=2; ++side) {
        for (int c=0; c!=8; ++c) {
            B->nb_pawns_by_file[side][c] = 1;
            B->nb_rooks_by_file[side][c] = 0;
            B->least_advanced[side][c] = side==Color::white ? 6 : 1;
        }
        B->nb_rooks_by_file[side][0] = 1;
        B->nb_rooks_by_file[side][7] = 1;
        for (int p=0; p!=2; ++p) {
            B->nb_pawns_by_parity[side][p] = 4;
            B->nb_bishops_by_parity[side][p] = 1;
        }
    }
    int const quint_counts[2][5] = {{4,3,0,0,0}, {0,0,4,3,0}};
    for (int side=0; side!=2; ++side) {
        for (int q=0; q!=5; ++q) {
            B->nb_pieces_by_quintant[side][q] = quint_counts[side][q];
        }
    }

    int const xrays[3][8] = {{ 0, 1, 1, 0, 1, 0, 1, 0}, /* king doesn't count as attacker (should we change this ??) */
                             { 1, 1, 1, 3, 3, 1, 1, 1},
                             { 1, 0, 1, 0, 0, 1, 0, 1}};
    for (int c=0; c!=8; ++c) {
        for (int r=0; r!=8; ++r) {
            for (int side=0; side!=2; ++side) {
                B->nb_xrays[side][r][c] = 0;
            }
        }
        B->nb_xrays[0][0][c] = xrays[0][c];
        B->nb_xrays[0][1][c] = xrays[1][c];
        B->nb_xrays[0][2][c] = xrays[2][c];
        B->nb_xrays[1][5][c] = xrays[2][c];
        B->nb_xrays[1][6][c] = xrays[1][c];
        B->nb_xrays[1][7][c] = xrays[0][c];
    }
    B->nb_king_attacks_near[0] = 0;
    B->nb_king_attacks_near[1] = 0;
}

int const height = 4;
int const width = 6;
char const icons[height][7][width] = {
    {"     ", " _^  ", "  o  ", "     ", "!_|_/", "!_+_/","     "},
    {"  o  ", "/* ! ", " (_) ", "|_|_|", " (_) ", "_) (_","     "},
    {" ( ) ", " / | ", "`| |`", " | | ", " / ! ", " / ! ","     "},
    {" / ! ", "|  !_", "_/ !_", "|___|", "/___!", "/___!","     "},
};


void print_board_fancy(Board const* B)
{
    DisaggregatedScore ds = B->evaluation_stack.back(); 
    bool is_white = (B->next_to_move == Color::white);

    for (int rr=0; rr!=8*height; ++rr) {
        for (int k=0; k!=110; ++k) { std::cout << " "; }
        std::cout << "\33[110D";
        for (int c=0; c!=8; ++c) {
            Piece p = B->grid[rr/height][c];
            //std::cout << ((rr%height==0) ? "` " : "  ");
            if (rr%height==0) {
                std::cout << (rr==0 ? "abcdefgh"[c] : '`');
                std::cout << (c==0  ? "87654321"[rr/height] : ' ');
            } else {
                std::cout << "  ";
            }
            //if (rr%height==0) { std::cout << "abcdefgh"[c] << (8-rr/height); } 
            //else              { std::cout << "  ";                           } 
            std::cout << ((p.color==Color::white) ? ANSI_MAGENTA : ANSI_CYAN); 
            for (int x=0; x!=width; ++x) {
                char c = icons[rr%height][p.species][x];
                std::cout << (c=='!' ? '\\' : c);
            }
            std::cout << ANSI_YELLOW;
            std::cout << (((c==7||(rr/height)==7) && rr%height==height-1) ? " ." : "  ");
            //std::cout << "  ";
        }
        auto hued_digit = [](char const* ansi_hue, int nb) {
            if (!nb) { std::cout << ansi_hue << "."; }
            else     { std::cout << ansi_hue << nb; }
        };
        auto comp_sq_counts = [hued_digit](int const arr[2][8][8], int row) {
            for(int k=0; k!=8; ++k) {
                hued_digit(ANSI_CYAN,    arr[0][row][k]);
                hued_digit(ANSI_MAGENTA, arr[1][row][k]);
                std::cout << " ";
            }
        };

        std::cout << "      ";
        switch (rr) {
        break; case  0: std::cout << "turn = " << (is_white?ANSI_MAGENTA:ANSI_CYAN) << (is_white?"White":"Black");
        break; case  1: std::cout << "plies = " << ANSI_BLUE << B->plies_since_irreversible.back();
        break; case  2: std::cout << "hash = " << ANSI_BLUE << B->hash;
        break; case  3: std::cout << "mat = " << ANSI_RED << std::setw(4) << std::right << std::showpos << ds.material;
        break; case  4: std::cout << "kng = " << ANSI_RED << std::setw(4) << std::right << std::showpos << ds.king_safety + 34 * ( B->nb_king_attacks_near[0]-B->nb_king_attacks_near[1]);
        break; case  5: std::cout << "pwn = " << ANSI_RED << std::setw(4) << std::right << std::showpos << ds.pawn_structure;
        break; case  6: std::cout << "sqr = " << ANSI_RED << std::setw(4) << std::right << std::showpos << ds.cozy_squares;
        break; case  7: comp_sq_counts(B->nb_xrays, 0);
        break; case  8: comp_sq_counts(B->nb_xrays, 1);
        break; case  9: comp_sq_counts(B->nb_xrays, 2);
        break; case 10: comp_sq_counts(B->nb_xrays, 3);
        break; case 11: comp_sq_counts(B->nb_xrays, 4);
        break; case 12: comp_sq_counts(B->nb_xrays, 5);
        break; case 13: comp_sq_counts(B->nb_xrays, 6);
        break; case 14: comp_sq_counts(B->nb_xrays, 7);
        break; case 15:
        break; case 16: comp_sq_counts(B->attacks_by_pawn, 0);
        break; case 17: comp_sq_counts(B->attacks_by_pawn, 1);
        break; case 18: comp_sq_counts(B->attacks_by_pawn, 2);
        break; case 19: comp_sq_counts(B->attacks_by_pawn, 3);
        break; case 20: comp_sq_counts(B->attacks_by_pawn, 4);
        break; case 21: comp_sq_counts(B->attacks_by_pawn, 5);
        break; case 22: comp_sq_counts(B->attacks_by_pawn, 6);
        break; case 23: comp_sq_counts(B->attacks_by_pawn, 7);
        }
        std::cout << std::noshowpos << ANSI_YELLOW << std::endl;
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
    DisaggregatedScore ds = B->evaluation_stack.back(); 
    std::cout << ANSI_YELLOW <<  "mtr" << ANSI_RED << ds.material
              << ANSI_YELLOW << " kng" << ANSI_RED << ds.king_safety 
              << ANSI_YELLOW << " pwn" << ANSI_RED << ds.pawn_structure
              << ANSI_YELLOW << " sqr" << ANSI_RED << ds.cozy_squares
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


Piece get_piece(Board const* B, Coordinate rc)
{
    return B->grid[rc.row][rc.col]; 
} 

bool kronecker_piece(Board const* B, Coordinate rc, Piece p)
{
    return KRON(piece_equals(get_piece(B, rc), p));
}

