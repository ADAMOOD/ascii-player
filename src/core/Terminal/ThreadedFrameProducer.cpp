#include "core/terminal/ThreadedFrameProducer.h"


void ThreadedFrameProducer::start()
{
    m_isCaptureRunning = true;
    m_videoProcessingThread = std::thread(&ThreadedFrameProducer::frameProducerTask, this);
}
void ThreadedFrameProducer::stop()
{
    m_isCaptureRunning = false;
    m_frameReady.notify_one();
    m_queueNotFull.notify_one();
    if (m_videoProcessingThread.joinable())
    {
        m_videoProcessingThread.join();
    }

    std::unique_lock<std::mutex> lock(m_queueMutex);
    while (!m_frames.empty())
    {
        m_frames.pop();
    }
    lock.unlock();
}

void ThreadedFrameProducer::frameProducerTask()
{
    while (m_isCaptureRunning)
    {
        // decendants will pass
        cv::Mat tmp = this->grabSingleFrame(); 

        if (tmp.empty())
        {
            // to ensure lag
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        std::unique_lock<std::mutex> uniqueLock(m_queueMutex);
        m_queueNotFull.wait(uniqueLock, [&] { 
            return m_frames.size() < MAX_QUEUE_SIZE || !m_isCaptureRunning; 
        });
        
        if (!m_isCaptureRunning) break;

        m_frames.push(tmp);
        uniqueLock.unlock();
        m_frameReady.notify_one();
    }
}