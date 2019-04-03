#include <nes.h>

#include <SPI.h>

#include <Adafruit_WS2801.h>

const uint8_t pixelsDataPin = 10; // Yellow wire on Adafruit Pixels
const uint8_t pixelsClockPin = 11; // Green wire on Adafruit Pixels

const uint8_t boardWidth = 10;
const uint8_t boardHeight = 20;

//Defined constants that correspond with the keys in the controller class
#define BUTTON_A 0
#define BUTTON_B 1
#define BUTTON_SELECT 2
#define BUTTON_START 3
#define BUTTON_UP 4
#define BUTTON_DOWN 5
#define BUTTON_LEFT 6
#define BUTTON_RIGHT 7

const uint8_t pulse = 2;
const uint8_t latch = 3;
const uint8_t data = 4;
const int NUM_PIECES_PER_TETRAMINO = 4;
const int X_INDEX = 0;
const int Y_INDEX = 1;

Controller controller(latch, pulse, data);

bool lastButtonsPressed[8];
bool currentButtonsPressed[8];


Adafruit_WS2801 strip = Adafruit_WS2801(boardWidth, boardHeight, pixelsDataPin, pixelsClockPin);


uint32_t board[boardWidth][boardHeight];
uint32_t tetraminoColors[7] = {0xFF0000,0x00FF00,0x0000FF,0xFFFF00,0x00FFFF,0xFF00FF,0xFF7800};

//tetramino,rotation,piece,position
const int tetraminoPiecePositions[7][4][3][2] = {
  {{{-1,0},{1,0},{2,0}}, {{0,-1},{0,1},{0,2}}, {{-1,0},{1,0},{2,0}}, {{0,-1},{0,1},{0,2}}},
  {{{-1,0},{1,0},{0,1}}, {{-1,0},{0,-1},{0,1}}, {{-1,0},{0,-1},{1,0}}, {{0,-1},{1,0},{0,1}}},
  {{{-1,0},{-1,1},{0,1}}, {{-1,0},{-1,1},{0,1}}, {{-1,0},{-1,1},{0,1}}, {{-1,0},{-1,1},{0,1}}},
  {{{1,0},{-1,1},{0,1}}, {{0,-1},{1,0},{1,1}}, {{1,0},{-1,1},{0,1}}, {{0,-1},{1,0},{1,1}}},
  {{{-1,0},{0,1},{1,1}}, {{1,-1},{1,0},{0,1}}, {{-1,0},{0,1},{1,1}}, {{1,-1},{1,0},{0,1}}},
  {{{-1,0},{1,0},{-1,1}}, {{-1,-1},{0,-1},{0,1}}, {{-1,0},{1,0},{1,-1}}, {{0,-1},{0,1},{1,1}}},
  {{{-1,0},{1,0},{1,1}}, {{-1,1},{0,1},{0,-1}}, {{-1,-1},{-1,0},{1,0}}, {{0,1},{0,-1},{1,-1}}}
};

int currPosX;
int currPosY;
int currTetramino;
int currRotation;

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

  runGame();  

  drawBoard();
}

//---------------------------- Methods for Starting New Game --------------------------------
void newGame() {
  currPosX = 0;
  currPosY = 0;
  currTetramino = 0;
  currRotation = 0;
  
  clearBoard();
  setPiece();
}

void clearBoard() {
  for (int i = 0; i < boardWidth; i++) {
    for (int j = 0; j < boardHeight; j++) {
      board[i][j] = 0;
    }
  }
}

void setPiece() {
  currPosX = 4;
  currPosY = 0;
  currTetramino=random(7);
}

//------------------------- Game Methods -------------------------------------------------------------------

void runGame() {

  handleInput();   
}

void putTetraminoOnBoard() {
  
   const int LAST_PIECE = NUM_PIECES_PER_TETRAMINO - 1;

    // each tetramino consists of 4 pieces
    // loop through each piece and find its relative x & y position
   for(int tetraminoPiece=0;tetraminoPiece<NUM_PIECES_PER_TETRAMINO;tetraminoPiece++){
      int pieceXPos = 0;
      int pieceYPos = 0;

      // always uses position (0,0) for last piece (saves memory space)
      // need to calculate the other pieces
      if(tetraminoPiece != LAST_PIECE) {
        pieceXPos = tetraminoPiecePositions[currTetramino][currRotation][tetraminoPiece][X_INDEX];
        pieceYPos = tetraminoPiecePositions[currTetramino][currRotation][tetraminoPiece][Y_INDEX];
      }

      // add relative position to absolute position
      int xPos = currPosX + pieceXPos;
      int yPos = currPosY + pieceYPos;

      bool xInBounds = xPos >= 0 && xPos < boardWidth;
      bool yInBounds = yPos >= 0 && yPos < boardHeight;

      // draw on the board
      if(xInBounds && yInBounds) {
        board[xPos][yPos] = tetraminoColors[currTetramino];
      }
    }
}

