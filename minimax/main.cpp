#include <stdio.h>
#include <queue>
#include <time.h>
#include <limits.h>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <iostream>

#include "state.h"
#include "heu1.h"
#include "heu2.h"
#include "heu3.h"
#include "heu4.h"
#include "heu5.h"
#include "heu6.h"
#include "heu7.h"
#include "heu8.h"
#include "heu9.h"
#include "heu10.h"

using namespace std;

State initState;
int player1 = 1;
int player2 = 1;

void init() {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            initState.exist[i][j] = false;
            initState.pos[i][j] = false;
        }
    }
    initState.exist[3][3] = true;
    initState.exist[4][4] = true;
    initState.exist[3][4] = true;
    initState.exist[4][3] = true;

    initState.pos[3][3] = true;
    initState.pos[4][4] = true;
    initState.pos[3][4] = false;
    initState.pos[4][3] = false;
    
}

void playerMove(State& s, const bool (&available)[8][8], bool redTurn = true) {
    char row, col;

    int x, y;
    while (true) {

        printf ("Input your move (1~8, a~f): ");
        fflush(NULL);
        cin >> row >> col;

        x = row - '1';
        y = col - 'a';

        if (inBoard(x, y) && available[x][y]) {
            State newS = takeStep(s, x, y, redTurn);
            if (newS == s) {
                cout << "Error!!!!!!!!!!!" << endl;
            }
            s = newS;
            break;
        }
        printf ("Invalid Input\n");
    }
}

int playerHeuristic(const State& s, bool firstPlayer) {
    int player = (firstPlayer ? player1 : player2);

    switch(player) {
        case 1:
            return heuristic1(s);
        case 2:
            return heuristic2(s);
        case 3:
            return heuristic3(s);
        case 4:
            return heuristic4(s);
        case 5:
            return heuristic5(s);
        case 6:
            return heuristic6(s);
        case 7:
            return heuristic7(s);
        case 8:
            return heuristic8(s);
        case 9:
            return heuristic9(s);
        case 10:
            return heuristic10(s);
        default:
            return heuristic(s);
    }

    return heuristic(s);
}

// return best heuristic value with a limited trace depth
int minimax(const State& s, int& mX, int& mY, bool redTurn, int depth, bool firstPlayer = true, int alpha = INT_MIN, int beta = INT_MAX) {

    mX = -1;
    mY = -1;

    int bestHeu = (redTurn ? -10000 : 10000);

    bool newAvailables[8][8];
    availablePlaces(s, newAvailables, redTurn);
    bool finish = false;

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (!newAvailables[i][j])
                continue;

            int h;
            State newS = takeStep(s, i, j, redTurn);
            if (depth <= 1 || isEnd(newS)) {
                h = heuristic(newS);
            } else {
                int tempMX = -1, tempMY = -1;
                h = minimax(newS, tempMX, tempMY, !redTurn, depth - 1, firstPlayer, alpha, beta);
            }

            if (mX == -1 || (redTurn && h > bestHeu) || (!redTurn && h < bestHeu)) {
                mX = i;
                mY = j;
                bestHeu = h;
                if (redTurn && bestHeu > alpha)
                    alpha = bestHeu;
                else if (!redTurn && bestHeu < beta)
                    beta = bestHeu;


                if (alpha >= beta)
                    finish = true;
            }
            if (finish)
                break;
        }
        if (finish)
            break;
    }
    if (mX == -1) {
        int tempMX = -1, tempMY = -1;
        bestHeu = minimax(s, tempMX, tempMY, !redTurn, depth, firstPlayer, alpha, beta);
    }

    return bestHeu;
} 

int main() {
    // srand(time(NULL));
    // srand(2);
    init();

    printf ("Player Mode (first serve): 1\nPlayer Mode (second serve): 2\nBot vs Bot: 3\nCompetition: 4\nPlease choose the mode: ");
    fflush(NULL);
    int mode;
    cin >> mode;

    State s;

    bool redTurn = (mode != 2);

    bool available[8][8];
    int depth = 5;

    char c;


    if (mode <= 3) {
        s = initState;

        bool playerMode = (mode < 3);

        if (playerMode)
            printf ("Player: x (black)\n");

        while (!isEnd(s)) {
            printState(s, redTurn);

            if (availablePlaces(s, available, redTurn) != 0) {
                if (redTurn && playerMode)
                    playerMove(s, available);

                else {
                    int moveX, moveY;
                    minimax(s, moveX, moveY, redTurn, depth, true);
                    s = takeStep(s, moveX, moveY, redTurn);
                }
            }

            redTurn = !redTurn;
        }
        printf ("End State:\n");
        printState(s, redTurn);
        printf ("Winner: %s\n", (heuristic(s) > 0) ? "x (Black)" : (heuristic(s) < 0) ? "y (White)" : "break even");
    } else {
        while (true) {
            int result1, result2;
            s = initState;
            do {
                printf ("Give two competitors: ");
                cin >> player1 >> player2;
                printf ("\n");
            } while (player1 < 1 || player1 > 10 || player2 < 1 || player2 > 10);

            // player1 first
            while (!isEnd(s)) {
                printState(s, redTurn);

                if (availablePlaces(s, available, redTurn) != 0) {
                    int moveX, moveY;
                    minimax(s, moveX, moveY, redTurn, depth, redTurn);
                    s = takeStep(s, moveX, moveY, redTurn);
                }

                redTurn = !redTurn;
            }
            printf ("End State:\n");
            printState(s, redTurn);
            result1 = countResult(s);
            printf ("Winner: %s\n", (result1 > 0) ? "x (Black)" : (result1 < 0) ? "y (White)" : "break even");

            // player2 first
            s = initState;
            swap(player1, player2);

            while (!isEnd(s)) {
                printState(s, redTurn);

                if (availablePlaces(s, available, redTurn) != 0) {
                    int moveX, moveY;
                    minimax(s, moveX, moveY, redTurn, depth, redTurn);
                    s = takeStep(s, moveX, moveY, redTurn);
                }

                redTurn = !redTurn;
            }
            printf ("End State:\n");
            printState(s, redTurn);
            result2 = countResult(s);
            printf ("Winner: %s\n", (result2 > 0) ? "x (Black)" : (result2 < 0) ? "y (White)" : "break even");
        }

    }

}

