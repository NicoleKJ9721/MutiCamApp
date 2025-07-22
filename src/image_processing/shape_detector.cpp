#include "shape_detector.h"
#include <iostream>
#include <algorithm>

ShapeDetector::ShapeDetector()
    : m_lineParams(), m_circleParams()
{
}

void ShapeDetector::setLineDetectionParams(const LineDetectionParams& params)
{
    if (validateLineParams(params)) {
        m_lineParams = params;
    } else {
        std::cerr << "错误：直线检测参数无效，保持原参数不变" << std::endl;
    }
}

ShapeDetector::LineDetectionParams ShapeDetector::getLineDetectionParams() const
{
    return m_lineParams;
}

bool ShapeDetector::detectLines(const cv::Mat& edgeImage, QVector<DetectedLine>& detectedLines)
{
    try {
        detectedLines.clear();

        if (edgeImage.empty()) {
            std::cerr << "错误：边缘图像为空" << std::endl;
            return false;
        }

        if (edgeImage.type() != CV_8UC1) {
            std::cerr << "错误：边缘图像必须是单通道二值图" << std::endl;
            return false;
        }

        // 使用霍夫变换检测直线
        std::vector<cv::Vec4i> lines;
        cv::HoughLinesP(edgeImage, lines,
                        m_lineParams.rho,
                        m_lineParams.theta,
                        m_lineParams.threshold,
                        m_lineParams.minLineLength,
                        m_lineParams.maxLineGap);

        // 转换为DetectedLine格式
        for (const auto& line : lines) {
            QPoint start(line[0], line[1]);
            QPoint end(line[2], line[3]);
            
            DetectedLine detectedLine(start, end);
            detectedLine.confidence = calculateLineConfidence(line, edgeImage);
            
            detectedLines.append(detectedLine);
        }

        std::cout << "检测到 " << detectedLines.size() << " 条直线" << std::endl;
        return true;

    } catch (const cv::Exception& e) {
        std::cerr << "OpenCV异常：" << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "标准异常：" << e.what() << std::endl;
        return false;
    }
}

bool ShapeDetector::detectLinesInROI(const cv::Mat& edgeImage, const cv::Rect& roi, QVector<DetectedLine>& detectedLines)
{
    try {
        detectedLines.clear();

        if (edgeImage.empty()) {
            std::cerr << "错误：边缘图像为空" << std::endl;
            return false;
        }

        // 验证ROI区域有效性
        if (roi.x < 0 || roi.y < 0 || 
            roi.x + roi.width > edgeImage.cols || 
            roi.y + roi.height > edgeImage.rows ||
            roi.width <= 0 || roi.height <= 0) {
            std::cerr << "错误：ROI区域无效" << std::endl;
            return false;
        }

        // 提取ROI区域
        cv::Mat roiEdges = edgeImage(roi);

        // 在ROI区域内检测直线
        QVector<DetectedLine> roiLines;
        if (!detectLines(roiEdges, roiLines)) {
            return false;
        }

        // 将ROI坐标转换为原图坐标
        for (auto& line : roiLines) {
            line.start.setX(line.start.x() + roi.x);
            line.start.setY(line.start.y() + roi.y);
            line.end.setX(line.end.x() + roi.x);
            line.end.setY(line.end.y() + roi.y);
            
            // 重新计算属性（长度和角度不变，但为了保持一致性）
            line.calculateProperties();
        }

        detectedLines = roiLines;
        return true;

    } catch (const cv::Exception& e) {
        std::cerr << "OpenCV异常：" << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "标准异常：" << e.what() << std::endl;
        return false;
    }
}

ShapeDetector::DetectedLine ShapeDetector::selectBestLine(const QVector<DetectedLine>& detectedLines)
{
    if (detectedLines.isEmpty()) {
        return DetectedLine();
    }

    // 选择最长的直线作为最佳直线
    auto bestLine = std::max_element(detectedLines.begin(), detectedLines.end(),
        [](const DetectedLine& a, const DetectedLine& b) {
            return a.length < b.length;
        });

    return *bestLine;
}

void ShapeDetector::setCircleDetectionParams(const CircleDetectionParams& params)
{
    if (validateCircleParams(params)) {
        m_circleParams = params;
    } else {
        std::cerr << "错误：圆形检测参数无效，保持原参数不变" << std::endl;
    }
}

ShapeDetector::CircleDetectionParams ShapeDetector::getCircleDetectionParams() const
{
    return m_circleParams;
}

bool ShapeDetector::detectCircles(const cv::Mat& grayImage, QVector<DetectedCircle>& detectedCircles)
{
    try {
        detectedCircles.clear();

        if (grayImage.empty()) {
            std::cerr << "错误：灰度图像为空" << std::endl;
            return false;
        }

        if (grayImage.type() != CV_8UC1) {
            std::cerr << "错误：输入图像必须是单通道灰度图" << std::endl;
            return false;
        }

        // 使用霍夫圆变换检测圆形
        std::vector<cv::Vec3f> circles;
        cv::HoughCircles(grayImage, circles,
                         cv::HOUGH_GRADIENT,
                         m_circleParams.dp,
                         m_circleParams.minDist,
                         m_circleParams.param1,
                         m_circleParams.param2,
                         m_circleParams.minRadius,
                         m_circleParams.maxRadius);

        // 转换为DetectedCircle格式
        for (const auto& circle : circles) {
            QPoint center(static_cast<int>(circle[0]), static_cast<int>(circle[1]));
            int radius = static_cast<int>(circle[2]);
            double confidence = calculateCircleConfidence(circle, grayImage);
            
            DetectedCircle detectedCircle(center, radius, confidence);
            detectedCircles.append(detectedCircle);
        }

        std::cout << "检测到 " << detectedCircles.size() << " 个圆形" << std::endl;
        return true;

    } catch (const cv::Exception& e) {
        std::cerr << "OpenCV异常：" << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "标准异常：" << e.what() << std::endl;
        return false;
    }
}