void clearFilledRows() {
  for(int y = 0; y < boardHeight; y++) {
    int numFilled = 0;
    for(int x = 0; x < boardWidth; x++) {
      if(board[x][y] != 0) {
        numFilled++;
      }
    }

    if(numFilled == boardWidth) {
      // clear my row
      for(int x = 0; x < boardWidth; x++) {
        board[x][y] = 0;
      }

      // pull down all rows above me
      for(int y2 = y-1; y2 >=0; y2--) {
        for(int x = 0; x < boardWidth; x++) {
          board[x][y2+1] = board[x][y2];
        }
      }
    }    
  }
}

void moveIfClear(int posX, int posY, int rotation) {
  if(!isBlocked(posX, posY, rotation)) {
    currPosX = posX;
    currPosY = posY;
    currRotation = rotation;    
  }
}

bool isBlocked(int posX, int posY, int rotation) {  
 
     const int LAST_PIECE = NUM_PIECES_PER_TETRAMINO - 1;

    // each tetramino consists of 4 pieces
    // loop through each piece and find its relative x & y position
   for(int tetraminoPiece=0;tetraminoPiece<NUM_PIECES_PER_TETRAMINO;tetraminoPiece++){
      int pieceXPos = 0;
      int pieceYPos = 0;

      // always uses position (0,0) for last piece (saves memory space)
      // need to calculate the other pieces
      if(tetraminoPiece != LAST_PIECE) {
        pieceXPos = tetraminoPiecePositions[currTetramino][rotation][tetraminoPiece][X_INDEX];
        pieceYPos = tetraminoPiecePositions[currTetramino][rotation][tetraminoPiece][Y_INDEX];
      }

      // add relative position to absolute position
      int xPos = posX + pieceXPos;
      int yPos = posY + pieceYPos;

      bool xInBounds = xPos >= 0 && xPos < boardWidth;
      if(!xInBounds) {
        return true;
      }
      if(yPos >= boardHeight) {
        return true;
      }

      if(yPos >= 0 && board[xPos][yPos] != 0) {
        return true;
      }
    }
    return false;
}

void drawTetramino() {
  
    const int LAST_PIECE = NUM_PIECES_PER_TETRAMINO - 1;

    // each tetramino consists of 4 pieces
    // loop through each piece and find its relative x & y position
   for(int tetraminoPiece=0;tetraminoPiece<NUM_PIECES_PER_TETRAMINO;tetraminoPiece++){
      int pieceXPos = 0;
      int pieceYPos = 0;

      // always uses position (0,0) for last piece (saves memory space)
      // need to calculate the other pieces
      if(tetraminoPiece != LAST_PIECE) {
        pieceXPos = tetraminoPiecePositions[currTetramino][currRotation][tetraminoPiece][X_INDEX];
        pieceYPos = tetraminoPiecePositions[currTetramino][currRotation][tetraminoPiece][Y_INDEX];
      }

      // add relative position to absolute position
      int xPos = currPosX + pieceXPos;
      int yPos = currPosY + pieceYPos;

      bool xInBounds = xPos >= 0 && xPos < boardWidth;
      bool yInBounds = yPos >= 0 && yPos < boardHeight;

      // draw on this pixel
      if(xInBounds && yInBounds) {
        strip.setPixelColor(xPos,yPos,tetraminoColors[currTetramino]);
      }
    }
}

void drawBoard() {
  for(int x = 0; x < boardWidth; x++) {
    for (int y = 0; y < boardHeight; y++) {
      strip.setPixelColor(x,y,board[x][y]);
    }
  }

  drawTetramino();

  strip.show();
}

void moveLeft() {
  moveIfClear(currPosX-1,currPosY,currRotation);
}


void moveRight() {
  moveIfClear(currPosX+1,currPosY,currRotation);
}

void moveDown() {
  int newPosY = currPosY+1;
  if(!isBlocked(currPosX, newPosY, currRotation)) {
      currPosY = newPosY;
  } else {
      // put the tetramino on the board
      putTetraminoOnBoard();
      clearFilledRows();
      setPiece();
  }
}

void spinClockwise() {
  int newRotation = currRotation+1;
  if(newRotation > 3) {
    newRotation = 0;
  }
  moveIfClear(currPosX,currPosY,newRotation);
}

void spinCounterClockwise() {
  int newRotation = currRotation-1;
  if(newRotation < 0) {
    newRotation = 3;
  }
  moveIfClear(currPosX,currPosY,newRotation);
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

