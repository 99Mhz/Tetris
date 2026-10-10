/*  GamePiece.cpp
    impliments attributes and actions a game piece can perform while in play. 
    Author: John WIltse
    Date: 10/2026
    Revisions: 

    Open bugs: Finish canRotateCCW method. you can rotate a piece when next to another piece that should blocking it. 
*/

#include "GamePiece.h"

//difference between these two is the boarder size
#define BLOCKSIZE 16
#define DRAWSIZE 14

// defines the pieces in a 4x4 bounding block
// all 4 rotation states are defined 
PieceData GamePiece::oBlock = {
    {
        0x6600,  // State 0
        0x6600,  // State 1
        0x6600,  // State 2
        0x6600   // State 3
    },
};

PieceData GamePiece::iBlock = {
    {
        0x0F00,  // State 0: 0000 1111 0000 0000 (Row 1 filled)
        0x2222,  // State 1: 0010 0010 0010 0010 (Col 2 filled)
        0x0F00,  // State 2: 0000 0000 1111 0000 (Row 2 filled) 0x00F0
        0x2222   // State 3: 0100 0100 0100 0100 (Col 1 filled) 0x4444
    },
};

PieceData GamePiece::jBlock = {
    {
        0x44C0,  // State 0: 0100 0100 1100 0000 
        0x0E20,  // State 1: 0000 1110 0010 0000
        0x6440,  // State 2: 0110 0100 0100 0000 
        0x8E00   // State 3: 1000 1110 0000 0000
    },
};

PieceData GamePiece::lBlock = {
    {
        0x4460,  // State 0: 0100 0100 0110 0000 
        0x2E00,  // State 1: 0010 1110 0000 0000
        0xC440,  // State 2: 1100 0100 0100 0000 
        0xE800   // State 3: 1110 1000 0000 0000
    },
};

PieceData GamePiece::tBlock = {
    {
        0x4E00,  // State 0: 0100 1110 0000 0000
        0x4640,  // State 1: 0100 0110 0100 0000
        0x0E40,  // State 2: 0000 1110 0100 0000
        0x4C40   // State 3: 0100 1100 0100 0000
    },
};

PieceData GamePiece::zBlock = {
    {
        0x6300,  // State 0: 0110 0011 0000 0000
        0x2640,  // State 1: 0010 0110 0100 0000
        0x6300,  // State 2: 0000 0110 0011 0000
        0x2640   // State 3: 0010 0110 0100 0000
    },
};

PieceData GamePiece::sBlock = {
    {
        0x3600,  // State 0: 0011 0110 0000 0000
        0x4620,  // State 1: 0100 0110 0010 0000
        0x3600,  // State 2: 0000 0011 0110 0000
        0x4620   // State 3: 0010 0011 0001 0000
    },
};

//constructor
GamePiece::GamePiece(const PieceData& shape, int color, int column, int row, int rotationState) {
  _shape = shape;
  _color = color;
  _column = column;
  _row = row;
  _inPlay = true;
  _rotationState = rotationState;
}

void GamePiece::draw(Adafruit_ILI9341 &gfxContext) {
    // Get the 16-bit integer for the current rotation state
    uint16_t currentBitmask = _shape.states[_rotationState]; 
    int boarderSize = BLOCKSIZE-DRAWSIZE;

    // Loop through the rows and columns of the bounding box
    // TODO: this loop gets run in multiple functions with litte modification. 
    //       figure out a way to refactor for DRY-er code
    for (int r = 0; r < _boundingBoxSize; r++) {
        for (int c = 0; c < _boundingBoxSize; c++) {
            
            // Calculate which bit we are checking (0 to 15)
            // 4 bits per row r * 4 + what column we are on
            int bitIndex = (r * 4) + c;
            
            // Check if the bit at this position is a 1
            if ((currentBitmask >> (15 - bitIndex)) & 1) {
                
                // Calculate pixel coordinates
                int pixelX = (c * BLOCKSIZE) + (_column * BLOCKSIZE);
                int pixelY = (r * BLOCKSIZE) + (_row * BLOCKSIZE);

                //gfxContext.startWrite();
                // Draw the block
                gfxContext.fillRect(pixelX+boarderSize, pixelY+boarderSize, 
                    DRAWSIZE, DRAWSIZE, _color);
            }
        }
    }
}

