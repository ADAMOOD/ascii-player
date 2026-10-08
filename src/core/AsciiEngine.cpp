#include "core/AsciiEngine.h"
#include <iostream>
#include "core/ConfigManager.h"
#include "strategies/StrategiesFactory.h"
#include "strategies/EdgeDetections/BaseEdgeDetectionStrategy.h"
#include "strategies/ImageUtilities.h"
#include <opencv2/core/utils/logger.hpp>


bool AsciiEngine::setupEngineConfigs()
{
    auto strategy = ConfigManager::getValFromSettings("render_strategy");
    this->setStrategy(strategy);

    if (auto edgeStrategy = dynamic_cast<BaseEdgeDetectionStrategy *>(m_currentStrategy.get()))
    {
        std::string savedChar = ConfigManager::getValFromSettings("fill_char");
        char fill = savedChar.empty() ? ' ' : savedChar[0];
        edgeStrategy->setFillChar(fill);
    }

    m_menuStartIndex = 0;
    m_selectedPropertyIndex = 0;
    double origWidth;
    double origHeight;

    m_streamManager.getOriginalSize(origWidth, origHeight);

    if (origWidth <= 0 || origHeight <= 0)
    {
        origWidth = 640;
        origHeight = 480;
    }

    double aspectRatio = origWidth / origHeight;

    m_displayManager.init(aspectRatio);
    m_displayManager.updateTerminalSize();
    return true;
}

bool AsciiEngine::init(const std::string &videoPath)
{
    if (!m_streamManager.init(videoPath))
    {
        std::cerr << "[ERROR] Stream manager ERROR " << std::endl;
        return false;
    }
    return setupEngineConfigs();
}

bool AsciiEngine::init()
{
    if (!m_streamManager.init())
    {
        std::cerr << "[ERROR] Stream manager ERROR " << std::endl;
        return false;
    }
    return setupEngineConfigs();
}


void AsciiEngine::play()
{
    m_inputHandler.init();
    m_isEngineRunning = true;
    m_streamManager.start();
    std::cout << "\x1b[2J\x1b[?25l";

    while (m_isEngineRunning)
    {
        m_displayManager.updateTerminalSize();

        m_Pmonitor.startFrame();
        cv::Mat frame = m_streamManager.getNextFrame();

        if (frame.empty())
        {
            if (!m_isEngineRunning)
                break;
            checkUserInput();
            continue;
        }

        processFrameToBuffer(frame);
        m_Pmonitor.endFrame();

        bool useColor = m_currentStrategy->getProperty("Use Color") > 0.5f;
        bool use8Bit = m_currentStrategy->getProperty("8-bit Colors") > 0.5f;
        int tolerance = static_cast<int>(m_currentStrategy->getProperty("Color Tolerance"));

        m_displayManager.renderBuffer(useColor, use8Bit, tolerance);


        if (m_currentStrategy->getProperty("Show Debug Window") > 0.5f)
        {
            m_displayManager.showDebugWindow(m_currentStrategy->getDebugFrame(), frame, m_currentStrategy->getName());
        }

        if (!m_activeProperties.empty() || m_currentStrategy)
        {
            m_displayManager.renderHUD(m_activeProperties, m_selectedPropertyIndex, m_menuStartIndex);
        }
        m_displayManager.printMonitoredStats(m_Pmonitor);
        syncFramerate();
        checkUserInput();
    }

    m_inputHandler.shutdown();
    m_streamManager.stop();

    std::cout << "\x1b[?25h";
}

void AsciiEngine::setStrategy(std::string newStrategy)
{
    m_currentStrategy = std::move(StrategiesFactory::createStrategy(newStrategy));
    m_activeProperties = m_currentStrategy->getProperties();
}

void AsciiEngine::processFrameToBuffer(const cv::Mat &frame)
{
    if (m_currentStrategy)
    {
        m_currentStrategy->render(frame, m_displayManager.getBuffer(), m_displayManager.getWidth(), m_displayManager.getHeight());
    }
}

void AsciiEngine::syncFramerate()
{
    // this will be implemented in the future to sync with the video's original framerate
    std::this_thread::sleep_for(std::chrono::milliseconds(33));
}

void AsciiEngine::checkUserInput()
{
    InputAction action = m_inputHandler.pollInput();
    switch (action)
    {
    case InputAction::QUIT:
        m_isEngineRunning = false;
        break;
    case InputAction::NEXT_PROPERTY:
    {
        if (m_selectedPropertyIndex > 0)
            m_selectedPropertyIndex--;
        break;
    }

    case InputAction::PREV_PROPERTY:
    {
        if (!m_activeProperties.empty() && m_selectedPropertyIndex < static_cast<int>(m_activeProperties.size()) - 1)
            m_selectedPropertyIndex++;
        break;
    }

    case InputAction::DECREASE_VALUE:
    case InputAction::INCREASE_VALUE:
    {
        if (m_activeProperties.empty())
            break;

        Property prop = m_activeProperties[m_selectedPropertyIndex];
        prop.ShiftedValue(action == InputAction::INCREASE_VALUE);
        m_currentStrategy->setProperty(prop);
        m_activeProperties = m_currentStrategy->getProperties();

        if (m_activeProperties.empty())
        {
            m_selectedPropertyIndex = 0;
        }
        else if (m_selectedPropertyIndex >= static_cast<int>(m_activeProperties.size()))
        {
            m_selectedPropertyIndex = static_cast<int>(m_activeProperties.size()) - 1;
        }
        break;
    }
    }
}
