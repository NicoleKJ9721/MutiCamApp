#ifndef EDGE_DETECTOR_H
#define EDGE_DETECTOR_H

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

/**
 * @brief 边缘检测器类
 * 封装Canny边缘检测算法，提供可配置的参数接口
 */
class EdgeDetector
{
public:
    /**
     * @brief 边缘检测参数结构体
     */
    struct EdgeDetectionParams {
        double lowThreshold = 50.0;     // Canny低阈值
        double highThreshold = 150.0;   // Canny高阈值
        int apertureSize = 3;           // Sobel算子孔径大小
        bool useL2Gradient = false;     // 是否使用L2梯度
        
        EdgeDetectionParams() = default;
        EdgeDetectionParams(double low, double high, int aperture = 3, bool l2 = false)
            : lowThreshold(low), highThreshold(high), apertureSize(aperture), useL2Gradient(l2) {}
    };

public:
    EdgeDetector();
    explicit EdgeDetector(const EdgeDetectionParams& params);
    ~EdgeDetector() = default;

    /**
     * @brief 设置边缘检测参数
     * @param params 参数结构体
     */
    void setParams(const EdgeDetectionParams& params);

    /**
     * @brief 获取当前参数
     * @return 参数结构体
     */
    EdgeDetectionParams getParams() const;

    /**
     * @brief 执行边缘检测
     * @param inputImage 输入图像（灰度图或彩色图）
     * @param outputEdges 输出边缘图像（二值图）
     * @return 是否成功
     */
    bool detectEdges(const cv::Mat& inputImage, cv::Mat& outputEdges);

    /**
     * @brief 在指定ROI区域内执行边缘检测
     * @param inputImage 输入图像
     * @param roi ROI矩形区域
     * @param outputEdges 输出边缘图像（与输入图像同尺寸）
     * @return 是否成功
     */
    bool detectEdgesInROI(const cv::Mat& inputImage, const cv::Rect& roi, cv::Mat& outputEdges);

    /**
     * @brief 预处理图像（转换为灰度图）
     * @param inputImage 输入图像
     * @param outputGray 输出灰度图
     * @return 是否成功
     */
    static bool preprocessImage(const cv::Mat& inputImage, cv::Mat& outputGray);

    /**
     * @brief 验证参数有效性
     * @param params 要验证的参数
     * @return 是否有效
     */
    static bool validateParams(const EdgeDetectionParams& params);

private:
    EdgeDetectionParams m_params;   // 边缘检测参数
};

#endif // EDGE_DETECTOR_H
