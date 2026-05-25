#pragma once
#include "../IRenderStrategy.h"

class BaseGrayscaleStrategy : public IRenderStrategy
{
private:
    std::string m_asciiChars = " .:-=+*#%@";

public:
    void render(const cv::Mat &inputFrame, std::vector<ImageUtils::Pixel> &outBuffer, int width, int height)
    {
        cv::Mat resizedFrame;
        cv::resize(inputFrame, resizedFrame, cv::Size(width, height));

        // Musíme změnit velikost bufferu, pokud nebyla správná
        outBuffer.resize(width * height);

        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                cv::Vec3b pixel = resizedFrame.at<cv::Vec3b>(y, x);
                uchar brightness = calculateBrightness(pixel[2], pixel[1], pixel[0]);

                int charIndex = (brightness * (m_asciiChars.length() - 1)) / 255;

                // Teď plníš Pixel strukturu
                outBuffer[y * width + x] = {m_asciiChars[charIndex], pixel};
            }
        }
    }
    virtual uchar calculateBrightness(uchar r, uchar g, uchar b) = 0;
};
