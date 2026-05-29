#pragma once
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

/**
 * @namespace ImageUtils
 * @brief Collection of standalone utility functions for image processing and terminal color formatting.
 */
namespace ImageUtils
{
    /**
     * @struct Pixel
     * @brief Represents a single character cell in the terminal.
     */
    struct Pixel
    {
        char symbol;
        cv::Vec3b fgColor; ///< Foreground text color
        cv::Vec3b bgColor; ///< Background cell color
    };

    // --- MALÉ FUNKCE ZŮSTÁVAJÍ INLINE ---

    inline std::string getAnsiFgColor(cv::Vec3b pixel)
    {
        return "\x1b[38;2;" + std::to_string(pixel[2]) + ";" +
               std::to_string(pixel[1]) + ";" +
               std::to_string(pixel[0]) + "m";
    }

    inline std::string getAnsiBgColor(cv::Vec3b pixel)
    {
        return "\x1b[48;2;" + std::to_string(pixel[2]) + ";" +
               std::to_string(pixel[1]) + ";" +
               std::to_string(pixel[0]) + "m";
    }

    inline uchar get8BitAnsiIndex(cv::Vec3b pixel)
    {
        uchar r = std::round(pixel[2] * 5.0f / 255.0f);
        uchar g = std::round(pixel[1] * 5.0f / 255.0f);
        uchar b = std::round(pixel[0] * 5.0f / 255.0f);
        return 16 + (36 * r) + (6 * g) + b;
    }

    inline bool isColorDifferent(cv::Vec3b a, cv::Vec3b b, int diffTreashold)
    {
        int diff = std::abs(a[0] - b[0]) + std::abs(a[1] - b[1]) + std::abs(a[2] - b[2]);
        return diff > diffTreashold;
    }

    // --- VELKÉ FUNKCE SE JEN DEKLARUJÍ (Bez slova inline!) ---

    /**
     * @brief Retrieves a pre-computed 8-bit ANSI color escape code from a cache.
     * @param index The 8-bit color index (0-255).
     * @return Reference to the cached ANSI string.
     */
    const std::string &get8BitAnsiCode(uchar index);

    /**
     * @brief Fast, direct-pointer conversion of a BGR image to grayscale.
     * @param src Source BGR image.
     * @param dst Destination single-channel grayscale image.
     * @param width Image width.
     * @param height Image height.
     */
    void convertToGrayscale(const cv::Mat &src, cv::Mat &dst, int width, int height);

    /**
     * @brief Applies a 1D separable convolution kernel horizontally and vertically with clamping padding.
     * @param src Source single-channel image.
     * @param dst Destination single-channel image.
     * @param kernel 1D floating-point kernel matrix.
     * @param width Image width.
     * @param height Image height.
     */
    void applyFilter(const cv::Mat &src, cv::Mat &dst, const cv::Mat &kernel, int width, int height);
}
