#pragma once
#include "core/terminal/ThreadedFrameProducer.h"
#include <string>

#include <opencv2/opencv.hpp>

// TODO make stream manager descendant of BaseFrameProvider
class StreamManager : public ThreadedFrameProducer {
  private:
    cv::VideoCapture m_cap;      ///< OpenCV video capture object (handles both files and webcam).
    bool m_isLiveStream = false; ///< Flag indicating if the source is a live webcam feed.

  protected:
    cv::Mat grabSingleFrame() override;

  public:
    bool init();
    bool init(const std::string& videoPath);
    void getOriginalSize(double& outWidth, double& outHeight);
    double getStreamFps() const;
};