bool ShapeDetector::detectCirclesInROI(const cv::Mat& grayImage, const cv::Rect& roi, QVector<DetectedCircle>& detectedCircles)
{
    try {
        detectedCircles.clear();

        if (grayImage.empty()) {
            std::cerr << "错误：灰度图像为空" << std::endl;
            return false;
        }

        // 验证ROI区域有效性
        if (roi.x < 0 || roi.y < 0 ||
            roi.x + roi.width > grayImage.cols ||
            roi.y + roi.height > grayImage.rows ||
            roi.width <= 0 || roi.height <= 0) {
            std::cerr << "错误：ROI区域无效" << std::endl;
            return false;
        }

        // 提取ROI区域
        cv::Mat roiGray = grayImage(roi);

        // 在ROI区域内检测圆形
        QVector<DetectedCircle> roiCircles;
        if (!detectCircles(roiGray, roiCircles)) {
            return false;
        }

        // 将ROI坐标转换为原图坐标
        for (auto& circle : roiCircles) {
            circle.center.setX(circle.center.x() + roi.x);
            circle.center.setY(circle.center.y() + roi.y);
        }

        detectedCircles = roiCircles;
        return true;

    } catch (const cv::Exception& e) {
        std::cerr << "OpenCV异常：" << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "标准异常：" << e.what() << std::endl;
        return false;
    }
}

ShapeDetector::DetectedCircle ShapeDetector::selectBestCircle(const QVector<DetectedCircle>& detectedCircles)
{
    if (detectedCircles.isEmpty()) {
        return DetectedCircle();
    }

    // 选择置信度最高的圆形作为最佳圆形
    auto bestCircle = std::max_element(detectedCircles.begin(), detectedCircles.end(),
        [](const DetectedCircle& a, const DetectedCircle& b) {
            return a.confidence < b.confidence;
        });

    return *bestCircle;
}

bool ShapeDetector::validateLineParams(const LineDetectionParams& params)
{
    if (params.rho <= 0) {
        std::cerr << "错误：距离分辨率必须大于0" << std::endl;
        return false;
    }

    if (params.theta <= 0 || params.theta > CV_PI) {
        std::cerr << "错误：角度分辨率必须在(0, π]范围内" << std::endl;
        return false;
    }

    if (params.threshold <= 0) {
        std::cerr << "错误：累加器阈值必须大于0" << std::endl;
        return false;
    }

    if (params.minLineLength < 0) {
        std::cerr << "错误：最小线段长度不能为负" << std::endl;
        return false;
    }

    if (params.maxLineGap < 0) {
        std::cerr << "错误：最大线段间隙不能为负" << std::endl;
        return false;
    }

    return true;
}

bool ShapeDetector::validateCircleParams(const CircleDetectionParams& params)
{
    if (params.dp <= 0) {
        std::cerr << "错误：累加器分辨率倍数必须大于0" << std::endl;
        return false;
    }

    if (params.minDist <= 0) {
        std::cerr << "错误：圆心间最小距离必须大于0" << std::endl;
        return false;
    }

    if (params.param1 <= 0 || params.param2 <= 0) {
        std::cerr << "错误：Canny阈值和累加器阈值必须大于0" << std::endl;
        return false;
    }

    if (params.minRadius < 0 || params.maxRadius < 0) {
        std::cerr << "错误：半径范围不能为负" << std::endl;
        return false;
    }

    if (params.minRadius >= params.maxRadius) {
        std::cerr << "错误：最小半径必须小于最大半径" << std::endl;
        return false;
    }

    return true;
}

cv::Vec4i ShapeDetector::qPointsToVec4i(const QPoint& start, const QPoint& end)
{
    return cv::Vec4i(start.x(), start.y(), end.x(), end.y());
}

QPoint ShapeDetector::vec4iToQPoint(const cv::Vec4i& line, bool isStart)
{
    if (isStart) {
        return QPoint(line[0], line[1]);
    } else {
        return QPoint(line[2], line[3]);
    }
}

double ShapeDetector::calculateLineConfidence(const cv::Vec4i& line, const cv::Mat& edgeImage)
{
    try {
        // 简单的置信度计算：基于线段长度
        int dx = line[2] - line[0];
        int dy = line[3] - line[1];
        double length = std::sqrt(dx * dx + dy * dy);

        // 长度越长，置信度越高（归一化到0-100）
        double confidence = std::min(100.0, length / 10.0);

        return confidence;

    } catch (const std::exception& e) {
        std::cerr << "计算直线置信度失败：" << e.what() << std::endl;
        return 0.0;
    }
}

double ShapeDetector::calculateCircleConfidence(const cv::Vec3f& circle, const cv::Mat& grayImage)
{
    try {
        // 简单的置信度计算：基于圆的半径和图像对比度
        float radius = circle[2];

        // 半径在合理范围内的置信度更高
        double radiusScore = 100.0;
        if (radius < 10 || radius > 200) {
            radiusScore = 50.0;
        }

        // 可以在这里添加更复杂的置信度计算，比如检查圆周上的边缘点数量
        return radiusScore;

    } catch (const std::exception& e) {
        std::cerr << "计算圆形置信度失败：" << e.what() << std::endl;
        return 0.0;
    }
}
