#pragma once


enum class InputAction {
    NONE,
    QUIT,
    PREV_PROPERTY,
    NEXT_PROPERTY,
    INCREASE_VALUE,
    DECREASE_VALUE,
    TOGGLE
};

class InputHandler{

    public:
    InputAction pollInput();
    /**
     * @brief Disables terminal line buffering and echoing.
     * * Ensures that keyboard input is read instantly without waiting for the Enter key
     *  and that key presses are not displayed in the terminal.
     *  This is essential for real-time interaction during video playback.
     */
    bool init();
    
    /**
     * @brief Restores the terminal to its default canonical mode.
     * * Re-enables line buffering and echoing so the terminal behaves normally 
     * after the engine finishes playback.
     */
    bool shutdown();

};