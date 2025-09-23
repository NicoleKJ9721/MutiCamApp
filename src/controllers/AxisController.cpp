#include "AxisController.h"
#include "MCC6DLL.h"
#include <QElapsedTimer>
#include <QCoreApplication>
#include <QThread>
#include <QDateTime>
#include <cmath>

/**
 * @file AxisController.cpp
 * @brief AxisController类的完整实现
 * @version 1.0
 * @date 2024
 * 
 * 提供完整全面的轴控制功能实现，包括所有MCC6DLL集成、
 * 错误处理、状态监控和线程安全控制
 */

// ==================== 构造和析构 ====================

AxisController::AxisController(QObject *parent)
    : QObject(parent)
    , m_controller(nullptr)
    , m_isInitialized(false)
    , m_connectionActive(false)
    , m_statusTimer(new QTimer(this))
    , m_statusMonitorEnabled(false)
    , m_statusUpdateInterval(Constants::STATUS_UPDATE_INTERVAL)
    , m_lastError(AxisError::NoError)
    , m_emergencyStopActive(false)
    , m_motionTimeoutTimer(new QTimer(this))
{
    // 初始化设备信息
    m_deviceInfo.isConnected = false;
    m_deviceInfo.connectionType = ConnectionType::Serial;
    m_deviceInfo.baudRate = Constants::DEFAULT_BAUDRATE;
    
    // 初始化轴状态
    for (int i = 0; i < Constants::MAX_AXIS_COUNT; ++i) {
        m_axisStates[i] = AxisState{};
        m_motionParams[i] = MotionParams{};
    }
    
    // 设置状态更新定时器
    m_statusTimer->setSingleShot(false);
    connect(m_statusTimer, &QTimer::timeout, this, &AxisController::updateAxisStatus);
    
    // 设置运动超时定时器
    m_motionTimeoutTimer->setSingleShot(true);
    connect(m_motionTimeoutTimer, &QTimer::timeout, [this]() {
        setError(AxisError::MotionTimeout, "运动超时，已自动停止所有轴");
        stopAllAxes(true); // 急停
    });
    
    qDebug() << "AxisController initialized";
}

AxisController::~AxisController()
{
    cleanup();
    qDebug() << "AxisController destroyed";
}

// ==================== 连接管理实现 ====================

bool AxisController::connectDevice(const QString& portName, int baudRate, ConnectionType connectionType)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_connectionActive) {
        setError(AxisError::ConnectionFailed, "设备已连接，请先断开");
        return false;
    }
    
    // 验证参数
    if (portName.isEmpty()) {
        setError(AxisError::InvalidParameter, "端口名称不能为空");
        return false;
    }
    
    // 注：串口模式下，MCC6 串口初始化不使用波特率；仅以太网模式使用第二参数作为端口。
    
    try {
        // 创建MCC6控制卡实例
        if (m_controller) {
            delete m_controller;
            m_controller = nullptr;
        }
        
        m_controller = new MoCtrCard();
        if (!m_controller) {
            setError(AxisError::InitializationFailed, "创建MCC6控制卡实例失败");
            return false;
        }
        
        // 初始化串口连接
        int result = 0;
        if (connectionType == ConnectionType::Serial) {
            // 从端口名解析串口号（例如 "COM3" -> 3）
            QString portNumStr = portName.mid(3); // 去掉"COM"前缀
            bool ok;
            int comPort = portNumStr.toInt(&ok);
            if (!ok || comPort < 1 || comPort > 255) {
                setError(AxisError::InvalidParameter, QString("无效的串口号：%1").arg(portName));
                return false;
            }
            result = m_controller->MoCtrCard_Initial(static_cast<uint8_t>(comPort));
        } else if (connectionType == ConnectionType::Ethernet) {
            // 网络连接（需要IP地址和端口）
            result = m_controller->MoCtrCard_Net_Initial(const_cast<char*>(portName.toLocal8Bit().data()), baudRate);
        }
        
        if (!handleMCC6Error(result, "连接设备")) {
            return false;
        }
        
        // 初始化所有轴
        if (!initializeAllAxes()) {
            disconnectDevice();
            return false;
        }
        
        // 更新设备信息
        m_deviceInfo.portName = portName;
        m_deviceInfo.baudRate = baudRate;
        m_deviceInfo.connectionType = connectionType;
        m_deviceInfo.isConnected = true;
        m_connectionActive = true;
        m_isInitialized = true;
        
        // 设置设备信息
        m_deviceInfo.firmwareVersion = "MCC6DLL v1.0";
        m_deviceInfo.deviceModel = "MCC6 Motion Controller";
        
        // 状态监控启动应在主线程进行（避免跨线程启动QTimer）
        // 由UI线程在连接完成回调中调用 setStatusMonitorEnabled(true)

        // 清除错误状态
        clearError();
        
        emit deviceConnected(m_deviceInfo);
        emit connectionStateChanged(true);
        
        if (connectionType == ConnectionType::Ethernet) {
            qDebug() << QString("成功连接到设备 %1，端口：%2").arg(portName).arg(baudRate);
        } else {
            qDebug() << QString("成功连接到设备 %1（串口）").arg(portName);
        }
        return true;
        
    } catch (const std::exception& e) {
        setError(AxisError::HardwareError, QString("连接异常：%1").arg(e.what()));
        cleanup();
        return false;
    } catch (...) {
        setError(AxisError::UnknownError, "连接时发生未知异常");
        cleanup();
        return false;
    }
}

bool AxisController::connectDevice(const QString& portName, ConnectionType connectionType)
{
    // 仅端口重载：对串口不校验波特率，Ethernet 沿用默认端口 Constants::DEFAULT_BAUDRATE 仅作占位
    return connectDevice(portName,
                         Constants::DEFAULT_BAUDRATE,
                         connectionType);
}

bool AxisController::disconnectDevice()
{
    try {
        // 先在持锁状态下仅检查连接状态，避免后续再入死锁
        {
            QMutexLocker locker(&m_mutex);
            if (!m_connectionActive) {
                return true; // 已经断开
            }
        }

        // 在未持有互斥锁时执行可能阻塞/再次加锁的操作
        // 1) 停止所有运动（内部会自行加锁）
        stopAllAxes(true);

        // 2) 停止状态监控（需在UI线程调用）
        setStatusMonitorEnabled(false);

        // 3) 卸载控制器（避免在持锁状态下调用硬件API）
        MoCtrCard* controllerToUnload = nullptr;
        {
            QMutexLocker locker(&m_mutex);
            controllerToUnload = m_controller;
        }
        if (controllerToUnload) {
            int result = controllerToUnload->MoCtrCard_Unload();
            handleMCC6Error(result, "断开设备连接");
            delete controllerToUnload;
            controllerToUnload = nullptr;
        }

        // 4) 持锁更新本地状态，但不在锁内发射任何信号
        {
            QMutexLocker locker(&m_mutex);
            m_controller = nullptr;
            m_deviceInfo.isConnected = false;
            m_connectionActive = false;
            m_isInitialized = false;
            for (int i = 0; i < Constants::MAX_AXIS_COUNT; ++i) {
                m_axisStates[i] = AxisState{};
            }
        }

        // 5) 解锁后发射信号
        emit deviceDisconnected();
        emit connectionStateChanged(false);

        qDebug() << "设备已断开连接";
        return true;

    } catch (const std::exception& e) {
        setError(AxisError::HardwareError, QString("断开连接异常：%1").arg(e.what()));
        return false;
    } catch (...) {
        setError(AxisError::UnknownError, "断开连接时发生未知异常");
        return false;
    }
}

