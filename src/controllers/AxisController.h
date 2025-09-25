#ifndef AXISCONTROLLER_H
#define AXISCONTROLLER_H

#include <QObject>
#include <QMutex>
#include <QTimer>
#include <QThread>
#include <QString>
#include <QDebug>
#include <array>
#include <memory>
#include <atomic>

#include "AxisControllerEnums.h"
#include "third_party/mcc6/include/MCC6DLL.h"

using namespace AxisControl;

/**
 * @brief MCC6DLL返回值常量
 */
namespace MCC6Constants {
    constexpr int MCC6_SUCCESS = 0x01;      ///< MCC6成功返回值 (funResOk)
    constexpr int MCC6_ERROR = 0x83;        ///< MCC6错误返回值 (funResErr)
    constexpr int MCC6_AXIS_ERROR = 0x02;   ///< 轴序号错误 (funResErrAxisId)
    constexpr int MCC6_PORT_ERROR = 0x80;   ///< 串口打开失败 (funResOpenPortErr)
}

/**
 * @class AxisController
 * @brief 完整全面的轴控制系统类
 * 
 * 提供完整的多轴运动控制功能，包括：
 * - 设备连接管理
 * - 多轴运动控制  
 * - 实时状态监控
 * - 参数配置管理
 * - 完善的错误处理
 * - 线程安全设计
 * 
 * @version 1.0
 * @date 2024
 */
class AxisController : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 运动参数结构体
     */
    struct MotionParams {
        double maxSpeed = 3000.0;          ///< 最大速度 (μm/s)
        double acceleration = 3000.0;      ///< 加速度 (μm/s²)
        double deceleration = 3000.0;      ///< 减速度 (μm/s²)
        int subdivision = Constants::DEFAULT_SUBDIVISION;  ///< 细分数
        double stepSize = 1.0;          ///< 基础步长 (μm)
        double softLimitPos = Constants::MAX_POSITION;     ///< 正向软限位
        double softLimitNeg = Constants::MIN_POSITION;     ///< 负向软限位
        
        bool isValid() const {
            return maxSpeed >= Constants::MIN_SPEED && maxSpeed <= Constants::MAX_SPEED &&
                   acceleration >= Constants::MIN_ACCELERATION && acceleration <= Constants::MAX_ACCELERATION &&
                   subdivision >= Constants::MIN_SUBDIVISION && subdivision <= Constants::MAX_SUBDIVISION &&
                   softLimitPos > softLimitNeg;
        }
    };

    /**
     * @brief 轴状态结构体
     */
    struct AxisState {
        double currentPosition = 0.0;    ///< 当前命令位置 (μm)
        double actualPosition = 0.0;     ///< 当前实际位置/光栅尺读数 (μm)
        double targetPosition = 0.0;     ///< 目标位置 (μm)  
        MotionState motionState = MotionState::Idle;  ///< 运动状态
        bool isHomed = false;            ///< 是否已回零
        LimitState limitState = LimitState::None;     ///< 限位状态
        bool isEnabled = false;          ///< 轴是否使能
        AxisError lastError = AxisError::NoError;     ///< 最后错误
        qint64 lastUpdateTime = 0;       ///< 最后更新时间
        qint64 lastRunningTimeMs = 0;    ///< 最近一次检测到“运行中”的时间戳(ms)
    };

    /**
     * @brief 设备信息结构体
     */
    struct DeviceInfo {
        QString portName;                ///< 端口名称
        int baudRate = Constants::DEFAULT_BAUDRATE;  ///< 波特率
        ConnectionType connectionType = ConnectionType::Serial;  ///< 连接类型
        QString firmwareVersion;         ///< 固件版本
        QString deviceModel;             ///< 设备型号
        bool isConnected = false;        ///< 连接状态
    };

