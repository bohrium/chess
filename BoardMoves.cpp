#include "Board.h"
#include "Helpers.h"
#include <iostream>

unsigned int hash_by_piece[3][7] = {
  {1234567, 2345671, 3456712, 45767123, 5671234, 6712345, 0},
  {3456701, 4567101, 5671201, 76712301, 7123401, 1234501, 0},
  {0, 0, 0, 0, 0, 0, 0},
};

unsigned int hash_by_square[8][8] = {
    {123*1301, 123*3501, 123*5701, 123*7901, 123*3101, 123*5301, 123*7501, 123*9701},
    {345*1301, 345*3501, 345*5701, 345*7901, 345*3101, 345*5301, 345*7501, 345*9701},
    {567*1301, 567*3501, 567*5701, 567*7901, 567*3101, 567*5301, 567*7501, 567*9701},
    {789*1301, 789*3501, 789*5701, 789*7901, 789*3101, 789*5301, 789*7501, 789*9701},
    {321*1301, 321*3501, 321*5701, 321*7901, 321*3101, 321*5301, 321*7501, 321*9701},
    {543*1301, 543*3501, 543*5701, 543*7901, 543*3101, 543*5301, 543*7501, 543*9701},
    {765*1301, 765*3501, 765*5701, 765*7901, 765*3101, 765*5301, 765*7501, 765*9701},
    {987*1301, 987*3501, 987*5701, 987*7901, 987*3101, 987*5301, 987*7501, 987*9701},
};

int quintant_by_coor[8][8] = {
    {0,0,0,0,1,1,1,1},
    {0,0,0,0,1,1,1,1},
    {0,0,0,4,4,1,1,1},
    {0,0,4,4,4,4,1,1},
    {2,2,4,4,4,4,3,3},
    {2,2,2,4,4,3,3,3},
    {2,2,2,2,3,3,3,3},
    {2,2,2,2,3,3,3,3},
};

unsigned int hash_of(Move m, Piece mover)
{
    /* mode is whether for updating forward or backward*/
    int sr = m.source.row;
    int sc = m.source.col;
    int dr = m.dest.row;
    int dc = m.dest.col;
    Piece taken = m.taken;
    return (
         (hash_by_piece[mover.color][mover.species] * hash_by_square[sr][sc]) ^
         (hash_by_piece[mover.color][mover.species] * hash_by_square[dr][dc]) ^
        (is_capture(m) ?
         (hash_by_piece[taken.color][taken.species] * hash_by_square[dr][dc]) : 0)
    );
} 

#define TO_MOVE_HASH 271828
void apply_null(Board* B)
{
    B->next_to_move = flip_color(B->next_to_move);
    B->hash ^= TO_MOVE_HASH;
    /* todo: change side-to-move hash */
}
void undo_null(Board* B)
{
    B->next_to_move = flip_color(B->next_to_move);
    B->hash ^= TO_MOVE_HASH;
    /* todo: change side-to-move hash */
}

