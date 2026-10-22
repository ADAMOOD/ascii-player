#include "strategies/Grayscale/BaseGrayscaleStrategy.h"

void BaseGrayscaleStrategy::render(const cv::Mat &inputFrame, std::vector<ImageUtils::Pixel> &outBuffer, int width, int height)
{
    cv::Mat resizedFrame;
    cv::resize(inputFrame, resizedFrame, cv::Size(width, height));

    outBuffer.resize(width * height);

    if (m_show_debug_window) {
        // create() faster than constructor
        m_debugFrame.create(height, width, CV_8UC1); 
    }

    for (int y = 0; y < height; y++)
    {
        // pointer on the whole mat row
        uchar* debugRow = m_show_debug_window ? m_debugFrame.ptr<uchar>(y) : nullptr;

        for (int x = 0; x < width; x++)
        {
            cv::Vec3b pixel = resizedFrame.at<cv::Vec3b>(y, x);
            
            // one call only
            uchar brightness = calculateBrightness(pixel[2], pixel[1], pixel[0]);

            // direct write to memory
            if (debugRow) {
                debugRow[x] = brightness;
            }

            int charIndex = (brightness * (m_asciiChars.length() - 1)) / 255;
            outBuffer[y * width + x] = {m_asciiChars[charIndex], pixel, {0, 0, 0}};
        }
    }
}