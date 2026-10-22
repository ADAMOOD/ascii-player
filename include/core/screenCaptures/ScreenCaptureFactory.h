#pragma once
#pragma once
#include <string>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>

#include "core/IScreenCapture.h"

class ScreenCaptureFactory
{
    public:
    static std::unique_ptr<IScreenCapture> create();
};