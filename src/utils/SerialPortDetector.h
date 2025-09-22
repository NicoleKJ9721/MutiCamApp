#ifndef SERIALPORTDETECTOR_H
#define SERIALPORTDETECTOR_H

#include <QObject>
#include <QStringList>
#include <QSerialPortInfo>
#include <QTimer>
#include <QComboBox>

/**
 * @brief 串口自动检测工具类
 * 
 * 提供系统串口的自动检测、列表更新和选择框填充功能
 */
class SerialPortDetector : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 串口信息结构
     */
    struct SerialPortInfo {
        QString portName;           // 端口名称 (如 COM1, /dev/ttyUSB0)
        QString description;        // 设备描述
        QString manufacturer;       // 制造商
        QString serialNumber;       // 序列号
        bool isBusy;               // 是否被占用

        QString displayName() const {
            QString display = portName;
            if (!description.isEmpty()) {
                display += QString(" (%1)").arg(description);
            }
            return display;
        }
    };

    explicit SerialPortDetector(QObject *parent = nullptr);
    ~SerialPortDetector();

    /**
     * @brief 获取当前可用的串口列表
     * @return 串口信息列表
     */
    QList<SerialPortInfo> getAvailableSerialPorts();

    /**
     * @brief 获取串口名称列表
     * @return 串口名称字符串列表
     */
    QStringList getSerialPortNames();

    /**
     * @brief 更新ComboBox的串口列表
     * @param comboBox 目标下拉框
     * @param keepCurrentSelection 是否保持当前选择
     * @return 是否成功更新
     */
    bool updateComboBoxPorts(QComboBox* comboBox, bool keepCurrentSelection = true);

    /**
     * @brief 启动自动检测定时器
     * @param interval 检测间隔（毫秒），默认2000ms
     */
    void startAutoDetection(int interval = 2000);

    /**
     * @brief 停止自动检测
     */
    void stopAutoDetection();

    /**
     * @brief 检查指定串口是否可用
     * @param portName 串口名称
     * @return 是否可用
     */
    bool isPortAvailable(const QString& portName);

    /**
     * @brief 获取推荐的串口（优先CH340串口，如果没有CH340串口则不推荐）
     * @return CH340串口名称，如果没有找到CH340串口或没有可用串口则返回空字符串
     */
    QString getRecommendedPort();

signals:
    /**
     * @brief 串口列表发生变化时发出
     * @param availablePorts 当前可用串口列表
     */
    void portsChanged(const QList<SerialPortInfo>& availablePorts);

    /**
     * @brief 新串口被检测到时发出
     * @param portInfo 新检测到的串口信息
     */
    void portAdded(const SerialPortInfo& portInfo);

    /**
     * @brief 串口被移除时发出
     * @param portName 被移除的串口名称
     */
    void portRemoved(const QString& portName);

private slots:
    /**
     * @brief 定时检测串口变化
     */
    void detectPortChanges();

private:
    /**
     * @brief 比较两个串口列表，检测变化
     * @param oldPorts 旧串口列表
     * @param newPorts 新串口列表
     */
    void comparePortLists(const QList<SerialPortInfo>& oldPorts, 
                         const QList<SerialPortInfo>& newPorts);

    /**
     * @brief 将QSerialPortInfo转换为SerialPortInfo
     * @param info Qt串口信息
     * @return 自定义串口信息结构
     */
    SerialPortInfo convertPortInfo(const QSerialPortInfo& info);

private:
    QTimer* m_detectionTimer;                    // 自动检测定时器
    QList<SerialPortInfo> m_lastDetectedPorts;  // 上次检测到的串口列表
    bool m_autoDetectionEnabled;                 // 是否启用自动检测
};

#endif // SERIALPORTDETECTOR_H
