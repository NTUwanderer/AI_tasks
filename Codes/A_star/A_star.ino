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

#include "util.h"
#include "state.h"
#include "myQueue.h"
#include "zkey.h"
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

enum GameState{startMode, playerMode, aStarMode, endMode};
GameState gameState = startMode;

int gameScreen = 1; // deprecated
int moves = 1;
State initState, currentState;

int winner = 0;  //0 = Draw, 1 = Human, 2 = CPU, deprecated

boolean buttonEnabled = true; // deprecated

int board[]={0,0,0,0,0,0,0,0,0};// holds position data 0 is blank, 1 human, 2 is computer, deprecated

buttonCoordinate playerButton, AStarButton;
buttonCoordinate buttons[9];

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
    if (istouched) {
        istouched = false;
        TS_Point p0 = ts.getPoint(),p;  //Get touch point

        p.x = map(p0.x, TS_MINX, TS_MAXX, 0, 320);
        p.y = map(p0.y, TS_MINY, TS_MAXY, 0, 240);

        Serial.print("X = "); Serial.print(p.x);
        Serial.print("\tY = "); Serial.print(p.y);
        Serial.print("\n");
                
        if (gameState == startMode) {
            if (playerButton.pressed(p.x, p.y)) {
                gameState = playerMode;
                resetGame();    
                printState(currentState);
            }
        } else if (gameState == playerMode) {
            int clickedIndex = -1;
            for (int i = 0; i < 9; ++i) {
                if (buttons[i].pressed(p.x, p.y)) {
                    clickedIndex = i;
                    tft.fillRect(buttons[i].x, buttons[i].y, buttons[i].width, buttons[i].height, RED);
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
                tft.setCursor(10, 165);
                tft.print("Error");
            }

            if (currentState == goalState) {
                delay(1000);
                gameState = endMode;
                drawGameOverScreen(currentState.g);
            }
        } else if (gameState == endMode) {
            gameState = startMode;
            drawStartScreen();
        }

    
        /*
        if(p.x>60 && p.x<260 && p.y>180 && p.y<220 && buttonEnabled)// The user has pressed inside the red rectangle
        {
            buttonEnabled = false; //Disable button
            Serial.println("Button Pressed");
            resetGame();    
            drawGameScreen();
            playGame();
        } 
        */
        delay(10);  
    }
}

void gameSetup() {
    gameState = startMode;
    playerButton = buttonCoordinate(30,180,120,40);
    AStarButton  = buttonCoordinate(170,180,120,40);
    for (int i = 0; i < 9; ++i) {
        buttons[i] = buttonCoordinate(110 + (i % 3) * 70, 30 + (i / 3) * 70, 65, 65);
    }

    for (int i = 1; i <= 8; ++i) {
        goalState.pos[i-1] = i;
    }
    goalState.pos[8] = 0;
    goalState.h = 0;
    goalState.g = 0;
    goalState.f = 0;


}

void resetGame() {
    moves = 0;
    do {
        initState = randomState();
    } while (initState == goalState);
    currentState = initState;
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
    tft.print("8 Puzzle");
    
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
    AStarButton.fillAndDraw(tft, RED, WHITE);
    tft.setCursor(176,188);
    tft.setTextColor(WHITE);
    tft.setTextSize(3);
    tft.print("A star");
}

void initDisplay()
{
//  tft.reset();
    tft.begin();
    tft.setRotation(3);
}

void drawGameScreen()
{
     tft.fillScreen(BLACK);

     //Draw frame
     tft.drawRect(0,0,319,240,WHITE);

     drawVerticalLine(125);

     drawVerticalLine(195);

     drawHorizontalLine(80);

     drawHorizontalLine(150);
}

void drawGameOverScreen(int moves) {
     
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
    tft.print("You Spent ");
    tft.print(myToChars(moves));
    tft.print("\nMoves.");
    
    tft.setCursor(50,220);
    tft.setTextColor(BLUE);
    tft.setTextSize(2);
    tft.print("Click to Continue...");
}

void createPlayAgainButton()
{
     //Create Red Button
    tft.fillRect(60,180, 200, 40, RED);
    tft.drawRect(60,180,200,40,WHITE);
    tft.setCursor(72,188);
    tft.setTextColor(WHITE);
    tft.setTextSize(3);
    tft.print("Play Again");
}

void printBoard()
{
    int i=0;
    Serial.println("Board: [");
    for(i=0;i<9;i++)
    {
        Serial.print(board[i]);
        Serial.print(",");
    }
    Serial.print("]");
}

int checkOpponent()
{
    if(board[0]==1 && board[1]==1 && board[2]==0)
    return 2;
    else if(board[0]==1 && board[1]==0 && board[2]==1)
    return 1;
    else if (board[1]==1 && board [2]==1 && board[0]==0)
    return 0;
    else if (board[3]==1 && board[4]==1 && board[5]==0)
    return 5;
    else if (board[4]==1 && board[5]==1&& board[3]==0)
    return 3;
    else if (board[3]==1 && board[4]==0&& board[5]==1)
    return 4;
    else if (board[1]==0 && board[4]==1&& board[7]==1)
    return 1;
    else
    return 100;
}

