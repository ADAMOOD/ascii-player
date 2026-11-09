#include "core/SaliencyModel.h"
#include <iostream>

bool SaliencyModel::init(const std::string& modelPath) {
    m_ready = false;
    m_warned = false;
    try {
        m_net = cv::dnn::readNet(modelPath);
    } catch (const cv::Exception& e) {
        std::cerr << "[SaliencyModel] cannot load '" << modelPath << "': " << e.what() << std::endl;
        return false;
    }
    if (m_net.empty()) {
        std::cerr << "[SaliencyModel] '" << modelPath << "' loaded as empty network\n";
        return false;
    }
    m_ready = true;
    return true;
}

cv::Mat SaliencyModel::getSegmentationMask(const cv::Mat& inputFrame) {
    if (!m_ready || inputFrame.empty())
        return cv::Mat();

    try {
        // image, scale, size, mean, swapRB (BGR->RGB), crop
        cv::Mat blob = cv::dnn::blobFromImage(
            inputFrame, 1.0 / 255.0, cv::Size(kInputSize, kInputSize), cv::Scalar(), true, false
        );
        m_net.setInput(blob);
        cv::Mat outputTensor = m_net.forward(); // expexted [1,1,256,256]

        if (outputTensor.depth() != CV_32F || !outputTensor.isContinuous() ||
            outputTensor.total() != static_cast<size_t>(kInputSize) * kInputSize) {
            if (!m_warned) {
                std::cerr << "[SaliencyModel] unexpected output tensor (total="
                          << outputTensor.total() << ", depth=" << outputTensor.depth()
                          << "), expected 1x1x" << kInputSize << "x" << kInputSize << " float\n";
                m_warned = true;
            }
            return cv::Mat();
        }

        cv::Mat mask2D(kInputSize, kInputSize, CV_32FC1, outputTensor.ptr<float>());
        cv::Mat mask8Bit;
        mask2D.convertTo(mask8Bit, CV_8UC1, 255.0); // convertTo craates copy
        return mask8Bit;
    } catch (const cv::Exception& e) {
        if (!m_warned) {
            std::cerr << "[SaliencyModel] inference failed: " << e.what() << std::endl;
            m_warned = true;
        }
        return cv::Mat();
    }
}

void SaliencyModel::getSegmentationFrame(
    const cv::Mat& inputFrame, cv::Mat& outProcessedFrame, int width, int height, float threshold
) {
    cv::Mat mask = getSegmentationMask(inputFrame);
    applyMask(inputFrame, mask, outProcessedFrame, width, height, threshold);
}

void SaliencyModel::applyMask(
    const cv::Mat& frame, const cv::Mat& mask, cv::Mat& out, int width, int height, float threshold
) {
    if (width <= 0 || height <= 0) {
        out.release();
        return;
    }
    out = cv::Mat::zeros(height, width, CV_8UC3);
    if (frame.empty() || mask.empty())
        return;

    cv::Mat resizedFrame, resizedMask, binary;
    cv::resize(frame, resizedFrame, cv::Size(width, height), 0, 0, cv::INTER_AREA);
    cv::resize(mask, resizedMask, cv::Size(width, height), 0, 0, cv::INTER_LINEAR);
    cv::threshold(resizedMask, binary, threshold * 255.0, 255, cv::THRESH_BINARY);

    resizedFrame.copyTo(out, binary); // copy using binary AND
}