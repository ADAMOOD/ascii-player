#include "ui/DisplayManager.h"
#include <iostream>
// --- OS dependent libraries  ---
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
    int imgH = static_cast<int>(winRect.width / m_aspectRatio);

    if (imgH * 2 > winRect.height)
    {
        imgH = winRect.height / 2;
    }

    cv::Mat resizedOriginal;
    cv::resize(originalFrame, resizedOriginal, cv::Size(winRect.width, imgH));
    resizedOriginal.copyTo(canvas(cv::Rect(0, 0, winRect.width, imgH)));

    int arrowStartY = imgH + 10;
    int arrowEndY = winRect.height - imgH - 10;

    if (arrowEndY > arrowStartY)
    {
        cv::arrowedLine(canvas, cv::Point(winRect.width / 2, arrowStartY),
                        cv::Point(winRect.width / 2, arrowEndY),
                        cv::Scalar(0, 255, 255), 2);

        cv::putText(canvas, currentStrategy,
                    cv::Point(10, arrowStartY + (arrowEndY - arrowStartY) / 2),
                    cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 255), 1);
    }

    cv::Mat rgbDebugFrame, rgbDebugFrameResized;
    cv::cvtColor(debugFrame, rgbDebugFrame, cv::COLOR_GRAY2BGR);
    cv::resize(rgbDebugFrame, rgbDebugFrameResized, cv::Size(winRect.width, imgH), 0, 0);
    rgbDebugFrameResized.copyTo(canvas(cv::Rect(0, winRect.height - imgH, winRect.width, imgH)));

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
                int targetWinH = physConsoleH - 60;

                if (targetWinW < 150) targetWinW = 150;
                if (targetWinH < 200) targetWinH = 200;

                cv::resizeWindow("Debug", targetWinW, targetWinH);
                int targetX = consoleRect.right - targetWinW - 30;
                int targetY = consoleRect.top + 30;
                cv::moveWindow("Debug", targetX, targetY);
            }
        }
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
    // --- multiplatform terminal size fetching ---
    // windows returns the size of the whole console buffer, so we have to calculate the actual visible area
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int termW = csbi.srWindow.Right - csbi.srWindow.Left;
    int termH = csbi.srWindow.Bottom - csbi.srWindow.Top;
#else // linux ioctl gives us the visible area right away
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
    std::cout << std::flush;
}

void DisplayManager::showDebugWindow(const cv::Mat &debugFrame, const cv::Mat &originalFrame, const std::string currentStrategy)
{
    if (debugFrame.empty()) return;

    cv::namedWindow("Debug", cv::WINDOW_NORMAL);

    updateDebugWindowPosition();

    cv::Rect winRect = cv::getWindowImageRect("Debug");
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
