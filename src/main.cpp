#include <Arduino.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ILI9341.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include <GamePiece.h>
#include <SPI.h>
#include <vector>

// Define Waveshare ESP32-S3-Zero specific control pins
#define TFT_CS   10
#define TFT_RST  9
#define TFT_DC   8

// Define Waveshare ESP32-S3-Zero default hardware SPI data pins
#define TFT_MOSI 11
#define TFT_SCK  13
#define TFT_MISO 12 // Optional

//difference between these two is the boarder size
#define BLOCKSIZE 16
#define DRAWSIZE 14

//screen size
#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 320

Adafruit_ILI9341 tft = Adafruit_ILI9341(&SPI, TFT_DC, TFT_CS, TFT_RST);

const uint8_t kFrameInterval = 30;
const uint8_t kRightButton = 3;
const uint8_t kLeftButton = 4;     //5 on the new boards 4 on the proto
const uint8_t kDropButton = 5;     //4 on the new boards 5 on the proto
const uint8_t kRotateButton = 6;   
const uint8_t kGameRowCount = 20;
const uint8_t kGameColumnCount = 10;
const uint8_t kStartingColumn = 3;
const uint8_t kStartingRow = 0;
const uint8_t kStartingRotation = 0;

//all states should be the same or the buttons will fire on startup
bool gameOver = false;
bool leftButtonHeldState = false;
bool rightButtonHeldState = false;

int8_t rightButtonState = HIGH;        // the current reading from the input pin
int8_t rightLastButtonState = HIGH;    // the previous reading from the input pin
int8_t leftButtonState = HIGH;         // the current reading from the input pin
int8_t leftLastButtonState = HIGH;     // the previous reading from the input pin
int8_t dropButtonState = HIGH;         // the current reading from the input pin
int8_t dropLastButtonState = HIGH;     // the previous reading from the input pin
int8_t rotateButtonState = HIGH;       // the current reading from the input pin
int8_t rotateLastButtonState = HIGH;   // the previous reading from the input pin

uint16_t frameCount = 0;
uint16_t fallSpeed = 2;                 //30 = fall one frame per second at 30FPS 2 is very fast!
uint16_t debounceDelay = 50;            //the debounce time; increase if the output flickers
uint16_t leftButtonHeldDelay = 150;     //microseconds before auto scroll sets in
uint16_t rightButtonHeldDelay = 150;    
uint16_t randomPiece = 0;
uint16_t lastRandomPiece = 0;
uint16_t nextRandomPiece = 0;
uint16_t totalRowsCleared = 0;
uint16_t level = 1;

int32_t score = 0;

uint64_t rotateLastDebounceTime = 0; // the last time the output pin was toggled
uint64_t leftLastDebounceTime = 0;   // the last time the output pin was toggled
uint64_t rightLastDebounceTime = 0;  // the last time the output pin was toggled
uint64_t dropLastDebounceTime = 0;   // the last time the output pin was toggled
uint64_t previousMillis = 0;
uint64_t leftButtonHeldMillis = 0;
uint64_t rightButtonHeldMillis = 0;

/*  The gameboard is too big to express as a unsigned long 
    so double array is the best we can do.
    If a color (int) is in the game area then a piece is there
    Holds color so we can drop rows and redraw the board
    TODO: perhaps make a class out of this
*/
uint16_t gameArea[kGameRowCount][kGameColumnCount];

//to pick a random game piece
static GamePiece pieces[7] = {
  GamePiece(GamePiece::oBlock, ILI9341_YELLOW, kStartingColumn, kStartingRow, kStartingRotation),
  GamePiece(GamePiece::sBlock, ILI9341_GREEN, kStartingColumn, kStartingRow, kStartingRotation),
  GamePiece(GamePiece::zBlock, ILI9341_RED, kStartingColumn, kStartingRow, kStartingRotation),
  GamePiece(GamePiece::lBlock, ILI9341_ORANGE, kStartingColumn, kStartingRow, kStartingRotation),
  GamePiece(GamePiece::jBlock, ILI9341_BLUE, kStartingColumn, kStartingRow, kStartingRotation),
  GamePiece(GamePiece::iBlock, ILI9341_CYAN, kStartingColumn, kStartingRow, kStartingRotation),
  GamePiece(GamePiece::tBlock, ILI9341_MAGENTA, kStartingColumn, kStartingRow, kStartingRotation)
};

