#pragma once
#include "strategies/AbstractRenderStrategy.h"
#include <string>

/**
 * @class BaseGrayscaleStrategy
 * @brief Base class for all grayscale-based ASCII rendering strategies.
 * * Handles the common logic of resizing the frame and mapping a calculated 
 * brightness value to a character from the ASCII ramp. Subclasses only need 
 * to define the math for calculating that brightness.
 */
class BaseGrayscaleStrategy : public AbstractRenderStrategy
{
private:
    std::string m_asciiChars = " .:-=+*#%@";

public:
    /**
     * @brief Resizes the frame and converts pixels to ASCII using subclass brightness logic.
     */
    void render(const cv::Mat &inputFrame, std::vector<ImageUtils::Pixel> &outBuffer, int width, int height) override;

protected:
    /**
     * @brief Pure virtual function to calculate brightness. Implemented by subclasses.
     * @param r Red channel value (0-255).
     * @param g Green channel value (0-255).
     * @param b Blue channel value (0-255).
     * @return Calculated grayscale brightness (0-255).
     */
    virtual uchar calculateBrightness(uchar r, uchar g, uchar b) = 0;
};