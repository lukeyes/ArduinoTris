#include <SPI.h>

#include <Adafruit_WS2801.h>

#include <NESController.h>

const uint8_t pixelsDataPin = 10; // Yellow wire on Adafruit Pixels
const uint8_t pixelsClockPin = 11; // Green wire on Adafruit Pixels


const uint16_t boardWidth = 10;
const uint16_t boardHeight = 20;

NESController controller = NESController(2,3,4);
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
  
  controller.controllerRead();
  
  if(shouldRestart()){    
    newGame();
  }

  drawBoard();
}

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

void drawBoard() {
  for(int x = 0; x < boardWidth; x++) {
    for (int y = 0; y < boardHeight; y++) {
      strip.setPixelColor(x,y,board[x][y]);
    }
  }

  strip.show();
}

bool shouldRestart() {
  if(controller.buttonPressed(controller.BUTTON_SELECT) && !controller.buttonHandled(controller.BUTTON_SELECT)){
    controller.handleButton(controller.BUTTON_SELECT);
    return true;
  }
  return false;
}

