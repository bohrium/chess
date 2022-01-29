#include "Board.h"
#include "Helpers.h"
#include <iostream>
#include "EvalParams.h"


int score_total(DisaggregatedScore ds) {
    return ds.material
         + ds.king_safety
         + ds.pawn_structure
         + ds.cozy_squares;
}

int evaluate(Board* B) /* todo: constify type signature */
{
    if (B->plies_since_irreversible.back() >= NB_PLIES_TIL_DRAW ) { /* the 50 move rule */
        return -KING_POINTS/2; /* black wins*/
    }

    /* TODO: incorporate king attacks etc into difference eval */
    int score = score_total(B->evaluation_stack.back()) 
       + TURN_BONUS  * (B->next_to_move==Color::white ? +1 : -1)
       + KING_XRAY   * ( B->nb_king_attacks_near[0] 
                        -B->nb_king_attacks_near[1]) 
       + WEAK_SQUARE * ( B->nb_weak_squares[0] 
                        -B->nb_weak_squares[1]) 
       + MOBILITY    * (-B->nb_xrays_by_side[0] 
                        +B->nb_xrays_by_side[1]);

    float fraction_to_exhaustion = B->plies_since_irreversible.back()/((float)NB_PLIES_TIL_DRAW); 
    return (fraction_to_exhaustion*(-CONTEMPT) + (1.0-fraction_to_exhaustion)*score); 
}


void add_eval_diff(Board* B, Coordinate rc, Piece p, bool is_add)
{
    int const sign = p.color==Color::white ? +1 : -1;
    Color const self = p.color; 
    Color const them = flip_color(p.color);
    int const r=rc.row;
    int const c=rc.col;

    /* evaluation */
    int d_initiative     = 0;
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
            if (kq == pq) { d_king_safety += sign * KING_TROPISM; }
        }
        { /* king-xrays */ /* TODO */ }
        { /* loose-piece */
            //d_initiative -= sign * ATTACKED_PIECE * (nb_xrays[them][r][c] ? 1 : 0); 
        }
    }

    /* update piece-square counts and pawn structure terms */
    switch (p.species) {
    break; case Species::pawn:
        { /* cramped-bishop */
            d_cozy_squares -= sign * BLOCKED_BISHOP * B->nb_bishops_by_parity[self][parity(rc)];
        }
        { /* connected-pawn, opened-rook */
            if ( is_add && B->nb_pawns_by_file[self][c]==1 || /* we have just filled an empty file */
                !is_add && B->nb_pawns_by_file[self][c]==0) { /* ... or are undoing that */
                if (c != 0) { d_pawn_structure += sign * DISCONNECTED * KRON(B->nb_pawns_by_file[self][c-1]); }
                if (c != 7) { d_pawn_structure += sign * DISCONNECTED * KRON(B->nb_pawns_by_file[self][c+1]); }
                    d_cozy_squares += sign * B->nb_rooks_by_file[self][c] * (
                            B->nb_pawns_by_file[them][c] ?  0-ROOK_ON_SEMI : /* for friendly: semi-open to closed*/
                                                            0-ROOK_ON_OPEN); /* for friendly: open to closed */
                    d_cozy_squares -= sign * B->nb_rooks_by_file[them][c] * (
                            B->nb_pawns_by_file[them][c] ? 0           -0 : /* for enemy: closed to closed */ 
                                                           ROOK_ON_SEMI-ROOK_ON_OPEN); /* for enemy: open to semi-open */
            }
        }
        { /* king-shelter */
            Coordinate kl = B->king_locs[self].back();
            if (r==kl.row-sign && ABS(c - kl.col)<=1) {
                d_king_safety += sign * KING_SHELTER;
            }
        }
        { /* weak-squares, posted-knight */ /* TODO */ }
        { /* king-xrays */ /* TODO */ }
        { /* loose-pieces */ /* TODO */ }
        { /* cramped-rook */
            d_material -= sign * ROOK_PAWN * B->nb_rooks[self];  
        }
        { /* cocramped-knight */
            d_material += sign * KNIGHT_PAWN * B->nb_knights[self];  
        }
    break; case Species::knight:
        { /* posted-knight */ /* TODO */ }
        { /* cocramped-knight */
            d_material += sign * KNIGHT_PAWN * B->nb_pawns[self];  
        }
    break; case Species::bishop:
        { /* cramped-bishop */
            d_cozy_squares -= sign * BLOCKED_BISHOP * B->nb_pawns_by_parity[self][parity(rc)];
        }
        { /* bishop-pair */
            d_material += sign * BISHOP_PAIR * B->nb_bishops_by_parity[self][1-parity(rc)];
        }
    break; case Species::rook:
        { /* opened-rook */
            d_cozy_squares += sign * (
                B->nb_pawns_by_parity[self][c] ?            0 : /* closed */
                B->nb_pawns_by_parity[them][c] ? ROOK_ON_SEMI : /* semi-open */
                                                 ROOK_ON_OPEN); /* open */
        }
        { /* redundant-majors */
            d_material -= sign * MAJOR_PAIR * (B->nb_majors[self] + (is_add ? -1 : 0));
        }
        { /* cramped-rook */
            d_material -= sign * ROOK_PAWN * B->nb_pawns[self];  
        }
    break; case Species::queen:
        { /* redundant-majors */
            d_material -= sign * MAJOR_PAIR * (B->nb_majors[self] + (is_add ? -1 : 0));
        }
    break; case Species::king:
        Coordinate old_rc = GET_FROM_LAST(B->king_locs[self], 2);
        int old_r = old_rc.row;
        int old_c = old_rc.col;
        { /* king-tropism */
            int old_q = quintant_from(old_rc);
            int new_q = quintant_from(rc);
            if (old_q != new_q) {
                d_king_safety -= sign * KING_TROPISM * ( B->nb_pieces_by_quintant[them][new_q]
                                                        -B->nb_pieces_by_quintant[them][old_q]);
            }
        }
        { /* king-shelter */
            auto has_pawn = [self, B](int row, int col){
                if (!(0<=col && col<8)) { return 0; }
                return KRON(piece_equals(get_piece(B, {row,col}), {self,Species::pawn}));
            };
            d_king_safety += sign * KING_SHELTER * (
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
    ds->initiative     += (is_add ? +1 : -1) * d_initiative;
    ds->material       += (is_add ? +1 : -1) * d_material;
    ds->king_safety    += (is_add ? +1 : -1) * d_king_safety;
    ds->pawn_structure += (is_add ? +1 : -1) * d_pawn_structure;
    ds->cozy_squares   += (is_add ? +1 : -1) * d_cozy_squares;
}


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
