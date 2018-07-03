/* author : samtenka
 * changed: 2018-05-20
 * created: 2018-03-??
 * descrp : interface for chess board, including move and evaluation functionality
 */
#ifndef BOARD_H
#define BOARD_H
#include "Piece.h"



/************
 * A. BOARD *
 ************/

struct Coordinate {
    int row, col; 
};
bool is_valid(Coordinate coor);
 
struct Board;
void init_board(Board* B);
Piece get_piece(Board const* B, Coordinate coor);
void set_piece(Board const* B, Coordinate coor, Piece piece);

void print_board(Board const* B);
// input board needs to be initialized separately and *before* read_board is called:
// returns TRUE if and only if it encountered a format error:
bool read_board(Board* B, char const* string);
/* EXAMPLE OF BOARD PRINTED FORMAT (FOR print_board AND read_board):
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


/***********
 * B. MOVE *
 ***********/

struct Move;
void apply_move(Board* B, Move const* M);
void undo_move(Board* B, Move const* M);
void print_move(Board const* B, Move const* M);
struct MoveList;
void print_movelist(Board const* B, MoveList* ML);
void generate_pawn_moves  (Board const* B, MoveList* ML, Coordinate source);
void generate_knight_moves(Board const* B, MoveList* ML, Coordinate source);
void generate_bishop_moves(Board const* B, MoveList* ML, Coordinate source);
void generate_rook_moves  (Board const* B, MoveList* ML, Coordinate source);
void generate_queen_moves (Board const* B, MoveList* ML, Coordinate source);
void generate_king_moves  (Board const* B, MoveList* ML, Coordinate source);
void generate_moves(Board const* B, MoveList* ML);



/***************
 * C. EVALUATE *
 ***************/

float evaluate(Board* B);
float reevaluate(Board* B);

#endif//BOARD_H
