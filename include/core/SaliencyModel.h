#pragma once
#include <opencv2/opencv.hpp>
#include <string>

/**
 * @class SaliencyModel
 * @brief Wrapper of ONNX net, for each frame returns its mask(saliency / segmentation).
 */
class SaliencyModel {
  private:
    static constexpr int kInputSize = 256; ///< expected input size

    cv::dnn::Net m_net;   ///< net
    bool m_ready = false; ///< true after whole init
    bool m_warned = false; ///< to print err just once

  public:
    bool init(const std::string& modelPath);
    bool isReady() const { return m_ready; }

    /// Saliency mask 256x256, CV_8UC1 (0 = background, 255 = forground). empty cv::Mat when err.
    cv::Mat getSegmentationMask(const cv::Mat& inputFrame);

    void getSegmentationFrame(
        const cv::Mat& inputFrame,
        cv::Mat& outProcessedFrame,
        int width,
        int height,
        float threshold = 0.5f
    );

    static void applyMask(
        const cv::Mat& frame,
        const cv::Mat& mask,
        cv::Mat& out,
        int width,
        int height,
        float threshold
    );
};