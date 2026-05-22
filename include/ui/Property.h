#pragma once
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>

enum class PropertyType
{
    BOOLEAN,
    INTEGER,
    FLOAT
};

struct Property
{
    std::string name;
    PropertyType type;
    float currentValue;
    float step;
    float minValue;
    float maxValue;

    std::string toString() const
    {
        std::stringstream ss;
        ss << "[ " << name << ": ";
        
        if (type == PropertyType::BOOLEAN)
        {
            ss << (currentValue > 0.5f ? "ON" : "OFF");
        }
        else if (type == PropertyType::INTEGER)
        {
            ss << static_cast<int>(currentValue);
        }
        else if (type == PropertyType::FLOAT)
        {
            ss << std::fixed << std::setprecision(1) << currentValue;
        }
        
        ss << " ]  ";
        return ss.str();
    }
void ShiftedValue(bool increase)
{
    if (increase)
    {
        currentValue += step;
    }
    else
    {
        currentValue -= step;
    }
    
    // clamp the value
    currentValue = std::clamp(currentValue, minValue, maxValue);
}
};