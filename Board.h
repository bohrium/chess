#ifndef BOARD_H
#define BOARD_H
#include <vector>

enum Color {
    black=0, /* 0 by fiat */
    white=1, /* 1 by fiat */
    empty_color
};
Color flip_color(Color c);
enum Species {
    pawn=0,
    knight=1,
    bishop=2,
    rook=3,
    queen=4,
    king=5,
    empty_species=6
};
struct Piece {
    Color color;
    Species species;
};
const Piece empty_piece = {Color::empty_color, Species::empty_species};
struct Board {
    Color next_to_move;
    Piece grid[8][8];
    std::vector<float> evaluation_stack;
};

extern Species init_row[];

void init_board(Board* B);

extern char letters[];
void print_board(Board const* B);
bool read_board(Board* B, char const* string);

struct Coordinate {
    int row, col; 
};
inline bool is_valid(Coordinate coor);

inline Piece get_piece(Board const* B, Coordinate coor);
 
struct Move { // a standard move (no promotion, en passant, or castling)
    Coordinate source, dest; 
    Piece taken;
};

void apply_move(Board* B, Move M);
void undo_move(Board* B, Move M);

// A chess position can have at most 332 possible next moves:
// Consider a board with 9 queens, 2 rooks, 2 bishops, 2 knights, 1 king
#define MAX_NB_MOVES (9*4*7 + 2*2*7 + 2*2*7 + 2*8 + 1*8) 
struct MoveList {
    int length;
    Move moves[MAX_NB_MOVES];
};
void print_move(Board const* B, Move M);

void print_movelist(Board const* B, MoveList* ML);

void generate_pawn_moves  (Board const* B, MoveList* ML, Coordinate source);
void generate_knight_moves(Board const* B, MoveList* ML, Coordinate source);
void generate_bishop_moves(Board const* B, MoveList* ML, Coordinate source);
void generate_rook_moves  (Board const* B, MoveList* ML, Coordinate source);
void generate_queen_moves (Board const* B, MoveList* ML, Coordinate source);
void generate_king_moves  (Board const* B, MoveList* ML, Coordinate source);
void generate_moves(Board const* B, MoveList* ML);

extern int points[];
extern int KING_POINTS;
float evaluate(Board* B);
float evaluation_difference(Board* B, Move m);

#endif//BOARD_H
