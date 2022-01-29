# chess

TODO: track nb-plies-since-irreversible in hash 

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

We partition the six species of chess-person or *chess-stone* into *pawns*,
*pieces*, and *kings*.  Knights, bishops, rooks, and queens are pieces, but
pawns and kings aren't.  The classical board evaluation is linear in population
counts: it grants 100,300,300,500,900 points for the pnbrq.  Our heuristic
supplements this term by several others in order to treat five themes: 
**king attacks**,
**pawn structure** (determined entirely by pawn locations), 
**material** (determined entirely by population counts), 
**piece-pawn interactions**, and
**ideal squares**.
Since we set these by intuition, we use a very coarse set of weights: 5, 13,
34, 89 centipawns. 

    score =   king-xray + king-tropism + king-shelter
            + pawn-connectivity + weak-squares + passed-pawns 
            + material-linear + bishop-pair + redundant-majors + cramped-rook
            + cramped-bishop + knight-outpost + opened-rook
            + piece-square 

Most terms are quadratic in our one-hot board representation.  All arise by 
combining factors sparsely updated in each move --- a boon to fast computation.

The `king-xray` term adds `+13` for each potentially-blocked attack by an
friendly piece to the enemy king's square or a square a king-move away.  By
*potentially-blocked attack*, we mean that we ignore occlusions.  In the
following board (left), for instance, we add (+13)(2+2+1+2+2) to white's score
(based on attacks from QBNNR) and (+13)(2+2+2) to black's score (based on
attacks from qrr).

The `king-tropism` term adds `+34` for each friendly piece in the same quadrant
as the enemy king.  The `king-shelter` term adds `+13` for each friendly
pawn in front of and a king-move away from the king.  In the example below,
white gets 68 in tropism score to black's 34.  White gets 13 in shelter score
to black's 39. 
 
       king attacks                pawn structure       
    [q][ ][ ][ ][ ][ ][k][ ]    [ ][ ][ ][ ][ ][ ][k][ ]
    [r][p][p][ ][ ][p][p][p]    [ ][ ][p][ ][ ][ ][p][ ]
    [ ][ ][ ][ ][ ][ ][ ][ ]    [ ][ ][ ][p][ ][p][p][ ]
    [ ][ ][ ][ ][p][N][N][ ]    [ ][ ][ ][ ][p][ ][ ][ ]
    [ ][ ][ ][ ][P][ ][ ][ ]    [ ][P][ ][P][P][ ][ ][ ]
    [ ][P][Q][ ][ ][P][ ][ ]    [ ][P][ ][ ][ ][ ][P][ ]
    [r][B][P][ ][ ][ ][P][P]    [ ][ ][ ][ ][ ][ ][P][P]
    [ ][K][ ][ ][ ][ ][ ][R]    [ ][K][ ][ ][ ][ ][ ][ ]

To obtain `pawn-connectivity`, we add `-34` for each pair of adjacent columns
not both containing white pawns.  For example, in the above board (right), we
add -34x7 to white's score and we add -34x3 to black's score.  Even though
white has one more pawn than black, the `pawn-connectivity` term suggests that
the players' pawn structures are roughly equal.

A square is *weak* when it is neither behind a friendly pawn nor horizontally
adjacent to a square in front of a friendly pawn.  If a weak square is
additionally attacked by an enemy pawn AND in front of a friendly pawn, it is
an *outpost* for the enemy.  The `weak-squares` penalty adds `-13` for each
enemy outpost and `-5` for each remaining weak square.  The `passed-pawn` term
grants `+34` for each friendly pawn on a square weak for the enemy.  In the
example board above, black has 8+2+1+2+3+2+1+2 weak squares; of these, none are
outposts for white and none are passed pawns for white.  White has
3+4+3+3+3+3+4+4 weak squares; of these, 0+1+0+0+0+0+0+0 are outposts for black
and none are passed pawns for black.  So this paragraph's terms award 105 to
white and 138 to black.   

The `cramped-bishop` term penalize bishops of the same square parity as many of
their own side's pawns (`-5` per pawn-bishop pair of same color and same square
parity).  The `knight-outpost` term rewards knights on outposts (`+89` per) or
non-outpost weak squares (`+34` per).  The `opened-rook` terms grants `+34` for
each rook on an *open file* (a file with neither friendly nor enemy pawns) and
`+13` for each rook on a *semi-open file* (a file with enemy but not friendly
pawns).

