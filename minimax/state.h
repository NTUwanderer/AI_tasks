#include <stdlib.h>
#include <vector>
#include <math.h>

struct State {
    int pos[8][8]; // 0: x (black), 1: o (white): x, -1 means empty
    int h;
} initState;

bool operator< (const State& s1, const State& s2) {
    return s1.h > s2.h;
}

bool operator== (const State& s1, const State& s2) {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (s1.pos[i][j] != s2.pos[i][j]) {
                return false;
            }
        }
    }

    return true;
}

bool operator!= (const State& s1, const State& s2) {
    return !(s1 == s2);
}

bool inBoard(int x, int y) {
    return (x >= 0 && x < 8 && y >= 0 && y < 8);
}

int availablePlaces(const State& s, bool (&available)[8][8], bool blackTurn) {

    int currentC = blackTurn ? 0 : 1;
    int opponentC = (currentC + 1) % 2;

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            available[i][j] = false;
            if (s.pos[i][j] >= 0) {
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

                        if (s.pos[x][y] == -1)
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

int heuristic(State& s) {

    s.h = 0;

    bool available[8][8];
    int blackMoves = availablePlaces(s, available, true);
    int whiteMoves = availablePlaces(s, available, false);
    if (blackMoves != 0 || whiteMoves != 0)
        s.h = blackMoves - whiteMoves;
    else {
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                if (s.pos[i][j] == 0)
                    ++(s.h);
                else if (s.pos[i][j] == 1)
                    --(s.h);
            }
        }
    }

    return s.h;
}

bool isEnd(const State& s) {

    bool available[8][8];
    int blackMoves = availablePlaces(s, available, true);
    int whiteMoves = availablePlaces(s, available, false);
    return (blackMoves == 0 && whiteMoves == 0);
}

void printState(State& s) {

    printf ("  a b c d e f g h\n");
    for (int i = 0; i < 8; ++i) {
        printf ("%i", i + 1);
        for (int j = 0; j < 8; ++j)
            printf (" %c", s.pos[i][j] == 0 ? 'x' : s.pos[i][j] == 1 ? 'o' : '.');

        printf ("\n");
    }
    printf ("heu: %i\n", heuristic(s));
}

State takeStep(const State& s, int i, int j, bool blackTurn) {
    State newS = s;

    int currentC = blackTurn ? 0 : 1;
    int opponentC = (currentC + 1) % 2;

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

                if (s.pos[x][y] == -1)
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
    newS.pos[i][j] = currentC;

    return newS;
}

/*
unsigned long getKey(State s) {
    unsigned long key = 0;
    for (int i = 0; i < 9; ++i) {
        key ^= zKey[i * 9 + s.pos[i]];
    }

    return key;
}

State takeStep(State s, Step step, bool reverse = false) {
    State newS = s;
    swap(newS.pos[step.p1], newS.pos[step.p2]);

    if (reverse)
        newS.g = s.g - 1;
    else
        newS.g = s.g + 1;
    newS.h = heuristic(newS);
    newS.f = newS.g + newS.h;

    return newS;
}

void getAvailableSteps(State s, vector<Step>& steps) {
    int p = 0;
    for (int i = 0; i < 9; ++i) {
        if (s.pos[i] == 0) {
            p = i;
            break;
        }
    }

    if (p >= 3)
        steps.push_back(getStep(p-3, p));

    if (p < 6)
        steps.push_back(getStep(p, p+3));

    if ((p % 3) != 0)
        steps.push_back(getStep(p-1, p));

    if ((p % 3) != 2)
        steps.push_back(getStep(p, p+1));
}
*/

