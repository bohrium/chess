/* author : samtenka
 * changed: 2018-05-25
 * created: 2018-05-24
 * descrp : define chess move functionality
 */

#include "Piece.h"
#include "Board.h"
#include <vector>


enum MoveType {
    ordinary,
    pawn_double_jump,
    pawn_en_passant,
    castle,
    king_en_passant,
    promotion, // we only promote to queens
};

struct Move {
    MoveType type;

    Coordinate src, dst;

    // For ordinary move, is empty_piece or taken piece;
    Piece taken;

    // 8-bit flag for files of pieces participating in castle (0 for non-castling moves): 
    char castle_participants;
};

void apply_move(Board* B, Move const* M) {
    History hist;

    for (int i=0; i!=2; ++i) {
        hist.back_changed[i] = B->history.back().back_changed[i];
    }
    hist.castled_through = 0;
    hist.en_passantable = 0;

    switch (M->type) {
    case MoveType::ordinary:
        if (M->src.row == row_from_color(B->next_to_move)) {
            hist.back_changed[index_from_color(B->next_to_move)] |= (1 << (7-M->src.col)); 
        }
        break;
    case MoveType::pawn_double_jump:
        hist.en_passantable = (1 << (7-M->src.col));
        break;
    case MoveType::pawn_en_passant:
        set_piece(B, {M->dst.row + M->taken.color, M->dst.col}, empty_piece);
        break;
    case MoveType::castle:
        int row = row_from_color(B->next_to_move);  
        if (M->castle_participants == 0b10001000) { // queenside
            hist.castled_through = 0b00111000; 
            set_piece(B, {0, row}, empty_piece);
            set_piece(B, {3, row}, {B->next_to_move, Species::rook}); 
        } else if (M->castle_participants == 0b00001001) { // kingside
            hist.castled_through = 0b00001110; 
            set_piece(B, {7, row}, empty_piece);
            set_piece(B, {4, row}, {B->next_to_move, Species::rook}); 
        }
        hist.back_changed[index_from_color(B->next_to_move)] = 0b11111111;
        break;
    case MoveType::king_en_passant:
        break;
    case MoveType::promotion:
        set_piece(B, M->dst, {B->next_to_move, Species::queen});
        break;
    }

    Piece p = get_piece(B, M->src);
    set_piece(B, M->dst, p);
    set_piece(B, M->src, empty_piece);

    B->next_to_move = flip_color(B->next_to_move);
    B->history.push_back(hist);
}
void undo_move(Board* B, Move const* M) {
    B->history.pop_back();
    B->next_to_move = flip_color(B->next_to_move);

    set_piece(B, M->src, get_piece(B, M->dst));
    set_piece(B, M->dst, M->taken);

    switch (M->type) {
    case MoveType::ordinary:
        break;

    case MoveType::king_en_passant:
        break;
    case MoveType::castle:
        int row = row_from_color(B->next_to_move);  
        if (M->castle_participants == 0b10001000) { // queenside
            set_piece(B, {3, row}, empty_piece); 
            set_piece(B, {0, row}, {B->next_to_move, Species::rook}); 
        } else if (M->castle_participants == 0b00001001) { // kingside
            set_piece(B, {4, row}, empty_piece); 
            set_piece(B, {7, row}, {B->next_to_move, Species::rook}); 
        }
        break;

    case MoveType::pawn_double_jump:
        break;
    case MoveType::pawn_en_passant:
        set_piece(B, M->dst, empty_piece);
        set_piece(B, {M->dst.row + M->taken.color, M->dst.col}, M->taken);
        break;

    case MoveType::promotion:
        set_piece(B, M->dst, {B->next_to_move, Species::pawn});
        break;
    }
}

void print_move(Board const* B, Move const* M);
struct MoveList {
    std::vector<Move> moves;
};
void print_movelist(Board const* B, MoveList* ML);

int const nb_knight_directions = 8; 
int const knight_directions[8][2] = {{-2, -1}, {-1, -2}, {-2, +1}, {+1, -2}, {+2, -1}, {-1, +2}, {+2, +1}, {+1, +2}}; 
int const nb_bishop_directions = 4; 
int const bishop_directions[4][2] = {{-1, -1}, {-1, +1}, {+1, -1}, {+1, +1}}; 
int const nb_rook_directions = 4; 
int const rook_directions[4][2] = {{0, -1}, {0, +1}, {-1, 0}, {+1, 0}}; 

void generate_ray_moves(Board const* B, MoveList* ML, Coordinate source, int nb_directions, int directions[][2]) {
    Color player = B->get_piece(source).color; 
    Color opponent = flip_color(player);
    for (int d=0; d!=nb_directions; ++d) {
        int dr = directions[d][0];
        int dc = directions[d][1];
        int r = source->row;
        int c = source->col;
        do {
            r += dr;
            c += dc; 
            if (!is_valid({r, c})) { break; }
            Piece taken = get_piece(B, {r, c});
            if (taken.color == player) { break; }
            ML->moves.push_back({MoveType::ordinary, source, {r,c}, taken, 0}); 
            if (taken.color == opponent) { break; }
        } while (true);
    }
}
void generate_point_moves(Board const* B, MoveList* ML, Coordinate source, int nb_directions, int directions[][2]) {
    Color player = B->get_piece(source).color; 
    Color opponent = flip_color(player);
    for (int d=0; d!=nb_directions; ++d) {
        int r = source->row + directions[d][0];
        int c = source->col + directions[d][1];
        if (!is_valid({r, c})) { continue; }
        Piece taken = get_piece(B, {r, c});
        if (taken.color == player) { continue; }
        ML->moves.push_back({MoveType::ordinary, source, {r,c}, taken, 0}); 
    }
}

