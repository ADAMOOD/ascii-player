#include "core/AsciiEngine.h"
#include <iostream>
#include "core/ConfigManager.h"
#include "strategies/StrategiesFactory.h"
#include "strategies/EdgeDetections/BaseEdgeDetectionStrategy.h"
#include "strategies/ImageUtilits.h"
#include <opencv2/core/utils/logger.hpp>

// --- OS dependent libraries  ---
#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
#else
    #include <termios.h>
    #include <unistd.h>
    #include <sys/ioctl.h>
#endif
// ------------------------------------------

void AsciiEngine::enableRawMode()
{
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#else
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~(ICANON | ECHO);
    term.c_cc[VMIN] = 0;
    term.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
#endif
}

void AsciiEngine::disableRawMode()
{
#ifndef _WIN32
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
#endif
}

bool AsciiEngine::setupEngineConfigs()
{
    auto strategy = ConfigManager::getValFromSettings("render_strategy");
    this->setStrategy(strategy);

    if (auto edgeStrategy = dynamic_cast<BaseEdgeDetectionStrategy *>(m_currentStrategy.get()))
    {
        std::string savedChar = ConfigManager::getValFromSettings("fill_char");
        char fill = savedChar.empty() ? ' ' : savedChar[0];
        edgeStrategy->setFillChar(fill);
    }

    m_menuStartIndex = 0;
    m_selectedPropertyIndex = 0;
    double origWidth = m_cap.get(cv::CAP_PROP_FRAME_WIDTH);
    double origHeight = m_cap.get(cv::CAP_PROP_FRAME_HEIGHT);

    if (origWidth <= 0 || origHeight <= 0)
    {
        origWidth = 640;
        origHeight = 480;
    }

    m_aspectRatio = origWidth / origHeight;
    m_width = 0;

    updateTerminalSize();
    return true;
}

bool AsciiEngine::init(const std::string &videoPath)
{
    m_isLiveStream = false;
    m_cap.open(videoPath);
    if (!m_cap.isOpened())
    {
        std::cerr << "[ERROR] Could not open video file: [" << videoPath <<"]"<< std::endl;
        return false;
    }
    return setupEngineConfigs();
}

bool AsciiEngine::init()
{
    m_isLiveStream = true;

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
    return setupEngineConfigs();
}

void AsciiEngine::updateTerminalSize()
{
    // --- multiplatform terminal size fetching ---
    //windows returns the size of the whole console buffer, so we have to calculate the actual visible area   
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int termW = csbi.srWindow.Right - csbi.srWindow.Left;
    int termH = csbi.srWindow.Bottom - csbi.srWindow.Top;
#else//linux ioctl gives us the visible area right away
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int termW = w.ws_col - 1;
    int termH = w.ws_row - 1;
#endif

    int newWidth = termW;
    int newHeight = static_cast<int>((newWidth / m_aspectRatio) * 0.5);

    if (newHeight > termH)
    {
        newHeight = termH;
        newWidth = static_cast<int>((newHeight / 0.5) * m_aspectRatio);
    }

    if (newWidth != m_width || newHeight != m_height)
    {
        m_width = newWidth;
        m_height = newHeight;
        m_frameBuffer.assign(m_width * m_height, {' ', {0, 0, 0}, {0, 0, 0}});
        std::cout << "\x1b[2J" << std::flush;
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
            if (m_isLiveStream)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }
            else
            {
                m_isRunning = false;
                m_frameReady.notify_one();
                break;
            }
        }
        std::unique_lock<std::mutex> uniqueLock(m_queueMutex);
        m_queueNotFull.wait(uniqueLock, [&] { return m_frames.size() < MAX_QUEUE_SIZE || !m_isRunning; });
        m_frames.push(tmp);
        uniqueLock.unlock();
        m_frameReady.notify_one();
    }
}

void AsciiEngine::play()
{
    enableRawMode();
    m_isRunning = true;
    m_videoProcessingThread = std::thread(&AsciiEngine::frameProducerTask, this);
    std::cout << "\x1b[2J\x1b[?25l";

    while (m_isRunning)
    {
        updateTerminalSize();
        cv::Mat frame = fetchFrameFromQueue();

        if (frame.empty())
        {
            if (!m_isRunning) break;
            checkUserInput();
            continue;
        }

        processFrameToBuffer(frame);
        renderBuffer();
        renderHUD();
        syncFramerate();
        checkUserInput();
    }

    disableRawMode();
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

    std::cout << "\x1b[?25h";
}

