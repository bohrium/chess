#include "Board.h"
#include "Display.h"
#include <iostream>


int KING_POINTS = 100000; /* should exceed twice the total remaining value */ 
             /* p    n       b       r    q     k             */
int points[] = {100, 300+34, 300+34, 600, 1000, KING_POINTS};

/* note: pawn, knight, and rook placements are asymmetrical*/
int XX=-89,_X=-34,xx=-13,_x=-5,_o=+5,oo=+13,_O=+34,OO=+89; 

int piece_placement[][8][8] = {
    /*pawn*/ { /* push pawns and control the center */
        { 0, 0, 0, 0, 0, 0, 0, 0},
        {OO,OO,OO,OO,OO,OO,OO,OO},
        {xx,_O,_O,_O,_O,_O,_O,xx},
        {xx, 0,oo,oo,oo,oo, 0,xx},
        {xx, 0, 0,_o,_o, 0, 0,xx},
        {xx, 0, 0, 0, 0, 0, 0,xx},
        {xx, 0, 0, 0, 0, 0, 0,xx},
        { 0, 0, 0, 0, 0, 0, 0, 0}, 
    },
    /*knight*/ { /* a knight on the rim is dim*/ 
        {XX,_X,_X,_X,_X,_X,_X,XX},
        {_X,xx,_x,_x,_x,_x,xx,_X},
        {_X,_x, 0, 0, 0, 0,_x,_X},
        {_X,_x, 0,_O,_O, 0,_x,_X},
        {_X,_x, 0,oo,oo, 0,_x,_X},
        {_X,_x, 0, 0, 0, 0,_x,_X},
        {_X,xx,_x,_x,_x,_x,xx,_X},
        {XX,xx,xx,xx,xx,xx,xx,XX},
    },
    /*bishop*/ { /* bishops love long diagonals */
        {xx,xx,xx,xx,xx,xx,xx,xx},
        {xx,oo, 0, 0, 0, 0,oo,xx},
        {xx, 0,oo, 0, 0,oo, 0,xx},
        {xx, 0, 0,oo,oo, 0, 0,xx},
        {xx, 0, 0,oo,oo, 0, 0,xx},
        {xx, 0,oo, 0, 0,oo, 0,xx},
        {xx,oo, 0, 0, 0, 0,oo,xx},
        {xx,xx,xx,xx,xx,xx,xx,xx},
    },
    /*rook*/ { /* rooks love 7th ranks */
        { 0, 0, 0, 0, 0, 0, 0, 0},
        {_o,oo,oo,oo,oo,oo,oo,_o},
        { 0, 0, 0, 0, 0, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0, 0, 0}, 
    },
    /*queen*/ { /* centralize the queen */
        { 0, 0, 0, 0, 0, 0, 0, 0},
        { 0, 0, 0,_o,_o, 0, 0, 0},
        { 0, 0,_o,oo,oo,_o, 0, 0},
        { 0,_o,oo,_O,_O,oo,_o, 0},
        { 0,_o,oo,_O,_O,oo,_o, 0},
        { 0, 0,_o,oo,oo,_o, 0, 0},
        { 0, 0, 0,_o,_o, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0, 0, 0},
    },
    /*king*/ { /* centralize the king (hopefully overridden in middle game by safety factors) */
        { 0, 0, 0, 0, 0, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0, 0, 0},
        { 0, 0, 0,_o,_o, 0, 0, 0},
        { 0, 0,_o,oo,oo,_o, 0, 0},
        { 0, 0,_o,oo,oo,_o, 0, 0},
        { 0, 0, 0,_o,_o, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0, 0, 0},
    },
};

int king_tropism(Board const* B)
{
    int quad[2] = {
        quadrant_by_coor[B->king_loc[0].row][B->king_loc[0].col],
        quadrant_by_coor[B->king_loc[1].row][B->king_loc[1].col],
    };
    return 13 * (
        B->nb_pieces_by_quadrant[1][quad[0]] /* white pieces near black king */
      - B->nb_pieces_by_quadrant[0][quad[1]] /* black pieces near white king */
    );
}