//Define an active piece
GamePiece activePiece = GamePiece(GamePiece::oBlock, ILI9341_YELLOW, kStartingColumn, kStartingRow, kStartingRotation);
GamePiece nextPiece = GamePiece(GamePiece::oBlock, ILI9341_YELLOW, kStartingColumn, kStartingRow, kStartingRotation);

//Function templates
void readInputs();
void renderGraphics();
void renderNextUp();
void renderScore();
void renderLevel();
void renderTotalRows();
void renderSetteledRow(int32_t row);
void getNextPiece();
void checkFullRow();
void updateGameLogic();

void setup() {

  Serial.begin(115200);
  uint64_t startTimer = millis();
  while(!Serial && (millis() - startTimer < 3000)) { 
    delay(10); 
  }

  //Windows is slow to open comm
  delay(2000);

  Serial.println("Starting Tetris Clone on Waveshare esp32 s3 zero!");
  Serial.println("Display uses a Waveshare 2.4 inch LCD display module using SPI!");

  //seed a random number
  randomSeed(analogRead(A0)); 

  // Set nextup to a new random piece
  //Create another NEXT piece
  nextRandomPiece = random(7);

// Initialize SPI bus
  SPI.begin(TFT_SCK, TFT_MISO, TFT_MOSI, TFT_CS);
  tft.begin();

  //not two in a row
  while (lastRandomPiece == randomPiece) {
    randomPiece = random(7);
  }
  lastRandomPiece = randomPiece;

  activePiece = pieces[randomPiece];
  getNextPiece();

  //Setup the control buttons
  //I have SMD (0805) pads on the back of the board for external pull up resistors 10k Ohm
  pinMode(kRotateButton, INPUT_PULLUP);
  pinMode(kDropButton, INPUT_PULLUP);
  pinMode(kRightButton, INPUT_PULLUP);
  pinMode(kLeftButton, INPUT_PULLUP);

  neopixelWrite(21, 0, 0, 0);  // Turn distracting onboard neopixel off (Pin, R, G, B)

  //rotate to go landscape USB to upper right 0,0 is upper left
  tft.setRotation(0);
  tft.fillScreen(ILI9341_BLACK);
  tft.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ILI9341_GREEN); //size of the play area
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
  tft.setTextSize(3);

  renderNextUp();
  renderScore();
  renderLevel();
  renderTotalRows();
}

void loop() {
  readInputs();
 
  uint64_t currentMillis = millis();
  if(currentMillis - previousMillis >= kFrameInterval) {
    frameCount++;

    //save current millis
    previousMillis = currentMillis;
    
    //Now update the game
    updateGameLogic();
    renderGraphics();
  }
}

