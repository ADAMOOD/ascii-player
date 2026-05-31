#pragma once
#include "strategies/EdgeDetections/BaseEdgeDetectionStrategy.h"

class SobelEdgeDetectionStrategy : public BaseEdgeDetectionStrategy
{
public:
    void render(const cv::Mat &inputFrame, std::vector<ImageUtils::Pixel> &outBuffer, int width, int height) override;
};