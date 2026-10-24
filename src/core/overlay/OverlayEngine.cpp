#include "core/overlay/OverlayEngine.h"
#include "core/overlay/screenCaptures/ScreenCaptureFactory.h"
#include "ui/DebugWindow.h"
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

    setupEngineConfigs();
    m_debugWindow.init(4.0 / 3.0);
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

        cv::Mat frame = m_frameProvider->consumeFrame();
        if (frame.empty())
            continue;

        int asciiCols, asciiRows;
        m_windowManager.calculateGridSize(asciiCols, asciiRows);

        m_windowManager.getBuffer().assign(asciiCols * asciiRows, {' ', {0, 0, 0}, {0, 0, 0}});

        processFrameToBuffer(frame, m_windowManager.getBuffer(), asciiCols, asciiRows);

        m_windowManager.renderBuffer(asciiCols, asciiRows);

        if (m_currentStrategy->getProperty("Show Debug Window") > 0.5f) {
            m_debugWindow.showDebugWindow(
                m_currentStrategy->getDebugFrame(), frame, m_currentStrategy->getName()
            );
        } else {
            m_debugWindow.destroy();
        }
    }
    m_debugWindow.destroy();
    m_frameProvider->stop();
}