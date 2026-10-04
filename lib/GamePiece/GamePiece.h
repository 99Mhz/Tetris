/*  GamePiece.h
    Defines attributes and actions a game piece can perform while in play. 
    Author: John WIltse
    Date: 10/2026
    Revisions: 

    Open bugs: Finish canRotateCCW method. you can rotate a piece when next to another piece that should blocking it. 
*/

#ifndef GAMEPIECE_H
#define GAMEPIECE_H

//#include "Arduino.h" // Gives access to standard Arduino functions JIC
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ILI9341.h> // Hardware-specific library for ST7789
#include <array>

const int kCols = 10; // Must be a compile-time constant

struct Point {
    int x;
    int y;
};

//TODO: figure this out. its out of place for this class
struct PieceData {
    uint16_t states[4]; 
};


class GamePiece {

  private:
    bool _inPlay;
    uint16_t _color;
    uint16_t _borderColor = ILI9341_DARKGREY;
    uint16_t _boundingBoxSize = 4; //all pieces live in a 4x4 bounding box
    uint16_t _rotationState;
    uint32_t _column;
    uint32_t _row;
    PieceData _shape; //TODO: Piece _piece proper name is tetromino

  public:
   
    static PieceData oBlock;

    static PieceData sBlock;

    static PieceData zBlock;

    static PieceData lBlock;

    static PieceData jBlock;

    static PieceData iBlock;

    static PieceData tBlock;

    GamePiece(const PieceData& shape, int color, int column, int row, int rotation);

    void draw(Adafruit_ILI9341 &gfxContext);

    void rotateCCW(Adafruit_ILI9341 &gfxContext);

    void rotateCW(Adafruit_ILI9341 &gfxContext);

    void erase(Adafruit_ILI9341 &gfxContext);

    void updateLocation(Adafruit_ILI9341 &gfxContext, int column, int row);

    void moveRight(Adafruit_ILI9341 &gfxContext);

    void moveLeft(Adafruit_ILI9341 &gfxContext);

    void moveDown(Adafruit_ILI9341 &gfxContext);

    int getCurrentRow();

    int getCurrentColumn();

    bool canMoveLeft(uint16_t gameArea[][kCols], int rows);
    
    bool canMoveRight(uint16_t gameArea[][kCols], int rows);

    bool canMoveDown(uint16_t gameArea[][kCols], int rows);

    bool canRotateCCW(uint16_t gameArea[][kCols], int rows);

    void kickRight();

    void kickLeft();

    void setInplay(bool inplay, uint16_t gameArea[][kCols], int rows);

    bool inplay();

};

#endif