bool GamePiece::canMoveLeft(uint16_t gameArea[][kCols], int rows) {
    //TODO: check if we hit another piece! bug
    if(_inPlay) {
        // Get the 16-bit integer for the current rotation state
        uint16_t currentBitmask = _shape.states[_rotationState]; 
        //int boarderSize = BLOCKSIZE-DRAWSIZE;

        // Loop through the rows and columns of the bounding box
        for (int r = 0; r < _boundingBoxSize; r++) {
            for (int c = 0; c < _boundingBoxSize; c++) {
                
                // Calculate which bit we are checking (0 to 15)
                // 4 bits per row r * 4 + what column we are on
                int bitIndex = (r * 4) + c;
                
                // Check if the bit at this position is a 1
                if ((currentBitmask >> (15 - bitIndex)) & 1) {
    
                    // Calculate pixel coordinates
                    int pixelX = (c * BLOCKSIZE) + (_column * BLOCKSIZE);
                    int pixelY = (r * BLOCKSIZE) + (_row * BLOCKSIZE);
                    
                    Serial.printf("pixelX %d\n", pixelX);

                    if(pixelX <= 0) {
                        Serial.println("Hit left side of the wall");
                        return false;
                    }              
                    else if(gameArea[(pixelY/BLOCKSIZE)][(pixelX/BLOCKSIZE - 1)] > 0) {
                        Serial.println("Hit Another Piece going left");
                        return false;
                    }
                     
                }
            }
        }
        return true;
    }
    else {
        //we are not in play
        return false;
    }
}

bool GamePiece::canMoveRight(uint16_t gameArea[][kCols], int rows) {
    //TODO: check if we hit another piece! bug
    if(_inPlay) {
        // Get the 16-bit integer for the current rotation state
        uint16_t currentBitmask = _shape.states[_rotationState]; 
        //int boarderSize = BLOCKSIZE-DRAWSIZE;

        // Loop through the rows and columns of the bounding box
        for (int r = 0; r < _boundingBoxSize; r++) {
            for (int c = 0; c < _boundingBoxSize; c++) {
                
                // Calculate which bit we are checking (0 to 15)
                // 4 bits per row r * 4 + what column we are on
                int bitIndex = (r * 4) + c;
                
                // Check if the bit at this position is a 1
                if ((currentBitmask >> (15 - bitIndex)) & 1) {
    
                    // Calculate pixel coordinates
                    int pixelX = (c * BLOCKSIZE) + (_column * BLOCKSIZE);
                    int pixelY = (r * BLOCKSIZE) + (_row * BLOCKSIZE);
                    
                    if(gameArea[(pixelY/BLOCKSIZE)][(pixelX/BLOCKSIZE + 1)] > 0) {
                        Serial.println("Hit Another Piece going right");
                        return false;
                    }
                    else if(pixelX >= 144) {
                        Serial.println("Hit right side of the wall");
                        return false;
                    }
                }
            }
        }
        return true;
    }
    else {
        //We are not in play
        return false;
    }
}

bool GamePiece::canMoveDown(uint16_t gameArea[][kCols], int rows) {
    if(_inPlay) {
        // Get the 16-bit integer for the current rotation state
        uint16_t currentBitmask = _shape.states[_rotationState]; 
        //int boarderSize = BLOCKSIZE-DRAWSIZE;

        // Loop through the rows and columns of the bounding box
        // Looking for the bottom >= 304
        for (int r = 0; r < _boundingBoxSize; r++) {
            for (int c = 0; c < _boundingBoxSize; c++) {
                
                // Calculate which bit we are checking (0 to 15)
                // 4 bits per row r * 4 + what column we are on
                int bitIndex = (r * 4) + c;
                
                // Check if the bit at this position is a 1
                if ((currentBitmask >> (15 - bitIndex)) & 1) {
    
                    // Calculate pixel coordinates
                    int pixelX = (c * BLOCKSIZE) + (_column * BLOCKSIZE);
                    int pixelY = (r * BLOCKSIZE) + (_row * BLOCKSIZE);
                    
                    if(gameArea[(pixelY/BLOCKSIZE) + 1][(pixelX/BLOCKSIZE)] > 0) {
                        Serial.println("Hit Another Piece going down");
                        return false;
                    }
                    else if (pixelY >= 304) {
                        Serial.println("Hit Bottom");
                        return false;
                    }
                }
            }
        }
        return true;
    }
    else {
        return false;
    }
}