int king_shelter(Board const* B)
{
    auto has_pawn = [B](Color c, Coordinate rc){
        if (!(0<=rc.col && rc.col<8)) { return 0; }
        Piece p = get_piece(B, rc);
        return p.color==c && p.species==Species::pawn ? 1 : 0;
    };
    Coordinate k0 = B->king_loc[0];
    Coordinate k1 = B->king_loc[1];
    return 13 * (
       -has_pawn(Color::black, {k0.row+1, k0.col-1}) /* black king*/
       -has_pawn(Color::black, {k0.row+1, k0.col  }) 
       -has_pawn(Color::black, {k0.row+1, k0.col+1}) 
       +has_pawn(Color::white, {k1.row-1, k1.col-1}) /* white king */
       +has_pawn(Color::white, {k1.row-1, k1.col  }) 
       +has_pawn(Color::white, {k1.row-1, k1.col+1}) 
    );
}

int bishop_adjustment(Board const* B)
{
    int net_pairs = (
        B->nb_bishops_by_square_parity[1][0] * B->nb_bishops_by_square_parity[1][1]
      - B->nb_bishops_by_square_parity[0][0] * B->nb_bishops_by_square_parity[0][1]
    );
    int net_pawnblocks = (
        (B->nb_pawns_by_square_parity[1][0] * B->nb_bishops_by_square_parity[1][0])
      + (B->nb_pawns_by_square_parity[1][1] * B->nb_bishops_by_square_parity[1][1])
      - (B->nb_pawns_by_square_parity[0][0] * B->nb_bishops_by_square_parity[0][0])
      - (B->nb_pawns_by_square_parity[0][1] * B->nb_bishops_by_square_parity[0][1])
    );
    return 34*net_pairs - 5*net_pawnblocks; 
}

/* includes knight outposts, too */
int weak_square_malus(Board const* B)
{
    int acc=0;
    for (int r=0; r!=8; ++r) {
        for (int c=0; c!=8; ++c) {
            /* WHITE's perspective: LESSER .row means MORE advanced */
            bool is_weak_white =
                (             !B->nb_pawns_by_file[1][c]   || B->least_advanced[1][c]<= r) && 
                (!(0<=c-1) || !B->nb_pawns_by_file[1][c-1] || B->least_advanced[1][c-1]<r) &&
                (!(c+1<8 ) || !B->nb_pawns_by_file[1][c+1] || B->least_advanced[1][c+1]<r);
            bool is_outpost_black = is_weak_white && (
                B->attacks_by_pawn[0][r][c] &&
                B->nb_pawns_by_file[1][c] &&
                B->least_advanced[1][c]<r 
            );
            bool knighted_black= 
                get_piece(B, {r,c}).species==Species::knight &&
                get_piece(B, {r,c}).color==Color::black;

            /* BLACK's perspective: GREATER .row means MORE advanced */
            bool is_weak_black =
                (             !B->nb_pawns_by_file[0][c]   || B->least_advanced[0][c]>= r) && 
                (!(0<=c-1) || !B->nb_pawns_by_file[0][c-1] || B->least_advanced[0][c-1]>r) &&
                (!(c+1<8 ) || !B->nb_pawns_by_file[0][c+1] || B->least_advanced[0][c+1]>r);
            bool is_outpost_white = is_weak_black && (
                B->attacks_by_pawn[1][r][c] &&
                B->nb_pawns_by_file[0][c] &&
                B->least_advanced[0][c]<r 
            );
            bool knighted_white = 
                get_piece(B, {r,c}).species==Species::knight &&
                get_piece(B, {r,c}).color==Color::white;

            acc += knighted_black ?
                (is_weak_white ? (is_outpost_black ? -89-13 : -34-5) : 0) :
                (is_weak_white ? (is_outpost_black ?    -13 :    -5) : 0);
            acc -= knighted_white ?
                (is_weak_black ? (is_outpost_white ? -89-13 : -34-5) : 0) :
                (is_weak_black ? (is_outpost_white ?    -13 :    -5) : 0);
        }
    }
    return acc;
}
int redundant_majors(Board const* B)
{
    return 34 * (- B->nb_majors[1]*(B->nb_majors[1]-1)/2
                 + B->nb_majors[0]*(B->nb_majors[0]-1)/2);
}


int rook_placement(Board const* B)
{
    auto bonus = [B](Color c, int file){
        return B->nb_rooks_by_file[c][file]*
            (B->nb_pawns_by_file[c][file]             ?  0 :
             B->nb_pawns_by_file[flip_color(c)][file] ? 13 : /* semi-open */
                                                        34); /* open */
    };
    return bonus(Color::white,0) - bonus(Color::black,0)
         + bonus(Color::white,1) - bonus(Color::black,1)
         + bonus(Color::white,2) - bonus(Color::black,2)
         + bonus(Color::white,3) - bonus(Color::black,3)
         + bonus(Color::white,4) - bonus(Color::black,4)
         + bonus(Color::white,5) - bonus(Color::black,5)
         + bonus(Color::white,6) - bonus(Color::black,6)
         + bonus(Color::white,7) - bonus(Color::black,7);
}

