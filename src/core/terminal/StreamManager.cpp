#include "core/terminal/StreamManager.h"

#include <iostream>
#include "core/ConfigManager.h"
#include <opencv2/core/utils/logger.hpp>

bool StreamManager::init(const std::string &videoPath)
{
    m_isLiveStream = false;
    m_cap.open(videoPath);
    if (!m_cap.isOpened())
    {
        std::cerr << "[ERROR] Could not open video file: [" << videoPath << "]" << std::endl;
        return false;
    }
    return true;
}

bool StreamManager::init()
{
    m_isLiveStream = true;

    std::string camStr = ConfigManager::getValFromSettings("camera_index");
    int camIndex = 0;
    if (!camStr.empty())
    {
        try
        {
            camIndex = std::stoi(camStr);
        }
        catch (...)
        { // TODO
        }
    }

// multiplatform openingwebcam
#ifdef _WIN32
    m_cap.open(camIndex, cv::CAP_MSMF);
#elif __APPLE__
    m_cap.open(camIndex, cv::CAP_AVFOUNDATION);
#else
    // GStreamer pipeline for  Jetson MIPI CSI cam
    std::string pipeline = "nvarguscamerasrc ! video/x-raw(memory:NVMM), width=1280, height=720, format=(string)NV12, framerate=30/1 ! nvvidconv flip-method=0 ! video/x-raw, width=1280, height=720, format=(string)BGRx ! videoconvert ! video/x-raw, format=(string)BGR ! appsink";
    m_cap.open(pipeline, cv::CAP_GSTREAMER);
#endif

    if (!m_cap.isOpened())
    {
        std::cerr << "[ERROR] Could not open webcam. Check connection." << std::endl;
        return false;
    }

    //setting webcam properties
    #ifndef __linux__
    // setting webcam properties (only for standardí USB wbcams on  Win/Mac)
    m_cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
    m_cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    m_cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    m_cap.set(cv::CAP_PROP_BUFFERSIZE, 1);
#endif

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    return true;
}

cv::Mat StreamManager::grabSingleFrame() {
    cv::Mat tmp;
    m_cap.read(tmp);
    return tmp;
}

void StreamManager::getOriginalSize(double &outWidth, double &outHeight)
{
    outWidth = m_cap.get(cv::CAP_PROP_FRAME_WIDTH);
    outHeight = m_cap.get(cv::CAP_PROP_FRAME_HEIGHT);
}

double StreamManager::getStreamFps() const
{
    if(m_cap.isOpened())
    {
        return m_cap.get(cv::CAP_PROP_FPS);
    }
    return 0;
}
