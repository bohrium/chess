# chess


     vvv


TODO : 
       resolve illegal and board-corrupting (upon undo?) Ra8(P) first move.

       address possibility of no legal move

       implement "get best k" moves for small k using alpha beta cutoffs.
            (implemented, untested)

       add alpha/beta cutoffs to stable_eval, too!

       implement stable node eval (linear greedy search for pass/bestcapture) 
         (implemented.)

       analyze mystery of hash table underusage
           partial progress: in apply_move/undo_move, computation of `mover`
                             should know that the `mover` piece is in different
                             squares (dest or source)!  thus originally hash
                             computation was incorrect and led to path
                             dependent value
           partial progress: orig_alpha

       analyze mystery of core dumps as recorded in commits for game-00, -01

       implement pawn promotion to queen (enrich Move type)

       add and tune terms for
           `piece-square tables`                (implemented, un-tuned)
           `attackers in same quadrant as king` (implemented, un-tuned)
           `pawn connectivity`                  (implemented, un-tuned)
           `bishop pair / bishop-pawn badness`  (implemented, un-tuned)
           `knights on protected weak squares` 
           `attackers xray king neighborhood`   (implemented slowly, un-tuned)
           `"random" mobility-incentivizing term via hash`
           `rooks on semi-open or open files`

       accelerate (xray'd king) term by integrating differences

       make sure hash includes who-is-to-move data!

     ^^^


## overview 
This project has two aims: to practice tree search and to use learning
techniques to infer and visualize interpretable chess heuristics.

### tree search
We wish to approximate **perfect play**.  Thus, we make no effort to model the
opponent (by finding Tal moves: incorrect moves whose refutations might be
difficult for the opponent to find) and we make no effort to model ourselves.
In other words, we seek a next move that most advantages our side if perfect
players are to substitute for our opponent and ourselves immediately after we
make our move.  In other words, we seek to approximate minimax values on an
enormous gametree whose leaves are labeled with {-1, 0, +1}. 

As a first approximation, we compute minimax values of a constant-depth subtree
labeled by a hand-crafted evaluation function whose target toset is richer than
{-1, 0, +1} and meant to correlate with probability of a forced win.
Alpha-beta is a standard algorithm that in the best case halves the effective
depth.  Conveniently, we may introduce initial values for alpha and beta
to induce cutoff at checkmate. 

and 4 optimizations:

*   **recursive move ordering** (call shallow alpha-beta to determine move
    ordering of deep alpha-beta)
*   **difference evaluation** (moves are sparse board updates, so it's faster
    to compute d(heuristic) and integrate) made interesting via **reversible
    move** (our version of Smith notation --- allows us to keep one board
    instance in Main)

*   TODO: BFS beam search outer layer for deep searching 
*   TODO: memoize {(depth, board) --> eval} during search
*   TODO: quiescnece search to handle captures (but not (yet) checks)
*   TODO: zero-window scouting

#### BFS beam search

    Vanilla alpha-beta is by nature a DFS: its sibling-to-sibling interface
    is very narrow, being only an (alpha,beta) pair.  Sometimes, though, we 
    may wish to go deeper than vanilla alpha-beta allows by keeping only the
    top k nodes (according to some advanced heuristic such as depth-8 vanilla
    alpha-beta) at each successive depth.  The notion of ``top k'' requires a
    wide sibling-to-sibling interface: we must at least keep track of the 
    best k kids so far.

### evaluation heuristic

Our heuristic decomposes as:

    score = king-safety + pawns + pawn-piece + material + (midgame)(placement)

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

The `pawn-piece` adjustment penalizes bishops of the same square parity as many
of their own side's pawns (-10 per pawn-bishop pair of same color and same
square parity; +10 per pawn-bishop pair of same color and different square
parity), rewards knights on outposts (+50 per knight protected by friendly
pawn on a square weak for the opponent), and rewards rooks on open files
(+50 per rook on a file with neither friendly nor enemy pawns). 

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
