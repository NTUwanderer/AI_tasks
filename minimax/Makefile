CXX = g++
#CXXFLAGS = -O0 -g -std=c++11
CXXFLAGS = -O2 -g -std=c++11

all: Reversi

main.o: main.cpp state.h heu1.h heu2.h heu3.h heu4.h heu5.h heu6.h heu7.h heu8.h heu9.h heu10.h 
	$(CXX) $(CXXFLAGS) -c main.cpp 

Reversi: main.o
	$(CXX) $(CXXFLAGS) -o Reversi main.o

clean:
	rm -f $(OBJ) Reversi

