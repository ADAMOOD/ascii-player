#include "strategies/EdgeDetections/ComicEdgeDetectionStrategy.h"

std::vector<Property> ComicEdgeDetectionStrategy::getProperties()
{
    auto props = AdvancedEdgeDetectionStrategy::getProperties();
    props.push_back({"Search Radius", PropertyType::INTEGER, static_cast<float>(m_searchRadius), 1.0f, 1.0f, 10.0f});
    props.push_back({"Sensitivity", PropertyType::FLOAT, m_edgeSensitivity, 0.05f, 0.0f, 1.0f});
    return props;
}

void ComicEdgeDetectionStrategy::setProperty(const Property property)
{
    if (property.name == "Search Radius")
        m_searchRadius = static_cast<int>(property.currentValue);
    else if (property.name == "Sensitivity")
        m_edgeSensitivity = property.currentValue;
    else
        AdvancedEdgeDetectionStrategy::setProperty(property);
}

uchar ComicEdgeDetectionStrategy::getShadingChar(uchar brightness)
{
    return m_asciiChars[(brightness * (m_asciiChars.length() - 1)) / 255];
}

char ComicEdgeDetectionStrategy::determinePixelChar(int x, int y, float mag, float /*angle*/, const cv::Mat &allMagnitudes, const cv::Mat &allAngles, const cv::Mat &grayFrame)
{
    if (mag >= m_edgeThreshold) 
    {
        char edgeChar = getSmartEdgeChar(x, y, allAngles, allMagnitudes, m_edgeThreshold);
        
        if (edgeChar != '\0')
        {
            return edgeChar;
        }
    }
    
    // Fallback
    uchar brightness = grayFrame.at<uchar>(y, x);
    return getShadingChar(brightness);
}

uchar ComicEdgeDetectionStrategy::getSmartEdgeChar(int x, int y, const cv::Mat &angles, const cv::Mat &finalEdges, float edgeThreshold)
{
    float angle = angles.at<float>(y, x);
    char pixelChar = getAsciiForAngle(angle);

    int dy = 0, dx = 0;
    
    switch (pixelChar)
    {
    case '-': dy = 0; dx = 1; break;  
    case '|': dy = 1; dx = 0; break;  
    case '/': dy = -1; dx = 1; break; 
    case '\\': dy = 1; dx = 1; break; 
    default: return '\0';
    }

    int matchingNeighbors = 0;
    int totalNeighborsChecked = m_searchRadius * 2;

    auto checkDirection = [&](int stepDy, int stepDx) {
        int ny = y + stepDy;
        int nx = x + stepDx;
        
        for (int j = -1; j <= 1; j++) {
            for (int i = -1; i <= 1; i++) {
                int cy = ny + j;
                int cx = nx + i;
                
                if (cy < 0 || cx < 0 || cy >= finalEdges.rows || cx >= finalEdges.cols) continue;
                if (cy == y && cx == x) continue;
                
                if (finalEdges.at<float>(cy, cx) >= edgeThreshold) {
                    
                    float neighborAngle = angles.at<float>(cy, cx);
                    float diff = std::abs(neighborAngle - angle);
                    
                    if (diff > 90.0f) {
                        diff = 180.0f - diff; 
                    }
                    
                    if (diff <= 22.5f) {
                        return true;
                    }
                }
            }
        }
        return false;
    };

    for (int step = 1; step <= m_searchRadius; step++) {
        if (checkDirection(step * dy, step * dx)) {
            matchingNeighbors++;
        }
        if (checkDirection(-step * dy, -step * dx)) {
            matchingNeighbors++;
        }
    }

float matchRatio = (float)matchingNeighbors / totalNeighborsChecked;

    if (matchRatio >= m_edgeSensitivity)
    {
        return pixelChar;
    }
    else
    {
        return '\0'; 
    }
}