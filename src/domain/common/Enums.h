#pragma once

namespace MutiCamApp {
namespace Domain {

// 相机类型枚举
enum class CameraType {
    USB_CAMERA = 0,
    GIGE_CAMERA = 1,
    UNKNOWN = -1
};

// 相机状态枚举
enum class CameraStatus {
    DISCONNECTED = 0,
    CONNECTED = 1,
    CAPTURING = 2,
    ERROR = 3,
    INITIALIZING = 4
};

// 相机视图类型
enum class CameraView {
    VERTICAL = 0,      // 垂直视图
    LEFT = 1,          // 左侧视图  
    FRONT = 2          // 对向视图
};

// 测量类型枚举
enum class MeasurementType {
    POINT = 0,                  // 点测量
    LINE = 1,                   // 直线测量
    LINE_SEGMENT = 2,           // 线段测量
    CIRCLE = 3,                 // 圆形测量
    SIMPLE_CIRCLE = 4,          // 简单圆
    FINE_CIRCLE = 5,            // 精细圆
    PARALLEL_LINES = 6,         // 平行线测量
    CIRCLE_LINE = 7,            // 圆线距离测量
    TWO_LINES_ANGLE = 8,        // 两线夹角测量
    POINT_TO_POINT = 9,         // 点到点距离
    POINT_TO_LINE = 10,         // 点到线距离
    POINT_TO_CIRCLE = 11,       // 点到圆距离
    LINE_TO_CIRCLE = 12,        // 线到圆距离
    LINE_SEGMENT_ANGLE = 13     // 线段角度测量
};

// 绘图类型枚举
enum class DrawingType {
    NONE = 0,
    MANUAL_DRAWING = 1,         // 手动绘制
    AUTO_DETECTION = 2,         // 自动检测
    GRID_OVERLAY = 3,           // 网格叠加
    MEASUREMENT_RESULT = 4      // 测量结果
};

// 绘图对象状态
enum class DrawingObjectState {
    CREATING = 0,               // 正在创建
    COMPLETED = 1,              // 已完成
    SELECTED = 2,               // 已选中
    EDITING = 3,                // 正在编辑
    HIDDEN = 4                  // 隐藏状态
};

// 图层类型
enum class LayerType {
    BASE_IMAGE = 0,             // 基础图像层
    MANUAL_DRAWING = 1,         // 手动绘制层
    AUTO_DETECTION = 2,         // 自动检测层
    MEASUREMENT_ANNOTATION = 3, // 测量标注层
    GRID_OVERLAY = 4,           // 网格叠加层
    UI_OVERLAY = 5              // UI叠加层
};

// 检测算法类型
enum class DetectionAlgorithm {
    CANNY_EDGE = 0,             // Canny边缘检测
    HOUGH_LINES = 1,            // Hough直线检测
    HOUGH_CIRCLES = 2,          // Hough圆形检测
    CONTOUR_DETECTION = 3,      // 轮廓检测
    TEMPLATE_MATCHING = 4       // 模板匹配
};

// 图像滤波类型
enum class FilterType {
    NONE = 0,
    GAUSSIAN_BLUR = 1,          // 高斯模糊
    MEDIAN_BLUR = 2,            // 中值滤波
    BILATERAL = 3,              // 双边滤波
    MORPHOLOGY_OPEN = 4,        // 形态学开运算
    MORPHOLOGY_CLOSE = 5,       // 形态学闭运算
    LAPLACIAN = 6,              // 拉普拉斯锐化
    SOBEL_X = 7,                // Sobel X方向
    SOBEL_Y = 8                 // Sobel Y方向
};

// 标定类型
enum class CalibrationType {
    PIXEL_SCALE = 0,            // 像素比例标定
    SINGLE_POINT = 1,           // 单点标定
    TWO_POINT = 2,              // 两点标定
    MULTI_POINT = 3,            // 多点标定
    CHECKERBOARD = 4            // 棋盘格标定
};

// 操作模式
enum class OperationMode {
    IDLE = 0,                   // 空闲模式
    DRAWING = 1,                // 绘制模式
    MEASURING = 2,              // 测量模式
    DETECTING = 3,              // 检测模式
    CALIBRATING = 4,            // 标定模式
    REVIEWING = 5               // 审查模式
};

// 鼠标交互模式
enum class MouseMode {
    NONE = 0,
    PAN = 1,                    // 平移
    ZOOM = 2,                   // 缩放
    DRAW = 3,                   // 绘制
    SELECT = 4,                 // 选择
    EDIT = 5                    // 编辑
};

// 日志级别
enum class LogLevel {
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARNING = 3,
    ERROR = 4,
    CRITICAL = 5
};

// 配置类别
enum class ConfigCategory {
    CAMERA = 0,                 // 相机配置
    DETECTION = 1,              // 检测配置
    UI = 2,                     // 界面配置
    MEASUREMENT = 3,            // 测量配置
    SYSTEM = 4                  // 系统配置
};

// 错误代码
enum class ErrorCode {
    SUCCESS = 0,
    CAMERA_NOT_FOUND = 1001,
    CAMERA_INIT_FAILED = 1002,
    CAMERA_START_FAILED = 1003,
    CAMERA_CAPTURE_FAILED = 1004,
    IMAGE_PROCESSING_FAILED = 2001,
    MEASUREMENT_CALCULATION_FAILED = 3001,
    FILE_NOT_FOUND = 4001,
    FILE_READ_ERROR = 4002,
    FILE_WRITE_ERROR = 4003,
    CONFIG_LOAD_ERROR = 5001,
    CONFIG_SAVE_ERROR = 5002,
    INVALID_PARAMETER = 6001,
    OUT_OF_MEMORY = 6002,
    THREAD_ERROR = 6003,
    UNKNOWN_ERROR = 9999
};

// 单位类型
enum class UnitType {
    PIXEL = 0,                  // 像素
    MILLIMETER = 1,             // 毫米
    MICROMETER = 2,             // 微米
    INCH = 3,                   // 英寸
    CENTIMETER = 4              // 厘米
};

// 坐标系类型
enum class CoordinateSystem {
    IMAGE_COORDINATES = 0,      // 图像坐标系
    WORLD_COORDINATES = 1,      // 世界坐标系
    CAMERA_COORDINATES = 2      // 相机坐标系
};

} // namespace Domain
} // namespace MutiCamApp 