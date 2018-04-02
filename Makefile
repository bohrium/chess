all:
	g++ --std=c++11 Main.cpp Board.cpp Search.cpp -o main.o 
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
