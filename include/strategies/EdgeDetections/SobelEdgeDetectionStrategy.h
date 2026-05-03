#pragma once
#include "BaseEdgeDetectionStrategy.h"
#include <cmath>

class SobelEdgeDetectionStrategy : public BaseEdgeDetectionStrategy
{
public:
    void render(const cv::Mat &inputFrame, std::string &outBuffer, int width, int height) override
    {
        cv::Mat resizedFrame;
        cv::resize(inputFrame, resizedFrame, cv::Size(width, height));

        // initializacion
        cv::Mat magnitudes = cv::Mat::zeros(height, width, CV_32F);
        cv::Mat angles = cv::Mat::zeros(height, width, CV_32F);

        computeSobelData(resizedFrame, magnitudes, angles, width, height);

        // 3. DRUHÝ PRŮCHOD: Samotné kreslení
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int bufferIndex = y * (width + 1) + x;

                // Tohle je kreslení do bufferu, takže tady okraje pořád chráníme!
                if (x == 0 || y == 0 || x == width - 1 || y == height - 1)
                {
                    outBuffer[bufferIndex] = ' ';
                    continue;
                }

                // Tady si už jen vytáhneme hotová čísla
                float magnitude = magnitudes.at<float>(y, x);
                float angle = angles.at<float>(y, x);

                if (magnitude < 50)
                {
                    outBuffer[bufferIndex] = ' ';
                    continue;
                }
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
