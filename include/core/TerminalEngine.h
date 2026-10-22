#pragma once
#include <string>

#include <memory>

#include <opencv2/opencv.hpp>

#include "core/ConfigManager.h"
#include "core/StreamManager.h"
#include "core/InputHandler.h"
#include "ui/DisplayManager.h"


#include "core/IScreenCapture.h"
#include "core/CoreEngine.h"


/**
 * @class TerminalEngine
 * @brief engine responsible for reading video frames, processing them, and rendering ASCII art.
 * * Utilizes a multi-threaded producer-consumer pattern. The producer thread reads frames 
 * via OpenCV, while the main thread applies the active IRenderStrategy and outputs to the terminal.
 */
class TerminalEngine : public CoreEngine
{
public:
    TerminalEngine() = default;
    ~TerminalEngine() = default;

    bool init(const std::string &videoPath) ;
    /**
     * @brief initializes the engine for webcam input. Opens the default webcam and sets necessary properties.
     * 
     * @return true webcam successfully opened and initialized
     * @return false failed to access webcam or set properties
     */
    bool init() override; // for webcam

    /**
     * @brief the main loop of the engine. It continuously fetches frames from the queue, processes them using the active IRenderStrategy, and renders the ASCII art to the terminal. It also handles user input for dynamic property adjustments and strategy switching.
     * 
     */
    void play() override;

private:

    InputHandler m_inputHandler;             ///< Handels interactions from user when in active mode (playback)
    StreamManager m_streamManager;           ///< Provider of raw video frames
    DisplayManager m_displayManager;         ///<administrate terminal display and CV windows
        /**
     * @brief processes a single video frame using the active IRenderStrategy.
     * 
     * @param frame The input video frame to be processed.
     */
    void processFrameToBuffer(const cv::Mat &frame);
            /**
     * @brief responsible for synchronizing the frame output with the video's original framerate.
     * 
     */
    void syncFramerate();

    /**
     * @brief Reads user configuration, initializes the rendering strategy, and sets up terminal bounds.
     * @return true if configuration was successfully applied.
     */
    bool setupEngineConfigs();


    /**
     * @brief Checks for user input without blocking the main thread.
     *  Handles quitting, property navigation, and adjustments. Updates the active strategy's properties based on user input.
     * 
     */
    void checkUserInput();


};