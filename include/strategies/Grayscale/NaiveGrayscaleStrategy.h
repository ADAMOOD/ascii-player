#pragma once
#include "BaseGrayscaleStrategy.h"

class NaiveGrayscaleStrategy : public BaseGrayscaleStrategy
{
private:
public:
    uchar calculateBrightness(uchar r, uchar g, uchar b)
    {
        return static_cast<uchar>((r + g + b) / 3);
    }
};
