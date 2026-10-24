#pragma once
#include <memory>

#include <opencv2/opencv.hpp>

#include "core/CoreEngine.h"
#include "core/FrameBufferManager.h"
#include "core/overlay/WindowManager.h"

class OverlayEngine : public CoreEngine {
public:
  bool init() override;
  void play() override;

private:
  std::unique_ptr<FrameBufferManager> m_frameProvider;
  WindowManager m_windowManager;
};