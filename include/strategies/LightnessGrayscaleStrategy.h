#pragma once
#include "BaseGrayscaleStrategy.h"

class LightnessGrayscaleStrategy:public BaseGrayscaleStrategy
{

public:
    uchar calculateBrightness(uchar r, uchar g, uchar b)
    {
        int min = std::min({r, g, b});
        int max = std::max({r, g, b});
        return static_cast<uchar>((min + max) / 2);
    }
};

