#include "core/SaliencyModel.h"
#include <iostream>
#include <fstream>
#include <vector>

bool SaliencyModel::init(const std::string& modelPath) {
    m_ready = false;
    m_warned = false;
#ifdef USE_TENSORRT
    auto data = std::ifstream(modelPath, std::ios::binary | std::ios::ate);
    if (!data) {
        std::cerr << "[SaliencyModel] cannot open '" << modelPath << "'\n";
        return false;
    }
    data.seekg(0, std::ios::end);
    int dataSize = data.tellg();
    data.seekg(0, std::ios::beg);
    std::vector<char> buffer(dataSize);
    data.read(buffer.data(), dataSize);
    data.close();
    m_runtime = nvinfer1::createInferRuntime(m_logger);
    if (m_runtime == nullptr) {
        std::cerr << "[SaliencyModel] cannot create runtime instance\n";
        return false;
    }
    m_engine = m_runtime->deserializeCudaEngine(buffer.data(), buffer.size());
    if (m_engine == nullptr) {
        std::cerr << "[SaliencyModel] cannot create engine instance\n";
        return false;
    }
    m_context = m_engine->createExecutionContext();
    if (m_context == nullptr) {
        std::cerr << "[SaliencyModel] cannot create context instance\n";
        return false;
    }
    int inputSize = 3 * kInputSize * kInputSize * sizeof(float);
    cudaError_t err = cudaMalloc(&m_deviceBuffers[0], inputSize);
    if (err != cudaSuccess) {
        std::cerr << "[SaliencyModel] cannot allocate device buffers (input image)\n";
        return false;
    }
    int maskSize = 1 * kInputSize * kInputSize * sizeof(float); // one rgb chanel only
    err = cudaMalloc(&m_deviceBuffers[1], maskSize);
    if (err != cudaSuccess) {
        std::cerr << "[SaliencyModel] cannot allocate device buffers (mask)\n";
        return false;
    }

#else
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
#endif

    m_ready = true;
    return true;
}
#ifdef USE_TENSORRT
SaliencyModel::~SaliencyModel() {
    if (m_deviceBuffers[0])
        cudaFree(m_deviceBuffers[0]);
    if (m_deviceBuffers[1])
        cudaFree(m_deviceBuffers[1]);
    delete m_context;
    m_context = nullptr;
    delete m_engine;
    m_engine = nullptr;
    delete m_runtime;
    m_runtime = nullptr;
}
#endif

cv::Mat SaliencyModel::getSegmentationMask(const cv::Mat& inputFrame) {
    if (!m_ready || inputFrame.empty())
        return cv::Mat();

    try {
        // image, scale, size, mean, swapRB (BGR->RGB), crop
        cv::Mat blob = cv::dnn::blobFromImage(
            inputFrame,
            1.0 / 255.0,
            cv::Size(kInputSize, kInputSize),
            cv::Scalar(),
            true,
            false
        );
        cv::Mat outputTensor;
#ifdef USE_TENSORRT
        outputTensor = cv::Mat(kInputSize, kInputSize, CV_32FC1);
        cudaMemcpy(m_deviceBuffers[0],
                   blob.ptr<float>(0),
                   3 * kInputSize * kInputSize * sizeof(float),
                   cudaMemcpyHostToDevice); //transfer blob form CPU to NVIDIA NVRAM

        m_context->executeV2(m_deviceBuffers); // run neural network on GPU
        cudaMemcpy(outputTensor.ptr<float>(), m_deviceBuffers[1], kInputSize * kInputSize * sizeof(float), cudaMemcpyDeviceToHost);
#else
        m_net.setInput(blob);
        outputTensor = m_net.forward(); // expexted [1,1,256,256]
#endif

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
    const cv::Mat& inputFrame,
    cv::Mat& outProcessedFrame,
    int width,
    int height,
    float threshold
) {
    cv::Mat mask = getSegmentationMask(inputFrame);
    applyMask(inputFrame, mask, outProcessedFrame, width, height, threshold);
}

void SaliencyModel::applyMask(
    const cv::Mat& frame,
    const cv::Mat& mask,
    cv::Mat& out,
    int width,
    int height,
    float threshold
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