#pragma once

#include <vector>
#include <memory>
#include "../../common/Enums.h"

namespace MutiCamApp {
namespace Domain {

// 前向声明
class Point;
class Line;
class Circle;
class MeasurementResult;

/**
 * @brief 测量计算接口
 * 定义了所有几何测量和计算功能
 */
class IMeasurementCalculator {
public:
    virtual ~IMeasurementCalculator() = default;
    
    // === 基础距离测量 ===
    
    /**
     * @brief 计算两点之间的距离
     * @param p1 第一个点
     * @param p2 第二个点
     * @return 距离值
     */
    virtual double CalculateDistance(const Point& p1, const Point& p2) const = 0;
    
    /**
     * @brief 计算点到直线的距离
     * @param point 点
     * @param line 直线
     * @return 距离值
     */
    virtual double CalculatePointToLineDistance(const Point& point, const Line& line) const = 0;
    
    /**
     * @brief 计算点到圆的距离（最近距离）
     * @param point 点
     * @param circle 圆
     * @return 距离值
     */
    virtual double CalculatePointToCircleDistance(const Point& point, const Circle& circle) const = 0;
    
    /**
     * @brief 计算两条平行线之间的距离
     * @param line1 第一条直线
     * @param line2 第二条直线
     * @return 距离值，如果不平行返回-1
     */
    virtual double CalculateParallelLinesDistance(const Line& line1, const Line& line2) const = 0;
    
    /**
     * @brief 计算直线到圆的距离（最近距离）
     * @param line 直线
     * @param circle 圆
     * @return 距离值
     */
    virtual double CalculateLineToCircleDistance(const Line& line, const Circle& circle) const = 0;
    
    // === 角度测量 ===
    
    /**
     * @brief 计算两条直线的夹角
     * @param line1 第一条直线
     * @param line2 第二条直线
     * @return 夹角（度数，0-180度）
     */
    virtual double CalculateAngle(const Line& line1, const Line& line2) const = 0;
    
    /**
     * @brief 计算直线的倾斜角度
     * @param line 直线
     * @return 倾斜角度（度数，-90到90度）
     */
    virtual double CalculateLineAngle(const Line& line) const = 0;
    
    /**
     * @brief 计算线段的角度
     * @param startPoint 起点
     * @param endPoint 终点
     * @return 角度（度数，0-360度）
     */
    virtual double CalculateSegmentAngle(const Point& startPoint, const Point& endPoint) const = 0;
    
    // === 几何拟合 ===
    
    /**
     * @brief 通过多个点拟合直线
     * @param points 点集合
     * @return 拟合的直线
     */
    virtual Line FitLine(const std::vector<Point>& points) const = 0;
    
    /**
     * @brief 通过三个点拟合圆
     * @param p1 第一个点
     * @param p2 第二个点
     * @param p3 第三个点
     * @return 拟合的圆
     */
    virtual Circle FitCircleFromThreePoints(const Point& p1, const Point& p2, const Point& p3) const = 0;
    
    /**
     * @brief 通过多个点拟合圆（最小二乘法）
     * @param points 点集合
     * @return 拟合的圆
     */
    virtual Circle FitCircle(const std::vector<Point>& points) const = 0;
    
    // === 几何计算 ===
    
    /**
     * @brief 计算两条直线的交点
     * @param line1 第一条直线
     * @param line2 第二条直线
     * @return 交点，如果平行返回无效点
     */
    virtual Point CalculateIntersection(const Line& line1, const Line& line2) const = 0;
    
    /**
     * @brief 计算直线与圆的交点
     * @param line 直线
     * @param circle 圆
     * @return 交点集合（0、1或2个点）
     */
    virtual std::vector<Point> CalculateLineCircleIntersection(const Line& line, const Circle& circle) const = 0;
    
    /**
     * @brief 计算两个圆的交点
     * @param circle1 第一个圆
     * @param circle2 第二个圆
     * @return 交点集合（0、1或2个点）
     */
    virtual std::vector<Point> CalculateCircleIntersection(const Circle& circle1, const Circle& circle2) const = 0;
    
    /**
     * @brief 计算点在直线上的投影点
     * @param point 点
     * @param line 直线
     * @return 投影点
     */
    virtual Point CalculateProjection(const Point& point, const Line& line) const = 0;
    
