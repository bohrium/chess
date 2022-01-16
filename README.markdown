# chess

## overview 
This project has two aims: to practice tree search and to use learning
techniques to infer and visualize interpretable chess heuristics.

### tree search
We wish to approximate **perfect play**.  Thus, we make no effort to model the
opponent (by finding Tal moves: incorrect moves whose refutations might be
difficult for the opponent to find) and we make no effort to model ourselves.
In other words, we seek a next move that most advantages our side if perfect
players are to substitute for our opponent and ourselves immediately after we
make our move.  We seek, in short, to approximate minimax values on an enormous
gametree whose leaves are labeled with {-1, 0, +1}. 

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

#### Reductions

We reduce when a position seems too good to be relevant (as judged by null
move) or too bad to be (as judged by late move in two ways).  We extend when a
position when it seems substantially better than all of its siblings --- since
we like to phrase everything in terms of reductions, this means we reduce
unless we are at a best sibling seems much better than the others.  Thus, we
have NULL MOVE REDUCTIONS, LATE MOVE REDUCTIONS, and CO-SINGULAR REDUCTIONS. 
The last of those e.g. helps us evade checks.

#### BFS beam search

    Vanilla alpha-beta is by nature a DFS: its sibling-to-sibling interface
    is very narrow, being only an (alpha,beta) pair.  Sometimes, though, we 
    may wish to go deeper than vanilla alpha-beta allows by keeping only the
    top k nodes (according to some advanced heuristic such as depth-8 vanilla
    alpha-beta) at each successive depth.  The notion of ``top k'' requires a
    wide sibling-to-sibling interface: we must at least keep track of the 
    best k kids so far.

### evaluation heuristic

Our heuristic decomposes into five themes: king attacks, pawn structure,
piece-pawn interactions, piece-piece interactions, and material.  We set these
by intuition, so we use a very coarse set of weights: 5, 13, 34, 89 centipawns. 

    score =   king-xray + king-tropism
            + pawn-connectivity + weak-squares + passed-pawns 
            + bishop-pawn + knight-outpost + rook-open-files + king-shelter
            + loose-pieces + double-attacked-squares + bishop-pair 
            + material+ piece-square

Most terms are quadratic in our one-hot board representation.  All arise by 
combining factors sparsely updated in each move --- a boon to fast computation.

The `king-xray` term adds `+13` for each potentially-blocked attack by an
friendly piece to the enemy king's square or a square a king-move away.  By
*potentially-blocked attack*, we mean that we ignore occlusions.  In the
following board (left), for instance, we add (+13)(2+2+1+2+2) to white's score
(based on attacks from QBNNR) and (+13)(2+2+2) to black's score (based on
attacks from qrr).

The `king-tropism` term adds `+34` for each friendly piece in the same quadrant
as the enemy king. 

       king attacks                pawn structure       
    [q][ ][ ][ ][ ][ ][k][ ]    [ ][ ][ ][ ][ ][ ][k][ ]
    [r][p][p][ ][ ][p][p][p]    [ ][ ][p][ ][ ][ ][p][ ]
    [ ][ ][ ][ ][ ][ ][ ][ ]    [ ][ ][ ][p][ ][p][p][ ]
    [ ][ ][ ][ ][p][N][N][ ]    [ ][ ][ ][ ][p][ ][ ][ ]
    [ ][ ][Q][ ][P][ ][ ][ ]    [ ][P][ ][P][P][ ][ ][ ]
    [ ][P][ ][ ][ ][P][ ][ ]    [ ][P][ ][ ][ ][ ][P][ ]
    [r][B][P][ ][ ][ ][P][P]    [ ][ ][ ][ ][ ][ ][P][P]
    [ ][K][ ][ ][ ][ ][ ][R]    [ ][K][ ][ ][ ][ ][ ][ ]

To obtain `pawn-connectivity`, we add `+34` for each pair of adjacent columns
both containing white pawns.  For example, in the above board (right), we add
34 to white's score and we add 136 to black's score.  Even though white has two
more pawns than black, the `pawns` term suggests that the players' pawn
structures are roughly equal.

A square is *weak* when it is neither behind a friendly pawn nor horizontally
adjacent to a square in front of a friendly pawn.  The `weak-squares` penalty
adds `-13` for each weak square.

A pawn is *passed* when it is on a square weak for the enemy.  We add 
`+34` for each passed pawn.

