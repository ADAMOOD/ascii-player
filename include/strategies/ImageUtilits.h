#pragma once
#include <opencv2/opencv.hpp>

namespace ImageUtils
{
    inline void convertToGrayscale(const cv::Mat &src, cv::Mat &dst, int width, int height)
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
    inline void applyFilter(const cv::Mat &src, cv::Mat &dst, const cv::Mat &kernel, int width, int height)
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
}