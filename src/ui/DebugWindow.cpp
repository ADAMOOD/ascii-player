#include "ui/DebugWindow.h"
#include <iostream>

bool DebugWindow::init(double source_aspectRatio) {
    if (source_aspectRatio >= (double) 9 / 20 && source_aspectRatio <= (double) 32 / 9) {
        m_aspectRatio = source_aspectRatio;
        return true;
    }
    std::cerr << "[DEBUG WINDOW] invalid aspect ration to init\n";
    return false;
}

cv::Mat DebugWindow::drawDebugCanvas(
    const cv::Mat& debugFrame,
    const cv::Mat& originalFrame,
    const std::string& currentStrategy,
    const cv::Rect& winRect
) {
    cv::Mat canvas = cv::Mat::zeros(winRect.height, winRect.width, CV_8UC3);

    const int UI_RESERVED_SPACE = 60;

    int maxAvailableHeightPerImage = (winRect.height - UI_RESERVED_SPACE) / 2;
    if (maxAvailableHeightPerImage <= 0)
        return canvas;

    int targetW = winRect.width;
    int targetH = static_cast<int>(targetW / m_aspectRatio);

    if (targetH > maxAvailableHeightPerImage) {

        targetH = maxAvailableHeightPerImage;
        targetW = static_cast<int>(targetH * m_aspectRatio);
    }

    int offsetX = (winRect.width - targetW) / 2;

    cv::Mat resizedOriginal;
    cv::resize(originalFrame, resizedOriginal, cv::Size(targetW, targetH));
    resizedOriginal.copyTo(canvas(cv::Rect(offsetX, 0, targetW, targetH)));

    int arrowStartY = targetH + 10;
    int arrowEndY = winRect.height - targetH - 10;

    if (arrowEndY > arrowStartY) {
        cv::arrowedLine(
            canvas,
            cv::Point(winRect.width / 2, arrowStartY),
            cv::Point(winRect.width / 2, arrowEndY),
            cv::Scalar(0, 255, 255),
            2
        );

        int baseline = 0;
        cv::Size textSize =
            cv::getTextSize(currentStrategy, cv::FONT_HERSHEY_DUPLEX, 0.6, 1, &baseline);

        cv::Point textOrg(20, arrowStartY + (arrowEndY - arrowStartY) / 2 + textSize.height / 2);

        cv::putText(
            canvas,
            currentStrategy,
            textOrg,
            cv::FONT_HERSHEY_DUPLEX,
            0.6,
            cv::Scalar(255, 255, 255),
            1
        );
    }
    cv::Mat rgbDebugFrame, rgbDebugFrameResized;

    if (debugFrame.channels() == 1) {
        cv::cvtColor(debugFrame, rgbDebugFrame, cv::COLOR_GRAY2BGR);
    } else {
        rgbDebugFrame = debugFrame;
    }

    cv::resize(rgbDebugFrame, rgbDebugFrameResized, cv::Size(targetW, targetH));
    rgbDebugFrameResized.copyTo(
        canvas(cv::Rect(offsetX, winRect.height - targetH, targetW, targetH))
    );

    return canvas;
}

void DebugWindow::updateDebugWindowPosition() {
#ifdef _WIN32
    static int lastTermCols = 0;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int termCols = csbi.srWindow.Right - csbi.srWindow.Left + 1;

    if (termCols != lastTermCols && termCols > 0) {
        lastTermCols = termCols;
        HWND consoleHwnd = GetForegroundWindow();
        RECT consoleRect;

        if (consoleHwnd && GetWindowRect(consoleHwnd, &consoleRect)) {
            int physConsoleW = consoleRect.right - consoleRect.left;
            int physConsoleH = consoleRect.bottom - consoleRect.top;
            float physCharW = (float) physConsoleW / termCols;
            int sidebarChars = termCols - m_width;

            if (sidebarChars > 5) {
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

    if (!initialized) {
        int targetWinW = 350;
        int targetWinH = static_cast<int>((targetWinW / m_aspectRatio) * 2);

        cv::resizeWindow("Debug", targetWinW, targetWinH);
        m_winPosX = 900;
        m_winPosY = 100;
        cv::moveWindow("Debug", m_winPosX, m_winPosY);

        initialized = true;
    }
#endif
}
void DebugWindow::showDebugWindow(
    const cv::Mat& debugFrame, const cv::Mat& originalFrame, const std::string currentStrategy
) {
    if (debugFrame.empty())
        return;

    cv::namedWindow("Debug", cv::WINDOW_NORMAL);

    m_isWindowOpen = true;

    // debug window always on the top layer
    cv::setWindowProperty("Debug", cv::WND_PROP_TOPMOST, 1);
    updateDebugWindowPosition();

    cv::Rect winRect = cv::getWindowImageRect("Debug");

#if defined(__APPLE__) || defined(__linux__)
    if (winRect.width <= 0 || winRect.height <= 0) {
        int fallbackW = 500;
        int fallbackH = static_cast<int>((fallbackW / m_aspectRatio) * 2) - 50;
        winRect = cv::Rect(0, 0, fallbackW, fallbackH);
    }
#endif

    if (winRect.width > 0 && winRect.height > 0) {
        cv::Mat canvas = drawDebugCanvas(debugFrame, originalFrame, currentStrategy, winRect);
        cv::imshow("Debug", canvas);
    } else {
        cv::imshow("Debug", debugFrame);
    }

    cv::waitKey(1);
}
void DebugWindow::destroy() {
    if (m_isWindowOpen) {
        try {
            //TODO still doesnt work getting the position of window
            cv::Rect winRect = cv::getWindowImageRect("Debug");
            if (winRect.x >= 0 && winRect.y >= 0) {
                m_winPosX = winRect.x;
                m_winPosY = winRect.y - 30; // title barr compensation
            }
        } catch (...) {
            // ignore for now
        }

        cv::destroyWindow("Debug");
        cv::waitKey(1);
        m_isWindowOpen = false;
    }
}