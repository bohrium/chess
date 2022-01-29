#include "Board.h"
#include "Helpers.h"
#include <iostream>

void update_xrays_by_pawn(Board* B, Coordinate rc, Piece p, bool is_add); 
void update_king_attacks(Board* B, Coordinate rc, Color king_color, bool is_add);
void update_xrays_by_piece(Board* B, Coordinate rc, Piece p, bool is_add);
void update_weak_squares(Board* B, Color side, int c);
void update_least_advanced(Board* B, Color side, int c);

/*=============================================================================
====  0. MOVE GENERATION  =====================================================
=============================================================================*/

void generate_pawn_moves  (Board const* B, MoveList* ML, Coordinate source, bool captures_only);
void generate_knight_moves(Board const* B, MoveList* ML, Coordinate source, bool captures_only);
void generate_bishop_moves(Board const* B, MoveList* ML, Coordinate source, bool captures_only);
void generate_rook_moves  (Board const* B, MoveList* ML, Coordinate source, bool captures_only);
void generate_queen_moves (Board const* B, MoveList* ML, Coordinate source, bool captures_only);
void generate_king_moves  (Board const* B, MoveList* ML, Coordinate source, bool captures_only);

void generate_moves_for_piece(Board const* B, MoveList* ML, Piece mover, Coordinate source, bool captures_only)
{
    /* does nothing for empty Piece */
    switch (mover.species) {
    break; case Species::empty_species: 
    break; case Species::pawn:   generate_pawn_moves  (B, ML, source, captures_only);  
    break; case Species::knight: generate_knight_moves(B, ML, source, captures_only);  
    break; case Species::bishop: generate_bishop_moves(B, ML, source, captures_only);  
    break; case Species::rook:   generate_rook_moves  (B, ML, source, captures_only);  
    break; case Species::queen:  generate_queen_moves (B, ML, source, captures_only);  
    break; case Species::king:   generate_king_moves  (B, ML, source, captures_only);  
    }
}
void generate_moves(Board const* B, MoveList* ML, bool captures_only)
{
    ML->length = 0;
    for (int r=0; r!=8; ++r) {
        for (int c=0; c!=8; ++c) {
            Piece mover = get_piece(B, {r, c});
            if (mover.color != B->next_to_move) { continue; }
            generate_moves_for_piece(B, ML, mover, {r, c}, captures_only);
        }
    }
}

bool add_move_color_guard(Board const* B, MoveList* ML, Coordinate source, Coordinate dest, Color needed_color, bool captures_only)
{
    Piece taken = get_piece(B, dest);
    if (is_valid(dest) && taken.color == needed_color) {
        if (!captures_only || needed_color!=Color::empty_color) {
            ML->moves[ML->length++] = {source, dest, taken, MoveType::ordinary}; 
        }
        return true;
    }
    return false;
} 

bool add_pawn_move_color_guard(Board const* B, MoveList* ML, Coordinate source, Coordinate dest, Color needed_color, bool captures_only)
{ /* only difference from `add_move_color_guard` is possibility of promotion */
    Piece taken = get_piece(B, dest);
    if (is_valid(dest) && taken.color == needed_color) {
        if (!captures_only || needed_color!=Color::empty_color) {
            Move m = {source, dest, taken, MoveType::ordinary};
            /* CAUTION: row "0" is farthest from white's camp */
            Piece mover = get_piece(B, source);
            if (mover.color==Color::white && dest.row==0 || 
                mover.color==Color::black && dest.row==7) {
                m.type = MoveType::promote_to_queen;
            } 
            ML->moves[ML->length++] = m; 
        }
        return true;
    }
    return false;
}

