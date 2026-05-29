#include "strategies/StrategiesFactory.h"

#include "strategies/Grayscale/NaiveGrayscaleStrategy.h"
#include "strategies/Grayscale/PerceptualGrayscaleStrategy.h"
#include "strategies/Grayscale/LightnessGrayscaleStrategy.h"
#include "strategies/Grayscale/ValueGrayscaleStrategy.h"
#include "strategies/EdgeDetections/SobelEdgeDetectionStrategy.h"
#include "strategies/EdgeDetections/CannyEdgeDetectionStrategy.h"
#include "strategies/EdgeDetections/ComicEdgeDetectionStrategy.h"

const std::vector<std::string> StrategiesFactory::getAvailableStrategies()
{
    return _strategies;
}

std::unique_ptr<IRenderStrategy> StrategiesFactory::createStrategy(const std::string &selectedStrategy)
{
    if (selectedStrategy == _strategies[0])
    {
        return std::make_unique<NaiveGrayscaleStrategy>();
    }
    if (selectedStrategy == _strategies[1])
    {
        return std::make_unique<PerceptualGrayscaleStrategy>();
    }
    if (selectedStrategy == _strategies[2])
    {
        return std::make_unique<LightnessGrayscaleStrategy>();
    }
    if (selectedStrategy == _strategies[3])
    {
        return std::make_unique<ValueGrayscaleStrategy>();
    }
    if (selectedStrategy == _strategies[4])
    {
        return std::make_unique<SobelEdgeDetectionStrategy>();
    }
    if (selectedStrategy == _strategies[5])
    {
        return std::make_unique<CannyEdgeDetectionStrategy>();
    }
    if (selectedStrategy == _strategies[6])
    {
        return std::make_unique<ComicEdgeDetectionStrategy>();
    }

    // Fallback
    return std::make_unique<PerceptualGrayscaleStrategy>();
}