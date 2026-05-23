#pragma once
#include "AdvancedEdgeDetectionStrategy.h"
#include "CannyEdgeDetectionStrategy.h"

class ComicEdgeDetectionStrategy : public AdvancedEdgeDetectionStrategy
{
private:
    int m_searchRadius = 2;
    float m_edgeSensitivity = 0.7f;

    std::string m_asciiChars = " .:-=+*#%@";
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
        int totalNeighborsChecked = m_searchRadius * 2;

        // Check one direction (positive steps)
        for (int step = 1; step <= m_searchRadius; step++)
        {
            int ny = y + (step * dy);
            int nx = x + (step * dx);
            if (ny < 0 || nx < 0 || ny >= finalEdges.rows || nx >= finalEdges.cols)
                continue;
            if (finalEdges.at<float>(ny, nx) >= edgeThreshold && getAsciiForAngle(angles.at<float>(ny, nx)) == pixelChar)
            {
                matchingNeighbors++;
            }
        }

        // Check the opposite direction (negative steps)
        for (int step = 1; step <= m_searchRadius; step++)
        {
            int ny = y - (step * dy);
            int nx = x - (step * dx);
            if (ny < 0 || nx < 0 || ny >= finalEdges.rows || nx >= finalEdges.cols)
                continue;
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
            // todo other lines and detail handeling
            return '+'; // Broken line, corner, or detail
        }
    }

public:
    std::vector<Property> getProperties() override
    {
        auto props = AdvancedEdgeDetectionStrategy::getProperties();
        props.push_back({"Search Radius", PropertyType::INTEGER, static_cast<float>(m_searchRadius), 1.0f, 1.0f, 10.0f});
        props.push_back({"Sensitivity", PropertyType::FLOAT, m_edgeSensitivity, 0.05f, 0.0f, 1.0f});

        return props;
    }

    void setProperty(const Property property) override
    {
        if (property.name == "Search Radius")
            m_searchRadius = static_cast<int>(property.currentValue);
        else if (property.name == "Sensitivity")
            m_edgeSensitivity = property.currentValue;
        else
        {
            AdvancedEdgeDetectionStrategy::setProperty(property);
        }
    }
    char determinePixelChar(int x, int y, float mag, float /*angle*/, const cv::Mat &allMagnitudes, const cv::Mat &allAngles, const cv::Mat &grayFrame) override
    {
        if (mag >= 50.0f) // Hrana
        {
            return getSmartEdgeChar(x, y, allAngles, allMagnitudes, 50.0f);
        }
        else // Výplň (Shading)
        {
            // Tady použijeme ten grayFrame, který nám Advanced třída laskavě podala jako parametr
            uchar brightness = grayFrame.at<uchar>(y, x);
            return getShadingChar(brightness);
        }
    }
};