bool GamePiece::canRotateCCW(uint16_t gameArea[][kCols], int rows) {
    //
    //TODO: test all this!!!!
    // for now this function does not get called. 
    // this is intending to fix a bug where you can rotate when another piece should prevent it!
    if(_inPlay) {
        // Get the 16-bit integer for the next desired rotation state
        uint16_t nextBitmask = _shape.states[(_rotationState + 1) % 4];

        // Loop through the rows and columns of the bounding box
        // Looking for the bottom >= 304
        for (int r = 0; r < _boundingBoxSize; r++) {
            for (int c = 0; c < _boundingBoxSize; c++) {
                
                // Calculate which bit we are checking (0 to 15)
                // 4 bits per row r * 4 + what column we are on
                int bitIndex = (r * 4) + c;
                
                // Check if the bit at this position is a 1
                if ((nextBitmask >> (15 - bitIndex)) & 1) {

                    // Calculate where this block(bit) is on the game area
                    int pixelX = (c * BLOCKSIZE) + (_column * BLOCKSIZE);
                    int pixelY = (r * BLOCKSIZE) + (_row * BLOCKSIZE);
                    
                    //TODO: check if kick is possible and allow kick
                    //conjector has it that a piece won't kick with this code because there
                    //is random memory beyond the game area. 
                    /*   |  |  | I || x |
                    -------------------
                         | N| N| IN|| XN|   <-a rotate would hit this unknown value, and a kick never attempted 
                    -------------------
                         |  |  | I || 0 |
                    -------------------
                         |  |  | I || 0 |                             
                    */
                    // maybe the fix is I just don't test anything beyond the game area.
                    // But maybe there is a fatle flaw of corruption memory by allowing pieces to go off board.
                    // or allow virtually but not try to write anything out of bounds 
                    
                    //am I checking inbounds. this probably won't work because i'm not checking after the kick.
                    //maybe need a can kick method?

                    if(gameArea[(pixelY/BLOCKSIZE)][(pixelX/BLOCKSIZE + 1)] > 0) {
                        Serial.println("ROTATE: Will hit Another Piece to the right");
                            return false;
                    }
                    else if(gameArea[(pixelY/BLOCKSIZE)][(pixelX/BLOCKSIZE - 1)] > 0) {
                        Serial.println("ROTATE: Will hit Another Piece to the left");
                            return false;
                    }
                    else if(gameArea[(pixelY/BLOCKSIZE) + 1][(pixelX/BLOCKSIZE)] > 0) {
                        Serial.println("ROTATE: Will hit Another Piece below");
                        return false;
                    }
                    else if (pixelY >= 304) {
                        Serial.println("Hit Bottom");
                        return false;
                    }
                }
            }
        }
        return true;
    }
    //can't rotate we are not in play
    return false; 
}

void GamePiece::rotateCCW(Adafruit_ILI9341 &gfxContext) {
    //TODO: need a can rotate function to disallow rotation. JohnW 10/2026 in progress

    if(_inPlay) {
        //erase the piece at the old position
        erase(gfxContext); 
        
        //figure out the new rotation state
        //_rotationState = (_rotationState + 3) % 4; //for CW
        _rotationState = (_rotationState + 1) % 4;

        kickRight(); //if needed
        kickLeft(); //if needed
    
        //now redraw
        draw(gfxContext);
    }
}

void GamePiece::moveRight(Adafruit_ILI9341 &gfxContext) {
    if(_inPlay) {
        updateLocation(gfxContext, _column + 1, _row);
    }
}

void GamePiece::moveLeft(Adafruit_ILI9341 &gfxContext) {
    if(_inPlay) {
        updateLocation(gfxContext, _column - 1, _row);
    }
}

void GamePiece::moveDown(Adafruit_ILI9341 &gfxContext) {
    if(_inPlay) {
        updateLocation(gfxContext, _column, _row + 1);
    }
}

