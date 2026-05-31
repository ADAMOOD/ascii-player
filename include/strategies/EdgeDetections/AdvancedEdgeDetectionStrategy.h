#pragma once
#include "strategies/EdgeDetections/BaseEdgeDetectionStrategy.h"

/**
 * @class AdvancedEdgeDetectionStrategy
 * @brief Extends basic Sobel edge detection with advanced Canny-like features.
 * * Adds Non-Maximum Suppression (NMS) to thin edges to a single pixel width, 
 * and optional Hysteresis thresholding to connect weak edges to strong ones.
 * Serves as a base class for specific advanced edge renderers.
 */
class AdvancedEdgeDetectionStrategy : public BaseEdgeDetectionStrategy
{
protected:
    bool m_useHysteresis = false;   ///< Toggles hysteresis thresholding ON/OFF.
    float m_lowThreshold = 30.0f;   ///< Lower bound for weak edge candidates.
    float m_highThreshold = 100.0f; ///< Upper bound for strong, definite edges.

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
     * @brief Executes the advanced edge detection pipeline (Base -> NMS -> Hysteresis).
     */
    void render(const cv::Mat &inputFrame, std::vector<ImageUtils::Pixel> &outBuffer, int width, int height) override;

protected:
    /**
     * @brief Pure virtual hook for child classes to define specific ASCII characters for edges.
     * @param x X coordinate.
     * @param y Y coordinate.
     * @param mag Filtered gradient magnitude.
     * @param angle Gradient angle.
     * @param allMagnitudes The full matrix of magnitudes for neighbor checking.
     * @param allAngles The full matrix of angles for neighbor checking.
     * @param grayFrame The original grayscale frame for shading.
     * @return The chosen ASCII character.
     */
    virtual char determinePixelChar(int x, int y, float mag, float angle, const cv::Mat &allMagnitudes, const cv::Mat &allAngles, const cv::Mat &grayFrame) = 0;

/**
     * @brief Thins edges by suppressing pixels that are not the local maximum in the direction of the gradient.
     * @details
     * [ How does Non-Maximum Suppression work? ]
     * Sobel edge detection often produces "thick" edges because it highlights all pixels where a color transition occurs.
     * Non-Maximum Suppression (NMS) thins these edges down to a single pixel width.
     * It does this by checking each pixel against its neighbors ALONG THE DIRECTION OF THE GRADIENT (the direction of the sharpest color change), which is always perpendicular to the edge itself.
     * For example, if we have a thick HORIZONTAL edge, the color changes VERTICALLY. Therefore, NMS will compare the current pixel's magnitude to the pixels directly ABOVE and BELOW it.
     * If either the top or bottom neighbor has a stronger magnitude, the current pixel is not the true "ridge" of the edge and is suppressed (set to zero).
     * The result is a much cleaner edge map with thin, well-defined lines.
     */
    void applyNonMaximumSuppression(const cv::Mat &magnitudes, const cv::Mat &angles, cv::Mat &dst, int width, int height);

/**
     * @brief Filters edges using two thresholds. Weak edges are only kept if connected to strong edges.
     * @details
     * [ How does Hysteresis Thresholding work? ]
     * After Non-Maximum Suppression, we have a thinned edge map, but it may still contain noise and weak edges.
     *  Hysteresis Thresholding is a technique used to further refine the edge map by classifying edges into three categories: strong, weak, and non-edges.
     * 1. Strong edges: Pixels with gradient magnitudes above the high threshold are considered strong edges and are immediately kept.
     * 2. Weak edges: Pixels with gradient magnitudes between the low and high thresholds are considered weak edges. 
     * They are not immediately discarded, but they are only kept if they are connected to strong edges.
     *  This means that if a weak edge pixel is adjacent to a strong edge pixel, it is promoted to a strong edge and kept in the final edge map.
     * 3. Non-edges: Pixels with gradient magnitudes below the low threshold are considered non-edges and are discarded.
     * The result of hysteresis thresholding is a cleaner edge map that retains important
     * edges while reducing noise and false positives, especially in areas where the gradient is not very strong
     *  but still significant due to connectivity to strong edges.
     */
    void applyHysteresis(const cv::Mat &magnitudes, cv::Mat &dst, int width, int height, float lowThreshold, float highThreshold);
};