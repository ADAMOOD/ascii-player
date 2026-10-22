#pragma once
#include <string>

#include <memory>

#include <opencv2/opencv.hpp>

#include "core/CoreEngine.h"
#include "core/IScreenCapture.h"

class OverlayEngine : public CoreEngine {
public:
  bool init() override;
  void play() override;

private:
  std::unique_ptr<IScreenCapture> m_screenCapture;
};