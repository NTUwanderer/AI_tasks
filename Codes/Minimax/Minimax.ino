#include <Adafruit_GFX.h>        // Core graphics library
#include <SPI.h>
#include <Wire.h>            // this is needed even tho we aren't using it
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>
#include <ESP8266WiFi.h>
#include <WebSocketServer.h>
#include <WebSocketClient.h>

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

#define HOSTNAME "ESP-"

WiFiServer server(80);
WebSocketServer webSocketServer;
WiFiClient client;
WebSocketClient webSocketClient;
const String path = "";
const String host = "192.168.4.1";

const char* espSSID = "ESP-5A12F8";
const char* espPASS = "NTUEE_AI";

const char* mySSID = "Justin_And_Harvey";
const char* myPASS = "27199342jh";

extern uint8_t circle[];
extern uint8_t x_bitmap[];

typedef unordered_map<unsigned long long, int>  HeuMap;

enum GameState{startMode, playerMode, PvEMode, MinimaxMode, HostWaitMode, HostMode, ClientConnectMode, ClientMode, endMode};
GameState gameState;

ButtonCoordinate playerButton, PvEButton, MinimaxButton, HostButton, ClientButton, giveupButton, resetButton;
ButtonCoordinate buttons[8][8], ssidButtons[10];
String ssids[10];

State initState, currentState;
int countMinimax;
bool availables[8][8];
bool redTurn;
const int depth = 5;

int minimax(State& s, const bool (&availables)[8][8], bool redTurn, int depth, int alpha = INT_MIN, int beta = INT_MAX);

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
    if (istouched || (gameState == PvEMode && !redTurn) || gameState == MinimaxMode) {
        istouched = false;
        TS_Point p0 = ts.getPoint(),p;  //Get touch point

        p.x = map(p0.x, TS_MINX, TS_MAXX, 0, 320);
        p.y = map(p0.y, TS_MINY, TS_MAXY, 0, 240);

        if (istouched) {
            Serial.print("X = "); Serial.print(p.x);
            Serial.print("\tY = "); Serial.print(p.y);
            Serial.print("\n");
        }
                
        if (gameState == startMode) {
            if (playerButton.pressed(p.x, p.y)) {
                resetGame();
                printState(currentState, redTurn);
                gameState = playerMode;
                drawGiveupButton();
            } else if (PvEButton.pressed(p.x, p.y)) {
                resetGame();
                printState(currentState, redTurn);
                gameState = PvEMode;
                drawGiveupButton();
            } else if (MinimaxButton.pressed(p.x, p.y)) {
                resetGame();
                printState(currentState, redTurn);
                gameState = MinimaxMode;
            } else if (HostButton.pressed(p.x, p.y)) {
                hostSetup();
                drawResetButton();
                gameState = HostWaitMode;
            } else if (ClientButton.pressed(p.x, p.y)) {
                clientSetup();
                drawResetButton();
                gameState = ClientConnectMode;
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
        } else if (gameState == PvEMode) {
            if (!redTurn) {
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
                } else {
                    drawGiveupButton();
                }
            }
            else if (giveupButton.pressed(p.x, p.y)) {
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

        } else if (gameState == HostWaitMode) {

            if (resetButton.pressed(p.x, p.y)) {
                WiFi.softAPdisconnect(true);
                gameState = startMode;
                drawStartScreen();
            }

            client = server.available();
            if (client && client.connected()) {
                tft.print ("client connected");
                if (webSocketServer.handshake(client)) {

                    tft.fillScreen(BLACK);
                    gameState = HostMode;
                } else {
                    tft.print ("handshake fail");
                }
            } else {
                Serial.println ("client not connected");
            }

            
        } else if (gameState == ClientConnectMode) {

            if (resetButton.pressed(p.x, p.y)) {
                WiFi.disconnect(true);
                gameState = startMode;
                drawStartScreen();
            }

            int clickedI = -1;
            for (int i = 0; i < 10; ++i) {
                if (ssidButtons[i].pressed(p.x, p.y)) {
                    clickedI = i;
                    break;
                }
            }
            if (clickedI >= 0) {
            // if (clickedI >= 0 && ssids[clickedI].substring(0, 3) == "ESP") {
                Serial.print ("ssid: ");
                Serial.println (ssids[clickedI].c_str());
                // WiFi.begin(ssids[clickedI].c_str(), espPASS);
                Serial.print ("strcmp: ");
                Serial.print (strcmp(espSSID, ssids[clickedI].c_str()));
                WiFi.begin(ssids[clickedI].c_str(), espPASS);
                // WiFi.begin(mySSID, myPASS);
                Serial.print ("Start connecting");
                while (WiFi.status() != WL_CONNECTED) {
                    Serial.print ("Connecting to ");
                    Serial.print (ssids[clickedI].c_str());
                    delay(1000);
                }
                
                webSocketClient.path = "/";
                webSocketClient.host = "192.168.4.1";
                if (client.connect(host.c_str(), 80)) {
                    tft.print ("connect to server");
                    if (webSocketClient.handshake(client)) {
                        tft.fillScreen(BLACK);
                        gameState = ClientMode;
                    } else {
                        tft.print ("handshake fail");
                    }
                } else {
                    tft.print("Fail to Connect");
                }
            }
        } else if (gameState == HostMode) {

            if (resetButton.pressed(p.x, p.y)) {
                WiFi.softAPdisconnect(true);
                gameState = startMode;
                drawStartScreen();
            }
            String data;
            while (client.connected()) {
                data = webSocketServer.getData();
 
                if (data.length() > 0) {
                   Serial.println(data);
                   webSocketServer.sendData("I'm host");
                }
                delay(1000);
            }
            Serial.println("The client disconnected");
            delay(3000);

        } else if (gameState == ClientMode) {

            if (resetButton.pressed(p.x, p.y)) {
                WiFi.disconnect(true);
                gameState = startMode;
                drawStartScreen();
            }
            String data;
            while (client.connected()) {
                webSocketClient.sendData("I'm client");
 
                webSocketClient.getData(data);
                if (data.length() > 0) {
                   Serial.println(data);
                }
                delay(1000);
            }
            Serial.println("The host disconnected");
            delay(3000);

        } else if (gameState == endMode) {
            gameState = startMode;
            drawStartScreen();
        } else {
            Serial.println("Unknown GameState:");
            Serial.println(gameState);
        }

        delay(10);  
    }
}