bool AxisController::isConnected() const
{
    return m_connectionActive && m_deviceInfo.isConnected;
}

AxisController::DeviceInfo AxisController::getDeviceInfo() const
{
    QMutexLocker locker(&m_mutex);
    return m_deviceInfo;
}

bool AxisController::reconnectDevice()
{
    if (!disconnectDevice()) {
        return false;
    }
    
    QThread::msleep(1000); // 等待1秒
    
    return connectDevice(m_deviceInfo.portName, m_deviceInfo.baudRate, m_deviceInfo.connectionType);
}

// ==================== 运动控制实现 ====================

bool AxisController::moveRelative(AxisIndex axis, double distance, double speed)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        setError(AxisError::DeviceNotConnected, "设备未连接", axis);
        return false;
    }
    
    if (!validateMotionParams(axis, distance, speed)) {
        return false;
    }
    
    if (m_emergencyStopActive) {
        setError(AxisError::EmergencyStop, "急停状态下无法运动", axis);
        return false;
    }
    
    int axisIndex = axisToMCC6Index(axis);
    double moveSpeed = (speed > 0) ? speed : m_motionParams[axisIndex].maxSpeed;
    
    try {
        // 检查轴状态
        if (m_axisStates[axisIndex].motionState == MotionState::Moving) {
            setError(AxisError::AxisBusy, "轴正在运动中", axis);
            return false;
        }
        
        // 计算目标位置
        double currentPos = m_axisStates[axisIndex].currentPosition;
        double targetPos = currentPos + distance;
        
        // 检查软限位
        if (!checkSoftLimits(axis, targetPos)) {
            setError(AxisError::LimitReached, 
                    QString("目标位置 %1 μm 超出软限位范围").arg(targetPos), axis);
            return false;
        }
        
        // 执行相对运动（MCC6DLL的函数同时设置速度和加速度）
        int result = m_controller->MoCtrCard_MCrlAxisRelMove(
            static_cast<uint8_t>(axisIndex), 
            static_cast<float>(distance),
            static_cast<float>(moveSpeed),
            static_cast<float>(m_motionParams[axisIndex].acceleration)
        );
        if (!handleMCC6Error(result, QString("%1相对运动").arg(axisToString(axis)))) {
            return false;
        }
        
        // 更新状态
        m_axisStates[axisIndex].targetPosition = targetPos;
        m_axisStates[axisIndex].motionState = MotionState::Moving;
        m_axisStates[axisIndex].lastError = AxisError::NoError;
        
        // 启动运动超时检查
        m_motionTimeoutTimer->start(Constants::MOTION_TIMEOUT);
        
        emitMotionStateChanged(axis, MotionState::Moving);
        
        qDebug() << QString("%1相对运动 %2 μm，速度：%3 μm/min")
                    .arg(axisToString(axis)).arg(distance).arg(moveSpeed);
        
        return true;
        
    } catch (const std::exception& e) {
        setError(AxisError::HardwareError, QString("相对运动异常：%1").arg(e.what()), axis);
        return false;
    } catch (...) {
        setError(AxisError::UnknownError, "相对运动时发生未知异常", axis);
        return false;
    }
}

bool AxisController::moveAbsolute(AxisIndex axis, double position, double speed)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        setError(AxisError::DeviceNotConnected, "设备未连接", axis);
        return false;
    }
    
    if (!isValidAxis(axis)) {
        setError(AxisError::InvalidAxis, "无效的轴编号", axis);
        return false;
    }
    
    if (m_emergencyStopActive) {
        setError(AxisError::EmergencyStop, "急停状态下无法运动", axis);
        return false;
    }
    
    int axisIndex = axisToMCC6Index(axis);
    double moveSpeed = (speed > 0) ? speed : m_motionParams[axisIndex].maxSpeed;
    
    try {
        // 检查轴状态
        if (m_axisStates[axisIndex].motionState == MotionState::Moving) {
            setError(AxisError::AxisBusy, "轴正在运动中", axis);
            return false;
        }
        
        // 检查软限位
        if (!checkSoftLimits(axis, position)) {
            setError(AxisError::LimitReached, 
                    QString("目标位置 %1 μm 超出软限位范围").arg(position), axis);
            return false;
        }
        
        // 执行绝对运动（MCC6DLL的函数同时设置速度和加速度）
        int result = m_controller->MoCtrCard_MCrlAxisAbsMove(
            static_cast<uint8_t>(axisIndex), 
            static_cast<float>(position),
            static_cast<float>(moveSpeed),
            static_cast<float>(m_motionParams[axisIndex].acceleration)
        );
        if (!handleMCC6Error(result, QString("%1绝对运动").arg(axisToString(axis)))) {
            return false;
        }
        
        // 更新状态
        m_axisStates[axisIndex].targetPosition = position;
        m_axisStates[axisIndex].motionState = MotionState::Moving;
        m_axisStates[axisIndex].lastError = AxisError::NoError;
        
        // 启动运动超时检查
        m_motionTimeoutTimer->start(Constants::MOTION_TIMEOUT);
        
        emitMotionStateChanged(axis, MotionState::Moving);
        
        qDebug() << QString("%1绝对运动到 %2 μm，速度：%3 μm/min")
                    .arg(axisToString(axis)).arg(position).arg(moveSpeed);
        
        return true;
        
    } catch (const std::exception& e) {
        setError(AxisError::HardwareError, QString("绝对运动异常：%1").arg(e.what()), axis);
        return false;
    } catch (...) {
        setError(AxisError::UnknownError, "绝对运动时发生未知异常", axis);
        return false;
    }
}

