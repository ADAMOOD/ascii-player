#pragma once

#include "core/FrameBufferManager.h"
#include <thread>
#include <opencv2/dnn.hpp>

class ThreadedFrameProducer : public FrameBufferManager {
  private:
    std::thread m_videoProcessingThread;
    void frameProducerTask();

  protected:
    // Descendants have to implement
    virtual cv::Mat grabSingleFrame() = 0;

  public:
    void start() override;
    void stop() override;
};