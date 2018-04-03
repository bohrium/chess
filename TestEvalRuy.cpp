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
    return score_move(B, m, 4) > score_move(B, m_, 4);
}

int main(int argc, char** argv)
{
    std::cout << "Welcome!" << std::endl; 

    Board B;
    init_board(&B);
    MoveList ML;

    /* Ruy Lopez, Exchange Variation */
    Piece white_bishop = {Color::white, Species::bishop};
    Piece black_knight = {Color::black, Species::knight};
    Move ruy[7] = {
        {{6,4}, {4,4}, empty_piece}, // e4
        {{1,4}, {3,4}, empty_piece}, // e5
        {{7,6}, {5,5}, empty_piece}, // Nf3
        {{0,1}, {2,2}, empty_piece}, // Nc6
        {{7,5}, {3,1}, empty_piece}, // Bb5
        {{1,0}, {2,0}, empty_piece}, // a6
        {{3,1}, {2,2}, black_knight} // Bxc6
    };
    /* After the above 7 moves, the game state is as follows:

        Black to move
           a b c d e f g h
        8 |r| |b|q|k|b|n|r|
        7 | |p|p|p| |p|p|p|
        6 |p| |B| | | | | |
        5 | | | | |p| | | |
        4 | | | | |P| | | |
        3 | | | | | |N| | |
        2 |P|P|P|P| |P|P|P|
        1 |R|N|B|Q|K| | |R|
           ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    */
    for (int i=0; i!=7; ++i) {
        apply_move(&B, ruy[i]);
    }
    print_board(&B);

    Move potential_responses[7] = {
        {{1,1}, {2,2}, white_bishop},// bxc6 (ok for black)         --- should have low (white) score
        {{1,3}, {2,2}, white_bishop},// dxc6 (ok for black)         --- should have low (white) score
        {{0,6}, {2,5}, empty_piece}, // nf6  (bad for black)        --- should have medium (white) score
        {{0,5}, {3,2}, empty_piece}, // bc5  (bad for black)        --- should have medium (white) score
        {{0,3}, {4,7}, empty_piece}, // Qh4  (terrible for black)   --- should have high (white) score
        {{0,3}, {3,6}, empty_piece}, // Qg5  (terrible for black)   --- should have high (white) score
        {{1,3}, {2,3}, empty_piece}  // d6   (disastrous for black) --- should have very high (white) score
    };

    for (int i=0; i!=6; ++i) {
        Move m =potential_responses[i],
             m_=potential_responses[i+1];
        print_move(&B, m);
        if (is_better(&B, m, m_)) {
            std::cout << " is better than ";
        } else {
            std::cout << " is worse than ";
        }
        print_move(&B, m_);
        std::cout << " for white " << std::endl;
    }  

    return 0;
}

