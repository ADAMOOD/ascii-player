#pragma once

#include "core/FrameBufferManager.h"
#include "core/SaliencyModel.h"
#include <atomic>
#include <thread>
#include <opencv2/dnn.hpp>


class ThreadedFrameProducer : public FrameBufferManager {
private:
    std::thread m_videoProcessingThread;
    void frameProducerTask();
    SaliencyModel m_saliencyModel;
    std::atomic<bool> m_saliencyEnabled;

protected:
    // Descendants have to implement
    virtual cv::Mat grabSingleFrame() = 0;

public:

    void start() override;
    void stop() override;
};