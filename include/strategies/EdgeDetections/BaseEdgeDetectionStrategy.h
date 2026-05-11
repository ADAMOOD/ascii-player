#pragma once
#include <opencv2/opencv.hpp>
#include "../IRenderStrategy.h"

class BaseEdgeDetectionStrategy : public IRenderStrategy
{
private:
    /* data */
protected:
    void convertToGrayscale(const cv::Mat &src, cv::Mat &dst, int width, int height)
    {
        for (int y = 0; y < height; y++)
        {
            const cv::Vec3b *srcRow = src.ptr<cv::Vec3b>(y);
            uchar *dstRow = dst.ptr<uchar>(y);

            for (int x = 0; x < width; x++)
            {
                // Tady přečteš barvy přímo z pointeru
                uchar b = srcRow[x][0];
                uchar g = srcRow[x][1];
                uchar r = srcRow[x][2];

                uchar gray = static_cast<uchar>(0.299 * r + 0.587 * g + 0.114 * b);

                // direct save to dst
                dstRow[x] = gray;
            }
        }
    }
    void applyFilter(const cv::Mat &src, cv::Mat &dst, const cv::Mat &kernel, int width, int height)
    {
        const float sum = cv::sum(kernel)[0];
        int halfSize = kernel.rows / 2;

        for (int i = halfSize; i < height - halfSize; i++)
        {
            for (int j = halfSize; j < width - halfSize; j++)
            {
                float blurredPixelValue = 0;
                for (int k = -halfSize; k <= halfSize; k++)
                {
                    for (int l = -halfSize; l <= halfSize; l++)
                    {
                        blurredPixelValue += src.at<uchar>(i + k, j + l) * kernel.at<float>(k + halfSize, l + halfSize);
                    }
                }
                blurredPixelValue /= sum;
                if (blurredPixelValue > 255.0f)
                    blurredPixelValue = 255.0f;
                dst.at<uchar>(i, j) = static_cast<uchar>(blurredPixelValue);
            }
        }
    }
    void applyNonMaximumSuppression(const cv::Mat &magnitudes, const cv::Mat &angles, cv::Mat &dst, int width, int height)
    {
        for (int y = 1; y < height - 1; y++)
        {
            for (int x = 1; x < width - 1; x++)
            {
                float mag = magnitudes.at<float>(y, x);
                float angle = angles.at<float>(y, x);
                float neighbor1 = 0, neighbor2 = 0;
                if ((angle >= 0 && angle < 22.5) || (angle >= 157.5 && angle <= 180))
                {
                    neighbor1 = magnitudes.at<float>(y, x + 1);
                    neighbor2 = magnitudes.at<float>(y, x - 1);
                }
                else if (angle >= 22.5 && angle < 67.5)
                {
                    neighbor1 = magnitudes.at<float>(y + 1, x - 1);
                    neighbor2 = magnitudes.at<float>(y - 1, x + 1);
                }
                else if (angle >= 67.5 && angle < 112.5)
                {
                    neighbor1 = magnitudes.at<float>(y + 1, x);
                    neighbor2 = magnitudes.at<float>(y - 1, x);
                }
                else if (angle >= 112.5 && angle < 157.5)
                {
                    neighbor1 = magnitudes.at<float>(y - 1, x - 1);
                    neighbor2 = magnitudes.at<float>(y + 1, x + 1);
                }
                if (mag >= neighbor1 && mag >= neighbor2)
                {
                    dst.at<float>(y, x) = mag;
                }
            }
        }
    }
    void applyHysteresis(const cv::Mat &magnitudes, cv::Mat &dst, int width, int height, float lowThreshold, float highThreshold)
    {
        for (int y = 1; y < height - 1; y++)
        {
            for (int x = 1; x < width - 1; x++)
            {
                float mag = magnitudes.at<float>(y, x);

                if (mag >= highThreshold)
                {
                    // Silná hrana -> 100% zapisujeme 255
                    dst.at<float>(y, x) = 255.0f;
                }
                else if (mag >= lowThreshold)
                {
                    bool connectedToStrongEdge = false;
                    for (int j = -1; j <= 1; j++)
                    {
                        for (int i = -1; i <= 1; i++)
                        {
                            if (j == 0 && i == 0)
                                continue;

                            if (magnitudes.at<float>(y + j, x + i) >= highThreshold)
                            {
                                connectedToStrongEdge = true;
                                break;
                            }
                        }
                        if (connectedToStrongEdge)
                            break;
                    }

                    if (connectedToStrongEdge)
                    {
                        dst.at<float>(y, x) = 255.0f;
                    }
                }
            }
        }
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

                float magnitude = std::abs(sumX) + std::abs(sumY);
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
#pragma once
#include <opencv2/opencv.hpp>
#include "../IRenderStrategy.h"

class BaseEdgeDetectionStrategy : public IRenderStrategy
{
private:
    /* data */
protected:
    void convertToGrayscale(const cv::Mat &src, cv::Mat &dst, int width, int height)
    {
        for (int y = 0; y < height; y++)
        {
            const cv::Vec3b *srcRow = src.ptr<cv::Vec3b>(y);
            uchar *dstRow = dst.ptr<uchar>(y);

            for (int x = 0; x < width; x++)
            {
                // Tady přečteš barvy přímo z pointeru
                uchar b = srcRow[x][0];
                uchar g = srcRow[x][1];
                uchar r = srcRow[x][2];

                uchar gray = static_cast<uchar>(0.299 * r + 0.587 * g + 0.114 * b);

                // direct save to dst
                dstRow[x] = gray;
            }
        }
    }
    void applyFilter(const cv::Mat &src, cv::Mat &dst, const cv::Mat &kernel, int width, int height)
    {
        const float sum = cv::sum(kernel)[0];
        int halfSize = kernel.rows / 2;

        for (int i = halfSize; i < height - halfSize; i++)
        {
            for (int j = halfSize; j < width - halfSize; j++)
            {
                float blurredPixelValue = 0;
                for (int k = -halfSize; k <= halfSize; k++)
                {
                    for (int l = -halfSize; l <= halfSize; l++)
                    {
                        blurredPixelValue += src.at<uchar>(i + k, j + l) * kernel.at<float>(k + halfSize, l + halfSize);
                    }
                }
                blurredPixelValue /= sum;
                if (blurredPixelValue > 255.0f)
                    blurredPixelValue = 255.0f;
                dst.at<uchar>(i, j) = static_cast<uchar>(blurredPixelValue);
            }
        }
    }
    void applyNonMaximumSuppression(const cv::Mat &magnitudes, const cv::Mat &angles, cv::Mat &dst, int width, int height)
    {
        for (int y = 1; y < height - 1; y++)
        {
            for (int x = 1; x < width - 1; x++)
            {
                float mag = magnitudes.at<float>(y, x);
                float angle = angles.at<float>(y, x);
                float neighbor1 = 0, neighbor2 = 0;
                if ((angle >= 0 && angle < 22.5) || (angle >= 157.5 && angle <= 180))
                {
                    neighbor1 = magnitudes.at<float>(y, x + 1);
                    neighbor2 = magnitudes.at<float>(y, x - 1);
                }
                else if (angle >= 22.5 && angle < 67.5)
                {
                    neighbor1 = magnitudes.at<float>(y + 1, x - 1);
                    neighbor2 = magnitudes.at<float>(y - 1, x + 1);
                }
                else if (angle >= 67.5 && angle < 112.5)
                {
                    neighbor1 = magnitudes.at<float>(y + 1, x);
                    neighbor2 = magnitudes.at<float>(y - 1, x);
                }
                else if (angle >= 112.5 && angle < 157.5)
                {
                    neighbor1 = magnitudes.at<float>(y - 1, x - 1);
                    neighbor2 = magnitudes.at<float>(y + 1, x + 1);
                }
                if (mag >= neighbor1 && mag >= neighbor2)
                {
                    dst.at<float>(y, x) = mag;
                }
            }
        }
    }
    void applyHysteresis(const cv::Mat &magnitudes, cv::Mat &dst, int width, int height, float lowThreshold, float highThreshold)
    {
        for (int y = 1; y < height - 1; y++)
        {
            for (int x = 1; x < width - 1; x++)
            {
                float mag = magnitudes.at<float>(y, x);

                if (mag >= highThreshold)
                {
                    // Silná hrana -> 100% zapisujeme 255
                    dst.at<float>(y, x) = 255.0f;
                }
                else if (mag >= lowThreshold)
                {
                    bool connectedToStrongEdge = false;
                    for (int j = -1; j <= 1; j++)
                    {
                        for (int i = -1; i <= 1; i++)
                        {
                            if (j == 0 && i == 0)
                                continue;

                            if (magnitudes.at<float>(y + j, x + i) >= highThreshold)
                            {
                                connectedToStrongEdge = true;
                                break;
                            }
                        }
                        if (connectedToStrongEdge)
                            break;
                    }

                    if (connectedToStrongEdge)
                    {
                        dst.at<float>(y, x) = 255.0f;
                    }
                }
            }
        }
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

                float magnitude = std::abs(sumX) + std::abs(sumY);
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
