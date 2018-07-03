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
    castle,
    promotion,
    pawn_en_passant,
    king_en_passant
};

struct Move {
    MoveType type;

    Coordinate src, dst;

    // For ordinary move, is empty_piece or taken piece;
    // for promotion, is piece to which a pawn promotes:
    Piece taken;

    // 8-bit flag for files of pieces participating in castle (0 for non-castling moves): 
    char castle_participants;
};

void apply_move(Board* B, Move const* M) {
    switch (M->type) {
    case MoveType::ordinary:
        set_piece(B, M->dst, get_piece(B, M->src));
        set_piece(B, M->src, empty_piece);
        /* TODO: update castling allowed */
        break;
    case MoveType::castle:
        switch (M->castle_participants) {
        case 0b10001000:
        case 0b00001001:
        } 
        /* TODO: update castling allowed */
        break;
    case MoveType::promotion:
    case MoveType::pawn_en_passant:
    case MoveType::king_en_passant:
    }
}
void undo_move(Board* B, Move const* M) {
    switch (M->type) {
    case MoveType::ordinary:
        /* TODO: update castling allowed */
        set_piece(B, M->src, get_piece(B, M->dst));
        set_piece(B, M->dst, M->taken);
        break;
    case MoveType::castle:
    case MoveType::promotion:
    case MoveType::pawn_en_passant:
    case MoveType::king_en_passant:
    }

}

void print_move(Board const* B, Move const* M);
struct MoveList {
    std::vector<Move> moves;
};
void print_movelist(Board const* B, MoveList* ML);

int const nb_knight_directions = 8; 
int knight_directions[8][2] = {{-2, -1}, {-1, -2}, {-2, +1}, {+1, -2}, {+2, -1}, {-1, +2}, {+2, +1}, {+1, +2}}; 
int const nb_bishop_directions = 4; 
int bishop_directions[4][2] = {{-1, -1}, {-1, +1}, {+1, -1}, {+1, +1}}; 
int const nb_rook_directions = 4; 
int rook_directions[4][2] = {{0, -1}, {0, +1}, {-1, 0}, {+1, 0}}; 

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
            ML->moves.push({MoveType::ordinary, source, {r,c}, taken, 0}); 
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
        ML->moves.push({MoveType::ordinary, source, {r,c}, taken, 0}); 
    }
}

void generate_pawn_moves  (Board const* B, MoveList* ML, Coordinate source) {
    Color player = B->get_piece(source).color; 
    Color opponent = flip_color(player);

    int r=source->row;
    int c=source->col;

    // advancing moves:
    int init_row = (7 - player*5)/2; // 1 if white, 6 if black
    if (get_piece(B, {r+player, c}).color == empty_color) {
        ML->moves.push({MoveType::ordinary, source, {r+player,c}, taken, 0}); 
        if (r==init_row && get_piece(B, {r+2*player, c}).color == empty_color) {
            ML->moves.push({MoveType::ordinary, source, {r+2*player,c}, taken, 0}); 
        }
    }

    // taking moves:
    for (int dc=-1; dc!=+3; dc+=2) {
        Piece taken = get_piece(B, {r+player, c+dc});
        if (taken.color == opponent) {
            ML->moves.push({MoveType::ordinary, source, {r+player,c+dc}, taken, 0}); 
        }
    } 

    /* TODO: add promotion! */

    /* TODO: add en passant ? (actually, this should be in get_moves, since there we have more context) */
}
void generate_knight_moves(Board const* B, MoveList* ML, Coordinate source) {
    generate_point_moves(B, ML, source, nb_knight_directions, knight_directions);
}
void generate_bishop_moves(Board const* B, MoveList* ML, Coordinate source) 
    generate_ray_moves(B, ML, source, nb_bishop_directions, bishop_directions);
}
void generate_rook_moves  (Board const* B, MoveList* ML, Coordinate source) 
    generate_ray_moves(B, ML, source, nb_rook_directions, rook_directions);
}
void generate_queen_moves (Board const* B, MoveList* ML, Coordinate source) {
    generate_ray_moves(B, ML, source, nb_bishop_directions, bishop_directions);
    generate_ray_moves(B, ML, source, nb_rook_directions, rook_directions);
}
void generate_king_moves  (Board const* B, MoveList* ML, Coordinate source) {
    generate_point_moves(B, ML, source, nb_bishop_directions, bishop_directions);
    generate_point_moves(B, ML, source, nb_rook_directions, rook_directions);

    /* TODO: add castling? */
}
void generate_moves(Board const* B, MoveList* ML) {
    /* TODO */
    /* TODO: add king en passant? */
}
