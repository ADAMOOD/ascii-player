#pragma once
#include "../IRenderStrategy.h"

class BaseEdgeDetectionStrategy : public IRenderStrategy
{
private:
    /* data */
protected:
    void computeSobelData(const cv::Mat &resizedFrame, cv::Mat &magnitudes, cv::Mat &angles, int width, int height)
    {
        const cv::Matx33f Gx(-1, 0, 1, -2, 0, 2, -1, 0, 1);
        const cv::Matx33f Gy(1, 2, 1, 0, 0, 0, -1, -2, -1);

        // Tady můžeme začít bezpečně od 1 a skončit před okrajem
        for (int y = 1; y < height - 1; y++)
        {
            for (int x = 1; x < width - 1; x++)
            {
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

                float magnitude = std::abs(sumX) + std::abs(sumY);
                if (magnitude > 255)
                    magnitude = 255;

                float angle = std::atan2(sumY, sumX) * 180 / M_PI;
                if (angle < 0)
                    angle += 180;

                magnitudes.at<float>(y, x) = magnitude;
                angles.at<float>(y, x) = angle;
            }
        }
    }
};
