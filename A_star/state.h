#include <stdlib.h>
#include <vector>
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
    return s1.f < s2.f;
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

int heuristic(State s) {
    int h = 0;
    for (int i = 0; i < 9; ++i) {
        if (s.pos[i] != goalState.pos[i]) {
            ++h;
        }
    }

    return h;
}

State randomState() {
    State s;
    for (int i = 0; i < 9; ++i) {
        s.pos[i] = i;
    }

    shuffle(s.pos, 9);
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
    printf ("heuristic: %i\n", heuristic(s));
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

void getAvailableSteps(State s, vector<Step> steps) {
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