bool AxisController::moveMultiAxis(const std::vector<AxisIndex>& axes, 
                                  const std::vector<double>& distances,
                                  double speed)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        setError(AxisError::DeviceNotConnected, "设备未连接");
        return false;
    }
    
    if (axes.size() != distances.size() || axes.empty()) {
        setError(AxisError::InvalidParameter, "轴数量与距离数量不匹配");
        return false;
    }
    
    if (m_emergencyStopActive) {
        setError(AxisError::EmergencyStop, "急停状态下无法运动");
        return false;
    }
    
    try {
        // 验证所有轴
        for (size_t i = 0; i < axes.size(); ++i) {
            if (!isValidAxis(axes[i])) {
                setError(AxisError::InvalidAxis, QString("无效的轴编号：%1").arg(static_cast<int>(axes[i])));
                return false;
            }
            
            int axisIndex = axisToMCC6Index(axes[i]);
            if (m_axisStates[axisIndex].motionState == MotionState::Moving) {
                setError(AxisError::AxisBusy, QString("%1正在运动中").arg(axisToString(axes[i])));
                return false;
            }
            
            // 检查软限位
            double currentPos = m_axisStates[axisIndex].currentPosition;
            double targetPos = currentPos + distances[i];
            if (!checkSoftLimits(axes[i], targetPos)) {
                setError(AxisError::LimitReached, 
                        QString("%1目标位置 %2 μm 超出软限位范围")
                        .arg(axisToString(axes[i])).arg(targetPos));
                return false;
            }
        }
        
        // 执行多轴运动（MCC6DLL没有直接的多轴同步函数，依次启动各轴）
        for (size_t i = 0; i < axes.size(); ++i) {
            int axisIndex = axisToMCC6Index(axes[i]);
            double moveSpeed = (speed > 0) ? speed : m_motionParams[axisIndex].maxSpeed;
            
            int result = m_controller->MoCtrCard_MCrlAxisRelMove(
                static_cast<uint8_t>(axisIndex), 
                static_cast<float>(distances[i]),
                static_cast<float>(moveSpeed),
                static_cast<float>(m_motionParams[axisIndex].acceleration)
            );
            
            if (!handleMCC6Error(result, QString("%1多轴运动").arg(axisToString(axes[i])))) {
                // 如果有轴失败，停止所有已启动的轴
                for (size_t j = 0; j < i; ++j) {
                    m_controller->MoCtrCard_EmergencyStopAxisMov(static_cast<uint8_t>(axisToMCC6Index(axes[j])));
                }
                return false;
            }
        }
        
        // 更新所有轴状态
        for (size_t i = 0; i < axes.size(); ++i) {
            int axisIndex = axisToMCC6Index(axes[i]);
            double currentPos = m_axisStates[axisIndex].currentPosition;
            m_axisStates[axisIndex].targetPosition = currentPos + distances[i];
            m_axisStates[axisIndex].motionState = MotionState::Moving;
            m_axisStates[axisIndex].lastError = AxisError::NoError;
            
            emitMotionStateChanged(axes[i], MotionState::Moving);
        }
        
        // 启动运动超时检查
        m_motionTimeoutTimer->start(Constants::MOTION_TIMEOUT);
        
        qDebug() << QString("多轴同步运动开始，轴数量：%1").arg(axes.size());
        
        return true;
        
    } catch (const std::exception& e) {
        setError(AxisError::HardwareError, QString("多轴运动异常：%1").arg(e.what()));
        return false;
    } catch (...) {
        setError(AxisError::UnknownError, "多轴运动时发生未知异常");
        return false;
    }
}

bool AxisController::stopAxis(AxisIndex axis, bool immediate)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        setError(AxisError::DeviceNotConnected, "设备未连接", axis);
        return false;
    }
    
    if (!isValidAxis(axis)) {
        setError(AxisError::InvalidAxis, "无效的轴编号", axis);
        return false;
    }
    
    int axisIndex = axisToMCC6Index(axis);
    
    try {
        int result;
        if (immediate) {
            result = m_controller->MoCtrCard_EmergencyStopAxisMov(static_cast<uint8_t>(axisIndex));
            if (!handleMCC6Error(result, QString("%1急停").arg(axisToString(axis)))) {
                return false;
            }
        } else {
            // 使用默认减速度停止轴
            result = m_controller->MoCtrCard_StopAxisMov(
                static_cast<uint8_t>(axisIndex), 
                static_cast<float>(m_motionParams[axisIndex].deceleration)
            );
            if (!handleMCC6Error(result, QString("%1停止").arg(axisToString(axis)))) {
                return false;
            }
        }
        
        // 更新状态
        m_axisStates[axisIndex].motionState = MotionState::Stopped;
        m_axisStates[axisIndex].lastError = AxisError::NoError;
        
        emitMotionStateChanged(axis, MotionState::Stopped);
        
        qDebug() << QString("%1已%2").arg(axisToString(axis)).arg(immediate ? "急停" : "停止");
        
        return true;
        
    } catch (const std::exception& e) {
        setError(AxisError::HardwareError, QString("停止轴异常：%1").arg(e.what()), axis);
        return false;
    } catch (...) {
        setError(AxisError::UnknownError, "停止轴时发生未知异常", axis);
        return false;
    }
}

bool AxisController::stopAllAxes(bool immediate)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        setError(AxisError::DeviceNotConnected, "设备未连接");
        return false;
    }
    
    try {
        // MCC6DLL没有停止所有轴的函数，需要依次停止每个轴
        for (int i = 0; i < Constants::MAX_AXIS_COUNT; ++i) {
            if (m_axisStates[i].motionState == MotionState::Moving) {
                int result;
                if (immediate) {
                    result = m_controller->MoCtrCard_EmergencyStopAxisMov(static_cast<uint8_t>(i));
                } else {
                    result = m_controller->MoCtrCard_StopAxisMov(
                        static_cast<uint8_t>(i), 
                        static_cast<float>(m_motionParams[i].deceleration)
                    );
                }
                // 记录错误但继续停止其他轴
                handleMCC6Error(result, QString("停止轴%1").arg(i));
            }
        }
        
        // 更新所有轴状态
        for (int i = 0; i < Constants::MAX_AXIS_COUNT; ++i) {
            if (m_axisStates[i].motionState == MotionState::Moving) {
                m_axisStates[i].motionState = MotionState::Stopped;
                emitMotionStateChanged(static_cast<AxisIndex>(i), MotionState::Stopped);
            }
        }
        
        // 停止运动超时定时器
        m_motionTimeoutTimer->stop();
        
        qDebug() << QString("所有轴已%1").arg(immediate ? "急停" : "停止");
        
        return true;
        
    } catch (const std::exception& e) {
        setError(AxisError::HardwareError, QString("停止所有轴异常：%1").arg(e.what()));
        return false;
    } catch (...) {
        setError(AxisError::UnknownError, "停止所有轴时发生未知异常");
        return false;
    }
}

bool AxisController::goHome(AxisIndex axis)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        setError(AxisError::DeviceNotConnected, "设备未连接", axis);
        return false;
    }
    
    if (!isValidAxis(axis)) {
        setError(AxisError::InvalidAxis, "无效的轴编号", axis);
        return false;
    }
    
    if (m_emergencyStopActive) {
        setError(AxisError::EmergencyStop, "急停状态下无法回零", axis);
        return false;
    }
    
    int axisIndex = axisToMCC6Index(axis);
    
    try {
        // 检查轴状态
        if (m_axisStates[axisIndex].motionState == MotionState::Moving) {
            setError(AxisError::AxisBusy, "轴正在运动中", axis);
            return false;
        }
        
        // 执行回零（使用寻零功能，正向寻零）
        int result = m_controller->MoCtrCard_SeekZero(
            static_cast<uint8_t>(axisIndex), 
            static_cast<float>(m_motionParams[axisIndex].maxSpeed * 0.3), // 使用30%的最大速度回零
            static_cast<float>(m_motionParams[axisIndex].acceleration)
        );
        if (!handleMCC6Error(result, QString("%1回零").arg(axisToString(axis)))) {
            return false;
        }
        
        // 更新状态
        m_axisStates[axisIndex].motionState = MotionState::Homing;
        m_axisStates[axisIndex].isHomed = false;
        m_axisStates[axisIndex].lastError = AxisError::NoError;
        
        emitMotionStateChanged(axis, MotionState::Homing);
        
        qDebug() << QString("%1开始回零").arg(axisToString(axis));
        
        return true;
        
    } catch (const std::exception& e) {
        setError(AxisError::HardwareError, QString("回零异常：%1").arg(e.what()), axis);
        return false;
    } catch (...) {
        setError(AxisError::UnknownError, "回零时发生未知异常", axis);
        return false;
    }
}

