#pragma once
#include <opencv2/opencv.hpp>
#include "ui/Property.h"
#include "strategies/ImageUtilits.h"
#include <vector>
#include <string>

/**
 * @interface IRenderStrategy
 * @brief The absolute core interface for all ASCII rendering algorithms.
 * @details 
 * Defines a unified contract (Strategy Pattern) for converting raw OpenCV frames 
 * into a terminal-friendly ASCII format. Any new rendering effect must implement this interface.
 */
class IRenderStrategy
{
public:
    virtual ~IRenderStrategy() = default;

    /**
     * @brief Core rendering method. Converts an OpenCV frame into ASCII pixels.
     * @param inputFrame The raw, unprocessed image/frame directly from the camera or video.
     * @param outBuffer Output 1D array representing the 2D terminal screen.
     * @param width The target width of the terminal window in characters.
     * @param height The target height of the terminal window in characters.
     */
    virtual void render(const cv::Mat &inputFrame, std::vector<ImageUtils::Pixel> &outBuffer, int width, int height) = 0;
    
    /**
     * @brief Retrieves all adjustable properties for this specific strategy.
     * @return A vector of Property objects that the UI will generate sliders/toggles for.
     */
    virtual std::vector<Property> getProperties() = 0;

    /**
     * @brief Retrieves the value of a property by its exact name.
     * @param name The exact string name of the property.
     * @return The current float value of the requested property.
     */
    virtual float getProperty(const std::string &name) = 0;

    /**
     * @brief Updates a specific property's value within the strategy.
     * @param property The modified property object received from the user interface.
     */
    virtual void setProperty(const Property property) = 0;
};