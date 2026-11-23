#include "core/terminal/ThreadedFrameProducer.h"

#include <chrono>

void ThreadedFrameProducer::setSaliencyEnabled(bool enabled) {
    m_saliencyEnabled.store(enabled);
}

bool ThreadedFrameProducer::getSaliencyEnabled() {
    return m_saliencyEnabled.load();
}

void ThreadedFrameProducer::start() {
    if (m_videoProcessingThread.joinable())
        return;

#ifdef USE_TENSORRT
        if (!m_saliencyModel.init("../model.engine")) {
            std::cerr << "[ERROR] Saliency model not loaded (../model.engine)\n";
            return;
        }
#endif

    m_isCaptureRunning = true;
    m_videoProcessingThread = std::thread(&ThreadedFrameProducer::frameProducerTask, this);
}

void ThreadedFrameProducer::stop() {
    {
        // to prevent lost wakeup
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_isCaptureRunning = false;
    }
    m_frameReady.notify_all();
    m_queueNotFull.notify_all();

    if (m_videoProcessingThread.joinable()) {
        m_videoProcessingThread.join();
    }

    std::lock_guard<std::mutex> lock(m_queueMutex);
    while (!m_frames.empty()) {
        m_frames.pop();
    }
}

void ThreadedFrameProducer::frameProducerTask() {
#ifdef USE_TENSORRT
    cudaSetDevice(0);
#endif
    while (m_isCaptureRunning) {
        // descendants will pass
        cv::Mat tmp = this->grabSingleFrame();
        if (tmp.empty()) {
            // if lag
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        cv::Mat mask;
        if (m_saliencyEnabled && m_saliencyModel.isReady()) {
            mask = m_saliencyModel.getSegmentationMask(tmp);
        }

        std::unique_lock<std::mutex> uniqueLock(m_queueMutex);
        m_queueNotFull.wait(uniqueLock,
                            [&] {
                                return m_frames.size() < MAX_QUEUE_SIZE || !m_isCaptureRunning;
                            });

        if (!m_isCaptureRunning)
            break;

        m_frames.push(FramePacket{std::move(tmp), std::move(mask)});
        uniqueLock.unlock();
        m_frameReady.notify_one();
    }
}