#pragma once
#include "core/FrameBufferManager.h"

class MacScreenCapture : public FrameBufferManager {
  public:
    void start() override;
    void stop() override;
};