#ifndef SHAPE_DETECTOR_H
#define SHAPE_DETECTOR_H

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <QPoint>
#include <QVector>

/**
 * @brief 形状检测器类
 * 封装霍夫变换等形状检测算法
 */
class ShapeDetector
{
public:
    /**
     * @brief 直线检测参数结构体
     */
    struct LineDetectionParams {
        double rho = 1.0;               // 距离分辨率（像素）
        double theta = CV_PI / 180.0;   // 角度分辨率（弧度）
        int threshold = 50;             // 累加器阈值
        double minLineLength = 50.0;    // 最小线段长度
        double maxLineGap = 10.0;       // 最大线段间隙
        
        LineDetectionParams() = default;
        LineDetectionParams(double r, double t, int thresh, double minLen, double maxGap)
            : rho(r), theta(t), threshold(thresh), minLineLength(minLen), maxLineGap(maxGap) {}
    };

    /**
     * @brief 检测到的直线结构体
     */
    struct DetectedLine {
        QPoint start;       // 起点
        QPoint end;         // 终点
        double length;      // 长度
        double angle;       // 角度（度）
        double confidence;  // 置信度
        
        DetectedLine() : length(0), angle(0), confidence(0) {}
        DetectedLine(const QPoint& s, const QPoint& e) 
            : start(s), end(e) {
            calculateProperties();
        }
        
        void calculateProperties() {
            // 计算长度
            int dx = end.x() - start.x();
            int dy = end.y() - start.y();
            length = std::sqrt(dx * dx + dy * dy);
            
            // 计算角度（度）
            angle = std::atan2(dy, dx) * 180.0 / CV_PI;
            if (angle < 0) angle += 180.0; // 转换为0-180度
        }
    };

    /**
     * @brief 圆形检测参数结构体
     */
    struct CircleDetectionParams {
        double dp = 1.0;                // 累加器分辨率倍数
        double minDist = 50.0;          // 圆心间最小距离
        double param1 = 200.0;          // Canny高阈值
        double param2 = 50.0;           // 累加器阈值
        int minRadius = 10;             // 最小半径
        int maxRadius = 200;            // 最大半径
        
        CircleDetectionParams() = default;
        CircleDetectionParams(double d, double minD, double p1, double p2, int minR, int maxR)
            : dp(d), minDist(minD), param1(p1), param2(p2), minRadius(minR), maxRadius(maxR) {}
    };

    /**
     * @brief 检测到的圆形结构体
     */
    struct DetectedCircle {
        QPoint center;      // 圆心
        int radius;         // 半径
        double confidence;  // 置信度
        
        DetectedCircle() : radius(0), confidence(0) {}
        DetectedCircle(const QPoint& c, int r, double conf = 0) 
            : center(c), radius(r), confidence(conf) {}
    };

public:
    ShapeDetector();
    ~ShapeDetector() = default;

    // 直线检测相关方法
    void setLineDetectionParams(const LineDetectionParams& params);
    LineDetectionParams getLineDetectionParams() const;

    /**
     * @brief 在边缘图像中检测直线
     * @param edgeImage 边缘图像（二值图）
     * @param detectedLines 检测到的直线列表
     * @return 是否成功
     */
    bool detectLines(const cv::Mat& edgeImage, QVector<DetectedLine>& detectedLines);

    /**
     * @brief 在ROI区域内检测直线
     * @param edgeImage 边缘图像
     * @param roi ROI矩形区域
     * @param detectedLines 检测到的直线列表（坐标已转换为原图坐标系）
     * @return 是否成功
     */
    bool detectLinesInROI(const cv::Mat& edgeImage, const cv::Rect& roi, QVector<DetectedLine>& detectedLines);

    /**
     * @brief 选择最佳直线（通常是最长的）
     * @param detectedLines 检测到的直线列表
     * @return 最佳直线，如果没有则返回空的DetectedLine
     */
    DetectedLine selectBestLine(const QVector<DetectedLine>& detectedLines);

    // 圆形检测相关方法
    void setCircleDetectionParams(const CircleDetectionParams& params);
    CircleDetectionParams getCircleDetectionParams() const;

    /**
     * @brief 在图像中检测圆形
     * @param grayImage 灰度图像
     * @param detectedCircles 检测到的圆形列表
     * @return 是否成功
     */
    bool detectCircles(const cv::Mat& grayImage, QVector<DetectedCircle>& detectedCircles);

    /**
     * @brief 在ROI区域内检测圆形
     * @param grayImage 灰度图像
     * @param roi ROI矩形区域
     * @param detectedCircles 检测到的圆形列表（坐标已转换为原图坐标系）
     * @return 是否成功
     */
    bool detectCirclesInROI(const cv::Mat& grayImage, const cv::Rect& roi, QVector<DetectedCircle>& detectedCircles);

    /**
     * @brief 选择最佳圆形（基于置信度）
     * @param detectedCircles 检测到的圆形列表
     * @return 最佳圆形，如果没有则返回空的DetectedCircle
     */
    DetectedCircle selectBestCircle(const QVector<DetectedCircle>& detectedCircles);

    // 参数验证方法
    static bool validateLineParams(const LineDetectionParams& params);
    static bool validateCircleParams(const CircleDetectionParams& params);

private:
    LineDetectionParams m_lineParams;       // 直线检测参数
    CircleDetectionParams m_circleParams;   // 圆形检测参数

    // 辅助方法
    cv::Vec4i qPointsToVec4i(const QPoint& start, const QPoint& end);
    QPoint vec4iToQPoint(const cv::Vec4i& line, bool isStart);
    double calculateLineConfidence(const cv::Vec4i& line, const cv::Mat& edgeImage);
    double calculateCircleConfidence(const cv::Vec3f& circle, const cv::Mat& grayImage);
};

#endif // SHAPE_DETECTOR_H
