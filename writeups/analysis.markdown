We consider alpha-beta style reduction techniques in the following model.
The `true` gametree has constant branching factor B and depth D.  So there are
D+1 layers.  Each node n has a static evaluation s(n), viewed as a noisy
observation of a tree-markov hidden static evaluation h(n).  All noise and
updates are **unbiased** and **independent**:

    s(n') ~ h(n') + e(n),       e(n') ~ N(0, 1000) 
    h(n') ~ h(n) + u(n'),       u(n') ~ N(0, 100) 
    s(0) = h(0) ~ 0 

Here n' is a child of n and all units are in centipawns and the node 0 is the
root node.  e(n) might include width from capture fests --- with typical scale
two rooks (~1000 centipawns) and non-negligible probability ~1/150 of king
capture (~5000 centipawns, or roughly all pieces combined plus a second queen).
Ignoring noise, we have that the hidden static evaluation h(n) ~ N(0, 100
sqrt(120)) after 120 plies (a typical game length) is typically worth two rooks
(~1000 centipawns), which seems reasonable.

We wish to estimate the minimax score of the leaf nodes' hidden static
evaluations.  To start with, we might apply alpha-beta to the observed s(n)s
at the leaves.

Okay, what heuristics arise from this model?  For one, it might behoove us to 
reduce children we estimate to have relatively low h(n).
