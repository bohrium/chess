#include "Search.h"
#include "Helpers.h"
#include "EvalParams.h"
#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <time.h>
#include <chrono>

ply_t const ENGINE_DEPTH     = 12; 
ply_t const WHITE_DEPTH      = 12; 
ply_t const BLACK_DEPTH      = 12;
ply_t const COMMENTARY_DEPTH =  8;

ply_t const PV_PLIES = 4;

bool const MULTITHREADED = 0;
int const THREAD_LAYERS = 2;

ply_t const MAX_NB_PLIES = 360;

Move get_move(Board* B)
{
    std::cout << "enter move (e.g. \"e2e4<enter>\"): ";

    Move m;
    MoveList ML;
    generate_moves(B, &ML, false);

    while (true) {
        char ans;
        char sr,sc,dr,dc;
        std::cin >> sc>>sr>>dc>>dr; GO_UP(1); CLEAR_LINE(60); 
        m.source = {8-(sr-'0'), sc-'a'}; 
        m.dest = {8-(dr-'0'), dc-'a'}; 
        if (!is_valid(m.source) || !is_valid(m.dest)) {
            std::cout << "i only understand notation like e2e4 : ";
            continue;
        }
        Piece mover = get_piece(B, m.source); 
        m.taken = get_piece(B, m.dest); 

        if (mover.color == Color::empty_color) {
            std::cout << "the square " << sr << sc << " is empty : ";
            continue;
        } else if (mover.color == flip_color(B->next_to_move)) {
            std::cout << "that's my piece (" << piece_name(mover) << ") : ";
            continue;
        }

        m.type = MoveType::ordinary;
        if (mover.color==Color::white && m.dest.row==0 || 
            mover.color==Color::black && m.dest.row==7) {
            m.type = MoveType::promote_to_queen;
        } 

        bool is_legal = false;
        for (int l=0; l!=ML.length; ++l) {
            if (!same_move(m, ML.moves[l])) { continue; }
            is_legal = true; break;
        }
        if (!is_legal) {
            std::cout << "that's not how " << piece_name(mover) << "s move : ";
            continue;
        }

        std::cout << "is "; print_move(B, m); std::cout << " your move? (y/n)";
        std::cin >> ans; GO_UP(1); CLEAR_LINE(60); 
        switch (ans) {
        break; case 'y': return m;
        }

        std::cout << "enter move (e.g. \"e2e4<enter>\"): ";
    }
}

/*=============================================================================
====  0. MAIN LOOP  ===========================================================
=============================================================================*/

int main(int argc, char** argv)
{
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ~~~~  0.0. Initialize Board  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    std::cout << COLORIZE(YELLOW, "Welcome!") << std::endl; 

    Board B;
    init_board(&B);
    static PVTable pv_table; /* static : don't waste stack space */
    zero_table(pv_table);
    srand(time(NULL));

    int const alpha=-KING_POINTS/2;
    int const beta=+KING_POINTS/2;

    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ~~~~  0.1. Main Loop  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    //int const indent_mate         = 0;

    int const indent_move         = 0;
    int const indent_gen_height   = 0+6;
    int const indent_gen_time     = 0+6+10; 
    int const indent_kibitz_score = 0+6+10+10;//+(2+PV_PLIES*6);
    int const indent_kibitz_line  = 0+6+10+10+10;

    for (int t=0; t!=MAX_NB_PLIES; ++t) {
        bool is_white = t%2==0;
        bool is_human = is_white;

        /*--------  0.1.0. display game state  ------------------------------*/
        print_board_fancy(&B);
        GO_UP(32);

        /*--------  0.1.1. read or compute next move  -----------------------*/

        GO_DOWN(40);
        auto start=std::chrono::system_clock::now();
        ScoredMove sm = is_human ? ScoredMove{get_move(&B), 0, 0}  
                                 : get_best_move(&B, ENGINE_DEPTH, alpha, beta, true, false, MAX_VERBOSE, pv_table); 
        auto end=std::chrono::system_clock::now();
        GO_UP(40);
        CLEAR_LINE(90);

        /*--------  0.1.1. thinking metrics: height and time  ---------------*/

        GO_RIGHT(indent_gen_height);
        if (!is_human) { std::cout << " height " << COLORIZE(GREEN, sm.height); }
        GO_LEFT(60);

        GO_RIGHT(indent_gen_time);
        float secs = ((std::chrono::duration<double>)(end-start)).count();
        std::cout << " time " << COLORIZE(GREEN, (int)secs);
        GO_LEFT(60);

        /*--------  0.1.2. announce move  -----------------------------------*/
        GO_RIGHT(indent_move);
        GO_RIGHT(indent_move);
        if (sm.m.taken.species == Species::king) {
            std::cout << COLORIZE(RED,  "CHECKMATE!") << std::endl;
            break; 
        }
        print_move(&B, sm.m);
        GO_LEFT(60);

        /*--------  0.1.3. make move (update game state)  -------------------*/
        apply_move(&B, sm.m);

        /*--------  0.1.4. kibitz score and predicted line  -----------------*/
        ScoredMove commentary = get_best_move(&B, COMMENTARY_DEPTH, alpha, beta, true, true, 0, pv_table);

        GO_RIGHT(indent_kibitz_score);
        std::cout << "score ";
        std::cout << COLORIZE(RED, SHOW_SIGN(commentary.score)); 
        GO_LEFT(60);

        GO_RIGHT(indent_kibitz_line) 
        std::cout << "after ";
        print_pv(&B, COMMENTARY_DEPTH, PV_PLIES, pv_table);
        std::cout << "...";
        GO_LEFT(60);

        std::cout << std::endl;
    }

    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ~~~~  0.2. Print Checkmate Compactly  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    print_board(&B);
    return 0;
} 

