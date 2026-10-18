#pragma once
#include <chrono>
#include <string>
#include <functional>

class PerformanceMonitor
{
public:
    PerformanceMonitor() { m_lastUiUpdateTime = std::chrono::high_resolution_clock::now(); }

    void startFrame();
    void endFrame();

    double getFrameTimeMs() const { return m_UI_avgFrameTimeMs; }
    double getTheoreticalFps() const { return m_UI_avgTheoreticalFps; }

    std::string getStatsString() const;

    void setUpdateCallback(std::function<void(double, double)> callback) { m_onUpdateCallback = callback; }

private:
    std::function<void(double fps, double ms)> m_onUpdateCallback;

    std::chrono::time_point<std::chrono::high_resolution_clock> m_frameStart;

    std::chrono::time_point<std::chrono::high_resolution_clock> m_lastUiUpdateTime;

    bool updateUIStats(const double timeSinceLastUpdate);

    double m_avgFrameTimeMs = 0.0;
    double m_UI_avgFrameTimeMs = 0.0;

    double m_avgTheoreticalFps = 0.0;
    double m_UI_avgTheoreticalFps = 0.0;

    const double SMOOTHING_FACTOR = 0.1;
};