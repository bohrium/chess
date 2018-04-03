#include "Board.h"
#include "Search.h"
#include <iostream>

float score_move(Board* B, Move m, int nb_plies)
{
    apply_move(B, m);
    float score = alpha_beta(B, nb_plies, -10000.0, +10000.0);
    undo_move(B, m);
    return score;
}

bool is_better(Board* B, Move m, Move m_)
{
    float s = score_move(B,m, 4),
          s_= score_move(B,m_,4);
    //std::cout << s << " " << s_ << std::endl;
    return s > s_;
}

int main(int argc, char** argv)
{
    std::cout << "Welcome!" << std::endl; 

    Board B;
    init_board(&B);
    MoveList ML;

    /* Sicilian, Sveshnikov Variation */
    Piece white_pawn   = {Color::white, Species::pawn};
    Piece black_pawn   = {Color::black, Species::pawn};
    Piece black_knight = {Color::black, Species::knight};
    Move sicilian[8] = {
        {{6,4}, {4,4}, empty_piece}, // e4
        {{1,2}, {3,2}, empty_piece}, // c5
        {{7,6}, {5,5}, empty_piece}, // Nf3
        {{0,1}, {2,2}, empty_piece}, // Nc6
        {{6,3}, {4,3}, empty_piece}, // d4
        {{3,2}, {4,3}, white_pawn},  // cxd4
        {{5,5}, {4,3}, black_pawn},  // Nxd4
        {{0,6}, {2,5}, empty_piece}  // Nf6
    };
    /* After the above 8 moves, the game state is as follows:

        White to move
           a b c d e f g h
        8 |r| |b|q|k|b| |r|
        7 |p|p| |p|p|p|p|p|
        6 | | |n| | |n| | |
        5 | | | | | | | | |
        4 | | | |N|P| | | |
        3 | | | | | | | | |
        2 |P|P|P| | |P|P|P|
        1 |R|N|B|Q|K|B| |R|
           ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    */
    for (int i=0; i!=8; ++i) {
        apply_move(&B, sicilian[i]);
    }
    print_board(&B);

    Move potential_responses[7] = {
        {{7,1}, {5,2}, empty_piece}, // Nc3  (classic response)     --- should have high (white) score
        {{4,3}, {2,2}, black_knight},// Nxc6 (knight trade)         --- should have medium (white) score
        {{4,4}, {3,4}, empty_piece}, // e5   (hanging pawn)         --- should have low (white) score
        {{7,3}, {5,5}, empty_piece}, // Qf3  (lose knight)          --- should have low (white) score
        {{7,2}, {2,7}, empty_piece}, // Bh6  (lose bishop)          --- should have low (white) score
        {{7,1}, {6,3}, empty_piece}, // Nd2  (lose knight)          --- should have low (white) score
        {{7,3}, {4,6}, empty_piece}, // Qg4  (lose queen)           --- should have very low (white) score
    };

    for (int i=0; i!=6; ++i) {
        Move m =potential_responses[i],
             m_=potential_responses[i+1];
        bool b = is_better(&B, m, m_);
        print_move(&B, m);
        if (b) {
            std::cout << " is better than ";
        } else {
            std::cout << " is worse than ";
        }
        print_move(&B, m_);
        std::cout << " for white " << std::endl;
    }  

    return 0;
}

