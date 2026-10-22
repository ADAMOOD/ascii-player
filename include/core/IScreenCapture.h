#pragma once
#include <opencv2/opencv.hpp>

class IScreenCapture {
public:
    virtual ~IScreenCapture() = default;
    virtual cv::Mat grabScreen() = 0;
};