/* author : samtenka
 * changed: 2018-05-20
 * created: 2018-03-??
 * descrp : definition of chess pieces
 */
#ifndef PIECE_H
#define PIECE_H

enum Color {
    black=-1,
    empty_color=0,
    white=+1
};
Color flip_color(Color c) 
{
    return -c;
}
int index_from_color(Color c)
{
    return (c+1)/2; 
}
int row_from_color(Color c)
{
    return 7 * index_from_color(c);
}
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

#endif//PIECE_H
