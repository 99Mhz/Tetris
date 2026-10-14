/* 
    None of this works yet until I seperate game logic from UI updates. 
    TODO: remove the Adafruit libs from GamePiece.h
*/
#include <unity.h>
#include "GamePiece.h" // Include your game piece header

// This runs before every single test
void setUp(void) {
    // Optional: Reset state if needed
}

// This runs after every single test
void tearDown(void) {
    // Optional: Clean up if needed
}

// 1. Write a test case for kickRight
void test_kick_right_adjusts_column_when_out_of_bounds(void) {

    // Set up a state where the piece is artificially clipped past the left wall (pixelX < 0)
    //TODO: i'll need to recreate the color definitions to be able to get rid of the Adafruit libs
    GamePiece piece = GamePiece(GamePiece::oBlock, ILI9341_YELLOW, -1, 3, 0);
    
    // Run the function we are testing
    piece.kickRight();

    // Assert that the piece correctly kicked itself back into bounds
    TEST_ASSERT_GREATER_THAN_INT(-1, piece.getCurrentColumn());
}

// 2. Main test runner entry point
int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    RUN_TEST(test_kick_right_adjusts_column_when_out_of_bounds);
    
    return UNITY_END();
}