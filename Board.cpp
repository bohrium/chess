#include "Board.h"
#include <iostream>

Color flip_color(Color c)
{
    return c==Color::white ? Color::black : Color::white; 
}

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

char letters[] = "PNBRQK ";
void print_board(Board const* B)
{
    if (B->next_to_move == Color::white) {
        std::cout << "\tWhite to move" << std::endl;
    } else {
        std::cout << "\tBlack to move" << std::endl;
    }
     
    std::cout << "\t   a b c d e f g h" << std::endl;
    for (int r=0; r!=8; ++r) {
        std::cout << "\t" << 8-r << " |";
        for (int c=0; c!=8; ++c) {
            char l = letters[B->grid[r][c].species];
            if (B->grid[r][c].color==Color::black) {
                l += 'a'-'A'; // lowercase
                std::cout << "\033[36;1m"; /* black is cyan*/
            } else {
                std::cout << "\033[35;1m"; /* white is magenta */
            }
            std::cout << l << "\033[0;33m|"; 
        }
        std::cout << std::endl;
    }
    std::cout << "\t   ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾ " << std::endl;
} 
/*
        White to move
           a b c d e f g h
        8 | | | | | | |k| |
        7 | | | | | |p|p|p|
        6 | | | | | | | | |
        5 | | | | | | | | |
        4 | | | | | | | | |
        3 | | | | | | | | |
        2 | | |R| |K| | | |
        1 | | | | | | | | |
           ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
*/
void skip_white(char const** string) {
    while (**string==' ' || **string=='\t' || **string=='\n') { ++*string; } 
}
void to_next_line(char const** string) {
    while (**string!='\n') { ++*string; }
    ++*string;
}
bool read_board(Board* B, char const* string) /* returns TRUE if error */
{
    skip_white(&string);
    if (*string=='W') { B->next_to_move = Color::white; }
    else if (*string=='B') { B->next_to_move = Color::black; }
    else { return true; /* ERROR */ }
    to_next_line(&string); /* go to "   a b c" etc */
    for (int r=0; r!=8; ++r) {
        to_next_line(&string); /* go to "8 |r|n|b" etc */
        skip_white(&string);
        string += 1; /* skip "8" etc */ 
        for (int c=0; c!=8; ++c) {
            string += 2; /*skip " |" etc */ 
            if (*string==' ') {
                B->grid[r][c] = empty_piece;
                continue;
            }  
            Color color = (*string<'a') ? Color::white : Color::black;
            Species species;
            switch ((char)(*string + ((*string<'a') ? 0 : 'A'-'a'))) {
            case 'P': species = Species::pawn;   break;
            case 'N': species = Species::knight; break;
            case 'B': species = Species::bishop; break;
            case 'R': species = Species::rook;   break;
            case 'Q': species = Species::queen;  break;
            case 'K': species = Species::king;   break;
            }
            B->grid[r][c] = {color, species}; 
        }
    } 
    return false;
}

bool is_valid(Coordinate coor)
{
    return (0<=coor.row && coor.row<8) &&
           (0<=coor.col && coor.col<8); 
}
inline Piece get_piece(Board const* B, Coordinate coor)
{
    return B->grid[coor.row][coor.col]; 
} 

void apply_move(Board* B, Move M)
{
    B->next_to_move = flip_color(B->next_to_move);
    B->grid[M.dest.row][M.dest.col] = B->grid[M.source.row][M.source.col];
    B->grid[M.source.row][M.source.col] = empty_piece;
}
void undo_move(Board* B, Move M)
{
    B->next_to_move = flip_color(B->next_to_move);
    B->grid[M.source.row][M.source.col] = B->grid[M.dest.row][M.dest.col];
    B->grid[M.dest.row][M.dest.col] = M.taken;
}

void print_move(Board const* B, Move M)
{
    char mover = letters[get_piece(B, M.source).species];
    std::cout << mover << (char)(M.dest.col+'a') << 8-M.dest.row;
    std::cout << "(" << letters[M.taken.species] << ")";
}
void print_movelist(Board const* B, MoveList* ML)
{
    for (int i=0; i!=ML->length; ++i) {
        print_move(B, ML->moves[i]);
        std::cout << std::endl;
    }
}

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

               /* p    n    b     r    q    k      */
float points[] = {1.0, 3.0, 3.25, 5.0, 9.0, 1000.0};
float evaluate(Board const* B) /* TODO: symmetrize aggression score */
{
    float material=0.0, centrality=0.0, aggression=0.0;
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

    MoveList ML;
    generate_moves(B, &ML);
    for (int i=0; i!=ML.length; ++i) {
        Move m = ML.moves[i];
        Color s = m.taken.color;
        if (s == Color::empty_color) { continue; }
        int sign = (s == Color::white) ? -1.0 : +1.0;  
        int is_threat = points[get_piece(B, m.dest).species] > points[get_piece(B, m.source).species] ? +1.0 : 0.0;
        aggression += sign * is_threat; 
    }

    return material + 1.5 * centrality + 1.0 * aggression;
}
