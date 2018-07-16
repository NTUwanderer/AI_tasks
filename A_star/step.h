#ifndef ASTAR_STEP
#define ASTAR_STEP

#include <stdlib.h>

unsigned long long step_mask = 15;

struct Step {
    int p1;
    int p2;
};

Step getStep(int i, int j) {
    struct Step step;
    step.p1 = i;
    step.p2 = j;

    return step;
}

char getHash(Step step) {
    char hash = 0;
    hash += step.p1;
    hash <<= 4;
    hash += step.p2;

    return hash;
}

Step getStep(char hash) {
    Step step;
    step.p2 = hash & step_mask;
    hash >>= 4;
    step.p1 = hash & step_mask;

    return step;
}


bool operator== (const Step& s1, const Step& s2) {
    if ((s1.p1 == s2.p1 && s1.p2 == s2.p2) || (s1.p1 == s2.p2 && s1.p2 == s2.p1))
        return true;
    return false;
}

bool operator!= (const Step& s1, const Step& s2) {
    return !(s1 == s2);
}

#endif