void self_play(Board* B, PVTable pv_table)
{ 
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ~~~~  0.1. Main Loop  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    for (int t=0; t!=MAX_NB_PLIES; ++t) {
        /*--------  0.1.0. display game state  ------------------------------*/
        print_board_fancy(B);
        GO_UP(33);
        //char cc; std::cin >>cc;

        /*--------  0.1.1. compute best move  -------------------------------*/
        std::cout << std::endl;
        int alpha=-KING_POINTS/2, beta=+KING_POINTS/2;
        int nb_plies = t%2==0 ? WHITE_DEPTH : BLACK_DEPTH; 
        ScoredMove sm;
        {
            GO_DOWN(40);
            if (MULTITHREADED) {
                sm = get_best_move_multithreaded(B, nb_plies, alpha, beta, THREAD_LAYERS, pv_table);
            } else {
                sm = get_best_move(B, nb_plies, alpha, beta, true, false, MAX_VERBOSE, pv_table); 
            }
            GO_UP(40);
        }

        /*--------  0.1.2. announce move (and predicted line)  --------------*/
        CLEAR_LINE(120);
        std::cout << COLORIZE(t%2?CYAN:MAGENTA, (t%2?'B':'W')) << " ";
        std::cout << COLORIZE(GREEN, FLUSH_RIGHT(2, sm.height)) << ": ";
        if (sm.m.taken.species == Species::king) {
            std::cout << std::endl;
            std::cout << "\t" << COLORIZE(GRAY,  "CHECKMATE!") << std::endl;
            std::cout << "\t" << COLORIZE(RED,   "CHECKMATE!") << std::endl;
            std::cout << "\t" << COLORIZE(YELLOW,"CHECKMATE!") << std::endl;
            std::cout << "\t" << COLORIZE(BLUE,  "CHECKMATE!") << std::endl;
            std::cout << "\t" << COLORIZE(GRAY,  "CHECKMATE!") << std::endl;
            break; 
        }
        print_pv(B, nb_plies, PV_PLIES, pv_table);
        std::cout << " \"";
        std::cout << COLORIZE(RED, FLUSH_RIGHT_POS(4, sm.score)); 
        std::cout << "\"";

        /*--------  0.1.3. make move (update game state)  -------------------*/
        apply_move(B, sm.m);

        /*--------  0.1.4. evaluate and display move quality  ---------------*/
        std::cout << " (";
        ScoredMove commentary = get_best_move(B, COMMENTARY_DEPTH, -KING_POINTS/2, +KING_POINTS/2, true, true, 0, pv_table);
        std::cout << COLORIZE(RED, FLUSH_RIGHT_POS(4, commentary.score)); 
        std::cout << ")";
        CLEAR_LINE(120);
        std::cout << std::endl;
    }
}

