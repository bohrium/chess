# chess

## overview 
This project has two aims: to practice tree search and to use learning
techniques to infer and visualize interpretable chess heuristics.

### tree search
We use constant-depth tree search with a hand-crafted leaf evaluation function
and 4 optimizations:

*   alpha-beta pruning (with initial values to induce cutoff at checkmate)
*   recursive move ordering (call shallow alpha-beta to determine move ordering
    of deep alpha-beta)
*   reversible move (our version of Smith notation --- allows us to keep one
    board instance in Main)
*   difference evaluation (moves are sparse board updates, so it's faster to
    compute d(heuristic) and integrate) 

*   TODO: memoize {(depth, board) --> eval} during search
*   TODO: quiescnece search to handle checks and captures

### evaluation heuristic

Our heuristic decomposes as:

    score = king-safety + pawns + material + (midgame)(placement)

Each term is quadratic in our one-hot board representation.  The expressions
`material`, `midgame`, and `placement` are in fact linear and thus especially
straightforward to implement difference evaluation for.  The `king-safety` is
a rank-64 bilinear arising from a sparsely-updated factor and a sparse factor. 
The `pawns` term likewise is a rank-15 bilinear arising from sparsely-updated
factors.    

The `king-safety` term adds -25 for each potentially-blocked attack by an
enemy piece to the king's square or a square a king-move away.  By
*potentially-blocked attack*, we mean that we ignore occlusions.  For example,
we add (-25)(2+2+1+2) = -175 to white's score in the following board (left),
based on attacks from the queen, bishop, knight, and rook.

       king-safety example         pawn island example  
    [ ][ ][ ][ ][ ][ ][k][ ]    [ ][ ][ ][ ][ ][ ][k][ ]
    [p][p][p][ ][ ][p][p][p]    [ ][ ][p][ ][ ][ ][p][ ]
    [ ][ ][ ][ ][ ][ ][ ][ ]    [ ][ ][ ][p][ ][p][ ][ ]
    [ ][ ][ ][ ][p][N][ ][ ]    [ ][ ][ ][ ][p][ ][ ][ ]
    [ ][ ][Q][ ][P][ ][ ][ ]    [ ][P][ ][P][P][ ][ ][ ]
    [ ][P][ ][ ][ ][P][ ][ ]    [ ][P][ ][ ][ ][ ][P][ ]
    [P][B][P][ ][ ][ ][P][P]    [ ][ ][ ][ ][ ][ ][P][P]
    [ ][K][ ][ ][ ][ ][ ][R]    [ ][K][ ][ ][ ][ ][ ][ ]

The `pawns` term incentivizes pawn connectivity and penalizes doubled pawns.
That is: if a pair of adjacent columns contain a,b many white pawns, then we
add (+25 ab) in connectivity score.  And if a column contains b many white
pawns, we add (-20 a^2) in doubled pawn score.  For example, in the above
board (right), we add (+25)(+1)+(-20)(+4+1+1+4+1) = -195 to white's score and
we add (+25)(+1+1+1+1)+(-20)(-1-1-1-1-1) = 0 to black's score.  Even though
white has two more pawns than black, the `pawns` term suggests that the 
players' pawn structures are roughly equal.

We count `material` using Fischer's estimates: p,n,b,r,q = 100,300,325,500,900.
The `placement` term adjusts these values based on piece location (hence
without loss absorbs the `material` term) using values simplified from those of
Tomasz Michniewski --- see [this wonderful
article](https://www.chessprogramming.org/Simplified_Evaluation_Function).
Specifically, it grants -40,-15,+15,+40 for horrible,bad,good,great (=,-,+,#)
squares:  

             pawns                       knights
    [ ][ ][ ][ ][ ][ ][ ][ ]    [=][=][=][=][=][=][=][=]
    [#][#][#][#][#][#][#][#]    [=][-][ ][ ][ ][ ][-][=]
    [ ][+][+][+][+][+][+][ ]    [=][ ][ ][+][+][ ][ ][=]
    [ ][ ][ ][+][+][ ][ ][ ]    [=][ ][+][+][+][+][ ][=]
    [ ][ ][ ][ ][ ][ ][ ][ ]    [=][ ][+][+][+][+][ ][=]
    [ ][ ][-][ ][ ][-][ ][ ]    [=][ ][ ][+][+][ ][ ][=]
    [ ][ ][ ][-][-][ ][ ][ ]    [=][-][ ][ ][ ][ ][-][=]
    [ ][ ][ ][ ][ ][ ][ ][ ]    [=][=][=][=][=][=][=][=]

             bishops                     rooks           
    [=][=][=][=][=][=][=][=]    [ ][ ][ ][ ][ ][ ][ ][ ]
    [=][-][-][-][-][-][-][=]    [ ][+][+][+][+][+][+][ ]
    [=][-][-][ ][ ][-][-][=]    [ ][ ][ ][ ][ ][ ][ ][ ]
    [=][-][-][ ][ ][-][-][=]    [ ][ ][ ][ ][ ][ ][ ][ ]
    [=][-][ ][ ][ ][ ][-][=]    [ ][ ][ ][ ][ ][ ][ ][ ]
    [=][ ][ ][ ][ ][ ][ ][=]    [ ][ ][ ][ ][ ][ ][ ][ ]
    [=][-][-][-][-][-][-][=]    [ ][ ][ ][ ][ ][ ][ ][ ]
    [=][=][=][=][=][=][=][=]    [ ][ ][ ][ ][ ][ ][ ][ ]

             queens                      kings          
    [-][-][-][ ][ ][-][-][-]    [=][=][=][=][=][=][=][=]
    [-][ ][ ][ ][ ][ ][ ][-]    [=][=][=][=][=][=][=][=]
    [-][ ][ ][ ][ ][ ][ ][-]    [=][=][=][=][=][=][=][=]
    [ ][ ][ ][ ][ ][ ][ ][ ]    [=][=][=][=][=][=][=][=]
    [ ][ ][ ][ ][ ][ ][ ][ ]    [-][=][=][=][=][=][=][-]
    [-][ ][ ][ ][ ][ ][ ][-]    [-][-][-][-][-][-][-][-]
    [-][ ][ ][ ][ ][ ][ ][-]    [+][+][ ][ ][ ][ ][+][+]
    [-][-][-][ ][ ][-][-][-]    [+][#][ ][ ][ ][ ][#][+]

The `midgame` coefficient starts at 1.0 and decreases to 0.0 as the game
progresses.  It equals the piece density: (number of pieces of either
color)/32.  Thus, the `placement` term diminishes in influence as we approach
the end game.  Note that bishops' `placement` coefficients are entirely
non-positive.  Thus, bishops grow in value as the board opens up.

### linear svm

    
## issues
trouble profiling... try gprof on caen machines?

want to tune heuristic... get Matthew S to look at TestEvalRuy and TestEvalSicilian (and make a middlegame and endgame
testeval too)? 
