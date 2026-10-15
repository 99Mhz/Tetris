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
    //msg = "Current Column after: " + std::to_string(piece.getCurrentColumn());
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

// Main test runner entry point
int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    RUN_TEST(test_kick_right_adjusts_column_when_out_of_bounds);
    RUN_TEST(test_kick_left_adjusts_column_when_out_of_bounds);
    
    return UNITY_END();
}