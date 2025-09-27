#ifndef AXISCONTROLLERENUMS_H
#define AXISCONTROLLERENUMS_H

/**
 * @file AxisControllerEnums.h
 * @brief AxisController相关的枚举定义和常量
 * @version 1.0
 * @date 2024
 * 
 * 定义了轴控制系统中使用的所有枚举类型、错误码和常量
 */

namespace AxisControl {

/**
 * @brief 轴编号枚举
 */
enum class AxisIndex {
    X_AXIS = 0,     ///< X轴
    Y_AXIS = 1,     ///< Y轴  
    Z_AXIS = 2,     ///< Z轴
    INVALID_AXIS = -1
};

/**
 * @brief 轴控制错误类型
 */
enum class AxisError {
    NoError = 0,                ///< 无错误
    ConnectionFailed,           ///< 连接失败
    DeviceNotConnected,         ///< 设备未连接
    InvalidAxis,                ///< 无效的轴编号
    InvalidParameter,           ///< 无效参数
    MotionTimeout,              ///< 运动超时
    LimitReached,               ///< 到达限位
    CommunicationError,         ///< 通信错误
    HardwareError,              ///< 硬件错误
    AxisNotHomed,               ///< 轴未回零
    AxisBusy,                   ///< 轴正在运动
    EmergencyStop,              ///< 急停状态
    ParameterOutOfRange,        ///< 参数超出范围
    InitializationFailed,       ///< 初始化失败
    UnknownError = 999          ///< 未知错误
};

/**
 * @brief 运动状态枚举
 */
enum class MotionState {
    Idle = 0,           ///< 空闲状态
    Moving,             ///< 正在运动
    Stopped,            ///< 已停止
    Homing,             ///< 正在回零
    Error               ///< 错误状态
};

/**
 * @brief 限位开关状态
 */
enum class LimitState {
    None = 0,           ///< 无限位触发
    Positive = 1,       ///< 正向限位触发
    Negative = 2,       ///< 负向限位触发
    Both = 3            ///< 双向限位触发
};

/**
 * @brief 连接类型枚举
 */
enum class ConnectionType {
    Serial = 0,         ///< 串口连接
    USB,                ///< USB连接
    Ethernet,           ///< 以太网连接
    Unknown             ///< 未知连接类型
};

/**
 * @brief 运动模式枚举
 */
enum class MotionMode {
    Relative = 0,       ///< 相对运动
    Absolute,           ///< 绝对运动
    Continuous,         ///< 连续运动
    Jog                 ///< 点动模式
};

/**
 * @brief 系统常量定义
 */
namespace Constants {
    // 轴数量限制
    constexpr int MAX_AXIS_COUNT = 3;
    constexpr int MIN_AXIS_INDEX = 0;
    constexpr int MAX_AXIS_INDEX = 2;
    
    // 运动参数限制
    constexpr double MIN_SPEED = 0.1;     ///< 最小速度 (μm/s) 
    constexpr double MAX_SPEED = 7000;   ///< 最大速度 (μm/s)
    constexpr double MIN_ACCELERATION = 0.1;     ///< 最小加速度 (mm/s²)
    constexpr double MAX_ACCELERATION = 7000;   ///< 最大加速度 (mm/s²)
    
    // 位置限制  
    constexpr double MIN_POSITION = -999999.9;  ///< 最小位置 (μm)
    constexpr double MAX_POSITION = 999999.9;   ///< 最大位置 (μm)
    constexpr double POSITION_TOLERANCE = 0.1;   ///< 位置容差 (μm)
    
    // 通信参数
    constexpr int DEFAULT_BAUDRATE = 9600;       ///< 默认波特率
    constexpr int CONNECTION_TIMEOUT = 3000;     ///< 连接超时 (ms)
    constexpr int MOTION_TIMEOUT = 30000;        ///< 运动超时 (ms)
    constexpr int STATUS_UPDATE_INTERVAL = 100;  ///< 状态更新间隔 (ms)

    
    // 单位转换
    constexpr double UM_TO_MM = 0.001;          ///< 微米到毫米转换
    constexpr double MM_TO_UM = 1000.0;         ///< 毫米到微米转换
}

/**
 * @brief 错误码到字符串转换
 */
inline const char* errorToString(AxisError error) {
    switch (error) {
        case AxisError::NoError:                return "无错误";
        case AxisError::ConnectionFailed:       return "连接失败";
        case AxisError::DeviceNotConnected:     return "设备未连接";
        case AxisError::InvalidAxis:            return "无效的轴编号";
        case AxisError::InvalidParameter:       return "无效参数";
        case AxisError::MotionTimeout:          return "运动超时";
        case AxisError::LimitReached:           return "到达限位";
        case AxisError::CommunicationError:     return "通信错误";
        case AxisError::HardwareError:          return "硬件错误";
        case AxisError::AxisNotHomed:           return "轴未回零";
        case AxisError::AxisBusy:               return "轴正在运动";
        case AxisError::EmergencyStop:          return "急停状态";
        case AxisError::ParameterOutOfRange:    return "参数超出范围";
        case AxisError::InitializationFailed:   return "初始化失败";
        default:                               return "未知错误";
    }
}

/**
 * @brief 运动状态到字符串转换
 */
inline const char* motionStateToString(MotionState state) {
    switch (state) {
        case MotionState::Idle:     return "空闲";
        case MotionState::Moving:   return "运动中";
        case MotionState::Stopped:  return "已停止";
        case MotionState::Homing:   return "回零中";
        case MotionState::Error:    return "错误状态";
        default:                   return "未知状态";
    }
}

/**
 * @brief 轴名称到字符串转换
 */
inline const char* axisToString(AxisIndex axis) {
    switch (axis) {
        case AxisIndex::X_AXIS:     return "X轴";
        case AxisIndex::Y_AXIS:     return "Y轴";
        case AxisIndex::Z_AXIS:     return "Z轴";
        default:                   return "无效轴";
    }
}

} // namespace AxisControl

#endif // AXISCONTROLLERENUMS_H