bool AxisController::goHomeAll()
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        setError(AxisError::DeviceNotConnected, "设备未连接");
        return false;
    }
    
    if (m_emergencyStopActive) {
        setError(AxisError::EmergencyStop, "急停状态下无法回零");
        return false;
    }
    
    try {
        // 检查所有轴状态
        for (int i = 0; i < Constants::MAX_AXIS_COUNT; ++i) {
            if (m_axisStates[i].motionState == MotionState::Moving) {
                setError(AxisError::AxisBusy, QString("%1正在运动中")
                        .arg(axisToString(static_cast<AxisIndex>(i))));
                return false;
            }
        }
        
        // 执行所有轴回零（依次启动每个轴的回零）
        for (int i = 0; i < Constants::MAX_AXIS_COUNT; ++i) {
            int result = m_controller->MoCtrCard_SeekZero(
                static_cast<uint8_t>(i), 
                static_cast<float>(m_motionParams[i].maxSpeed * 0.3), // 使用30%的最大速度回零
                static_cast<float>(m_motionParams[i].acceleration)
            );
            if (!handleMCC6Error(result, QString("轴%1回零").arg(i))) {
                // 如果有轴失败，取消所有已启动的回零
                for (int j = 0; j < i; ++j) {
                    m_controller->MoCtrCard_CancelSeekZero(static_cast<uint8_t>(j));
                }
                return false;
            }
        }
        
        // 更新所有轴状态
        for (int i = 0; i < Constants::MAX_AXIS_COUNT; ++i) {
            m_axisStates[i].motionState = MotionState::Homing;
            m_axisStates[i].isHomed = false;
            m_axisStates[i].lastError = AxisError::NoError;
            
            emitMotionStateChanged(static_cast<AxisIndex>(i), MotionState::Homing);
        }
        
        qDebug() << "所有轴开始回零";
        
        return true;
        
    } catch (const std::exception& e) {
        setError(AxisError::HardwareError, QString("所有轴回零异常：%1").arg(e.what()));
        return false;
    } catch (...) {
        setError(AxisError::UnknownError, "所有轴回零时发生未知异常");
        return false;
    }
}

bool AxisController::startJogging(AxisIndex axis, int direction, double speed)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        setError(AxisError::DeviceNotConnected, "设备未连接", axis);
        return false;
    }
    
    if (!isValidAxis(axis)) {
        setError(AxisError::InvalidAxis, "无效的轴编号", axis);
        return false;
    }
    
    if (direction == 0) {
        setError(AxisError::InvalidParameter, "方向参数无效", axis);
        return false;
    }
    
    if (speed <= 0) {
        setError(AxisError::InvalidParameter, "速度参数无效", axis);
        return false;
    }
    
    if (m_emergencyStopActive) {
        setError(AxisError::EmergencyStop, "急停状态下无法点动", axis);
        return false;
    }
    
    int axisIndex = axisToMCC6Index(axis);
    
    try {
        // 检查轴状态
        if (m_axisStates[axisIndex].motionState == MotionState::Moving) {
            setError(AxisError::AxisBusy, "轴正在运动中", axis);
            return false;
        }
        
        // 开始点动（使用手动距离运动）
        int jogDirection = (direction > 0) ? 1 : -1;
        int result = m_controller->MoCtrCard_MCrlAxisMove(
            static_cast<uint8_t>(axisIndex), 
            static_cast<int8_t>(jogDirection)
        );
        if (!handleMCC6Error(result, QString("%1开始点动").arg(axisToString(axis)))) {
            return false;
        }
        
        // 更新状态
        m_axisStates[axisIndex].motionState = MotionState::Moving;
        m_axisStates[axisIndex].lastError = AxisError::NoError;
        
        emitMotionStateChanged(axis, MotionState::Moving);
        
        qDebug() << QString("%1开始%2点动，速度：%3 μm/min")
                    .arg(axisToString(axis))
                    .arg(direction > 0 ? "正向" : "负向")
                    .arg(speed);
        
        return true;
        
    } catch (const std::exception& e) {
        setError(AxisError::HardwareError, QString("点动异常：%1").arg(e.what()), axis);
        return false;
    } catch (...) {
        setError(AxisError::UnknownError, "点动时发生未知异常", axis);
        return false;
    }
}

bool AxisController::stopJogging(AxisIndex axis)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        setError(AxisError::DeviceNotConnected, "设备未连接", axis);
        return false;
    }
    
    if (!isValidAxis(axis)) {
        setError(AxisError::InvalidAxis, "无效的轴编号", axis);
        return false;
    }
    
    int axisIndex = axisToMCC6Index(axis);
    
    try {
        // 停止点动（使用急停）
        int result = m_controller->MoCtrCard_EmergencyStopAxisMov(static_cast<uint8_t>(axisIndex));
        if (!handleMCC6Error(result, QString("%1停止点动").arg(axisToString(axis)))) {
            return false;
        }
        
        // 更新状态
        m_axisStates[axisIndex].motionState = MotionState::Idle;
        
        emitMotionStateChanged(axis, MotionState::Idle);
        
        qDebug() << QString("%1停止点动").arg(axisToString(axis));
        
        return true;
        
    } catch (const std::exception& e) {
        setError(AxisError::HardwareError, QString("停止点动异常：%1").arg(e.what()), axis);
        return false;
    } catch (...) {
        setError(AxisError::UnknownError, "停止点动时发生未知异常", axis);
        return false;
    }
}

// ==================== 状态查询实现 ====================

double AxisController::getCurrentPosition(AxisIndex axis) const
{
    QMutexLocker locker(&m_mutex);
    
    if (!isValidAxis(axis)) {
        return 0.0;
    }
    
    return m_axisStates[axisToMCC6Index(axis)].currentPosition;
}

double AxisController::getActualPosition(AxisIndex axis) const
{
    QMutexLocker locker(&m_mutex);
    
    if (!isValidAxis(axis)) {
        return 0.0;
    }
    
    return m_axisStates[axisToMCC6Index(axis)].actualPosition;
}

double AxisController::getTargetPosition(AxisIndex axis) const
{
    QMutexLocker locker(&m_mutex);
    
    if (!isValidAxis(axis)) {
        return 0.0;
    }
    
    return m_axisStates[axisToMCC6Index(axis)].targetPosition;
}

