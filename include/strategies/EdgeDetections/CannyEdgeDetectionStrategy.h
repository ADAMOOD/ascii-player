#pragma once
#include "BaseEdgeDetectionStrategy.h"

class CannyEdgeDetectionStrategy : public BaseEdgeDetectionStrategy
{
private:
    bool m_useHysteresis = false;

public:
    void onKeyPress(char key) override
    {
        if (key == 'h' || key == 'H')
        {
            m_useHysteresis = !m_useHysteresis;
        }
    }
    void render(const cv::Mat &inputFrame, std::string &outBuffer, int width, int height)
    {
        cv::Mat resizedFrame;
        cv::resize(inputFrame, resizedFrame, cv::Size(width, height));

        // initializacion
        cv::Mat magnitudes = cv::Mat::zeros(height, width, CV_32F);
        cv::Mat angles = cv::Mat::zeros(height, width, CV_32F);

        computeSobelData(resizedFrame, magnitudes, angles, width, height);

        cv::Mat nmsMagnitudes = cv::Mat::zeros(height, width, CV_32F); // Non-Maximum Suppression - NMS

        for (int y = 1; y < height - 1; y++) // cycles have to start from 1 and end before the edge, and end before the edge, because we are going to compare with neighbors
        {
            for (int x = 1; x < width - 1; x++)
            {
                float mag = magnitudes.at<float>(y, x);
                float angle = angles.at<float>(y, x);
                float neighbor1 = 0, neighbor2 = 0;

                if (angle >= 0 && angle < 22.5 || angle >= 157.5 && angle <= 180)
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

                int bufferIndex = y * (width + 1) + x;

                if (mag >= neighbor1 && mag >= neighbor2)
                {
                    nmsMagnitudes.at<float>(y, x) = mag;

                    // Rychlá cesta pro vypnutou Hysterezi (rovnou kreslíme)
                    if (!m_useHysteresis && mag >= 50)
                    {
                        outBuffer[bufferIndex] = getAsciiForAngle(angle);
                    }
                    else if (!m_useHysteresis)
                    {
                        outBuffer[bufferIndex] = ' ';
                    }
                }
                else if (!m_useHysteresis)
                {
                    outBuffer[bufferIndex] = ' ';
                }
            }
        }

        // A pak teprve Hysterezní (3.) cyklus
        if (m_useHysteresis)
        {
            float highThreshold = 100.0f; // Edge is certain if above this
            float lowThreshold = 30.0f;   // noise if below this, edge if between low and high and connected to strong edge
            for (int y = 0; y < height; y++)
            {
                for (int x = 0; x < width; x++)
                {
                    int bufferIndex = y * (width + 1) + x;
                    float mag = nmsMagnitudes.at<float>(y, x);
                    float angle = angles.at<float>(y, x);
                    if(mag >= highThreshold)
                    {
                        outBuffer[bufferIndex] = getAsciiForAngle(angle);
                    }
                    else if (mag >= lowThreshold)
                    {
                        // Check 8-connected neighbors for a strong edge
                        bool connectedToStrongEdge = false;
                        for (int j = -1; j <= 1; j++)
                        {
                            for (int i = -1; i <= 1; i++)
                            {
                                if (j == 0 && i == 0)
                                    continue;
                                int neighborY = y + j;
                                int neighborX = x + i;
                                if (neighborY >= 0 && neighborY < height && neighborX >= 0 && neighborX < width)
                                {
                                    if (nmsMagnitudes.at<float>(neighborY, neighborX) >= highThreshold)
                                    {
                                        connectedToStrongEdge = true;
                                        break;
                                    }
                                }
                            }
                            if (connectedToStrongEdge)
                                break;
                        }
                        if (connectedToStrongEdge)
                        {
                            outBuffer[bufferIndex] = getAsciiForAngle(angle);
                        }
                        else
                        {
                            outBuffer[bufferIndex] = ' ';
                        }
                    }
                    else
                    {
                        outBuffer[bufferIndex] = ' ';
                    }
                }
            }
        }
    }
};
