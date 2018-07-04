#include <stdio.h>
#include <queue>
#include <time.h>
#include <limits.h>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <iostream>

#include "util.h"
#include "state.h"
#include "myQueue.h"

typedef unordered_map<unsigned long long, Step> StepMap;
typedef unordered_map<unsigned long long, int>  HeuMap;

using namespace std;

int A_star_search(State initState, int cutoff, vector<Step>& steps) {
    myQueue<unsigned long long> queue;
    // priority_queue<unsigned long long, vector<unsigned long long>, decltype(&myHashCompare)> queue(&myHashCompare);
    // priority_queue<unsigned long long, vector<unsigned long long>, decltype(&myHashCompare)> queue(&myHashCompare);
    queue.push(getHash(initState));
    StepMap prevStep;
    HeuMap explored; // key to heuristic f
    HeuMap frontier; // key to heuristic f
    frontier[getKey(initState)] = initState.f;

    int count = 1;
    while (!(queue.empty())) {
        State s = getState(queue.top());
        queue.pop();

        explored[getKey(s)] = s.h;
        frontier.erase(getKey(s));

        if (s == goalState) {
            unsigned long long key = getKey(s);
            while (prevStep.find(key) != prevStep.end()) {
                Step step = prevStep[key];
                steps.push_back(step);
                s = takeStep(s, step, true);
                key = getKey(s);
            }
            reverse(steps.begin(), steps.end());
            return count;
        }

        vector<Step> avSteps;
        getAvailableSteps(s, avSteps);

        for (int i = 0; i < avSteps.size(); ++i) {
            Step step = avSteps[i];
            State successor = takeStep(s, step);
            unsigned long long key = getKey(successor);
            if (explored.find(key) != explored.end())
                continue;

            if (frontier.find(key) != explored.end()) {
                if (successor.f < frontier[key]) {
                    prevStep[key] = step;
                    frontier[key] = successor.f;
                    queue.remove(getHash(successor));
                    queue.push(getHash(successor));
                }
            } else {
                ++count;
                prevStep[key] = step;
                frontier[key] = successor.f;
                queue.push(getHash(successor));
            }
        }
    }

    return count;
}

int main() {
    // srand(time(NULL));
    srand(2);
    for (int i = 1; i <= 8; ++i) {
        goalState.pos[i-1] = i;
    }
    goalState.pos[8] = 0;
    goalState.h = 0;
    goalState.g = -1;
    goalState.f = -1;


    for (int index = 0; index < 1; ++index) {
        State initState = randomState();
        printState(initState);

        vector<Step> steps;
        int cutoff = INT_MAX;
        int count = A_star_search(initState, cutoff, steps);

        printf ("count: %i, step size: %i\n", count, steps.size());
        State s = initState;
        for (int i = 0; i < steps.size(); ++i) {
            s = takeStep(s, steps[i]);
            // printState(s);
        }
        if (s == goalState)
            printf ("Success!\n");
        else
            printf ("Fail!\n");
    }
}

