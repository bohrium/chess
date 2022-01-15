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
struct Coordinate {
    int row, col; 
};
inline bool is_valid(Coordinate coor);
struct Board {
    Color next_to_move;
    Piece grid[8][8];
    std::vector<int> evaluation_stack;
    std::vector<int> plies_since_irreversible;
    unsigned int hash;

    // KING SAFETY TERMS
    /* We define four quadrants (0123) representing black's queenside, black's
     * kingside, white's queenside, white's kingside.  We say that a piece
     * "xray attacks" a square if, in the absence of all occluding material and
     * inhibiting pins, that piece would attack that square.  A king is less
     * safe when enemy pieces lie in its current quadrant.  A king is less safe
     * when any of the up-to-9 squares at and around it are xray attacked by
     * enemy pieces.  
     */ 
    Coordinate king_loc[2];
    int nb_pieces_by_quadrant[2][4];
    int nb_xrays[2][8][8];

    // PAWN STRUCTURE
    /*     (--- caution:   rank == 7-row   ---)
     * For us, a "weak square" (from white's perspective) is a square in ranks
     * 2,3,4 (of 01234567) and files b,c,d,e,f,g (of abcdefgh) such that no
     * friendly pawn in either of the two adjacent files has strictly lesser
     * rank.  For example, if three consecutive pawns advance, then the square
     * in front of the middle pawn's initial square becomes weak.  An "outpost"
     * for the enemy is a weak square of ours also attacked by an enemy pawn.
     */ 
    int nb_pawns_by_file[2][8];
    int most_initial_pawn_rank[2][8]; 
    bool attacked_by_pawn[2][8][8];
    bool weak_squares[2][8][8];
    bool outposts[2][8][8];
    int nb_weak_squares[2];

    // BISHOP TERMS
    int nb_pawns_by_square_parity[2][2];
    int nb_bishops_by_square_parity[2][2];

    // KNIGHT TERMS
    int nb_knights_on_weak_squares[2]; 
    int nb_knights_on_outposts[2]; 

    // ROOK TERMS
    int nb_rooks_on_semi_files[2]; 
    int nb_rooks_on_open_files[2]; 
};
Board copy_board(Board B);

extern Species init_row[];

void init_board(Board* B);

extern char letters[];
void print_board(Board const* B);
bool read_board(Board* B, char const* string);


inline Piece get_piece(Board const* B, Coordinate coor);
 
enum MoveType {
    ordinary =0,
    promotion_to_queen=1,
    extra_legal=-1,
};
struct Move { // a standard move (no promotion, en passant, or castling)
    Coordinate source, dest; 
    Piece taken;
    MoveType type; 
};
const Move unk_move = {{0,0}, {0,0}, empty_piece, MoveType::extra_legal};
bool is_capture(Move m);

void apply_move(Board* B, Move M);
void undo_move(Board* B, Move M);

void apply_null(Board* B);
void undo_null(Board* B);

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
int evaluate(Board* B);
int evaluation_difference(Board* B, Move m);

#endif//BOARD_H
