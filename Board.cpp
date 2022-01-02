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

bool is_capture(Move m)
{
    return m.taken.species != Species::empty_species;
}

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
    B->hash = 0;

    B->king_loc[0] = {0, 4};
    B->king_loc[1] = {7, 4};

    for (int color=0; color!=2; ++color) {
        for (int c=0; c!=8; ++c) {
            B->nb_pawns_by_file[color][c] = 1;
        }
        for (int q=0; q!=4; ++q) {
            B->nb_pieces_by_quadrant[color][q] = (q&2)^(color<<1) ? 0 : 8;
        }
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
    for (int color=0; color!=2; ++color) {
        for (int c=0; c!=8; ++c) {
            std::cout << B->nb_pawns_by_file[color][c] << ".";
        }
        std::cout << " ";
        for (int q=0; q!=4; ++q) {
            std::cout << B->nb_pieces_by_quadrant[color][q] <<".";
        }
        std::cout << std::endl;
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
    /* TODO: compute hash!! (and other associated precomputed vals!)  */
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

unsigned int hash_by_piece[3][7] = {
  {1234567, 2345671, 3456712, 45767123, 5671234, 6712345, 7123456},
  {3456701, 4567101, 5671201, 76712301, 7123401, 1234501, 2345601},
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

int quadrant_by_coor[8][8] = {
    {0,0,0,0,1,1,1,1},
    {0,0,0,0,1,1,1,1},
    {0,0,0,0,1,1,1,1},
    {0,0,0,0,1,1,1,1},
    {2,2,2,2,3,3,3,3},
    {2,2,2,2,3,3,3,3},
    {2,2,2,2,3,3,3,3},
    {2,2,2,2,3,3,3,3},
};

void apply_move(Board* B, Move M)
{
    /* note asymmetry with analogous line in undo_move */
    Piece mover = get_piece(B, M.source);
    B->hash ^= hash_of(M, mover); 

    B->next_to_move = flip_color(B->next_to_move);
    B->evaluation_stack.push_back(B->evaluation_stack.back() + evaluation_difference(B, M));
    B->grid[M.dest.row][M.dest.col] = B->grid[M.source.row][M.source.col];
    B->grid[M.source.row][M.source.col] = empty_piece;

    if (mover.species == Species::king) {
        B->king_loc[mover.color] = M.dest;
    }

    B->nb_pieces_by_quadrant[mover.color][quadrant_by_coor[M.source.row][M.source.col]] -= 1; 
    B->nb_pieces_by_quadrant[mover.color][quadrant_by_coor[M.dest.row][M.dest.col]] += 1; 

    if (is_capture(M)) {
        B->nb_pieces_by_quadrant[M.taken.color][quadrant_by_coor[M.dest.row][M.dest.col]] -= 1; 
        if (M.taken.species == Species::pawn) {
            B->nb_pawns_by_file[M.taken.color][M.dest.col] -= 1;
        }
        if (mover.species == Species::pawn) {
            B->nb_pawns_by_file[mover.color][M.dest.col] += 1;
            B->nb_pawns_by_file[mover.color][M.source.col] -= 1;
        }
    }
}
void undo_move(Board* B, Move M)
{
    /* note asymmetry with analogous line in apply_move */
    Piece mover = get_piece(B, M.dest);
    B->hash ^= hash_of(M, mover); 

    B->next_to_move = flip_color(B->next_to_move);
    B->evaluation_stack.pop_back();
    B->grid[M.source.row][M.source.col] = B->grid[M.dest.row][M.dest.col];
    B->grid[M.dest.row][M.dest.col] = M.taken;

    if (mover.species == Species::king) {
        B->king_loc[mover.color] = M.source;
    }

    B->nb_pieces_by_quadrant[mover.color][quadrant_by_coor[M.source.row][M.source.col]] += 1; 
    B->nb_pieces_by_quadrant[mover.color][quadrant_by_coor[M.dest.row][M.dest.col]] -= 1; 

    if (is_capture(M)) {
        B->nb_pieces_by_quadrant[M.taken.color][quadrant_by_coor[M.dest.row][M.dest.col]] += 1; 
        if (M.taken.species == Species::pawn) {
            B->nb_pawns_by_file[M.taken.color][M.dest.col] += 1;
        }
        if (mover.species == Species::pawn) {
            B->nb_pawns_by_file[mover.color][M.dest.col] -= 1;
            B->nb_pawns_by_file[mover.color][M.source.col] += 1;
        }
    }
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
int XX=-89,_X=-34,xx=-13,_x=-5,_o=+5,oo=+13,_O=+34,OO=+89; 

int piece_placement[][8][8] = {
    /*pawn*/ {
        { 0, 0, 0, 0, 0, 0, 0, 0},
        {OO,OO,OO,OO,OO,OO,OO,OO},
        { 0,_O,_O,_O,_O,_O,_O, 0},
        { 0, 0, 0,oo,oo, 0, 0, 0},
        { 0, 0, 0,_o,_o, 0, 0, 0},
        { 0, 0,xx, 0, 0,xx, 0, 0},
        { 0, 0, 0,xx,xx, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0, 0, 0}, 
    },
    /*knight*/ { 
        {XX,xx,xx,xx,xx,xx,xx,XX},
        {xx,_x, 0, 0, 0, 0,_x,xx},
        {xx, 0, 0,_o,_o, 0, 0,xx},
        {xx, 0,_o,oo,oo,_o, 0,xx},
        {xx, 0,_o,oo,oo,_o, 0,xx},
        {xx, 0, 0,_o,_o, 0, 0,xx},
        {xx,_x, 0, 0, 0, 0,_x,xx},
        {XX,xx,xx,xx,xx,xx,xx,XX},
    },
    /*bishop*/ {
        {_X,_X,_X,_X,_X,_X,_X,_X},
        {_X,xx,xx,xx,xx,xx,xx,_X},
        {_X,xx,xx, 0, 0,xx,xx,_X},
        {_X,xx,_x, 0, 0,_x,xx,_X},
        {_X,xx, 0, 0, 0, 0,_x,_X},
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
        {xx,_x,_x, 0, 0,_x,_x,xx},
        {_x, 0, 0, 0, 0, 0, 0,_x},
        {_x, 0, 0, 0, 0, 0, 0,_x},
        { 0, 0, 0, 0, 0, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0, 0, 0},
        {_x, 0, 0, 0, 0, 0, 0,_x},
        {_x, 0, 0, 0, 0, 0, 0,_x},
        {xx,_x,_x, 0, 0,_x,_x,xx},
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

int king_tropism(Board* B)
{
    int quad[2] = {
        quadrant_by_coor[B->king_loc[0].row][B->king_loc[0].col],
        quadrant_by_coor[B->king_loc[1].row][B->king_loc[1].col],
    };
    return 15 * (
        B->nb_pieces_by_quadrant[1][quad[0]] /* white pieces near black king */
      - B->nb_pieces_by_quadrant[0][quad[1]] /* black pieces near white king */
    );
}

int pawn_connectivity(Board* B)
{
    return 25 * (
        (B->nb_pawns_by_file[1][0] * B->nb_pawns_by_file[1][1] ? 1 : 0)
      + (B->nb_pawns_by_file[1][1] * B->nb_pawns_by_file[1][2] ? 1 : 0)
      + (B->nb_pawns_by_file[1][2] * B->nb_pawns_by_file[1][3] ? 1 : 0)
      + (B->nb_pawns_by_file[1][3] * B->nb_pawns_by_file[1][4] ? 1 : 0)
      + (B->nb_pawns_by_file[1][4] * B->nb_pawns_by_file[1][5] ? 1 : 0)
      + (B->nb_pawns_by_file[1][5] * B->nb_pawns_by_file[1][6] ? 1 : 0)
      + (B->nb_pawns_by_file[1][6] * B->nb_pawns_by_file[1][7] ? 1 : 0)
      // 
      - (B->nb_pawns_by_file[0][0] * B->nb_pawns_by_file[0][1] ? 1 : 0)
      - (B->nb_pawns_by_file[0][1] * B->nb_pawns_by_file[0][2] ? 1 : 0)
      - (B->nb_pawns_by_file[0][2] * B->nb_pawns_by_file[0][3] ? 1 : 0)
      - (B->nb_pawns_by_file[0][3] * B->nb_pawns_by_file[0][4] ? 1 : 0)
      - (B->nb_pawns_by_file[0][4] * B->nb_pawns_by_file[0][5] ? 1 : 0)
      - (B->nb_pawns_by_file[0][5] * B->nb_pawns_by_file[0][6] ? 1 : 0)
      - (B->nb_pawns_by_file[0][6] * B->nb_pawns_by_file[0][7] ? 1 : 0)
    );        
}

int evaluate(Board* B) /*TODO: constify*/
{
    //return B->evaluation_stack.back() + king_safety(B);
    return B->evaluation_stack.back() + king_tropism(B) + pawn_connectivity(B);
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
