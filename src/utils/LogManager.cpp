#include "LogManager.h"
#include <QApplication>
#include <QScrollBar>
#include <QDebug>

// 静态常量定义
const QString LogManager::LOG_DIR_NAME = "Logs";
const QString LogManager::LOG_FILE_EXTENSION = ".txt";
const int LogManager::MAX_UI_LOG_LINES = 1000;

LogManager::LogManager(QScrollArea* scrollArea, QObject* parent)
    : QObject(parent)
    , m_scrollArea(scrollArea)
    , m_textEdit(nullptr)
    , m_logLevel(LogLevel::INFO)
    , m_fileLoggingEnabled(true)
    , m_uiLoggingEnabled(true)
    , m_logFile(nullptr)
    , m_logStream(nullptr)
    , m_rotationTimer(nullptr)
{
    initializeLogging();
}

LogManager::~LogManager()
{
    if (m_logStream) {
        delete m_logStream;
        m_logStream = nullptr;
    }
    
    if (m_logFile) {
        if (m_logFile->isOpen()) {
            m_logFile->close();
        }
        delete m_logFile;
        m_logFile = nullptr;
    }
}

void LogManager::initializeLogging()
{
    // 创建日志目录
    createLogDirectory();
    
    // 设置UI组件
    if (m_scrollArea) {
        setUIComponent(m_scrollArea);
    }
    
    // 初始化当前日期和日志文件
    m_currentDate = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    checkNewLogFile();
    
    // 设置日志轮转定时器（每小时检查一次）
    m_rotationTimer = new QTimer(this);
    connect(m_rotationTimer, &QTimer::timeout, this, &LogManager::checkLogRotation);
    m_rotationTimer->start(3600000); // 1小时 = 3600000毫秒
    
    // 记录系统启动日志
    log("日志管理器初始化完成", LogLevel::INFO);
}

void LogManager::createLogDirectory()
{
    m_logDirectory = QDir::currentPath() + "/" + LOG_DIR_NAME;
    QDir dir;
    if (!dir.exists(m_logDirectory)) {
        if (!dir.mkpath(m_logDirectory)) {
            qDebug() << "创建日志目录失败：" << m_logDirectory;
        }
    }
}

void LogManager::setUIComponent(QScrollArea* scrollArea)
{
    m_scrollArea = scrollArea;
    
    if (m_scrollArea) {
        // 创建文本编辑器
        if (!m_textEdit) {
            m_textEdit = new QTextEdit();
            m_textEdit->setReadOnly(true);
            // Qt6中使用document()->setMaximumBlockCount()来限制最大行数
            m_textEdit->document()->setMaximumBlockCount(MAX_UI_LOG_LINES);
            m_scrollArea->setWidget(m_textEdit);
        }
    }
}

void LogManager::setLogLevel(LogLevel level)
{
    QMutexLocker locker(&m_mutex);
    m_logLevel = level;
}

void LogManager::setFileLoggingEnabled(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_fileLoggingEnabled = enabled;
}

void LogManager::setUILoggingEnabled(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_uiLoggingEnabled = enabled;
}

void LogManager::logCameraOperation(const QString& operation, const QString& cameraSN, const QString& details)
{
    QString message = QString("相机操作 - %1").arg(operation);
    if (!cameraSN.isEmpty()) {
        message += QString(" - 相机SN: %1").arg(cameraSN);
    }
    if (!details.isEmpty()) {
        message += QString(" - 详情: %1").arg(details);
    }
    log(message, LogLevel::INFO);
}

void LogManager::logParameterChange(const QString& paramName, const QString& oldValue, const QString& newValue, const QString& cameraSN)
{
    QString message = QString("参数修改 - %1 - 从 %2 改为 %3").arg(paramName, oldValue, newValue);
    if (!cameraSN.isEmpty()) {
        message += QString(" - 相机SN: %1").arg(cameraSN);
    }
    log(message, LogLevel::INFO);
}

void LogManager::logUIOperation(const QString& operation, const QString& details)
{
    QString message = QString("界面操作 - %1").arg(operation);
    if (!details.isEmpty()) {
        message += QString(" - 详情: %1").arg(details);
    }
    log(message, LogLevel::INFO);
}

void LogManager::logDrawingOperation(const QString& operation, const QString& details)
{
    QString message = QString("绘图操作 - %1").arg(operation);
    if (!details.isEmpty()) {
        message += QString(" - 详情: %1").arg(details);
    }
    log(message, LogLevel::INFO);
}

