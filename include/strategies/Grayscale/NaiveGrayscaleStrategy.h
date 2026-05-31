#pragma once
#include "BaseGrayscaleStrategy.h"


/**
 * @class NaiveGrayscaleStrategy
 * @brief uses the simplest method to calculate brightness by averaging the RGB values.
 * * This method treats all color channels equally, which can lead to less accurate brightness representation.
 * 
 */
class NaiveGrayscaleStrategy : public BaseGrayscaleStrategy
{
public:
    /**
     * @brief Computes grayscale brightness by averaging the RGB values.
     * 
     * @param r red channel value (0-255)
     * @param g green channel value (0-255)
     * @param b blue channel value (0-255)
     * @return uchar value representing the grayscale brightness (0-255)
     */
    uchar calculateBrightness(uchar r, uchar g, uchar b)override
    {
        return static_cast<uchar>((r + g + b) / 3);
    }
};
