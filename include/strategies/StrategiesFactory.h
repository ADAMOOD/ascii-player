#pragma once
#include <memory>
#include <vector>
#include <string>
#include "strategies/IRenderStrategy.h"

/**
 * @class StrategiesFactory
 * @brief Factory class for instantiating rendering strategies.
 * * Follows the Factory Design Pattern to decouple the creation of specific 
 * strategy objects from the classes that use them (like AsciiEngine).
 */
class StrategiesFactory
{
private:
    inline static const std::vector<std::string> _strategies = {
        "Naive Grayscale",
        "Perceptual Grayscale",
        "Lightness Grayscale",
        "Value Grayscale",
        "Edge Detection (Sobel)",
        "Edge Detection (Canny)",
        "Edge/Grayscale (Comic)"
    };

public:
    /**
     * @brief Returns a list of all available rendering strategies.
     * @return Constant vector of strategy names suitable for UI dropdowns.
     */
    static const std::vector<std::string> getAvailableStrategies();

    /**
     * @brief Creates and returns a unique pointer to the requested rendering strategy.
     * @param selectedStrategy The exact name of the strategy (matching the list).
     * @return std::unique_ptr<IRenderStrategy> Polymorphic pointer to the created strategy.
     * Defaults to PerceptualGrayscaleStrategy if the name is not found.
     */
    static std::unique_ptr<IRenderStrategy> createStrategy(const std::string &selectedStrategy);
};