void readInputs() {
  int32_t rotateReading = digitalRead(kRotateButton);
  int32_t dropReading = digitalRead(kDropButton);
  int32_t leftReading = digitalRead(kLeftButton);
  int32_t rightReading = digitalRead(kRightButton);

  if(rotateReading != rotateLastButtonState) {
    rotateLastDebounceTime = millis();
  }

  if(leftReading != leftLastButtonState) {
    //starts the debounce counter 
    leftLastDebounceTime = millis(); 
  }

  if(rightReading != rightLastButtonState) {
    rightLastDebounceTime = millis();
  }

  if(dropReading != dropLastButtonState) {
    dropLastDebounceTime = millis();
  }

  if (!gameOver && (millis() - rotateLastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (rotateReading != rotateButtonState) {
      rotateButtonState = rotateReading;

      if (rotateButtonState == LOW) {
        Serial.print("ROTATE Button Was pushed. Current row is: ");
        Serial.println(activePiece.getCurrentRow());
        //TODO: call canRotatePiece first!!
        //      There is a bug here!
        activePiece.rotateCCW(tft); //-90
      }
    }
  }
  
  if (!gameOver && (millis() - leftLastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (leftReading != leftButtonState) {
      leftButtonState = leftReading;

      if (leftButtonState == LOW) {
        Serial.print("LEFT Button Was pushed. Current row is: ");
        Serial.print(activePiece.getCurrentRow());
        Serial.print("  Current column is: ");
        Serial.println(activePiece.getCurrentColumn());

        if(activePiece.canMoveLeft(gameArea, kGameRowCount)) {
          activePiece.moveLeft(tft);
        }
        //reset the hold counter
        leftButtonHeldMillis = millis();
      }
    }
    if(leftButtonState == LOW) { //held down      
      if(!leftButtonHeldState) {
        leftButtonHeldMillis = millis();
        leftButtonHeldState = true; //set bit so we don't keep updating it
      }
      if(millis() - leftButtonHeldMillis > leftButtonHeldDelay) {
        Serial.print("LEFT Button is Held. Current column is: ");
        Serial.println(activePiece.getCurrentColumn());
        if(activePiece.canMoveLeft(gameArea, kGameRowCount)) {
          activePiece.moveLeft(tft);      
        }
        leftButtonHeldState = false;  //reset to get a new delay
      }
    }
  }

  if (!gameOver && (millis() - rightLastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than 
    // the debounce delay, so take it as the actual current state:

    // if the button state has changed:
    if (rightReading != rightButtonState) {
      rightButtonState = rightReading;

      if (rightButtonState == LOW) {
        Serial.print("RIGHT Button Was pushed. Current row is: ");
        Serial.println(activePiece.getCurrentRow());
                
        if(activePiece.canMoveRight(gameArea, kGameRowCount)) {
          activePiece.moveRight(tft);
        }
        //reset the hold counter
        rightButtonHeldMillis = millis();
      }
    }
    if(rightButtonState == LOW) { //held down
      if(!rightButtonHeldState) {
        rightButtonHeldMillis = millis();
        rightButtonHeldState = true; //set bit so we don't keep updating it
      }
      if(millis() - rightButtonHeldMillis > rightButtonHeldDelay) {
        Serial.print("RIGHT Button is Held. Current column is: ");
        Serial.println(activePiece.getCurrentColumn());
        if(activePiece.canMoveRight(gameArea, kGameRowCount)) {
          activePiece.moveRight(tft);      
        }
        rightButtonHeldState = false; //reset to get a new delay
      }
    }
  }

  if (!gameOver && (millis() - dropLastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (dropReading != dropButtonState) {
      dropButtonState = dropReading;

      if (dropButtonState == LOW) {
        Serial.print("DROP Button Was pushed. Current row is: ");
        Serial.println(activePiece.getCurrentRow());

        Serial.print("In play from move down: ");
        Serial.println(activePiece.inplay());

        while(activePiece.canMoveDown(gameArea, kGameRowCount)) {
          activePiece.moveDown(tft);
        }
        //kill this piece, and place it in the gameArea array
        activePiece.setInplay(false, gameArea, kGameRowCount);
      }
    }
  }

  if( gameOver
      && ((millis() - dropLastDebounceTime) > debounceDelay) 
      && ((millis() - rotateLastDebounceTime) > debounceDelay)) {
        if((dropReading != dropButtonState) && (rotateReading != rotateButtonState)) {
          rotateButtonState = rotateReading;
          dropButtonState = rotateReading;
          Serial.println("RESET sequence was pressed");   

          score = 0;
          level = 1;
          totalRowsCleared = 0;

          for(int32_t column=0; column<kGameColumnCount; column++) {
            for(int32_t row=0; row<kGameRowCount; row++) {
              gameArea[row][column] = 0;
            }
          }

          tft.fillScreen(ILI9341_BLACK);
          gameOver = false;

          updateGameLogic();
          renderGraphics();
          renderNextUp();
          renderScore();
          renderLevel();
          renderTotalRows();       

          rotateButtonState = HIGH;
          dropButtonState = HIGH;
          
          //Needed so the new current gamePiece will not 
          //rotate and drop because the buttons are held down!
          sleep(2);
        }
  }

  rotateLastButtonState = rotateReading;
  leftLastButtonState = leftReading;
  rightLastButtonState = rightReading;
  dropLastButtonState = dropReading;
}

void renderGraphics() {
  if(gameOver) {
    tft.setTextSize(3);
    tft.setCursor(0, 150);
    tft.print("Game Over");
  }
  else {
    //Draw the active piece, and other game elements
    activePiece.draw(tft);
      //draw rectangle of game are for 16x16 block
    tft.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ILI9341_GREEN);
  }
}

void renderNextUp() {
  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setCursor(170, 20);
  tft.print("Next");
  
  //for performance reasons I don't want this rendered 30/second!
  tft.fillRect(165, 48, 90, 48, ILI9341_BLACK);
  //Draw next up
  nextPiece.draw(tft);
}

void renderScore() {
  //int score = 12000;
  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setCursor(170, 120);
  tft.print("Score");

  tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
  tft.setCursor(170, 150);
  tft.print(score);
}

void renderLevel() {
  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setCursor(170, 190);
  tft.print("Level");

  tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
  tft.setCursor(170, 220);
  tft.print(level);
}

void renderTotalRows() {
  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setCursor(170, 260);
  tft.print("Rows");

  tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
  tft.setCursor(170, 290);
  tft.print(totalRowsCleared);
}

void renderSetteledRow(int32_t row) {
  // Calculate dimensions
  //int rows = (sizeof(gameArea) / sizeof(gameArea[0]));
  int32_t cols = (sizeof(gameArea[0]) / sizeof(gameArea[0][0]));

  //redraw all the settled blocks. We need this when we clear full lines
  //consider this in a method that takes a row to update to make it quicker
  if(!gameOver) {
    int32_t boarderSize = BLOCKSIZE-DRAWSIZE;

    //render all the settled pieces in a row
    for(int32_t col=0; col<cols; ++col) {
                  
      int32_t xPos = col * BLOCKSIZE;
      int32_t yPos = row * BLOCKSIZE;

      //Won't draw black because > 0 in loop. 
      //draw a gamepiece block with the color that is there
      tft.fillRect(xPos + boarderSize, yPos + boarderSize, DRAWSIZE, DRAWSIZE, gameArea[row][col]);          
    }
  }
}

void getNextPiece() {
        // Set game piece to the next up
        activePiece = pieces[nextRandomPiece];

        // Set nextup to a new random piece
        //Create another NEXT piece
        randomPiece = random(7);

        //not two in a row
        while (lastRandomPiece == randomPiece) {
          randomPiece = random(7);
        }
        lastRandomPiece = randomPiece;

        //nextPiece = GamePiece(pieces[randomPiece].gp, pieces[randomPiece].color, 11, 3, 0);
        nextPiece = pieces[randomPiece];
        nextPiece.updateLocation(tft, 11, 3); //show piece in the gutter to the right

        Serial.print("Current piece: ");
        Serial.println(nextRandomPiece);

        Serial.print("Next up: ");
        Serial.println(randomPiece);

        //Save this for the next round
        nextRandomPiece = randomPiece; 

        renderNextUp();
}

void checkFullRow() {
    // Calculate dimensions
    int32_t rows = sizeof(gameArea) / sizeof(gameArea[0]);
    int32_t cols = sizeof(gameArea[0]) / sizeof(gameArea[0][0]);
    std::vector<int32_t> fullRows;

  //look backward through the game grid seeing if any row is full
  for(int32_t row=rows; row>0; --row) {
    bool fullRow = true; //assume full till its not
    for(int32_t col=0; col<cols; col++) {
      if(gameArea[row][col] == false) {
        fullRow = false; //all hope is lost for a full row
      }
    }
    //if we still have a full row 
    if(fullRow) {
      fullRows.push_back(row);
    }
  }

  totalRowsCleared += fullRows.size();
  score += (fullRows.size() * 100) * level;
  if(fullRows.size() >= 2) {
    //Bonus
    score += 50 * fullRows.size();
  }
  renderScore();
  renderLevel();
  renderTotalRows();

  //Scan 1 find all the full rows
  for (const auto& r : fullRows) {
    for(int32_t col=0; col<cols; col++) {
      gameArea[r][col] = 0; //clear every cell in the row
    }
    renderSetteledRow(r);
  }

  //Scan 2 fall all the rows above by how many empty rows are below
  //look backward through the game grid seeing if any row is full

  if(!fullRows.empty()) {
    Serial.println("in drop code!");
    int32_t firstEmptyRow = fullRows.at(0);
    int32_t emptyRowsBelow = 1; //keep track of how much we have to fall each row

    Serial.print("first empty row");
    Serial.println(firstEmptyRow);
    Serial.print("full rows: ");
    Serial.println(fullRows.size());
    
    //firstEmptyRow - 1 because we are working above the empty row. 
    for(int32_t row=firstEmptyRow-1; row>0; --row) {
      for(int32_t col=0; col<cols; col++) {
        //fall each column by emptyRowsBelow
        gameArea[row+emptyRowsBelow][col] = gameArea[row][col]; 
      }
      //now render the row below we just cleared and filled in
      renderSetteledRow(row+emptyRowsBelow);

      //check if the next row up is empty
      for (const auto& r : fullRows) {
        //check below!!! probably wrong
        if(r == row) {
          emptyRowsBelow++;
        }
      }
    }
  }
}

void updateGameLogic() {
  //TODO: finish this

  if(!gameOver) {
    //move characters, check collisions, update scores etc
    if (frameCount >= fallSpeed) {

      //Calculate level
      //Could be a switch statement also, or better find a function!
      if(totalRowsCleared < 10) { level = 1; }
      else if(totalRowsCleared < 20) { level = 2; }
      else if(totalRowsCleared < 30) { level = 3; }
      else if(totalRowsCleared < 40) { level = 4; }
      else if(totalRowsCleared < 50) { level = 5; }
      else { level = 6; } //!!

      //Calculate level speed
      //Could be a switch statement also, or better find a function!
      if(level == 1) { fallSpeed = 22; }
      else if(level == 2) { fallSpeed = 18; }
      else if(level == 3) { fallSpeed = 14; }
      else if(level == 4) { fallSpeed = 10; }
      else if(level == 5) { fallSpeed = 6; }
      else { fallSpeed = 2; } //must be level 6!!
      
      if(!activePiece.inplay()) {
        Serial.print("Inplay: ");
        Serial.println(activePiece.inplay());

        getNextPiece();
 
        if(!activePiece.canMoveDown(gameArea, kGameRowCount))
        {
          gameOver = true;
          Serial.printf("!!! Game Over !!! Final score is %d \n", score);
        }
      }
      else if (activePiece.canMoveDown(gameArea, kGameRowCount)) {
        activePiece.updateLocation(tft, activePiece.getCurrentColumn(), activePiece.getCurrentRow() + 1);
        frameCount = 0; // Reset counter
        checkFullRow();
      }
      else {
          //kill this piece, and place it in the gameArea array
          activePiece.setInplay(false, gameArea, kGameRowCount);
      }
    }
  }
}