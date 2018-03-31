all:
	g++ --std=c++11 Main.cpp -o main.o 

test:
	g++ --std=c++11 TestBoard.cpp -o testboard.o 
	./testboard.o > out  
	diff out testboard_out
