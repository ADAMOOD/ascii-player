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
     * @brief producer task that continuously reads frames from the video source and pushes them into a thread-safe queue for processing.
     * 
     */
    void frameProducerTask();

    /**
     * @brief the main loop of the engine. It continuously fetches frames from the queue, processes them using the active IRenderStrategy, and renders the ASCII art to the terminal. It also handles user input for dynamic property adjustments and strategy switching.
     * 
     */
    void play();

private:
    cv::VideoCapture m_cap;                  ///< OpenCV video capture object (handles both files and webcam).
    std::vector<ImageUtils::Pixel> m_frameBuffer; ///< 1D array representing the 2D terminal screen.
    int m_width;                             ///< Current terminal width in characters.
    int m_height;                            ///< Current terminal height in characters.
    std::thread m_videoProcessingThread;     ///< Background thread executing the frame producer task.
    std::queue<cv::Mat> m_frames;            ///< Shared queue containing decoded video frames.
    std::mutex m_queueMutex;                 ///< Mutex to protect access to the frame queue.
    double m_aspectRatio;                    ///< Original aspect ratio of the video source.

    //producer-consumer synchronization
    std::atomic<bool> m_isRunning;           ///< Atomic flag indicating if the playback is currently active.
    std::condition_variable m_frameReady;    ///< Signaled when a new frame is added to the queue.
    std::condition_variable m_queueNotFull;  ///< Signaled when a frame is popped, meaning space is available.
    const size_t MAX_QUEUE_SIZE = 30;        ///< Maximum number of frames held in memory.

    bool m_isLiveStream = false;             ///< Flag indicating if the source is a live webcam feed.

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
     * @brief Polls the OS for the current terminal dimensions and adjusts the internal frame buffer.
     */
    void updateTerminalSize();
    /**
     * @brief Safely pops the oldest frame from the thread-safe queue. Waits if the queue is empty.
     * @return cv::Mat The fetched frame, or an empty matrix if playback stopped.
     */
    cv::Mat fetchFrameFromQueue();
    /**
     * @brief processes a single video frame using the active IRenderStrategy.
     * 
     * @param frame The input video frame to be processed.
     */
    void processFrameToBuffer(const cv::Mat &frame);
    /**
     * @brief Renders the current frame buffer to the terminal, applying ANSI color codes if enabled.
     *  Also handles the display of the HUD with active properties and highlights the selected one.
     * 
     */
    void renderBuffer();
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
    /**
     * @brief Renders the heads-up display at the bottom of the terminal, showing active properties of the current strategy.
     * 
     */
    void renderHUD();

    /**
     * @brief Disables terminal line buffering and echoing.
     * * Ensures that keyboard input is read instantly without waiting for the Enter key
     *  and that key presses are not displayed in the terminal.
     *  This is essential for real-time interaction during video playback.
     */
    void enableRawMode();

    /**
     * @brief Restores the terminal to its default canonical mode.
     * * Re-enables line buffering and echoing so the terminal behaves normally 
     * after the engine finishes playback.
     */
    void disableRawMode();
};