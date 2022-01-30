#include "Board.h"
#include "Helpers.h"
#include "EvalParams.h"
#include <iostream>
#include <iomanip>

/*=============================================================================
====  0. GENERATORS  ==========================================================
=============================================================================*/

Board copy_board(Board B)
{
    Board rtrn = B; 
    rtrn.evaluation_stack = B.evaluation_stack; 
    rtrn.plies_since_irreversible = B.plies_since_irreversible; 
    return rtrn;
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

void zero_board(Board* B);
void add_piece_to_board(Board* B, Coordinate rc, Piece p)
{
    if (p.species == Species::king) {
        B->king_locs[p.color].back() = rc;
    }
    change_piece(B, rc, p, true);
    add_eval_diff(B, rc, p, true);
}

void init_board(Board* B)
{
    zero_board(B);
    for (int c=0; c!=8; ++c) {
        add_piece_to_board(B, {0,c}, {Color::black,init_row[c]  });
        add_piece_to_board(B, {1,c}, {Color::black,Species::pawn});
        add_piece_to_board(B, {6,c}, {Color::white,Species::pawn});
        add_piece_to_board(B, {7,c}, {Color::white,init_row[c]  });
    }
}

void zero_board(Board* B)
{
    B->next_to_move = Color::white;
    B->plies_since_irreversible.push_back(0);
    B->hash = 0;

    B->king_locs[0].push_back({-1, -1});
    B->king_locs[1].push_back({-1, -1});

    B->evaluation_stack.push_back(all_zeros); /* initial evaluation */ 

    for (int r=0; r!=8; ++r) {
        for (int c=0; c!=8; ++c) {
            B->grid[r][c] = empty_piece;
            for (int side=0; side!=2; ++side) {
                B->attacks_by_pawn[side][r][c] = 0;
                B->is_weak_square [side][r][c] = true;
                B->nb_xrays       [side][r][c] = 0;
            }
        }
    }

    for (int side=0; side!=2; ++side) {
        for (int c=0; c!=8; ++c) {
            B->nb_pawns_by_file[side][c] = 0;
            B->nb_rooks_by_file[side][c] = 0;
            B->least_advanced  [side][c] = side==Color::white ? 8 : -1;
        }
        for (int p=0; p!=2; ++p) {
            B->nb_pawns_by_parity  [side][p] = 0;
            B->nb_bishops_by_parity[side][p] = 0;
        }
        for (int q=0; q!=5; ++q) {
            B->nb_pieces_by_quintant[side][q] = 0;
        }
        B->nb_pawns            [side] =  0;
        B->nb_knights          [side] =  0;
        B->nb_rooks            [side] =  0;
        B->nb_majors           [side] =  0;
        B->nb_xrays_by_side    [side] =  0;
        B->nb_king_attacks_near[side] =  0;
        B->nb_weak_squares     [side] = 64;
    }
}

void skip_white(char const** str) {
    while (**str==' ' || **str=='\t' || **str=='\n') { ++*str; } 
}
void to_next_line(char const** str) {
    while (**str!='\n') { ++*str; }
    ++*str;
}
bool read_board(Board* B, char const* str) /* returns TRUE if error */
{
    zero_board(B);

    skip_white(&str);
    if (*str=='W') { B->next_to_move = Color::white; }
    else if (*str=='B') { B->next_to_move = Color::black; }
    else { return true; /* ERROR */ }
    to_next_line(&str); /* go to "   a b c" etc */
    for (int r=0; r!=8; ++r) {
        to_next_line(&str); /* go to "8 |r|n|b" etc */
        skip_white(&str);
        str += 1; /* skip "8" etc */ 
        for (int c=0; c!=8; ++c) {
            str += 2; /*skip " |" etc */ 
            if (*str==' ') {
                B->grid[r][c] = empty_piece;
                continue;
            }  
            Color color = (*str<'a') ? Color::white : Color::black;
            Species species;
            switch ((char)(*str + ((*str<'a') ? 0 : 'A'-'a'))) {
              break; case 'P': species = Species::pawn;
              break; case 'K': species = Species::king;
            //break; case 'W': species = Species::wazir  ; /*      soil               */
            //break; case 'F': species = Species::ferz   ; /*      air                */
              break; case 'N': species = Species::knight ; /*      horse              */
              break; case 'R': species = Species::rook   ; /*      tower              */
              break; case 'B': species = Species::bishop ; /*      heaven             */
            //break; case 'U': species = Species::unicorn; /*      myth               */
            //break; case 'V': species = Species::veggie ; /* WF = soil air           */
            //break; case 'T': species = Species::tapir  ; /* WN = soil horse         */
            //break; case 'O': species = Species::pope   ; /* WB = heavenly soil(rock)*/
            //break; case 'L': species = Species::golem  ; /* WZ = soil myth          */
            //break; case 'G': species = Species::giraffe; /* FN = air horse          */
            //break; case 'S': species = Species::spire  ; /* FR = airy tower         */
            //break; case 'X': species = Species::phoenix; /* FZ = air myth           */
            //break; case 'C': species = Species::chariot; /* NR = horse building     */
            //break; case 'D': species = Species::death  ; /* NB = heaven horse       */
              break; case 'Q': species = Species::queen  ; /* RB = divine mandate     */
            //break; case 'A': species = Species::akupara; /* RZ = tower myth         */
            //break; case 'Z': species = Species::zeus   ; /* BZ = heaven myth        */
            }
            add_piece_to_board(B, {r,c}, {color,species});
        }
    } 
    /* TODO: compute hash, plies_since_irreversible!! (and other associated precomputed vals!)  */
    return false;
}

/*=============================================================================
====  2. DISPLAY  =============================================================
=============================================================================*/

int const height = 4;
int const width = 6;
char const icons[height][7][width] = {
    {"     ", " _^  ", "  o  ", "     ", "!_|_/", "!_+_/","     "},
    {"  _  ", "[* ! ", " (_) ", "|_|_|", " (_) ", "_) (_","     "},
    {" ( ) ", " / | ", "`| |`", " | | ", " / ! ", " / ! ","     "},
    {" / ! ", "|  !_", "_/ !_", "|___|", "/___!", "/___!","     "},
};

void print_board_fancy(Board const* B)
{
    bool is_white = (B->next_to_move == Color::white);
    DisaggregatedScore ds = B->evaluation_stack.back(); 
    int mat = ds.material;
    int kng = ds.king_safety + 13 * ( B->nb_king_attacks_near[0]-B->nb_king_attacks_near[1]);
    int pwn = ds.pawn_structure + 5 * ( B->nb_weak_squares[0]-B->nb_weak_squares[1]);
    int sqr = ds.cozy_squares;
    int ini = MOBILITY * (-B->nb_xrays_by_side[0] + B->nb_xrays_by_side[1]) + TURN_BONUS * (B->next_to_move==Color::white ? +1 : -1);

    for (int rr=0; rr!=8*height; ++rr) {
        for (int k=0; k!=110; ++k) { std::cout << " "; }
        std::cout << "\33[110D";
        for (int c=0; c!=8; ++c) {
            Piece p = B->grid[rr/height][c];
            bool dark_square = ((rr/height) + c)%2;
            bool is_white    = (p.color==Color::white);
            REPEAT_SAY(2+width+2, (dark_square ? "\033[40;1m \033[0m" : " "));
            GO_LEFT   (2+width+2);

            if (rr%height==0) {
                if (dark_square) { std::cout << COLORIZE(GRAY, "\033[40;1m"<<"abcdefgh"[c]<<"87654321"[rr/height]<<"\033[0m"); }
                else             { std::cout << COLORIZE(GRAY, "abcdefgh"[c]<<"87654321"[rr/height]                         ); }
            } else {
                GO_RIGHT(2);
            }

            //if (rr%height==0) {
            //    std::cout << COLORIZE(YELLOW, (rr==0 ? "abcdefgh"[c] : '`'));
            //    std::cout << COLORIZE(YELLOW, (c==0  ? "87654321"[rr/height] : ' '));
            for (int x=0; x!=width; ++x) {
                char ch = icons[rr%height][p.species][x];
                if (ch=='!') { ch = '\\'; }
                switch (ch) {
                break; case ' ': GO_RIGHT(1);
                break; default : 
                    if (dark_square) { std::cout << COLORIZE((is_white?MAGENTA:CYAN), "\033[40;1m"<<ch<<"\033[0m"); }
                    else             { std::cout << COLORIZE((is_white?MAGENTA:CYAN), ch                         ); }
                }
            }
            //std::cout << COLORIZE(YELLOW, (((c==7||(rr/height)==7) && rr%height==height-1) ? " ." : "  "));
            GO_RIGHT(1); //WHITE_OUT(1);
            GO_RIGHT(1); //WHITE_OUT(1);
        }
        auto hued_digit = [](char const* ansi_hue, int nb) {
            if (!nb) { std::cout << COLORIZE(ansi_hue, "."); }
            else     { std::cout << COLORIZE(ansi_hue, nb ); }
        };
        auto hued_bool  = [](char const* ansi_hue, bool nb) {
            if (!nb) { std::cout << COLORIZE(ansi_hue, "."); }
            else     { std::cout << COLORIZE(ansi_hue, "1"); }
        };

        auto comp_sq_counts = [hued_digit](int const arr[2][8][8], int row) {
            for(int k=0; k!=8; ++k) {
                hued_digit(CYAN,    arr[0][row][k]);
                hued_digit(MAGENTA, arr[1][row][k]);
                std::cout << " ";
            }
        };
        auto comp_sq_flags  = [hued_bool ](bool const arr[2][8][8], int row) {
            for(int k=0; k!=8; ++k) {
                hued_bool(CYAN,    arr[0][row][k]);
                hued_bool(MAGENTA, arr[1][row][k]);
                std::cout << " ";
            }
        };

        WHITE_OUT(6);
        switch (rr) {
        break; case  0: std::cout << "turn = " << COLORIZE((is_white?MAGENTA:CYAN), (is_white?"White":"Black"));
        break; case  1: std::cout << "plies = " << COLORIZE(BLUE, B->plies_since_irreversible.back());
        break; case  2: std::cout << "hash = " << COLORIZE(BLUE, B->hash);
        break; case  3: std::cout << "mat = " << COLORIZE(RED , FLUSH_RIGHT_POS(4, mat));
        break; case  4: std::cout << "kng = " << COLORIZE(RED , FLUSH_RIGHT_POS(4, kng)); 
        break; case  5: std::cout << "pwn = " << COLORIZE(RED , FLUSH_RIGHT_POS(4, pwn)); 
        break; case  6: std::cout << "sqr = " << COLORIZE(RED , FLUSH_RIGHT_POS(4, sqr)); 
        break; case  7: std::cout << "ini = " << COLORIZE(RED , FLUSH_RIGHT_POS(4, ini)); 
;
        break; case  8: comp_sq_counts(B->nb_xrays, 0);
        break; case  9: comp_sq_counts(B->nb_xrays, 1);
        break; case 10: comp_sq_counts(B->nb_xrays, 2);
        break; case 11: comp_sq_counts(B->nb_xrays, 3);
        break; case 12: comp_sq_counts(B->nb_xrays, 4);
        break; case 13: comp_sq_counts(B->nb_xrays, 5);
        break; case 14: comp_sq_counts(B->nb_xrays, 6);
        break; case 15: comp_sq_counts(B->nb_xrays, 7);
        break; case 16:                                                                                                                                                    
        break; case 17: comp_sq_counts(B->attacks_by_pawn, 0);
        break; case 18: comp_sq_counts(B->attacks_by_pawn, 1);
        break; case 19: comp_sq_counts(B->attacks_by_pawn, 2);
        break; case 20: comp_sq_counts(B->attacks_by_pawn, 3);
        break; case 21: comp_sq_counts(B->attacks_by_pawn, 4);
        break; case 22: comp_sq_counts(B->attacks_by_pawn, 5);
        break; case 23: comp_sq_counts(B->attacks_by_pawn, 6);
        break; case 24: comp_sq_counts(B->attacks_by_pawn, 7);
        }
        std::cout << std::endl;
    }
} 


void print_board(Board const* B)
{
    auto new_tabbed_line = []() {
        CLEAR_LINE(30); std::cout << std::endl << "\t";
    };

    new_tabbed_line();
    std::cout << "   a b c d e f g h ";
    new_tabbed_line();
    for (int r=0; r!=8; ++r) {
        std::cout << 8-r << " |";
        for (int c=0; c!=8; ++c) {
            bool is_black = B->grid[r][c].color==Color::black ;
            char l = species_names[B->grid[r][c].species];
            if (is_black) { l += 'a'-'A'; }
            std::cout << COLORIZE(is_black ? CYAN : MAGENTA, l);
            std::cout << "|";
        }
        new_tabbed_line();
    }
    std::cout << "   ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾ ";
    
    bool is_white = B->next_to_move==Color::white;
    DisaggregatedScore ds = B->evaluation_stack.back(); 
    int mat = ds.material;
    int kng = ds.king_safety + 13 * ( B->nb_king_attacks_near[0]-B->nb_king_attacks_near[1]);
    int pwn = ds.pawn_structure + 5 * ( B->nb_weak_squares[0]-B->nb_weak_squares[1]);
    int sqr = ds.cozy_squares;
    int ini = MOBILITY * (-B->nb_xrays_by_side[0] + B->nb_xrays_by_side[1]) + TURN_BONUS * (B->next_to_move==Color::white ? +1 : -1);

    new_tabbed_line(); std::cout << "turn = " << COLORIZE((is_white?MAGENTA:CYAN), (is_white?"White":"Black"));
    new_tabbed_line(); std::cout << "plies = " << COLORIZE(BLUE, B->plies_since_irreversible.back());
    new_tabbed_line(); std::cout << "hash = " << COLORIZE(BLUE, B->hash);
    new_tabbed_line(); std::cout << "mat = " << COLORIZE(RED, FLUSH_RIGHT_POS(4, mat)); 
    new_tabbed_line(); std::cout << "kng = " << COLORIZE(RED, FLUSH_RIGHT_POS(4, kng)); 
    new_tabbed_line(); std::cout << "pwn = " << COLORIZE(RED, FLUSH_RIGHT_POS(4, pwn)); 
    new_tabbed_line(); std::cout << "sqr = " << COLORIZE(RED, FLUSH_RIGHT_POS(4, sqr)); 
    new_tabbed_line(); std::cout << "ini = " << COLORIZE(RED, FLUSH_RIGHT_POS(4, ini)); 
    new_tabbed_line();
}
