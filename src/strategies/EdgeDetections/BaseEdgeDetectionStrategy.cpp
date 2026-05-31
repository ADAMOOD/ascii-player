#include "strategies/EdgeDetections/BaseEdgeDetectionStrategy.h"
#include <cmath>

BaseEdgeDetectionStrategy::BaseEdgeDetectionStrategy()
{
    createGaussianKernel();
}

std::vector<Property> BaseEdgeDetectionStrategy::getProperties()
{
    auto props = AbstractRenderStrategy::getProperties();
    props.push_back({"Kernel Size", PropertyType::INTEGER, (float)m_kernelSize, 2.0f, 3.0f, 11.0f});
    props.push_back({"Sobel Boost", PropertyType::FLOAT, m_sobelMultiplier, 0.1f, 0.1f, 5.0f});
    props.push_back({"Edge Threshold", PropertyType::FLOAT, m_edgeThreshold, 5.0f, 0.0f, 255.0f});
    return props;
}

void BaseEdgeDetectionStrategy::setProperty(const Property property)
{
    if (property.name == "Kernel Size")
    {
        int newSize = static_cast<int>(property.currentValue);
        if (newSize != m_kernelSize)
        {
            m_kernelSize = newSize;
            createGaussianKernel();
        }
    }
    else if (property.name == "Sobel Boost")
    {
        m_sobelMultiplier = property.currentValue;
    }
    else if (property.name == "Edge Threshold")
    {
        m_edgeThreshold = property.currentValue;
    }

    else
    {
        AbstractRenderStrategy::setProperty(property);
    }
}

void BaseEdgeDetectionStrategy::generateBaseEdgeData(const cv::Mat &inputFrame, cv::Mat &grayFrame, cv::Mat &coloredFrame, cv::Mat &magnitudes, cv::Mat &angles, int width, int height)
{
    cv::Mat resizedFrame;
    cv::resize(inputFrame, resizedFrame, cv::Size(width, height));
    coloredFrame = resizedFrame;

    ImageUtils::convertToGrayscale(resizedFrame, grayFrame, width, height);

    cv::Mat blurredFrame = cv::Mat::zeros(height, width, CV_8UC1);
    ImageUtils::applyFilter(grayFrame, blurredFrame, m_kernel, width, height);

    computeSobelData(blurredFrame, magnitudes, angles, width, height);
}

void BaseEdgeDetectionStrategy::generateBaseEdgeData(const cv::Mat &inputFrame, cv::Mat &coloredFrame, cv::Mat &magnitudes, cv::Mat &angles, int width, int height)
{
    cv::Mat dummyGrayFrame = cv::Mat::zeros(height, width, CV_8UC1);
    generateBaseEdgeData(inputFrame, dummyGrayFrame, coloredFrame, magnitudes, angles, width, height);
}

char BaseEdgeDetectionStrategy::getAsciiForAngle(float angle)
{
    if (angle > 22.5f && angle <= 67.5f)
        return '\\';
    else if (angle > 67.5f && angle <= 112.5f)
        return '-';
    if (angle > 112.5f && angle <= 157.5f)
        return '/';
    else
        return '|';
}

void BaseEdgeDetectionStrategy::createGaussianKernel()
{
    m_kernel = cv::Mat::zeros(1, m_kernelSize, CV_32FC1);
    
    // SIGMA EQUATION
    // 0.3 * ((ksize - 1) * 0.5 - 1) + 0.8
    // this formula is derived from OpenCV's standard deviation calculation for Gaussian blur, ensuring that the kernel values 
    //form a proper bell curve that fits within the specified kernel size.
    float sigma = 0.3f * (((m_kernelSize - 1) / 2.0f) - 1.0f) + 0.8f;
    
    int halfSize = m_kernelSize / 2;
    float sum = 0.0f;
    
    // compute the kernel values using the Gaussian function
    // going from -halfSize to +halfSize allows us to center the kernel around zero, which is essential for proper blurring.
    for (int i = -halfSize; i <= halfSize; i++)
    {
        // -x^2 / (2 * sigma^2)
        float exponent = -((i * i)) / (2.0f * sigma * sigma);
        
        // full Gauss: e^(exponent) / (sqrt(2*PI) * sigma)
        float kernelValue = std::exp(exponent) / (std::sqrt(2.0f * (float)M_PI) * sigma);
        
        sum += kernelValue; // save sum for normalization later
        
        // save the kernel value in the matrix, offset by halfSize to shift from [-halfSize, halfSize] to [0, kernelSize-1]
        m_kernel.at<float>(0, i + halfSize) = kernelValue;
    }
    
    // NORMALIZATION
    // this ensures that the sum of all kernel values equals 1, which is crucial for maintaining the overall brightness of the image after convolution.
    for (int i = 0; i < m_kernelSize; i++)
    {
        m_kernel.at<float>(0, i) /= sum;
    }
}

void BaseEdgeDetectionStrategy::computeSobelData(const cv::Mat &resizedFrame, cv::Mat &magnitudes, cv::Mat &angles, int width, int height)
{
    const cv::Matx33f Gx(-1, 0, 1, -2, 0, 2, -1, 0, 1);
    const cv::Matx33f Gy(1, 2, 1, 0, 0, 0, -1, -2, -1);

    for (int y = 1; y < height - 1; y++)
    {
        for (int x = 1; x < width - 1; x++)
        {
            float sumX = 0;
            float sumY = 0;

            for (int j = -1; j <= 1; j++)
            {
                for (int i = -1; i <= 1; i++)
                {
                    uchar average = resizedFrame.at<uchar>(y + j, x + i);
                    sumX += average * Gx(j + 1, i + 1);
                    sumY += average * Gy(j + 1, i + 1);
                }
            }

            float magnitude = (std::abs(sumX) + std::abs(sumY)) * m_sobelMultiplier;
            if (magnitude > 255)
                magnitude = 255;

            float angle = std::atan2(sumY, sumX) * 180 / M_PI;
            if (angle < 0)
                angle += 180;

            magnitudes.at<float>(y, x) = magnitude;
            angles.at<float>(y, x) = angle;
        }
    }
}