#include <stdlib.h>

#ifndef ASTAR_STEP
#define ASTAR_STEP

struct Step {
    int p1;
    int p2;
};

struct Step getStep(int i, int j) {
    struct Step step;
    step.p1 = i;
    step.p2 = j;

    return step;
}

#endif

