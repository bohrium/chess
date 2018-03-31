#ifndef BOARD_H
#define BOARD_H

#include <iostream>

enum Color {
    black,
    white,
    empty_color
};
enum Species {
    pawn,
    knight,
    bishop,
    rook,
    queen,
    king,
    empty_species
};
struct Piece {
    Color color;
    Species species;
};
const Piece empty_piece = {Color::empty_color, Species::empty_species};
struct Board {
    Color next_to_move;
    Piece grid[8][8];
};

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
    for (int c=0; c!=8; ++c) {
        B->grid[0][c] = {Color::black, init_row[c]};
        B->grid[1][c] = {Color::black, Species::pawn};
        for (int r=2; r!=6; ++r) {
            B->grid[r][c] = empty_piece;
        }
        B->grid[6][c] = {Color::white, Species::pawn};
        B->grid[7][c] = {Color::white, init_row[c]};
    }
}

char letters[] = "pnbrqk ";
void print_board(Board const* B)
{
    if (B->next_to_move == Color::white) {
        std::cout << "\tWhite to move" << std::endl;
    } else {
        std::cout << "\tBlack to move" << std::endl;
    }
     
    std::cout << "\t _______________ " << std::endl;
    for (int r=0; r!=8; ++r) {
        std::cout << "\t|";
        for (int c=0; c!=8; ++c) {
            char l = letters[B->grid[r][c].species];
            l += B->grid[r][c].color==Color::white ? 'A'-'a' : 0; // capitalize if white
            std::cout << l << "|"; 
        }
        std::cout << std::endl;
    }
    std::cout << "\t ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾ " << std::endl;
} 

struct Coordinate {
    char row, col; 
};
struct Move { // a standard move (no promotion, en passant, or castling)
    Coordinate source, dest; 
    Piece taken;
};

void apply_move(Board* B, Move M)
{
    B->next_to_move = B->next_to_move==Color::black ? Color::white : Color::black; 
    B->grid[M.dest.row][M.dest.col] = B->grid[M.source.row][M.source.col];
    B->grid[M.source.row][M.source.col] = empty_piece;
};
void undo_move(Board* B, Move M)
{
    B->next_to_move = B->next_to_move==Color::black ? Color::white : Color::black; 
    B->grid[M.source.row][M.source.col] = B->grid[M.dest.row][M.dest.col];
    B->grid[M.dest.row][M.dest.col] = M.taken;
};

#endif//BOARD_H
