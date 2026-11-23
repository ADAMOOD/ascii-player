#include "core/FrameBufferManager.h"

FramePacket FrameBufferManager::consumeFrame()
{
    std::unique_lock<std::mutex> uniqueLock(m_queueMutex);

    bool gotFrame = m_frameReady.wait_for(uniqueLock, std::chrono::milliseconds(50), [&]
                                          { return !m_frames.empty() || !m_isCaptureRunning; });

    if (!gotFrame || (!m_isCaptureRunning && m_frames.empty()))
        return FramePacket{};

    FramePacket frame = m_frames.front();
    m_frames.pop();
    uniqueLock.unlock();
    m_queueNotFull.notify_one();

    return frame;
}

