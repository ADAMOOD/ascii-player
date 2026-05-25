#pragma once
#include <opencv2/opencv.hpp>
#include "../ui/Property.h"
#include "ImageUtilits.h"
#include <string>

class IRenderStrategy
{

public:

    virtual void render(const cv::Mat &inputFrame, std::vector<ImageUtils::Pixel> &outBuffer, int width, int height) = 0;

    // nessesary
    virtual ~IRenderStrategy() = default;

    // Dynamic property changes
    virtual std::vector<Property> getProperties()
    {
        std::vector<Property> properties;
        properties.push_back({"Use Color", PropertyType::BOOLEAN, m_useColor ? 1.0f : 0.0f, 1.0f, 0.0f, 1.0f});
        return properties;
    }

    virtual float getProperty(const std::string &name)
    {
        if (name == "Use Color")
            return m_useColor ? 1.0f : 0.0f;
        return 0.0f; // Neznámá vlastnost
    }

    virtual void setProperty(const Property property)
    {
        if (property.name == "Use Color")
            m_useColor = (property.currentValue > 0.5f);
    }

protected:

    bool m_useColor = false;

    uchar calculateAverage(cv::Vec3b chanels)
    {
        return static_cast<uchar>((chanels[0] + chanels[1] + chanels[2]) / 3);
    }
};