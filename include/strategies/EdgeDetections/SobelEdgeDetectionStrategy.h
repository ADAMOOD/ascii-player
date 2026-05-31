#pragma once
#include "strategies/EdgeDetections/BaseEdgeDetectionStrategy.h"
/**
 * @class SobelEdgeDetectionStrategy
 * @brief Renders edges using raw Sobel gradient magnitude and direction.
 * Pixels above the edge threshold are drawn as directional ASCII characters.
 * Non-edge pixels use the configurable fill character.
 */
class SobelEdgeDetectionStrategy : public BaseEdgeDetectionStrategy
{
public:
    void render(const cv::Mat &inputFrame, std::vector<ImageUtils::Pixel> &outBuffer, int width, int height) override;
};