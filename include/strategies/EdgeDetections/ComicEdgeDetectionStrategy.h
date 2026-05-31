#pragma once
#include "strategies/EdgeDetections/AdvancedEdgeDetectionStrategy.h"
#include <string>

/**
 * @class ComicEdgeDetectionStrategy
 * @brief Renders the video in a stylized "comic book" or sketch aesthetic.
 * * Combines structural edge detection with background grayscale shading.
 * * Uses a custom algorithm (Smart Edge) to analyze the consistency of lines, 
 * rendering solid lines for strong structural boundaries and different characters 
 * (like '+') for corners or broken details.
 */
class ComicEdgeDetectionStrategy : public AdvancedEdgeDetectionStrategy
{
private:
    int m_searchRadius = 2;         ///< Distance to check for line continuity.
    float m_edgeSensitivity = 0.7f; ///< Required ratio of matching neighbors to form a solid line.
    std::string m_asciiChars = " .:-=+*#%@";

    /**
     * @brief Maps a background brightness value to a shading ASCII character.
     */
    uchar getShadingChar(uchar brightness);

    /**
     * @brief Analyzes the neighborhood of an edge pixel to determine its structural consistency.
     * @param x X coordinate.
     * @param y Y coordinate.
     * @param angles Matrix of gradient angles.
     * @param finalEdges Matrix of NMS/Hysteresis filtered magnitudes.
     * @param edgeThreshold The minimum magnitude to be considered a valid neighbor.
     * @return A directional character for solid lines, or a detail character for corners/broken lines.
     */
    uchar getSmartEdgeChar(int x, int y, const cv::Mat &angles, const cv::Mat &finalEdges, float edgeThreshold);

public:
    std::vector<Property> getProperties() override;
    void setProperty(const Property property) override;

protected:
    /**
     * @brief Decides whether to draw a structural edge or background shading for a specific pixel.
     */
    char determinePixelChar(int x, int y, float mag, float /*angle*/, const cv::Mat &allMagnitudes, const cv::Mat &allAngles, const cv::Mat &grayFrame) override;
};