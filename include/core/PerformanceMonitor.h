#pragma once
#include <chrono>
#include <string>

class PerformanceMonitor
{
public:
    PerformanceMonitor() = default;

    void startFrame();
    void endFrame();

    double getFrameTimeMs() const { return m_avgFrameTimeMs; }
    double getTheoreticalFps() const { return m_avgTheoreticalFps; }

    std::string getStatsString() const;

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_frameStart;
    
    double m_avgFrameTimeMs = 0.0;
    double m_avgTheoreticalFps = 0.0;
    
    const double SMOOTHING_FACTOR = 0.1; 
};