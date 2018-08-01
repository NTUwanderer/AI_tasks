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

// typedef short KeyType;
typedef unsigned long long KeyType;
typedef unsigned long long HashType;
typedef unordered_map<KeyType, char> StepMap;
typedef unordered_map<KeyType, char>  HeuMap;
typedef unordered_map<KeyType, char>  FMap;

using namespace std;

int A_star_search(State initState, int cutoff, vector<Step>& steps) {
    myQueue<State> queue;
    queue.push(initState);
    StepMap prevStep;
    HeuMap explored; // key to heuristic f
    FMap frontier; // key to heuristic f
    frontier[getKey(initState)] = initState.f;

    int count = 1;

    while (!(queue.empty())) {
        if (queue.size() != frontier.size()) {
            printf ("qs: %u, fs: %u\n", queue.size(), frontier.size());
            break;
        }

        State s = queue.top();
        if (!isLegal(s)) {
            printf ("not legal\n");
            break;
        }
        queue.pop();

        explored[getKey(s)] = s.h;
        frontier.erase(getKey(s));

        if (s == goalState) {
            printf ("should find\n");
            KeyType key = getKey(s);
            while (prevStep.find(key) != prevStep.end()) {
                Step step = getStep(prevStep[key]);
                steps.push_back(step);
                s = takeStep(s, step, true);
                key = getKey(s);
            }
            reverse(steps.begin(), steps.end());

            break;
        }

        vector<Step> avSteps;
        getAvailableSteps(s, avSteps);

        for (int i = 0; i < avSteps.size(); ++i) {
            KeyType origKey = getKey(s);
            Step step = avSteps[i];
            // if (prevStep.find(origKey) != prevStep.end() && step == getStep(prevStep[getKey(s)]))
            //     continue;

            State successor = takeStep(s, step);
            KeyType key = getKey(successor);
            if (explored.find(key) != explored.end())
                continue;

            if (frontier.find(key) != frontier.end()) {
                if (successor.f < frontier[key]) {
                    prevStep[key] = getHash(step);
                    frontier[key] = successor.f;
                    queue.remove(successor);
                    queue.push(successor);
                }
            } else {
                ++count;
                prevStep[key] = getHash(step);
                frontier[key] = successor.f;
                queue.push(successor);
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
        // State initState = worstState();
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