/* REQUIRES: assumes nb_pawns_by_file is correct! */
void update_least_advanced(Board* B, Color side, int col) {
    int start = (side == Color::white ?  6 :  1);
    int step  = (side == Color::white ? -1 : +1);
    int end   = (side == Color::white ?  0 :  7);

    if ( ! B->nb_pawns_by_file[side][col] ) {
        B->least_advanced[side][col] = end+step; /* sentinel value */
        return;
    }
    for (int row=start; row!=end; row+=step) {
        if (!piece_equals(get_piece(B, {row,col}), {side,Species::pawn})) { continue; }
        B->least_advanced[side][col] = row;
        return;
    }
    std::cout << "WOAH!  SHOULDN'T ARRIVE HERE!" << std::endl;
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

    B->plies_since_irreversible.push_back(
        (is_irreversible(m, mover) &&
        B->plies_since_irreversible.back() < NB_PLIES_TIL_DRAW) ?
        0 : (B->plies_since_irreversible.back() + 1)
    );
    B->hash ^= TO_MOVE_HASH;
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

    B->plies_since_irreversible.pop_back();
    B->hash ^= TO_MOVE_HASH;
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

void print_move(Board const* B, Move M)
{
    char mover = species_names[get_piece(B, M.source).species];
    std::cout << mover << (char)(M.dest.col+'a') << 8-M.dest.row;
    std::cout << "(" << species_names[M.taken.species] << ")";
}
void print_movelist(Board const* B, MoveList* ML)
{
    for (int i=0; i!=ML->length; ++i) {
        print_move(B, ML->moves[i]);
        std::cout << std::endl;
    }
}

void generate_moves_for_piece(Board const* B, MoveList* ML, Piece mover, Coordinate source)
{
    /* does nothing for empty Piece */
    switch (mover.species) {
    case Species::empty_species: break;
    case Species::pawn:   generate_pawn_moves  (B, ML, source); break; 
    case Species::knight: generate_knight_moves(B, ML, source); break; 
    case Species::bishop: generate_bishop_moves(B, ML, source); break; 
    case Species::rook:   generate_rook_moves  (B, ML, source); break; 
    case Species::queen:  generate_queen_moves (B, ML, source); break; 
    case Species::king:   generate_king_moves  (B, ML, source); break; 
    }
}
void generate_moves(Board const* B, MoveList* ML, Color next_to_move)
{
    ML->length = 0;
    for (int r=0; r!=8; ++r) {
        for (int c=0; c!=8; ++c) {
            Piece mover = get_piece(B, {r, c});
            if (mover.color != next_to_move) { continue; }
            generate_moves_for_piece(B, ML, mover, {r, c});
        }
    }
}  
void generate_moves(Board const* B, MoveList* ML)
{
    generate_moves(B, ML, B->next_to_move);
}

bool add_move_color_guard(Board const* B, MoveList* ML, Coordinate source, Coordinate dest, Color needed_color)
{
    Piece taken = get_piece(B, dest);
    if (is_valid(dest) && taken.color == needed_color) {
        ML->moves[ML->length++] = {source, dest, taken, MoveType::ordinary}; 
        return true;
    }
    return false;
} 

bool add_pawn_move_color_guard(Board const* B, MoveList* ML, Coordinate source, Coordinate dest, Color needed_color)
{
    Piece taken = get_piece(B, dest);
    if (is_valid(dest) && taken.color == needed_color) {
        Move m = {source, dest, taken, MoveType::ordinary};
        /* CAUTION: row "0" is farthest from white's camp */
        Piece mover = get_piece(B, source);
        if (mover.color==Color::white && dest.row==0 || 
            mover.color==Color::black && dest.row==7) {
            m.type = MoveType::promote_to_queen;
        } 
        ML->moves[ML->length++] = m; 
        return true;
    }
    return false;
}

void generate_pawn_moves  (Board const* B, MoveList* ML, Coordinate source)
{
    int row=source.row, col=source.col;
    int direction = B->next_to_move==Color::white ? -1 : +1;
    int start = B->next_to_move==Color::white ? 6 : 1;
    Coordinate dest = {row+direction, col}; 
    if (add_pawn_move_color_guard(B, ML, source, {row+direction, col}, Color::empty_color) &&
        row == start) {
        add_move_color_guard(B, ML, source, {row+2*direction, col}, Color::empty_color);
    }
    add_pawn_move_color_guard(B, ML, source, {row+direction, col+1}, flip_color(B->next_to_move));
    add_pawn_move_color_guard(B, ML, source, {row+direction, col-1}, flip_color(B->next_to_move));
}
void generate_knight_moves(Board const* B, MoveList* ML, Coordinate source)
{
    int row=source.row, col=source.col;
    for (int dr=-2; dr!=3; ++dr) {
        for (int dc=-2; dc!=3; ++dc) {
            if (dr*dr + dc*dc != 2*2 + 1*1) { continue; }  
            add_move_color_guard(B, ML, source, {row+dr, col+dc}, Color::empty_color);
            add_move_color_guard(B, ML, source, {row+dr, col+dc}, flip_color(B->next_to_move));
        }
    }
}
void generate_bishop_moves(Board const* B, MoveList* ML, Coordinate source)
{
    int row=source.row, col=source.col;
    for (int dr=-1; dr!=2; ++dr) {
        for (int dc=-1; dc!=2; ++dc) {
            if (dr*dr + dc*dc != 1*1 + 1*1) { continue; }
            for (int t=1; t!=8; ++t) {
                if (! add_move_color_guard(B, ML, source, {row+t*dr, col+t*dc}, Color::empty_color)) {
                    add_move_color_guard(B, ML, source, {row+t*dr, col+t*dc}, flip_color(B->next_to_move));
                    break;
                }
            }
        }
    }
}
void generate_rook_moves  (Board const* B, MoveList* ML, Coordinate source)
{
    int row=source.row, col=source.col;
    for (int dr=-1; dr!=2; ++dr) {
        for (int dc=-1; dc!=2; ++dc) {
            if (dr*dr + dc*dc != 1*1) { continue; }
            for (int t=1; t!=8; ++t) {
                if (! add_move_color_guard(B, ML, source, {row+t*dr, col+t*dc}, Color::empty_color)) {
                    add_move_color_guard(B, ML, source, {row+t*dr, col+t*dc}, flip_color(B->next_to_move));
                    break;
                }
            }
        }
    }
}
void generate_queen_moves (Board const* B, MoveList* ML, Coordinate source)
{
    int row=source.row, col=source.col;
    for (int dr=-1; dr!=2; ++dr) {
        for (int dc=-1; dc!=2; ++dc) {
            if (dr*dr + dc*dc == 0) { continue; }
            for (int t=1; t!=8; ++t) {
                if (! add_move_color_guard(B, ML, source, {row+t*dr, col+t*dc}, Color::empty_color)) {
                    add_move_color_guard(B, ML, source, {row+t*dr, col+t*dc}, flip_color(B->next_to_move));
                    break;
                }
            }
        }
    }
}
void generate_king_moves (Board const* B, MoveList* ML, Coordinate source)
{
    int row=source.row, col=source.col;
    for (int dr=-1; dr!=2; ++dr) {
        for (int dc=-1; dc!=2; ++dc) {
            if (dr*dr + dc*dc == 0) { continue; }
            add_move_color_guard(B, ML, source, {row+dr, col+dc}, Color::empty_color);
            add_move_color_guard(B, ML, source, {row+dr, col+dc}, flip_color(B->next_to_move));
        }
    }
}



int quintant_from(Coordinate rc)
{
    return quintant_by_coor[rc.row][rc.col];
}

void change_piece(Board* B, Coordinate rc, Piece p, bool is_add)
{
    int const sign = p.color==Color::white ? +1 : -1;
    Color const self = p.color; 
    Color const them = flip_color(p.color);
    int const r=rc.row;
    int const c=rc.col;

    int const sign_d = is_add ? +1 : -1;

    /* hash (TODO: EXCEPT to-move-color, 50 move rule, etc: extra state) */
    B->hash ^= hash_by_piece[self][p.species] * hash_by_square[r][c];

    /* update grid */
    if (is_add) { B->grid[r][c] = p; }
    else        { B->grid[r][c] = empty_piece; }

    if (p.species != Species::pawn && p.species != Species::king) {
        int pq = quintant_from(rc);
        int kq = quintant_from(B->king_locs[them].back());
        B->nb_pieces_by_quintant[self][pq] += sign_d; 
        { /* king-xrays */
            /* TODO */
        }
        { /* loose pieces */
        }
    }

    /* update piece-square counts and pawn structure terms */
    switch (p.species) {
    break; case Species::pawn:
        B->nb_pawns_by_parity[self][parity(rc)] += sign_d;
        B->nb_pawns_by_file[self][c] += sign_d;
        update_least_advanced(B, self, c);
        { /* weak-squares, posted-knight */
            if (c != 0) { B->attacks_by_pawn[self][r-sign][c-1] += sign_d; }
            if (c != 7) { B->attacks_by_pawn[self][r-sign][c+1] += sign_d; }
            /* TODO; note that order of operations might depend on is_add */
        }
        { /* king x-rays */
        }
        { /* loose pieces */
        }
    //break; case Species::knight:
    break; case Species::bishop:
        B->nb_pawns_by_parity[self][parity(rc)] += sign_d;
    break; case Species::rook:
        B->nb_rooks_by_file[self][c] += sign_d;
        B->nb_majors[self] += sign_d;
    break; case Species::queen:
        B->nb_majors[self] += sign_d;
    //break; case Species::king: 
    }
}

