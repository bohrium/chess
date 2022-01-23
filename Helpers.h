#ifndef HELPERS_H
#define HELPERS_H

#define ANSI_YELLOW     "\033[0;33m"
#define ANSI_MAGENTA    "\033[1;35m"
#define ANSI_CYAN       "\033[1;36m"
#define ANSI_BLUE       "\033[0;34m"
#define ANSI_RED        "\033[0;31m"
#define ANSI_GREEN      "\033[0;32m"
#define DEFAULT_COLOR   ANSI_YELLOW

#define GO_DOWN(N) {for (int k=0; k!=N; ++k) { std::cout << std::endl; }}
#define GO_UP(N) {for (int k=0; k!=N; ++k) { std::cout << "\033[1A" << std::flush; }}

#define CLEAR_REST_OF_LINE                              \
{                                                       \
    for (int k=0; k!=120; ++k) { std::cout << " "; }    \
    std::cout << "\33[120D";                            \
}
#define SHOW_SIGN(X) (std::showpos)<<X<<(std::noshowpos)
#define FLUSH_RIGHT(N,X) (std::setw(N))<<(std::right)<<X
#define COLORIZE(C,X) C<<X<<DEFAULT_COLOR

#define MIN(X,Y) (((X)<(Y))?(X):(Y))
#define MAX(X,Y) (((X)>(Y))?(X):(Y))
#define ABS(X) ((0<=(X))?(X):(-(X)))
#define KRON(N) ((N)?1:0)

#define CLONE_BACK(V) ((V).push_back((V).back()))
#define GET_FROM_LAST(V,N) ((V)[((V).size()-N)])

#endif//HELPERS_H
