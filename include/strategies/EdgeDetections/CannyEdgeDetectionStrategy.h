#pragma once
#include "BaseEdgeDetectionStrategy.h"

class CannyEdgeDetectionStrategy : public BaseEdgeDetectionStrategy
{
private:
public:
    void render(const cv::Mat &inputFrame, std::string &outBuffer, int width, int height)
    {
        cv::Mat resizedFrame;
        cv::resize(inputFrame, resizedFrame, cv::Size(width, height));

        // initializacion
        cv::Mat magnitudes = cv::Mat::zeros(height, width, CV_32F);
        cv::Mat angles = cv::Mat::zeros(height, width, CV_32F);

        computeSobelData(resizedFrame, magnitudes, angles, width, height);

        cv::Mat nmsMagnitudes = cv::Mat::zeros(height, width, CV_32F);//Non-Maximum Suppression - NMS

        for (int y = 1; y < height-1; y++)//cycles have to start from 1 and end before the edge, and end before the edge, because we are going to compare with neighbors
        {
            for (int x = 1; x < width-1; x++)
            {
                float mag = magnitudes.at<float>(y, x);
                float angle = angles.at<float>(y, x);
                float neighbor1 = 0, neighbor2 = 0;

                if(angle >= 0 && angle < 22.5 || angle >= 157.5 && angle <= 180)
                {
                    neighbor1 = magnitudes.at<float>(y, x + 1);
                    neighbor2 = magnitudes.at<float>(y, x - 1);
                }
                else if (angle >= 22.5 && angle < 67.5)
                {
                    neighbor1 = magnitudes.at<float>(y + 1, x - 1);
                    neighbor2 = magnitudes.at<float>(y - 1, x + 1);
                }
                else if (angle >= 67.5 && angle < 112.5)
                {
                    neighbor1 = magnitudes.at<float>(y + 1, x);
                    neighbor2 = magnitudes.at<float>(y - 1, x);
                }
                else if (angle >= 112.5 && angle < 157.5)
                {
                    neighbor1 = magnitudes.at<float>(y - 1, x - 1);
                    neighbor2 = magnitudes.at<float>(y + 1, x + 1);
                }
                if(mag>=neighbor1 && mag>= neighbor2)
                {
                    nmsMagnitudes.at<float>(y,x) = mag;
                }
            }
        }
    }
};
