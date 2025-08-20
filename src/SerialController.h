#pragma once

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QTimer>
#include <QString>
#include <QDebug>
#include <atomic>
#include <memory>

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#endif

/**
 * @brief 串口控制器类
 * 
 * 基于参考项目serial_test实现的跨平台串口通信模块
 * 支持物理按钮事件检测和状态监控
 */
class SerialController : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 按钮事件类型枚举
     */
    enum class ButtonEvent {
        Button1Pressed,     // 按钮1按下
        Button2Pressed,     // 按钮2按下  
        Button3Pressed,     // 按钮3按下
        EmergencyStop,      // 急停按钮
        Button1Released,    // 按钮1释放
        Button2Released,    // 按钮2释放
        Button3Released,    // 按钮3释放
        Unknown             // 未知事件
    };

    /**
     * @brief 串口状态枚举
     */
    enum class SerialStatus {
        Disconnected,       // 未连接
        Connected,          // 已连接
        Listening,          // 监听中
        Error              // 错误状态
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit SerialController(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~SerialController();

    /**
     * @brief 打开串口
     * @param portName 串口名称 (如 "COM5" 或 "/dev/ttyUSB0")
     * @param baudRate 波特率 (默认9600)
     * @return 是否成功打开
     */
    bool openPort(const QString& portName, int baudRate = 9600);

    /**
     * @brief 关闭串口
     */
    void closePort();

    /**
     * @brief 开始监听串口数据
     */
    void startListening();

    /**
     * @brief 停止监听串口数据
     */
    void stopListening();

    /**
     * @brief 获取当前串口状态
     * @return 串口状态
     */
    SerialStatus getStatus() const { return m_status; }

    /**
     * @brief 获取最后的错误信息
     * @return 错误信息字符串
     */
    QString getLastError() const { return m_lastError; }

    /**
     * @brief 检查是否正在监听
     * @return 是否正在监听
     */
    bool isListening() const { return m_isListening; }

signals:
    /**
     * @brief 按钮事件信号
     * @param event 按钮事件类型
     * @param data 原始数据
     */
    void buttonEventReceived(ButtonEvent event, const QByteArray& data);

    /**
     * @brief 串口状态变化信号
     * @param status 新的状态
     */
    void statusChanged(SerialStatus status);

    /**
     * @brief 错误发生信号
     * @param errorMsg 错误信息
     */
    void errorOccurred(const QString& errorMsg);

    /**
     * @brief 数据接收信号
     * @param data 接收到的原始数据
     */
    void dataReceived(const QByteArray& data);

private slots:
    /**
     * @brief 监听线程处理槽函数
     */
    void processSerialData();

private:
    /**
     * @brief 解析接收到的数据
     * @param data 原始数据
     * @return 按钮事件类型
     */
    ButtonEvent parseButtonEvent(const QByteArray& data);

    /**
     * @brief 设置串口状态
     * @param status 新状态
     */
    void setStatus(SerialStatus status);

    /**
     * @brief 设置错误信息
     * @param error 错误信息
     */
    void setError(const QString& error);

    /**
     * @brief 配置串口参数 (Windows)
     * @param baudRate 波特率
     * @return 是否成功
     */
#ifdef _WIN32
    bool configurePortWindows(int baudRate);
#else
    /**
     * @brief 配置串口参数 (Linux)
     * @param baudRate 波特率
     * @return 是否成功
     */
    bool configurePortLinux(int baudRate);
#endif

private:
    // 串口句柄
#ifdef _WIN32
    HANDLE m_serialHandle;
#else
    int m_serialFd;
#endif

    // 状态管理
    std::atomic<SerialStatus> m_status;
    std::atomic<bool> m_isListening;
    QString m_lastError;
    QString m_portName;
    int m_baudRate;

    // 线程安全
    mutable QMutex m_mutex;
    
    // 数据监听
    QTimer* m_listenTimer;
    
    // 数据缓冲
    QByteArray m_dataBuffer;
    
    // 常量定义
    static const int LISTEN_INTERVAL_MS = 100;  // 监听间隔(毫秒)，进一步降低CPU占用
    static const int BUFFER_SIZE = 256;        // 缓冲区大小
};
