#pragma once
#include "../IRenderStrategy.h"

class BaseGrayscaleStrategy : public IRenderStrategy {
private:
    std::string m_asciiChars = " .:-=+*#%@"; 
public:
    void render(const cv::Mat& inputFrame, std::string& outBuffer, int width, int height) override;
    virtual uchar calculateBrightness(uchar r, uchar g, uchar b) = 0;
    
};
