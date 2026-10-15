/* 
    Unit tests for Tetris. Currently all we can test is methods in GamePiece.h
    Need to refactor entire main.cpp into testable parts to go further
    Author: John Wiltse
    Date: 10/14/2026

    test with pio test -e native -v
*/
#include <unity.h>
#include <string>
#include "GamePiece.h" 

// This runs before every single test
void setUp(void) {
    // Optional: Reset state if needed
}

// This runs after every single test
void tearDown(void) {
    // Optional: Clean up if needed
}

//test case for kickRight
void test_kick_right_adjusts_column_when_out_of_bounds(void) {

    // Set up a state where the piece is artificially clipped past the left wall (pixelX < 0)
    GamePiece piece = GamePiece(GamePiece::lBlock, 0xFFE0, -1, 3, 1);

    // Run the function we are testing
    piece.kickRight();

    //messages in test run with -v flag
    //std::string msg = "Current Column after: " + std::to_string(piece.getCurrentColumn());
    //TEST_MESSAGE(msg.c_str()); // This safely outputs to the test window!

    bool anyBlockOutOfBounds = false;   
    piece.forEachActiveBlock([&](int r, int c, int pixelX, int pixelY) {
        if (pixelX < 0) {
            anyBlockOutOfBounds = true;
        }
    });

    // Assert that the piece correctly kicked itself back into bounds
    //TEST_ASSERT_GREATER_THAN_INT(-1, piece.getCurrentColumn());
    TEST_ASSERT_FALSE(anyBlockOutOfBounds);
}

//test case for kickRight
void test_kick_left_adjusts_column_when_out_of_bounds(void) {

    // Set up a state where the piece is artificially clipped past the right wall (pixelX < 0)
    GamePiece piece = GamePiece(GamePiece::lBlock, 0xFFE0, 8, 3, 3); //From rotation 2 to 3 pushes the tail of the L out beyond 9

    piece.kickLeft();

    bool anyBlockOutOfBounds = false;
    piece.forEachActiveBlock([&](int r, int c, int pixelX, int pixelY) {
        if (pixelX < 9) {
            anyBlockOutOfBounds = true;
        }
    });

    TEST_ASSERT_FALSE(anyBlockOutOfBounds);
}

//test case for kickRight
void test_rotate_t_middle_with_one_to_the_left(void) {

    /*
    -------------
    |   | T |   |
    -------------
    | X | T | T |
    -------------
    |   | T |   |
    -------------

    then rotate T CCW

    -------------
    |   | T |   |
    -------------
    | T | T | T |
    -------------
    |   |   |   |
    -------------

    Should not be possible but wipes out rendering of X, 
    and will keep faling if nothing is under

    I does this too on rotate on third block up. probably others!

    */

    const uint8_t kGameRowCount = 20;

    // Setup a gameArea 20 rows 10 columns
    uint16_t testGameArea[20][10] = {0};

    // Place X blocker piece tile on the board
    testGameArea[4][3] = 1; // 1 represents a settled block color/ID
    testGameArea[5][3] = 1; // 1 represents a settled block color/ID
    testGameArea[6][3] = 1; // 1 represents a settled block color/ID

    // Set up a T with middle tile just to the right. A rotate will push top leg into the blocker
    int column = 3;
    int row = 4;
    int rotation = 0;   //will get next bitmask of 1 Middle leg of the T should be in 5,3
    int color = 0xFFE0;
    GamePiece piece = GamePiece(GamePiece::tBlock, color, column, row, rotation); //From rotation 3 to 0 pushes the top tile into X

    bool rotateAllowed = piece.canRotateCCW(testGameArea, kGameRowCount);

    std::string msg = "";
    piece.forEachActiveBlock([&](int r, int c, int pixelX, int pixelY) {
        msg = "Color at [" 
        + std::to_string(pixelY/16) 
        + "][" 
        + std::to_string(pixelX/16) 
        + "]: "
        + std::to_string(testGameArea[pixelY/16][pixelX/16]);
        TEST_MESSAGE(msg.c_str());
    });

    TEST_ASSERT_FALSE(rotateAllowed);
}


//test case for kickRight
void test_move_t_to_the_left_with_blocker(void) {

    /*
    T rotate state 3 X is blocking
    -------------
    |   | T |   |
    -------------
    | X | T | T |
    -------------
    |   | T |   |
    -------------
    */

    const uint8_t kGameRowCount = 20;

    // Setup a gameArea 20 rows 10 columns
    uint16_t testGameArea[20][10] = {0};

    // Place X blocking piece on the board
    testGameArea[5][3] = 1; // 1 represents a settled block color/ID

    //set up a T just to the right of the blocker. blocking middle tile
    GamePiece piece = GamePiece(GamePiece::tBlock, 0xFFE0, 3, 4, 3); //From rotation 3 to 0 pushes the top tile into X

    bool moveAllowed = piece.canMoveLeft(testGameArea, kGameRowCount);


    TEST_ASSERT_FALSE(moveAllowed);
}


// Main test runner entry point
int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    RUN_TEST(test_kick_right_adjusts_column_when_out_of_bounds);
    RUN_TEST(test_kick_left_adjusts_column_when_out_of_bounds);
    RUN_TEST(test_rotate_t_middle_with_one_to_the_left);
    RUN_TEST(test_move_t_to_the_left_with_blocker);

    return UNITY_END();
}