#ifndef EVAL_PARAMS_H
#define EVAL_PARAMS_H

/*=============================================================================
====  0. SEARCH FUNCTION  =====================================================
=============================================================================*/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~  0.0. Initiative  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

int const TURN_BONUS     = 13;
int const MOBILITY       = 13;
/* UNIMPLEMENTED! */ int const LOOSE_PIECE    = 13; /* malus */
/* UNIMPLEMENTED! */ int const ATTACKED_PIECE = 13; /* malus */

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~  0.1. Material  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*------  0.1.0. linear material     ----------------------------------------*/

int const PAWN_MALUS   = 34; /* malus */
int const KNIGHT_BONUS = 13;
int const BISHOP_BONUS = 34;
int const ROOK_BONUS   = 89;
int const QUEEN_BONUS  = 89;

/*------  0.1.1. bilinear material   ----------------------------------------*/

int const BISHOP_PAIR  = 34;
int const MAJOR_PAIR   = 34; /* malus */
int const ROOK_PAWN    = 13; /* malus */
int const KNIGHT_PAWN  =  5;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~  0.2. King Safety  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

int const KING_XRAY    = 13; /* malus */
int const KING_SHELTER = 13;
int const KING_TROPISM = 34; /* malus */

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~  0.3. Pawn Structure  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

int const DISCONNECTED = 34; /* malus */
int const WEAK_SQUARE  =  5; /* malus */
/* UNIMPLEMENTED! */ int const OUTPOST      = 13; /* malus */

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~  0.4. Cozy Squares  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*------  0.4.0. piece-pawn interactions  -----------------------------------*/

/* UNIMPLEMENTED! */ int const KNIGHT_ON_POST = 89;
/* UNIMPLEMENTED! */ int const KNIGHT_ON_WEAK = 34;

int const BLOCKED_BISHOP = 13; /* malus */

int const ROOK_ON_OPEN   = 34;
int const ROOK_ON_SEMI   = 13;

/* UNIMPLEMENTED! */ int const Q_INFILTRATION = 13;

/*------  0.4.1. piece-square table  ----------------------------------------*/

int const PROMOTE_1      = 89;
int const PROMOTE_2      = 34;
int const PROMOTE_3      = 13;
int const PROMOTE_4      =  5;
int const FLANK_P        = 13; /* malus */

int const CORNERED_N     = 89; /* malus */
int const RIMMED_N       = 13; /* malus */
int const ADVANCED_N     = 34;

int const RIMMED_B       = 13; /* malus */
int const CORNERED_B     = 34; /* malus */
int const B_SNIPER       = 34;

int const R_PIG          = 13;

int const CENTERED_Q     =  5;

int const CENTERED_K0    = 34; 
int const CENTERED_K1    = 13; 
int const CENTERED_K2    =  5; 

/*=============================================================================
====  1. MATERIAL IMPLEMENTATION  =============================================
=============================================================================*/

const int KING_POINTS = 100000; /* exceeds twice the total remaining value */ 
const int points[] = {
    100-PAWN_MALUS,   /*  p  */
    300+KNIGHT_BONUS, /*  n  */
    300+BISHOP_BONUS, /*  b  */
    500+ROOK_BONUS,   /*  r  */
    900+QUEEN_BONUS,  /*  q  */
    KING_POINTS       /*  k  */
};

/*=============================================================================
====  2. PIECE-SQUARE TABLE IMPLEMENTATION  ===================================
=============================================================================*/

int const piece_placement[][8][8] = {
    /*pawn*/ { /* push pawns, control center, rook pawns worth less */
    #define _1 (+PROMOTE_1)
    #define _2 (+PROMOTE_2)
    #define _3 (+PROMOTE_3)
    #define _4 (+PROMOTE_4)
    #define F  (-FLANK_P)
        { 0, 0, 0, 0, 0, 0, 0, 0},
        {_1,_1,_1,_1,_1,_1,_1,_1},
        { F,_2,_2,_2,_2,_2,_2, F},
        { F, 0,_3,_3,_3,_3, 0, F},
        { F, 0, 0,_4,_4, 0, 0, F},
        { F, 0, 0, 0, 0, 0, 0, F},
        { F, 0, 0, 0, 0, 0, 0, F},
        { 0, 0, 0, 0, 0, 0, 0, 0}, 
    #undef _1
    #undef _2
    #undef _3
    #undef _4
    #undef F
    },
    /*knight*/ { /* a knight on the rim is dim*/ 
    #define R (-RIMMED_N)
    #define C (-CORNERED_N)
    #define A (+ADVANCED_N)
        { C, R, R, R, R, R, R, C},
        { R, 0, 0, 0, 0, 0, 0, R},
        { R, 0, A, A, A, A, 0, R},
        { R, 0, 0, A, A, 0, 0, R},
        { R, 0, 0, 0, 0, 0, 0, R},
        { R, 0, 0, 0, 0, 0, 0, R},
        { R, 0, 0, 0, 0, 0, R, R},
        { C, R, R, R, R, R, R, C},
    #undef R
    #undef C
    #undef A
    },
    /*bishop*/ { /* bishops love long diagonals */
    #define R (-RIMMED_B)
    #define C (-CORNERED_B)
    #define D (+B_SNIPER)
        { C, C, R, R, R, R, C, C},
        { C, D, 0, 0, 0, 0, D, C},
        { R, 0, D, 0, 0, D, 0, R},
        { R, 0, 0, D, D, 0, 0, R},
        { R, 0, 0, D, D, 0, 0, R},
        { R, 0, D, 0, 0, D, 0, R},
        { C, D, 0, 0, 0, 0, D, C},
        { C, C, R, R, R, R, C, C},
    #undef R
    #undef C
    #undef D
    },
    /*rook*/ { /* rooks love 7th ranks (term small since so mobile) */
    #define P (+R_PIG)
        { 0, 0, 0, 0, 0, 0, 0, 0},
        { P, P, P, P, P, P, P, P},
        { 0, 0, 0, 0, 0, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0, 0, 0}, 
    #undef P
    },
    /*queen*/ { /* centralize the queen (term small since so mobile) */
    #define C (+CENTERED_Q)
        { 0, 0, 0, 0, 0, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0, 0, 0},
        { 0, 0, 0, C, C, 0, 0, 0},
        { 0, 0, C, C, C, C, 0, 0},
        { 0, 0, C, C, C, C, 0, 0},
        { 0, 0, 0, C, C, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0, 0, 0},
    #undef C
    },
    /*king*/ { /* centralize the king (hopefully overridden in middle game by safety factors) */
    #define _0 (+CENTERED_K0)
    #define _1 (+CENTERED_K1)
    #define _2 (+CENTERED_K2)
        { 0, 0, 0, 0, 0, 0, 0, 0},
        { 0, 0, 0,_2,_2, 0, 0, 0},
        { 0, 0,_2,_1,_1,_2, 0, 0},
        { 0,_2,_1,_0,_0,_1,_2, 0},
        { 0,_2,_1,_0,_0,_1,_2, 0},
        { 0, 0,_2,_1,_1,_2, 0, 0},
        { 0, 0, 0,_2,_2, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0, 0, 0},
    #undef _0
    #undef _1
    #undef _2
    },
};

#endif//EVAL_PARAMS_H

