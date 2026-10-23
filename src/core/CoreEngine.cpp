#include "core/CoreEngine.h"
#include "strategies/StrategiesFactory.h"

void CoreEngine::setStrategy(std::string newStrategy)
{
    m_currentStrategy = StrategiesFactory::createStrategy(newStrategy);
    m_activeProperties = m_currentStrategy->getProperties();
}

void CoreEngine::processFrameToBuffer(const cv::Mat &frame, std::vector<ImageUtils::Pixel> &debug_buffer, int output_width, int output_height)
{
    if (m_currentStrategy)
    {
        m_currentStrategy->render(frame, debug_buffer, output_width, output_height);
    }
}