public:
    explicit AxisController(QObject *parent = nullptr);
    virtual ~AxisController();

    // ==================== 连接管理 ====================
    
    /**
     * @brief 连接设备
     * @param portName 端口名称 (如 "COM3", "/dev/ttyUSB0")
     * @param baudRate 波特率 (默认9600)
     * @param connectionType 连接类型 (默认串口)
     * @return 成功返回true，失败返回false
     */
    bool connectDevice(const QString& portName, 
                      int baudRate = Constants::DEFAULT_BAUDRATE,
                      ConnectionType connectionType = ConnectionType::Serial);

    /**
     * @brief 连接设备（不关心波特率，适用于串口载物台）
     * @param portName 端口名称 (如 "COM3", "/dev/ttyUSB0")
     * @param connectionType 连接类型 (默认串口)
     * @return 成功返回true，失败返回false
     */
    bool connectDevice(const QString& portName,
                       ConnectionType connectionType);

    /**
     * @brief 断开设备连接
     * @return 成功返回true，失败返回false
     */
    bool disconnectDevice();

    /**
     * @brief 检查设备连接状态
     * @return 已连接返回true，未连接返回false
     */
    bool isConnected() const;

    /**
     * @brief 获取设备信息
     * @return 设备信息结构体
     */
    DeviceInfo getDeviceInfo() const;

    /**
     * @brief 重新连接设备
     * @return 成功返回true，失败返回false
     */
    bool reconnectDevice();

    // ==================== 运动控制 ====================

    /**
     * @brief 相对运动
     * @param axis 轴编号
     * @param distance 移动距离 (μm)
     * @param speed 运动速度 (μm/s，0表示使用默认速度)
     * @return 成功返回true，失败返回false
     */
    bool moveRelative(AxisIndex axis, double distance, double speed = 0);

    /**
     * @brief 绝对运动
     * @param axis 轴编号  
     * @param position 目标位置 (μm)
     * @param speed 运动速度 (μm/s，0表示使用默认速度)
     * @return 成功返回true，失败返回false
     */
    bool moveAbsolute(AxisIndex axis, double position, double speed = 0);

    /**
     * @brief 多轴同步运动
     * @param axes 轴编号数组
     * @param distances 移动距离数组 (μm)
     * @param speed 运动速度 (μm/s，0表示使用默认速度)
     * @return 成功返回true，失败返回false
     */
    bool moveMultiAxis(const std::vector<AxisIndex>& axes, 
                      const std::vector<double>& distances,
                      double speed = 0);

    /**
     * @brief 停止指定轴运动
     * @param axis 轴编号
     * @param immediate 是否立即停止（true为急停，false为减速停止）
     * @return 成功返回true，失败返回false
     */
    bool stopAxis(AxisIndex axis, bool immediate = false);

    /**
     * @brief 停止所有轴运动
     * @param immediate 是否立即停止（true为急停，false为减速停止）
     * @return 成功返回true，失败返回false
     */
    bool stopAllAxes(bool immediate = false);

    /**
     * @brief 轴回零操作
     * @param axis 轴编号
     * @return 成功返回true，失败返回false
     */
    bool goHome(AxisIndex axis);

    /**
     * @brief 所有轴回零操作
     * @return 成功返回true，失败返回false
     */
    bool goHomeAll();

    /**
     * @brief 点动运动开始
     * @param axis 轴编号
     * @param direction 运动方向（正数为正向，负数为负向）
     * @param speed 运动速度 (μm/s)
     * @return 成功返回true，失败返回false
     */
    bool startJogging(AxisIndex axis, int direction, double speed);

    /**
     * @brief 停止点动运动
     * @param axis 轴编号
     * @return 成功返回true，失败返回false
     */
    bool stopJogging(AxisIndex axis);

    // ==================== 状态查询 ====================

    /**
     * @brief 获取当前命令位置
     * @param axis 轴编号
     * @return 当前命令位置 (μm)，错误时返回0
     */
    double getCurrentPosition(AxisIndex axis) const;

    /**
     * @brief 获取当前实际位置（光栅尺读数）
     * @param axis 轴编号
     * @return 当前实际位置 (μm)，错误时返回0
     */
    double getActualPosition(AxisIndex axis) const;

    /**
     * @brief 获取目标位置
     * @param axis 轴编号
     * @return 目标位置 (μm)，错误时返回0
     */
    double getTargetPosition(AxisIndex axis) const;

    /**
     * @brief 检查轴是否正在运动
     * @param axis 轴编号
     * @return 正在运动返回true，否则返回false
     */
    bool isAxisMoving(AxisIndex axis) const;

    /**
     * @brief 检查轴是否已回零
     * @param axis 轴编号
     * @return 已回零返回true，否则返回false
     */
    bool isAxisHomed(AxisIndex axis) const;

    /**
     * @brief 检查轴是否已使能
     * @param axis 轴编号
     * @return 已使能返回true，否则返回false
     */
    bool isAxisEnabled(AxisIndex axis) const;

    /**
     * @brief 获取轴状态
     * @param axis 轴编号
     * @return 轴状态结构体
     */
    AxisState getAxisState(AxisIndex axis) const;

    /**
     * @brief 获取所有轴状态
     * @return 所有轴状态数组
     */
    std::array<AxisState, Constants::MAX_AXIS_COUNT> getAllAxisStates() const;

    /**
     * @brief 获取限位开关状态
     * @param axis 轴编号
     * @return 限位状态
     */
    LimitState getLimitState(AxisIndex axis) const;

    /**
     * @brief 检查是否有任何轴在运动
     * @return 有轴运动返回true，否则返回false
     */
    bool isAnyAxisMoving() const;

    // ==================== 参数配置 ====================

    /**
     * @brief 设置轴运动参数
     * @param axis 轴编号
     * @param params 运动参数
     * @return 成功返回true，失败返回false
     */
    bool setAxisParams(AxisIndex axis, const MotionParams& params);

    /**
     * @brief 获取轴运动参数
     * @param axis 轴编号
     * @return 运动参数结构体
     */
    MotionParams getAxisParams(AxisIndex axis) const;

    /**
     * @brief 设置轴速度
     * @param axis 轴编号
     * @param speed 速度 (μm/s)
     * @return 成功返回true，失败返回false
     */
    bool setAxisSpeed(AxisIndex axis, double speed);

    /**
     * @brief 设置轴加速度
     * @param axis 轴编号
     * @param acceleration 加速度 (μm/s²)
     * @return 成功返回true，失败返回false
     */
    bool setAxisAcceleration(AxisIndex axis, double acceleration);

    /**
     * @brief 设置轴使能状态
     * @param axis 轴编号
     * @param enabled 使能状态
     * @return 成功返回true，失败返回false
     */
    bool setAxisEnabled(AxisIndex axis, bool enabled);

    /**
     * @brief 设置手柄/摇杆功能使能（针对指定轴）
     * @param axis 轴编号
     * @param enabled 使能状态（true 使能；false 关闭）
     * @return 成功返回true，失败返回false
     */
    bool setJoystickEnabled(AxisIndex axis, bool enabled);

    /**
     * @brief 设置位置为零点
     * @param axis 轴编号
     * @return 成功返回true，失败返回false
     */
    bool setPositionZero(AxisIndex axis);

    // ==================== 错误处理 ====================

    /**
     * @brief 获取最后错误
     * @return 错误类型
     */
    AxisError getLastError() const;

    /**
     * @brief 获取最后错误描述
     * @return 错误描述字符串
     */
    QString getLastErrorString() const;

    /**
     * @brief 清除错误状态
     */
    void clearError();

    /**
     * @brief 获取轴的最后错误
     * @param axis 轴编号
     * @return 错误类型
     */
    AxisError getAxisLastError(AxisIndex axis) const;

    // ==================== 系统控制 ====================

    /**
     * @brief 启用状态监控
     * @param enabled 是否启用
     * @param interval 更新间隔 (ms)
     */
    void setStatusMonitorEnabled(bool enabled, int interval = Constants::STATUS_UPDATE_INTERVAL);

    /**
     * @brief 初始化所有轴
     * @return 成功返回true，失败返回false
     */
    bool initializeAllAxes();

    /**
     * @brief 急停所有运动
     */
    void emergencyStop();

    /**
     * @brief 重置控制器
     * @return 成功返回true，失败返回false
     */
    bool resetController();

    /**
     * @brief 软件版本信息
     * @return 版本字符串
     */
    static QString getVersion();

