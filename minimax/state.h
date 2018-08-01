#ifndef STATE_H
#define STATE_H

#include <stdlib.h>
#include <vector>
#include <math.h>
#include <string>

#define RED "\033[0;32;31m"
#define BLUE "\033[0;32;34m"
#define NONE "\033[m"

using namespace std;

struct State {
    bool pos[8][8]; // false: x (red), true: o (blue)
    bool exist[8][8];
};

bool operator== (const State& s1, const State& s2) {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (s1.exist[i][j] != s2.exist[i][j])
                return false;
            if (s1.exist[i][j] && (s1.pos[i][j] != s2.pos[i][j]))
                return false;
        }
    }

    return true;
}

bool operator!= (const State& s1, const State& s2) {
    return !(s1 == s2);
}

bool inBoard(int x, int y);
int countResult(const State& s);
void countResult(const State& s, int& r, int& b);
int availablePlaces(const State& s, bool (&available)[8][8], bool redTurn);
double heuristic(const State& s);
bool isEnd(const State& s);
void printState(State& s, bool redTurn);
State takeStep(const State& s, int i, int j, bool redTurn);
bool randomMove(const State&s, int& mX, int& mY, int nMoves, bool (&available)[8][8]);

bool inBoard(int x, int y) {
    return (x >= 0 && x < 8 && y >= 0 && y < 8);
}

int countResult(const State& s) {
    int result = 0;
    for (int i = 0; i < 8; ++i)
        for (int j = 0; j < 8; ++j)
            result += (!s.exist[i][j]) ? 0 : (!s.pos[i][j]) ? 1 : -1;

    return result;
}

void countResult(const State& s, int& r, int& b) {
    r = 0;
    b = 0;
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (s.exist[i][j] == false)
                continue;

            if (s.pos[i][j])
                ++b;
            else
                ++r;
        }
    }
}

int availablePlaces(const State& s, bool (&available)[8][8], bool redTurn) {

    bool currentC = redTurn ? false : true;
    bool opponentC = !currentC;

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            available[i][j] = false;
            if (s.exist[i][j]) {
                continue;
            }

            // direction (a, b)
            for (int a = -1; a <= 1; ++a) {
                for (int b = -1; b <= 1; ++b) {
                    if (a == 0 && b == 0)
                        continue;

                    bool findOpponent = false;
                    int nStep = 1;
                    while (true) {
                        int x = i + nStep * a, y = j + nStep * b;
                        if (!(inBoard(x, y)))
                            break;

                        if (!s.exist[x][y])
                            break;

                        if (s.pos[x][y] == opponentC)
                            findOpponent = true;

                        if (s.pos[x][y] == currentC) {
                            available[i][j] = findOpponent;
                            break;
                        }

                        ++nStep;
                    }

                    if (available[i][j])
                        break;
                }
                if (available[i][j])
                    break;
            }
        }
    }
    
    int count = 0;
    for (int i = 0; i < 8; ++i)
        for (int j = 0; j < 8; ++j)
            if (available[i][j])
                ++count;

    return count;

}

double heuristic(const State& s) {

    int h = 0;

    bool available[8][8];
    int redMoves = availablePlaces(s, available, true);
    int blueMoves = availablePlaces(s, available, false);
    if (redMoves != 0 || blueMoves != 0)
        h = redMoves - blueMoves;
    else {
        h = countResult(s);
    }

    return (double)h;
}

bool isEnd(const State& s) {

    bool available[8][8];
    int redMoves = availablePlaces(s, available, true);
    int blueMoves = availablePlaces(s, available, false);
    return (redMoves == 0 && blueMoves == 0);
}

void printState(State& s, bool redTurn) {

    printf ("  a b c d e f g h\n");
    for (int i = 0; i < 8; ++i) {
        printf ("%i", i + 1);
        for (int j = 0; j < 8; ++j) {
            if (s.exist[i][j]) {
                if (s.pos[i][j]) {
                    printf (BLUE " o" NONE);
                } else {
                    printf (RED " x" NONE);
                }
            } else {
                printf ("  ");
            }

        }

        printf ("\n");
    }
}

State takeStep(const State& s, int i, int j, bool redTurn) {
    State newS = s;

    bool currentC = redTurn ? false : true;
    bool opponentC = !currentC;

    // direction (a, b)
    for (int a = -1; a <= 1; ++a) {
        for (int b = -1; b <= 1; ++b) {
            if (a == 0 && b == 0)
                continue;

            bool check = false;
            bool findOpponent = false;
            int nStep = 1;
            while (true) {
                int x = i + nStep * a, y = j + nStep * b;
                if (!(inBoard(x, y)))
                    break;

                if (!s.exist[x][y])
                    break;

                if (newS.pos[x][y] == opponentC)
                    findOpponent = true;

                if (newS.pos[x][y] == currentC) {
                    check = findOpponent;
                    break;
                }

                ++nStep;
            }

            if (check) {
                for (int k = 1; k < nStep; ++k) {
                    int x = i + k * a, y = j + k * b;
                    newS.pos[x][y] = currentC;
                }
            }
        }
    }
    newS.exist[i][j] = true;
    newS.pos[i][j] = currentC;
    heuristic(newS);

    return newS;
}

bool randomMove(const State&s, int& mX, int& mY, int nMoves, bool (&available)[8][8]) {
    
    if (nMoves <= 0) {
        printf ("It's endState");
        return false;
    }
    size_t k = rand() % nMoves;
    size_t ith = 0;
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (available[i][j]) {
                if (ith == k) {
                    mX = i;
                    mY = j;
                    return true;
                }
                ++ith;
            }
        }
    }

    return false;
}

#endif

