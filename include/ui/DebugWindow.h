#pragma once
#include <opencv2/opencv.hpp>
#include <string>

class DebugWindow {
  private:
    double m_aspectRatio; ///< Original aspect ratio of the video source.

    cv::Mat drawDebugCanvas(
        const cv::Mat& debugFrame,
        const cv::Mat& originalFrame,
        const std::string& currentStrategy,
        const cv::Rect& winRect
    );
    void updateDebugWindowPosition();
    bool m_isWindowOpen = false;

    int m_winPosX = -1;
    int m_winPosY = -1;

     std::string m_windowName;

  public:
    void destroy();
    DebugWindow() = default;
    bool init(double source_aspectRatio, const std::string& debugWindowName = "Debug");

    void showDebugWindow(
        const cv::Mat& debugFrame, const cv::Mat& originalFrame, const std::string currentStrategy
    );
};