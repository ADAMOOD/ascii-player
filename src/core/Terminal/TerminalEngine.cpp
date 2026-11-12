#include "core/terminal/TerminalEngine.h"
#include "core/ConfigManager.h"
#include "core/SaliencyModel.h"
#include "ui/DebugWindow.h"
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/utils/logger.hpp>
#include <thread>

bool TerminalEngine::init(const std::string& videoPath) {
    if (!m_streamManager.init(videoPath)) {
        std::cerr << "[ERROR] Stream manager ERROR " << std::endl;
        return false;
    }

    m_menuStartIndex = 0;
    m_selectedPropertyIndex = 0;
    double origWidth;
    double origHeight;

    m_streamManager.getOriginalSize(origWidth, origHeight);

    if (origWidth <= 0 || origHeight <= 0) {
        origWidth = 640;
        origHeight = 480;
    }

    double aspectRatio = origWidth / origHeight;

    m_displayManager.init(aspectRatio);
    m_displayManager.updateTerminalSize();
    m_debugWindow.init(aspectRatio);
    return setupEngineConfigs();
}

bool TerminalEngine::init() {
    if (!m_streamManager.init()) {
        std::cerr << "[ERROR] Stream manager ERROR " << std::endl;
        return false;
    }
    m_menuStartIndex = 0;
    m_selectedPropertyIndex = 0;
    double origWidth;
    double origHeight;

    m_streamManager.getOriginalSize(origWidth, origHeight);

    if (origWidth <= 0 || origHeight <= 0) {
        origWidth = 640;
        origHeight = 480;
    }

    double aspectRatio = origWidth / origHeight;

    m_displayManager.init(aspectRatio);
    m_displayManager.updateTerminalSize();
    m_debugWindow.init(aspectRatio);
    return setupEngineConfigs();
}

void TerminalEngine::play() {
    SaliencyModel model;
    if (!model.init("../model_fixed.onnx")) {
        std::cerr << "[ERROR] Saliency model not loaded (../model.onnx)\n";
        return;
    }

    double srcW = 0, srcH = 0;
    m_streamManager.getOriginalSize(srcW, srcH);
    const double srcAspect = (srcW > 0 && srcH > 0) ? srcW / srcH : 4.0 / 3.0;

    DebugWindow modelView;
    if (!modelView.init(srcAspect, "MODEL", 520, 100)) {
        std::cerr << "[ERROR] unable to init model view window\n";
        return;
    }

    m_inputHandler.init();
    m_isEngineRunning = true;
    m_streamManager.start();
    m_nextFrameTime = std::chrono::steady_clock::time_point{};
    std::cout << "\x1b[2J\x1b[?25l";

    if (m_measure_performance) {
        auto strategy = ConfigManager::getValFromSettings("render_strategy");
        std::string videoStream;
        if (ConfigManager::GetBoolFromSettings("use_webcam")) {
            videoStream = "webcam";
        } else {
            videoStream = ConfigManager::getFileName(ConfigManager::loadVideoPath());
        }
        m_Pmonitor.initLogging("../performance/", strategy, videoStream);
    }

    // sending real fps tohud
    auto statsLambda = [this](double /*computeFps*/, double ms) {
        m_displayManager.setStatsData(m_Pmonitor.getRealFps(), ms);
    };
    m_Pmonitor.setUpdateCallback(statsLambda);

    while (m_isEngineRunning) {
        m_Pmonitor.startFrame();
        m_displayManager.updateTerminalSize();

        cv::Mat frame = m_streamManager.consumeFrame();

        if (frame.empty()) {
            if (!m_isEngineRunning)
                break;
            checkUserInput();
            continue;
        }
        cv::Mat processedFrame;
        model.getSegmentationFrame(frame, processedFrame, frame.cols, frame.rows);

        processFrameToBuffer(
            frame,
            m_displayManager.getBuffer(),
            m_displayManager.getWidth(),
            m_displayManager.getHeight()
        );

        bool useColor = m_currentStrategy->getProperty("Use Color") > 0.5f;
        bool use8Bit = m_currentStrategy->getProperty("8-bit Colors") > 0.5f;
        int tolerance = static_cast<int>(m_currentStrategy->getProperty("Color Tolerance"));

        m_displayManager.renderBuffer(useColor, use8Bit, tolerance);

        if (m_currentStrategy->getProperty("Show Debug Window") > 0.5f) {
            m_debugWindow.showDebugWindow(
                m_currentStrategy->getDebugFrame(), frame, m_currentStrategy->getName()
            );
        } else {
            m_debugWindow.destroy();
        }
        
        modelView.showDebugWindow(processedFrame, frame, "saliency");

        if (!m_activeProperties.empty() || m_currentStrategy) {
            m_displayManager.renderHUD(
                m_activeProperties, m_selectedPropertyIndex, m_menuStartIndex
            );
        }
        m_Pmonitor.endFrame(); // time of frame procedure
        syncFramerate();
        m_Pmonitor.markPresented(); // real tempo for synchronisation
        checkUserInput();
    }

    modelView.destroy();
    m_debugWindow.destroy();
    m_inputHandler.shutdown();
    m_streamManager.stop();

    std::cout << "\x1b[?25h";
}
void TerminalEngine::syncFramerate() {
    using clock = std::chrono::steady_clock;

    // target fps FPS: config > stream FPS > Webcam (AVFoundation)
    // returning 0/NaN; 1000.0/0 =  inf -> long
    double fps = (m_targetFPS > 0) ? m_targetFPS : m_streamManager.getStreamFps();
    if (!(fps >= 1.0) || fps > 1000.0) {
        fps = 30.0;
    }

    const auto frameDuration =
        std::chrono::duration_cast<clock::duration>(std::chrono::duration<double>(1.0 / fps));

    const auto now = clock::now();
    if (m_nextFrameTime == clock::time_point{}) {
        m_nextFrameTime = now; // first frame
    }
    m_nextFrameTime += frameDuration;

    if (now < m_nextFrameTime) {
        std::this_thread::sleep_until(m_nextFrameTime);
    } else if (now - m_nextFrameTime > frameDuration * 5) {
        // if latency
        m_nextFrameTime = now;
    }
}

void TerminalEngine::checkUserInput() {
    InputAction action = m_inputHandler.pollInput();
    switch (action) {
    // maybe implemented in future
    case InputAction::NONE:
    case InputAction::TOGGLE:
        break;
    case InputAction::QUIT:
        m_isEngineRunning = false;
        break;
    case InputAction::NEXT_PROPERTY: {
        if (m_selectedPropertyIndex > 0)
            m_selectedPropertyIndex--;
        break;
    }

    case InputAction::PREV_PROPERTY: {
        if (!m_activeProperties.empty() &&
            m_selectedPropertyIndex < static_cast<int>(m_activeProperties.size()) - 1)
            m_selectedPropertyIndex++;
        break;
    }

    case InputAction::DECREASE_VALUE:
    case InputAction::INCREASE_VALUE: {
        if (m_activeProperties.empty())
            break;

        Property prop = m_activeProperties[m_selectedPropertyIndex];
        prop.ShiftedValue(action == InputAction::INCREASE_VALUE);
        m_currentStrategy->setProperty(prop);
        m_activeProperties = m_currentStrategy->getProperties();

        if (m_activeProperties.empty()) {
            m_selectedPropertyIndex = 0;
        } else if (m_selectedPropertyIndex >= static_cast<int>(m_activeProperties.size())) {
            m_selectedPropertyIndex = static_cast<int>(m_activeProperties.size()) - 1;
        }
        break;
    }
    }
}