#pragma once
#include "BaseGrayscaleStrategy.h"

class PerceptualGrayscaleStrategy : public BaseGrayscaleStrategy {
public:

    uchar calculateBrightness(uchar r, uchar g, uchar b) override {
        return static_cast<uchar>(0.299 * r + 0.587 * g + 0.114 * b);
    }
};