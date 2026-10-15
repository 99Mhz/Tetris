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
        0x0E80   // State 3: 1110 1000 0000 0000  0000 1110 1000 0000
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

void GamePiece::forEachActiveBlock(std::function<void(int r, int c, int pixelX, int pixelY) > action, uint16_t bitmask) {
    //no bitmask was passed
    uint16_t currentBitmask = (bitmask == 0) ? _shape.states[_rotationState] : bitmask;
    
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

                action(r, c, pixelX, pixelY);

            }
        }
    }
}

bool GamePiece::canMoveLeft(uint16_t gameArea[][kCols], int rows) {
    if (!_inPlay) return false;

    bool canMove = true;

    //TODO: check if we hit another piece! bug
    forEachActiveBlock([&](int r, int c, int pixelX, int pixelY) {
//        Serial.printf("pixelX %d\n", pixelX);

        if(pixelX <= 0) {
//            Serial.println("Hit left side of the wall");
            canMove = false;
        }              
        else if(gameArea[(pixelY/BLOCKSIZE)][(pixelX/BLOCKSIZE - 1)] > 0) {
//            Serial.println("Hit Another Piece going left");
            canMove = false;
        }
    });

    return canMove;
}

bool GamePiece::canMoveRight(uint16_t gameArea[][kCols], int rows) {
    if(!_inPlay) { return false; }
    
    bool canMove = true;

    forEachActiveBlock([&](int r, int c, int pixelX, int pixelY) {
        if(gameArea[(pixelY/BLOCKSIZE)][(pixelX/BLOCKSIZE + 1)] > 0) {
//            Serial.println("Hit Another Piece going right");
            canMove = false;
        }
        else if(pixelX >= 144) {
//            Serial.println("Hit right side of the wall");
            canMove = false;
        }
    });

    return canMove;
}

bool GamePiece::canMoveDown(uint16_t gameArea[][kCols], int rows) {
    if(!_inPlay) { return false; }
    
    bool canMove = true;

    forEachActiveBlock([&](int r, int c, int pixelX, int pixelY) {
        if(gameArea[(pixelY/BLOCKSIZE) + 1][(pixelX/BLOCKSIZE)] > 0) {
//            Serial.println("Hit Another Piece going down");
            canMove = false;
        }
        else if (pixelY >= 304) {
//            Serial.println("Hit Bottom");
            canMove = false;
        }
    });

    return canMove;
}

bool GamePiece::canRotateCCW(uint16_t gameArea[][kCols], int rows) {
    if(!_inPlay) { return false; }

    bool bKickLeftNeeded = false;
    bool bKickRightNeeded = false;
    bool bCanKickLeft = false;
    bool bCanKickRight = false;
    bool canRotate = true;

    // Get the 16-bit integer for the next desired rotation state
    uint16_t nextBitmask = _shape.states[(_rotationState + 1) % 4];

    forEachActiveBlock([&](int r, int c, int pixelX, int pixelY) {
        //since we virtually rotated a part of the piece can be out of the game area
        //But this one is whitin the game area
        if((pixelX/BLOCKSIZE) <= 9 && (pixelX/BLOCKSIZE) >= 0) {
//            Serial.println("Rotate: Within game area");
            //since we are within the gamearea check the piece part with something in that located in the gamearea
            if(gameArea[(pixelY/BLOCKSIZE)][(pixelX/BLOCKSIZE)] > 0) {
//                Serial.println("ROTATE: Will hit Another Piece at:");
//                Serial.printf("X: %d Y: %d Color: %d \n", (pixelX/BLOCKSIZE), (pixelY/BLOCKSIZE), gameArea[(pixelY/BLOCKSIZE)][(pixelX/BLOCKSIZE)]);
                canRotate = false;
            }
        }
        //this piece part is right of the gamearea. Allow rotate only if it cn kick left
        else if((pixelX/BLOCKSIZE) > 9) {
//            Serial.println("Rotate: Outside game area");
            bKickLeftNeeded = true;
            if(canKickLeft(gameArea)) {
                bCanKickLeft = true;
            }
        }
        //this piece part if left of the gamearea. Allow rotate only if it cn kick right
        else if((pixelX/BLOCKSIZE) < 0) {
//            Serial.println("Rotate: Outside game area");
            bKickRightNeeded = true;
            if(canKickRight(gameArea)) {
                bCanKickRight = true;
            }
        }
        else if (pixelY >= 304) {
//            Serial.println("Hit Bottom");
            canRotate = false;
        }
    }, nextBitmask);

    //now that every piece part has been checked for interference with other pieces
    if(!bKickLeftNeeded && !bKickRightNeeded) {
        //we didn't need to kick left or right so all is good
        return true;
    }
    else if (bKickLeftNeeded && bCanKickLeft) {
        //we need to kick left but we can so all is still good
        return true;
    }
    else if (bKickRightNeeded && bCanKickRight) {
        //we need to kick right but can so all is still good
        return true;
    }
    else {
        //kick is needed in some direction but we can't without running into another piece. 
        return canRotate;
    }
}