void AsciiEngine::renderHUD()
{
    if (m_activeProperties.empty() || !m_currentStrategy)
        return;

    std::cout << "\x1b[2K";

    if (m_selectedPropertyIndex < m_menuStartIndex)
    {
        m_menuStartIndex = m_selectedPropertyIndex;
    }

    int totalWidthToCursor = 0;
    for (int i = m_menuStartIndex; i <= m_selectedPropertyIndex; ++i)
    {
        std::string plainText = m_activeProperties[i].toString();
        totalWidthToCursor += plainText.length();

        if (totalWidthToCursor > m_width)
        {
            m_menuStartIndex++;
            i = m_menuStartIndex - 1;
            totalWidthToCursor = 0;
        }
    }

    int visibleChars = 0;
    for (size_t i = m_menuStartIndex; i < m_activeProperties.size(); ++i)
    {
        std::string plainText = m_activeProperties[i].toString();
        int propLength = plainText.length();

        if (visibleChars + propLength > m_width) break;
        
        visibleChars += propLength;
        if (i == static_cast<size_t>(m_selectedPropertyIndex))
        {
            std::cout << "\x1b[7m" << plainText << "\x1b[0m";
        }
        else
        {
            std::cout << plainText;
        }
    }
    std::cout << std::flush;
}

cv::Mat AsciiEngine::fetchFrameFromQueue()
{
    cv::Mat frame;
    std::unique_lock<std::mutex> uniqueLock(m_queueMutex);

    bool gotFrame = m_frameReady.wait_for(uniqueLock, std::chrono::milliseconds(50), [&] { return !m_frames.empty() || !m_isRunning; });

    if (!gotFrame || (!m_isRunning && m_frames.empty()))
        return cv::Mat();

    frame = m_frames.front();
    m_frames.pop();
    uniqueLock.unlock();
    m_queueNotFull.notify_one();

    return frame;
}

void AsciiEngine::setStrategy(std::string newStrategy)
{
    m_currentStrategy = std::move(StrategiesFactory::createStrategy(newStrategy));
    m_activeProperties = m_currentStrategy->getProperties();
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
    bool useColor = m_currentStrategy->getProperty("Use Color") > 0.5f;
    bool use8Bit = m_currentStrategy->getProperty("8-bit Colors") > 0.5f;
    int tolerance = static_cast<int>(m_currentStrategy->getProperty("Color Tolerance"));

    std::string frameOutput;
    frameOutput.reserve(m_width * m_height * 20);

    cv::Vec3b lastColor = cv::Vec3b(255, 255, 255);
    uchar last8BitIndex = 255;

    for (int y = 0; y < m_height; ++y)
    {
        for (int x = 0; x < m_width; ++x)
        {
            ImageUtils::Pixel p = m_frameBuffer[y * m_width + x];

            if (useColor)
            {
                if (use8Bit)
                {
                    uchar current8BitIndex = ImageUtils::get8BitAnsiIndex(p.fgColor);
                    if (current8BitIndex != last8BitIndex)
                    {
                        frameOutput += ImageUtils::get8BitAnsiCode(current8BitIndex);
                        last8BitIndex = current8BitIndex;
                    }
                }
                else
                {
                    if (ImageUtils::isColorDifferent(lastColor, p.fgColor, tolerance))
                    {
                        frameOutput += ImageUtils::getAnsiFgColor(p.fgColor);
                        lastColor = p.fgColor;
                    }
                }
            }
            frameOutput += p.symbol;
        }
        if (y < m_height - 1) frameOutput += "\n";
    }

    if (useColor) frameOutput += "\x1b[0m";
    frameOutput += "\n";
    std::cout << "\x1b[H" << frameOutput << std::flush;
}

void AsciiEngine::syncFramerate()
{
    // this will be implemented in the future to sync with the video's original framerate
    std::this_thread::sleep_for(std::chrono::milliseconds(33));
}

void AsciiEngine::checkUserInput()
{
    char c = 0;
    bool hasInput = false;

// --- MULTIPLATFORM ---
#ifdef _WIN32
    if (_kbhit())
    {
        c = _getch();
        hasInput = true;
    }
#else
    if (read(STDIN_FILENO, &c, 1) == 1)
    {
        hasInput = true;
    }
#endif
// ------------------------------------------

    if (hasInput)
    {
        if (c == 'q' || c == 'Q')
        {
            m_isRunning = false;
            m_frameReady.notify_one();
            m_queueNotFull.notify_one();
            return;
        }

        switch (c)
        {
        case 'a':
            if (m_selectedPropertyIndex > 0) m_selectedPropertyIndex--;
            break;
        case 'd':
            if (!m_activeProperties.empty() && m_selectedPropertyIndex < static_cast<int>(m_activeProperties.size()) - 1)
                m_selectedPropertyIndex++;
            break;
        case 'w':
        case 's':
            if (m_activeProperties.empty()) break;

            Property prop = m_activeProperties[m_selectedPropertyIndex];
            prop.ShiftedValue(c == 'w');
            m_currentStrategy->setProperty(prop);
            m_activeProperties = m_currentStrategy->getProperties();

            if (m_activeProperties.empty())
            {
                m_selectedPropertyIndex = 0;
            }
            else if (m_selectedPropertyIndex >= static_cast<int>(m_activeProperties.size()))
            {
                m_selectedPropertyIndex = m_activeProperties.size() - 1;
            }
            break;
        }
    }
}