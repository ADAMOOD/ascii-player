#include "ui/DisplayManager.h"
#include <iostream>
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#endif

cv::Mat DisplayManager::drawDebugCanvas(const cv::Mat &debugFrame, const cv::Mat &originalFrame, const std::string &currentStrategy, const cv::Rect &winRect)
{
    cv::Mat canvas = cv::Mat::zeros(winRect.height, winRect.width, CV_8UC3);

    const int UI_RESERVED_SPACE = 60;

    int maxAvailableHeightPerImage = (winRect.height - UI_RESERVED_SPACE) / 2;
    if (maxAvailableHeightPerImage <= 0)
        return canvas;

    int targetW = winRect.width;
    int targetH = static_cast<int>(targetW / m_aspectRatio);

    if (targetH > maxAvailableHeightPerImage)
    {

        targetH = maxAvailableHeightPerImage;
        targetW = static_cast<int>(targetH * m_aspectRatio);
    }

    int offsetX = (winRect.width - targetW) / 2;

    cv::Mat resizedOriginal;
    cv::resize(originalFrame, resizedOriginal, cv::Size(targetW, targetH));
    resizedOriginal.copyTo(canvas(cv::Rect(offsetX, 0, targetW, targetH)));

    int arrowStartY = targetH + 10;
    int arrowEndY = winRect.height - targetH - 10;

    if (arrowEndY > arrowStartY)
    {
        cv::arrowedLine(canvas, cv::Point(winRect.width / 2, arrowStartY),
                        cv::Point(winRect.width / 2, arrowEndY),
                        cv::Scalar(0, 255, 255), 2);

        int baseline = 0;
        cv::Size textSize = cv::getTextSize(currentStrategy, cv::FONT_HERSHEY_DUPLEX, 0.6, 1, &baseline);

        cv::Point textOrg(20,
                          arrowStartY + (arrowEndY - arrowStartY) / 2 + textSize.height / 2);

        cv::putText(canvas, currentStrategy, textOrg,
                    cv::FONT_HERSHEY_DUPLEX, 0.6, cv::Scalar(255, 255, 255), 1);
    }
    cv::Mat rgbDebugFrame, rgbDebugFrameResized;

    if (debugFrame.channels() == 1)
    {
        cv::cvtColor(debugFrame, rgbDebugFrame, cv::COLOR_GRAY2BGR);
    }
    else
    {
        rgbDebugFrame = debugFrame;
    }

    cv::resize(rgbDebugFrame, rgbDebugFrameResized, cv::Size(targetW, targetH));
    rgbDebugFrameResized.copyTo(canvas(cv::Rect(offsetX, winRect.height - targetH, targetW, targetH)));

    return canvas;
}

void DisplayManager::updateDebugWindowPosition()
{
#ifdef _WIN32
    static int lastTermCols = 0;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int termCols = csbi.srWindow.Right - csbi.srWindow.Left + 1;

    if (termCols != lastTermCols && termCols > 0)
    {
        lastTermCols = termCols;
        HWND consoleHwnd = GetForegroundWindow();
        RECT consoleRect;

        if (consoleHwnd && GetWindowRect(consoleHwnd, &consoleRect))
        {
            int physConsoleW = consoleRect.right - consoleRect.left;
            int physConsoleH = consoleRect.bottom - consoleRect.top;
            float physCharW = (float)physConsoleW / termCols;
            int sidebarChars = termCols - m_width;

            if (sidebarChars > 5)
            {
                int targetWinW = static_cast<int>(sidebarChars * physCharW);
                int targetWinH = physConsoleH - 200;

                if (targetWinW < 150)
                    targetWinW = 150;
                if (targetWinH < 200)
                    targetWinH = 200;

                cv::resizeWindow("Debug", targetWinW, targetWinH);
                int targetX = consoleRect.right - targetWinW - 30;
                int targetY = consoleRect.top + 100;
                cv::moveWindow("Debug", targetX, targetY);
            }
        }
    }
#elif __APPLE__ || __linux__
    static bool initialized = false;

    if (!initialized)
    {
        int targetWinW = 350;
        int targetWinH = static_cast<int>((targetWinW / m_aspectRatio) * 2) + 60;

        cv::resizeWindow("Debug", targetWinW, targetWinH);
        cv::moveWindow("Debug", 900, 100);

        initialized = true;
    }
#endif
}

