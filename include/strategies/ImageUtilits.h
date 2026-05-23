#pragma once
#include <opencv2/opencv.hpp>
#include <algorithm>

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
        // 1. U 1D kernelu nás zajímá počet sloupců (cols), ne řádků!
        int halfSize = kernel.cols / 2;

        int paddedWidth = width + (2 * halfSize);
        int paddedHeight = height + (2 * halfSize);

        cv::Mat paddedSrc = cv::Mat::zeros(paddedHeight, paddedWidth, CV_8UC1);

        // TATO MATICE MUSÍ BÝT FLOAT (32F) a musí být PADDED (velká), aby vertikální průchod nespadl!
        cv::Mat tempFrame = cv::Mat::zeros(paddedHeight, paddedWidth, CV_32FC1);

        // 2. Vycpání (Padding) - Tvůj kód s clamp je skvělý, nech ho tu.
        for (int y = 0; y < paddedHeight; y++)
        {
            for (int x = 0; x < paddedWidth; x++)
            {
                int srcY = std::clamp(y - halfSize, 0, height - 1);
                int srcX = std::clamp(x - halfSize, 0, width - 1);
                paddedSrc.at<uchar>(y, x) = src.at<uchar>(srcY, srcX);
            }
        }

        // 3. HORIZONTÁLNÍ ROZMAZÁNÍ (čte uchar z paddedSrc, zapisuje float do tempFrame)
        for (int y = 0; y < paddedHeight; y++) // Procházíme celou výšku
        {
            for (int x = halfSize; x < paddedWidth - halfSize; x++) // Vynecháme úplné okraje šířky
            {
                float val = 0.0f;
                for (int k = -halfSize; k <= halfSize; k++)
                {
                    // Měníme jen osu X! Kernel má indexaci (0, k + halfSize)
                    val += paddedSrc.at<uchar>(y, x + k) * kernel.at<float>(0, k + halfSize);
                }
                tempFrame.at<float>(y, x) = val;
            }
        }

        // 4. VERTIKÁLNÍ ROZMAZÁNÍ (čte float z tempFrame, zapisuje uchar do dst)
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                float val = 0.0f;

                // Přepočet indexu z originálu (i, j) na vycpanou matici
                int paddedY = i + halfSize;
                int paddedX = j + halfSize;

                for (int k = -halfSize; k <= halfSize; k++)
                {
                    // Měníme jen osu Y! (paddedY + k)
                    val += tempFrame.at<float>(paddedY + k, paddedX) * kernel.at<float>(0, k + halfSize);
                }

                if (val > 255.0f)
                    val = 255.0f;
                dst.at<uchar>(i, j) = static_cast<uchar>(val);
            }
        }
    }
}