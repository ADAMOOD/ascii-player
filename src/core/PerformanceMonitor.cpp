#include "core/PerformanceMonitor.h"
#include <ctime>

PerformanceMonitor::PerformanceMonitor()
{
    m_lastUiUpdateTime = std::chrono::high_resolution_clock::now();
    m_isLogging = false;
}

PerformanceMonitor::~PerformanceMonitor()
{
    if (m_isLogging)
    {
        m_isLogging = false;
        m_cv.notify_one();
        m_loggerThread.join();
        if (m_logFile.is_open())
        {
            m_logFile << "---,---,---,---,---,---\n";
            m_logFile << "END OF RECORD,NORMAL SHUTDOWN\n";
            m_logFile.close();
        }
    }
}

void PerformanceMonitor::initLogging(const std::string p_recordDir, const std::string strategy, const std::string video_stream)
{
    //to ensure dir existence 
    std::filesystem::create_directories(p_recordDir);
    m_videoStart = std::chrono::high_resolution_clock::now();

    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream fileName;

    fileName << p_recordDir << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S")
             << " - {"
             << strategy
             << "}_("
             << video_stream
             << ").csv";

    m_logFile.open(fileName.str());
    if (m_logFile.is_open())
    {
        m_logFile << "Frame,VideoTimeMs,RawTimeMs,RawFPS,AvgTimeMs,AvgFPS\n";
    }

    m_isLogging = true;
    m_loggerThread = std::thread(&PerformanceMonitor::loggingTask, this);
}

void PerformanceMonitor::startFrame()
{
    m_frameStart = std::chrono::high_resolution_clock::now();
}

void PerformanceMonitor::endFrame()
{
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - m_frameStart).count();

    m_frameCounter++;

    m_lastRawFrameTimeMs = duration / 1000.0;
    m_lastRawFps = (m_lastRawFrameTimeMs > 0.0) ? (1000.0 / m_lastRawFrameTimeMs) : 999.0;

    if (m_avgFrameTimeMs == 0.0)
    {
        m_avgFrameTimeMs = m_lastRawFrameTimeMs;
        m_avgTheoreticalFps = m_lastRawFps;
    }
    else
    {
        m_avgFrameTimeMs = (SMOOTHING_FACTOR * m_lastRawFrameTimeMs) + ((1.0 - SMOOTHING_FACTOR) * m_avgFrameTimeMs);
        m_avgTheoreticalFps = (SMOOTHING_FACTOR * m_lastRawFps) + ((1.0 - SMOOTHING_FACTOR) * m_avgTheoreticalFps);
    }

    auto timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastUiUpdateTime).count();
    updateUIStats(timeSinceLastUpdate);

    if (m_logFile.is_open() && m_isLogging)
    {
        pushRecord(now);
    }
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

void PerformanceMonitor::pushRecord(std::chrono::time_point<std::chrono::high_resolution_clock> now)
{
    PerformanceRecord record;
    record.frameNum = m_frameCounter;
    record.videoTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_videoStart).count(); // since video started
    record.avg_frameMs = m_avgFrameTimeMs;
    record.avgTheoreticalFps = m_avgTheoreticalFps;
    record.raw_fps = m_lastRawFps;
    record.raw_timeMs = m_lastRawFrameTimeMs;

    std::lock_guard<std::mutex> lock(m_queueMutex);
    m_logQueue.push(record);
    m_cv.notify_one();
}

std::string PerformanceMonitor::record_to_string(PerformanceRecord pr)
{
    return std::to_string(pr.frameNum) + "," +
           std::to_string(pr.videoTimeMs) + "," +
           std::to_string(pr.raw_timeMs) + "," +
           std::to_string(pr.raw_fps) + "," +
           std::to_string(pr.avg_frameMs) + "," +
           std::to_string(pr.avgTheoreticalFps) + "\n";
}

void PerformanceMonitor::loggingTask()
{
    while (m_isLogging || !m_logQueue.empty())
    {
        PerformanceRecord recordToSave;

        {
            std::unique_lock<std::mutex> lock(m_queueMutex);

            // thread sleeps till the producer pushes into front
            m_cv.wait(lock, [this]()
                      { return !m_logQueue.empty() || !m_isLogging; });

            // if notified but queue empty program is about to end
            if (m_logQueue.empty())
                break;

            recordToSave = m_logQueue.front();
            m_logQueue.pop();
        }

        if (m_logFile.is_open())
        {
            m_logFile << record_to_string(recordToSave);
        }
    }
}