void generate_pawn_moves  (Board const* B, MoveList* ML, Coordinate source, bool captures_only)
{
    int row=source.row, col=source.col;
    int direction = B->next_to_move==Color::white ? -1 : +1;
    int start = B->next_to_move==Color::white ? 6 : 1;
    Coordinate dest = {row+direction, col}; 
    if (add_pawn_move_color_guard(B, ML, source, {row+direction, col}, Color::empty_color, captures_only) &&
        row == start) {
        add_move_color_guard(B, ML, source, {row+2*direction, col}, Color::empty_color, captures_only);
    }
    add_pawn_move_color_guard(B, ML, source, {row+direction, col+1}, flip_color(B->next_to_move), captures_only);
    add_pawn_move_color_guard(B, ML, source, {row+direction, col-1}, flip_color(B->next_to_move), captures_only);
}
void generate_knight_moves(Board const* B, MoveList* ML, Coordinate source, bool captures_only)
{
    int row=source.row, col=source.col;
    for (int dr=-2; dr!=3; ++dr) {
        for (int dc=-2; dc!=3; ++dc) {
            if (dr*dr + dc*dc != 2*2 + 1*1) { continue; }  
            add_move_color_guard(B, ML, source, {row+dr, col+dc}, Color::empty_color, captures_only);
            add_move_color_guard(B, ML, source, {row+dr, col+dc}, flip_color(B->next_to_move), captures_only);
        }
    }
}
void generate_bishop_moves(Board const* B, MoveList* ML, Coordinate source, bool captures_only)
{
    int row=source.row, col=source.col;
    for (int dr=-1; dr!=2; ++dr) {
        for (int dc=-1; dc!=2; ++dc) {
            if (dr*dr + dc*dc != 1*1 + 1*1) { continue; }
            for (int t=1; t!=8; ++t) {
                if (! add_move_color_guard(B, ML, source, {row+t*dr, col+t*dc}, Color::empty_color, captures_only)) {
                    add_move_color_guard(B, ML, source, {row+t*dr, col+t*dc}, flip_color(B->next_to_move), captures_only);
                    break;
                }
            }
        }
    }
}
void generate_rook_moves  (Board const* B, MoveList* ML, Coordinate source, bool captures_only)
{
    int row=source.row, col=source.col;
    for (int dr=-1; dr!=2; ++dr) {
        for (int dc=-1; dc!=2; ++dc) {
            if (dr*dr + dc*dc != 1*1) { continue; }
            for (int t=1; t!=8; ++t) {
                if (! add_move_color_guard(B, ML, source, {row+t*dr, col+t*dc}, Color::empty_color, captures_only)) {
                    add_move_color_guard(B, ML, source, {row+t*dr, col+t*dc}, flip_color(B->next_to_move), captures_only);
                    break;
                }
            }
        }
    }
}
void generate_queen_moves (Board const* B, MoveList* ML, Coordinate source, bool captures_only)
{
    int row=source.row, col=source.col;
    for (int dr=-1; dr!=2; ++dr) {
        for (int dc=-1; dc!=2; ++dc) {
            if (dr*dr + dc*dc == 0) { continue; }
            for (int t=1; t!=8; ++t) {
                if (! add_move_color_guard(B, ML, source, {row+t*dr, col+t*dc}, Color::empty_color, captures_only)) {
                    add_move_color_guard(B, ML, source, {row+t*dr, col+t*dc}, flip_color(B->next_to_move), captures_only);
                    break;
                }
            }
        }
    }
}
void generate_king_moves (Board const* B, MoveList* ML, Coordinate source, bool captures_only)
{
    int row=source.row, col=source.col;
    for (int dr=-1; dr!=2; ++dr) {
        for (int dc=-1; dc!=2; ++dc) {
            if (dr*dr + dc*dc == 0) { continue; }
            add_move_color_guard(B, ML, source, {row+dr, col+dc}, Color::empty_color, captures_only);
            add_move_color_guard(B, ML, source, {row+dr, col+dc}, flip_color(B->next_to_move), captures_only);
        }
    }
}

/*=============================================================================
====  1. MOVE DISPLAY  ========================================================
=============================================================================*/

void print_move(Board const* B, Move m)
{
    if (m.type == MoveType::extra_legal) {
        std::cout << "######";
        return;
    }
    char mover = species_names[get_piece(B, m.source).species];
    char taken = species_names[m.taken.species];
    bool is_white = B->next_to_move==Color::white; 
    std::cout << COLORIZE((is_white?MAGENTA:CYAN), mover <<
                                (char)(m.dest.col+'a') << 8-m.dest.row); 
    std::cout << COLORIZE((is_white?MAGENTA:CYAN), "("  );
    std::cout << COLORIZE((is_white?CYAN:MAGENTA), taken);
    std::cout << COLORIZE((is_white?MAGENTA:CYAN), ")"  );
}
void print_movelist(Board const* B, MoveList* ML)
{
    for (int i=0; i!=ML->length; ++i) {
        print_move(B, ML->moves[i]);
        std::cout << std::endl;
    }
}

