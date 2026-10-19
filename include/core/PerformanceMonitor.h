#pragma once
#include <chrono>
#include <string>
#include <functional>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>
#include <iostream>
#include <fstream>
#include "strategies/IRenderStrategy.h"

struct PerformanceRecord
{
    int frameNum;
    double videoTimeMs;

    double avg_frameMs;
    double avgTheoreticalFps;

    double raw_timeMs;
    double raw_fps;
};

class PerformanceMonitor
{
public:
    double getRawLastFrameTimeMs() const { return m_lastRawFrameTimeMs; }
    PerformanceMonitor();
    ~PerformanceMonitor();

    void startFrame();
    void endFrame();

    int getFrameCount() const { return m_frameCounter; }
    double getFrameTimeMs() const { return m_UI_avgFrameTimeMs; }
    double getTheoreticalFps() const { return m_UI_avgTheoreticalFps; }

    std::string getStatsString() const;

    void setUpdateCallback(std::function<void(double, double)> callback) { m_onUpdateCallback = callback; }
    void initLogging(const std::string p_recordDir, const std::string strategy, const std::string video_stream);

private:
    std::function<void(double fps, double ms)> m_onUpdateCallback;

    std::chrono::time_point<std::chrono::high_resolution_clock> m_frameStart;

    std::chrono::time_point<std::chrono::high_resolution_clock> m_lastUiUpdateTime;

    bool updateUIStats(const double timeSinceLastUpdate);

    // for fps synchronization in engine
    double m_lastRawFrameTimeMs;
    double m_lastRawFps;

    double m_avgFrameTimeMs = 0.0;
    double m_UI_avgFrameTimeMs = 0.0;

    double m_avgTheoreticalFps = 0.0;
    double m_UI_avgTheoreticalFps = 0.0;

    const double SMOOTHING_FACTOR = 0.1;

    std::ofstream m_logFile;
    int m_frameCounter = 0;

    std::string record_to_string(PerformanceRecord pr);

    // producer-consumer writing measured performance
    std::chrono::time_point<std::chrono::high_resolution_clock> m_videoStart;
    std::queue<PerformanceRecord> m_logQueue;
    std::mutex m_queueMutex;
    std::condition_variable m_cv;
    std::atomic<bool> m_isLogging;
    std::thread m_loggerThread;
    void loggingTask();
    void pushRecord(std::chrono::time_point<std::chrono::high_resolution_clock> now);
};