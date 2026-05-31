#pragma once
#include "BaseGrayscaleStrategy.h"
/**
 * @class ValueGrayscaleStrategy
 * @brief Calculates brightness using the HSV Value channel (max of R, G, B).
 * Produces a brighter, more high-contrast result compared to other methods.
 */
class ValueGrayscaleStrategy : public BaseGrayscaleStrategy
{
public:
    /**
     * @brief Returns the maximum of R, G, B as the brightness value.
     * @param r Red channel (0-255). @param g Green channel (0-255). @param b Blue channel (0-255).
     * @return uchar Brightness value (0-255).
     */
    uchar calculateBrightness(uchar r, uchar g, uchar b) override
    {
        return std::max({r, g, b});
    }
};

