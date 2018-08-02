#ifndef ASTAR_STATE
#define ASTAR_STATE

#include <stdlib.h>
#include <vector>
#include <math.h>
#include <string>
#include <Adafruit_ILI9341.h>

#include "step.h"

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF


extern Adafruit_ILI9341 tft;

using namespace std;

String num_str[9] = {" ", "1", "2", "3", "4", "5", "6", "7", "8"};

struct State {
    int pos[9]; // 1~8, 0 means empty
    int g;
    int h;
    int f;

} goalState;

bool operator< (const State& s1, const State& s2) {
    /* TODO
     * Depends on BFS/GS/A*, how to compare States in priority_queue?
     * Hint: priority_queue.top gives the largest State, how can we get the smallest?
     */
    return true;
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
        return (l > r);
    }
    bool operator() (const State& lhs, const State& rhs) const {
        return (lhs < rhs);
    }
};


unsigned long long mask = 15;

int findPos(State s, int n);
int heuristic(State s);
bool solvable(State s);
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
unsigned long long getKey(State s);
unsigned long long getHash(State s);
State getState(unsigned long long hash);
State getState(unsigned long long key, int f);
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

    // TODO: design your heuristic function

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

State randomStepState(int step) {
    State s = goalState;
    Step prevStep = getStep(0, 0);

    do {
        for (int i = 0; i < step; ++i) {
            vector<Step> steps;
            getAvailableSteps(s, steps);

            size_t j;
            do {
                j = rand() / (RAND_MAX / steps.size() + 1);
            } while (prevStep == steps[j]);
            s = takeStep(s, steps[j]);
            prevStep = steps[j];
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

void drawVerticalLine(int x) {
    for(int i=0;i<5;i++) {
        tft.drawLine(x+i,25,x+i,235,WHITE);
    }
}

void drawHorizontalLine(int y) {
    for(int i=0;i<5;i++) {
        tft.drawLine(105,y+i,320,y+i,WHITE);
    }
}

char* myToChars(int i) {
    char buffer [50];
    sprintf (buffer, "%3i", i);
    return buffer;
}

char* myToChars_six(int i) {
    char buffer [50];
    sprintf (buffer, "%6i", i);
    return buffer;
}

void printNumber(State s, int index) {
    int x = 105 + 20 + (index % 3) * 70;
    int y = 25 + 20 + (index / 3) * 70;
    tft.setCursor(x, y);
    tft.print(num_str[s.pos[index]]);
}

void printState(State s) {
    // for (int i = 0; i < 9; ++i) {
    //     printf ("%i ", s.pos[i]);
    //     if ((i + 1) % 3 == 0)
    //         printf ("\n");
    // }
    // printf ("g: %i, h: %i, f: %i, heu: %i\n", s.g, s.h, s.f, heuristic(s));
    tft.fillScreen(BLACK);

    //Draw frame
    // tft.drawRect(0,0,319,240,WHITE);

    drawVerticalLine(105);
    drawVerticalLine(175);
    drawVerticalLine(245);
    drawVerticalLine(315);

    drawHorizontalLine(25);
    drawHorizontalLine(95);
    drawHorizontalLine(165);
    drawHorizontalLine(235);

    tft.setTextColor(BLUE);
    tft.setTextSize(6);
    for (int i = 0; i < 9; ++i) {
        printNumber(s, i);
    }

    tft.setTextColor(CYAN);
    tft.setTextSize(3);
    tft.setCursor(10, 25);
    tft.print("g:");
    tft.print(myToChars(s.g));
    tft.setCursor(10, 65);
    tft.print("h:");
    tft.print(myToChars(s.h));
    tft.setCursor(10, 105);
    tft.print("f:");
    tft.print(myToChars(s.f));
}


unsigned long long getKey(State s) {
    unsigned long long key = 0;
    for (int i = 0; i < 9; ++i) {
        unsigned long long n = s.pos[i];
        n <<= i * 4;
        key ^= n;
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

State getState(unsigned long long key, int f) {
    State s;
    for (int i = 0; i < 9; ++i) {
        s.pos[i] = key & mask;
        key >>= 4;
    }

    s.f = f;
    s.h = heuristic(s);
    s.g = s.f - s.h;

    return s;
}

State takeStep(State s, Step step, bool reverse) {
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

bool getLegalStep(State s, int index, Step& step) {
    int emptyIndex = findPos(s, 0);
    int delta = abs(index - emptyIndex);
    if (delta == 1 || delta == 3) {
        step = getStep(index, emptyIndex);
        return true;
    }
    return false;
}

#endif

