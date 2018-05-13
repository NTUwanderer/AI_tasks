#include <stdlib.h>
#include <vector>
#include <math.h>
#include "zkey.h"
#include "step.h"

ZKey zKey;

struct State {
    int pos[9]; // 1~8, 0 means empty
    int g;
    int h;
    int f;

    /*
    State& operator= (const State& s) {
        for (int i = 0; i < 9; ++i) {
            pos[i] = s.pos[i];
        }
    
        g = s.g;
        h = s.h;
        f = s.f;
    
        return *this;
    }
    */

} goalState;

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

bool solvable(State s) {
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

void printState(State s) {
    for (int i = 0; i < 9; ++i) {
        printf ("%i ", s.pos[i]);
        if ((i + 1) % 3 == 0)
            printf ("\n");
    }
    printf ("g: %i, h: %i, f: %i, heu: %i\n", s.g, s.h, s.f, heuristic(s));
}

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

