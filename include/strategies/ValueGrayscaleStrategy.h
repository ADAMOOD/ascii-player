#pragma once
#include "BaseGrayscaleStrategy.h"
class ValueGrayscaleStrategy:public BaseGrayscaleStrategy
{

public:
    uchar calculateBrightness(uchar r, uchar g, uchar b)
    {
        return std::max({r, g, b});
    }
};

