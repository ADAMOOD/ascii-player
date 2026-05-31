#include "strategies/EdgeDetections/SobelEdgeDetectionStrategy.h"

void SobelEdgeDetectionStrategy::render(const cv::Mat &inputFrame, std::vector<ImageUtils::Pixel> &outBuffer, int width, int height)
{
        cv::Mat magnitudes = cv::Mat::zeros(height, width, CV_32F);
        cv::Mat angles = cv::Mat::zeros(height, width, CV_32F);
        cv::Mat coloredResizedFrame = cv::Mat::zeros(height, width, CV_8UC3);
       
       
generateBaseEdgeData(inputFrame, coloredResizedFrame, magnitudes, angles, width, height);

        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int bufferIndex = y * width + x;
                cv::Vec3b color = coloredResizedFrame.at<cv::Vec3b>(y,x);
                
                if (x == 0 || y == 0 || x == width - 1 || y == height - 1)
                {
                    outBuffer[bufferIndex] = {' ',{0,0,0}, color};
                    continue;
                }

                float magnitude = magnitudes.at<float>(y, x);
                float angle = angles.at<float>(y, x);

                if (magnitude < m_edgeThreshold)
                {
                    outBuffer[bufferIndex] = {m_fillChar, color,{0, 0, 0}};;
                    continue;
                }
                if (angle < 0)
                    angle += 180;
                outBuffer[bufferIndex] = {getAsciiForAngle(angle), color,{0,0,0}};
            }
        }
}