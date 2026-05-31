#pragma once
#include "strategies/IRenderStrategy.h"

/**
 * @class AbstractRenderStrategy
 * @brief Intermediate base class managing shared color settings for all strategies.
 * @details
 * implementing the property 
 * management for color features (Use Color, 8-bit Colors, Color Tolerance) so 
 * that derived grayscale and edge detection classes do not have to duplicate this logic.
 */
class AbstractRenderStrategy : public IRenderStrategy
{
protected:
    bool m_useColor = false;        ///< Toggles background/foreground color rendering ON/OFF.
    bool m_use8BitColor = false;    ///< Toggles 8-bit color mode (256 colors) ON/OFF for faster/wider terminal support.
    float m_colorTolerance = 30.0f; ///< Threshold for grouping similar colors together.

public:
    /**
     * @brief Returns the base color properties (Use Color, Tolerance, etc.).
     * @details Child classes should call this method first, and then append their own specific properties.
     */
    virtual std::vector<Property> getProperties() override;

    /**
     * @brief Looks up a specific color property value.
     */
    virtual float getProperty(const std::string &name) override;

    /**
     * @brief Updates color settings. Unrecognized properties are ignored.
     */
    virtual void setProperty(const Property property) override;
};