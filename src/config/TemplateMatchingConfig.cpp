#include "TemplateMatchingConfig.h"
#include <QFile>
#include <QJsonArray>
#include <QDebug>
#include <QDir>
#include <QCoreApplication>

TemplateMatchingConfig* TemplateMatchingConfig::s_instance = nullptr;

TemplateMatchingConfig::TemplateMatchingConfig(QObject *parent)
    : QObject(parent)
{
    resetToDefaults();
}

TemplateMatchingConfig* TemplateMatchingConfig::instance()
{
    if (!s_instance) {
        s_instance = new TemplateMatchingConfig();
    }
    return s_instance;
}

bool TemplateMatchingConfig::loadConfig(const QString& configPath)
{
    m_configPath = configPath;
    
    // 处理相对路径
    QString fullPath = configPath;
    if (!QDir::isAbsolutePath(configPath)) {
        fullPath = QDir(QCoreApplication::applicationDirPath()).filePath(configPath);
    }
    
    QFile file(fullPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "无法打开配置文件:" << fullPath;
        qDebug() << "使用默认配置";
        resetToDefaults();
        return false;
    }
    
    QByteArray data = file.readAll();
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "配置文件JSON解析错误:" << error.errorString();
        qDebug() << "使用默认配置";
        resetToDefaults();
        return false;
    }
    
    fromJsonObject(doc.object());
    qDebug() << "配置文件加载成功:" << fullPath;
    return true;
}

bool TemplateMatchingConfig::saveConfig(const QString& configPath)
{
    QString savePath = configPath.isEmpty() ? m_configPath : configPath;
    
    // 处理相对路径
    QString fullPath = savePath;
    if (!QDir::isAbsolutePath(savePath)) {
        fullPath = QDir(QCoreApplication::applicationDirPath()).filePath(savePath);
    }
    
    // 确保目录存在
    QDir dir = QFileInfo(fullPath).dir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    QFile file(fullPath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "无法写入配置文件:" << fullPath;
        return false;
    }
    
    QJsonDocument doc(toJsonObject());
    file.write(doc.toJson());
    
    qDebug() << "配置文件保存成功:" << fullPath;
    emit configChanged();
    return true;
}

TemplateMatchingConfig::TemplateCreationParams TemplateMatchingConfig::getTemplateCreationParams() const
{
    return m_creationParams;
}

TemplateMatchingConfig::TemplateMatchingParams TemplateMatchingConfig::getTemplateMatchingParams() const
{
    return m_matchingParams;
}

TemplateMatchingConfig::UIDefaultParams TemplateMatchingConfig::getUIDefaultParams() const
{
    return m_uiParams;
}

void TemplateMatchingConfig::setTemplateCreationParams(const TemplateCreationParams& params)
{
    m_creationParams = params;
}

void TemplateMatchingConfig::setTemplateMatchingParams(const TemplateMatchingParams& params)
{
    m_matchingParams = params;
}

void TemplateMatchingConfig::setUIDefaultParams(const UIDefaultParams& params)
{
    m_uiParams = params;
}

void TemplateMatchingConfig::updateTemplateCreationFromJson(const QJsonObject& jsonConfig)
{
    // Halcon官方参数
    m_creationParams.numLevels = jsonConfig["num_levels"].toInt(m_creationParams.numLevels);
    m_creationParams.angleStart = jsonConfig["angle_start"].toDouble(m_creationParams.angleStart);
    m_creationParams.angleExtent = jsonConfig["angle_extent"].toDouble(m_creationParams.angleExtent);
    m_creationParams.angleStep = jsonConfig["angle_step"].toDouble(m_creationParams.angleStep);
    m_creationParams.scaleMin = jsonConfig["scale_min"].toDouble(m_creationParams.scaleMin);
    m_creationParams.scaleMax = jsonConfig["scale_max"].toDouble(m_creationParams.scaleMax);
    m_creationParams.scaleStep = jsonConfig["scale_step"].toDouble(m_creationParams.scaleStep);
    m_creationParams.optimization = jsonConfig["optimization"].toString(m_creationParams.optimization);
    m_creationParams.metric = jsonConfig["metric"].toString(m_creationParams.metric);
    m_creationParams.contrast = jsonConfig["contrast"].toString(m_creationParams.contrast);
    m_creationParams.minContrast = jsonConfig["min_contrast"].toString(m_creationParams.minContrast);
}

QJsonObject TemplateMatchingConfig::templateCreationToJson() const
{
    QJsonObject config;
    
    // Halcon官方参数
    config["num_levels"] = m_creationParams.numLevels;
    config["angle_start"] = m_creationParams.angleStart;
    config["angle_extent"] = m_creationParams.angleExtent;
    config["angle_step"] = m_creationParams.angleStep;
    config["scale_min"] = m_creationParams.scaleMin;
    config["scale_max"] = m_creationParams.scaleMax;
    config["scale_step"] = m_creationParams.scaleStep;
    config["optimization"] = m_creationParams.optimization;
    config["metric"] = m_creationParams.metric;
    config["contrast"] = m_creationParams.contrast;
    config["min_contrast"] = m_creationParams.minContrast;
    
    return config;
}

void TemplateMatchingConfig::resetToDefaults()
{
    // 重置为默认值
    m_creationParams = TemplateCreationParams{};
    m_matchingParams = TemplateMatchingParams{};
    m_uiParams = UIDefaultParams{};
}

