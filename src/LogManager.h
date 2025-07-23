#pragma once

#include <QObject>
#include <QString>
#include <QTextEdit>
#include <QScrollArea>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QTimer>
#include <memory>

/**
 * @brief 日志级别枚举
 */
enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3
};

/**
 * @brief 日志管理器类
 * 
 * 提供完整的日志记录功能，包括：
 * - 文件日志记录（按日期分文件）
 * - UI日志显示（可选）
 * - 多种日志级别
 * - 线程安全
 * - 自动日志轮转
 */
class LogManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param scrollArea 可选的UI滚动区域，用于显示日志
     * @param parent 父对象
     */
    explicit LogManager(QScrollArea* scrollArea = nullptr, QObject* parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~LogManager();

    /**
     * @brief 设置UI显示组件
     * @param scrollArea 滚动区域
     */
    void setUIComponent(QScrollArea* scrollArea);

    /**
     * @brief 设置日志级别
     * @param level 最低记录级别
     */
    void setLogLevel(LogLevel level);

    /**
     * @brief 设置是否启用文件日志
     * @param enabled 是否启用
     */
    void setFileLoggingEnabled(bool enabled);

    /**
     * @brief 设置是否启用UI日志显示
     * @param enabled 是否启用
     */
    void setUILoggingEnabled(bool enabled);

    /**
     * @brief 记录相机操作日志
     * @param operation 操作描述
     * @param cameraSN 相机序列号
     * @param details 详细信息
     */
    void logCameraOperation(const QString& operation, const QString& cameraSN = "", const QString& details = "");

    /**
     * @brief 记录参数修改日志
     * @param paramName 参数名称
     * @param oldValue 旧值
     * @param newValue 新值
     * @param cameraSN 相机序列号（可选）
     */
    void logParameterChange(const QString& paramName, const QString& oldValue, const QString& newValue, const QString& cameraSN = "");

    /**
     * @brief 记录UI操作日志
     * @param operation 操作描述
     * @param details 详细信息
     */
    void logUIOperation(const QString& operation, const QString& details = "");

    /**
     * @brief 记录绘图操作日志
     * @param operation 操作描述
     * @param details 详细信息
     */
    void logDrawingOperation(const QString& operation, const QString& details = "");

    /**
     * @brief 记录测量操作日志
     * @param operation 操作描述
     * @param details 详细信息
     */
    void logMeasurementOperation(const QString& operation, const QString& details = "");

    /**
     * @brief 记录设置操作日志
     * @param operation 操作描述
     * @param details 详细信息
     */
    void logSettingsOperation(const QString& operation, const QString& details = "");

    /**
     * @brief 记录错误日志
     * @param errorMsg 错误消息
     * @param details 详细信息
     */
    void logError(const QString& errorMsg, const QString& details = "");

    /**
     * @brief 记录警告日志
     * @param warningMsg 警告消息
     * @param details 详细信息
     */
    void logWarning(const QString& warningMsg, const QString& details = "");

    /**
     * @brief 记录调试日志
     * @param debugMsg 调试消息
     * @param details 详细信息
     */
    void logDebug(const QString& debugMsg, const QString& details = "");

    /**
     * @brief 通用日志记录方法
     * @param message 日志消息
     * @param level 日志级别
     */
    void log(const QString& message, LogLevel level = LogLevel::INFO);

    /**
     * @brief 清空UI日志显示
     */
    void clearUILog();

    /**
     * @brief 获取当前日志文件路径
     * @return 日志文件路径
     */
    QString getCurrentLogFile() const;

private slots:
    /**
     * @brief 检查日志文件轮转
     */
    void checkLogRotation();

private:
    /**
     * @brief 初始化日志系统
     */
    void initializeLogging();

    /**
     * @brief 创建日志目录
     */
    void createLogDirectory();

    /**
     * @brief 获取日志级别字符串
     * @param level 日志级别
     * @return 级别字符串
     */
    QString getLevelString(LogLevel level) const;

    /**
     * @brief 格式化日志消息
     * @param message 原始消息
     * @param level 日志级别
     * @return 格式化后的消息
     */
    QString formatLogMessage(const QString& message, LogLevel level) const;

    /**
     * @brief 写入文件日志
     * @param message 日志消息
     */
    void writeToFile(const QString& message);

    /**
     * @brief 更新UI日志显示
     * @param message 日志消息
     */
    void updateUILog(const QString& message);

    /**
     * @brief 检查是否需要创建新的日志文件
     */
    void checkNewLogFile();

private:
    // UI组件
    QScrollArea* m_scrollArea;
    QTextEdit* m_textEdit;

    // 日志配置
    LogLevel m_logLevel;
    bool m_fileLoggingEnabled;
    bool m_uiLoggingEnabled;

    // 文件相关
    QString m_logDirectory;
    QString m_currentLogFile;
    QString m_currentDate;
    QFile* m_logFile;
    QTextStream* m_logStream;

    // 线程安全
    mutable QMutex m_mutex;

    // 定时器（用于日志轮转检查）
    QTimer* m_rotationTimer;

    // 常量
    static const QString LOG_DIR_NAME;
    static const QString LOG_FILE_EXTENSION;
    static const int MAX_UI_LOG_LINES;
};
