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
	g++ --std=c++11 TestEvalRuy.cpp Board.cpp Search.cpp -o testevalruy.o 
	./testevalruy.o > out  
	diff out testevalruy_out
	g++ --std=c++11 TestEvalSicilian.cpp Board.cpp Search.cpp -o testevalsicilian.o 
	./testevalsicilian.o > out
	diff out testevalsicilian_out
	
preptest:
	g++ --std=c++11 TestBoard.cpp Board.cpp -o testboard.o 
	./testboard.o > testboard_out  
	g++ --std=c++11 TestMoves.cpp Board.cpp -o testmoves.o 
	./testmoves.o > testmoves_out  
	g++ --std=c++11 TestSearch.cpp Board.cpp Search.cpp -o testsearch.o 
	./testsearch.o > testsearch_out 
	g++ --std=c++11 TestEvalRuy.cpp Board.cpp Search.cpp -o testevalruy.o 
	./testevalruy.o > testevalruy_out 
	g++ --std=c++11 TestEvalSicilian.cpp Board.cpp Search.cpp -o testevalsicilian.o 
	./testevalsicilian.o > testevalsicilian_out 

