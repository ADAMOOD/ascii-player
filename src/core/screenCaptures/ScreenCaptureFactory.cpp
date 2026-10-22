#include "core/screenCaptures/ScreenCaptureFactory.h"

#ifdef __APPLE__
    #include "core/screenCaptures/MacScreenCapture.h"
#elif _WIN32
    // #include "core/screenCaptures/WinScreenCapture.h"
#endif

std::unique_ptr<IScreenCapture> ScreenCaptureFactory::create() {
#ifdef __APPLE__
    return std::make_unique<MacScreenCapture>();
#elif _WIN32
    // return std::make_unique<WinScreenCapture>();
    return nullptr;
#else
    return nullptr;
#endif
}