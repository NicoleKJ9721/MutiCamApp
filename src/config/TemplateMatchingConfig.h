#pragma once

#include <QObject>
#include <QJsonObject>
#include <QJsonDocument>
#include <QString>

/**
 * @brief 模板匹配配置管理类
 * 负责配置文件的读写和参数管理
 */
class TemplateMatchingConfig : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 模板创建参数结构体
     */
    struct TemplateCreationParams {
        // Halcon官方参数命名
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

    /**
     * @brief 模板匹配参数结构体
     */
    struct TemplateMatchingParams {
        // Halcon官方参数命名
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

    /**
     * @brief UI默认参数结构体
     */
    struct UIDefaultParams {
        double confidenceThreshold = 0.5;
        int maxMatches = 1;
        bool enableRotation = true;
        double rotationRange = 360.0;
        bool enableScaling = true;
        double scaleRange = 0.2;
    };

    explicit TemplateMatchingConfig(QObject *parent = nullptr);
    ~TemplateMatchingConfig() = default;

    // 单例模式
    static TemplateMatchingConfig* instance();

    // 配置文件操作
    bool loadConfig(const QString& configPath = "config/template_matching.json");
    bool saveConfig(const QString& configPath = "config/template_matching.json");

    // 参数获取
    TemplateCreationParams getTemplateCreationParams() const;
    TemplateMatchingParams getTemplateMatchingParams() const;
    UIDefaultParams getUIDefaultParams() const;

    // 参数设置
    void setTemplateCreationParams(const TemplateCreationParams& params);
    void setTemplateMatchingParams(const TemplateMatchingParams& params);
    void setUIDefaultParams(const UIDefaultParams& params);

    // 便捷方法：从QJsonObject设置参数（用于弹窗）
    void updateTemplateCreationFromJson(const QJsonObject& jsonConfig);
    QJsonObject templateCreationToJson() const;

signals:
    void configChanged();

private:
    static TemplateMatchingConfig* s_instance;
    
    TemplateCreationParams m_creationParams;
    TemplateMatchingParams m_matchingParams;
    UIDefaultParams m_uiParams;
    
    QString m_configPath;

    // 内部方法
    void resetToDefaults();
    QJsonObject toJsonObject() const;
    void fromJsonObject(const QJsonObject& obj);
};
