#ifndef ASTAR_STATE
#define ASTAR_STATE

#include <stdlib.h>
#include <vector>
#include <math.h>
#include "step.h"
#include "zkey.h"

using namespace std;

struct State {
    int pos[9]; // 1~8, 0 means empty
    int g;
    int h;
    int f;

} goalState;

unsigned long long mask = 15;

bool operator< (const State& s1, const State& s2) {
    return s1.f > s2.f;
}

bool operator== (const State& s1, const State& s2) {
    for (int i = 0; i < 9; ++i) {
        if (s1.pos[i] != s2.pos[i]) {
            return false;
        }
    }

    return true;
}

bool operator!= (const State& s1, const State& s2) {
    return !(s1 == s2);
}

class myComparison {
    bool reverse;
public:
    bool operator() (const unsigned long long& lhs, const unsigned long long& rhs) const {
        unsigned long long l = (lhs >> 36);
        unsigned long long r = (rhs >> 36);
        return (l < r);
    }
    bool operator() (const State& lhs, const State& rhs) const {
        return (lhs < rhs);
    }
};

int findPos(State s, int n);
int heuristic(State s);
bool isLegal(const State& s);
bool solvable(const State& s);
State randomStepState(int step);
State randomState();
void drawHorizontalLine(int y);
void drawVerticalLine(int x);
char* myToChars(int i);
char* myToChars_six(int i);
void printNumber(State s, int index);
void printState(State s);
void getAvailableSteps(State s, vector<Step>& steps);
bool getLegalStep(State s, int index, Step& step);
// short getKey(State s);
unsigned long long getKey(State s);
unsigned long long getLongKey(State s);
unsigned long long getHash(State s);
State takeStep(State s, Step step, bool reverse = false);

int findPos(State s, int n) {
    for (int i = 0; i < 9; ++i) {
        if (s.pos[i] == n) {
            return i;
        }
    }

    return 0;
}

int heuristic(State s) {
    int h = 0;
    /*
    for (int i = 0; i < 9; ++i) {
        if (s.pos[i] != goalState.pos[i]) {
            ++h;
        }
    }
    */
    for (int i = 0; i < 9; ++i) {
        int p1 = findPos(s, i);
        int p2 = findPos(goalState, i);

        h += (abs(p1/3 - p2/3) + abs(p1%3 - p2%3));
    }
    h /= 2;


    return h;
}

bool isLegal(const State& s) {
    if (s.g < 0)
        return false;

    int count = 0;
    for (int i = 0; i < 9; ++i) {
        if (findPos(s, i) == 0)
            ++count;
    }
    if (count != 1)
        return false;

    return solvable(s);
}

bool solvable(const State& s) {
    int inversion = 0;
    for (int i = 1; i < 9; ++i) {
        if (s.pos[i] == 0)
            continue;
        for (int j = 0; j < i; ++j) {
            if (s.pos[i] < s.pos[j])
                ++inversion;
        }

    }

    return (inversion % 2 == 0);
}

State randomStepState(int step) {
    State s = goalState;

    do {
        for (int i = 0; i < step; ++i) {
            vector<Step> steps;
            getAvailableSteps(s, steps);

            size_t j = rand() / (RAND_MAX / steps.size() + 1);
            s = takeStep(s, steps[j]);
        }
    } while (s == goalState);
    s.g = 0;
    s.h = heuristic(s);
    s.f = s.g + s.h;

    return s;
}

State randomState() {
    State s;
    for (int i = 0; i < 9; ++i) {
        s.pos[i] = i;
    }

    do {
        shuffle(s.pos, 9);
    } while (!solvable(s));
    s.g = 0;
    s.h = heuristic(s);
    s.f = s.g + s.h;

    return s;
}

State worstState() {
    State s;
    s.pos[0] = 8;
    s.pos[1] = 6;
    s.pos[2] = 7;
    s.pos[3] = 2;
    s.pos[4] = 5;
    s.pos[5] = 4;
    s.pos[6] = 3;
    s.pos[7] = 0;
    s.pos[8] = 1;
    s.g = 0;
    s.h = heuristic(s);
    s.f = s.g + s.h;

    return s;
}

void printState(State s) {
    for (int i = 0; i < 9; ++i) {
        printf ("%i ", s.pos[i]);
        if ((i + 1) % 3 == 0)
            printf ("\n");
    }
    printf ("g: %i, h: %i, f: %i, heu: %i\n", s.g, s.h, s.f, heuristic(s));
}

/*
short getKey(State s) {
    short key = 0;
    for (int i = 0; i < 9; ++i) {
        key ^= zkey[i * 9 + s.pos[i]];
    }

    return key;
}
*/

unsigned long long getKey(State s) {
    unsigned long long key = 0;
    for (int i = 0; i < 9; ++i) {
        unsigned long long n = s.pos[i];
        n <<= i * 4;
        key ^= n;
        // key ^= zKey[i * 9 + s.pos[i]];
    }

    return key;
}

unsigned long long getLongKey(State s) {
    unsigned long long key = 0;
    for (int i = 0; i < 9; ++i) {
        unsigned long long n = s.pos[i];
        n <<= i * 4;
        key ^= n;
        // key ^= zKey[i * 9 + s.pos[i]];
    }

    return key;
}

unsigned long long getHash(State s) {
    unsigned long long key = s.f;
    key <<= 36;
    for (int i = 0; i < 9; ++i) {
        unsigned long long n = s.pos[i];
        n <<= i * 4;
        key ^= n;
    }

    return key;
}

State getState(unsigned long long hash) {
    State s;
    for (int i = 0; i < 9; ++i) {
        s.pos[i] = hash & mask;
        hash >>= 4;
    }

    s.f = hash;
    s.h = heuristic(s);
    s.g = s.f - s.h;

    return s;
}

State takeStep(State s, Step step, bool reverse) {
    State newS = s;
    if (s.pos[step.p1] != 0 && s.pos[step.p2] != 0) {
        printf ("Step ???");
        if (reverse)
            printf ("-\n");
        else
            printf ("+\n");

    }
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

#endif

