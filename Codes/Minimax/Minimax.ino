#include <Adafruit_GFX.h>        // Core graphics library
#include <SPI.h>
#include <Wire.h>            // this is needed even tho we aren't using it
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>
#include <queue>
#include <limits.h>
#include <unordered_map>
#include <vector>
#include <algorithm>

#include "state.h"
#include "buttonCoordinate.h"

#define TOUCH_CS_PIN D3
#define TOUCH_IRQ_PIN D2 

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 386
#define TS_MINY 178
#define TS_MAXX 3949
#define TS_MAXY 3906

XPT2046_Touchscreen ts(TOUCH_CS_PIN,TOUCH_IRQ_PIN);

// The display also uses hardware SPI, plus #9 & #10
static uint8_t SD3 = 10;
#define TFT_CS SD3
#define TFT_DC D4
#define BL_LED D8

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

extern uint8_t circle[];
extern uint8_t x_bitmap[];

typedef unordered_map<unsigned long long, int>  HeuMap;

enum GameState{startMode, playerMode, MinimaxMode, endMode};
GameState gameState;

ButtonCoordinate playerButton, giveupButton, MinimaxButton;
ButtonCoordinate buttons[8][8];

State initState, currentState;
int countMinimax;
bool availables[8][8];
bool redTurn;
const int depth = 4;

int minimax(State& s, const bool (&available)[8][8], bool redTurn, int depth, int alpha = INT_MIN, int beta = INT_MAX);

void setup() {
    pinMode(BL_LED,OUTPUT);
    digitalWrite(BL_LED, HIGH);
    Serial.begin(9600);
    Serial.print("Starting...");
    randomSeed(millis());
 
    if (!ts.begin()) {
        Serial.println("Couldn't start touchscreen controller");
        while (1);
    }

    gameSetup();
    initDisplay();
    drawStartScreen();
}