void generate_pawn_moves  (Board const* B, MoveList* ML, Coordinate source) {
    Color player = B->get_piece(source).color; 
    Color opponent = flip_color(player);

    int r=source->row;
    int c=source->col;

    int init_row = row_from_color(player) - player;
    int promotion_row = 7 - init_row;
    MoveType single_jump = (r==promotion_row ? MoveType::promotion : MoveType::ordinary);

    // advancing moves:
    if (get_piece(B, {r+player, c}).color == empty_color) {
        ML->moves.push_back({single_jump, source, {r+player,c}, taken, 0}); 
        if (r==init_row && get_piece(B, {r+2*player, c}).color == empty_color) {
            ML->moves.push_back({MoveType::pawn_double_jump, source, {r+2*player,c}, taken, 0}); 
        }
    }

    // taking moves:
    for (int dc=-1; dc!=+3; dc+=2) {
        Piece taken = get_piece(B, {r+player, c+dc});
        if (taken.color == opponent) {
            ML->moves.push_back({MoveType::single_jump, source, {r+player,c+dc}, taken, 0}); 
        }
    } 
}
void generate_knight_moves(Board const* B, MoveList* ML, Coordinate source)
{
    generate_point_moves(B, ML, source, nb_knight_directions, knight_directions);
}
void generate_bishop_moves(Board const* B, MoveList* ML, Coordinate source) 
{
    generate_ray_moves(B, ML, source, nb_bishop_directions, bishop_directions);
}
void generate_rook_moves  (Board const* B, MoveList* ML, Coordinate source) 
{
    generate_ray_moves(B, ML, source, nb_rook_directions, rook_directions);
}
void generate_queen_moves (Board const* B, MoveList* ML, Coordinate source)
{
    generate_ray_moves(B, ML, source, nb_bishop_directions, bishop_directions);
    generate_ray_moves(B, ML, source, nb_rook_directions, rook_directions);
}
void generate_king_moves  (Board const* B, MoveList* ML, Coordinate source)
{
    generate_point_moves(B, ML, source, nb_bishop_directions, bishop_directions);
    generate_point_moves(B, ML, source, nb_rook_directions, rook_directions);

    int row = row_from_color(B->next_move); 
    char back_changed = B->history.back().back_changed[index_from_color(B->next_to_move)];
    if (back_changed | 0b10001000) { // queenside
        ML->moves.push_back({MoveType::castle, source, {row, 2}, empty_piece, 0b10001000}); 
    } else if (back_changed | 0b00001001) { // kingside 
        ML->moves.push_back({MoveType::castle, source, {row, 6}, empty_piece, 0b00001001}); 
    }
}
void generate_pawn_en_passant (Board const* B, MoveList* ML)
{
    char ep = B->history.back().en_passantable; 
    if (!ep) { return; }
    
    Color player = B->next_to_move;
    Color opponent = B->flip_color(player);
    int r = row_from_color(opponent) + 3*opponent;

    for (int c=0; c!=8; ++c) {
        Piece p = get_piece(B, {row, c});
        if (p.species != Species:pawn && p.color==player) { continue; }
        for (dc = -1; dc!=3; dc+=2) {
            if ((1 << (7-(c+dc))) == ep) {
                ML->moves.push_back({MoveType::pawn_en_passant, {r, c+dc}, {r+player, c}, {opponent, Species::pawn}, 0}); 
            }
        }
    } 
}
void generate_king_en_passant (Board const* B, MoveList* ML)
    // scans through move list so far, so should be done *once* and *last*
{
    char ct = B->history.back().castled_through; 
    if (!ct) { return; }

    Color opponent = flip_color(B->next_to_move);
    int r = row_from_color(opponent);

    for (auto it=ML.begin(); it!=ML.end(); ++it) {
        if (it->dst.row==row && ((1 << 7-it->dst.col)|castled_through)) {
            ML->moves.push_back({MoveType::king_en_passant, it->src, {r, ct==0b00111000 ? 2 : 6}, {opponent, Species::king}, 0}); 
        }
    }
}

void generate_moves(Board const* B, MoveList* ML)
{
    for (int r=0; r!=8; ++r) {
        for (int c=0; c!=8; ++c) {
            Piece p = get_piece(B, {r,c});
            if (p.color!=B->next_to_move) { continue; }
            switch (p.species) {
            case Species::pawn:
                generate_pawn_moves(B, ML, {r,c});
                break;
            case Species::knight:
                generate_knight_moves(B, ML, {r,c});
                break;
            case Species::bishop:
                generate_bishop_moves(B, ML, {r,c});
                break;
            case Species::rook:
                generate_rook_moves(B, ML, {r,c});
                break;
            case Species::queen:
                generate_queen_moves(B, ML, {r,c});
                break;
            case Species::king:
                generate_king_moves(B, ML, {r,c});
                break;
            } 
        }
    }
    generate_pawn_en_passant(B, ML);
    generate_king_en_passant(B, ML);
}
