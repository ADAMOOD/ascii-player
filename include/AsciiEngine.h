#include <string>
#include <opencv2/opencv.hpp>
using namespace std;

#pragma once

class AsciiEngine
{
public:
    bool init(const std::string& videoPath, int targetWidth);
private:
    int width;
    int height;
    std::string frameBuffer;
    cv::VideoCapture cap;
};
