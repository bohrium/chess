ab:
	pdflatex alpha-beta.tex
	evince alpha-beta.pdf

all:
	g++ -pthread --std=c++11 Main.cpp BoardBasic.cpp BoardMoves.cpp BoardEval.cpp Search.cpp -o main.o 

prof:
	g++ -pthread -pg --std=c++11 Main.cpp BoardBasic.cpp BoardMoves.cpp BoardEval.cpp Search.cpp -o main.o 

grind:
	g++ -pthread -g --std=c++11 Main.cpp BoardBasic.cpp BoardMoves.cpp BoardEval.cpp Search.cpp -o main.o 
	valgrind --leak-check=yes ./main.o

profile:
	g++ --std=c++11 TestSearch.cpp BoardBasic.cpp BoardMoves.cpp BoardEval.cpp Search.cpp -o testsearch.o -pg 
	./testsearch.o 

preptest:
	g++ --std=c++11 TestBoard.cpp Board.cpp -o testboard.o 
	./testboard.o > testboard_out  
	g++ --std=c++11 TestMoves.cpp Board.cpp -o testmoves.o 
	./testmoves.o > testmoves_out  
	g++ --std=c++11 TestSearch.cpp Board.cpp Search.cpp -o testsearch.o 
	./testsearch.o > testsearch_out 

test:
	g++ --std=c++11 TestBoard.cpp Board.cpp -o testboard.o 
	./testboard.o > out  
	diff out testboard_out
	g++ --std=c++11 TestMoves.cpp Board.cpp -o testmoves.o 
	./testmoves.o > out  
	diff out testmoves_out
	g++ --std=c++11 TestSearch.cpp Board.cpp Search.cpp -o testsearch.o 
	./testsearch.o > out  
	diff out testsearch_out
	

