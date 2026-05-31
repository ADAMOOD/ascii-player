#include "strategies/EdgeDetections/AdvancedEdgeDetectionStrategy.h"

std::vector<Property> AdvancedEdgeDetectionStrategy::getProperties()
{
    // 1. Získej properties z Base (Kernel, Sobel Boost)
    auto props = BaseEdgeDetectionStrategy::getProperties();

    // 2. Přidej Hysteresis properties
    props.push_back({"Hysteresis", PropertyType::BOOLEAN, m_useHysteresis ? 1.0f : 0.0f, 1.0f, 0.0f, 1.0f});
    if (m_useHysteresis)
    {
        props.push_back({"Hysteresis Low", PropertyType::FLOAT, m_lowThreshold, 5.0f, 0.0f, 255.0f});
        props.push_back({"Hysteresis High", PropertyType::FLOAT, m_highThreshold, 5.0f, 0.0f, 255.0f});
    }
    return props;
}

void AdvancedEdgeDetectionStrategy::setProperty(const Property property)
{
    if (property.name == "Hysteresis")
        m_useHysteresis = (property.currentValue > 0.5f);
    else if (property.name == "Hysteresis Low")
        m_lowThreshold = property.currentValue;
    else if (property.name == "Hysteresis High")
        m_highThreshold = property.currentValue;
    else
        BaseEdgeDetectionStrategy::setProperty(property); // Pošli to o patro výš
}

void AdvancedEdgeDetectionStrategy::render(const cv::Mat &inputFrame, std::vector<ImageUtils::Pixel> &outBuffer, int width, int height)
{
    cv::Mat grayFrame = cv::Mat::zeros(height, width, CV_8UC1);
    cv::Mat magnitudes = cv::Mat::zeros(height, width, CV_32F);
    cv::Mat angles = cv::Mat::zeros(height, width, CV_32F);
    cv::Mat coloredResizedFrame = cv::Mat::zeros(height, width, CV_8UC3);
    
    // 1. Získáme předzpracovaná data od Base
    generateBaseEdgeData(inputFrame, grayFrame, coloredResizedFrame, magnitudes, angles, width, height);

    // 2. Aplikujeme NMS
    cv::Mat nmsMagnitudes = cv::Mat::zeros(height, width, CV_32F);
    applyNonMaximumSuppression(magnitudes, angles, nmsMagnitudes, width, height);

    // 3. Aplikujeme Hysterezi (volitelně)
    if (m_useHysteresis)
    {
        cv::Mat hysteresisResult = cv::Mat::zeros(height, width, CV_32F);
        applyHysteresis(nmsMagnitudes, hysteresisResult, width, height, m_lowThreshold, m_highThreshold);
        nmsMagnitudes = hysteresisResult;
    }

    // 4. Kreslení
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int bufferIndex = y * width + x;
            cv::Vec3b color = coloredResizedFrame.at<cv::Vec3b>(y, x);

            if (x == 0 || y == 0 || x == width - 1 || y == height - 1)
            {
                outBuffer[bufferIndex] = {' ', color, {0, 0, 0}};
                continue;
            }

            float mag = nmsMagnitudes.at<float>(y, x);
            float angle = angles.at<float>(y, x);

            // DELEGUJEME ROZHODNUTÍ NA POTOMKA!
            outBuffer[bufferIndex] = {determinePixelChar(x, y, mag, angle, nmsMagnitudes, angles, grayFrame), color, {0, 0, 0}};
        }
    }
}

void AdvancedEdgeDetectionStrategy::applyNonMaximumSuppression(const cv::Mat &magnitudes, const cv::Mat &angles, cv::Mat &dst, int width, int height)
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

void AdvancedEdgeDetectionStrategy::applyHysteresis(const cv::Mat &magnitudes, cv::Mat &dst, int width, int height, float lowThreshold, float highThreshold)
{
    for (int y = 1; y < height - 1; y++)
    {
        for (int x = 1; x < width - 1; x++)
        {
            float mag = magnitudes.at<float>(y, x);

            if (mag >= highThreshold)
            {
                dst.at<float>(y, x) = 255.0f;
            }
            else if (mag >= lowThreshold)
            {
                bool connectedToStrongEdge = false;
                for (int j = -1; j <= 1; j++)
                {
                    for (int i = -1; i <= 1; i++)
                    {
                        if (j == 0 && i == 0) continue;

                        if (magnitudes.at<float>(y + j, x + i) >= highThreshold)
                        {
                            connectedToStrongEdge = true;
                            break;
                        }
                    }
                    if (connectedToStrongEdge) break;
                }

                if (connectedToStrongEdge)
                {
                    dst.at<float>(y, x) = 255.0f;
                }
            }
        }
    }
}