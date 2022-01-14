       resolve illegal and board-corrupting (upon undo?) Ra8(P) first move.
           (resolved)

       analyze mystery of hash table underusage
           partial progress: in apply_move/undo_move, computation of `mover`
                             should know that the `mover` piece is in different
                             squares (dest or source)!  thus originally hash
                             computation was incorrect and led to path
                             dependent value
           partial progress: orig_alpha

       analyze mystery of core dumps as recorded in commits for game-00, -01
           (seemingly resolved)

       implement "get best k" moves for small k using alpha beta cutoffs.
            (implemented, sorta tested, unused)

       add alpha/beta cutoffs to stable_eval, too!
            (implemented)

       implement stable node eval (linear greedy search for pass/bestcapture) 
         (implemented.)



We especially play with **goal based reductions** toward programs that behave
according to `plans'.  For us, a *goal* is a piece-square element, i.e. an
element of 'pnbrqk PNBRQK'x[0,8)[0,8) or a small monotonic logical combination
thereof.  Every several plies, if we are in a quiet position, we select a
handful of goals to try for, and during search we reduce branches that don't
seem to be helping us toward these goals.  Four related questions:

    How do we select interesting goals to try for?
    How do we assess progress toward a goal?
    How do we bias search toward a goal (reduce away from that goal)? 
    How do we tell whether a goal-directed search ended up being worthwhile? 

When is a goal interesting?  A sufficient criterion might be to run three
experiments, as follows (thought this might yield very shallow goals?):
    
    A ordinary      search with moderate depth and      narrow width
    B ordinary      search with shallow  depth and very narrow width
    C goal-directed search with shallow  depth and very narrow width

We regard A as a gold standard.  If B does not find A's best move but C does
find A's best move, then the goal might be a good one to consider. 

Another interesting criterion might be to do a simulation where we make a few
moves alternating with null moves (or perhaps shallow greedy moves, perhaps
greedy with respect to ordinary evaluation) by the opponent, and we maximize
for positional rather than total score (perhaps disallow captures?).  The best
position gotten against a weak (or perhaps even cooperative?) opponent gives us
positional information to aspire to (but might also lead to too-ambitious
goals). 

I like the latter approach a bit more, partly because it suggests rather than
filters candidate goals.  So we start with a board.  We consider the game tree 
up to depth 6 (but really depth 3, since opponent makes null moves) and find
a leaf board with greatest positional score.  Comparing this (sparsely updated)
board with our original board yields a small handful of interesting `changes`
to try to effect (though sometimes a human might have the goal of maintaining
status quo, e.g. having a piece stay put?). 

Once we have goals in mind, we might compute  


