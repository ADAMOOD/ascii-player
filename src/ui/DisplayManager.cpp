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


void DisplayManager::init(double original_aspectRatio)
{
    m_width = 0;
    m_aspectRatio = original_aspectRatio;
}
