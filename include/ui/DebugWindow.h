#pragma once
#include <opencv2/opencv.hpp>

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

  public:
    void destroy();
    DebugWindow() = default;
    bool init(double source_aspectRatio);

    void showDebugWindow(
        const cv::Mat& debugFrame, const cv::Mat& originalFrame, const std::string currentStrategy
    );
};