signals:
    // ==================== 连接状态信号 ====================
    
    /**
     * @brief 设备连接成功信号
     * @param deviceInfo 设备信息
     */
    void deviceConnected(const DeviceInfo& deviceInfo);

    /**
     * @brief 设备断开连接信号
     */
    void deviceDisconnected();

    /**
     * @brief 连接状态改变信号
     * @param connected 连接状态
     */
    void connectionStateChanged(bool connected);

    // ==================== 运动状态信号 ====================

    /**
     * @brief 命令位置改变信号
     * @param axis 轴编号
     * @param position 当前命令位置 (μm)
     */
    void positionChanged(AxisIndex axis, double position);

    /**
     * @brief 实际位置改变信号（光栅尺读数）
     * @param axis 轴编号
     * @param actualPosition 当前实际位置 (μm)
     */
    void actualPositionChanged(AxisIndex axis, double actualPosition);

    /**
     * @brief 运动状态改变信号
     * @param axis 轴编号
     * @param state 运动状态
     */
    void motionStateChanged(AxisIndex axis, MotionState state);

    /**
     * @brief 运动完成信号
     * @param axis 轴编号
     * @param finalPosition 最终位置 (μm)
     */
    void motionCompleted(AxisIndex axis, double finalPosition);

    /**
     * @brief 多轴运动完成信号
     * @param axes 轴编号列表
     */
    void multiAxisMotionCompleted(const std::vector<AxisIndex>& axes);

    /**
     * @brief 回零完成信号
     * @param axis 轴编号
     * @param success 是否成功
     */
    void homeCompleted(AxisIndex axis, bool success);

    // ==================== 限位和错误信号 ====================

    /**
     * @brief 限位触发信号
     * @param axis 轴编号
     * @param limitState 限位状态
     */
    void limitTriggered(AxisIndex axis, LimitState limitState);

    /**
     * @brief 错误发生信号
     * @param axis 轴编号
     * @param error 错误类型
     * @param errorString 错误描述
     */
    void errorOccurred(AxisIndex axis, AxisError error, const QString& errorString);

    /**
     * @brief 急停触发信号
     */
    void emergencyStopTriggered();

    /**
     * @brief 清除急停状态（复位）信号
     * 说明：当控制器完成复位操作后发出，用于通知UI层恢复控件可用
     */
    void emergencyResetCleared();

    /**
     * @brief 系统状态更新信号
     */
    void systemStatusUpdated();

