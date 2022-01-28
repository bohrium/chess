#include "Board.h"
#include "Helpers.h"
#include <iostream>


//Color flip_color(Color c)
//{
//    return c==Color::white ? Color::black : Color::white; 
//}

//bool is_valid(Coordinate coor)
//{
//    return (0<=coor.row && coor.row<8) &&
//           (0<=coor.col && coor.col<8); 
//}

//int parity(Coordinate rc)
//{
//    return (rc.row + rc.col)%2;
//}

//bool piece_equals(Piece p, Piece q)
//{
//    return p.color==q.color && p.species==q.species;
//}

//bool is_capture(Move m)
//{
//    return m.taken.species != Species::empty_species;
//}

//bool is_irreversible(Move m, Piece mover)
//{
//    return is_capture(m) || mover.species==Species::pawn;
//}