bool AxisController::isAxisMoving(AxisIndex axis) const
{
    QMutexLocker locker(&m_mutex);
    
    if (!isValidAxis(axis)) {
        return false;
    }
    
    return m_axisStates[axisToMCC6Index(axis)].motionState == MotionState::Moving;
}

bool AxisController::isAxisHomed(AxisIndex axis) const
{
    QMutexLocker locker(&m_mutex);
    
    if (!isValidAxis(axis)) {
        return false;
    }
    
    return m_axisStates[axisToMCC6Index(axis)].isHomed;
}

bool AxisController::isAxisEnabled(AxisIndex axis) const
{
    QMutexLocker locker(&m_mutex);
    
    if (!isValidAxis(axis)) {
        return false;
    }
    
    return m_axisStates[axisToMCC6Index(axis)].isEnabled;
}

AxisController::AxisState AxisController::getAxisState(AxisIndex axis) const
{
    QMutexLocker locker(&m_mutex);
    
    if (!isValidAxis(axis)) {
        return AxisState{};
    }
    
    return m_axisStates[axisToMCC6Index(axis)];
}

std::array<AxisController::AxisState, Constants::MAX_AXIS_COUNT> AxisController::getAllAxisStates() const
{
    QMutexLocker locker(&m_mutex);
    return m_axisStates;
}

LimitState AxisController::getLimitState(AxisIndex axis) const
{
    QMutexLocker locker(&m_mutex);
    
    if (!isValidAxis(axis)) {
        return LimitState::None;
    }
    
    return m_axisStates[axisToMCC6Index(axis)].limitState;
}

bool AxisController::isAnyAxisMoving() const
{
    QMutexLocker locker(&m_mutex);
    
    for (int i = 0; i < Constants::MAX_AXIS_COUNT; ++i) {
        if (m_axisStates[i].motionState == MotionState::Moving) {
            return true;
        }
    }
    
    return false;
}

// ==================== 参数配置实现 ====================

bool AxisController::setAxisParams(AxisIndex axis, const MotionParams& params)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isValidAxis(axis)) {
        setError(AxisError::InvalidAxis, "无效的轴编号", axis);
        return false;
    }
    
    if (!params.isValid()) {
        setError(AxisError::InvalidParameter, "运动参数无效", axis);
        return false;
    }
    
    int axisIndex = axisToMCC6Index(axis);
    
    try {
        if (isConnected()) {
            // 设置硬件参数（使用通用参数设置接口）
            // 注意：具体的参数索引需要根据MCC6DLL手册确定
            int result;
            
            // 设置最大速度（假设参数索引0为速度）
            result = m_controller->MoCtrCard_SendPara(
                static_cast<uint8_t>(axisIndex), 
                0, // 速度参数索引
                static_cast<float>(params.maxSpeed)
            );
            handleMCC6Error(result, QString("设置%1最大速度").arg(axisToString(axis)));
            
            // 设置加速度（假设参数索引1为加速度）
            result = m_controller->MoCtrCard_SendPara(
                static_cast<uint8_t>(axisIndex), 
                1, // 加速度参数索引
                static_cast<float>(params.acceleration)
            );
            handleMCC6Error(result, QString("设置%1加速度").arg(axisToString(axis)));
            
            // 设置细分（假设参数索引2为细分）
            result = m_controller->MoCtrCard_SendPara(
                static_cast<uint8_t>(axisIndex), 
                2, // 细分参数索引
                static_cast<uint32_t>(params.subdivision)
            );
            handleMCC6Error(result, QString("设置%1细分").arg(axisToString(axis)));
        }
        
        // 更新本地参数
        m_motionParams[axisIndex] = params;
        
        qDebug() << QString("%1参数已更新：速度=%2, 加速度=%3, 细分=%4")
                    .arg(axisToString(axis))
                    .arg(params.maxSpeed)
                    .arg(params.acceleration)
                    .arg(params.subdivision);
        
        return true;
        
    } catch (const std::exception& e) {
        setError(AxisError::HardwareError, QString("设置参数异常：%1").arg(e.what()), axis);
        return false;
    } catch (...) {
        setError(AxisError::UnknownError, "设置参数时发生未知异常", axis);
        return false;
    }
}

AxisController::MotionParams AxisController::getAxisParams(AxisIndex axis) const
{
    QMutexLocker locker(&m_mutex);
    
    if (!isValidAxis(axis)) {
        return MotionParams{};
    }
    
    return m_motionParams[axisToMCC6Index(axis)];
}

bool AxisController::setAxisSpeed(AxisIndex axis, double speed)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isValidAxis(axis)) {
        setError(AxisError::InvalidAxis, "无效的轴编号", axis);
        return false;
    }
    
    if (speed < Constants::MIN_SPEED || speed > Constants::MAX_SPEED) {
        setError(AxisError::ParameterOutOfRange, 
                QString("速度 %1 超出范围 [%2-%3]")
                .arg(speed).arg(Constants::MIN_SPEED).arg(Constants::MAX_SPEED), axis);
        return false;
    }
    
    int axisIndex = axisToMCC6Index(axis);
    
    try {
        if (isConnected()) {
            int result = m_controller->MoCtrCard_SendPara(
                static_cast<uint8_t>(axisIndex), 
                0, // 速度参数索引
                static_cast<float>(speed)
            );
            if (!handleMCC6Error(result, QString("设置%1速度").arg(axisToString(axis)))) {
                return false;
            }
        }
        
        m_motionParams[axisIndex].maxSpeed = speed;
        
        qDebug() << QString("%1速度设置为 %2 μm/min").arg(axisToString(axis)).arg(speed);
        
        return true;
        
    } catch (const std::exception& e) {
        setError(AxisError::HardwareError, QString("设置速度异常：%1").arg(e.what()), axis);
        return false;
    } catch (...) {
        setError(AxisError::UnknownError, "设置速度时发生未知异常", axis);
        return false;
    }
}

bool AxisController::setAxisAcceleration(AxisIndex axis, double acceleration)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isValidAxis(axis)) {
        setError(AxisError::InvalidAxis, "无效的轴编号", axis);
        return false;
    }
    
    if (acceleration < Constants::MIN_ACCELERATION || acceleration > Constants::MAX_ACCELERATION) {
        setError(AxisError::ParameterOutOfRange, 
                QString("加速度 %1 超出范围 [%2-%3]")
                .arg(acceleration).arg(Constants::MIN_ACCELERATION).arg(Constants::MAX_ACCELERATION), axis);
        return false;
    }
    
    int axisIndex = axisToMCC6Index(axis);
    
    try {
        if (isConnected()) {
            int result = m_controller->MoCtrCard_SendPara(
                static_cast<uint8_t>(axisIndex), 
                1, // 加速度参数索引
                static_cast<float>(acceleration)
            );
            if (!handleMCC6Error(result, QString("设置%1加速度").arg(axisToString(axis)))) {
                return false;
            }
        }
        
        m_motionParams[axisIndex].acceleration = acceleration;
        m_motionParams[axisIndex].deceleration = acceleration; // 默认减速度与加速度相同
        
        qDebug() << QString("%1加速度设置为 %2 μm/s²").arg(axisToString(axis)).arg(acceleration);
        
        return true;
        
    } catch (const std::exception& e) {
        setError(AxisError::HardwareError, QString("设置加速度异常：%1").arg(e.what()), axis);
        return false;
    } catch (...) {
        setError(AxisError::UnknownError, "设置加速度时发生未知异常", axis);
        return false;
    }
}

