#include "SerialPortDetector.h"
#include <QDebug>
#include <QSerialPort>
#include <QApplication>
#include <algorithm>

SerialPortDetector::SerialPortDetector(QObject *parent)
    : QObject(parent)
    , m_detectionTimer(nullptr)
    , m_autoDetectionEnabled(false)
{
    // 创建检测定时器
    m_detectionTimer = new QTimer(this);
    m_detectionTimer->setSingleShot(false);
    connect(m_detectionTimer, &QTimer::timeout, this, &SerialPortDetector::detectPortChanges);

    // 初始检测一次
    m_lastDetectedPorts = getAvailableSerialPorts();
    
    qDebug() << "SerialPortDetector初始化完成，检测到" << m_lastDetectedPorts.size() << "个串口";
}

SerialPortDetector::~SerialPortDetector()
{
    stopAutoDetection();
}

QList<SerialPortDetector::SerialPortInfo> SerialPortDetector::getAvailableSerialPorts()
{
    QList<SerialPortInfo> ports;
    
    // 获取系统中所有串口信息
    const QList<QSerialPortInfo> availablePorts = QSerialPortInfo::availablePorts();
    
    for (const QSerialPortInfo& portInfo : availablePorts) {
        SerialPortInfo info = convertPortInfo(portInfo);
        
        // 检查串口是否被占用（可选）
        info.isBusy = false;  // 简化实现，不检查占用状态
        
        ports.append(info);
    }
    
    // 按端口名称排序
    std::sort(ports.begin(), ports.end(), [](const SerialPortInfo& a, const SerialPortInfo& b) {
        return a.portName < b.portName;
    });
    
    return ports;
}

QStringList SerialPortDetector::getSerialPortNames()
{
    QStringList portNames;
    const QList<SerialPortInfo> ports = getAvailableSerialPorts();
    
    for (const SerialPortInfo& port : ports) {
        portNames.append(port.portName);
    }
    
    return portNames;
}

bool SerialPortDetector::updateComboBoxPorts(QComboBox* comboBox, bool keepCurrentSelection)
{
    if (!comboBox) {
        qWarning() << "ComboBox指针为空";
        return false;
    }
    
    // 保存当前选择
    QString currentSelection;
    if (keepCurrentSelection) {
        currentSelection = comboBox->currentText();
    }
    
    // 获取当前可用串口
    const QList<SerialPortInfo> ports = getAvailableSerialPorts();
    
    // 清空现有项目
    comboBox->clear();
    
    // 添加新的串口项目
    for (const SerialPortInfo& port : ports) {
        comboBox->addItem(port.displayName(), port.portName);
    }
    
    // 如果没有找到串口，添加提示项
    if (ports.isEmpty()) {
        comboBox->addItem("未检测到串口", "");
        comboBox->setEnabled(false);
    } else {
        comboBox->setEnabled(true);
        
        // 尝试恢复之前的选择
        if (keepCurrentSelection && !currentSelection.isEmpty()) {
            int index = comboBox->findText(currentSelection);
            if (index >= 0) {
                comboBox->setCurrentIndex(index);
            } else {
                // 如果之前选择的串口不再可用，查找部分匹配
                for (int i = 0; i < comboBox->count(); ++i) {
                    if (comboBox->itemData(i).toString() == currentSelection) {
                        comboBox->setCurrentIndex(i);
                        break;
                    }
                }
            }
        }
    }
    
    qDebug() << "更新ComboBox串口列表，找到" << ports.size() << "个串口";
    return true;
}

void SerialPortDetector::startAutoDetection(int interval)
{
    if (m_detectionTimer && !m_autoDetectionEnabled) {
        m_detectionTimer->setInterval(interval);
        m_detectionTimer->start();
        m_autoDetectionEnabled = true;
        qDebug() << "启动串口自动检测，间隔:" << interval << "ms";
    }
}

void SerialPortDetector::stopAutoDetection()
{
    if (m_detectionTimer && m_autoDetectionEnabled) {
        m_detectionTimer->stop();
        m_autoDetectionEnabled = false;
        qDebug() << "停止串口自动检测";
    }
}

bool SerialPortDetector::isPortAvailable(const QString& portName)
{
    if (portName.isEmpty()) {
        return false;
    }
    
    const QStringList availablePorts = getSerialPortNames();
    return availablePorts.contains(portName);
}

QString SerialPortDetector::getRecommendedPort()
{
    const QList<SerialPortInfo> ports = getAvailableSerialPorts();
    
    if (ports.isEmpty()) {
        return QString();
    }
    
    // 优先查找包含"CH340"的串口
    for (const SerialPortInfo& port : ports) {
        if (port.description.contains("CH340", Qt::CaseInsensitive) ||
            port.portName.contains("CH340", Qt::CaseInsensitive)) {
            qDebug() << "找到CH340串口，优先使用:" << port.portName << port.description;
            return port.portName;
        }
    }
    
    // 如果没有找到CH340串口，返回第一个可用串口
    qDebug() << "未找到CH340串口，使用第一个可用串口:" << ports.first().portName;
    return ports.first().portName;
}

void SerialPortDetector::detectPortChanges()
{
    const QList<SerialPortInfo> currentPorts = getAvailableSerialPorts();
    
    // 比较端口列表变化
    comparePortLists(m_lastDetectedPorts, currentPorts);
    
    // 更新缓存的端口列表
    m_lastDetectedPorts = currentPorts;
}

void SerialPortDetector::comparePortLists(const QList<SerialPortInfo>& oldPorts, 
                                        const QList<SerialPortInfo>& newPorts)
{
    // 检测新增的端口
    for (const SerialPortInfo& newPort : newPorts) {
        bool found = false;
        for (const SerialPortInfo& oldPort : oldPorts) {
            if (oldPort.portName == newPort.portName) {
                found = true;
                break;
            }
        }
        if (!found) {
            qDebug() << "检测到新串口:" << newPort.portName;
            emit portAdded(newPort);
        }
    }
    
    // 检测移除的端口
    for (const SerialPortInfo& oldPort : oldPorts) {
        bool found = false;
        for (const SerialPortInfo& newPort : newPorts) {
            if (newPort.portName == oldPort.portName) {
                found = true;
                break;
            }
        }
        if (!found) {
            qDebug() << "检测到串口移除:" << oldPort.portName;
            emit portRemoved(oldPort.portName);
        }
    }
    
    // 如果列表有变化，发出信号
    if (oldPorts.size() != newPorts.size()) {
        emit portsChanged(newPorts);
    }
}

SerialPortDetector::SerialPortInfo SerialPortDetector::convertPortInfo(const QSerialPortInfo& info)
{
    SerialPortInfo serialInfo;
    serialInfo.portName = info.portName();
    serialInfo.description = info.description();
    serialInfo.manufacturer = info.manufacturer();
    serialInfo.serialNumber = info.serialNumber();
    serialInfo.isBusy = false;  // 简化实现
    
    return serialInfo;
}
