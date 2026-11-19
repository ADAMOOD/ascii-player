#pragma once
#include <opencv2/opencv.hpp>
#include <string>
#ifdef USE_TENSORRT
#include <NvInfer.h>
#include <NvInferRuntime.h>
#include <cuda_runtime.h>

class TrtLogger : public nvinfer1::ILogger {
    void log(Severity severity, const char* msg) noexcept override {
        if (severity <= Severity::kWARNING) {
            std::cerr << "[TensorRT LOGGER]" << msg << std::endl;
        }
    }
};
#endif

/**
 * @class SaliencyModel
 * @brief Wrapper of ONNX net, for each frame returns its mask(saliency / segmentation).
 */
class SaliencyModel {
private:
#ifdef USE_TENSORRT
    nvinfer1::IRuntime* m_runtime; ///< Deserialiser. To create live engine from model.engine
    nvinfer1::ICudaEngine* m_engine; ///< compiled neural network (weights + GPU kernels)
    nvinfer1::IExecutionContext* m_context;
    void* m_deviceBuffers[2]; ///< GPU VRAM pointer. [0] = input image (pixel_values), [1] = output mask (alphas)
    TrtLogger m_logger; ///< to catch CUDA errors

#else
    cv::dnn::Net m_net; ///< net
#endif
    bool m_ready = false; ///< true after whole init
    bool m_warned = false; ///< to print err just once
    static constexpr int kInputSize = 256; ///< expected input size

public:
#ifdef USE_TENSORRT
    ~SaliencyModel(); //alocated CUDA TensorRT has to be freed
#endif

    bool init(const std::string& modelPath);

    bool isReady() const {
        return m_ready;
    }

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