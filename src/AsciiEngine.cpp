#include "AsciiEngine.h"
#include <iostream>

bool AsciiEngine::init(const std::string &videoPath, int targetWidth)
{
    m_cap.open(videoPath);
    if (!m_cap.isOpened())
    {
        std::cerr << "Chyba: Nelze otevrit video: " << videoPath << std::endl;
        return false;
    }
    m_width = targetWidth;

    // 3. Výpočet správné výšky (korekce obdélníkového fontu v terminálu)
    double origWidth = m_cap.get(cv::CAP_PROP_FRAME_WIDTH);
    double origHeight = m_cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    double aspectRatio = origWidth / origHeight;

    m_height = static_cast<int>((m_width / aspectRatio) * 0.5);

    // Prealocation
    //+1 because of /n at the end of each line
    int totalBufferSize = (m_width + 1) * m_height;
    m_frameBuffer.resize(totalBufferSize, ' ');

    for (int y = 0; y < m_height; ++y)
    {
        int newlineIndex = y * (m_width + 1) + m_width;
        m_frameBuffer[newlineIndex] = '\n';
    }

    return true;
}
void AsciiEngine::frameProducerTask()
{
    while (this->m_isRunning)
    {
        cv::Mat tmp;
        m_cap.read(tmp);
        if (tmp.empty())
        {
            m_isRunning = false;
            m_frameReady.notify_one();
            break;
        }
        std::unique_lock<std::mutex> uniqueLock(m_queueMutex);
        m_queueNotFull.wait(uniqueLock, [&]
                            { return m_frames.size() < MAX_QUEUE_SIZE || !m_isRunning; });
        m_frames.push(tmp);
        uniqueLock.unlock();
        m_frameReady.notify_one();
    }
}
void AsciiEngine::play()
{
    m_isRunning = true;
    // Start the background thread
    m_videoProcessingThread = std::thread(&AsciiEngine::frameProducerTask, this);
    cv::Mat grayFrame;
    cv::Mat resizedFrame;
    std::cout << "\x1b[2J";
    while (m_isRunning)
    {
        cv::Mat frame;
        { // this scope is only for the lock, so it will be released immediately after we get the frame
            std::unique_lock<std::mutex> uniqueLock(m_queueMutex);

            m_frameReady.wait(uniqueLock, [&]
                              { return !m_frames.empty() || !m_isRunning; });
            if (!m_isRunning && m_frames.empty())
            {
                break;
            }
            frame = m_frames.front();
            m_frames.pop();
            uniqueLock.unlock();
            m_queueNotFull.notify_one();
        }
        cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);
        cv::resize(grayFrame, resizedFrame, cv::Size(m_width, m_height));
        for (int y = 0; y < m_height; y++)
        {
            for (int x = 0; x < m_width; x++)
            {
                uchar brightness = resizedFrame.at<uchar>(y, x);
                int charIndex = (brightness * (m_asciiChars.length() - 1)) / 255;
                int bufferIndex = y * (m_width + 1) + x;
                m_frameBuffer[bufferIndex] = m_asciiChars[charIndex];
            }
        }
        std::cout << "\x1b[H";
        std::cout << m_frameBuffer;
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }

    if (m_videoProcessingThread.joinable())
    {
        m_videoProcessingThread.join();
    }
}