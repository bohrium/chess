#ifndef BOARD_H
#define BOARD_H

#include <iostream>

enum Color {
    black,
    white,
    empty_color
};
Color flip_color(Color c)
{
    return c==Color::white ? Color::black : Color::white; 
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
struct Board {
    Color next_to_move;
    Piece grid[8][8];
};

Species init_row[] = {
    Species::rook,
    Species::knight,
    Species::bishop,
    Species::queen,
    Species::king,
    Species::bishop,
    Species::knight,
    Species::rook
};

void init_board(Board* B)
{
    B->next_to_move = Color::white;
    for (int c=0; c!=8; ++c) {
        B->grid[0][c] = {Color::black, init_row[c]};
        B->grid[1][c] = {Color::black, Species::pawn};
        for (int r=2; r!=6; ++r) {
            B->grid[r][c] = empty_piece;
        }
        B->grid[6][c] = {Color::white, Species::pawn};
        B->grid[7][c] = {Color::white, init_row[c]};
    }
}

char letters[] = "pnbrqk ";
void print_board(Board const* B)
{
    if (B->next_to_move == Color::white) {
        std::cout << "\tWhite to move" << std::endl;
    } else {
        std::cout << "\tBlack to move" << std::endl;
    }
     
    std::cout << "\t\033[0;33m _______________ " << std::endl;
    for (int r=0; r!=8; ++r) {
        std::cout << "\t|";
        for (int c=0; c!=8; ++c) {
            char l = letters[B->grid[r][c].species];
            if (B->grid[r][c].color==Color::white) {
                l += 'A'-'a'; // capitalize
                std::cout << "\033[35;1m";
            } else {
                std::cout << "\033[36;1m";
            }
            std::cout << l << "\033[0;33m|"; 
        }
        std::cout << std::endl;
    }
    std::cout << "\t ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾ " << std::endl;
} 

struct Coordinate {
    int row, col; 
};
bool is_valid(Coordinate coor)
{
    return (0<=coor.row && coor.row<8) &&
           (0<=coor.col && coor.col<8); 
}
Piece get_piece(Board const* B, Coordinate coor)
{
    return B->grid[coor.row][coor.col]; 
} 
struct Move { // a standard move (no promotion, en passant, or castling)
    Coordinate source, dest; 
    Piece taken;
};

void apply_move(Board* B, Move M)
{
    B->next_to_move = flip_color(B->next_to_move);
    B->grid[M.dest.row][M.dest.col] = B->grid[M.source.row][M.source.col];
    B->grid[M.source.row][M.source.col] = empty_piece;
};
void undo_move(Board* B, Move M)
{
    B->next_to_move = flip_color(B->next_to_move);
    B->grid[M.source.row][M.source.col] = B->grid[M.dest.row][M.dest.col];
    B->grid[M.dest.row][M.dest.col] = M.taken;
};

struct MoveList {
    int length;
    // A chess position can have at most 332 possible next moves:
    // Consider a board with 9 queens, 2 rooks, 2 bishops, 2 knights, 1 king
    Move moves[9*4*7 + 2*2*7 + 2*2*7 + 2*8 + 1*8];
};
void print_move(Move M)
{
    std::cout << M.source.row << M.source.col << M.dest.row << M.dest.col;
    std::cout << "(" << letters[M.taken.species] << ")" << std::endl;
}
void print_movelist(MoveList* ML)
{
    for (int i=0; i!=ML->length; ++i) {
        print_move(ML->moves[i]);
    }
}

void generate_pawn_moves  (Board const* B, MoveList* ML, Coordinate source);
void generate_knight_moves(Board const* B, MoveList* ML, Coordinate source);
void generate_bishop_moves(Board const* B, MoveList* ML, Coordinate source);
void generate_rook_moves  (Board const* B, MoveList* ML, Coordinate source);
void generate_queen_moves (Board const* B, MoveList* ML, Coordinate source);
void generate_king__moves (Board const* B, MoveList* ML, Coordinate source);
void generate_moves(Board const* B, MoveList* ML)
{
    ML->length = 0;
    for (int r=0; r!=8; ++r) {
        for (int c=0; c!=8; ++c) {
            Piece mover = B->grid[r][c];
            if (mover.color != B->next_to_move) { continue; }
            switch (mover.species) {
            case Species::pawn:   generate_pawn_moves  (B, ML, {r, c}); break; 
            case Species::knight: generate_knight_moves(B, ML, {r, c}); break; 
            case Species::bishop: generate_bishop_moves(B, ML, {r, c}); break; 
            case Species::rook:   generate_rook_moves  (B, ML, {r, c}); break; 
            case Species::queen:  generate_queen_moves (B, ML, {r, c}); break; 
            case Species::king:   generate_king__moves (B, ML, {r, c}); break; 
            }     
        }
    }
}  

bool add_move(Board const* B, MoveList* ML, Coordinate source, Coordinate dest, Color needed_color)
{
    if (is_valid(dest) && get_piece(B, dest).color == needed_color) {
        ML->moves[ML->length++] = {source, dest, get_piece(B, dest)}; 
        return true;
    }
    return false;
} 
void generate_pawn_moves  (Board const* B, MoveList* ML, Coordinate source)
{
    int row=source.row, col=source.col;
    int direction = B->next_to_move==Color::white ? -1 : +1;
    int start = B->next_to_move==Color::white ? 6 : 1;
    if (! (0<=row+direction && row+direction<8)) { return; }
    Coordinate dest = {row+direction, col}; 
    if (add_move(B, ML, source, {row+direction, col}, Color::empty_color)) {
        if (row==start) {
            add_move(B, ML, source, {row+2*direction, col}, Color::empty_color);
        }
    }
    add_move(B, ML, source, {row+direction, col+1}, flip_color(B->next_to_move));
    add_move(B, ML, source, {row+direction, col-1}, flip_color(B->next_to_move));
}
void generate_knight_moves(Board const* B, MoveList* ML, Coordinate source)
{
    int row=source.row, col=source.col;
    for (int dr=-2; dr!=3; ++dr) {
        for (int dc=-2; dc!=3; ++dc) {
            if (dr*dr + dc*dc != 2*2 + 1*1) { continue; }  
            add_move(B, ML, source, {row+dr, col+dc}, Color::empty_color);
            add_move(B, ML, source, {row+dr, col+dc}, flip_color(B->next_to_move));
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
                if (! add_move(B, ML, source, {row+t*dr, col+t*dc}, Color::empty_color)) {
                    add_move(B, ML, source, {row+t*dr, col+t*dc}, flip_color(B->next_to_move));
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
                if (! add_move(B, ML, source, {row+t*dr, col+t*dc}, Color::empty_color)) {
                    add_move(B, ML, source, {row+t*dr, col+t*dc}, flip_color(B->next_to_move));
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
                if (! add_move(B, ML, source, {row+t*dr, col+t*dc}, Color::empty_color)) {
                    add_move(B, ML, source, {row+t*dr, col+t*dc}, flip_color(B->next_to_move));
                    break;
                }
            }
        }
    }
}
void generate_king__moves (Board const* B, MoveList* ML, Coordinate source)
{
    int row=source.row, col=source.col;
    for (int dr=-1; dr!=2; ++dr) {
        for (int dc=-1; dc!=2; ++dc) {
            if (dr*dr + dc*dc == 0) { continue; }
            add_move(B, ML, source, {row+dr, col+dc}, Color::empty_color);
            add_move(B, ML, source, {row+dr, col+dc}, flip_color(B->next_to_move));
        }
    }
}

               /* p    n     b    r    q    k      */
float points[] = {1.0, 2.75, 3.0, 5.0, 9.0, 100.0};
float evaluate(Board const* B)
{
    float material=0.0, centrality=0.0;
    for (int r=0; r!=8; ++r) {
        for (int c=0; c!=8; ++c) {
            Piece p = get_piece(B, {r,c});
            if (p.color == Color::empty_color) { continue; }
            int sign = (p.color==Color::white ? +1 : -1);

            material += sign * points[p.species];
            float dr=r-3.5, dc=c-3.5;
            centrality += sign * (1.0 - (dr*dr + dc*dc)/(2*3.5*3.5)); 
        }
    }

    return material + 0.01 * centrality;
}

#endif//BOARD_H
