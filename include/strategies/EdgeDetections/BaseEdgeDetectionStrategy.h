#pragma once

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "strategies/AbstractRenderStrategy.h"
#include <opencv2/opencv.hpp>

/**
 * @class BaseEdgeDetectionStrategy
 * @brief Base class for edge detection algorithms.
 * * Provides shared functionality for Gaussian blurring and Sobel gradient computation.
 */
class BaseEdgeDetectionStrategy : public AbstractRenderStrategy
{
private:
    float m_sobelMultiplier = 1.0f;
    cv::Mat m_kernel;

public:
    BaseEdgeDetectionStrategy();
    
    void setFillChar(char c) { m_fillChar = c; }
    
    std::vector<Property> getProperties() override;
    void setProperty(const Property property) override;

protected:
    int m_kernelSize = 5;///< Size of the Gaussian kernel (must be odd).
    char m_fillChar = ' ';///< Character used for non-edge pixels.
    float m_edgeThreshold = 50.0f; ///< Adjustable threshold for drawing an edge.

    /**
     * @brief Core pipeline: Resizes, converts to grayscale, blurs, and computes gradients.
     */
    void generateBaseEdgeData(const cv::Mat &inputFrame, cv::Mat &grayFrame, cv::Mat &coloredFrame, cv::Mat &magnitudes, cv::Mat &angles, int width, int height);

    /**
     * @brief Overloaded pipeline for strategies that do not need the grayscale frame.
     */
    void generateBaseEdgeData(const cv::Mat &inputFrame, cv::Mat &coloredFrame, cv::Mat &magnitudes, cv::Mat &angles, int width, int height);

    /**
     * @brief Maps a gradient angle (0-180) to a directional ASCII character.
     */
    char getAsciiForAngle(float angle);

/**
     * @brief Generates a 1D normalized Gaussian kernel.
     * @details
     * Instead of a full 2D matrix, this generates a 1D vector (row) to apply separable convolution, 
     * which is significantly faster.
     * * The kernel values are calculated using the 1D Gaussian distribution formula:
     * $$G(x) = \frac{1}{\sqrt{2\pi}\sigma} e^{-\frac{x^2}{2\sigma^2}}$$
     * * The standard deviation ($\sigma$) is automatically computed based on the kernel size 
     * using the standard OpenCV formula to ensure the bell curve fits perfectly within the kernel.
     */
    void createGaussianKernel();

/**
     * @brief Applies the Sobel operator to compute gradient magnitudes and angles.
     * @details 
     * [ How does Sobel edge detection work? ]
     * Sobel edge detection works by applying two 3x3 convolution kernels to the image: one for detecting horizontal changes (Gx) and one for vertical changes (Gy). 
     * Each kernel emphasizes changes in its respective direction. The algorithm processes the image pixel by pixel. 
     * It evalutes how drastically the color (brightness) changes between neighboring pixels in both horizontal and vertical directions.
     * Results in a magnitude (strength of the edge) and an angle (direction of the edge).
     * The output is a "thick" edge map where strong edges are highlighted, but they may appear blurred or wide due to the nature of 
     * the convolution and lack of non-maximum suppression.
     */
    void computeSobelData(const cv::Mat &resizedFrame, cv::Mat &magnitudes, cv::Mat &angles, int width, int height);
};