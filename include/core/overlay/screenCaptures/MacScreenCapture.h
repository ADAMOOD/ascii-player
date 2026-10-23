#pragma once
#include "core/BaseFrameProvider.h"

class MacScreenCapture : public BaseFrameProvider {
  public:
       cv::Mat grabSingleFrame() override;
};