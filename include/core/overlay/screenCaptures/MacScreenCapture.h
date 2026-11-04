#pragma once
#include <mutex>

#include "core/FrameBufferManager.h"

class MacScreenCapture : public FrameBufferManager {
  private:
    void* m_stream = nullptr;
    void* m_delegate = nullptr;
    std::mutex m_streamMutex; // to secure save and stop

  public:
    MacScreenCapture() : FrameBufferManager(2) {}
    ~MacScreenCapture();

    void enqueueFrame(cv::Mat frame);
    void start() override;
    void stop() override;
};