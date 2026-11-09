#pragma once
#include <opencv2/opencv.hpp>
#include <string>

class DebugWindow {
  private:
    double m_aspectRatio = 1.0; ///< Original aspect ratio of the video source.

    cv::Mat drawDebugCanvas(
        const cv::Mat& debugFrame,
        const cv::Mat& originalFrame,
        const std::string& currentStrategy,
        const cv::Rect& winRect
    );
    void updateDebugWindowPosition();
    bool m_isWindowOpen = false;

    int m_winPosX = 900; ///< Default window position
    int m_winPosY = 100;
    bool m_emptyWarned = false; ///< to print only once empty frame in init

    std::string m_windowName;
    bool m_positionInitialized = false;

  public:
    void destroy();
    DebugWindow() = default;
    bool init(
        double source_aspectRatio,
        const std::string& debugWindowName = "Debug",
        int posX = 900,
        int posY = 100
    );

    void showDebugWindow(
        const cv::Mat& debugFrame, const cv::Mat& originalFrame, const std::string currentStrategy
    );
};