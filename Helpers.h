#ifndef HELPERS_H
#define HELPERS_H

#define GRAY    "\033[0;90m"

#define BLUE    "\033[0;34m"
#define RED     "\033[0;31m"
#define GREEN   "\033[0;32m"

#define YELLOW  "\033[0;33m"
#define MAGENTA "\033[1;35m"
#define CYAN    "\033[1;36m"

#define DEFAULT_COLOR   GRAY

#define REPEAT_SAY(N,S)     {REPEAT(N,_, std::cout<<S); std::cout<<std::flush;}

#define GO_DOWN(N)     REPEAT_SAY(N,"\n"     )
#define GO_UP(N)       REPEAT_SAY(N,"\033[1A")
#define GO_RIGHT(N)    REPEAT_SAY(N,"\033[1C")
#define GO_LEFT(N)     REPEAT_SAY(N,"\033[1D")
#define WHITE_OUT(N)   REPEAT_SAY(N," "      )

#define CLEAR_LINE(N)              \
{                                  \
    WHITE_OUT(N);                  \
    GO_LEFT(120+N);                \
}

#define SHOW_SIGN(X) (std::showpos)<<X<<(std::noshowpos)
#define FLUSH_RIGHT(N,X) (std::setw(N))<<(std::right)<<X
#define FLUSH_RIGHT_POS(N,X) (std::setw(N))<<(std::right)<<(std::showpos)<<X<<(std::noshowpos)
#define COLORIZE(C,X) (C)<<X<<DEFAULT_COLOR

#define MIN(X,Y) (((X)<(Y))?(X):(Y))
#define MAX(X,Y) (((X)>(Y))?(X):(Y))
#define ABS(X) ((0<=(X))?(X):(-(X)))
#define KRON(N) ((N)?1:0)

#define CLONE_BACK(V) ((V).push_back((V).back()))
#define GET_FROM_LAST(V,N) ((V)[((V).size()-N)])

#define REPEAT(N,V,STMNT) {for (int V=0; V!=(N); ++V) { STMNT; }}

#endif//HELPERS_H