void GamePiece::rotateCCW() {
    if(!_inPlay) { return; }

    //erase the piece at the old position
    //erase(gfxContext); 
    
    //figure out the new rotation state
    //_rotationState = (_rotationState + 3) % 4; //for CW
    _rotationState = (_rotationState + 1) % 4;

    kickRight(); //if needed
    kickLeft(); //if needed

    //now redraw
    //draw(gfxContext);
}

void GamePiece::moveRight() {
    if(_inPlay) {
        updateLocation(_column + 1, _row);
    }
}

void GamePiece::moveLeft() {
    if(_inPlay) {
        updateLocation(_column - 1, _row);
    }
}

void GamePiece::moveDown() {
    if(_inPlay) {
        updateLocation(_column, _row + 1);
    }
}

void GamePiece::updateLocation(int column, int row) {
    //update new position
    _column = column;
    _row = row;
}

void GamePiece::setInplay(bool inplay, uint16_t gameArea[][kCols], int rows) {

    if(!inplay) {
        forEachActiveBlock([&](int r, int c, int pixelX, int pixelY) {
            gameArea[(pixelY/BLOCKSIZE)][(pixelX/BLOCKSIZE)] = _color;
        });
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

    forEachActiveBlock([&](int r, int c, int pixelX, int pixelY) {
    
        while(pixelX < 0) {
            _column += 1;
            pixelX = (c * BLOCKSIZE) + (_column * BLOCKSIZE);
        }                
    });
}

void GamePiece::kickLeft() {

    forEachActiveBlock([&](int r, int c, int pixelX, int pixelY) {
    
        //piece is already rotated by this point
        while(pixelX > 144) {
            _column -= 1;
            pixelX = (c * BLOCKSIZE) + (_column * BLOCKSIZE);
        }                
    });
}

bool GamePiece::canKickLeft(uint16_t gameArea[][kCols]) {
    if(!_inPlay) { return false; }

    bool canKick = true;

    // Get the 16-bit integer for the next desired rotation state
    uint16_t nextBitmask = _shape.states[(_rotationState + 1) % 4];

    forEachActiveBlock([&](int r, int c, int pixelX, int pixelY) {
        //save off the _column
        uint32_t virtualColumn = _column;

        //now kick it
        while(pixelX > 144) {
            virtualColumn -= 1;
            pixelX = (c * BLOCKSIZE) + (virtualColumn * BLOCKSIZE);
//            Serial.println("We are seeing if we can kick left...");
        }   
    }, nextBitmask);

    //at this point we have to check every piece one more time now that its rotated and kicked.              
    forEachActiveBlock([&](int r, int c, int pixelX, int pixelY) {
        //save off the _column
        uint32_t virtualColumn = _column;

        //now just check for interference
        if(gameArea[(pixelY/BLOCKSIZE)][(pixelX/BLOCKSIZE)] > 0) {
//            Serial.println("Something interfers after kicking left...");
            canKick = false; //bail out we are overlapping another piece
        }
    }, nextBitmask);

    //check every piece part and looks clean
//    Serial.println("Nothing interfers! after kicking left...");
    return canKick;
}

bool GamePiece::canKickRight(uint16_t gameArea[][kCols]) {
    if(!_inPlay) { return false; }

    bool canKick = true;

    // Get the 16-bit integer for the next desired rotation state
    uint16_t nextBitmask = _shape.states[(_rotationState + 1) % 4];

    forEachActiveBlock([&](int r, int c, int pixelX, int pixelY) {
        //save off the _column
        uint32_t virtualColumn = _column;

        //now kick it
        while(pixelX < 0) {
            virtualColumn += 1;
            pixelX = (c * BLOCKSIZE) + (virtualColumn * BLOCKSIZE);
//            Serial.println("We are seeing if we can kick right...");
        }   
   }, nextBitmask);

   //at this point we have to check every piece one more time now that its rotated and kicked. 
    forEachActiveBlock([&](int r, int c, int pixelX, int pixelY) {
        //save off the _column
        uint32_t virtualColumn = _column;

        //now just check for interference
        if(gameArea[(pixelY/BLOCKSIZE)][(pixelX/BLOCKSIZE)] > 0) {
//            Serial.println("Something interfers after kicking left...");
            canKick = false; //bail out we are overlapping another piece
        }
    }, nextBitmask);

    return canKick;
}

int GamePiece::getColor() {
    return _color;
}

void GamePiece::setColor(int color) {
    _color = color;
}

int GamePiece::getBoarderColor() {
    return _borderColor;
}

void GamePiece::setBoarderColor(int color) {
    _borderColor = color;
}

void GamePiece::setColumn(int column) {
    _column = column;
}