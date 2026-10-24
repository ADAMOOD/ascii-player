#include "core/overlay/OverlayEngine.h"
#include "core/ConfigManager.h"
#include "core/overlay/screenCaptures/ScreenCaptureFactory.h"
#include "strategies/EdgeDetections/BaseEdgeDetectionStrategy.h"
#include "strategies/ImageUtilities.h"
#include <cstddef>

bool OverlayEngine::init() {
    m_frameProvider = ScreenCaptureFactory::create();
    if (!m_frameProvider) {
        std::cerr << "[ERROR] Unable to create frame provider \n";
        return false;
    }

    if (!m_windowManager.init()) {
        std::cerr << "[ERROR] Unable to initialize window!\n";
        return false;
    }
    auto strategy = ConfigManager::getValFromSettings("render_strategy");
    this->setStrategy(strategy);
    if (auto edgeStrategy = dynamic_cast<BaseEdgeDetectionStrategy *>(m_currentStrategy.get())) {
        std::string savedChar = ConfigManager::getValFromSettings("fill_char");
        char fill = savedChar.empty() ? ' ' : savedChar[0];
        edgeStrategy->setFillChar(fill);
    }
    return true;
}
void OverlayEngine::play() {
    m_isEngineRunning = true;
    m_frameProvider->start();
    while (m_isEngineRunning) {
        if (!m_windowManager.pollEvents()) {

            m_isEngineRunning = false;
            continue;
        }
        cv::Mat frame = m_frameProvider->getNextFrame();
        if (frame.empty())
            continue;
        int width, height;
        m_windowManager.getSize(&width, &height);
        std::vector<ImageUtils::Pixel> frameBuffer_debug_tmp;
        processFrameToBuffer(frame, frameBuffer_debug_tmp, width, height);
        m_windowManager.renderBuffer(frameBuffer_debug_tmp, width, height);
    }
    m_frameProvider->stop();
}