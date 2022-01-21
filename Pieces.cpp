#include "Board.h"
#include "Display.h"
#include <iostream>


Color flip_color(Color c)
{
    return c==Color::white ? Color::black : Color::white; 
}

bool is_capture(Move m)
{
    return m.taken.species != Species::empty_species;
}

bool is_valid(Coordinate coor)
{
    return (0<=coor.row && coor.row<8) &&
           (0<=coor.col && coor.col<8); 
}

int parity(Coordinate rc)
{
    return (rc.row + rc.col)%2;
}


