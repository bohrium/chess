#include "Board.h"
#include "Helpers.h"
#include <iostream>

// BIG TODO: hook up eval function to these parameters!

#define TO_MOVE_BONUS   13

/* material parameters */

#define KNIGHT_BONUS    34
#define BISHOP_BONUS    34
#define ROOK_BONUS      89
#define QUEEN_BONUS     89

#define BISHOP_PAIR     34
#define MAJOR_PAIR      34 /* malus */
#define ROOK_PAWN       13 /* malus */ /* TODO: ROOK-PAWN, KNIGHT-PAWN BILINEARS!! */
#define KNIGHT_PAWN      5

/* king safety parameters */

#define KING_XRAY       13 /* malus */
#define KING_PAWN       13
#define KING_TROPISM    34 /* malus */

/* pawn structure parameters */

#define DISCONNECTED    34 /* malus */
#define WEAK_SQUARE      5 /* malus */
#define OUTPOST         13 /* malus */

/* cozy squares parameters */

// TODO: hook up piece-square tables to these!
//#define KNIGHT_ON_RIM   34 /* malus */
//#define BISHOP_ON_RIM    5 /* malus */
//#define LONG_DIAGONAL   34
//#define ROOK_AS_PIG     13
//#define PAWN_ON_FLANK   13 /* malus */

#define KNIGHT_ON_POST  89
#define KNIGHT_ON_WEAK  34

#define BLOCKED_BISHOP  13 /* malus */

#define ROOK_ON_OPEN    34
#define ROOK_ON_SEMI    13



int KING_POINTS = 100000; /* should exceed twice the total remaining value */ 
int points[] = {
    100,              /*  p  */
    300+KNIGHT_BONUS, /*  n  */
    300+BISHOP_BONUS, /*  b  */
    500+ROOK_BONUS,   /*  r  */
    900+QUEEN_BONUS,  /*  q  */
    KING_POINTS       /*  k  */
};

/* note: pawn, knight, and rook placements are asymmetrical*/
int XX=-89,_X=-34,xx=-13,_x=-5,_o=+5,oo=+13,_O=+34,OO=+89; 