We value `material-linear ` according to p,n,b,r,q =
100,300+13,300+34,600-34,1000.  The `bishop-pair` correction grants `+34` for
each pair of friendly bishops on oppositely-colored squares.  The
`cramped-rook` correction exacts a penalty `-13` for each friendly rook-pawn
pair.  And the `redundant-majors` correction exacts a penalty `-13` for each
friendly rook-rook, rook-queen, or queen-queen pair.  Note that at the game's
start, the rrq contribute 2x(600 - 8x13)+1000-3x34 = 1890, consistent with a
more familiar evaluation of r,q=500,900.  Meanwhile, if we also keep in mind
the cramped-bishop term (under the piece-pawn interactions), the nnbb at the
game's start contribute 2x(300+13 + 300+34) + 34 - 8x5 = 1288, consistent with
a more familiar evaluation of nn,bb=600,700.  These adjustments are based
roughly on Larry Kaufman's [inspiring essay on evaluating material
imbalances](https://www.chess.com/article/view/the-evaluation-of-material-imbalances-by-im-larry-kaufman).

The `piece-square` adjustment incentivizes piece placements known to be good
on average independent of other pieces's placements (hence, btw,
without loss absorbs the `material` term) using values simplified from those of
Tomasz Michniewski's [instructive article on piece-square
tables](https://www.chessprogramming.org/Simplified_Evaluation_Function).
Roughly, we grant -89,-34,-13,-5,+5,+13,+34,+89 for 
(==,=,--,-,+,++,#,##) squares ranging from horrible through great: 

             pawns                       knights
    |  |  |  |  |  |  |  |  |   |==|--|--|--|--|--|--|==|
    |##|##|##|##|##|##|##|##|   |--|- |- |- |- |- |- |--|
    |  |# |# |# |# |# |# |  |   |--|- |  |  |  |  |- |--|
    |  |  |  |++|++|  |  |  |   |--|- |  |# |# |  |- |--|
    |  |  |  |+ |+ |  |  |  |   |--|- |  |++|++|  |- |--|
    |  |  |  |  |  |  |  |  |   |--|- |  |  |  |  |- |--|
    |  |  |  |  |  |  |  |  |   |--|- |- |- |- |- |- |--|
    |  |  |  |  |  |  |  |  |   |==|--|--|--|--|--|--|==|

             bishops                     rooks           
    |- |- |- |- |- |- |- |- |   |  |  |  |  |  |  |  |  |
    |- |++|  |  |  |  |++|- |   |+ |++|++|++|++|++|++|+ |
    |- |  |++|  |  |++|  |- |   |  |  |  |  |  |  |  |  |
    |- |  |  |++|++|  |  |- |   |  |  |  |  |  |  |  |  |
    |- |  |  |++|++|  |  |- |   |  |  |  |  |  |  |  |  |
    |- |  |++|  |  |++|  |- |   |  |  |  |  |  |  |  |  |
    |- |++|  |  |  |  |++|- |   |  |  |  |  |  |  |  |  |
    |- |- |- |- |- |- |- |- |   |  |  |  |  |  |  |  |  |

             queens                      kings          
    |  |  |  |  |  |  |  |  |   |  |  |  |  |  |  |  |  |
    |  |  |  |+ |+ |  |  |  |   |  |  |  |  |  |  |  |  |
    |  |  |+ |++|++|+ |  |  |   |  |  |  |+ |+ |  |  |  |
    |  |+ |++|# |# |++|+ |  |   |  |  |+ |++|++|+ |  |  |
    |  |+ |++|# |# |++|+ |  |   |  |  |+ |++|++|+ |  |  |
    |  |  |+ |++|++|+ |  |  |   |  |  |  |+ |+ |  |  |  |
    |  |  |  |+ |+ |  |  |  |   |  |  |  |  |  |  |  |  |
    |  |  |  |  |  |  |  |  |   |  |  |  |  |  |  |  |  |





## linear svm





    
## TODO
     vvv


TODO :
       unite alpha_beta_inner() and get_best_move() functions
            (implemented)

       address possibility of no legal move, e.g. in stable_eval.
       this seems to be giving segfaults

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
           `attackers xray king neighborhood`   (implemented, un-tuned) 

         pawn structure
           `pawn connectivity`                  (implemented, un-tuned)
           `weak squares`                       (implemented, un-tuned)
           `passed pawns`                       

         piece-pawn interactions 
           `bishops-pawn malus`                 (implemented, un-tuned)
           `knights on outposts`                (implemented, un-tuned, commented out)
           `rooks on open files`                (implemented, un-tuned)

       weariness and contempt parameters for draw handling 

       make sure hash includes who-is-to-move data!
           (resolved, clunkily)

       design goal-based reductions

     ^^^


