#pragma once

// Standardní C++ knihovny potřebné pro proměnné ve třídě
#include <string>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>

// OpenCV a vlastní hlavičky
#include <opencv2/opencv.hpp>
#include "strategies/IRenderStrategy.h"
#include "strategies/ImageUtilits.h" // Pro ImageUtils::Pixel

class AsciiEngine
{
public:
    AsciiEngine() = default;
    ~AsciiEngine() = default;

    bool init(const std::string &videoPath);
    bool init(); // pro webkameru

    void frameProducerTask();
    void play();

private:
    cv::VideoCapture m_cap;
    std::vector<ImageUtils::Pixel> m_frameBuffer;
    int m_width;
    int m_height;
    std::thread m_videoProcessingThread;
    std::queue<cv::Mat> m_frames;
    std::mutex m_queueMutex;

    std::condition_variable m_frameDone;
    std::atomic<bool> m_isRunning;
    std::condition_variable m_frameReady;
    std::condition_variable m_queueNotFull;
    const size_t MAX_QUEUE_SIZE = 30;

    bool m_isLiveStream = false;

    std::unique_ptr<IRenderStrategy> m_currentStrategy;
    double m_aspectRatio;

    std::vector<Property> m_activeProperties;
    int m_selectedPropertyIndex = 0;
    int m_menuStartIndex = 0;

    bool setupEngineConfigs();
    void updateTerminalSize();
    cv::Mat fetchFrameFromQueue();
    void processFrameToBuffer(const cv::Mat &frame);
    void renderBuffer();
    void syncFramerate();
    void checkUserInput();
    void setStrategy(std::string newStrategy);
    void renderHUD();
};