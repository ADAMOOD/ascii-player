#include "core/FrameBufferManager.h"

cv::Mat FrameBufferManager::consumeFrame()
{
    cv::Mat frame;
    std::unique_lock<std::mutex> uniqueLock(m_queueMutex);

    bool gotFrame = m_frameReady.wait_for(uniqueLock, std::chrono::milliseconds(50), [&]
                                          { return !m_frames.empty() || !m_isCaptureRunning; });

    if (!gotFrame || (!m_isCaptureRunning && m_frames.empty()))
        return cv::Mat();

    frame = m_frames.front();
    m_frames.pop();
    uniqueLock.unlock();
    m_queueNotFull.notify_one();

    return frame;
}

