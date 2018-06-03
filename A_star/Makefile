CXX = g++
CXXFLAGS = -O2 -g -std=c++11

.SUFFIXES: .cpp
.o :
	$(CXX) $(CXXFLAGS) -c $<

SRC = main.cpp

OBJ = $(addsuffix .o, $(basename $(SRC)))

all: EightPuzzle

EightPuzzle: $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ)

# main.o: util.h state.h myQueue.h zkey.h 

