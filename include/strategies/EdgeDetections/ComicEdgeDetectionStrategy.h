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

    uchar getShadingChar(uchar brightness)
    {
        return m_asciiChars[(brightness * (m_asciiChars.length() - 1)) / 255];
    }
    uchar getSmartEdgeChar(int x, int y, const cv::Mat &angles, const cv::Mat &finalEdges)
    {
        float angle = angles.at<float>(y, x);
        char pixelChar = getAsciiForAngle(angle);

        int ny1 = 0, nx1 = 0;
        int ny2 = 0, nx2 = 0;
        switch (pixelChar)
        {
        case '-':
        {
            ny1 = y;
            nx1 = x - 1;
            ny2 = y;
            nx2 = x + 1;
            break;
        }

        case '/':
        {
            ny1 = y + 1;
            nx1 = x - 1;
            ny2 = y - 1;
            nx2 = x + 1;
            break;
        }
        case '\\':
        {
            ny1 = y - 1;
            nx1 = x - 1;
            ny2 = y + 1;
            nx2 = x + 1;
            break;
        }
        case '|':
        {
            ny1 = y - 1;
            nx1 = x;
            ny2 = y + 1;
            nx2 = x;
            break;
        }
        }
        bool neighbour1Ok = (finalEdges.at<float>(ny1, nx1) == 255.0f) && (getAsciiForAngle(angles.at<float>(ny1, nx1)) == pixelChar);
        bool neighbour2Ok = (finalEdges.at<float>(ny2, nx2) == 255.0f) && (getAsciiForAngle(angles.at<float>(ny2, nx2)) == pixelChar);

        if (neighbour1Ok && neighbour2Ok)
        {
            return pixelChar;
        }
        else
        {
            // TODO handle different angle changes
            return '+'; // Je to detail/roh, kreslím malý spojovací znak.
        }
    }

public:
    ComicEdgeDetectionStrategy(int kernelSize, float edgeTreashold)
    {
        //TODO allow user to configure image analysing properties
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

                if (mag == 255.0f)
                {
                    outBuffer[bufferIndex] = getSmartEdgeChar(x, y, angles, nmsMagnitudes);
                }
                else
                {
                    outBuffer[bufferIndex] = getShadingChar(grayFrame.at<uchar>(y, x));
                }
            }
        }
    }
};

