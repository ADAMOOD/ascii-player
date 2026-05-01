#pragma once
#include <memory>
#include <vector>
#include "NaiveGrayscaleStrategy.h"
#include "PerceptualGrayscaleStrategy.h"

class StrategiesFactory
{
private:
    inline static const std::vector<std::string> _strategies = {"Naive Grayscale", "Perceptual Grayscale"};

public:
    static const std::vector<std::string> getAvailableStrategies()
    {
        return _strategies;
    }
    static std::unique_ptr<IRenderStrategy> createStrategy(const std::string &selecctedStrategy)
    {
        if (selecctedStrategy == _strategies[0])
        {
            return std::make_unique<NaiveGrayscaleStrategy>();
        }
        if (selecctedStrategy == _strategies[1])
        {
            return std::make_unique<PerceptualGrayscaleStrategy>();
        }
        
        return std::make_unique<PerceptualGrayscaleStrategy>();
    }
};