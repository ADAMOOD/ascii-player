#pragma once
#include "BaseEdgeDetectionStrategy.h"

class CannyEdgeDetectionStrategy : public BaseEdgeDetectionStrategy
{
private:
    bool m_useHysteresis = false;
    const cv::Mat kernel5x5 = (cv::Mat_<float>(5, 5) << 1, 4, 7, 4, 1,
                               4, 16, 26, 16, 4,
                               7, 26, 41, 26, 7,
                               4, 16, 26, 16, 4,
                               1, 4, 7, 4, 1);

public:
    void render(const cv::Mat &inputFrame, std::string &outBuffer, int width, int height)
    {
        cv::Mat resizedFrame;
        cv::resize(inputFrame, resizedFrame, cv::Size(width, height));

        cv::Mat grayFrame = cv::Mat::zeros(height, width, CV_8UC1);
        convertToGrayscale(resizedFrame, grayFrame, width, height);

        cv::Mat blurredFrame = cv::Mat::zeros(height, width, CV_8UC1);
        applyFilter(grayFrame, blurredFrame, kernel5x5, width, height);

        cv::Mat magnitudes = cv::Mat::zeros(height, width, CV_32F);
        cv::Mat angles = cv::Mat::zeros(height, width, CV_32F);
        computeSobelData(blurredFrame, magnitudes, angles, width, height);

        cv::Mat nmsMagnitudes = cv::Mat::zeros(height, width, CV_32F);
        applyNonMaximumSuppression(magnitudes, angles, nmsMagnitudes, width, height);

        if (m_useHysteresis)
        {
            cv::Mat hysteresisResult = cv::Mat::zeros(height, width, CV_32F);
            applyHysteresis(nmsMagnitudes, hysteresisResult, width, height, 30.0f, 100.0f);
            nmsMagnitudes = hysteresisResult;
        }

        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int bufferIndex = y * (width + 1) + x;

                int deadZone = (kernel5x5.rows / 2) + 1;


                if (x < deadZone || y < deadZone || x >= width - deadZone || y >= height - deadZone)
                {
                    outBuffer[bufferIndex] = ' ';
                    continue;
                }

                // Pokud nejsme na okraji, načteme data
                float mag = nmsMagnitudes.at<float>(y, x);
                float angle = angles.at<float>(y, x);

                if (mag >= 50.0f)
                {
                    outBuffer[bufferIndex] = getAsciiForAngle(angle);
                }
                else
                {
                    outBuffer[bufferIndex] = ' ';
                }
            }
        }
    }
};
