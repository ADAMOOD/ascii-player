#include "core/PerformanceMonitor.h"

void PerformanceMonitor::startFrame()
{
    m_frameStart = std::chrono::high_resolution_clock::now();
}

void PerformanceMonitor::endFrame()
{
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - m_frameStart).count();

    double currentFrameTimeMs = duration / 1000.0;
    double currentFps = (currentFrameTimeMs > 0.0) ? (1000.0 / currentFrameTimeMs) : 999.0;

    //for fps synchronization in engine
    m_lastRawFrameTimeMs = currentFrameTimeMs;

    if (m_avgFrameTimeMs == 0.0)
    {
        m_avgFrameTimeMs = currentFrameTimeMs;
        m_avgTheoreticalFps = currentFps;
    }
    else
    {
        m_avgFrameTimeMs = (SMOOTHING_FACTOR * currentFrameTimeMs) + ((1.0 - SMOOTHING_FACTOR) * m_avgFrameTimeMs);
        m_avgTheoreticalFps = (SMOOTHING_FACTOR * currentFps) + ((1.0 - SMOOTHING_FACTOR) * m_avgTheoreticalFps);
    }

    auto timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastUiUpdateTime).count();
    updateUIStats(timeSinceLastUpdate);
}

bool PerformanceMonitor::updateUIStats(const double timeSinceLastUpdate)
{
    if (timeSinceLastUpdate > 1000)
    {
        m_lastUiUpdateTime = std::chrono::high_resolution_clock::now();
        m_UI_avgFrameTimeMs = m_avgFrameTimeMs;
        m_UI_avgTheoreticalFps = m_avgTheoreticalFps;
        if (m_onUpdateCallback)
        {
            // set callback values
            m_onUpdateCallback(m_UI_avgTheoreticalFps, m_avgFrameTimeMs);
        }
        return true;
    }
    return false;
}
