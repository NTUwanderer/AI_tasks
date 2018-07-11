#include <Adafruit_GFX.h>    // Core graphics library
#include <SPI.h>
#include <Wire.h>      // this is needed even tho we aren't using it
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>
#include <string.h>
#include "buttonCoordinate.h"
#define TOUCH_CS_PIN D3
#define TOUCH_IRQ_PIN D2 

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 309
#define TS_MINY 184
#define TS_MAXX 3658
#define TS_MAXY 3928

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

ButtonCoordinate startButton, giveupButton, reverseButton;
ButtonCoordinate numButtons[10];

enum GameState{startMode, gameMode, reverseMode, endMode};
GameState gameState;

struct GameData {
    char R[5];
    char guess[5];
    bool histR[10];
    bool histG[10];
    int A;
    int B;
    int count;
} gameData;

struct RevData {
    bool possible[10000];
    int guess;
    int A;
    int B;
    int count;
    int remain;
} revData;

void resetGuess() {
    strcpy(gameData.guess, "xxxx");
    for(int i = 0; i < 10; ++i) {
        gameData.histG[i] = 0;
    }
    gameData.A = -1;
    gameData.B = -1;
}

void resetGameData() {
    resetGuess();
    strcpy(gameData.R, "    ");
    for(int i = 0; i < 10; ++i) {
        gameData.histR[i] = 0;
    }
    gameData.count = 0;
}

void resetRevGuess() {
    revData.A = -1;
    revData.B = -1;
    revData.guess = 0;
}

void resetRevData() {
    Serial.print("resetRevData()");
    revData.remain = 10000;
    // remove all invalid number
    for(int i = 0; i < 10000; ++i) {
        revData.possible[i] = 1;
        bool hist[10] = {0};
        int target = i;
        for(int j = 0; j < 4; ++j) {
            int digit = target % 10;
            if(hist[digit]) {
                revData.possible[i] = 0;
                --revData.remain;
                break;
            }
            hist[digit] = 1;
            target /= 10;
        }
    }
    Serial.print("remain: ");
    Serial.print(revData.remain);
    resetRevGuess();
    revData.count = 0;
}

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

    menuSetup();
    initDisplay();
    drawStartScreen();
}

void menuSetup() {
    gameState = startMode;
    startButton = ButtonCoordinate(30,180,100,40);
    reverseButton  = ButtonCoordinate(150,180,140,40);
}

void gameSetup() {
    Serial.print("gameSetup\n");
    giveupButton = ButtonCoordinate(192,0,128,56);
    for (int i = 0; i < 10; ++i) {
        numButtons[i] = ButtonCoordinate((i % 5) * 64, 112 + (i / 5) * 64, 64, 64);
    }
    resetGameData();
    generate();
    gameState = gameMode;
    Serial.print("finish gameSetup\n");
}

void initDisplay() {
    tft.begin();
    tft.setRotation(3);
}

void drawStartScreen() {
    tft.fillScreen(BLACK);

    //Draw white frame
    tft.drawRect(0,0,319,240,WHITE);
    
    tft.setCursor(20,100);
    tft.setTextColor(WHITE);
    tft.setTextSize(4);
    tft.print("Guess Number");
    
    tft.setCursor(80,30);
    tft.setTextColor(GREEN);
    tft.setTextSize(4);
    tft.print("Arduino");

    createStartButton();
}

void createStartButton() {
    startButton.fillAndDraw(tft, RED, WHITE);
    tft.setCursor(36,188);
    tft.setTextColor(WHITE);
    tft.setTextSize(3);
    tft.print("Start");
    
    reverseButton.fillAndDraw(tft, RED, WHITE);
    tft.setCursor(156,188);
    tft.setTextColor(WHITE);
    tft.setTextSize(3);
    tft.print("Reverse");
}

void generate() {
    for(int i = 0; i < 4; ++i) {
        gameData.R[i] = ' ';
    }
    int i = 0;
    int r;
    randomSeed(millis());
    while (i < 4) {
        r = random(0, 10);
        if (!gameData.histR[r]) {
            gameData.R[i] = r + '0';
            gameData.histR[r] = 1;
            ++i;
        }
    }
    Serial.print("generate R: ");
    Serial.print(gameData.R);
    Serial.print("\n");
}

