#include "core/CoreEngine.h"
#include "core/ConfigManager.h"
#include "strategies/EdgeDetections/BaseEdgeDetectionStrategy.h"
#include "strategies/StrategiesFactory.h"

void CoreEngine::setStrategy(std::string newStrategy) {
    m_currentStrategy = StrategiesFactory::createStrategy(newStrategy);
    m_activeProperties = m_currentStrategy->getProperties();
}

void CoreEngine::processFrameToBuffer(
    const cv::Mat& frame,
    std::vector<ImageUtils::Pixel>& debug_buffer,
    int output_width,
    int output_height
) {
    if (m_currentStrategy) {
        m_currentStrategy->render(frame, debug_buffer, output_width, output_height);
    }
}
bool CoreEngine::setupEngineConfigs() {
    auto strategy = ConfigManager::getValFromSettings("render_strategy");
    this->setStrategy(strategy);
    if (auto edgeStrategy = dynamic_cast<BaseEdgeDetectionStrategy*>(m_currentStrategy.get())) {
        std::string savedChar = ConfigManager::getValFromSettings("fill_char");
        char fill = savedChar.empty() ? ' ' : savedChar[0];
        edgeStrategy->setFillChar(fill);
    }

    auto config_fpsString = ConfigManager::getValFromSettings("target_fps");
    m_targetFPS = ConfigManager::is_number(config_fpsString) ? std::stod(config_fpsString) : 0;
    m_measure_performance = ConfigManager::GetBoolFromSettings("measure_performance");
    return true;
}