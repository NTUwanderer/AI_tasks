#include <stdlib.h>
#include <vector>
#include <math.h>
#include <string>
#include <Adafruit_ILI9341.h>

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

struct State {
    bool pos[8][8]; // false: x (red), true: o (blue)
    bool exist[8][8];
};

bool operator== (const State& s1, const State& s2) {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (s1.exist[i][j] != s2.exist[i][j])
                return false;
            if (s1.exist[i][j] && s1.pos[i][j] != s2.pos[i][j])
                return false;
        }
    }

    return true;
}

bool operator!= (const State& s1, const State& s2) {
    return !(s1 == s2);
}

char* myToChars(int i);
char* myToChars_six(int i);
bool inBoard(int x, int y);
int countResult(const State& s);
int availablePlaces(const State& s, bool (&available)[8][8], bool redTurn);
int heuristic(State& s);
bool isEnd(const State& s);
void drawHorizontalLine(int y);
void drawVerticalLine(int x);
void printNumber(const State& s, int i, int j);
void printState(State& s);
State takeStep(const State& s, int i, int j, bool redTurn);

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

bool inBoard(int x, int y) {
    return (x >= 0 && x < 8 && y >= 0 && y < 8);
}

int countResult(const State& s) {
    int result = 0;
    for (int i = 0; i < 8; ++i)
        for (int j = 0; j < 8; ++j)
            result += (!s.exist[i][j]) ? 0 : (!s.pos[i][j]) ? 1 : -1;

    return result;
}

int availablePlaces(const State& s, bool (&available)[8][8], bool redTurn) {

    bool currentC = redTurn ? false : true;
    bool opponentC = !currentC;

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            available[i][j] = false;
            if (s.exist[i][j]) {
                continue;
            }

            // direction (a, b)
            for (int a = -1; a <= 1; ++a) {
                for (int b = -1; b <= 1; ++b) {
                    if (a == 0 && b == 0)
                        continue;

                    bool findOpponent = false;
                    int nStep = 1;
                    while (true) {
                        int x = i + nStep * a, y = j + nStep * b;
                        if (!(inBoard(x, y)))
                            break;

                        if (!s.exist[x][y])
                            break;

                        if (s.pos[x][y] == opponentC)
                            findOpponent = true;

                        if (s.pos[x][y] == currentC) {
                            available[i][j] = findOpponent;
                            break;
                        }

                        ++nStep;
                    }

                    if (available[i][j])
                        break;
                }
                if (available[i][j])
                    break;
            }
        }
    }
    
    int count = 0;
    for (int i = 0; i < 8; ++i)
        for (int j = 0; j < 8; ++j)
            if (available[i][j])
                ++count;

    return count;

}

int heuristic(State& s) {

    int h = 0;

    bool available[8][8];
    int redMoves = availablePlaces(s, available, true);
    int blueMoves = availablePlaces(s, available, false);
    if (redMoves != 0 || blueMoves != 0)
        h = redMoves - blueMoves;
    else {
        h = countResult(s);
    }

    return h;
}

bool isEnd(const State& s) {

    bool available[8][8];
    int redMoves = availablePlaces(s, available, true);
    int blueMoves = availablePlaces(s, available, false);
    return (redMoves == 0 && blueMoves == 0);
}

void drawVerticalLine(int x) {
    for(int i=0;i<1;i++) {
        tft.drawLine(x+i,10,x+i,234,WHITE);
    }
}

void drawHorizontalLine(int y) {
    for(int i=0;i<1;i++) {
        tft.drawLine(90,y+i,314,y+i,WHITE);
    }
}

void printNumber(const State& s, int i, int j) {
    if (!s.exist[i][j])
        return;

    int x = 90 + 5 + i * 28;
    int y = 10 + 2 + j * 28;
    tft.setCursor(x, y);
    if (!s.pos[i][j]) {
        tft.setTextColor(RED);
        tft.print("x");
    }
    else {
        tft.setTextColor(BLUE);
        tft.print("o");
    }
    
}

void printState(State& s, bool redTurn) {

    tft.fillScreen(BLACK);
    tft.setTextSize(3);

    for (int i = 0; i < 9; ++i)
        drawVerticalLine(90 + i * 28);

    for (int j = 0; j < 9; ++j)
        drawHorizontalLine(10 + j * 28);

    for (int i = 0; i < 8; ++i)
        for (int j = 0; j < 8; ++j)
            printNumber(s, i, j);

    tft.setTextColor(CYAN);
    tft.setTextSize(2);
    tft.setCursor(10, 25);
    tft.print("h:");
    tft.print(myToChars(heuristic(s)));

    tft.setCursor(10, 65);
    tft.setTextSize(2);
    if (redTurn) {
        tft.setTextColor(RED);
        tft.print("RED\n Turn");
    } else {
        tft.setTextColor(BLUE);
        tft.print("BLUE\n Turn");
    }
}

State takeStep(const State& s, int i, int j, bool redTurn) {
    State newS = s;

    bool currentC = redTurn ? false : true;
    bool opponentC = !currentC;

    // direction (a, b)
    for (int a = -1; a <= 1; ++a) {
        for (int b = -1; b <= 1; ++b) {
            if (a == 0 && b == 0)
                continue;

            bool check = false;
            bool findOpponent = false;
            int nStep = 1;
            while (true) {
                int x = i + nStep * a, y = j + nStep * b;
                if (!(inBoard(x, y)))
                    break;

                if (!s.exist[x][y])
                    break;

                if (newS.pos[x][y] == opponentC)
                    findOpponent = true;

                if (newS.pos[x][y] == currentC) {
                    check = findOpponent;
                    break;
                }

                ++nStep;
            }

            if (check) {
                for (int k = 1; k < nStep; ++k) {
                    int x = i + k * a, y = j + k * b;
                    newS.pos[x][y] = currentC;
                }
            }
        }
    }
    newS.exist[i][j] = true;
    newS.pos[i][j] = currentC;
    heuristic(newS);

    return newS;
}

