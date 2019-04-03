#include <nes.h>

#include <SPI.h>

#include <Adafruit_WS2801.h>

const uint8_t pixelsDataPin = 10; // Yellow wire on Adafruit Pixels
const uint8_t pixelsClockPin = 11; // Green wire on Adafruit Pixels


const uint16_t boardWidth = 10;
const uint16_t boardHeight = 20;

//Defined constants that correspond with the keys in the controller class
#define BUTTON_A 0
#define BUTTON_B 1
#define BUTTON_SELECT 2
#define BUTTON_START 3
#define BUTTON_UP 4
#define BUTTON_DOWN 5
#define BUTTON_LEFT 6
#define BUTTON_RIGHT 7

const int pulse = 2;
const int latch = 3;
const int data = 4;

Controller controller(latch, pulse, data);

bool lastButtonsPressed[8];
bool currentButtonsPressed[8];


Adafruit_WS2801 strip = Adafruit_WS2801(boardWidth, boardHeight, pixelsDataPin, pixelsClockPin);


uint32_t board[boardWidth][boardHeight];
uint32_t pieceColors[7] = {0xFF0000,0x00FF00,0x0000FF,0xFFFF00,0x00FFFF,0xFF00FF,0xFF7800};

void setup() {
  // setup LEDs
  strip.begin();
  strip.show();

  // "initialize" random
  randomSeed(300);
}

void loop() {
  
  updateController();
  
  if(shouldRestart()){    
    newGame();
  }

  drawBoard();
}

//---------------------------- Methods for Starting New Game --------------------------------
void newGame() {
  clearBoard();  
  randomizeBoard();
}

void clearBoard() {
  for (int i = 0; i < boardWidth; i++) {
    for (int j = 0; j < boardHeight; j++) {
      board[i][j] = 0;
    }
  }
}

void randomizeBoard() {
  for (int i = 0; i < boardWidth; i++) {
    for (int j = 0; j < boardHeight; j++) {
      int colorIndex = random(7);;
      uint32_t color = pieceColors[colorIndex];
      board[i][j] = color;
    }
  }
}

//------------------------- Game Methods -------------------------------------------------------------------

void drawBoard() {
  for(int x = 0; x < boardWidth; x++) {
    for (int y = 0; y < boardHeight; y++) {
      strip.setPixelColor(x,y,board[x][y]);
    }
  }

  strip.show();
}

void moveLeft() {

  // for now, set all blocks to RED
  setAllBlockColors(0);
}


void moveRight() {

  // for now, set all blocks to BLUE
  setAllBlockColors(1);
}

void moveDown() {
  // for now, set all block colors to GREEN
  setAllBlockColors(2);
}

void spinClockwise() {
  // for now, set all block colors to YELLOW
  setAllBlockColors(3);
}

void spinCounterClockwise() {
  setAllBlockColors(4);
}

void setAllBlockColors(int pieceColorIndex) {
  for(int x = 0; x < boardWidth; x++) {
    for(int y = 0; y < boardHeight; y++) {
      uint32_t color = pieceColors[pieceColorIndex];
      board[x][y] = color;
    }
  }
}


//----------------------- Input Methods -----------------------------------------------------------------

bool shouldRestart() {
  if(isControllerTriggered(BUTTON_SELECT)){
    return true;
  }
  return false;
}

void handleInput() {
  if(isControllerTriggered(BUTTON_LEFT)){
      moveLeft();
    }
    if(isControllerTriggered(BUTTON_RIGHT)){
      moveRight();
    }
    if(controller.pressed(BUTTON_DOWN)){
      moveDown();
    }
    if(isControllerTriggered(BUTTON_A)){
      spinClockwise();
    }
    if(isControllerTriggered(BUTTON_B)){
      spinCounterClockwise();
    }
}

void updateController() {
  // update last frame button info
  for(int i = 0; i < 8; i++) {
    lastButtonsPressed[i] = currentButtonsPressed[i];
  }
  controller.latch();
  // update current frame button info
  for(int i = 0; i < 8; i++) {
    currentButtonsPressed[i] = controller.pressed(i);
  }
}

bool isControllerTriggered(int button) {
  return(currentButtonsPressed[button] && !lastButtonsPressed[button]);
}