bool AxisController::setAxisEnabled(AxisIndex axis, bool enabled)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        setError(AxisError::DeviceNotConnected, "设备未连接", axis);
        return false;
    }
    
    if (!isValidAxis(axis)) {
        setError(AxisError::InvalidAxis, "无效的轴编号", axis);
        return false;
    }
    
    int axisIndex = axisToMCC6Index(axis);
    
    try {
        // 使用参数设置接口设置轴使能状态（假设参数索引3为使能）
        int result = m_controller->MoCtrCard_SendPara(
            static_cast<uint8_t>(axisIndex), 
            3, // 使能参数索引
            static_cast<int8_t>(enabled ? 1 : 0)
        );
        if (!handleMCC6Error(result, QString("%1%2使能").arg(axisToString(axis)).arg(enabled ? "启用" : "禁用"))) {
            return false;
        }
        
        m_axisStates[axisIndex].isEnabled = enabled;
        
        qDebug() << QString("%1%2使能").arg(axisToString(axis)).arg(enabled ? "启用" : "禁用");
        
        return true;
        
    } catch (const std::exception& e) {
        setError(AxisError::HardwareError, QString("设置使能异常：%1").arg(e.what()), axis);
        return false;
    } catch (...) {
        setError(AxisError::UnknownError, "设置使能时发生未知异常", axis);
        return false;
    }
}

bool AxisController::setPositionZero(AxisIndex axis)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        setError(AxisError::DeviceNotConnected, "设备未连接", axis);
        return false;
    }
    
    if (!isValidAxis(axis)) {
        setError(AxisError::InvalidAxis, "无效的轴编号", axis);
        return false;
    }
    
    int axisIndex = axisToMCC6Index(axis);
    
    try {
        // 使用MDI命令设置当前位置为零点
        QString axisChar;
        switch (axis) {
            case AxisIndex::X_AXIS: axisChar = "X"; break;
            case AxisIndex::Y_AXIS: axisChar = "Y"; break;
            case AxisIndex::Z_AXIS: axisChar = "Z"; break;
            default: axisChar = "X"; break;
        }
        QString mdiCommand = QString("G92 %1%2").arg(axisChar).arg(0.0);
        QByteArray mdiBytes = mdiCommand.toLocal8Bit();
        
        int result = m_controller->MoCtrCard_SendMDICommand(mdiBytes.data());
        if (!handleMCC6Error(result, QString("%1设置零点").arg(axisToString(axis)))) {
            return false;
        }
        
        m_axisStates[axisIndex].currentPosition = 0.0;
        m_axisStates[axisIndex].actualPosition = 0.0;
        m_axisStates[axisIndex].targetPosition = 0.0;
        
        emitPositionChanged(axis, 0.0);
        emitActualPositionChanged(axis, 0.0);
        
        qDebug() << QString("%1位置已设为零点").arg(axisToString(axis));
        
        return true;
        
    } catch (const std::exception& e) {
        setError(AxisError::HardwareError, QString("设置零点异常：%1").arg(e.what()), axis);
        return false;
    } catch (...) {
        setError(AxisError::UnknownError, "设置零点时发生未知异常", axis);
        return false;
    }
}

// ==================== 错误处理实现 ====================

AxisError AxisController::getLastError() const
{
    QMutexLocker locker(&m_errorMutex);
    return m_lastError;
}

QString AxisController::getLastErrorString() const
{
    QMutexLocker locker(&m_errorMutex);
    return m_lastErrorString;
}

void AxisController::clearError()
{
    QMutexLocker locker(&m_errorMutex);
    m_lastError = AxisError::NoError;
    m_lastErrorString.clear();
    
    // 清除所有轴的错误状态
    for (int i = 0; i < Constants::MAX_AXIS_COUNT; ++i) {
        m_axisStates[i].lastError = AxisError::NoError;
    }
}

AxisError AxisController::getAxisLastError(AxisIndex axis) const
{
    QMutexLocker locker(&m_mutex);
    
    if (!isValidAxis(axis)) {
        return AxisError::InvalidAxis;
    }
    
    return m_axisStates[axisToMCC6Index(axis)].lastError;
}

// ==================== 系统控制实现 ====================

void AxisController::setStatusMonitorEnabled(bool enabled, int interval)
{
    m_statusMonitorEnabled = enabled;
    m_statusUpdateInterval = qBound(10, interval, 10000); // 限制在10ms-10s之间
    
    if (enabled && isConnected()) {
        m_statusTimer->start(m_statusUpdateInterval);
        qDebug() << QString("状态监控已启用，更新间隔：%1ms").arg(m_statusUpdateInterval);
    } else {
        m_statusTimer->stop();
        qDebug() << "状态监控已停用";
    }
}

bool AxisController::initializeAllAxes()
{
    if (!m_controller) {
        setError(AxisError::InitializationFailed, "控制器未创建");
        return false;
    }
    
    try {
        // 初始化所有轴（设置默认参数）
        for (int i = 0; i < Constants::MAX_AXIS_COUNT; ++i) {
            // 设置默认参数
            int result;
            
            // 设置默认速度
            result = m_controller->MoCtrCard_SendPara(
                static_cast<uint8_t>(i), 
                0, // 速度参数索引
                static_cast<float>(m_motionParams[i].maxSpeed)
            );
            handleMCC6Error(result, QString("设置%1默认速度").arg(axisToString(static_cast<AxisIndex>(i))));
            
            // 设置默认加速度
            result = m_controller->MoCtrCard_SendPara(
                static_cast<uint8_t>(i), 
                1, // 加速度参数索引
                static_cast<float>(m_motionParams[i].acceleration)
            );
            handleMCC6Error(result, QString("设置%1默认加速度").arg(axisToString(static_cast<AxisIndex>(i))));
            
            // 设置默认细分
            result = m_controller->MoCtrCard_SendPara(
                static_cast<uint8_t>(i), 
                2, // 细分参数索引
                static_cast<uint32_t>(m_motionParams[i].subdivision)
            );
            handleMCC6Error(result, QString("设置%1默认细分").arg(axisToString(static_cast<AxisIndex>(i))));
            
            // 启用轴
            result = m_controller->MoCtrCard_SendPara(
                static_cast<uint8_t>(i), 
                3, // 使能参数索引
                static_cast<int8_t>(1)
            );
            if (handleMCC6Error(result, QString("启用%1").arg(axisToString(static_cast<AxisIndex>(i))))) {
                m_axisStates[i].isEnabled = true;
            }
        }
        
        qDebug() << "所有轴初始化完成";
        return true;
        
    } catch (const std::exception& e) {
        setError(AxisError::InitializationFailed, QString("初始化异常：%1").arg(e.what()));
        return false;
    } catch (...) {
        setError(AxisError::InitializationFailed, "初始化时发生未知异常");
        return false;
    }
}