std::vector<ImageUtils::Pixel> &DisplayManager::getBuffer()
{
    return m_frameBuffer;
}

int DisplayManager::getWidth() const
{
    return m_width;
}

int DisplayManager::getHeight() const
{
    return m_height;
}

void DisplayManager::updateTerminalSize()
{
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int termW = csbi.srWindow.Right - csbi.srWindow.Left;
    int termH = csbi.srWindow.Bottom - csbi.srWindow.Top;
#else
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

void DisplayManager::renderBuffer(bool useColor, bool use8Bit, int tolerance)
{
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
        if (y < m_height - 1)
            frameOutput += "\n";
    }

    if (useColor)
        frameOutput += "\x1b[0m";
    frameOutput += "\n";
    std::cout << "\x1b[H" << frameOutput << std::flush;
}
void DisplayManager::printMonitoredStats()
{
    std::cout << "Engine Compute FPS:: " << (int)m_currentFps
              << " | Render Time: " << std::fixed << std::setprecision(2) << m_currentFrameMs << " ms";
}

void DisplayManager::renderHUD(std::vector<Property> active_properties, int selected_property_index, int menu_start_index)
{
    std::cout << "\x1b[2K";

    if (selected_property_index < menu_start_index)
    {
        menu_start_index = selected_property_index;
    }

    int totalWidthToCursor = 0;
    for (int i = menu_start_index; i <= selected_property_index; ++i)
    {
        std::string plainText = active_properties[i].toString();
        totalWidthToCursor += static_cast<int>(plainText.length());

        if (totalWidthToCursor > m_width)
        {
            menu_start_index++;
            i = menu_start_index - 1;
            totalWidthToCursor = 0;
        }
    }

    int visibleChars = 0;
    for (size_t i = menu_start_index; i < active_properties.size(); ++i)
    {
        std::string plainText = active_properties[i].toString();
        int propLength = static_cast<int>(plainText.length());

        if (visibleChars + propLength > m_width)
            break;

        visibleChars += propLength;
        if (i == static_cast<size_t>(selected_property_index))
        {
            std::cout << "\x1b[7m" << plainText << "\x1b[0m";
        }
        else
        {
            std::cout << plainText;
        }
    }
    printMonitoredStats();
    std::cout << std::flush;
}

void DisplayManager::setStatsData(double fps, double ms)
{
    m_currentFps = fps;
    m_currentFrameMs = ms;
}

void DisplayManager::showDebugWindow(const cv::Mat &debugFrame, const cv::Mat &originalFrame, const std::string currentStrategy)
{
    if (debugFrame.empty())
        return;

    cv::namedWindow("Debug", cv::WINDOW_NORMAL);
    // debug window always on the top layer
    cv::setWindowProperty("Debug", cv::WND_PROP_TOPMOST, 1);
    updateDebugWindowPosition();

    cv::Rect winRect = cv::getWindowImageRect("Debug");

#if defined(__APPLE__) || defined(__linux__)
    if (winRect.width <= 0 || winRect.height <= 0)
    {
        int fallbackW = 500;
        int fallbackH = static_cast<int>((fallbackW / m_aspectRatio) * 2)-50;
        winRect = cv::Rect(0, 0, fallbackW, fallbackH);
    }
#endif

    if (winRect.width > 0 && winRect.height > 0)
    {
        cv::Mat canvas = drawDebugCanvas(debugFrame, originalFrame, currentStrategy, winRect);
        cv::imshow("Debug", canvas);
    }
    else
    {
        cv::imshow("Debug", debugFrame);
    }

    cv::waitKey(1);
}

void DisplayManager::init(double original_aspectRatio)
{
    m_width = 0;
    m_aspectRatio = original_aspectRatio;
}
