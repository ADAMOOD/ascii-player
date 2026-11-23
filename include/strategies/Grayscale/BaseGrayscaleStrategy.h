#pragma once
#include "strategies/AbstractRenderStrategy.h"
#include <string>

/**
 * @class BaseGrayscaleStrategy
 * @brief Base class for all grayscale-based ASCII rendering strategies.
 * * Handles the common logic of resizing the frame and mapping a calculated 
 * brightness value to a character from the ASCII ramp. Subclasses only need 
 * to define the math for calculating that brightness.
 */
class BaseGrayscaleStrategy : public AbstractRenderStrategy {
protected:
    int m_asciiSet = 4;

private:
    const std::array<std::string_view, 11> m_asciiCharSets = {
        " #", //  2
        " .+#", //  4
        " .:+#@", //  6
        " .-:*+#@", //  8
        " .:-=+*#%@", // 10
        " .,:;*+xX%#@", // 12
        " `.-',:~;*+=%#&@", // 16
        " .,:;+=i1tfLCG08#%@$", // 20
        " `.-',:~;!+r=?lLitvxJYj1funCX%#O&0M@", // 36
        " `.-',:_~;!^*/+r><|()?LcitvzxJY][j1f}{unCokXhZaw%U#bdqpmO&08$QWMB@", // 66
        R"( .'`^",:;Il!i><~+_-?][}{1)(|\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$)", // 70
    };

public:
    /**
 * @brief Appends hysteresis settings to the base edge detection properties.
 */
    std::vector<Property> getProperties() override;

    /**
     * @brief Handles updates for hysteresis settings, passing others to the base class.
     */
    void setProperty(const Property property) override;
    /**
     * @brief Resizes the frame and converts pixels to ASCII using subclass brightness logic.
     */
    void render(const cv::Mat& inputFrame, std::vector<ImageUtils::Pixel>& outBuffer, int width, int height) override;

protected:
    /**
     * @brief Pure virtual function to calculate brightness. Implemented by subclasses.
     * @param r Red channel value (0-255).
     * @param g Green channel value (0-255).
     * @param b Blue channel value (0-255).
     * @return Calculated grayscale brightness (0-255).
     */
    virtual uchar calculateBrightness(uchar r, uchar g, uchar b) = 0;
};