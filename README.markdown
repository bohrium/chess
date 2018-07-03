# chess

## overview 
This project has two aims: to practice tree search and to use learning techniques to infer and visualize interpretable
chess heuristics.

### modified rules of chess
We modify the standard rules of chess.  We keep castling, en passant, and pawn promotion.  The modified rules are blind
to the concept of check.  However, kings may move into check, stay in check, and castle through check.  However, in
what we call **generalized en passant**, kings that castle through check may be captured in the next ply.  The modified
rules do not recognize draws, whether by agreement, by repetition, by stalemate, by insufficient material, or by the
50-move rule.  The aim is not checkmate; the aim is to capture the enemy king. 

### tree search
We use variable-depth tree search with a hand-crafted evaluation function.  For us, "variable-depth" means
**fractional+quiescent search**, by which we interpolate between fixed-depth tree search and greedy beam search.  Each
each move that decreases the heuristic costs 3/2 of a depth; each move that improves the heuristic by more than 150
centipawns is free; all other moves cost 1 depth.  We aim for a depth budget of 8.  We implement 4 speed optizations:

- **reversible moves** (our version of Smith notation --- allows us to keep one board instance in `main`)
- **difference evaluation** (each move updates board sparsely, so it's faster to compute d(heuristic) and integrate) 
- **alpha-beta pruning** (with non-infinite initial values set to induce a cutoff when a king is taken)
- **recursive move ordering** (call shallow alpha-beta to determine move ordering of deep alpha-beta)

Our hand-crafted heuristic is measured in centipawns:

    score = material + control_of_center + space

Here, we use Fischer's estimates (p,n,b,r,q,k = 101,301,325,501,901,100000) for `material`.  As shown, we actually add
1 centipawn to each piece, thus breaking ties in favor of having more pieces. 
Meanwhile, we measure `control_of_center` by counting the number of attacks to the central 4 squares.  An **attack**
from `src` to `dst` we define as a legal move from `src` to `dst` in the potentially counterfactual world where `dst`
contains an enemy piece.  Capturing a pawn or king by generalized en passant doesn't count.  Each attack is worth 34
centipawns.
Finally, `space` awards 21 centipawns for each possessed square.  A square is **possessed** by white if
`nb_white_attackers - nb_black_attackers > 2 - nb_black_occupants`. 
Of course, white's pieces, attacks, and possessed squares count as positive, while black's count as negative.

### linear svm


    
## issues
trouble profiling... try gprof on caen machines?

want to tune heuristic... get Matthew S to look at TestEvalRuy and TestEvalSicilian (and make a middlegame and endgame
testeval too)? 

somehow, move ordering seems to affect opening used... this is bad (unless there are ties in evaluation?)!

todo: add castling and (queen only?) promotion.  ((promotion to other pieces?), en passant, and draw rules are too obscure)

idea: middle game is when absolute number of possessed squares begins to decrease 
