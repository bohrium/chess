#ifndef BOARD_H
#define BOARD_H

#include "Pieces.h"
#include <vector>

#define NB_PLIES_TIL_DRAW 20

struct DisaggregatedScore {
    int material;
    int king_safety;
    int pawn_structure;
    int cozy_squares;
};

struct Board {
    Color next_to_move;
    Piece grid[8][8];
    std::vector<int> plies_since_irreversible;
    unsigned int hash;

    std::vector<DisaggregatedScore> evaluation_stack;

    // KING SAFETY TERMS
    /* We define four quintants (0123) representing black's queenside, black's
     * kingside, white's queenside, white's kingside.  We say that a piece
     * "xray attacks" a square if, in the absence of all occluding material and
     * inhibiting pins, that piece would attack that square.  A king is less
     * safe when enemy pieces lie in its current quintant.  A king is less safe
     * when any of the up-to-9 squares at and around it are xray attacked by
     * enemy pieces.  
     */ 
    std::vector<Coordinate> king_locs[2]; /* TODO: swap axes */
    int nb_pieces_by_quintant[2][4];
    int nb_xrays[2][8][8];

    // PAWN STRUCTURE
    /*     (--- CAUTION:   rank == 7-row   ---)
     * For us, a "weak square" (for white) is a square such that {no friendly
     * pawn in either of the two adjacent files has strictly lesser rank} AND
     * {no friendly pawn of the same file has lesser-or-equal rank}.  An
     * "outpost" for the enemy is a weak square of ours also attacked by an
     * enemy pawn.
     */ 
    int nb_pawns_by_file[2][8];
    int least_advanced[2][8]; 
    int attacks_by_pawn[2][8][8];
    //bool weak_squares[2][8][8];
    //bool outposts[2][8][8];
    //int nb_weak_squares[2];

    // BISHOP TERMS
    int nb_pawns_by_parity[2][2];
    int nb_bishops_by_parity[2][2];

    // KNIGHT TERMS
    int nb_knights_on_weak_squares[2]; 
    int nb_knights_on_outposts[2]; 

    // MAJOR TERMS
    int nb_rooks_by_file[2][8];
    int nb_majors[2];
};
Board copy_board(Board B);

Piece get_piece(Board const* B, Coordinate coor);
bool kronecker_piece(Board const* B, Coordinate coor, Piece p);

extern Species init_row[];

void init_board(Board* B);

void print_board(Board const* B);
bool read_board(Board* B, char const* string);

// A chess position can have at most 332 possible next moves:
// Consider a board with 9 queens, 2 rooks, 2 bishops, 2 knights, 1 king
#define MAX_NB_MOVES (9*4*7 + 2*2*7 + 2*2*7 + 2*8 + 1*8) 
struct MoveList {
    int length;
    Move moves[MAX_NB_MOVES];
};
void print_move(Board const* B, Move M);

void print_movelist(Board const* B, MoveList* ML);


void change_piece(Board* B, Coordinate rc, Piece p, bool is_add);
void add_eval_diff(Board* B, Coordinate rc, Piece p, bool is_add);

void apply_move(Board* B, Move M);
void undo_move(Board* B, Move M);

void apply_null(Board* B);
void undo_null(Board* B);

void generate_pawn_moves  (Board const* B, MoveList* ML, Coordinate source);
void generate_knight_moves(Board const* B, MoveList* ML, Coordinate source);
void generate_bishop_moves(Board const* B, MoveList* ML, Coordinate source);
void generate_rook_moves  (Board const* B, MoveList* ML, Coordinate source);
void generate_queen_moves (Board const* B, MoveList* ML, Coordinate source);
void generate_king_moves  (Board const* B, MoveList* ML, Coordinate source);
void generate_moves(Board const* B, MoveList* ML);

extern int points[];
extern int KING_POINTS;
int evaluate(Board* B);
void add_evaluation_difference(Board* B, Move m);

//int king_tropism(Board const* B);
//int king_shelter(Board const* B);
//int pawn_connectivity(Board const* B);
//int bishop_adjustment(Board const* B); 
//int redundant_majors(Board const* B); 
////int knight_outpost(Board const* B); 
//int opened_rook(Board const* B);
//int weak_square_malus(Board const* B); 

extern unsigned int hash_by_piece[3][7];
extern unsigned int hash_by_square[8][8];
extern int quintant_by_coor[8][8];

/* REQUIRES: assumes nb_pawns_by_file is correct! */
void update_least_advanced(Board* B, Color side, int col);

#endif//BOARD_H
