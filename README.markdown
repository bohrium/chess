# chess

## overview 
This project has two aims: to practice tree search and to use learning techniques to infer and visualize interpretable
chess heuristics.

### tree search
We use constant-depth tree search with a hand-crafted leaf evaluation function and 4 optimizations:
*   reversible move (our version of Smith notation --- allows us to keep one board instance in Main)
*   alpha-beta pruning (with initial values to induce cutoff at checkmate)
*   recursive move ordering (call shallow alpha-beta to determine move ordering of deep alpha-beta)
*   difference evaluation (moves are sparse board updates, so it's faster to compute d(heuristic) and integrate) 

Our heuristic is simple:

    score = material + 1.5 * centrality + 0.5 * aggression

where we use Fischer's estimates (p,n,b,r,q = 1,3,3.25,5,9) for `material`, where `centrality` is distance-squared to
the board's center (affinely scaled to lie in [0, 1]), and where `aggression` counts the number of threats by lesser
pieces to greater pieces.  Only the last term fails to be linear in our one-hot board representation.  Thus, the last
term was the most trouble to implement difference evaluation for.

### linear svm

    
## issues
trouble profiling... try gprof on caen machines?

want to tune heuristic... get Matthew S to look at TestEvalRuy and TestEvalSicilian (and make a middlegame and endgame
testeval too)? 

somehow, move ordering seems to affect opening used... this is bad (unless there are ties in evaluation?)!