QJsonObject TemplateMatchingConfig::toJsonObject() const
{
    QJsonObject root;
    
    // 模板创建参数 - Halcon官方命名
    QJsonObject creation;
    creation["num_levels"] = m_creationParams.numLevels;
    creation["angle_start"] = m_creationParams.angleStart;
    creation["angle_extent"] = m_creationParams.angleExtent;
    creation["angle_step"] = m_creationParams.angleStep;
    creation["scale_min"] = m_creationParams.scaleMin;
    creation["scale_max"] = m_creationParams.scaleMax;
    creation["scale_step"] = m_creationParams.scaleStep;
    creation["optimization"] = m_creationParams.optimization;
    creation["metric"] = m_creationParams.metric;
    creation["contrast"] = m_creationParams.contrast;
    creation["min_contrast"] = m_creationParams.minContrast;
    root["template_creation"] = creation;
    
    // 模板匹配参数 - Halcon官方命名
    QJsonObject matching;
    matching["angle_start"] = m_matchingParams.angleStart;
    matching["angle_extent"] = m_matchingParams.angleExtent;
    matching["scale_min"] = m_matchingParams.scaleMin;
    matching["scale_max"] = m_matchingParams.scaleMax;
    matching["min_score"] = m_matchingParams.minScore;
    matching["num_matches"] = m_matchingParams.numMatches;
    matching["max_overlap"] = m_matchingParams.maxOverlap;
    matching["sub_pixel"] = m_matchingParams.subPixel;
    matching["num_levels"] = m_matchingParams.numLevels;
    matching["greediness"] = m_matchingParams.greediness;
    root["template_matching"] = matching;
    
    // UI默认参数
    QJsonObject uiDefaults;
    uiDefaults["confidence_threshold"] = m_uiParams.confidenceThreshold;
    uiDefaults["max_matches"] = m_uiParams.maxMatches;
    uiDefaults["enable_rotation"] = m_uiParams.enableRotation;
    uiDefaults["rotation_range"] = m_uiParams.rotationRange;
    uiDefaults["enable_scaling"] = m_uiParams.enableScaling;
    uiDefaults["scale_range"] = m_uiParams.scaleRange;
    root["ui_defaults"] = uiDefaults;
    
    return root;
}

void TemplateMatchingConfig::fromJsonObject(const QJsonObject& obj)
{
    // 模板创建参数 - Halcon官方命名
    if (obj.contains("template_creation") && obj["template_creation"].isObject()) {
        QJsonObject creation = obj["template_creation"].toObject();
        
        m_creationParams.numLevels = creation["num_levels"].toInt(5);
        m_creationParams.angleStart = creation["angle_start"].toDouble(0.0);
        m_creationParams.angleExtent = creation["angle_extent"].toDouble(360.0);
        m_creationParams.angleStep = creation["angle_step"].toDouble(1.0);
        m_creationParams.scaleMin = creation["scale_min"].toDouble(0.8);
        m_creationParams.scaleMax = creation["scale_max"].toDouble(1.2);
        m_creationParams.scaleStep = creation["scale_step"].toDouble(0.01);
        m_creationParams.optimization = creation["optimization"].toString("none");
        m_creationParams.metric = creation["metric"].toString("use_polarity");
        m_creationParams.contrast = creation["contrast"].toString("auto");
        m_creationParams.minContrast = creation["min_contrast"].toString("auto");
    }
    
    // 模板匹配参数 - Halcon官方命名
    if (obj.contains("template_matching") && obj["template_matching"].isObject()) {
        QJsonObject matching = obj["template_matching"].toObject();
        
        m_matchingParams.angleStart = matching["angle_start"].toDouble(0.0);
        m_matchingParams.angleExtent = matching["angle_extent"].toDouble(360.0);
        m_matchingParams.scaleMin = matching["scale_min"].toDouble(0.9);
        m_matchingParams.scaleMax = matching["scale_max"].toDouble(1.1);
        m_matchingParams.minScore = matching["min_score"].toDouble(0.5);
        m_matchingParams.numMatches = matching["num_matches"].toInt(1);
        m_matchingParams.maxOverlap = matching["max_overlap"].toDouble(0.5);
        m_matchingParams.subPixel = matching["sub_pixel"].toString("least_squares");
        m_matchingParams.numLevels = matching["num_levels"].toInt(0);
        m_matchingParams.greediness = matching["greediness"].toDouble(0.8);
    }
    
    // UI默认参数
    if (obj.contains("ui_defaults") && obj["ui_defaults"].isObject()) {
        QJsonObject uiDefaults = obj["ui_defaults"].toObject();
        m_uiParams.confidenceThreshold = uiDefaults["confidence_threshold"].toDouble(0.5);
        m_uiParams.maxMatches = uiDefaults["max_matches"].toInt(1);
        m_uiParams.enableRotation = uiDefaults["enable_rotation"].toBool(true);
        m_uiParams.rotationRange = uiDefaults["rotation_range"].toDouble(360.0);
        m_uiParams.enableScaling = uiDefaults["enable_scaling"].toBool(true);
        m_uiParams.scaleRange = uiDefaults["scale_range"].toDouble(0.2);
    }
}
