#ifndef BOARD_H
#define BOARD_H
#include <vector>



/************
 * A. PIECE *
 ************/

enum Color {
    black,
    white,
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



/************
 * B. BOARD *
 ************/

struct Board {
    Color next_to_move;

    Piece grid[8][8];
    // count how many times a square is attacked by pieces of given color (2) and species (5): 
    int attack_counts[8][8][2][5];

    std::vector<float> evaluation_stack;

    /* castling info */
    std::vector<bool> white_can_castle_queenside; 
    std::vector<bool> white_can_castle_kingside; 
    std::vector<bool> black_can_castle_queenside; 
    std::vector<bool> black_can_castle_kingside; 

    /*en passant info*/
    char white_en_passantable;
    char black_en_passantable;
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
 
struct Move {
    Coordinate source, dest; 
    Piece taken;

    // 0 for standard moves; else encodes promotion, en passant, or castling:
    int move_code; 
};



/***********
 * C. MOVE *
 ***********/

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



/***************
 * D. EVALUATE *
 ***************/

extern float points[];
float evaluate(Board* B);
float evaluation_difference(Board* B, Move m);

#endif//BOARD_H
