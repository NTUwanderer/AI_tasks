#include <stdio.h>
#include <queue>
#include <time.h>
#include <limits.h>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <iostream>

#include "state.h"

using namespace std;

void init() {
    for (int i = 0; i < 8; ++i)
        for (int j = 0; j < 8; ++j)
            initState.pos[i][j] = -1;
    initState.pos[3][3] = 1;
    initState.pos[4][4] = 1;
    initState.pos[3][4] = 0;
    initState.pos[4][3] = 0;
    initState.h = 0;
}

void playerMove(State& s, const bool (&available)[8][8], bool blackTurn = true) {
    char row, col;

    int x, y;
    while (true) {

        printf ("Input your move (1~8, a~f): ");
        fflush(NULL);
        cin >> row >> col;

        x = row - '1';
        y = col - 'a';

        if (inBoard(x, y) && available[x][y]) {
            s = takeStep(s, x, y, blackTurn);
            break;
        }
        printf ("Invalid Input\n");
    }
}

// return best heuristic value with a limited trace depth
int minimax(State& s, const bool (&available)[8][8], bool blackTurn, int depth, int alpha = INT_MIN, int beta = INT_MAX) {

    int bestX = -1, bestY = -1;
    int bestHeu = 0;

    bool newAvailable[8][8];

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (!available[i][j])
                continue;

            int h;
            State newS = takeStep(s, i, j, blackTurn);
            if (depth <= 1 || isEnd(newS)) {
                h = heuristic(newS);
            } else {
                availablePlaces(newS, newAvailable, !blackTurn);
                h = minimax(newS, newAvailable, !blackTurn, depth - 1);
            }

            if (bestX == -1 || (blackTurn && h > bestHeu) || (!blackTurn && h < bestHeu)) {
                bestX   = i;
                bestY   = j;
                bestHeu = h;
            }
        }
    }

    if (bestX == -1) {
        State newS = s;
        availablePlaces(newS, newAvailable, !blackTurn);
        bestHeu = minimax(s, available, !blackTurn, depth - 1);
    } else {
        s = takeStep(s, bestX, bestY, blackTurn);
    }

    return bestHeu;
} 

int main() {
    // srand(time(NULL));
    // srand(2);
    init();

    printf ("Player Mode (first serve): 1\nPlayer Mode (second serve): 2\nBot vs Bot: 3\nPlease choose the mode: ");
    fflush(NULL);
    int mode;
    cin >> mode;

    State s = initState;

    bool playerMode = (mode < 3);

    if (playerMode)
        printf ("Player: x (black)\n");

    bool blackTurn = (mode != 2);

    bool available[8][8];
    int depth = 5;

    while (!isEnd(s)) {
        printState(s);

        if (availablePlaces(s, available, blackTurn) == 0) {
            blackTurn = !blackTurn;
        } else {
            if (blackTurn && playerMode)
                playerMove(s, available);

            else
                minimax(s, available, blackTurn, depth);
        }


        blackTurn = !blackTurn;
    }

    printf ("End State:\n");
    printState(s);
    printf ("Winner: %s\n", (s.h > 0) ? "x (Black)" : (s.h < 0) ? "y (White)" : "break even");
}

