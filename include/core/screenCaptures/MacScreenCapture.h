#pragma once
#include "core/IScreenCapture.h"

class MacScreenCapture : public IScreenCapture {
public:
    cv::Mat grabScreen() override;
};