void AxisController::emergencyStop()
{
    m_emergencyStopActive = true;
    
    if (isConnected()) {
        stopAllAxes(true); // 急停所有轴
    }
    
    // 更新所有轴状态为错误状态
    for (int i = 0; i < Constants::MAX_AXIS_COUNT; ++i) {
        m_axisStates[i].motionState = MotionState::Error;
        m_axisStates[i].lastError = AxisError::EmergencyStop;
        
        emitMotionStateChanged(static_cast<AxisIndex>(i), MotionState::Error);
    }
    
    setError(AxisError::EmergencyStop, "急停已触发");
    emit emergencyStopTriggered();
    
    qDebug() << "急停已触发";
}

bool AxisController::resetController()
{
    QMutexLocker locker(&m_mutex);
    
    if (!isConnected()) {
        setError(AxisError::DeviceNotConnected, "设备未连接");
        return false;
    }
    
    try {
        // 重置控制器（使用MDI命令）
        char resetCommand[] = "M02"; // 程序结束并重置
        int result = m_controller->MoCtrCard_SendMDICommand(resetCommand);
        if (!handleMCC6Error(result, "重置控制器")) {
            return false;
        }
        
        // 清除急停状态
        m_emergencyStopActive = false;
        
        // 重新初始化所有轴
        if (!initializeAllAxes()) {
            return false;
        }
        
        // 清除错误状态
        clearError();
        
        qDebug() << "控制器重置完成";
        return true;
        
    } catch (const std::exception& e) {
        setError(AxisError::HardwareError, QString("重置控制器异常：%1").arg(e.what()));
        return false;
    } catch (...) {
        setError(AxisError::UnknownError, "重置控制器时发生未知异常");
        return false;
    }
}

QString AxisController::getVersion()
{
    return QString("AxisController v1.0.0 - MCC6DLL Integration");
}

// ==================== 私有槽函数实现 ====================

void AxisController::updateAxisStatus()
{
    if (!isConnected() || !m_statusMonitorEnabled) {
        return;
    }
    
    QMutexLocker locker(&m_mutex);
    
    try {
        bool anyStatusChanged = false;
        
        for (int i = 0; i < Constants::MAX_AXIS_COUNT; ++i) {
            // 查询命令位置
            float position[1] = {0.0f};
            int result = m_controller->MoCtrCard_GetAxisPos(static_cast<uint8_t>(i), position);
            if (result == 1) { // funResOk = 0x01
                double newPos = static_cast<double>(position[0]) * 1000.0; // mm -> µm
                if (qAbs(m_axisStates[i].currentPosition - newPos) > Constants::POSITION_TOLERANCE) {
                    m_axisStates[i].currentPosition = newPos;
                    emitPositionChanged(static_cast<AxisIndex>(i), newPos);
                    anyStatusChanged = true;
                }
            }
            
            // 查询实际位置（光栅尺读数）
            float actualPosition[1] = {0.0f};
            result = m_controller->MoCtrCard_GetAxisActualPos(static_cast<uint8_t>(i), actualPosition);
            if (result == 1) { // funResOk = 0x01
                double newActualPos = static_cast<double>(actualPosition[0]) * 1000.0; // mm -> µm
                if (qAbs(m_axisStates[i].actualPosition - newActualPos) > Constants::POSITION_TOLERANCE) {
                    m_axisStates[i].actualPosition = newActualPos;
                    emitActualPositionChanged(static_cast<AxisIndex>(i), newActualPos);
                    anyStatusChanged = true;
                }
            }
            
            // 查询运动状态
            int running[1] = {0};
            result = m_controller->MoCtrCard_IsAxisRunning(static_cast<uint8_t>(i), running);
            if (result == 1) { // funResOk = 0x01
                MotionState newState = (running[0] != 0) ? MotionState::Moving : MotionState::Idle;
                
                // 检查是否完成回零
                if (m_axisStates[i].motionState == MotionState::Homing && newState == MotionState::Idle) {
                    // 寻零完成，设置已回零标志
                    m_axisStates[i].isHomed = true;
                    const_cast<AxisController*>(this)->emit homeCompleted(static_cast<AxisIndex>(i), true);
                }
                
                // 检查运动是否完成
                if (m_axisStates[i].motionState == MotionState::Moving && newState == MotionState::Idle) {
                    const_cast<AxisController*>(this)->emit motionCompleted(static_cast<AxisIndex>(i), m_axisStates[i].currentPosition);
                    
                    // 停止运动超时定时器（如果没有其他轴在运动）
                    bool anyMoving = false;
                    for (int j = 0; j < Constants::MAX_AXIS_COUNT; ++j) {
                        if (m_axisStates[j].motionState == MotionState::Moving) {
                            anyMoving = true;
                            break;
                        }
                    }
                    if (!anyMoving) {
                        m_motionTimeoutTimer->stop();
                    }
                }
                
                if (m_axisStates[i].motionState != newState) {
                    m_axisStates[i].motionState = newState;
                    emitMotionStateChanged(static_cast<AxisIndex>(i), newState);
                    anyStatusChanged = true;
                }
            }
            
            // 暂时不查询限位状态（MCC6DLL中没有对应函数）
            // 如果需要限位状态，可以通过参数查询接口获取
            
            // 更新时间戳
            m_axisStates[i].lastUpdateTime = QDateTime::currentMSecsSinceEpoch();
        }
        
        if (anyStatusChanged) {
            emit systemStatusUpdated();
        }
        
    } catch (const std::exception& e) {
        setError(AxisError::CommunicationError, QString("状态更新异常：%1").arg(e.what()));
    } catch (...) {
        setError(AxisError::UnknownError, "状态更新时发生未知异常");
    }
}

void AxisController::handleMotionCompleted(int axis)
{
    if (axis >= 0 && axis < Constants::MAX_AXIS_COUNT) {
        updateSingleAxisStatus(static_cast<AxisIndex>(axis), true);
    }
}

// ==================== 私有辅助函数实现 ====================

bool AxisController::initialize()
{
    // 初始化已在connectDevice中实现
    return true;
}

void AxisController::cleanup()
{
    try {
        // 停止定时器
        if (m_statusTimer) {
            m_statusTimer->stop();
        }
        
        if (m_motionTimeoutTimer) {
            m_motionTimeoutTimer->stop();
        }
        
        // 断开连接
        if (m_controller && m_connectionActive) {
            stopAllAxes(true);
            m_controller->MoCtrCard_Unload();
            delete m_controller;
        }
        
        m_controller = nullptr;
        m_connectionActive = false;
        m_isInitialized = false;
        
    } catch (...) {
        // 忽略清理过程中的异常
    }
}

bool AxisController::isValidAxis(AxisIndex axis) const
{
    return axis >= AxisIndex::X_AXIS && axis <= AxisIndex::Z_AXIS;
}

