#pragma once
#include "IRenderStrategy.h"
#include <cmath>

class SobelEdgeDetectionStrategy : public IRenderStrategy
{
private:
public:
    void render(const cv::Mat &inputFrame, std::string &outBuffer, int width, int height)
    {
        cv::Mat resizedFrame;
        cv::resize(inputFrame, resizedFrame, cv::Size(width, height));

        const cv::Matx33f Gx(-1, 0, 1, -2, 0, 2, -1, 0, 1);
        const cv::Matx33f Gy(1, 2, 1, 0, 0, 0, -1, -2, -1);

        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int bufferIndex = y * (width + 1) + x;

                // Edge detection
                if (x == 0 || y == 0 || x == width - 1 || y == height - 1)
                {
                    outBuffer[bufferIndex] = ' ';
                    continue;
                }

                float sumX = 0;
                float sumY = 0;

                for (int j = -1; j <= 1; j++)
                {
                    for (int i = -1; i <= 1; i++)
                    {
                        cv::Vec3b pixel = resizedFrame.at<cv::Vec3b>(y + j, x + i);
                        uchar average = calculateAverage(pixel);
                        sumX += average * Gx(j + 1, i + 1);
                        sumY += average * Gy(j + 1, i + 1);
                    }
                }

                int magnitude = std::abs(sumX) + std::abs(sumY);
                if (magnitude > 255)
                    magnitude = 255;

                if (magnitude < 50)
                {
                    outBuffer[bufferIndex] = ' ';
                    continue;
                }
                float angle = std::atan2(sumY, sumX) * 180 / M_PI;

                if (angle < 0)
                    angle += 180;

                if (angle > 22.5 && angle <= 67.5)
                {
                    outBuffer[bufferIndex] = '\\';
                    continue;
                }
                else if (angle > 67.5 && angle <= 112.5)
                {
                    outBuffer[bufferIndex] = '-';
                    continue;
                }
                if (angle > 112.5 && angle <= 157.5)
                {
                    outBuffer[bufferIndex] = '/';
                    continue;
                }
                else{
                    outBuffer[bufferIndex] = '|';
                    continue;
                }
            }
        }
    }
};
