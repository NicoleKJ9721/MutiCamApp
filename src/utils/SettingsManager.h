#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTimer>
#include "../controllers/AxisControllerEnums.h"

/**
 * @brief 设置管理器类
 * 负责应用程序设置的加载、保存和管理
 * 参考Python版本的SettingsManager实现
 */
class SettingsManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 设置数据结构
     */
    struct Settings {
        // 相机参数
        QString verCamSN;
        QString leftCamSN;
        QString frontCamSN;
        
        // 直线查找参数
        int cannyLineLow;
        int cannyLineHigh;
        int lineDetThreshold;
        int lineDetMinLength;
        int lineDetMaxGap;
        
        // 圆查找参数
        int cannyCircleLow;
        int cannyCircleHigh;
        int circleDetParam2;

        // 绘制参数
        int lineCircleThickness;
        int overlayTextSize;

        // 模板匹配参数（Halcon 形状模板）
        struct TemplateCreationParams {
            int numLevels = 5;
            double angleStart = 0.0;
            double angleExtent = 360.0;
            double angleStep = 1.0;
            double scaleMin = 0.8;
            double scaleMax = 1.2;
            double scaleStep = 0.01;
            QString optimization = "none";
            QString metric = "use_polarity";
            QString contrast = "auto";
            QString minContrast = "auto";
        };

        struct TemplateMatchingParams {
            double angleStart = 0.0;
            double angleExtent = 360.0;
            double scaleMin = 0.9;
            double scaleMax = 1.1;
            double minScore = 0.5;
            int numMatches = 1;
            double maxOverlap = 0.5;
            QString subPixel = "least_squares";
            int numLevels = 0;
            double greediness = 0.8;
        };

        TemplateCreationParams templateCreation;
        TemplateMatchingParams templateMatching;
        
        // UI尺寸参数
        int uiWidth;
        int uiHeight;

        // 拍照参数预设
        QString captureFormat;
        QString imageQuality;

        // 载物台运动参数
        double stageDefaultSpeed;
        double stageDefaultAcceleration;
        double stageDefaultDeceleration;
        double stageStepSize;
        double stageSoftLimitPos;
        double stageSoftLimitNeg;
        double stageMaxSpeedLimit;
        double stageMaxAccelerationLimit;

        // 像素标定参数（每个视图独立）
        struct CalibrationData {
            double pixelScale;        // 像素比例 (μm/pixel)
            QString unit;            // 单位 ("μm", "mm", "cm")
            QString calibrationTime; // 标定时间
            double accuracy;         // 标定精度
            QString method;          // 标定方法
            bool isCalibrated;       // 是否已标定

            CalibrationData() :
                pixelScale(1.0),
                unit("μm"),
                calibrationTime(""),
                accuracy(0.0),
                method("单点标定"),
                isCalibrated(false)
            {}
        };

        CalibrationData verticalCalibration;
        CalibrationData leftCalibration;
        CalibrationData frontCalibration;
        
        // 串口配置参数
        QString stageControllerPort;     // 载物台控制器串口
        QString physicalButtonPort;      // 物理按键串口
        int physicalButtonBaudRate;      // 物理按键波特率
        bool autoDetectSerialPorts;      // 是否自动检测串口
        
        /**
         * @brief 构造函数，设置默认值
         */
        Settings() :
            // 相机参数默认值
            verCamSN("Vir21084717"),
            leftCamSN("Vir21128616"),
            frontCamSN("Vir158888"),
            
            // 直线查找参数默认值
            cannyLineLow(50),
            cannyLineHigh(150),
            lineDetThreshold(50),
            lineDetMinLength(100),
            lineDetMaxGap(10),
            
            // 圆查找参数默认值
            cannyCircleLow(100),
            cannyCircleHigh(200),
            circleDetParam2(50),

            // 绘制参数默认值
            lineCircleThickness(2),
            overlayTextSize(12),
            
            // UI尺寸参数默认值
            uiWidth(1100),
            uiHeight(700),

            // 拍照参数预设默认值
            captureFormat("PNG"),
            imageQuality("无损最高质量"),

            // 载物台运动参数默认值
            stageDefaultSpeed(3000.0),
            stageDefaultAcceleration(3000.0),
            stageDefaultDeceleration(3000.0),
            stageStepSize(1.0),
            stageSoftLimitPos(AxisControl::Constants::MAX_POSITION),
            stageSoftLimitNeg(AxisControl::Constants::MIN_POSITION),
            stageMaxSpeedLimit(AxisControl::Constants::MAX_SPEED),
            stageMaxAccelerationLimit(AxisControl::Constants::MAX_ACCELERATION),
            
            // 串口配置默认值
            stageControllerPort("COM1"),
            physicalButtonPort("COM5"),
            physicalButtonBaudRate(9600),
            autoDetectSerialPorts(true)
        {}
    };

    /**
     * @brief 构造函数
     * @param settingsFile 设置文件路径
     * @param parent 父对象
     */
    explicit SettingsManager(const QString& settingsFile = QString(), 
                           QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~SettingsManager();

    /**
     * @brief 加载设置到UI
     * @param ui UI对象指针
     * @return 是否加载成功
     */
    bool loadSettingsToUI(QObject* ui);

    /**
     * @brief 从UI保存设置
     * @param ui UI对象指针
     * @return 是否保存成功
     */
    bool saveSettingsFromUI(QObject* ui);

    /**
     * @brief 延迟保存设置（避免频繁保存）
     * @param ui UI对象指针
     */
    void saveSettingsDelayed(QObject* ui);

    /**
     * @brief 获取当前设置
     * @return 设置结构体
     */
    const Settings& getCurrentSettings() const { return m_currentSettings; }

    /**
     * @brief 设置当前设置
     * @param settings 设置结构体
     */
    void setCurrentSettings(const Settings& settings) { m_currentSettings = settings; }
    
    /**
     * @brief 更新设置并保存到文件
     * @param settings 新的设置结构体
     * @return 是否保存成功
     */
    bool updateSettings(const Settings& settings);

    /**
     * @brief 重置为默认设置
     */
    void resetToDefaults();

    /**
     * @brief 获取设置文件路径
     * @return 设置文件路径
     */
    QString getSettingsFilePath() const { return m_settingsFile; }

signals:
    /**
     * @brief 设置加载完成信号
     * @param success 是否成功
     */
    void settingsLoaded(bool success);

private slots:
    /**
     * @brief 延迟保存定时器触发槽函数
     */
    void onSaveTimerTimeout();

private:
    /**
     * @brief 从文件加载设置
     * @return 是否加载成功
     */
    bool loadSettingsFromFile();

    /**
     * @brief 保存设置到文件
     * @return 是否保存成功
     */
    bool saveSettingsToFile();

    /**
     * @brief 创建默认设置文件
     * @return 是否创建成功
     */
    bool createDefaultSettingsFile();

    /**
     * @brief 设置转换为JSON对象
     * @param settings 设置结构体
     * @return JSON对象
     */
    QJsonObject settingsToJson(const Settings& settings) const;

    /**
     * @brief JSON对象转换为设置
     * @param json JSON对象
     * @return 设置结构体
     */
    Settings jsonToSettings(const QJsonObject& json) const;

    /**
     * @brief 验证设置值的有效性
     * @param settings 设置结构体
     * @return 验证后的设置结构体
     */
    Settings validateSettings(const Settings& settings) const;

private:
    QString m_settingsFile;        ///< 设置文件路径
    Settings m_currentSettings;    ///< 当前设置
    Settings m_defaultSettings;    ///< 默认设置

    // 延迟保存相关
    QTimer* m_saveTimer;           ///< 延迟保存定时器
    QObject* m_pendingUI;          ///< 待保存的UI对象指针
};

#endif // SETTINGSMANAGER_H