The pawn-piece adjustments penalize bishops of the same square parity as many
of their own side's pawns (`-13` per pawn-bishop pair of same color and same
square parity), reward knights on outposts (`+89` per knight protected by
friendly pawn on a square weak for the opponent), reward rooks on open files
(`+34` per rook on a file with neither friendly nor enemy pawns), and rewards
each friendly pawn one king-move away in front of the king (`+34`).

The `loose-pieces` term adds `-13` for each friendly piece on a square attacked
by no friendly pieces.  The `double-attacked-squares` term adds `+13` for each
square attacked by at least two friendly pieces.  We add another `+34` for each
opposite-colored bishop pair.
**NOTE**: `loose-pieces` and `double-attacked-squares` are NOT YET IMPLEMENTED. 

We value `material` according to p,n,b,r,q = 100,313,334,500,900.  The
`placement` term adjusts these values based on piece location (hence without
loss absorbs the `material` term) using values simplified from those of Tomasz
Michniewski --- see [this wonderful
article](https://www.chessprogramming.org/Simplified_Evaluation_Function).
Roughly, it grants -89,-34,-13,-5,+5,+13,+34,+89 for horrible,bad,good,great
(==,=,--,-,+,++,#,##) squares:  

             pawns                       knights
    |  |  |  |  |  |  |  |  |   |==|--|--|--|--|--|--|==|
    |##|##|##|##|##|##|##|##|   |--|- |- |  |  |- |- |--|
    |  |# |# |# |# |# |# |  |   |--|- |  |+ |+ |  |- |--|
    |  |  |  |++|++|  |  |  |   |--|  |+ |# |# |+ |  |--|
    |  |  |  |+ |+ |  |  |  |   |--|  |+ |# |# |+ |  |--|
    |  |  |  |  |  |  |  |  |   |--|- |  |+ |+ |  |- |--|
    |  |  |  |  |  |  |  |  |   |--|- |- |  |  |- |- |--|
    |  |  |  |  |  |  |  |  |   |==|--|--|--|--|--|--|==|

             bishops                     rooks           
    |- |- |- |- |- |- |- |- |   |  |  |  |  |  |  |  |  |
    |- |++|  |  |  |  |++|- |   |+ |+ |+ |+ |+ |+ |+ |+ |
    |- |  |++|  |  |++|  |- |   |  |  |  |  |  |  |  |  |
    |- |  |  |++|++|  |  |- |   |  |  |  |  |  |  |  |  |
    |- |  |  |++|++|  |  |- |   |  |  |  |  |  |  |  |  |
    |- |  |++|  |  |++|  |- |   |  |  |  |  |  |  |  |  |
    |- |++|  |  |  |  |++|- |   |  |  |  |  |  |  |  |  |
    |- |- |- |- |- |- |- |- |   |  |  |  |  |  |  |  |  |

             queens                      kings          
    |  |  |  |  |  |  |  |  |   |  |  |  |  |  |  |  |  |
    |  |  |  |  |  |  |  |  |   |  |  |  |  |  |  |  |  |
    |  |  |  |+ |+ |  |  |  |   |  |  |  |  |  |  |  |  |
    |  |  |+ |+ |+ |+ |  |  |   |  |  |  |  |  |  |  |  |
    |  |  |+ |+ |+ |+ |  |  |   |  |  |  |  |  |  |  |  |
    |  |  |  |+ |+ |  |  |  |   |  |  |  |  |  |  |  |  |
    |  |  |  |  |  |  |  |  |   |  |  |  |  |  |  |  |  |
    |  |  |  |  |  |  |  |  |   |  |  |  |  |  |  |  |  |





## linear svm





    
## TODO
     vvv


TODO :
       unite alpha_beta_inner() and get_best_move() functions

       multithreading 
           (implemented)

       implement pawn promotion to queen (don't enrich Move type)
           (implemented, seemingly FAILs testing)

       add and tune terms for:

         miscellaneous 
           `piece-square tables`                (implemented, un-tuned)
           `bishop pair`                        (implemented, un-tuned)
           `"random" mobility term via hash`

         king safety
           `attackers in same quadrant as king` (implemented, un-tuned)
           `attackers xray king neighborhood`   ()

         pawn structure
           `pawn connectivity`                  (implemented, un-tuned)
           `weak squares`                       
           `passed pawns`                       

         piece-pawn interactions 
           `bishops-pawn malus`                 (implemented, un-tuned)
           `knights on outposts`               
           `rooks on open files`

       weariness and contempt parameters for draw handling 

       make sure hash includes who-is-to-move data!
           (resolved, clunkily)

       address possibility of no legal move

       design goal-based reductions

     ^^^


