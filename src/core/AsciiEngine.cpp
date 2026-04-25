#include "core/AsciiEngine.h"
#include <iostream>
#include "strategies/PerceptualGrayscaleStrategy.h"
#include "strategies/NaiveGrayscaleStrategy.h"

bool AsciiEngine::init(const std::string &videoPath)
{
    m_cap.open(videoPath);
    if (!m_cap.isOpened())
    {
        std::cerr << "Chyba: Nelze otevrit video: " << videoPath << std::endl;
        return false;
    }
    // TODO reading config for strategy
    //  std::make_unique safely allocates the GrayscaleStrategy on the heap
    // and immediately wraps it in a unique_ptr to ensure memory safety from day one.
    this->setStrategy(std::make_unique<NaiveGrayscaleStrategy>());
    double origWidth = m_cap.get(cv::CAP_PROP_FRAME_WIDTH);
    double origHeight = m_cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    m_aspectRatio = origWidth / origHeight;
    m_width = 0;
    updateTerminalSize();
    return true;
}
void AsciiEngine::updateTerminalSize()
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    int termW = w.ws_col;
    int termH = w.ws_row - 1; // prevent the last line from scrolling when we print the buffer

    int newWidth = termW;
    int newHeight = static_cast<int>((newWidth / m_aspectRatio) * 0.5);

    // if the video is too tall for the terminal, we need to limit the height and recalculate the width
    if (newHeight > termH)
    {
        newHeight = termH;
        newWidth = static_cast<int>((newHeight / 0.5) * m_aspectRatio);
    }

    // if the size has changed, we need to update the buffer
    if (newWidth != m_width || newHeight != m_height)
    {
        m_width = newWidth;
        m_height = newHeight;

        int totalBufferSize = (m_width + 1) * m_height;
        m_frameBuffer.assign(totalBufferSize, ' ');

        for (int y = 0; y < m_height; ++y)
        {
            int newlineIndex = y * (m_width + 1) + m_width;
            m_frameBuffer[newlineIndex] = '\n';
        }
    }
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
    std::cout << "\x1b[2J\x1b[?25l";
    while (m_isRunning)
    {
        updateTerminalSize();

        cv::Mat frame = fetchFrameFromQueue();
        if (frame.empty())
            break;

        processFrameToBuffer(frame);

        renderBuffer();

        syncFramerate();

        // checkUserInput();
    }

    if (m_videoProcessingThread.joinable())
    {
        m_videoProcessingThread.join();
    }
    std::cout << "\x1b[?25h";
}
cv::Mat AsciiEngine::fetchFrameFromQueue()
{
    cv::Mat frame;
    std::unique_lock<std::mutex> uniqueLock(m_queueMutex);

    m_frameReady.wait(uniqueLock, [&]
                      { return !m_frames.empty() || !m_isRunning; });

    // Pokud se vypina a fronta je prazdna, vratime prazdny frame
    if (!m_isRunning && m_frames.empty())
        return cv::Mat();

    frame = m_frames.front();
    m_frames.pop();
    uniqueLock.unlock();
    m_queueNotFull.notify_one();

    return frame;
}
void AsciiEngine::setStrategy(std::unique_ptr<IRenderStrategy> newStrategy)
{
    // unique_ptr cannot be copied (to prevent double-free crashes).
    // std::move explicitly transfers ownership from 'newStrategy' to 'm_currentStrategy'.
    m_currentStrategy = std::move(newStrategy);
}
void AsciiEngine::processFrameToBuffer(const cv::Mat &frame)
{
    if (m_currentStrategy)
    {
        m_currentStrategy->render(frame, m_frameBuffer, m_width, m_height);
    }
}
void AsciiEngine::renderBuffer()
{
    std::cout << "\x1b[H" << m_frameBuffer;
}

void AsciiEngine::syncFramerate()
{
    // TODO

    std::this_thread::sleep_for(std::chrono::milliseconds(33));
}
void AsciiEngine::checkUserInput()
{
    // TODO

    if (std::cin.peek() != EOF)
    {
        char c;
        std::cin.get(c);
        if (c == 'q' || c == 'Q')
        {
            m_isRunning = false;
            m_frameReady.notify_one();
        }
    }
}