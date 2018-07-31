CXX = g++
#CXXFLAGS = -O0 -g -std=c++11
CXXFLAGS = -O2 -g -std=c++11

.SUFFIXES: .cpp
.o :
	$(CXX) $(CXXFLAGS) -c $<

SRC = main.cpp

OBJ = $(addsuffix .o, $(basename $(SRC)))

all: Reversi

Reversi: $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ)

clean:
	rm -f $(OBJ) Reversi