bool AxisController::validateMotionParams(AxisIndex axis, double distance, double speed) const
{
    if (!isValidAxis(axis)) {
        setError(AxisError::InvalidAxis, "无效的轴编号", axis);
        return false;
    }
    
    if (qAbs(distance) < 0.001) {
        setError(AxisError::InvalidParameter, "移动距离太小", axis);
        return false;
    }
    
    if (speed > 0 && (speed < Constants::MIN_SPEED || speed > Constants::MAX_SPEED)) {
        setError(AxisError::ParameterOutOfRange, 
                QString("速度 %1 超出范围 [%2-%3]")
                .arg(speed).arg(Constants::MIN_SPEED).arg(Constants::MAX_SPEED), axis);
        return false;
    }
    
    return true;
}

bool AxisController::handleMCC6Error(int errorCode, const QString& operation)
{
    // MCC6成功返回值是1 (funResOk = 0x01)，不是0！
    if (errorCode == MCC6Constants::MCC6_SUCCESS) {
        return true; // 操作成功
    }
    
    QString errorMsg;
    AxisError axisError = AxisError::HardwareError;
    
    // 根据MCC6错误码映射到AxisError
    switch (errorCode) {
        // MCC6标准错误码
        case MCC6Constants::MCC6_AXIS_ERROR: // 0x02
            axisError = AxisError::InvalidParameter;
            errorMsg = QString("%1：轴序号错误").arg(operation);
            break;
        case MCC6Constants::MCC6_PORT_ERROR: // 0x80
            axisError = AxisError::CommunicationError;
            errorMsg = QString("%1：串口打开失败").arg(operation);
            break;
        case MCC6Constants::MCC6_ERROR: // 0x83
            axisError = AxisError::HardwareError;
            errorMsg = QString("%1：MCC6设备错误").arg(operation);
            break;
        
        // 自定义错误码（保持向后兼容）
        case -1:
            axisError = AxisError::CommunicationError;
            errorMsg = QString("%1：通信错误").arg(operation);
            break;
        case -2:
            axisError = AxisError::InvalidParameter;
            errorMsg = QString("%1：参数无效").arg(operation);
            break;
        case -3:
            axisError = AxisError::HardwareError;
            errorMsg = QString("%1：硬件错误").arg(operation);
            break;
        case -4:
            axisError = AxisError::MotionTimeout;
            errorMsg = QString("%1：操作超时").arg(operation);
            break;
        case -5:
            axisError = AxisError::LimitReached;
            errorMsg = QString("%1：到达限位").arg(operation);
            break;
        default:
            axisError = AxisError::UnknownError;
            errorMsg = QString("%1：未知错误（错误码：%2）").arg(operation).arg(errorCode);
            break;
    }
    
    setError(axisError, errorMsg);
    return false;
}

void AxisController::setError(AxisError error, const QString& errorString, AxisIndex axis) const
{
    {
        QMutexLocker locker(&m_errorMutex);
        const_cast<AxisController*>(this)->m_lastError = error;
        const_cast<AxisController*>(this)->m_lastErrorString = errorString;
    }
    
    if (axis != AxisIndex::INVALID_AXIS && isValidAxis(axis)) {
        int axisIndex = axisToMCC6Index(axis);
        const_cast<AxisController*>(this)->m_axisStates[axisIndex].lastError = error;
        const_cast<AxisController*>(this)->emit errorOccurred(axis, error, errorString);
    } else {
        const_cast<AxisController*>(this)->emit errorOccurred(AxisIndex::INVALID_AXIS, error, errorString);
    }
    
    qWarning() << QString("AxisController错误：%1").arg(errorString);
}

void AxisController::updateSingleAxisStatus(AxisIndex axis, bool forceUpdate)
{
    if (!isConnected() || !isValidAxis(axis)) {
        return;
    }
    
    int axisIndex = axisToMCC6Index(axis);
    
    try {
        // 查询命令位置
        float position[1] = {0.0f};
        int result = m_controller->MoCtrCard_GetAxisPos(static_cast<uint8_t>(axisIndex), position);
        if (result == 1) { // funResOk = 0x01
            double newPos = static_cast<double>(position[0]) * 1000.0; // mm -> µm
            if (forceUpdate || qAbs(m_axisStates[axisIndex].currentPosition - newPos) > Constants::POSITION_TOLERANCE) {
                m_axisStates[axisIndex].currentPosition = newPos;
                emitPositionChanged(axis, newPos);
            }
        }
        
        // 查询实际位置（光栅尺读数）
        float actualPosition[1] = {0.0f};
        result = m_controller->MoCtrCard_GetAxisActualPos(static_cast<uint8_t>(axisIndex), actualPosition);
        if (result == 1) { // funResOk = 0x01
            double newActualPos = static_cast<double>(actualPosition[0]) * 1000.0; // mm -> µm
            if (forceUpdate || qAbs(m_axisStates[axisIndex].actualPosition - newActualPos) > Constants::POSITION_TOLERANCE) {
                m_axisStates[axisIndex].actualPosition = newActualPos;
                emitActualPositionChanged(axis, newActualPos);
            }
        }
        
        // 查询运动状态
        int running[1] = {0};
        result = m_controller->MoCtrCard_IsAxisRunning(static_cast<uint8_t>(axisIndex), running);
        if (result == 1) { // funResOk = 0x01
            MotionState newState = (running[0] != 0) ? MotionState::Moving : MotionState::Idle;
            if (forceUpdate || m_axisStates[axisIndex].motionState != newState) {
                m_axisStates[axisIndex].motionState = newState;
                emitMotionStateChanged(axis, newState);
            }
        }
        
    } catch (const std::exception& e) {
        setError(AxisError::CommunicationError, QString("更新%1状态异常：%2").arg(axisToString(axis)).arg(e.what()), axis);
    } catch (...) {
        setError(AxisError::UnknownError, QString("更新%1状态时发生未知异常").arg(axisToString(axis)), axis);
    }
}

void AxisController::emitPositionChanged(AxisIndex axis, double newPosition)
{
    emit positionChanged(axis, newPosition);
}

void AxisController::emitActualPositionChanged(AxisIndex axis, double newActualPosition)
{
    emit actualPositionChanged(axis, newActualPosition);
}

void AxisController::emitMotionStateChanged(AxisIndex axis, MotionState newState)
{
    emit motionStateChanged(axis, newState);
}

bool AxisController::checkSoftLimits(AxisIndex axis, double targetPosition) const
{
    if (!isValidAxis(axis)) {
        return false;
    }
    
    int axisIndex = axisToMCC6Index(axis);
    const MotionParams& params = m_motionParams[axisIndex];
    
    return targetPosition >= params.softLimitNeg && targetPosition <= params.softLimitPos;
}

int AxisController::axisToMCC6Index(AxisIndex axis) const
{
    return static_cast<int>(axis);
}

QString AxisController::axisToString(AxisIndex axis) const
{
    switch (axis) {
        case AxisIndex::X_AXIS: return "X轴";
        case AxisIndex::Y_AXIS: return "Y轴";
        case AxisIndex::Z_AXIS: return "Z轴";
        default: return "未知轴";
    }
}

