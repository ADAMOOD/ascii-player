#pragma once
#include <opencv2/opencv.hpp>
#include <string>

class IRenderStrategy
{
public:
    virtual void render(const cv::Mat &inputFrame, std::string &outBuffer, int width, int height) = 0;

    // nessesary
    virtual ~IRenderStrategy() = default;

protected:
    uchar calculateAverage(cv::Vec3b chanels)
    {
        return static_cast<uchar>((chanels[0] + chanels[1] + chanels[2]) / 3);
    }
};