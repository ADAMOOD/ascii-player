#pragma once
#include <string>

/**
 * @enum PropertyType
 * @brief Defines the data type of a dynamic property.
 */
enum class PropertyType
{
    BOOLEAN, ///< Represents an ON/OFF state (threshold 0.5).
    INTEGER, ///< Represents whole numbers (e.g., Kernel size).
    FLOAT    ///< Represents decimal numbers (e.g., Color tolerance).
};

/**
 * @struct Property
 * @brief Represents a single adjustable parameter for a rendering strategy.
 * * Properties are dynamically exposed to the user interface (HUD) and can be 
 * modified in real-time during video playback.
 */
struct Property
{
    std::string name;    ///< Display name of the property.
    PropertyType type;   ///< Data type for correct UI formatting.
    float currentValue;  ///< Current active value.
    float step;          ///< Amount by which the value changes per user input.
    float minValue;      ///< Minimum allowed value.
    float maxValue;      ///< Maximum allowed value.

    /**
     * @brief Formats the property into a UI-friendly string based on its type.
     * @return Formatted string (e.g., "[ Hysteresis: ON ]").
     */
    std::string toString() const;

    /**
     * @brief Adjusts the current value by the defined step size.
     * * Automatically clamps the result between minValue and maxValue.
     * @param increase If true, value is increased; if false, value is decreased.
     */
    void ShiftedValue(bool increase);
};