void GamePiece::erase(Adafruit_ILI9341 &gfxContext) {

  //save colors  
  int orginalColor = _color;
  int orginalBoarder = _borderColor;

  //set to black 
  _color = ILI9341_BLACK;
  _borderColor = ILI9341_BLACK;

  draw(gfxContext); //delete piece by drawing black

  //restore origonal colors
  _color = orginalColor;
  _borderColor = orginalBoarder;
}

void GamePiece::updateLocation(Adafruit_ILI9341 &gfxContext, int column, int row) {
  
    //erase the piece at the old position
    erase(gfxContext); 
    
    //update new position
    _column = column;
    _row = row;

    //draw at new position
    draw(gfxContext); 

}

void GamePiece::setInplay(bool inplay, uint16_t gameArea[][kCols], int rows) {

    // Get the 16-bit integer for the current rotation state
    uint16_t currentBitmask = _shape.states[_rotationState]; 

    if(!inplay) {
        // Loop through the rows and columns of the bounding box
        for (int r = 0; r < _boundingBoxSize; r++) {
            for (int c = 0; c < _boundingBoxSize; c++) {
                
                // Calculate which bit we are checking (0 to 15)
                // 4 bits per row r * 4 + what column we are on
                int bitIndex = (r * 4) + c;
                
                // Check if the bit at this position is a 1
                if ((currentBitmask >> (15 - bitIndex)) & 1) {
    
                    // Calculate pixel coordinates
                    int pixelX = (c * BLOCKSIZE) + (_column * BLOCKSIZE);
                    int pixelY = (r * BLOCKSIZE) + (_row * BLOCKSIZE);

                    gameArea[(pixelY/BLOCKSIZE)][(pixelX/BLOCKSIZE)] = _color;                
                }
            }
        }
    }

    //kill the piece so we don't record it again!
    _inPlay = inplay;
}

bool GamePiece::inplay() {
    return _inPlay;
}

int GamePiece::getCurrentRow() {
    return _row;
}

int GamePiece::getCurrentColumn() {
    return _column;
}

void GamePiece::kickRight() {
    // Get the 16-bit integer for the current rotation state
    uint16_t currentBitmask = _shape.states[_rotationState]; 

    // Loop through the rows and columns of the bounding box
    for (int r = 0; r < _boundingBoxSize; r++) {
        for (int c = 0; c < _boundingBoxSize; c++) {
            
            // Calculate which bit we are checking (0 to 15)
            // 4 bits per row r * 4 + what column we are on
            int bitIndex = (r * 4) + c;
            
            // Check if the bit at this position is a 1
            if ((currentBitmask >> (15 - bitIndex)) & 1) {
 
                // Calculate pixel coordinates
                int pixelX = (c * BLOCKSIZE) + (_column * BLOCKSIZE);
                int pixelY = (r * BLOCKSIZE) + (_row * BLOCKSIZE);
                
                while(pixelX < 0) {
                    _column += 1;
                    pixelX = (c * BLOCKSIZE) + (_column * BLOCKSIZE);
                }                
            }
        }
    }
}

void GamePiece::kickLeft() {
    // Get the 16-bit integer for the current rotation state
    uint16_t currentBitmask = _shape.states[_rotationState]; 

    // Loop through the rows and columns of the bounding box
    for (int r = 0; r < _boundingBoxSize; r++) {
        for (int c = 0; c < _boundingBoxSize; c++) {
            
            // Calculate which bit we are checking (0 to 15)
            // 4 bits per row r * 4 + what column we are on
            int bitIndex = (r * 4) + c;
            
            // Check if the bit at this position is a 1
            if ((currentBitmask >> (15 - bitIndex)) & 1) {
 
                // Calculate pixel coordinates
                int pixelX = (c * BLOCKSIZE) + (_column * BLOCKSIZE);
                int pixelY = (r * BLOCKSIZE) + (_row * BLOCKSIZE);
                
                //piece is already rotated by this point
                while(pixelX > 144) {
                    _column -= 1;
                    pixelX = (c * BLOCKSIZE) + (_column * BLOCKSIZE);
                }                
            }
        }
    }
}