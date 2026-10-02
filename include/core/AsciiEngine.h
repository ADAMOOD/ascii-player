#pragma once
#include <string>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>

#include <opencv2/opencv.hpp>
#include "strategies/IRenderStrategy.h"
#include "strategies/ImageUtilits.h"
#include "core/StreamManager.h"
#include "core/InputHandler.h"
#include "ui/DisplayManager.h"


/**
 * @class AsciiEngine
 * @brief Core engine responsible for reading video frames, processing them, and rendering ASCII art.
 * * Utilizes a multi-threaded producer-consumer pattern. The producer thread reads frames 
 * via OpenCV, while the main thread applies the active IRenderStrategy and outputs to the terminal.
 */
class AsciiEngine
{
public:
    AsciiEngine() = default;
    ~AsciiEngine() = default;

    bool init(const std::string &videoPath);
    /**
     * @brief initializes the engine for webcam input. Opens the default webcam and sets necessary properties.
     * 
     * @return true webcam successfully opened and initialized
     * @return false failed to access webcam or set properties
     */
    bool init(); // pro webkameru

    /**
     * @brief the main loop of the engine. It continuously fetches frames from the queue, processes them using the active IRenderStrategy, and renders the ASCII art to the terminal. It also handles user input for dynamic property adjustments and strategy switching.
     * 
     */
    void play();

private:
    InputHandler m_inputHandler;             ///< Handels interactions from user when in active mode (playback)
    StreamManager m_streamManager;           ///< Provider of raw video frames
    DisplayManager m_displayManager;         ///<administrate terminal display and CV windows


    std::atomic<bool> m_isEngineRunning;      ///< Atomic flag indicating if the engine is currently active.

    std::unique_ptr<IRenderStrategy> m_currentStrategy; ///< Exclusively owned active rendering strategy.


    std::vector<Property> m_activeProperties;///< List of adjustable properties for the active strategy.
    int m_selectedPropertyIndex = 0;         ///< Index of the currently highlighted property in the HUD.
    int m_menuStartIndex = 0;                ///< Index of the first visible property in the HUD (for scrolling).

    /**
     * @brief Reads user configuration, initializes the rendering strategy, and sets up terminal bounds.
     * @return true if configuration was successfully applied.
     */
    bool setupEngineConfigs();
    /**
     * @brief processes a single video frame using the active IRenderStrategy.
     * 
     * @param frame The input video frame to be processed.
     */
    void processFrameToBuffer(const cv::Mat &frame);
    /**
     * @brief not implemented yet, but will be responsible for synchronizing the frame output with the video's original framerate.
     * 
     */
    void syncFramerate();
    /**
     * @brief Checks for user input without blocking the main thread.
     *  Handles quitting, property navigation, and adjustments. Updates the active strategy's properties based on user input.
     * 
     */
    void checkUserInput();
    /**
     * @brief Set the Strategy object to be used for rendering frames.
     *  using unique_ptr for automatic memory management.
     *  Also updates the active properties list for the HUD.
     * 
     * @param newStrategy Name of the new strategy to switch to. The StrategiesFactory will create the appropriate object.
     */
    void setStrategy(std::string newStrategy);

};