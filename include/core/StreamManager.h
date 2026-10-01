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

class StreamManager
{
private:
    cv::VideoCapture m_cap;                  ///< OpenCV video capture object (handles both files and webcam).
    std::thread m_videoProcessingThread;     ///< Background thread executing the frame producer task.
    std::queue<cv::Mat> m_frames;            ///< Shared queue containing decoded video frames.
    std::mutex m_queueMutex;                 ///< Mutex to protect access to the frame queue.

    //producer-consumer synchronization
    std::atomic<bool> m_isCaptureRunning;           ///< Atomic flag indicating if the playback is currently active.
    std::condition_variable m_frameReady;    ///< Signaled when a new frame is added to the queue.
    std::condition_variable m_queueNotFull;  ///< Signaled when a frame is popped, meaning space is available.
    const size_t MAX_QUEUE_SIZE = 30;        ///< Maximum number of frames held in memory.

    bool m_isLiveStream = false;             ///< Flag indicating if the source is a live webcam feed.

        /**
     * @brief producer task that continuously reads frames from the video source and pushes them into a thread-safe queue for processing.
     * 
     */
    void frameProducerTask();

public:
bool init();
bool init(const std::string &videoPath);
void start();
void stop();
cv::Mat getNextFrame();
void getOriginalSize(double &outWidth, double &outHeight);

};
