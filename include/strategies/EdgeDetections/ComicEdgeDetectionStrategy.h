#pragma once
#include "BaseEdgeDetectionStrategy.h"
#include "CannyEdgeDetectionStrategy.h"

class ComicEdgeDetectionStrategy : public BaseEdgeDetectionStrategy
{
private:
    std::string m_asciiChars = " .:-=+*#%@";
    bool m_useHysteresis = false;
    const cv::Mat kernel5x5 = (cv::Mat_<float>(5, 5) << 1, 4, 7, 4, 1,
                               4, 16, 26, 16, 4,
                               7, 26, 41, 26, 7,
                               4, 16, 26, 16, 4,
                               1, 4, 7, 4, 1);

    int m_edgeSearchRadius = 2;     // Check 2 neighbors in each direction (total 4)
    float m_edgeSensitivity = 0.7f; // 70% of neighbors must match

    uchar getShadingChar(uchar brightness)
    {
        return m_asciiChars[(brightness * (m_asciiChars.length() - 1)) / 255];
    }
    uchar getSmartEdgeChar(int x, int y, const cv::Mat &angles, const cv::Mat &finalEdges, float edgeThreshold)
    {
        float angle = angles.at<float>(y, x);
        char pixelChar = getAsciiForAngle(angle);

        int dy = 0, dx = 0;
        switch (pixelChar)
        {
        case '-':
            dy = 0;
            dx = 1;
            break; // Step left/right
        case '|':
            dy = 1;
            dx = 0;
            break; // Step up/down
        case '/':
            dy = -1;
            dx = 1;
            break; // Step top-right/bottom-left
        case '\\':
            dy = 1;
            dx = 1;
            break; // Step bottom-right/top-left
        default:
            return ' ';
        }
        
        int matchingNeighbors = 0;
        int totalNeighborsChecked = m_edgeSearchRadius * 2;

        // Check one direction (positive steps)
        for (int step = 1; step <= m_edgeSearchRadius; step++)
        {
            int ny = y + (step * dy);
            int nx = x + (step * dx);
            if (finalEdges.at<float>(ny, nx) >= edgeThreshold && getAsciiForAngle(angles.at<float>(ny, nx)) == pixelChar)
            {
                matchingNeighbors++;
            }
        }

        // Check the opposite direction (negative steps)
        for (int step = 1; step <= m_edgeSearchRadius; step++)
        {
            int ny = y - (step * dy);
            int nx = x - (step * dx);
            if (finalEdges.at<float>(ny, nx) >= edgeThreshold && getAsciiForAngle(angles.at<float>(ny, nx)) == pixelChar)
            {
                matchingNeighbors++;
            }
        }

        // Calculate the ratio
        float matchRatio = (float)matchingNeighbors / totalNeighborsChecked;

        if (matchRatio >= m_edgeSensitivity)
        {
            return pixelChar; // Strong, consistent line
        }
        else
        {
            //todo other lines and detail handeling
            return '+'; // Broken line, corner, or detail
        }
    }

public:
    ComicEdgeDetectionStrategy(int kernelSize, float edgeTreashold)
    {
        // TODO allow user to configure image analysing properties
    }
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

                // deadZone check to prevent out of bounds
                // becose method apply filter aplyes filter only on notPheripheral pixels it creates frame so we have to skip it
                if (x < deadZone || y < deadZone || x >= width - deadZone || y >= height - deadZone)
                {
                    outBuffer[bufferIndex] = ' ';
                    continue;
                }

                float mag = nmsMagnitudes.at<float>(y, x);
                float angle = angles.at<float>(y, x);

                if (m_useHysteresis)
                {
                    if (mag >= 255.0f)
                    {
                        outBuffer[bufferIndex] = getSmartEdgeChar(x, y, angles, nmsMagnitudes, 50.0f);
                    }
                    else
                    {
                        outBuffer[bufferIndex] = getShadingChar(grayFrame.at<uchar>(y, x));
                    }
                }
                else
                {
                    if (mag >= 50.0f)
                    {
                        outBuffer[bufferIndex] = getSmartEdgeChar(x, y, angles, nmsMagnitudes, 50.0f);
                    }
                    else
                    {
                        outBuffer[bufferIndex] = getShadingChar(grayFrame.at<uchar>(y, x));
                    }
                }
            }
        }
    }
};
