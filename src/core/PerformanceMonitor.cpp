#include "core/PerformanceMonitor.h"

void PerformanceMonitor::startFrame()
{
    m_frameStart = std::chrono::high_resolution_clock::now();
}

void PerformanceMonitor::endFrame()
{
    auto frameEnd = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(frameEnd - m_frameStart).count();
    
    double currentFrameTimeMs = duration / 1000.0;
    double currentFps = (currentFrameTimeMs > 0.0) ? (1000.0 / currentFrameTimeMs) : 999.0;

    if (m_avgFrameTimeMs == 0.0) {
        m_avgFrameTimeMs = currentFrameTimeMs;
        m_avgTheoreticalFps = currentFps;
    } else {
        m_avgFrameTimeMs = (SMOOTHING_FACTOR * currentFrameTimeMs) + ((1.0 - SMOOTHING_FACTOR) * m_avgFrameTimeMs);
        m_avgTheoreticalFps = (SMOOTHING_FACTOR * currentFps) + ((1.0 - SMOOTHING_FACTOR) * m_avgTheoreticalFps);
    }
}

/*std::string PerformanceMonitor::getStatsString() const
{
   return std::to_string("Max FPS: {:.0f} | Frame time: {:.2f} ms", m_avgTheoreticalFps, m_avgFrameTimeMs);
}*/