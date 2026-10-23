#pragma once
#include <opencv2/opencv.hpp>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

class BaseFrameProvider {
  private:
    // producer-consumer synchronization
    std::atomic<bool> m_isCaptureRunning; 
    std::condition_variable m_frameReady; 
    std::condition_variable m_queueNotFull; 
    const size_t MAX_QUEUE_SIZE = 30; 

    std::thread m_videoProcessingThread; 
    std::queue<cv::Mat> m_frames;        
    std::mutex m_queueMutex;             

    void frameProducerTask();

  protected:
    // Descendants have to implement
    virtual cv::Mat grabSingleFrame() = 0;

  public:
    virtual ~BaseFrameProvider() = default;


    cv::Mat getNextFrame();
    void start();
    void stop();
};