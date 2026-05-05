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

        cv::Mat grayFrame = cv::Mat::zeros(height, width, CV_8UC1);
        convertToGrayscale(resizedFrame, grayFrame, width, height);

        cv::Mat blurredFrame = cv::Mat::zeros(height, width, CV_8UC1);
        applyFilter(grayFrame, blurredFrame, kernel5x5, width, height);

        cv::Mat magnitudes = cv::Mat::zeros(height, width, CV_32F);
        cv::Mat angles = cv::Mat::zeros(height, width, CV_32F);
        computeSobelData(blurredFrame, magnitudes, angles, width, height);

        cv::Mat nmsMagnitudes = cv::Mat::zeros(height, width, CV_32F);
        applyNonMaximumSuppression(magnitudes, angles, nmsMagnitudes, width, height);

        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int bufferIndex = y * (width + 1) + x;

                int deadZone = (kernel5x5.rows / 2) + 1;

                // A všechny ty falešné hrany natvrdo smažeme!
                if (x < deadZone || y < deadZone || x >= width - deadZone || y >= height - deadZone)
                {
                    outBuffer[bufferIndex] = ' ';
                    continue;
                }

                // Pokud nejsme na okraji, načteme data
                float mag = nmsMagnitudes.at<float>(y, x);
                float angle = angles.at<float>(y, x);

                // 2. Vykreslení podle aktuálního režimu
                if (!m_useHysteresis)
                {
                    // --- Režim A: Bez hystereze (Klasický Canny/Sobel) ---
                    if (mag >= 50.0f)
                    {
                        outBuffer[bufferIndex] = getAsciiForAngle(angle); // Nebo znak '#', jak jsi měl
                    }
                    else
                    {
                        outBuffer[bufferIndex] = ' ';
                    }
                }
                else
                {
                    // --- Režim B: S hysterezí ---
                    float highThreshold = 100.0f;
                    float lowThreshold = 30.0f;

                    if (mag >= highThreshold)
                    {
                        outBuffer[bufferIndex] = getAsciiForAngle(angle);
                    }
                    else if (mag >= lowThreshold)
                    {
                        bool connectedToStrongEdge = false;
                        for (int j = -1; j <= 1; j++)
                        {
                            for (int i = -1; i <= 1; i++)
                            {
                                if (j == 0 && i == 0)
                                    continue;

                                int neighborY = y + j;
                                int neighborX = x + i;

                                // Tady už nemusíme kontrolovat meze (neighborY >= 0),
                                // protože okraje obrazu jsme vyloučili v kroku 1!
                                if (nmsMagnitudes.at<float>(neighborY, neighborX) >= highThreshold)
                                {
                                    connectedToStrongEdge = true;
                                    break;
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
