#pragma once
#include "strategies/Grayscale/BaseGrayscaleStrategy.h"

/**
 * @class PerceptualGrayscaleStrategy
 * @brief Calculates brightness based on human eye perception.
 * * Uses the standard luminance formula (0.299R + 0.587G + 0.114B) because 
 * the human eye is most sensitive to green light and least to blue.
 */
class PerceptualGrayscaleStrategy : public BaseGrayscaleStrategy 
{
public:
    /**
     * @brief Computes perceptually accurate grayscale value.
     */
    uchar calculateBrightness(uchar r, uchar g, uchar b) override 
    {
        return static_cast<uchar>(0.299 * r + 0.587 * g + 0.114 * b);
    }
};