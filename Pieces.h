#ifndef PIECES_H
#define PIECES_H
#include <vector>

/*=============================================================================
====  0. INTERNAL DEGREES OF FREEDOM  =========================================
=============================================================================*/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~  0.0. Color  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

enum Color {
    black=0,
    white=1,
    empty_color=2
};
inline Color flip_color(Color c)
{
    return c==Color::white ? Color::black : Color::white; 
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~  0.1. Species  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

enum Species {
    pawn=0,
    knight=1,
    bishop=2,
    rook=3,
    queen=4,
    king=5,
    empty_species=6
};
char const species_names[] = "PNBRQK ";

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~  0.2. Piece  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

struct Piece {
    Color color;
    Species species;
};
const Piece empty_piece = {Color::empty_color, Species::empty_species};
inline bool piece_equals(Piece p, Piece q)
{
    return p.color==q.color && p.species==q.species;
}

/*=============================================================================
====  1. EXTERNAL DEGREES OF FREEDOM  =========================================
=============================================================================*/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~  1.0. Coordinate  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

struct Coordinate {
    int row, col; 
};
inline bool is_valid(Coordinate coor)
{
    return (0<=coor.row && coor.row<8) &&
           (0<=coor.col && coor.col<8); 
}
inline int parity(Coordinate rc)
{
    return (rc.row + rc.col)%2;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~  1.1. Move  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

enum MoveType {
    ordinary =0,
    promote_to_queen=1,
    extra_legal=-1,
};
struct Move { // a standard move (no promotion, en passant, or castling)
    Coordinate source, dest; 
    Piece taken;
    MoveType type; 
};
inline bool is_capture(Move m)
{
    return m.taken.species != Species::empty_species;
}
inline bool is_irreversible(Move m, Piece mover)
{
    return is_capture(m) || mover.species==Species::pawn;
}

const Move unk_move = {{0,0}, {0,0}, empty_piece, MoveType::extra_legal};

#endif//PIECES_H
