#include "edge_detector.h"
#include <iostream>

EdgeDetector::EdgeDetector()
    : m_params()
{
}

EdgeDetector::EdgeDetector(const EdgeDetectionParams& params)
    : m_params(params)
{
    if (!validateParams(m_params)) {
        std::cerr << "警告：边缘检测参数无效，使用默认参数" << std::endl;
        m_params = EdgeDetectionParams();
    }
}

void EdgeDetector::setParams(const EdgeDetectionParams& params)
{
    if (validateParams(params)) {
        m_params = params;
    } else {
        std::cerr << "错误：边缘检测参数无效，保持原参数不变" << std::endl;
    }
}

EdgeDetector::EdgeDetectionParams EdgeDetector::getParams() const
{
    return m_params;
}

bool EdgeDetector::detectEdges(const cv::Mat& inputImage, cv::Mat& outputEdges)
{
    try {
        if (inputImage.empty()) {
            std::cerr << "错误：输入图像为空" << std::endl;
            return false;
        }

        // 预处理：转换为灰度图
        cv::Mat grayImage;
        if (!preprocessImage(inputImage, grayImage)) {
            return false;
        }

        // 执行Canny边缘检测
        cv::Canny(grayImage, outputEdges, 
                  m_params.lowThreshold, 
                  m_params.highThreshold,
                  m_params.apertureSize,
                  m_params.useL2Gradient);

        return !outputEdges.empty();

    } catch (const cv::Exception& e) {
        std::cerr << "OpenCV异常：" << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "标准异常：" << e.what() << std::endl;
        return false;
    }
}

bool EdgeDetector::detectEdgesInROI(const cv::Mat& inputImage, const cv::Rect& roi, cv::Mat& outputEdges)
{
    try {
        if (inputImage.empty()) {
            std::cerr << "错误：输入图像为空" << std::endl;
            return false;
        }

        // 验证ROI区域有效性
        if (roi.x < 0 || roi.y < 0 || 
            roi.x + roi.width > inputImage.cols || 
            roi.y + roi.height > inputImage.rows ||
            roi.width <= 0 || roi.height <= 0) {
            std::cerr << "错误：ROI区域无效" << std::endl;
            return false;
        }

        // 提取ROI区域
        cv::Mat roiImage = inputImage(roi);

        // 在ROI区域内执行边缘检测
        cv::Mat roiEdges;
        if (!detectEdges(roiImage, roiEdges)) {
            return false;
        }

        // 创建与原图同尺寸的输出图像
        outputEdges = cv::Mat::zeros(inputImage.size(), CV_8UC1);
        
        // 将ROI区域的边缘结果复制到对应位置
        roiEdges.copyTo(outputEdges(roi));

        return true;

    } catch (const cv::Exception& e) {
        std::cerr << "OpenCV异常：" << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "标准异常：" << e.what() << std::endl;
        return false;
    }
}

bool EdgeDetector::preprocessImage(const cv::Mat& inputImage, cv::Mat& outputGray)
{
    try {
        if (inputImage.empty()) {
            std::cerr << "错误：输入图像为空" << std::endl;
            return false;
        }

        if (inputImage.channels() == 1) {
            // 已经是灰度图，直接复制
            outputGray = inputImage.clone();
        } else if (inputImage.channels() == 3) {
            // BGR彩色图转灰度图
            cv::cvtColor(inputImage, outputGray, cv::COLOR_BGR2GRAY);
        } else if (inputImage.channels() == 4) {
            // BGRA彩色图转灰度图
            cv::cvtColor(inputImage, outputGray, cv::COLOR_BGRA2GRAY);
        } else {
            std::cerr << "错误：不支持的图像通道数：" << inputImage.channels() << std::endl;
            return false;
        }

        return !outputGray.empty();

    } catch (const cv::Exception& e) {
        std::cerr << "OpenCV异常：" << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "标准异常：" << e.what() << std::endl;
        return false;
    }
}

bool EdgeDetector::validateParams(const EdgeDetectionParams& params)
{
    // 验证阈值范围
    if (params.lowThreshold < 0 || params.lowThreshold > 255 ||
        params.highThreshold < 0 || params.highThreshold > 255) {
        std::cerr << "错误：Canny阈值必须在0-255范围内" << std::endl;
        return false;
    }

    // 验证阈值关系
    if (params.lowThreshold >= params.highThreshold) {
        std::cerr << "错误：低阈值必须小于高阈值" << std::endl;
        return false;
    }

    // 验证孔径大小
    if (params.apertureSize != 3 && params.apertureSize != 5 && params.apertureSize != 7) {
        std::cerr << "错误：孔径大小必须为3、5或7" << std::endl;
        return false;
    }

    return true;
}
