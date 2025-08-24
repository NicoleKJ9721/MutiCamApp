#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDir>
#include <QStandardPaths>
#include <opencv2/opencv.hpp>
#include <memory>
#include <vector>
#include <map>

#include "MatchingController.h"

/**
 * @brief 模板匹配管理器
 * 
 * 负责管理多个模板的创建、存储、加载和匹配功能
 * 作为MatchingController的高级封装，提供Qt友好的接口
 */
class TemplateMatchingManager : public QObject
{
    Q_OBJECT

public:
    struct TemplateInfo {
        QString name;           // 模板名称
        QString filePath;       // 模板文件路径
        cv::Rect originalROI;   // 原始ROI区域
        float rotationAngle;    // 旋转角度
        bool isActive;          // 是否激活
        
        TemplateInfo() : rotationAngle(0.0f), isActive(false) {}
    };

    explicit TemplateMatchingManager(QObject *parent = nullptr);
    ~TemplateMatchingManager();

    // 初始化
    bool initialize();
    
    // 模板管理
    bool createTemplate(const QString& templateName, const cv::Mat& sourceImage, 
                       const cv::Rect& roi, float rotationAngle = 0.0f);
    bool loadTemplate(const QString& templateName);
    bool deleteTemplate(const QString& templateName);
    QStringList getTemplateList() const;
    TemplateInfo getTemplateInfo(const QString& templateName) const;
    
    // 匹配功能
    bool setActiveTemplate(const QString& templateName);
    QString getActiveTemplate() const;
    std::vector<MatchResult> processFrame(const cv::Mat& frame);
    
    // ROI设置
    void setMatchingROI(const cv::Rect& roi);
    cv::Rect getMatchingROI() const;
    
    // 参数设置
    void setMatchingParameter(const QString& paramName, float value);
    void setMatchingParameter(const QString& paramName, int value);
    void setMatchingParameter(const QString& paramName, const QString& value);
    
    // 配置管理
    bool saveConfiguration();
    bool loadConfiguration();

signals:
    void templateCreated(const QString& templateName);
    void templateLoaded(const QString& templateName);
    void templateDeleted(const QString& templateName);
    void matchingStarted(const QString& templateName);
    void matchingStopped();
    void matchingResults(const std::vector<MatchResult>& results);

private slots:
    void onTemplateCreationFinished();

private:
    // 内部方法
    QString getTemplatesDirectory() const;
    QString getConfigFilePath() const;
    QString getTemplateConfigPath(const QString& templateName) const;
    bool saveTemplateInfo(const QString& templateName, const TemplateInfo& info);
    TemplateInfo loadTemplateInfo(const QString& templateName) const;
    void updateTemplatesList();
    void createDefaultConfig(const QString& configPath);

private:
    std::unique_ptr<MatchingController> m_controller;
    std::map<QString, TemplateInfo> m_templates;
    QString m_activeTemplate;
    QString m_templatesDir;
    QString m_configFile;
    cv::Rect m_matchingROI;
    bool m_initialized;
};
