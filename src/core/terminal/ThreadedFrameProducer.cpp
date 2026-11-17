#include "core/terminal/ThreadedFrameProducer.h"

#include <chrono>

void ThreadedFrameProducer::start()
{
    if (m_videoProcessingThread.joinable())
        return;

    m_isCaptureRunning = true;
    m_videoProcessingThread = std::thread(&ThreadedFrameProducer::frameProducerTask, this);
}

void ThreadedFrameProducer::stop()
{
    {
        // to prevent lost wakeup
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_isCaptureRunning = false;
    }
    m_frameReady.notify_all();
    m_queueNotFull.notify_all();

    if (m_videoProcessingThread.joinable())
    {
        m_videoProcessingThread.join();
    }

    std::lock_guard<std::mutex> lock(m_queueMutex);
    while (!m_frames.empty())
    {
        m_frames.pop();
    }
}

void ThreadedFrameProducer::frameProducerTask()
{
    while (m_isCaptureRunning)
    {
        // descendants will pass
        cv::Mat tmp = this->grabSingleFrame();

        if (tmp.empty())
        {
            // if lag
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        std::unique_lock<std::mutex> uniqueLock(m_queueMutex);
        m_queueNotFull.wait(uniqueLock, [&] {
            return m_frames.size() < MAX_QUEUE_SIZE || !m_isCaptureRunning;
        });

        if (!m_isCaptureRunning)
            break;

        m_frames.push(std::move(tmp));
        uniqueLock.unlock();
        m_frameReady.notify_one();
    }
}