#pragma once

#include <memory>

#include "core/BaseFrameProvider.h"

class ScreenCaptureFactory
{
    public:
    static std::unique_ptr<BaseFrameProvider> create();
};