int pawn_connectivity(Board const* B)
{
    return 34 * (
      - (B->nb_pawns_by_file[1][0] * B->nb_pawns_by_file[1][1] ? 0 : 1)
      - (B->nb_pawns_by_file[1][1] * B->nb_pawns_by_file[1][2] ? 0 : 1)
      - (B->nb_pawns_by_file[1][2] * B->nb_pawns_by_file[1][3] ? 0 : 1)
      - (B->nb_pawns_by_file[1][3] * B->nb_pawns_by_file[1][4] ? 0 : 1)
      - (B->nb_pawns_by_file[1][4] * B->nb_pawns_by_file[1][5] ? 0 : 1)
      - (B->nb_pawns_by_file[1][5] * B->nb_pawns_by_file[1][6] ? 0 : 1)
      - (B->nb_pawns_by_file[1][6] * B->nb_pawns_by_file[1][7] ? 0 : 1)
      // 
      + (B->nb_pawns_by_file[0][0] * B->nb_pawns_by_file[0][1] ? 0 : 1)
      + (B->nb_pawns_by_file[0][1] * B->nb_pawns_by_file[0][2] ? 0 : 1)
      + (B->nb_pawns_by_file[0][2] * B->nb_pawns_by_file[0][3] ? 0 : 1)
      + (B->nb_pawns_by_file[0][3] * B->nb_pawns_by_file[0][4] ? 0 : 1)
      + (B->nb_pawns_by_file[0][4] * B->nb_pawns_by_file[0][5] ? 0 : 1)
      + (B->nb_pawns_by_file[0][5] * B->nb_pawns_by_file[0][6] ? 0 : 1)
      + (B->nb_pawns_by_file[0][6] * B->nb_pawns_by_file[0][7] ? 0 : 1)
    );        
}

int evaluate(Board* B) /* todo: constify type signature */
{
    if (B->plies_since_irreversible.back() >= NB_PLIES_TIL_DRAW ) { /* the 20 ply rule */
        return -KING_POINTS/2; /* black wins*/
    }
    return B->evaluation_stack.back()
         + king_tropism(B)
         + king_shelter(B)
         + pawn_connectivity(B)
         + bishop_adjustment(B) 
         + redundant_majors(B)
         + rook_placement(B) 
         + weak_square_malus(B);
}

int evaluation_difference(Board* B, Move m) /*TODO: constify*/ // assumes m has not yet been applied to B 
{
    int material  = 0;
    int placement = 0;
    //int safety = 0;

    Piece mover = get_piece(B, m.source);
    int sign = (mover.color==Color::white ? +1 : -1);

    /* material */ 
    if (is_capture(m)) {
        material = sign * (points[m.taken.species] +
                piece_placement[m.taken.species][m.dest.row][m.dest.col]);
    }

    /* TODO: condense control flow */
    if (m.type == promote_to_queen) {
        material += sign * (points[Species::queen]-points[Species::pawn]);
        if (mover.color==Color::white) {
            placement = sign * ( 
                piece_placement[Species::queen][m.dest.row][m.dest.col] -
                piece_placement[Species::pawn][m.source.row][m.source.col] 
            );
        } else {
            placement = sign * ( 
                piece_placement[Species::queen][7-m.dest.row][m.dest.col] -
                piece_placement[Species::pawn][7-m.source.row][m.source.col] 
            );
        }
    } else {
        /* placement */ 
        if (mover.color==Color::white) {
            placement = sign * ( 
                piece_placement[mover.species][m.dest.row][m.dest.col] -
                piece_placement[mover.species][m.source.row][m.source.col] 
            );
        } else {
            placement = sign * ( 
                piece_placement[mover.species][7-m.dest.row][m.dest.col] -
                piece_placement[mover.species][7-m.source.row][m.source.col] 
            );
        }
    }

    return material + placement;
}