void loop() {
    bool isTouched = ts.touched();
    if(isTouched) {
        isTouched = false;
        TS_Point p0 = ts.getPoint();  //Get touch point
        TS_Point p;
        p.x = map(p0.x, TS_MINX, TS_MAXX, 0, 320);
        p.y = map(p0.y, TS_MINY, TS_MAXY, 0, 240);

        Serial.print("X = "); Serial.print(p.x);
        Serial.print("\tY = "); Serial.print(p.y);
        Serial.print("\n");

        if(startButton.pressed(p.x, p.y)) {
            gameSetup();
            execGame();
        }
        else if(reverseButton.pressed(p.x, p.y)) {
            revGameSetup();
            execRevGame();
        }
    }
    if(gameState == endMode) {
        drawStartScreen();
        gameState = startMode;
    }
}

void execGame() {
    Serial.print("execGame");
    while(gameState == gameMode) {
        drawGameScreen();
        playerMove();
        checkGuess();
    }
}

void drawVerticalLine(int x) {
    for(int i=0;i<5;i++) {
        tft.drawLine(x+i,112,x+i,240,WHITE);
    }
}

void drawHorizontalLine(int y) {
    for(int i=0;i<5;i++) {
        tft.drawLine(0,y+i,320,y+i,WHITE);
    }
}

void printNumber(int index) {
    // char* num_str[10] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
    int offset = 16;
    // int x = offset + (index % 5) * 64;
    // int y = offset + 112 + (index / 5) * 64;
    int x = offset + numButtons[index].x;
    int y = offset + numButtons[index].y;
    tft.setCursor(x, y);
    tft.print(index);
}

void drawGameScreen() {
    tft.fillScreen(BLACK);
    tft.drawRect(0,0,319,240,WHITE);
    for(int i = 0; i < 4; ++i) {
        drawVerticalLine(64 * (i+1) - 2);
    }
    for(int i = 0; i < 4; ++i) {
        drawHorizontalLine(112 + 64 * i - 2);
    }
    for(int i = 0; i < 10; ++i) {
        if(!gameData.histG[i]) printNumber(i);
    }
    tft.setCursor(192 + 8, 0 + 8);
    tft.print("giveup");
    // draw guess
    tft.setCursor(8, 8);
    tft.print(gameData.guess);
    // draw result
    if(gameData.A == 4) {
        tft.setCursor(8, 74 + 8);
        tft.print("Bingo!");
    }
    else {
        tft.setCursor(8, 37 + 8);
        tft.print("A: ");
        if(gameData.A >= 0) tft.print(gameData.A);
        tft.setCursor(8, 74 + 8);
        tft.print("B: ");
        if(gameData.B >= 0) tft.print(gameData.B);
    }
    tft.setCursor(128 + 8, 74 + 8);
    tft.print("count: ");
    tft.print(gameData.count);
}

void playerMove() {
    bool isTouched = false;
    int count = 0;
    while(count < 4) {
        isTouched = ts.touched();
        if(isTouched) {
            isTouched = false;
            if(count == 0) resetGuess();
            TS_Point p0 = ts.getPoint();  //Get touch point
            TS_Point p;
            p.x = map(p0.x, TS_MINX, TS_MAXX, 0, 320);
            p.y = map(p0.y, TS_MINY, TS_MAXY, 0, 240);
            Serial.print("X = "); Serial.print(p.x);
            Serial.print("\tY = "); Serial.print(p.y);
            Serial.print("\n");
            if(giveupButton.pressed(p.x, p.y)) {
                gameState = endMode;
                break;
            }
            for(int i = 0; i < 10; ++i) {
                if(numButtons[i].pressed(p.x, p.y)) {
                    if(!gameData.histG[i]) {
                        gameData.guess[count] = '0' + i;
                        gameData.histG[i] = 1;
                        ++count;
                        drawGameScreen();
                    }
                    Serial.print("count = ");
                    Serial.print(count);
                    Serial.print("\tguess = ");
                    Serial.print(gameData.guess);
                    Serial.print("\n");
                }
            }
        }
        delay(50); // prevent wdt reset and continuous input
    }
    ++gameData.count;
}

void checkGuess() {
    Serial.print("checkGuess\n");
    gameData.A = 0;
    gameData.B = 0;
    for(int i = 0; i < 4; ++i) {
        if(gameData.R[i] == gameData.guess[i]) ++gameData.A;
    }
    for(int i = 0; i < 10; ++i) {
        if(gameData.histR[i] && gameData.histG[i]) ++gameData.B;
    }
    gameData.B -= gameData.A;

    if(gameData.A == 4) {
        gameState = endMode;
        drawGameScreen();
        delay(2000);
    }
    for(int i = 0; i < 10; ++i) {
        gameData.histG[i] = 0;
    }
}

// Reverse mode: AI guess your number

