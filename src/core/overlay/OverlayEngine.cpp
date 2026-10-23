#include "core/overlay/OverlayEngine.h"
#include "strategies/ImageUtilities.h"
#include <cstddef>

bool OverlayEngine::init() {
    m_windowManager.init();
    return true;
}
void OverlayEngine::play() {
    m_isEngineRunning = true;
    m_frameProvider->start();
    while (m_isEngineRunning) {
        // 1. Zpracování událostí (jinak okno zamrzne)
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
        /*
                // 4. Odeslání hotového ASCII pole na grafickou kartu
                m_windowManager.renderBuffer( tvůj buffer a případně barvy );

                // 5. Omezení rychlosti (např. na 60 FPS), ať to Macu nesežere celou baterku
                syncFramerate();
                */
    }
    m_frameProvider->stop();
}