int piece_placement[][8][8] = {
    /*pawn*/ { /* push pawns and control the center */
        { 0, 0, 0, 0, 0, 0, 0, 0},
        {OO,OO,OO,OO,OO,OO,OO,OO},
        {xx,_O,_O,_O,_O,_O,_O,xx},
        {xx, 0,oo,oo,oo,oo, 0,xx},
        {xx, 0, 0,oo,oo, 0, 0,xx},
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
        {XX,_X,_X,_X,_X,_X,_X,XX},
    },
    /*bishop*/ { /* bishops love long diagonals */
        {XX,_X,_X,_X,_X,_X,XX,XX},
        {XX,oo, 0, 0, 0, 0,oo,XX},
        {_X, 0,oo, 0, 0,oo, 0,_X},
        {_X, 0, 0,oo,oo, 0, 0,_X},
        {_X, 0, 0,oo,oo, 0, 0,_X},
        {_X, 0,oo, 0, 0,oo, 0,_X},
        {XX,oo, 0, 0, 0, 0,oo,XX},
        {XX,XX,_X,_X,_X,_X,XX,XX},
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

int score_total(DisaggregatedScore ds) {
    return ds.material
         + ds.king_safety
         + ds.pawn_structure
         + ds.cozy_squares;
}

int evaluate(Board* B) /* todo: constify type signature */
{
    if (B->plies_since_irreversible.back() >= NB_PLIES_TIL_DRAW ) { /* the 20 ply rule */
        return -KING_POINTS/2; /* black wins*/
    }

    return score_total(B->evaluation_stack.back()) 
       + TO_MOVE_BONUS * (B->next_to_move==Color::white ? +1 : -1)
       + 34 * ( B->nb_king_attacks_near[0] 
               -B->nb_king_attacks_near[1]);
    /* TODO: incorporate king attacks into difference eval */
}


void add_eval_diff(Board* B, Coordinate rc, Piece p, bool is_add)
{
    int const sign = p.color==Color::white ? +1 : -1;
    Color const self = p.color; 
    Color const them = flip_color(p.color);
    int const r=rc.row;
    int const c=rc.col;

    /* evaluation */
    int d_material       = 0;
    int d_king_safety    = 0;
    int d_pawn_structure = 0;
    int d_cozy_squares   = 0;

    /* easy evaluation terms */
    d_material     += sign * points[p.species];
    d_cozy_squares += sign * (
        piece_placement[p.species][sign==+1 ? r : 7-r][c]
    );

    if (p.species != Species::pawn && p.species != Species::king) {
        { /* king tropism */
            int pq = quintant_from(rc);
            int kq = quintant_from(B->king_locs[them].back());
            if (kq == pq) { d_king_safety += sign * 34; }
        }
        { /* king-xrays */
            /* TODO */
        }
        { /* loose-pieces */
        }
    }

    /* update piece-square counts and pawn structure terms */
    switch (p.species) {
    break; case Species::pawn:
        { /* cramped-bishop */
            d_cozy_squares -= sign * 13 * B->nb_bishops_by_parity[self][parity(rc)];
        }
        { /* connected-pawn, opened-rook */
            if ( is_add && B->nb_pawns_by_file[self][c]==1 || /* we have just filled an empty file */
                !is_add && B->nb_pawns_by_file[self][c]==0) { /* ... or are undoing that */
                if (c != 0) { d_pawn_structure += sign * 34 * KRON(B->nb_pawns_by_file[self][c-1]); }
                if (c != 7) { d_pawn_structure += sign * 34 * KRON(B->nb_pawns_by_file[self][c+1]); }
                    d_cozy_squares += sign * B->nb_rooks_by_file[self][c] * (
                            B->nb_pawns_by_file[them][c] ?  0-13 : /* for friendly: semi-open to closed*/
                                                            0-34); /* for friendly: open to closed */
                    d_cozy_squares -= sign * B->nb_rooks_by_file[them][c] * (
                            B->nb_pawns_by_file[them][c] ?  0- 0 : /* for enemy: closed to closed */ 
                                                           13-34); /* for enemy: open to semi-open */
            }
        }
        { /* king-shelter */
            Coordinate kl = B->king_locs[self].back();
            if (r==kl.row-sign && ABS(c - kl.col)<=1) {
                d_king_safety += sign * 13;
            }
        }
        { /* weak-squares, posted-knight */ /* TODO */ }
        { /* king-xrays */ /* TODO */ }
        { /* loose-pieces */ /* TODO */ }
    break; case Species::knight:
        { /* posted-knight */ /* TODO */ }
    break; case Species::bishop:
        { /* cramped-bishop */
            d_cozy_squares -= sign * 13 * B->nb_pawns_by_parity[self][parity(rc)];
        }
        { /* bishop-pair */
            d_material += sign * 34 * B->nb_bishops_by_parity[self][1-parity(rc)];
        }
    break; case Species::rook:
        { /* opened-rook */
            d_cozy_squares += sign * (
                B->nb_pawns_by_parity[self][c] ?  0 : /* closed */
                B->nb_pawns_by_parity[them][c] ? 13 : /* semi-open */
                                                 34); /* open */
        }
        { /* redundant-majors */
            d_material -= sign * 34 * (B->nb_majors[self]-1);
        }
    break; case Species::queen:
        { /* redundant-majors */
            d_material -= sign * 34 * (B->nb_majors[self]-1);
        }
    break; case Species::king:
        Coordinate old_rc = GET_FROM_LAST(B->king_locs[self], 2);
        int old_r = old_rc.row;
        int old_c = old_rc.col;
        { /* king-tropism */
            int old_q = quintant_from(old_rc);
            int new_q = quintant_from(rc);
            if (old_q != new_q) {
                d_king_safety -= sign * 34 * ( B->nb_pieces_by_quintant[them][new_q]
                                              -B->nb_pieces_by_quintant[them][old_q]);
            }
        }
        { /* king-shelter */
            auto has_pawn = [self, B](int row, int col){
                if (!(0<=col && col<8)) { return 0; }
                return KRON(piece_equals(get_piece(B, {row,col}), {self,Species::pawn}));
            };
            d_king_safety += sign * 13 * (
               -has_pawn(old_r-sign, old_c-1) /* old */
               -has_pawn(old_r-sign, old_c  ) 
               -has_pawn(old_r-sign, old_c+1) 
               +has_pawn(    r-sign,     c-1) /* new */
               +has_pawn(    r-sign,     c  ) 
               +has_pawn(    r-sign,     c+1) 
            );
        }
        { /* king-xrays */ /* TODO */ }
    }

    /* update back of stack (assumed pre-pushed) */
    DisaggregatedScore* ds = &B->evaluation_stack.back();
    ds->material       += (is_add ? +1 : -1) * d_material;
    ds->king_safety    += (is_add ? +1 : -1) * d_king_safety;
    ds->pawn_structure += (is_add ? +1 : -1) * d_pawn_structure;
    ds->cozy_squares   += (is_add ? +1 : -1) * d_cozy_squares;
}

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


//int weak_square_malus(Board const* B)
//{
//    int acc=0;
//    for (int r=0; r!=8; ++r) {
//        for (int c=0; c!=8; ++c) {
//            /* WHITE's perspective: LESSER .row means MORE advanced */
//            bool is_weak_white =
//                (             !B->nb_pawns_by_file[1][c]   || B->least_advanced[1][c]<= r) && 
//                (!(0<=c-1) || !B->nb_pawns_by_file[1][c-1] || B->least_advanced[1][c-1]<r) &&
//                (!(c+1<8 ) || !B->nb_pawns_by_file[1][c+1] || B->least_advanced[1][c+1]<r);
//            bool is_outpost_black = is_weak_white && (
//                B->attacks_by_pawn[0][r][c] &&
//                B->nb_pawns_by_file[1][c] &&
//                B->least_advanced[1][c]<r 
//            );
//            bool knighted_black= 
//                get_piece(B, {r,c}).species==Species::knight &&
//                get_piece(B, {r,c}).color==Color::black;
//
//            /* BLACK's perspective: GREATER .row means MORE advanced */
//            bool is_weak_black =
//                (             !B->nb_pawns_by_file[0][c]   || B->least_advanced[0][c]>= r) && 
//                (!(0<=c-1) || !B->nb_pawns_by_file[0][c-1] || B->least_advanced[0][c-1]>r) &&
//                (!(c+1<8 ) || !B->nb_pawns_by_file[0][c+1] || B->least_advanced[0][c+1]>r);
//            bool is_outpost_white = is_weak_black && (
//                B->attacks_by_pawn[1][r][c] &&
//                B->nb_pawns_by_file[0][c] &&
//                B->least_advanced[0][c]<r 
//            );
//            bool knighted_white = 
//                get_piece(B, {r,c}).species==Species::knight &&
//                get_piece(B, {r,c}).color==Color::white;
//
//            acc += knighted_black ?
//                (is_weak_white ? (is_outpost_black ? -89-13 : -34-5) : 0) :
//                (is_weak_white ? (is_outpost_black ?    -13 :    -5) : 0);
//            acc -= knighted_white ?
//                (is_weak_black ? (is_outpost_white ? -89-13 : -34-5) : 0) :
//                (is_weak_black ? (is_outpost_white ?    -13 :    -5) : 0);
//        }
//    }
//    return acc;
//}
