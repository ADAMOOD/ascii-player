#include "ui/Property.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

std::string Property::toString() const
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

void Property::ShiftedValue(bool increase)
{
    if (increase)
    {
        currentValue += step;
    }
    else
    {
        currentValue -= step;
    }
    
    currentValue = std::clamp(currentValue, minValue, maxValue);
}