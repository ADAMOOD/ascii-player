#include "strategies/Grayscale/BaseGrayscaleStrategy.h"

void BaseGrayscaleStrategy::render(const cv::Mat& inputFrame, std::string& outBuffer, int width, int height)
{
    cv::Mat resizedFrame;
    cv::resize(inputFrame, resizedFrame, cv::Size(width, height));
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            cv::Vec3b pixel = resizedFrame.at<cv::Vec3b>(y, x);
            uchar b = pixel[0];
            uchar g = pixel[1];
            uchar r = pixel[2];
            uchar brightness = calculateBrightness(r, g, b);
            int charIndex = (brightness * (m_asciiChars.length() - 1)) / 255;
            int bufferIndex = y * (width + 1) + x;

            outBuffer[bufferIndex] = m_asciiChars[charIndex];
        }
    }
}

