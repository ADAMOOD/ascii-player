#include "core/StreamManager.h"

#include <iostream>
#include "core/ConfigManager.h"
#include <opencv2/core/utils/logger.hpp>

// TODO prendat tohle do TerminalView nebo inpout manageru
/*
// --- OS dependent libraries  ---
#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#endif
*/
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
    int camIndex = 0; // Výchozí pojistka
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
// --- multiplatform opening of webcam ---
#ifdef _WIN32
    m_cap.open(0, cv::CAP_MSMF);
#else
    m_cap.open(0, cv::CAP_V4L2);
#endif
    // ---------------------------------------------

    if (!m_cap.isOpened())
    {
        std::cerr << "[ERROR] Could not open webcam. Check connection." << std::endl;
        return false;
    }
    // --- setting webcam properties ---
    m_cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
    m_cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    m_cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    m_cap.set(cv::CAP_PROP_FPS, 30);
    m_cap.set(cv::CAP_PROP_BUFFERSIZE, 1);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    return true;
}

void StreamManager::start()
{
    m_isCaptureRunning = true;
    m_videoProcessingThread = std::thread(&StreamManager::frameProducerTask, this);
}

cv::Mat StreamManager::getNextFrame()
{
    cv::Mat frame;
    std::unique_lock<std::mutex> uniqueLock(m_queueMutex);

    bool gotFrame = m_frameReady.wait_for(uniqueLock, std::chrono::milliseconds(50), [&]
                                          { return !m_frames.empty() || !m_isCaptureRunning; });

    if (!gotFrame || (!m_isCaptureRunning && m_frames.empty()))
        return cv::Mat();

    frame = m_frames.front();
    m_frames.pop();
    uniqueLock.unlock();
    m_queueNotFull.notify_one();

    return frame;
}

void StreamManager::getOriginalSize(double &outWidth, double &outHeight)
{
    outWidth = m_cap.get(cv::CAP_PROP_FRAME_WIDTH);
    outHeight = m_cap.get(cv::CAP_PROP_FRAME_HEIGHT);
}

void StreamManager::stop()
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

void StreamManager::frameProducerTask()
{
    while (this->m_isCaptureRunning)
    {
        cv::Mat tmp;
        m_cap.read(tmp);
        if (tmp.empty())
        {
            if (m_isLiveStream)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }
            else
            {
                m_isCaptureRunning = false;
                m_frameReady.notify_one();
                break;
            }
        }
        std::unique_lock<std::mutex> uniqueLock(m_queueMutex);
        m_queueNotFull.wait(uniqueLock, [&]
                            { return m_frames.size() < MAX_QUEUE_SIZE || !m_isCaptureRunning; });
        m_frames.push(tmp);
        uniqueLock.unlock();
        m_frameReady.notify_one();
    }
}
