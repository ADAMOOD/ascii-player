#pragma once
#include "AdvancedEdgeDetectionStrategy.h"

class CannyEdgeDetectionStrategy : public AdvancedEdgeDetectionStrategy
{
private:


public:
char determinePixelChar(int /*x*/, int /*y*/, float mag, float angle, const cv::Mat& /*allMagnitudes*/, const cv::Mat& /*allAngles*/, const cv::Mat& /*grayFrame*/) override
    {
        if (mag >= 50.0f) 
        {
            return getAsciiForAngle(angle); 
        }
        return ' ';
    }
};