void loop() {
    boolean istouched = ts.touched();
    if (istouched || gameState == MinimaxMode) {
        istouched = false;
        TS_Point p0 = ts.getPoint(),p;  //Get touch point

        p.x = map(p0.x, TS_MINX, TS_MAXX, 0, 320);
        p.y = map(p0.y, TS_MINY, TS_MAXY, 0, 240);

        Serial.print("X = "); Serial.print(p.x);
        Serial.print("\tY = "); Serial.print(p.y);
        Serial.print("\n");
                
        if (gameState == startMode) {
            resetGame();
            printState(currentState, redTurn);
            if (playerButton.pressed(p.x, p.y)) {
                gameState = playerMode;
                drawGiveupButton();
            } else if (MinimaxButton.pressed(p.x, p.y)) {
                gameState = MinimaxMode;
            }
        } else if (gameState == playerMode) {
            if (giveupButton.pressed(p.x, p.y)) {
                gameState = endMode;
                drawGameOverScreen(INT_MIN);

            } else {
                int clickedI = -1, clickedJ = -1;
                for (int i = 0; i < 8; ++i) {
                    for (int j = 0; j < 8; ++j) {
                        if (buttons[i][j].pressed(p.x, p.y)) {
                            clickedI = i;
                            clickedJ = j;
                            tft.fillRect(buttons[i][j].x, buttons[i][j].y, buttons[i][j].width, buttons[i][j].height, GREEN);
                            tft.setTextSize(3);
                            printNumber(currentState, i, j);
                            break;
                        }
                    }
                }
                delay(200);
                availablePlaces(currentState, availables, redTurn);
                if (!(clickedI == -1 || clickedJ == -1) && availables[clickedI][clickedJ]) {
                    currentState = takeStep(currentState, clickedI, clickedJ, redTurn);
                    redTurn = !redTurn;

                    if (availablePlaces(currentState, availables, redTurn) == 0) {
                        redTurn = !redTurn;
                    }
                    printState(currentState, redTurn);
                } else {
                    printState(currentState, redTurn);
                    tft.setTextColor(RED);
                    tft.setTextSize(2);
                    tft.setCursor(10, 150);
                    tft.print("Error");
                }

                if (availablePlaces(currentState, availables, redTurn) == 0 && availablePlaces(currentState, availables, !redTurn) == 0) {
                    delay(1000);
                    gameState = endMode;
                    drawGameOverScreen(countResult(currentState));
                } else {
                    drawGiveupButton();
                }

            }
        } else if (gameState == MinimaxMode) {
            delay(300);
            availablePlaces(currentState, availables, redTurn);
            minimax(currentState, availables, redTurn, depth);
            redTurn = !redTurn;

            if (availablePlaces(currentState, availables, redTurn) == 0) {
                redTurn = !redTurn;
            }
            printState(currentState, redTurn);

            if (availablePlaces(currentState, availables, redTurn) == 0 && availablePlaces(currentState, availables, !redTurn) == 0) {
                delay(700);
                gameState = endMode;
                drawGameOverScreen(countResult(currentState));
            }

        } else if (gameState == endMode) {
            gameState = startMode;
            drawStartScreen();
        } else {
            Serial.println("Unknown GameState:");
            Serial.println(gameState);
        }
/*
            } else if (MinimaxButton.pressed(p.x, p.y)) {
                gameState = MinimaxMode;
                resetGame();
                printState(currentState);

                tft.setTextColor(RED);
                tft.setTextSize(2);
                tft.setCursor(10, 165);
                tft.print("Solving...");

                int cutoff = INT_MAX;

                tft.setTextColor(BLUE);
                tft.setCursor(10, 205);
                tft.print("Solved.");
            }
        } else if (gameState == playerMode) {
            if (giveupButton.pressed(p.x, p.y)) {
                gameState = endMode;
                drawGameOverScreen(-1);

            } else {
                int clickedIndex = -1;
                for (int i = 0; i < 9; ++i) {
                    if (buttons[i].pressed(p.x, p.y)) {
                        clickedIndex = i;
                        tft.fillRect(buttons[i].x, buttons[i].y, buttons[i].width, buttons[i].height, RED);
                        tft.setTextColor(BLUE);
                        tft.setTextSize(6);
                        printNumber(currentState, i);
                        break;
                    }
                }
                delay(200);
                Step step;
                if (getLegalStep(currentState, clickedIndex, step)) {
                    currentState = takeStep(currentState, step);
                    printState(currentState);
                } else {
                    printState(currentState);
                    
                    tft.setTextColor(RED);
                    tft.setTextSize(3);
                    tft.setCursor(10, 150);
                    tft.print("Error");
                }

                if (currentState == goalState) {
                    delay(1000);
                    gameState = endMode;
                    drawGameOverScreen(currentState.g);
                } else {
                    drawGiveupButton();
                }
            }
        } else if (gameState == MinimaxMode) {
            for (int i = 0; i < steps.size(); ++i) {
                printState(currentState);
                delay(500);

                int p1 = steps[i].p1;
                int p2 = steps[i].p2;
                tft.fillRect(buttons[p1].x, buttons[p1].y, buttons[p1].width, buttons[p1].height, RED);
                tft.fillRect(buttons[p2].x, buttons[p2].y, buttons[p2].width, buttons[p2].height, RED);
                tft.setTextColor(BLUE);
                tft.setTextSize(6);
                printNumber(currentState, p1);
                printNumber(currentState, p2);

                currentState = takeStep(currentState, steps[i]);
                delay(500);
            }
            if (currentState == goalState) {
                printState(currentState);
                delay(500);
                drawMinimaxGameOverScreen(currentState.g);
            }
            else
                drawMinimaxGameOverScreen(INT_MAX);

            gameState = endMode;

        } else if (gameState == endMode) {
            gameState = startMode;
            drawStartScreen();
        } else {
            Serial.println("Unknown GameState:");
            Serial.println(gameState);
        }
*/

        delay(10);  
    }
}

void drawGiveupButton() {
    giveupButton.fillAndDraw(tft, RED, WHITE);
    tft.setCursor(giveupButton.x + 5, giveupButton.y + 5);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.print("Giveup");
}

void gameSetup() {
    gameState = startMode;
    playerButton = ButtonCoordinate(10,180,140,40);
    giveupButton = ButtonCoordinate(5,200,80,30);
    MinimaxButton  = ButtonCoordinate(170,180,140,40);
    for (int i = 0; i < 8; ++i)
        for (int j = 0; j < 8; ++j)
            buttons[i][j] = ButtonCoordinate(90 + 1 + i * 28, 10 + 1 + j * 28, 27, 27);

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
    
    // for (int i = 1; i < 8; ++i)
    //     for (int j = 1; j < 8; ++j)
    //         initState.pos[i][j] = (i+j) % 2;
    // initState.h = heuristic(initState);
}

