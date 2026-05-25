#pragma once
#include <opencv2/opencv.hpp>
#include <cmath>
#include "../IRenderStrategy.h"
#include "../ImageUtilits.h"

class BaseEdgeDetectionStrategy : public IRenderStrategy
{
private:
    float m_sobelMultiplier = 1.0f;
    cv::Mat m_kernel;

public:
    BaseEdgeDetectionStrategy()
    {
        createGaussianKernel();
    }
    std::vector<Property> getProperties() override
    {
        auto props = IRenderStrategy::getProperties();
        props.push_back({"Kernel Size", PropertyType::INTEGER, (float)m_kernelSize, 2.0f, 3.0f, 11.0f}); // step 2 -> kernel has to be odd
        props.push_back({"Sobel Boost", PropertyType::FLOAT, m_sobelMultiplier, 0.1f, 0.1f, 5.0f});
        return props;
    }
    void setProperty(const Property property) override
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
        else
        {
            IRenderStrategy::setProperty(property);
        }
    }

protected:
    int m_kernelSize = 5;

    void generateBaseEdgeData(const cv::Mat &inputFrame, cv::Mat &grayFrame, cv::Mat &magnitudes, cv::Mat &angles, int width, int height)
    {
        cv::Mat resizedFrame;
        cv::resize(inputFrame, resizedFrame, cv::Size(width, height));

        // Tady se grayFrame zapíše do té proměnné, kterou jsme dostali přes referenci
        ImageUtils::convertToGrayscale(resizedFrame, grayFrame, width, height);

        cv::Mat blurredFrame = cv::Mat::zeros(height, width, CV_8UC1);
        ImageUtils::applyFilter(grayFrame, blurredFrame, m_kernel, width, height);

        computeSobelData(blurredFrame, magnitudes, angles, width, height);
    }

    void generateBaseEdgeData(const cv::Mat &inputFrame, cv::Mat &magnitudes, cv::Mat &angles, int width, int height)
    {
        // Vytvoříme si lokální grayFrame, který po skončení této metody prostě zanikne
        cv::Mat dummyGrayFrame = cv::Mat::zeros(height, width, CV_8UC1);

        // Zavoláme tu první, plnou metodu
        generateBaseEdgeData(inputFrame, dummyGrayFrame, magnitudes, angles, width, height);
    }
    char getAsciiForAngle(float angle)
    {
        if (angle > 22.5f && angle <= 67.5f)
        {
            return '\\';
        }
        else if (angle > 67.5f && angle <= 112.5f)
        {
            return '-';
        }
        if (angle > 112.5f && angle <= 157.5f)
        {
            return '/';
        }
        else
        {
            return '|';
        }
    }
    void createGaussianKernel()
    {
        m_kernel = cv::Mat::zeros(1, m_kernelSize, CV_32FC1);
        float sigma = 0.3f * (((m_kernelSize - 1) / 2.0f) - 1.0f) + 0.8f;
        int halfSize = m_kernelSize / 2;
        float sum = 0.0f;
        for (int i = -halfSize; i <= halfSize; i++)
        {
            float exponent = -((i * i)) / (2.0f * sigma * sigma);
            float kernelValue = std::exp(exponent) / (std::sqrt(2.0f * (float)M_PI) * sigma);

            sum += kernelValue;
            m_kernel.at<float>(0, i + halfSize) = kernelValue;
        }

        // Normalization
        for (int i = 0; i < m_kernelSize; i++)
        {
            m_kernel.at<float>(0, i) /= sum;
        }
    }
    void computeSobelData(const cv::Mat &resizedFrame, cv::Mat &magnitudes, cv::Mat &angles, int width, int height)
    {
        const cv::Matx33f Gx(-1, 0, 1, -2, 0, 2, -1, 0, 1);
        const cv::Matx33f Gy(1, 2, 1, 0, 0, 0, -1, -2, -1);

        // Tady můžeme začít bezpečně od 1 a skončit před okrajem
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
};
