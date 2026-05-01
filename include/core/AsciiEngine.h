#pragma once
#include <opencv2/opencv.hpp>
#include <string>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>
#include <sys/ioctl.h>
#include <unistd.h>

#include "strategies/IRenderStrategy.h"


class AsciiEngine
{
public:
    AsciiEngine() = default;
    ~AsciiEngine() = default;

    /**
     * @brief Inictializace AsciiEngine pro prehravani videa jako ASCII art v terminalu.
     * * This method opens the video file, calculates the correct height based on the target width and the aspect ratio of the original video.ion for terminal fonts, and prepares the buffer for storing ASCII characters.
     * * @param videoPath Path to the source video (e.g. "video.mp4").
     * @return true If the video was successfully opened and the engine initialized.
     * @return false If there was an error opening the video file or initializing the engine.
     */
    bool init(const std::string &videoPath);

    void frameProducerTask();
    void play();

private:
    cv::VideoCapture m_cap;
    std::string m_frameBuffer;
    int m_width;
    int m_height;
    std::thread m_videoProcessingThread;
    std::queue<cv::Mat> m_frames;
    std::mutex m_queueMutex;

    std::condition_variable m_frameDone;
    std::atomic<bool> m_isRunning;
    std::condition_variable m_frameReady;
    std::condition_variable m_queueNotFull;
    const size_t MAX_QUEUE_SIZE = 30;

    // Smart pointer ensuring EXCLUSIVE ownership of the rendering strategy.
    // Automatically calls 'delete' when AsciiEngine is destroyed or when the strategy changes.
    // Guarantees zero memory leaks without manual memory management.
    std::unique_ptr<IRenderStrategy> m_currentStrategy;
    double m_aspectRatio;

    void updateTerminalSize();
    cv::Mat fetchFrameFromQueue();
    void processFrameToBuffer(const cv::Mat &frame);
    void renderBuffer();
    void syncFramerate();
    void checkUserInput();
    void setStrategy(std::string newStrategy);
};