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
    B->evaluation_stack.push_back(0.0); /* initial evaluation */ 
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
            std::cout << l << "\033[0;33m|"; /* yellow */
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
    B->evaluation_stack.push_back(B->evaluation_stack.back() + evaluation_difference(B, M));
    B->grid[M.dest.row][M.dest.col] = B->grid[M.source.row][M.source.col];
    B->grid[M.source.row][M.source.col] = empty_piece;
}
void undo_move(Board* B, Move M)
{
    B->next_to_move = flip_color(B->next_to_move);
    B->evaluation_stack.pop_back();
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
    Coordinate dest = {row+direction, col}; 
    if (add_move_color_guard(B, ML, source, {row+direction, col}, Color::empty_color) &&
        row == start) {
        add_move_color_guard(B, ML, source, {row+2*direction, col}, Color::empty_color);
    }
    add_move_color_guard(B, ML, source, {row+direction, col+1}, flip_color(B->next_to_move));
    add_move_color_guard(B, ML, source, {row+direction, col-1}, flip_color(B->next_to_move));
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


int KING_POINTS = 10000; /* should exceed twice the total remaining value */ 
              /* p    n    b     r    q    k      */
int points[] = {100, 275, 325, 500, 900, KING_POINTS};
/* note: pawn, bishop, rook placements are asymmetrical*/
//int _X=-40,_x=-15,_o=+15,_O=+40; 
int _X=-40,_x=-16,_o=+16,_O=+40; 

int piece_placement[][8][8] = {
    /*pawn*/ {
        { 0, 0, 0, 0, 0, 0, 0, 0},
        {_O,_O,_O,_O,_O,_O,_O,_O},
        { 0,_o,_o,_o,_o,_o,_o, 0},
        { 0, 0, 0,_o,_o, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0, 0, 0},
        { 0, 0,_x, 0, 0,_x, 0, 0},
        { 0, 0, 0,_x,_x, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0, 0, 0}, 
    },
    /*knight*/ { 
        {_X,_X,_X,_X,_X,_X,_X,_X},
        {_X,_x, 0, 0, 0, 0,_x,_X},
        {_X, 0, 0,_o,_o, 0, 0,_X},
        {_X, 0,_o,_o,_o,_o, 0,_X},
        {_X, 0,_o,_o,_o,_o, 0,_X},
        {_X, 0, 0,_o,_o, 0, 0,_X},
        {_X,_x, 0, 0, 0, 0,_x,_X},
        {_X,_X,_X,_X,_X,_X,_X,_X},
    },
    /*bishop*/ {
        {_X,_X,_X,_X,_X,_X,_X,_X},
        {_X,_x,_x,_x,_x,_x,_x,_X},
        {_X,_x,_x, 0, 0,_x,_x,_X},
        {_X,_x,_x, 0, 0,_x,_x,_X},
        {_X,_x, 0, 0, 0, 0,_x,_X},
        {_X, 0, 0, 0, 0, 0, 0,_X},
        {_X,_x, 0, 0, 0, 0,_x,_X},
        {_X,_X,_X,_X,_X,_X,_X,_X},
    },
    /*rook*/ {
        { 0, 0, 0, 0, 0, 0, 0, 0},
        { 0,_o,_o,_o,_o,_o,_o, 0},
        { 0, 0, 0, 0, 0, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0, 0, 0}, 
    },
    /*queen*/ {
        {_x,_x,_x, 0, 0,_x,_x,_x},
        {_x, 0, 0, 0, 0, 0, 0,_x},
        {_x, 0, 0, 0, 0, 0, 0,_x},
        { 0, 0, 0, 0, 0, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0, 0, 0},
        {_x, 0, 0, 0, 0, 0, 0,_x},
        {_x, 0, 0, 0, 0, 0, 0,_x},
        {_x,_x,_x, 0, 0,_x,_x,_x},
    },
    /*king*/ { /* TODO: fill in */
        { 0, 0, 0, 0, 0, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0, 0, 0},
    },
};

int king_safety(Board* B) 
{
    int nb_attackers[2][8][8]; 
    for (int r=0; r!=8; ++r) {
        for (int c=0; c!=8; ++c) {
            nb_attackers[Color::black][r][c] = 0; /* black */
            nb_attackers[Color::white][r][c] = 0; /* white */
        } 
    }
    Coordinate king_locations[2];
    for (int r=0; r!=8; ++r) {
        for (int c=0; c!=8; ++c) {
            Piece p = get_piece(B, {r,c});
            if (p.species == Species::empty_species) { continue; }
            if (p.species == Species::king) { king_locations[p.color] = {r,c}; }
            switch (p.species) {
                case Species::knight:
                    for (int dr=-2; dr!=3; ++dr) {
                        for (int dc=-2; dc!=3; ++dc) {
                            if (dr*dr + dc*dc != 2*2 + 1*1) { continue; }  
                            if (! (0<=r+dr && r+dr<8 && 0<=c+dc && c+dc<8)) { continue; }  
                            nb_attackers[p.color][r+dr][c+dc] += 1; 
                        }
                    }
                    break;
                case Species::bishop:
                    for (int dr=-1; dr!=2; ++dr) {
                        for (int dc=-1; dc!=2; ++dc) {
                            if (dr*dr + dc*dc != 1*1 + 1*1) { continue; }
                            for (int t=1; t!=8; ++t) {
                                if (! (0<=r+t*dr && r+t*dr<8 && 0<=c+t*dc && c+t*dc<8)) { break; }  
                                nb_attackers[p.color][r+t*dr][c+t*dc] += 1; 
                            }
                        }
                    }
                    break;
                case Species::rook:
                    for (int dr=-1; dr!=2; ++dr) {
                        for (int dc=-1; dc!=2; ++dc) {
                            if (dr*dr + dc*dc != 1*1) { continue; }
                            for (int t=1; t!=8; ++t) {
                                if (! (0<=r+t*dr && r+t*dr<8 && 0<=c+t*dc && c+t*dc<8)) { break; }  
                                nb_attackers[p.color][r+t*dr][c+t*dc] += 1; 
                            }
                        }
                    }
                    break;
                case Species::queen:
                    for (int dr=-1; dr!=2; ++dr) {
                        for (int dc=-1; dc!=2; ++dc) {
                            if (dr*dr + dc*dc == 0) { continue; }
                            for (int t=1; t!=8; ++t) {
                                if (! (0<=r+t*dr && r+t*dr<8 && 0<=c+t*dc && c+t*dc<8)) { break; }  
                                nb_attackers[p.color][r+t*dr][c+t*dc] += 1; 
                            }
                        }
                    }
                    break;
                default: break;
            }
        }
    }
    int score = 0;
    for (int color = 0; color != 2; ++color) {
        int r = king_locations[color].row;
        int c = king_locations[color].col;
        for (int dr=-1; dr!=2; ++dr) {
            for (int dc=-1; dc!=2; ++dc) {
                //if (dr*dr + dc*dc == 0) { continue; }
                if (! (0<=r+dr && r+dr<8 && 0<=c+dc && c+dc<8)) { continue; }  
                score += (color==Color::black ? +1 : -1) * nb_attackers[1-color][r+dr][c+dc];
            }
        }
    }
    return 15 * score;
}

float evaluate(Board* B) /*TODO: constify*/
{
    return B->evaluation_stack.back() + king_safety(B);
}

float evaluation_difference(Board* B, Move m) /*TODO: constify*/ // assumes m has not yet been applied to B 
{
    int material  = 0;
    int placement = 0;
    //int safety = 0;

    Piece mover = get_piece(B, m.source);
    int sign = (mover.color==Color::white ? +1 : -1);

    /* material */ 
    if (m.taken.species != Species::empty_species) {
        material = sign * points[m.taken.species];
    }

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

    return material + placement;
}