private slots:
    /**
     * @brief 更新轴状态（定时器回调）
     */
    void updateAxisStatus();

    /**
     * @brief 处理运动完成
     * @param axis 轴编号
     */
    void handleMotionCompleted(int axis);

    /**
     * @brief 在工作线程启动或停止状态定时器
     * @param start 是否启动
     * @param interval 间隔(ms)
     */
    void controlStatusTimer(bool start, int interval);

private:
    // ==================== 私有成员变量 ====================
    
    // 硬件控制
    MoCtrCard* m_controller;                    ///< MCC6控制卡实例
    mutable QMutex m_mutex;                     ///< 线程安全互斥锁
    std::atomic<bool> m_isInitialized;          ///< 初始化状态
    
    // 连接状态
    DeviceInfo m_deviceInfo;                    ///< 设备信息
    std::atomic<bool> m_connectionActive;       ///< 连接活跃状态
    
    // 轴状态和参数
    std::array<AxisState, Constants::MAX_AXIS_COUNT> m_axisStates;     ///< 轴状态数组
    std::array<MotionParams, Constants::MAX_AXIS_COUNT> m_motionParams; ///< 运动参数数组
    
    // 状态监控
    QTimer* m_statusTimer;                      ///< 状态更新定时器（运行于工作线程）
    std::atomic<bool> m_statusMonitorEnabled;   ///< 状态监控使能
    int m_statusUpdateInterval;                 ///< 状态更新间隔
    QThread* m_statusThread;                    ///< 状态监控线程
    qint64 m_lastBusyTime = 0;                  ///< 最近一次检测到“忙”(Moving/Homing)的时间戳(ms)
    
    // 错误管理
    AxisError m_lastError;                      ///< 最后错误
    QString m_lastErrorString;                  ///< 最后错误描述
    mutable QMutex m_errorMutex;                ///< 错误信息互斥锁
    
    // 运动控制
    std::atomic<bool> m_emergencyStopActive;    ///< 急停状态
    QTimer* m_motionTimeoutTimer;               ///< 运动超时定时器

    // ==================== 私有成员函数 ====================
    
    /**
     * @brief 初始化控制器
     * @return 成功返回true，失败返回false
     */
    bool initialize();

    /**
     * @brief 清理资源
     */
    void cleanup();

    /**
     * @brief 验证轴编号
     * @param axis 轴编号
     * @return 有效返回true，无效返回false
     */
    bool isValidAxis(AxisIndex axis) const;

    /**
     * @brief 验证运动参数
     * @param axis 轴编号
     * @param distance 距离
     * @param speed 速度
     * @return 有效返回true，无效返回false
     */
    bool validateMotionParams(AxisIndex axis, double distance, double speed) const;

    /**
     * @brief 处理MCC6DLL错误
     * @param errorCode MCC6错误码
     * @param operation 操作描述
     * @return 处理结果
     */
    bool handleMCC6Error(int errorCode, const QString& operation);

    /**
     * @brief 设置错误状态
     * @param error 错误类型
     * @param errorString 错误描述
     * @param axis 轴编号（可选）
     */
    void setError(AxisError error, const QString& errorString, AxisIndex axis = AxisIndex::INVALID_AXIS) const;

    /**
     * @brief 更新轴状态
     * @param axis 轴编号
     * @param forceUpdate 强制更新
     */
    void updateSingleAxisStatus(AxisIndex axis, bool forceUpdate = false);

    /**
     * @brief 发射命令位置改变信号
     * @param axis 轴编号
     * @param newPosition 新命令位置
     */
    void emitPositionChanged(AxisIndex axis, double newPosition);

    /**
     * @brief 发射实际位置改变信号（光栅尺读数）
     * @param axis 轴编号
     * @param newActualPosition 新实际位置
     */
    void emitActualPositionChanged(AxisIndex axis, double newActualPosition);

    /**
     * @brief 发射运动状态改变信号
     * @param axis 轴编号
     * @param newState 新状态
     */
    void emitMotionStateChanged(AxisIndex axis, MotionState newState);

    /**
     * @brief 检查软限位
     * @param axis 轴编号
     * @param targetPosition 目标位置
     * @return 在限位内返回true，超出返回false
     */
    bool checkSoftLimits(AxisIndex axis, double targetPosition) const;
    
    /**
     * @brief 转换轴编号为MCC6轴编号
     * @param axis 轴编号
     * @return MCC6轴编号
     */
    int axisToMCC6Index(AxisIndex axis) const;
    QString axisToString(AxisIndex axis) const;

    /**
     * @brief 自适应计算当前应使用的监控间隔
     */
    int computeAdaptiveInterval() const;

    /**
     * @brief 在工作线程执行一次状态采集
     */
    void performStatusPoll();
};

#endif // AXISCONTROLLER_H
