#pragma once
#include <opencv2/opencv.hpp>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>

class FrameBufferManager {

  protected:
    // producer-consumer synchronization
    std::atomic<bool> m_isCaptureRunning;
    std::condition_variable m_queueNotFull;
    const size_t MAX_QUEUE_SIZE = 30;
    std::condition_variable m_frameReady;
    std::queue<cv::Mat> m_frames;
    std::mutex m_queueMutex;

  public:
    virtual ~FrameBufferManager() = default;

    /**
     * @brief Safely pops the oldest frame from the thread-safe queue. Waits if the queue is empty.
     * @return cv::Mat The fetched frame, or an empty matrix if playback stopped.
     */
    cv::Mat consumeFrame();
    virtual void start() = 0;
    virtual void stop() = 0;
};