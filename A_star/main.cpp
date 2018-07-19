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
typedef unsigned long long LongKeyType;
typedef unsigned long long HashType;
typedef unordered_map<KeyType, char> StepMap;
typedef unordered_map<KeyType, char>  HeuMap;
typedef unordered_map<LongKeyType, char>  FMap;

using namespace std;

int A_star_search(State initState, int cutoff, vector<Step>& steps) {
    myQueue<HashType>* queue = new myQueue<HashType>();
    queue->push(getHash(initState));
    StepMap prevStep;
    HeuMap explored; // key to heuristic f
    FMap frontier; // key to heuristic f
    frontier[getLongKey(initState)] = initState.f;

    int count = 1;
    int maxQueueSize = 0;
    int maxFrontierSize = 0;

    while (!(queue->empty())) {
        if (queue->size() != frontier.size()) {
            printf ("qs: %u, fs: %u\n", queue->size(), frontier.size());
            break;
        }
        maxQueueSize = max(maxQueueSize, int(queue->size()));
        maxFrontierSize = max(maxFrontierSize, int(frontier.size()));

        State s = getState(queue->top());
        if (!isLegal(s)) {
            printf ("not legal\n");
            break;
        }
        queue->pop();

        explored[getKey(s)] = s.h;
        frontier.erase(getLongKey(s));

        if (s == goalState) {
            printf ("should find\n");
            break;
            KeyType key = getKey(s);
            while (prevStep.find(key) != prevStep.end()) {
                Step step = getStep(prevStep[key]);
                steps.push_back(step);
                s = takeStep(s, step, true);
                key = getKey(s);
            }
            reverse(steps.begin(), steps.end());

            printf ("queue: %i, frontier: %i, explored: %u, prevStep: %u\n", maxQueueSize, maxFrontierSize, explored.size(), prevStep.size());
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
            LongKeyType longKey = getLongKey(successor);
            if (explored.find(key) != explored.end())
                continue;

            if (frontier.find(longKey) != frontier.end()) {
                if (successor.f < frontier[longKey]) {
                    prevStep[key] = getHash(step);
                    frontier[longKey] = successor.f;
                    queue->remove(getHash(successor));
                    queue->push(getHash(successor));
                }
            } else {
                ++count;
                prevStep[key] = getHash(step);
                frontier[longKey] = successor.f;
                queue->push(getHash(successor));
            }
        }

        continue;
        while (queue->size() > 3000) {
            unsigned origSize = queue->size();
            myQueue<HashType>* queue2 = new myQueue<HashType>();

            for (int i = 0; i < origSize / 2; ++i) {
                HashType sHash = queue->top();
                queue->pop();
                queue2->push(sHash);
            }
            while (!(queue->empty())) {
                State state = getState(queue->top());
                KeyType key = getKey(state);
                LongKeyType longKey = getLongKey(state);
                queue->pop();
                frontier.erase(longKey);
                if (explored.find(key) != explored.end())
                    explored.erase(key);

                State prevState = takeStep(state, getStep(prevStep[key]), true);
                prevStep.erase(key);
                if (frontier.find(getLongKey(prevState)) == frontier.end()) {
                    queue2->push(getHash(prevState));
                    frontier[getLongKey(prevState)] = prevState.f;
                    // if (prevState.f > state.f)
                    //     printf ("???\n");
                }
            }
            delete queue;
            queue = queue2;
        }
    }

    delete queue;
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

