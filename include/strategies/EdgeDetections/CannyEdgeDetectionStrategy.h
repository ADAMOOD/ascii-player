#pragma once
#include "strategies/EdgeDetections/AdvancedEdgeDetectionStrategy.h"

/**
 * @class CannyEdgeDetectionStrategy
 * @brief Implements character mapping for Canny edge detection.
 * * Uses gradient angles to map strong edges to directional ASCII characters.
 */
class CannyEdgeDetectionStrategy : public AdvancedEdgeDetectionStrategy
{
private:
public:
    std::vector<Property> getProperties() override
    {
        auto props = AdvancedEdgeDetectionStrategy::getProperties();
        return props;
    }

    void setProperty(const Property property) override
    {
        AdvancedEdgeDetectionStrategy::setProperty(property);
    }

    char determinePixelChar(int /*x*/, int /*y*/, float mag, float angle, const cv::Mat & /*allMagnitudes*/, const cv::Mat & /*allAngles*/, const cv::Mat & /*grayFrame*/) override
    {
        if (mag >= m_edgeThreshold)
        {
            return getAsciiForAngle(angle);
        }
        return m_fillChar;
    }
};