void revGameSetup() {
    giveupButton = ButtonCoordinate(192,0,128,56);
    for (int i = 0; i < 10; ++i) {
        numButtons[i] = ButtonCoordinate((i % 5) * 64, 112 + (i / 5) * 64, 64, 64);
    }
    resetRevData();
    randomSeed(millis());
    gameState = reverseMode;
}

void execRevGame() {
    Serial.print("execRevGame");
    while(gameState == reverseMode) {
        AIMove();
        drawRevScreen();
        revPlayerMove();
        AIFilter();
    }
}

void drawRevScreen() {
    tft.fillScreen(BLACK);
    tft.drawRect(0,0,319,240,WHITE);
    for(int i = 0; i < 4; ++i) {
        drawVerticalLine(64 * (i+1) - 2);
    }
    for(int i = 0; i < 4; ++i) {
        drawHorizontalLine(112 + 64 * i - 2);
    }
    for(int i = 0; i < 5; ++i) {
        printNumber(i);
    }
    tft.setCursor(192 + 8, 0 + 8);
    tft.print("back");
    // draw guess
    tft.setCursor(8, 8);
    if(revData.remain > 0) {
        if(revData.guess < 1000) tft.print("0");
        tft.print(revData.guess);
    }
    else {
        tft.print("Impossible!");
    }
    // draw result
    if(revData.A == 4) {
        tft.setCursor(8, 37 + 8);
        tft.print("Game Over!");
    }
    else {
        tft.setCursor(8, 37 + 8);
        tft.print("A: ");
        if(revData.A >= 0) tft.print(revData.A);
        tft.setCursor(8, 74 + 8);
        tft.print("B: ");
        if(revData.B >= 0) tft.print(revData.B);
    }
    tft.setCursor(128 + 8, 74 + 8);
    tft.print("count: ");
    tft.print(revData.count);
}

void AIMove() {
    // AI guess a number
    resetRevGuess();
    int index = random(0, 32767) % revData.remain;
    int counter = 0;
    for(int i = 0; i < 10000; ++i) {
        if(revData.possible[i]) ++counter;
        if(counter > index) {
            revData.guess = i;
            break;
        }
    }
    ++revData.count;
}

void revPlayerMove() {
    // player enter result
    bool isTouched = false;
    int count = 0;
    while(count < 2) {
        isTouched = ts.touched();
        if(isTouched) {
            isTouched = false;
            TS_Point p0 = ts.getPoint();  //Get touch point
            TS_Point p;
            p.x = map(p0.x, TS_MINX, TS_MAXX, 0, 320);
            p.y = map(p0.y, TS_MINY, TS_MAXY, 0, 240);
            Serial.print("X = "); Serial.print(p.x);
            Serial.print("\tY = "); Serial.print(p.y);
            Serial.print("\n");
            if(giveupButton.pressed(p.x, p.y)) {
                gameState = endMode;
                break;
            }
            for(int i = 0; i < 5; ++i) {
                if(numButtons[i].pressed(p.x, p.y)) {
                    if(count == 0) revData.A = i;
                    else if(count == 1) revData.B = i;
                    ++count;
                    drawRevScreen();
                    Serial.print("count = ");
                    Serial.print(count);
                    Serial.print("\tguess = ");
                    Serial.print(gameData.guess);
                    Serial.print("\n");
                }
            }
            if(revData.A == 4) {
                gameState = endMode;
                delay(2000);
                break;
            }
        }
        delay(50); // prevent wdt reset and continuous input
    }
    ++gameData.count;
}

void AIFilter() {
    Serial.print("filter\n");
    // AI filters out possibilities by player result
    if(gameState != reverseMode) return;
    bool histG[10] = {0};
    int guess[4] = {0};
    int temp = revData.guess;
    for(int i = 0; i < 4; ++i) {
        guess[i] = temp % 10;
        histG[temp % 10] = 1;
        temp /= 10;
    }
    for(int i = 0; i < 10000; ++i) {
        if(revData.possible[i]) {
            bool histR[10] = {0};
            int target[4] = {0};
            int temp = i;
            int A = 0;
            int B = 0;
            for(int j = 0; j < 4; ++j) {
                target[j] = temp % 10;
                histR[temp%10] = 1;
                temp /= 10;
            }
            for(int j = 0; j < 4; ++j) {
                if(guess[j] == target[j]) ++A;
            }
            for(int j = 0; j < 10; ++j) {
                if(histG[j] && histR[j]) ++B;
            }
            B -= A;
            if((A != revData.A) || (B != revData.B)) {
                revData.possible[i] = 0;
                --revData.remain;
            }
        }
    }
    if(revData.remain == 0) {
        drawRevScreen();
        gameState = endMode;
        delay(2000);
    }
}

