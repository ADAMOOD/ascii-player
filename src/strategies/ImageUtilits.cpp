#include "strategies/ImageUtilits.h"
#include <cmath> 

namespace ImageUtils
{
    const std::string &get8BitAnsiCode(uchar index)
    {
        static std::vector<std::string> cache = []()
        {
            std::vector<std::string> c(256);
            for (int i = 0; i < 256; ++i)
                c[i] = "\x1b[38;5;" + std::to_string(i) + "m";
            return c;
        }();
        return cache[index];
    }

    void convertToGrayscale(const cv::Mat &src, cv::Mat &dst, int width, int height)
    {
        for (int y = 0; y < height; y++)
        {
            const cv::Vec3b *srcRow = src.ptr<cv::Vec3b>(y);
            uchar *dstRow = dst.ptr<uchar>(y);

            for (int x = 0; x < width; x++)
            {
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
        // 1d kernel, odd size, e.g. 3, 5, 7
        int halfSize = kernel.cols / 2;

        int paddedWidth = width + (2 * halfSize);
        int paddedHeight = height + (2 * halfSize);

        cv::Mat paddedSrc = cv::Mat::zeros(paddedHeight, paddedWidth, CV_8UC1);

        cv::Mat tempFrame = cv::Mat::zeros(paddedHeight, paddedWidth, CV_32FC1);

        // Padding: copy src into the center of paddedSrc, replicating border pixels
        for (int y = 0; y < paddedHeight; y++)
        {
            for (int x = 0; x < paddedWidth; x++)
            {
                int srcY = std::clamp(y - halfSize, 0, height - 1);
                int srcX = std::clamp(x - halfSize, 0, width - 1);
                paddedSrc.at<uchar>(y, x) = src.at<uchar>(srcY, srcX);
            }
        }

        // Horizontal blur (read uchar from paddedSrc, write float to tempFrame)
        for (int y = 0; y < paddedHeight; y++)
        {
            for (int x = halfSize; x < paddedWidth - halfSize; x++)//Border handling: only process pixels that have a full kernel around them
            {
                float val = 0.0f;
                for (int k = -halfSize; k <= halfSize; k++)
                {
                    //Only change the X coordinate for horizontal blur (x + k)
                    val += paddedSrc.at<uchar>(y, x + k) * kernel.at<float>(0, k + halfSize);
                }
                tempFrame.at<float>(y, x) = val;
            }
        }

        // Vertical blur (read float from tempFrame, write uchar to dst)
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                float val = 0.0f;

                // recalculate padded coordinates for the original image pixel (i,j)
                int paddedY = i + halfSize;
                int paddedX = j + halfSize;

                for (int k = -halfSize; k <= halfSize; k++)
                {
                    // Only change the Y coordinate for vertical blur (paddedY + k)
                    val += tempFrame.at<float>(paddedY + k, paddedX) * kernel.at<float>(0, k + halfSize);
                }

                if (val > 255.0f)
                    val = 255.0f;
                dst.at<uchar>(i, j) = static_cast<uchar>(val);
            }
        }
    }
}