//int diff_king_safety(Board* B, Move M) 
//{
//    Piece mover = get_piece(B, M.source);
//    int sign = mover.color == Color::white : +1 : -1;
//    Coordinate king_loc = B->king_loc[flip_color(mover.color)];
//
//    int net_xrays = 0;
//
//    int king_hor  = 3;
//    int king_vert = 3;
//
//    int sr = M.source.row;
//    int sc = M.source.col;
//    int dr = M.dest.row;
//    int dc = M.dest.col;
//    int kr = king_loc.row;
//    int kc = king_loc.col;
//
//    switch (mover.species) {
//        break; case Species::pawn:
//        break; case Species::knight:
//        break; case Species::bishop:
//        break; case Species::rook:
//            net_xrays = (
//                (sr == dr) ? (
//                    ()
//                ) : (
//                ) 
//            );
//        break; case Species::queen:
//        break; case Species::king:
//    }
//
//    return sign * net_xrays;
//}


//int king_safety(Board* B) 
//{
//    int nb_attackers[2][8][8]; 
//    for (int r=0; r!=8; ++r) {
//        for (int c=0; c!=8; ++c) {
//            nb_attackers[Color::black][r][c] = 0; /* black */
//            nb_attackers[Color::white][r][c] = 0; /* white */
//        } 
//    }
//    Coordinate king_locations[2];
//    for (int r=0; r!=8; ++r) {
//        for (int c=0; c!=8; ++c) {
//            Piece p = get_piece(B, {r,c});
//            if (p.species == Species::empty_species) { continue; }
//            if (p.species == Species::king) { king_locations[p.color] = {r,c}; }
//            switch (p.species) {
//                case Species::knight:
//                    for (int dr=-2; dr!=3; ++dr) {
//                        for (int dc=-2; dc!=3; ++dc) {
//                            if (dr*dr + dc*dc != 2*2 + 1*1) { continue; }  
//                            if (! (0<=r+dr && r+dr<8 && 0<=c+dc && c+dc<8)) { continue; }  
//                            nb_attackers[p.color][r+dr][c+dc] += 1; 
//                        }
//                    }
//                    break;
//                case Species::bishop:
//                    for (int dr=-1; dr!=2; ++dr) {
//                        for (int dc=-1; dc!=2; ++dc) {
//                            if (dr*dr + dc*dc != 1*1 + 1*1) { continue; }
//                            for (int t=1; t!=8; ++t) {
//                                if (! (0<=r+t*dr && r+t*dr<8 && 0<=c+t*dc && c+t*dc<8)) { break; }  
//                                nb_attackers[p.color][r+t*dr][c+t*dc] += 1; 
//                            }
//                        }
//                    }
//                    break;
//                case Species::rook:
//                    for (int dr=-1; dr!=2; ++dr) {
//                        for (int dc=-1; dc!=2; ++dc) {
//                            if (dr*dr + dc*dc != 1*1) { continue; }
//                            for (int t=1; t!=8; ++t) {
//                                if (! (0<=r+t*dr && r+t*dr<8 && 0<=c+t*dc && c+t*dc<8)) { break; }  
//                                nb_attackers[p.color][r+t*dr][c+t*dc] += 1; 
//                            }
//                        }
//                    }
//                    break;
//                case Species::queen:
//                    for (int dr=-1; dr!=2; ++dr) {
//                        for (int dc=-1; dc!=2; ++dc) {
//                            if (dr*dr + dc*dc == 0) { continue; }
//                            for (int t=1; t!=8; ++t) {
//                                if (! (0<=r+t*dr && r+t*dr<8 && 0<=c+t*dc && c+t*dc<8)) { break; }  
//                                nb_attackers[p.color][r+t*dr][c+t*dc] += 1; 
//                            }
//                        }
//                    }
//                    break;
//                default: break;
//            }
//        }
//    }
//    int net_xrays = 0;
//    for (int color = 0; color != 2; ++color) {
//        int r = king_locations[color].row;
//        int c = king_locations[color].col;
//        for (int dr=-1; dr!=2; ++dr) {
//            for (int dc=-1; dc!=2; ++dc) {
//                //if (dr*dr + dc*dc == 0) { continue; }
//                if (! (0<=r+dr && r+dr<8 && 0<=c+dc && c+dc<8)) { continue; }  
//                net_xray += (color==Color::black ? +1 : -1) * nb_attackers[1-color][r+dr][c+dc];
//            }
//        }
//    }
//    return net_xrays;
//}



//int knight_outpost(Board const* B)
//{
//    /* NB: every outpost is also a weak square */
//    //return (
//    //    25 * (B->nb_knights_on_weak_squares[1]-B->nb_knights_on_weak_squares[0])
//    //  + (50-25) * (B->nb_knights_on_outposts[1]-B->nb_knights_on_outposts[0])
//    //);
//    return 0;
//}


