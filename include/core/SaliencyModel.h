#pragma once 
#include <opencv2/opencv.hpp>

class SaliencyModel{
    private:
    cv::dnn::Net m_net;     ///< neural network that holds the full structure and weights

    public:
    bool init(const std::string& modelPath);///< initialize neural network
    cv::Mat getSegmentationMask(cv::Mat& inputFrame);///<generating a Saiency mask over inputFrame

};