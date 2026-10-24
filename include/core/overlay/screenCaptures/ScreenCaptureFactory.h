#pragma once

#include <memory>

#include "core/FrameBufferManager.h"

class ScreenCaptureFactory
{
    public:
    static std::unique_ptr<FrameBufferManager> create();
};