/*=============================================================================
====  2. HASHING  =============================================================
=============================================================================*/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~  2.0. Non-Positional State  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

unsigned int const TO_MOVE_HASH     = 2718281828;
unsigned int const PLY_COUNTER_HASH = 3141592653;

/* decimal order of magnitude: 4.0 + 5.0 ~ 9.0, appropriate for 32 bits */
inline unsigned int hash_from_ply(int old_ply, int new_ply)
{ /* collapse positionally equal states if they are far from a draw */
    return TO_MOVE_HASH ^ 
        ((2*old_ply<NB_PLIES_TIL_DRAW ? 0 : old_ply) * PLY_COUNTER_HASH) ^
        ((2*old_ply<NB_PLIES_TIL_DRAW ? 0 : new_ply) * PLY_COUNTER_HASH);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~  2.1. Positional State  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/* decimal order of magnitude: 1.5 + 2.5 ~ 4.0 */
unsigned int const hash_by_piece[nb_colors][nb_species] = {
  {47*111, 47*211, 47*311, 47*411, 47*511, 47*611, 0},
  {27*111, 27*211, 27*311, 27*411, 27*511, 27*611, 0},
  {     0,      0,      0,      0,      0,      0, 0},
};

/* decimal order of magnitude: 1.5 + 3.5 ~ 5.0 */
unsigned int const hash_by_square[8][8] = {
    {13*103, 13*305, 13*507, 13*709, 13*301, 13*503, 13*705, 13*907},
    {35*103, 35*305, 35*507, 35*709, 35*301, 35*503, 35*705, 35*907},
    {57*103, 57*305, 57*507, 57*709, 57*301, 57*503, 57*705, 57*907},
    {79*103, 79*305, 79*507, 79*709, 79*301, 79*503, 79*705, 79*907},
    {31*103, 31*305, 31*507, 31*709, 31*301, 31*503, 31*705, 31*907},
    {53*103, 53*305, 53*507, 53*709, 53*301, 53*503, 53*705, 53*907},
    {75*103, 75*305, 75*507, 75*709, 75*301, 75*503, 75*705, 75*907},
    {97*103, 97*305, 97*507, 97*709, 97*301, 97*503, 97*705, 97*907},
};

/* decimal order of magnitude: 4.0 + 5.0 ~ 9.0, appropriate for 32 bits */
inline unsigned int hash_from_piece_square(Piece p, Coordinate rc)
{
    return hash_by_piece[p.color][p.species] *
           hash_by_square[rc.row][rc.col];
}

/*=============================================================================
====  3. APPLY / UNDO MOVE  ===================================================
=============================================================================*/

void apply_null(Board* B)
{
    B->next_to_move = flip_color(B->next_to_move);
    B->hash ^= TO_MOVE_HASH;
}

void undo_null(Board* B)
{
    B->next_to_move = flip_color(B->next_to_move);
    B->hash ^= TO_MOVE_HASH;
}

void apply_move(Board* B, Move m)
{
    // note asymmetry wrt undo_move 
    Piece mover  = get_piece(B, m.source);
    Piece lander = mover;
    if (m.type == MoveType::promote_to_queen) {
        lander.species = Species::queen;
    }
    Piece taken = m.taken;

    int old_ply = B->plies_since_irreversible.back();
    int new_ply = (is_irreversible(m, mover) && old_ply < NB_PLIES_TIL_DRAW) ?
        0 : (B->plies_since_irreversible.back() + 1);
    B->plies_since_irreversible.push_back(new_ply);
    B->hash ^= hash_from_ply(old_ply, new_ply);

    B->next_to_move = flip_color(B->next_to_move);

    CLONE_BACK(B->evaluation_stack);
    CLONE_BACK(B->king_locs[0]);
    CLONE_BACK(B->king_locs[1]);
    if (lander.species==Species::king) {
        B->king_locs[mover.color].back() = m.dest;
    } 

    change_piece(B, m.source, mover, false);
    add_eval_diff(B, m.source, mover, false);
    if (is_capture(m)) {
        change_piece(B, m.dest, taken, false);
        add_eval_diff(B, m.dest, taken, false);
    }
    change_piece(B, m.dest, lander, true);
    add_eval_diff(B, m.dest, lander, true);
}

void undo_move(Board* B, Move m)
{
    // note asymmetry wrt apply_move
    Piece lander = get_piece(B, m.dest);
    Piece mover = lander ;
    if (m.type == MoveType::promote_to_queen) {
        mover .species = Species::pawn;
    }
    Piece taken = m.taken;

    int new_ply = B->plies_since_irreversible.back();
    B->plies_since_irreversible.pop_back();
    int old_ply = B->plies_since_irreversible.back();
    B->hash ^= hash_from_ply(old_ply, new_ply);

    B->next_to_move = flip_color(B->next_to_move);

    B->evaluation_stack.pop_back();
    B->king_locs[0].pop_back();
    B->king_locs[1].pop_back();

    change_piece(B, m.dest, lander, false);
    if (is_capture(m)) {
        change_piece(B, m.dest, taken, true);
    }
    change_piece(B, m.source, mover, true);
}

////

/* TODO: codify GRAND CONVENTION: alternate signs of direction!! */
void change_piece(Board* B, Coordinate rc, Piece p, bool is_add)
{
    int const sign = p.color==Color::white ? +1 : -1;
    Color const self = p.color; 
    Color const them = flip_color(p.color);
    int const r=rc.row;
    int const c=rc.col;

    int const sign_d = is_add ? +1 : -1;

    /* hash (TODO: EXCEPT to-move-color, 50 move rule, etc: extra state) */
    B->hash ^= hash_from_piece_square(p, rc);

    /* update grid */
    if (is_add) { B->grid[r][c] = p; }
    else        { B->grid[r][c] = empty_piece; }


    if (p.species != Species::pawn && p.species != Species::king) {
        int pq = quintant_from(rc);
        int kq = quintant_from(B->king_locs[them].back());
        B->nb_pieces_by_quintant[self][pq] += sign_d; 
        
        update_xrays_by_piece(B, rc, p, is_add);
        update_king_attacks(B, B->king_locs[them].back(), them, is_add);
        //{ /* king-xrays */
        //    /* TODO */
        //}
        //{ /* loose pieces */
        //}
    }
    //nb_xrayed_stones[self] += sign_d * (nb_xrays[them][r][c] ? 1 : 0);
    //nb_loose_stones [self] += sign_d * (nb_xrays[them][r][c] ? 1 : 0);

    /* update piece-square counts and pawn structure terms */
    switch (p.species) {
    break; case Species::pawn:
        B->nb_pawns[self] += sign_d;
        B->nb_pawns_by_parity[self][parity(rc)] += sign_d;
        B->nb_pawns_by_file[self][c] += sign_d;
        update_least_advanced(B, self, c);
        { /* weak-squares, posted-knight */
            if (c != 0) { B->attacks_by_pawn[self][r-sign][c-1] += sign_d; }
            if (c != 7) { B->attacks_by_pawn[self][r-sign][c+1] += sign_d; }
            /* TODO; note that order of operations might depend on is_add */

            if (true) { update_weak_squares(B, self, c  ); }
            if (1<=c) { update_weak_squares(B, self, c-1); }
            if (c<7 ) { update_weak_squares(B, self, c+1); }
        }
        update_xrays_by_pawn(B, rc, p, is_add); 
        //{ /* king x-rays */
        //}
        //{ /* loose pieces */
        //}
    break; case Species::knight:
        B->nb_knights[self] += sign_d; 
    break; case Species::bishop:
        B->nb_pawns_by_parity[self][parity(rc)] += sign_d;
    break; case Species::rook:
        B->nb_rooks[self] += sign_d;
        B->nb_rooks_by_file[self][c] += sign_d;
        B->nb_majors[self] += sign_d;
    break; case Species::queen:
        B->nb_majors[self] += sign_d;
    break; case Species::king: 
        update_king_attacks(B, rc, self, is_add);
    }
}

/*=============================================================================
====  4. AUX BOARD STATE UPDATERS  ============================================
=============================================================================*/

/* REQUIRES: assumes nb_pawns_by_file is correct! */
void update_least_advanced(Board* B, Color side, int c) {
    int start = (side == Color::white ?  6 :  1);
    int step  = (side == Color::white ? -1 : +1);
    int end   = (side == Color::white ?  0 :  7);

    if ( ! B->nb_pawns_by_file[side][c] ) {
        B->least_advanced[side][c] = end+step; /* sentinel value */
        return;
    }
    for (int r=start; r!=end; r+=step) {
        if (!kronecker_piece(B, {r,c}, {side,Species::pawn})) { continue; }
        B->least_advanced[side][c] = r;
        return;
    }
    std::cout << "\033[120C" << "WOAH!  SHOULDN'T ARRIVE HERE!" << std::endl;
} 

void update_weak_squares(Board* B, Color side, int c) {
    int start = (side == Color::white ?  7 :  0); /* white's home base is row 7 */
    int step  = (side == Color::white ? -1 : +1);
    int end   = (side == Color::white ? -1 :  8);

    auto atx = B->attacks_by_pawn[side];
    int is_attackable = false;
    for (int r=start; r!=end; r+=step) {
        if (!is_attackable &&
            ((1<=c&& atx[r][c-1]) ||
             (c <7 && atx[r][c+1]))) {
            is_attackable = true;
        }
        Piece p = B->grid[r][c]; 
        bool is_weak = !is_attackable && (p.species!=Species::pawn || p.color!=side);
        bool* old = &B->is_weak_square[side][r][c];  
        if (*old != is_weak) {
            *old = is_weak;
            B->nb_weak_squares[side] += (is_weak ? +1 : -1); 
        }
    }
} 
void update_xrays_by_piece(Board* B, Coordinate rc, Piece p, bool is_add) 
{
    int const sign_d = is_add ? +1 : -1;
    Color const self = p.color;
    Color const them = flip_color(self);

    int nb_dirs;
    auto dirs = knight_dirs;
    switch (p.species) {
    break; case Species::knight: nb_dirs=nb_knight_dirs; dirs=knight_dirs; goto LEAPER; 
    break; case Species::bishop: nb_dirs=nb_bishop_dirs; dirs=bishop_dirs; goto RIDER ;
    break; case Species::rook  : nb_dirs=nb_rook_dirs  ; dirs=rook_dirs  ; goto RIDER ;
    break; case Species::queen : nb_dirs=nb_queen_dirs ; dirs=queen_dirs ; goto RIDER ;
    }

LEAPER:
    for (int k=0; k!=nb_dirs; ++k) {
        Coordinate new_rc = {rc.row + dirs[k][0], rc.col + dirs[k][1]};
        if (!is_valid(new_rc)) { continue; }
        B->nb_xrays[self][new_rc.row][new_rc.col] += sign_d;
        B->nb_xrays_by_side[self] += sign_d;
    }
    return;

RIDER :
    for (int k=0; k!=nb_dirs; ++k) {
        int dr = dirs[k][0]; int dc = dirs[k][1];

        Coordinate new_rc = rc;
        int T = max_ray_len(rc.row, rc.col, dr, dc); 
        int t=0;
        Piece p;
        #define LOOP_BODY                                            \
            new_rc.row += dr; new_rc.col += dc;                      \
            B->nb_xrays[self][new_rc.row][new_rc.col] += sign_d;     \
            B->nb_xrays_by_side[self] += sign_d;                     \
            p = B->grid[new_rc.row][new_rc.col];                     \
            if (p.species==Species::pawn && p.color==self) { break; } 

        switch (T) { /*waterfall*/
            case 7: LOOP_BODY 
            case 6: LOOP_BODY
            case 5: LOOP_BODY
            case 4: LOOP_BODY
            case 3: LOOP_BODY
            case 2: LOOP_BODY
            case 1: LOOP_BODY
            case 0: break;
        }

        #undef LOOP_BODY

        //for (int t=0; t!=T; ++t) {
        //    new_rc.row += dr; new_rc.col += dc;
        //    B->nb_xrays[self][new_rc.row][new_rc.col] += sign_d;
        //    B->nb_xrays_by_side[self] += sign_d;
        //    Piece p = B->grid[new_rc.row][new_rc.col];
        //    if (p.species==Species::pawn && p.color==self) { break; } /* can't see through own pawn */
        //}
    }
    return;

}

void update_king_attacks(Board* B, Coordinate rc, Color king_color, bool is_add)
{
    Color const self = king_color;
    Color const them = flip_color(self);

    B->nb_king_attacks_near[self] = 0; 
    if (!is_add) { return; } /* return only after zeroing */

    int r, c;
    for (int dr=-1; dr!=2; ++dr) {
        r = rc.row+dr; if (!(0<=r && r<8)) { continue; }
        for (int dc=-1; dc!=2; ++dc) {
            if (!(dc||dr)) { continue; }
            c = rc.col+dc; if (!(0<=c && c<8)) { continue; }
            B->nb_king_attacks_near[self] += B->nb_xrays[them][r][c];
        }
    }
}

void update_xrays_by_pawn(Board* B, Coordinate rc, Piece p, bool is_add) 
{
    int const sign_d = is_add ? +1 : -1;
    Color const self = p.color;
    //Color const them = flip_color(self);

    // removing an un-xrayed pawn changes nothing
    if (!is_add && !B->nb_xrays[self][rc.row][rc.col]) { return; }

    int nb_sources_by_dir[nb_queen_dirs] = {0,0,0,0,0,0,0,0};
    int ranges_by_dir    [nb_queen_dirs] = {0,0,0,0,0,0,0,0};

    for (int k=0; k!=nb_queen_dirs; ++k) {
        int dr = queen_dirs[k][0]; int dc = queen_dirs[k][1];
        Species goal_a = Species::queen;
        Species goal_b = (dr*dr+dc*dc==2) ? Species::bishop
                                          : Species::rook;

        Coordinate src_rc = rc;
        int T = max_ray_len(rc.row, rc.col, dr, dc);
        int t=0;
        Piece p;
        #define LOOP_BODY                                                                   \
            src_rc.row += dr; src_rc.col += dc;                                             \
            p = B->grid[src_rc.row][src_rc.col];                                            \
            if (p.color == self) { /*continue;*/                                            \
            if (p.species==Species::pawn) { t+=1; break; } /* can't see through own pawn */ \
            if (p.species==goal_a || p.species==goal_b) { nb_sources_by_dir[k] += 1; }      \
            }                                                                               \
            ++t; /* manual loop iter */                                                          

        switch (T) { /*waterfall*/
            case 7: LOOP_BODY 
            case 6: LOOP_BODY
            case 5: LOOP_BODY
            case 4: LOOP_BODY
            case 3: LOOP_BODY
            case 2: LOOP_BODY
            case 1: LOOP_BODY
            case 0: break;
        }

        #undef LOOP_BODY

        ranges_by_dir[k] = t; /* already incremented, mind you! */
    }

    for (int k=0; k!=nb_queen_dirs; ++k) {/* go in opposite direction */
        int minus_k = minus_idx(k);
        int nb_sources = nb_sources_by_dir[minus_k]; 
        if (!nb_sources || !ranges_by_dir[k]) { continue; }
        int dr = queen_dirs[k][0]; int dc = queen_dirs[k][1];

        Coordinate dst_rc = rc;
        int T = ranges_by_dir[k];
        int t=0;
        #define LOOP_BODY                                                       \
            dst_rc.row += dr; dst_rc.col += dc;                                 \
            B->nb_xrays[self][dst_rc.row][dst_rc.col] -= sign_d * nb_sources;   \
            B->nb_xrays_by_side[self] -= sign_d * nb_sources;                   

        switch (T) { /*waterfall*/
            case 7: LOOP_BODY 
            case 6: LOOP_BODY
            case 5: LOOP_BODY
            case 4: LOOP_BODY
            case 3: LOOP_BODY
            case 2: LOOP_BODY
            case 1: LOOP_BODY
            case 0: break;
        }

        #undef LOOP_BODY
    }
}
