/*  GamePiece.h
    Defines attributes and actions a game piece can perform while in play. 
    Author: John WIltse
    Date: 10/2026
    Revisions: 

    Open bugs: Finish canRotateCCW method. you can rotate a piece when next to another piece that should blocking it. 
*/

#ifndef GAMEPIECE_H
#define GAMEPIECE_H

//for unit testing
#include <cstdint>
#include <functional>

//TODO: Move all this to main.cpp to start writing tests!
//#include <Adafruit_GFX.h>    // Core graphics library
//#include <Adafruit_ILI9341.h> // Hardware-specific library for ST7789
//#include <Arduino.h>

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
    uint16_t _borderColor = 0x7BEF; //DARK_GREY
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

    void forEachActiveBlock(std::function<void(int r, int c, int pixelX, int pixelY)> action, uint16_t bitmask = 0);

    void rotateCCW();

    void updateLocation(int column, int row);

    void moveRight();

    void moveLeft();

    void moveDown();

    int getCurrentRow();

    int getCurrentColumn();

    bool canMoveLeft(uint16_t gameArea[][kCols], int rows);
    
    bool canMoveRight(uint16_t gameArea[][kCols], int rows);

    bool canMoveDown(uint16_t gameArea[][kCols], int rows);

    bool canRotateCCW(uint16_t gameArea[][kCols], int rows);

    void kickRight();

    void kickLeft();

    bool canKickLeft(uint16_t gameArea[][kCols]);

    bool canKickRight(uint16_t gameArea[][kCols]);

    void setInplay(bool inplay, uint16_t gameArea[][kCols], int rows);

    bool inplay();

    int getColor();

    int getBoarderColor();

    void setColor(int color);

    void setBoarderColor(int color);

    void setColumn(int column);

    void setInplay(int column);
};

#endif