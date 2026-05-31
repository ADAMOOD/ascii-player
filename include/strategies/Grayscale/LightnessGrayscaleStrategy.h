#pragma once
#include "BaseGrayscaleStrategy.h"

/**
 * @class LightnessGrayscaleStrategy
 * @brief uses the lightness method to calculate brightness by averaging the max and min RGB values.
 * * This method can produce a more contrasted image compared to the perceptual method, especially in
 * scenes with high saturation, because it emphasizes the difference between the brightest and darkest color channels.
 * results in a more "posterized" look, which can be desirable for certain artistic effects in ASCII rendering.
 *
 */
class LightnessGrayscaleStrategy : public BaseGrayscaleStrategy
{

public:
    /**
     * @brief Computes grayscale brightness using the lightness method.
     * @param r red channel value (0-255)
     * @param g green channel value (0-255)
     * @param b blue channel value (0-255)
     * @return uchar value representing the grayscale brightness (0-255)
     */
    uchar calculateBrightness(uchar r, uchar g, uchar b)
    {
        int min = std::min({r, g, b});
        int max = std::max({r, g, b});
        return static_cast<uchar>((min + max) / 2);
    }
};
