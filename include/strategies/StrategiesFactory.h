#pragma once
#include <memory>
#include <vector>
#include "NaiveGrayscaleStrategy.h"
#include "PerceptualGrayscaleStrategy.h"
#include "LightnessGrayscaleStrategy.h"
#include "ValueGrayscaleStrategy.h"
#include "SobelEdgeDetectionStrategy.h"

class StrategiesFactory
{
private:
    inline static const std::vector<std::string> _strategies = {"Naive Grayscale", "Perceptual Grayscale", "Lightness Grayscale", "Value Grayscale","Edge Detection (Sobel)"};

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
        if (selecctedStrategy == _strategies[2])
        {
            return std::make_unique<LightnessGrayscaleStrategy>();
        }
        if(selecctedStrategy == _strategies[3])
        {
            return std::make_unique<ValueGrayscaleStrategy>();
        }
        if(selecctedStrategy == _strategies[4])
        {
            return std::make_unique<SobelEdgeDetectionStrategy>();
        }


        return std::make_unique<PerceptualGrayscaleStrategy>();
    }
};