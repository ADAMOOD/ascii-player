#include "strategies/Grayscale/BaseGrayscaleStrategy.h"

void BaseGrayscaleStrategy::render(const cv::Mat &inputFrame, std::vector<ImageUtils::Pixel> &outBuffer, int width, int height)
{
    cv::Mat resizedFrame;
    cv::resize(inputFrame, resizedFrame, cv::Size(width, height));

    outBuffer.resize(width * height);

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            cv::Vec3b pixel = resizedFrame.at<cv::Vec3b>(y, x);
            // virtual method call to calculate brightness based on the specific Grayscale strategy
            uchar brightness = calculateBrightness(pixel[2], pixel[1], pixel[0]);

            int charIndex = (brightness * (m_asciiChars.length() - 1)) / 255;

            outBuffer[y * width + x] = {m_asciiChars[charIndex], pixel, {0,0,0}};
        }
    }
}