void resetGame() {
    currentState = initState;

    countMinimax = 0;
    redTurn = true;
    availablePlaces(currentState, availables, redTurn);
}

void drawStartScreen()
{
    tft.fillScreen(BLACK);

    //Draw white frame
    tft.drawRect(0,0,319,240,WHITE);
    
    //Print "8 Puzzle" Text
    tft.setCursor(30,100);
    tft.setTextColor(WHITE);
    tft.setTextSize(4);
    tft.print("Reversi");
    
    //Print "Arduino" Text
    tft.setCursor(80,30);
    tft.setTextColor(GREEN);
    tft.setTextSize(4);
    tft.print("Arduino");

    createStartButton();

}

void createStartButton()
{
    //Create Red Button
    //tft.fillRect(30,180,120,40,RED);
    //tft.drawRect(30,180,120,40,WHITE);
    playerButton.fillAndDraw(tft, RED, WHITE);
    tft.setCursor(36,188);
    tft.setTextColor(WHITE);
    tft.setTextSize(3);
    tft.print("Player");

    //tft.fillRect(170,180,120,40,RED);
    //tft.drawRect(170,180,120,40,WHITE);
    MinimaxButton.fillAndDraw(tft, RED, WHITE);
    tft.setCursor(176,188);
    tft.setTextColor(WHITE);
    tft.setTextSize(3);
    tft.print("Minimax");
}

void initDisplay()
{
//  tft.reset();
    tft.begin();
    tft.setRotation(3);
}

void drawGameOverScreen(int result) {
     
    tft.fillScreen(BLACK);

    //Draw frame
    tft.drawRect(0,0,319,240,WHITE);

    if (result != INT_MIN) {
        tft.setTextColor(RED);
        tft.setTextSize(3);
        if (result > 0) {
            tft.setCursor(100,30);
            tft.print("Red Win!");
        } else if (result == 0) {
            tft.setCursor(105,30);
            tft.print("Tie Up!");
        } else {
            tft.setCursor(95,30);
            tft.print("Blue Win!");
        }

        tft.setCursor(10,100);
        tft.setTextColor(WHITE);
        tft.setTextSize(3);
        tft.print("Result: ");
        tft.print(myToChars(result));
    } else {
        tft.setCursor(40,100);
        tft.setTextColor(RED);
        tft.setTextSize(3);
        tft.print("You Giveup QQ");

    }
    
    tft.setCursor(50,220);
    tft.setTextColor(BLUE);
    tft.setTextSize(2);
    tft.print("Click to Continue...");
}

void drawMinimaxGameOverScreen(int moves) {
     
    tft.fillScreen(BLACK);

    //Draw frame
    tft.drawRect(0,0,319,240,WHITE);

    tft.setCursor(100,30);
    tft.setTextColor(RED);
    tft.setTextSize(3);
    tft.print("You Won!");

    tft.setCursor(10,100);
    tft.setTextColor(WHITE);
    tft.setTextSize(3);
    tft.print("Move :   ");
    tft.print(myToChars(moves));

    tft.setCursor(10,140);
    tft.setTextColor(WHITE);
    tft.setTextSize(3);
    tft.print("Model:");
    tft.print(myToChars_six(countMinimax));
    
    tft.setCursor(50,220);
    tft.setTextColor(BLUE);
    tft.setTextSize(2);
    tft.print("Click to Continue...");
}

// return best heuristic value with a limited trace depth
int minimax(State& s, const bool (&available)[8][8], bool redTurn, int depth, int alpha, int beta) {

    int bestX = -1, bestY = -1;
    int bestHeu = 0;

    bool newAvailable[8][8];
    bool finish = false;

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (!available[i][j])
                continue;

            int h;
            State newS = takeStep(s, i, j, redTurn);
            if (depth <= 1 || isEnd(newS)) {
                h = heuristic(newS);
            } else {
                availablePlaces(newS, newAvailable, !redTurn);
                h = minimax(newS, newAvailable, !redTurn, depth - 1, alpha, beta);
            }

            if (bestX == -1 || (redTurn && h > bestHeu) || (!redTurn && h < bestHeu)) {
                bestX   = i;
                bestY   = j;
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
    if (bestX == -1) {
        State newS = s;
        availablePlaces(newS, newAvailable, !redTurn);
        bestHeu = minimax(newS, newAvailable, !redTurn, depth - 1);
    } else {
        s = takeStep(s, bestX, bestY, redTurn);
    }

    return bestHeu;
} 
