#include "strategies/AbstractRenderStrategy.h"

std::vector<Property> AbstractRenderStrategy::getProperties()
{
    std::vector<Property> properties;
    properties.push_back({"Use Color", PropertyType::BOOLEAN, m_useColor ? 1.0f : 0.0f, 1.0f, 0.0f, 1.0f});
    
    if (m_useColor) 
    {
        properties.push_back({"8-bit Colors", PropertyType::BOOLEAN, m_use8BitColor ? 1.0f : 0.0f, 1.0f, 0.0f, 1.0f});
        properties.push_back({"Color Tolerance", PropertyType::FLOAT, m_colorTolerance, 5.0f, 0.0f, 150.0f});
    }
    return properties;
}

float AbstractRenderStrategy::getProperty(const std::string &name)
{
    if (name == "Use Color") return m_useColor ? 1.0f : 0.0f;
    if (name == "8-bit Colors") return m_use8BitColor ? 1.0f : 0.0f;
    if (name == "Color Tolerance") return m_colorTolerance;
    return 0.0f; 
}

void AbstractRenderStrategy::setProperty(const Property property)
{
    if (property.name == "Use Color") m_useColor = (property.currentValue > 0.5f);
    else if (property.name == "8-bit Colors") m_use8BitColor = (property.currentValue > 0.5f);
    else if (property.name == "Color Tolerance") m_colorTolerance = property.currentValue;
}