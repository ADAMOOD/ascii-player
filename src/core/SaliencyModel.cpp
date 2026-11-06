#include "core/SaliencyModel.h"
#include <string>

bool SaliencyModel::init(const std::string& modelPath) {
    m_net = cv::dnn::readNet(modelPath);
    return true;
}
cv::Mat SaliencyModel::getSegmentationMask(cv::Mat& inputFrame) {
    // image, aspectratio, requiredsize, pcolor avg, swapRB (BGR->RGB), crop
    cv::Mat blob = cv::dnn::blobFromImage(
        inputFrame, 1.0 / 255.0, cv::Size(256, 256), cv::Scalar(), true, false
    );
    m_net.setInput(blob);
    cv::Mat outputTensor = m_net.forward(); // [1,1,256,256] 1 image processed, 1 canal for mask = 1
                                            // for white 0 black, width, height

    float* dataPtr = outputTensor.ptr<float>();

    cv::Mat mask2D(256, 256, CV_32FC1, dataPtr); 
    cv::Mat mask8Bit;

    mask2D.convertTo(mask8Bit, CV_8UC1, 255.0);


    //cv::threshold(mask8Bit, mask8Bit, 128, 255, cv::THRESH_BINARY);

    return mask8Bit;
}