void LogManager::logMeasurementOperation(const QString& operation, const QString& details)
{
    QString message = QString("测量操作 - %1").arg(operation);
    if (!details.isEmpty()) {
        message += QString(" - 详情: %1").arg(details);
    }
    log(message, LogLevel::INFO);
}

void LogManager::logSettingsOperation(const QString& operation, const QString& details)
{
    QString message = QString("设置操作 - %1").arg(operation);
    if (!details.isEmpty()) {
        message += QString(" - 详情: %1").arg(details);
    }
    log(message, LogLevel::INFO);
}

void LogManager::logError(const QString& errorMsg, const QString& details)
{
    QString message = QString("错误 - %1").arg(errorMsg);
    if (!details.isEmpty()) {
        message += QString(" - 详情: %1").arg(details);
    }
    log(message, LogLevel::ERROR);
}

void LogManager::logWarning(const QString& warningMsg, const QString& details)
{
    QString message = QString("警告 - %1").arg(warningMsg);
    if (!details.isEmpty()) {
        message += QString(" - 详情: %1").arg(details);
    }
    log(message, LogLevel::WARNING);
}

void LogManager::logDebug(const QString& debugMsg, const QString& details)
{
    QString message = QString("调试 - %1").arg(debugMsg);
    if (!details.isEmpty()) {
        message += QString(" - 详情: %1").arg(details);
    }
    log(message, LogLevel::DEBUG);
}

void LogManager::log(const QString& message, LogLevel level)
{
    QMutexLocker locker(&m_mutex);
    
    // 检查日志级别
    if (level < m_logLevel) {
        return;
    }
    
    // 检查是否需要创建新的日志文件
    checkNewLogFile();
    
    // 格式化日志消息
    QString formattedMessage = formatLogMessage(message, level);
    
    // 写入文件日志
    if (m_fileLoggingEnabled) {
        writeToFile(formattedMessage);
    }
    
    // 更新UI日志显示
    if (m_uiLoggingEnabled) {
        updateUILog(formattedMessage);
    }
}

QString LogManager::getLevelString(LogLevel level) const
{
    switch (level) {
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR:   return "ERROR";
        default:                return "UNKNOWN";
    }
}

QString LogManager::formatLogMessage(const QString& message, LogLevel level) const
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    return QString("[%1] [%2] %3").arg(timestamp, getLevelString(level), message);
}

void LogManager::writeToFile(const QString& message)
{
    if (m_logStream) {
        *m_logStream << message << Qt::endl;
        m_logStream->flush();
    }
}

void LogManager::updateUILog(const QString& message)
{
    if (m_textEdit) {
        // 在主线程中更新UI
        QMetaObject::invokeMethod(m_textEdit, [this, message]() {
            m_textEdit->append(message);
            // 滚动到底部
            QScrollBar* scrollBar = m_textEdit->verticalScrollBar();
            scrollBar->setValue(scrollBar->maximum());
        }, Qt::QueuedConnection);
    }
}

void LogManager::checkNewLogFile()
{
    QString currentDate = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    
    if (currentDate != m_currentDate || !m_logFile || !m_logFile->isOpen()) {
        // 关闭当前日志文件
        if (m_logStream) {
            delete m_logStream;
            m_logStream = nullptr;
        }
        
        if (m_logFile) {
            if (m_logFile->isOpen()) {
                m_logFile->close();
            }
            delete m_logFile;
            m_logFile = nullptr;
        }
        
        // 创建新的日志文件
        m_currentDate = currentDate;
        m_currentLogFile = m_logDirectory + "/" + m_currentDate + LOG_FILE_EXTENSION;
        
        m_logFile = new QFile(m_currentLogFile);
        if (m_logFile->open(QIODevice::WriteOnly | QIODevice::Append)) {
            m_logStream = new QTextStream(m_logFile);
            m_logStream->setEncoding(QStringConverter::Utf8);
        } else {
            qDebug() << "无法打开日志文件：" << m_currentLogFile;
        }
    }
}

void LogManager::checkLogRotation()
{
    QMutexLocker locker(&m_mutex);
    checkNewLogFile();
}

void LogManager::clearUILog()
{
    if (m_textEdit) {
        QMetaObject::invokeMethod(m_textEdit, [this]() {
            m_textEdit->clear();
        }, Qt::QueuedConnection);
    }
}

QString LogManager::getCurrentLogFile() const
{
    QMutexLocker locker(&m_mutex);
    return m_currentLogFile;
}