void arduinoMove()
{
    int b = 0;
    int counter =0;
    int movesPlayed = 0;
    Serial.print("\nArduino Move:");

    int firstMoves[]={0,2,6,8}; // will use these positions first

    for(counter=0;counter<4;counter++) //Count first moves played
    {
        if(board[firstMoves[counter]]!=0) // First move is played by someone
        {
            movesPlayed++;
        }
    }  
    do{
        if(moves<=2)
        {
            int randomMove =random(4); 
            int c=firstMoves[randomMove];
            
            if (board[c]==0)
            {  
                delay(1000);
                board[c]=2;
                Serial.print(firstMoves[randomMove]);
                Serial.println();
                drawCpuMove(firstMoves[randomMove]);
                b=1;
            }       
                
        }else
        {
        int nextMove = checkOpponent();
        if(nextMove == 100)
        {  
        if(movesPlayed == 4) //All first moves are played
        {
            int randomMove =random(9); 
            if (board[randomMove]==0)
            {  
                delay(1000);
                board[randomMove]=2;
                Serial.print(randomMove);
                Serial.println();
                drawCpuMove(randomMove);
                b=1;
            }       
        }else
        {
            int randomMove =random(4); 
            int c=firstMoves[randomMove];
            
            if (board[c]==0)
            {  
                delay(1000);
                board[c]=2;
                Serial.print(firstMoves[randomMove]);
                Serial.println();
                drawCpuMove(firstMoves[randomMove]);
                b=1;
            }       
    }
        }else
        {
             delay(1000);
             board[nextMove]=2;
             drawCpuMove(nextMove);
             b=1;
        }
    }
    }
    while (b<1);
}

void drawCircle(int x, int y)
{
    drawBitmap(x,y,circle,65,65,RED);
}

void drawX(int x, int y)
{
    drawBitmap(x,y,x_bitmap,65,65,BLUE);
}

void drawCpuMove(int move)
{
    switch(move)
    {
        case 0: drawCircle(55,15);  break;
        case 1: drawCircle(130,15); break;
        case 2: drawCircle(205,15); break;
        case 3: drawCircle(55,85);  break;
        case 4: drawCircle(130,85); break;
        case 5: drawCircle(205,85); break;
        case 6: drawCircle(55,155); break;
        case 7: drawCircle(130,155);break;
        case 8: drawCircle(205,155);break;
    }
}

void drawPlayerMove(int move)
{
    switch(move)
    {
        case 0: drawX(55,15);  break;
        case 1: drawX(130,15); break;
        case 2: drawX(205,15); break;
        case 3: drawX(55,85);  break;
        case 4: drawX(130,85); break;
        case 5: drawX(205,85); break;
        case 6: drawX(55,155); break;
        case 7: drawX(130,155);break;
        case 8: drawX(205,155);break;
    }
}

void checkWinner() 
// checks board to see if there is a winner
// places result in the global variable 'winner'
{
    int qq=0;
    // noughts win?
    if (board[0]==1 && board[1]==1 && board[2]==1) { 
        winner=1; 
    }
    if (board[3]==1 && board[4]==1 && board[5]==1) { 
        winner=1; 
    }
    if (board[6]==1 && board[7]==1 && board[8]==1) { 
        winner=1; 
    }  
    if (board[0]==1 && board[3]==1 && board[6]==1) { 
        winner=1; 
    }
    if (board[1]==1 && board[4]==1 && board[7]==1) { 
        winner=1; 
    }
    if (board[2]==1 && board[5]==1 && board[8]==1) { 
        winner=1; 
    }  
    if (board[0]==1 && board[4]==1 && board[8]==1) { 
        winner=1; 
    }
    if (board[2]==1 && board[4]==1 && board[6]==1) { 
        winner=1; 
    }
    // crosses win?
    if (board[0]==2 && board[1]==2 && board[2]==2) { 
        winner=2; 
    }
    if (board[3]==2 && board[4]==2 && board[5]==2) { 
        winner=2; 
    }
    if (board[6]==2 && board[7]==2 && board[8]==2) { 
        winner=2; 
    }  
    if (board[0]==2 && board[3]==2 && board[6]==2) { 
        winner=2; 
    }
    if (board[1]==2 && board[4]==2 && board[7]==2) { 
        winner=2; 
    }
    if (board[2]==2 && board[5]==2 && board[8]==2) { 
        winner=2; 
    }  
    if (board[0]==2 && board[4]==2 && board[8]==2) { 
        winner=2; 
    }
    if (board[2]==2 && board[4]==2 && board[6]==2) { 
        winner=2; 
    }
 
}
void drawBitmap(int16_t x, int16_t y,
 const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color) {

    int16_t i, j, byteWidth = (w + 7) / 8;
    uint8_t byte;

    for(j=0; j<h; j++) {
        for(i=0; i<w; i++) {
            if(i & 7) byte <<= 1;
            else            byte     = pgm_read_byte(bitmap + j * byteWidth + i / 8);
            if(byte & 0x80) tft.drawPixel(x+i, y+j, color);
        }
    }
}