void hostSetup() {
    tft.fillScreen(BLACK);
    tft.setTextColor(WHITE);
    tft.setTextSize(3);
    tft.setTextSize(3);
    tft.setCursor(10, 10);
    tft.print("Waiting for \nconnection...");

    WiFi.mode(WIFI_AP);
    String hostname(HOSTNAME);
    String id = String(ESP.getChipId(), HEX);
    id.toUpperCase();
    hostname += id;
    // local ip: 192.168.4.1
    // IPAddress local_IP(192,168,4,22);
    // IPAddress gateway(192,168,4,9);
    // IPAddress subnet(255,255,255,0);
    // WiFi.softAPConfig(local_IP, gateway, subnet);
    boolean result = WiFi.softAP(hostname.c_str(), espPASS);
    if(result == true) {
        Serial.println("Ready");
        Serial.print("My localIP: ");
        Serial.println(WiFi.softAPIP());
        tft.setCursor(10, 80);
        tft.print("My SSID:");
        tft.setCursor(30, 110);
        tft.print(hostname);
        server.begin();
    }
    else {
        Serial.println("Failed!");
        tft.setCursor(10, 50);
        tft.print("Fail to establish AP");
        delay(2000);

        WiFi.softAPdisconnect(true);
        gameState = startMode;
        drawStartScreen();
    }

}

void clientSetup() {
    tft.fillScreen(BLACK);
    tft.setTextColor(WHITE);
    tft.setTextSize(3);
    tft.setCursor(10, 10);
    tft.print("Choose a network");

    // WiFi.begin("Justin_And_Harvey", "27199342jh");
    // if (WiFi.status() == WL_CONNECTED) {
    //     tft.print ("connect to Wifi");
    // } else {
    //     tft.print ("not connect to Wifi");
    // }
    WiFi.mode(WIFI_STA);
    // WiFi.disconnect();
    int n = WiFi.scanNetworks();

    for (int i = 0; i < 10; ++i)
        ssidButtons[i].fillAndDraw(tft, BLACK, WHITE);

    tft.setTextSize(2);
    for (int i = 0; i < min(10, n); ++i) {
        ssids[i] = WiFi.SSID(i);
        tft.setCursor(ssidButtons[i].x + 5, ssidButtons[i].y + 5);
        tft.print(ssids[i].substring(0, 10));
        // tft.print(WiFi.SSID(i).substr(0, 10));
    }
}

void drawResetButton() {
    resetButton.fillAndDraw(tft, RED, WHITE);
    tft.setCursor(resetButton.x + 2, resetButton.y + 2);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.print("reset");
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
    playerButton = ButtonCoordinate(10,180,90,40);
    PvEButton = ButtonCoordinate(110,180,90,40);
    MinimaxButton  = ButtonCoordinate(210,180,90,40);

    HostButton  = ButtonCoordinate(30,130,120,40);
    ClientButton  = ButtonCoordinate(170,130,120,40);

    resetButton = ButtonCoordinate(250, 50, 70, 25);
    giveupButton = ButtonCoordinate(5,200,80,30);
    for (int i = 0; i < 8; ++i)
        for (int j = 0; j < 8; ++j)
            buttons[i][j] = ButtonCoordinate(90 + 1 + i * 28, 10 + 1 + j * 28, 27, 27);

    for (int i = 0; i < 10; ++i) {
        ssidButtons[i] = ButtonCoordinate(10 + (i/5) * 160, 75 + (i % 5) * 33, 140, 30);
    }

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
    tft.setCursor(30,80);
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
    playerButton.fillAndDraw(tft, RED, WHITE);
    tft.setCursor(playerButton.x + 10, playerButton.y + 12);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.print("Player");

    PvEButton.fillAndDraw(tft, RED, WHITE);
    tft.setCursor(PvEButton.x + 6, PvEButton.y + 10);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.print("P vs E");

    MinimaxButton.fillAndDraw(tft, RED, WHITE);
    tft.setCursor(MinimaxButton.x + 6, MinimaxButton.y + 10);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.print("Minimax");

    HostButton.fillAndDraw(tft, RED, WHITE);
    tft.setCursor(HostButton.x + 26, HostButton.y + 10);
    tft.setTextColor(WHITE);
    tft.setTextSize(3);
    tft.print("Host");

    ClientButton.fillAndDraw(tft, RED, WHITE);
    tft.setCursor(ClientButton.x + 6, ClientButton.y + 10);
    tft.setTextColor(WHITE);
    tft.setTextSize(3);
    tft.print("Client");
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
int minimax(State& s, const bool (&availables)[8][8], bool redTurn, int depth, int alpha, int beta) {

    yield();
    int bestX = -1, bestY = -1;
    int bestHeu = 0;

    bool newAvailable[8][8];
    bool finish = false;

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (!availables[i][j])
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
