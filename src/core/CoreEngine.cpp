#include "core/CoreEngine.h"
#include "strategies/StrategiesFactory.h"

void CoreEngine::setStrategy(std::string newStrategy)
{
    m_currentStrategy = StrategiesFactory::createStrategy(newStrategy);
    m_activeProperties = m_currentStrategy->getProperties();
}
