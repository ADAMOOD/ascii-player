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
        return getSmartEdgeChar(x, y, allAngles, allMagnitudes, m_edgeThreshold);
    }
    else 
    {
        // If it's not an edge, fill the background with grayscale shading
        uchar brightness = grayFrame.at<uchar>(y, x);
        return getShadingChar(brightness);
    }
}

uchar ComicEdgeDetectionStrategy::getSmartEdgeChar(int x, int y, const cv::Mat &angles, const cv::Mat &finalEdges, float edgeThreshold)
{
    float angle = angles.at<float>(y, x);
    char pixelChar = getAsciiForAngle(angle);

    int dy = 0, dx = 0;
    
    // decide the primary direction to check based on the edge orientation
    switch (pixelChar)
    {
    case '-': dy = 0; dx = 1; break;  
    case '|': dy = 1; dx = 0; break;  
    case '/': dy = -1; dx = 1; break; 
    case '\\': dy = 1; dx = 1; break; 
    default: return ' ';
    }

    int matchingNeighbors = 0;
    int totalNeighborsChecked = m_searchRadius * 2;

    // --- FORGIVING SEARCH LAMBDA ---
    // small lambda function to check for edge continuity in a given direction, allowing for some "forgiveness"
    // in the line detection by checking a 3x3 area around the expected neighbor position.
    auto checkDirection = [&](int stepDy, int stepDx) {
        int ny = y + stepDy;
        int nx = x + stepDx;
        
        // check a 3x3 area around the expected neighbor position (ny, nx) to allow for some "forgiveness" in line detection
        for (int j = -1; j <= 1; j++) {
            for (int i = -1; i <= 1; i++) {
                int cy = ny + j;
                int cx = nx + i;
                
                // border check to avoid accessing out of bounds
                if (cy < 0 || cx < 0 || cy >= finalEdges.rows || cx >= finalEdges.cols) continue;
                if (cy == y && cx == x) continue; // Nepočítáme sami sebe!
                
                if (finalEdges.at<float>(cy, cx) >= edgeThreshold) {
                    return true;
                }
            }
        }
        return false;
    };


    // primary search in the direction of the edge
    for (int step = 1; step <= m_searchRadius; step++) {
        if (checkDirection(step * dy, step * dx)) {
            matchingNeighbors++;
        }
    }

    // secondary search in the opposite direction to check for line continuity on both sides
    for (int step = 1; step <= m_searchRadius; step++) {
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
        return '+'; // Detail character for corners or broken lines
    }
}