    // === 几何属性计算 ===
    
    /**
     * @brief 计算直线的长度（在指定范围内）
     * @param line 直线
     * @param startPoint 起始点
     * @param endPoint 结束点
     * @return 长度
     */
    virtual double CalculateLineLength(const Line& line, const Point& startPoint, const Point& endPoint) const = 0;
    
    /**
     * @brief 计算圆的周长
     * @param circle 圆
     * @return 周长
     */
    virtual double CalculateCirclePerimeter(const Circle& circle) const = 0;
    
    /**
     * @brief 计算圆的面积
     * @param circle 圆
     * @return 面积
     */
    virtual double CalculateCircleArea(const Circle& circle) const = 0;
    
    // === 标定和转换 ===
    
    /**
     * @brief 设置像素到实际距离的比例
     * @param pixelScale 像素比例（像素/毫米）
     */
    virtual void SetPixelScale(double pixelScale) = 0;
    
    /**
     * @brief 获取像素比例
     * @return 像素比例
     */
    virtual double GetPixelScale() const = 0;
    
    /**
     * @brief 将像素距离转换为实际距离
     * @param pixelDistance 像素距离
     * @param unit 目标单位
     * @return 实际距离
     */
    virtual double ConvertPixelToReal(double pixelDistance, UnitType unit = UnitType::MILLIMETER) const = 0;
    
    /**
     * @brief 将实际距离转换为像素距离
     * @param realDistance 实际距离
     * @param unit 源单位
     * @return 像素距离
     */
    virtual double ConvertRealToPixel(double realDistance, UnitType unit = UnitType::MILLIMETER) const = 0;
    
    // === 精度和验证 ===
    
    /**
     * @brief 验证点是否在直线上
     * @param point 点
     * @param line 直线
     * @param tolerance 容差
     * @return 是否在直线上
     */
    virtual bool IsPointOnLine(const Point& point, const Line& line, double tolerance = 1.0) const = 0;
    
    /**
     * @brief 验证点是否在圆上
     * @param point 点
     * @param circle 圆
     * @param tolerance 容差
     * @return 是否在圆上
     */
    virtual bool IsPointOnCircle(const Point& point, const Circle& circle, double tolerance = 1.0) const = 0;
    
    /**
     * @brief 验证两条直线是否平行
     * @param line1 第一条直线
     * @param line2 第二条直线
     * @param angleThreshold 角度阈值（度数）
     * @return 是否平行
     */
    virtual bool AreParallel(const Line& line1, const Line& line2, double angleThreshold = 1.0) const = 0;
    
    /**
     * @brief 验证两条直线是否垂直
     * @param line1 第一条直线
     * @param line2 第二条直线
     * @param angleThreshold 角度阈值（度数）
     * @return 是否垂直
     */
    virtual bool ArePerpendicular(const Line& line1, const Line& line2, double angleThreshold = 1.0) const = 0;
    
    // === 复合测量 ===
    
    /**
     * @brief 执行完整的测量计算
     * @param measurementType 测量类型
     * @param points 输入点集合
     * @return 测量结果
     */
    virtual std::shared_ptr<MeasurementResult> PerformMeasurement(
        MeasurementType measurementType, 
        const std::vector<Point>& points) const = 0;
    
    /**
     * @brief 批量执行测量
     * @param measurements 测量任务列表
     * @return 测量结果列表
     */
    virtual std::vector<std::shared_ptr<MeasurementResult>> PerformBatchMeasurement(
        const std::vector<std::pair<MeasurementType, std::vector<Point>>>& measurements) const = 0;
    
    // === 统计分析 ===
    
    /**
     * @brief 计算点集的重心
     * @param points 点集合
     * @return 重心点
     */
    virtual Point CalculateCentroid(const std::vector<Point>& points) const = 0;
    
    /**
     * @brief 计算点集的标准差
     * @param points 点集合
     * @return 标准差
     */
    virtual double CalculateStandardDeviation(const std::vector<Point>& points) const = 0;
    
    /**
     * @brief 计算拟合误差
     * @param points 原始点集合
     * @param fittedGeometry 拟合的几何体（直线或圆）
     * @return 拟合误差
     */
    virtual double CalculateFittingError(const std::vector<Point>& points, const void* fittedGeometry) const = 0;
};

} // namespace Domain
} // namespace MutiCamApp 