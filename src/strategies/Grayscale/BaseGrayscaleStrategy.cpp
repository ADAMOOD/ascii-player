#include "strategies/Grayscale/BaseGrayscaleStrategy.h"

std::vector<Property> BaseGrayscaleStrategy::getProperties() {
    std::vector<Property> properties = AbstractRenderStrategy::getProperties();
    properties.push_back({"Ascii Set", PropertyType::INTEGER, (float)m_asciiSet, 1, 0, (float)m_asciiCharSets.size() - 1});
    return properties;
}

void BaseGrayscaleStrategy::setProperty(const Property property) {
    if (property.name == "Ascii Set") {
        m_asciiSet = static_cast<int>(property.currentValue);
    }
    else
    {
        AbstractRenderStrategy::setProperty(property);
    }
}

void BaseGrayscaleStrategy::render(const cv::Mat& inputFrame, std::vector<ImageUtils::Pixel>& outBuffer, int width, int height) {
    cv::Mat resizedFrame;
    cv::resize(inputFrame, resizedFrame, cv::Size(width, height));

    outBuffer.resize(width * height);

    if (m_show_debug_window) {
        // create() faster than constructor
        m_debugFrame.create(height, width, CV_8UC1);
    }

    for (int y = 0; y < height; y++) {
        // pointer on the whole mat row
        uchar* debugRow = m_show_debug_window ? m_debugFrame.ptr<uchar>(y) : nullptr;

        for (int x = 0; x < width; x++) {
            cv::Vec3b pixel = resizedFrame.at<cv::Vec3b>(y, x);

            // one call only
            uchar brightness = calculateBrightness(pixel[2], pixel[1], pixel[0]);

            // direct write to memory
            if (debugRow) {
                debugRow[x] = brightness;
            }

            int charIndex = (brightness * (m_asciiCharSets[m_asciiSet].length() - 1)) / 255;
            outBuffer[y * width + x] = {m_asciiCharSets[m_asciiSet][charIndex], pixel, {0, 0, 0}};
        }
    }
}