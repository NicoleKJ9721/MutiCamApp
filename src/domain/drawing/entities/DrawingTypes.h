#ifndef DRAWING_TYPES_H
#define DRAWING_TYPES_H

#include <QPoint>
#include <QColor>
#include <vector>
#include <map>
#include <string>

/**
 * 绘图类型枚举
 * 对应Python版本的DrawingType枚举
 */
enum class DrawingType {
    POINT,              // 点
    LINE,               // 直线
    CIRCLE,             // 圆形
    LINE_SEGMENT,       // 线段
    PARALLEL,           // 平行线
    CIRCLE_LINE,        // 圆线测量
    TWO_LINES,          // 两线夹角
    LINE_DETECT,        // 直线检测
    CIRCLE_DETECT,      // 圆检测
    POINT_TO_POINT,     // 点与点
    POINT_TO_LINE,      // 点与线
    POINT_TO_CIRCLE,    // 点与圆
    LINE_SEGMENT_TO_CIRCLE, // 线段与圆
    LINE_TO_CIRCLE,     // 直线与圆
    SIMPLE_CIRCLE,      // 简单圆
    FINE_CIRCLE,        // 精细圆
    LINE_SEGMENT_ANGLE  // 线段角度
};

/**
 * 绘图对象属性结构
 * 存储颜色、线宽等属性
 */
struct DrawingProperties {
    QColor color = QColor(0, 255, 0);  // 默认绿色
    int thickness = 2;                  // 线条粗细
    int radius = 10;                    // 点的半径
    bool visible = true;                // 是否可见
    bool selected = false;              // 是否被选中
    bool isDetection = false;           // 是否为检测结果
    bool isDashed = false;              // 是否为虚线
    bool showDistance = false;          // 是否显示距离
    
    // 额外属性（用于复杂绘图类型）
    std::map<std::string, std::string> extraProperties;
};

/**
 * 绘图对象数据结构
 * 对应Python版本的DrawingObject
 */
struct DrawingObject {
    DrawingType type;                   // 绘图类型
    std::vector<QPoint> points;         // 点集合
    DrawingProperties properties;       // 属性
    
    DrawingObject(DrawingType t) : type(t) {}
    
    DrawingObject(DrawingType t, const std::vector<QPoint>& pts, const DrawingProperties& props)
        : type(t), points(pts), properties(props) {}
};

/**
 * 绘图类型名称映射
 */
inline std::string getDrawingTypeName(DrawingType type) {
    switch (type) {
        case DrawingType::POINT: return "点";
        case DrawingType::LINE: return "直线";
        case DrawingType::CIRCLE: return "圆形";
        case DrawingType::LINE_SEGMENT: return "线段";
        case DrawingType::PARALLEL: return "平行线";
        case DrawingType::CIRCLE_LINE: return "圆线测量";
        case DrawingType::TWO_LINES: return "两线夹角";
        case DrawingType::LINE_DETECT: return "直线检测";
        case DrawingType::CIRCLE_DETECT: return "圆检测";
        case DrawingType::POINT_TO_POINT: return "点与点";
        case DrawingType::POINT_TO_LINE: return "点与线";
        case DrawingType::POINT_TO_CIRCLE: return "点与圆";
        case DrawingType::LINE_SEGMENT_TO_CIRCLE: return "线段与圆";
        case DrawingType::LINE_TO_CIRCLE: return "直线与圆";
        case DrawingType::SIMPLE_CIRCLE: return "简单圆";
        case DrawingType::FINE_CIRCLE: return "精细圆";
        case DrawingType::LINE_SEGMENT_ANGLE: return "线段角度";
        default: return "未知";
    }
}

#endif // DRAWING_TYPES_H 