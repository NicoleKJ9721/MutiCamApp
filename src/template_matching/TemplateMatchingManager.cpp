#include "TemplateMatchingManager.h"
#include <QApplication>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QStandardPaths>

TemplateMatchingManager::TemplateMatchingManager(QObject *parent)
    : QObject(parent)
    , m_controller(nullptr)
    , m_initialized(false)
{
    // 设置模板存储目录
    m_templatesDir = getTemplatesDirectory();
    m_configFile = getConfigFilePath();
    
    // 确保目录存在
    QDir dir;
    if (!dir.exists(m_templatesDir)) {
        dir.mkpath(m_templatesDir);
    }
}

TemplateMatchingManager::~TemplateMatchingManager()
{
    if (m_controller) {
        m_controller.reset();
    }
}

bool TemplateMatchingManager::initialize()
{
    if (m_initialized) {
        return true;
    }

    try {
        // 创建MatchingController实例
        m_controller = std::make_unique<MatchingController>();
        
        // 使用默认配置文件初始化
        QString defaultConfigPath = QString::fromStdString(QApplication::applicationDirPath().toStdString()) + "/template_matching_config.jsonc";
        
        // 如果默认配置不存在，创建一个基础配置
        if (!QFile::exists(defaultConfigPath)) {
            createDefaultConfig(defaultConfigPath);
        }
        
        bool initResult = m_controller->initialize(defaultConfigPath.toStdString());
        if (!initResult) {
            qWarning() << "Failed to initialize MatchingController";
            return false;
        }
        
        // 加载已有模板信息
        loadConfiguration();
        updateTemplatesList();
        
        m_initialized = true;
        qDebug() << "TemplateMatchingManager initialized successfully";
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "Exception during initialization:" << e.what();
        return false;
    }
}

bool TemplateMatchingManager::createTemplate(const QString& templateName, const cv::Mat& sourceImage, 
                                           const cv::Rect& roi, float rotationAngle)
{
    if (!m_initialized || !m_controller) {
        qWarning() << "TemplateMatchingManager not initialized";
        return false;
    }
    
    if (templateName.isEmpty()) {
        qWarning() << "Template name cannot be empty";
        return false;
    }
    
    if (m_templates.find(templateName) != m_templates.end()) {
        qWarning() << "Template with name" << templateName << "already exists";
        return false;
    }
    
    try {
        // 创建模板信息
        TemplateInfo info;
        info.name = templateName;
        info.originalROI = roi;
        info.rotationAngle = rotationAngle;
        info.isActive = false;
        info.filePath = getTemplateConfigPath(templateName);
        
        // 使用MatchingController创建模板
        auto future = m_controller->createTemplateAsync(sourceImage, roi, templateName.toStdString());
        
        // 等待创建完成
        bool success = future.get();
        if (!success) {
            qWarning() << "Failed to create template:" << templateName;
            return false;
        }
        
        // 保存模板信息
        if (!saveTemplateInfo(templateName, info)) {
            qWarning() << "Failed to save template info:" << templateName;
            return false;
        }
        
        // 添加到内存中的模板列表
        m_templates[templateName] = info;
        
        emit templateCreated(templateName);
        qDebug() << "Template created successfully:" << templateName;
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "Exception during template creation:" << e.what();
        return false;
    }
}

bool TemplateMatchingManager::loadTemplate(const QString& templateName)
{
    if (!m_initialized || !m_controller) {
        return false;
    }
    
    auto it = m_templates.find(templateName);
    if (it == m_templates.end()) {
        qWarning() << "Template not found:" << templateName;
        return false;
    }
    
    try {
        // 这里可以添加模板加载逻辑
        // 目前MatchingController在初始化时会加载所有模板
        
        emit templateLoaded(templateName);
        qDebug() << "Template loaded:" << templateName;
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "Exception during template loading:" << e.what();
        return false;
    }
}

bool TemplateMatchingManager::deleteTemplate(const QString& templateName)
{
    auto it = m_templates.find(templateName);
    if (it == m_templates.end()) {
        return false;
    }
    
    try {
        // 删除模板文件
        QString templatePath = getTemplateConfigPath(templateName);
        QFile::remove(templatePath);
        
        // 从内存中移除
        m_templates.erase(it);
        
        // 如果是当前激活的模板，清除激活状态
        if (m_activeTemplate == templateName) {
            m_activeTemplate.clear();
            emit matchingStopped();
        }
        
        emit templateDeleted(templateName);
        qDebug() << "Template deleted:" << templateName;
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "Exception during template deletion:" << e.what();
        return false;
    }
}

QStringList TemplateMatchingManager::getTemplateList() const
{
    QStringList list;
    for (const auto& pair : m_templates) {
        list.append(pair.first);
    }
    return list;
}

TemplateMatchingManager::TemplateInfo TemplateMatchingManager::getTemplateInfo(const QString& templateName) const
{
    auto it = m_templates.find(templateName);
    if (it != m_templates.end()) {
        return it->second;
    }
    return TemplateInfo();
}

bool TemplateMatchingManager::setActiveTemplate(const QString& templateName)
{
    if (templateName.isEmpty()) {
        m_activeTemplate.clear();
        emit matchingStopped();
        return true;
    }
    
    auto it = m_templates.find(templateName);
    if (it == m_templates.end()) {
        qWarning() << "Template not found:" << templateName;
        return false;
    }
    
    m_activeTemplate = templateName;
    emit matchingStarted(templateName);
    qDebug() << "Active template set to:" << templateName;
    return true;
}

QString TemplateMatchingManager::getActiveTemplate() const
{
    return m_activeTemplate;
}

std::vector<MatchResult> TemplateMatchingManager::processFrame(const cv::Mat& frame)
{
    if (!m_initialized || !m_controller || m_activeTemplate.isEmpty()) {
        return std::vector<MatchResult>();
    }
    
    try {
        auto results = m_controller->processSingleFrame(frame);
        
        if (!results.empty()) {
            emit matchingResults(results);
        }
        
        return results;
        
    } catch (const std::exception& e) {
        qCritical() << "Exception during frame processing:" << e.what();
        return std::vector<MatchResult>();
    }
}

void TemplateMatchingManager::setMatchingROI(const cv::Rect& roi)
{
    m_matchingROI = roi;
    if (m_controller) {
        m_controller->setROI(roi);
    }
}

cv::Rect TemplateMatchingManager::getMatchingROI() const
{
    return m_matchingROI;
}

void TemplateMatchingManager::setMatchingParameter(const QString& paramName, float value)
{
    if (m_controller) {
        nlohmann::json jsonValue = value;
        m_controller->setParameter(paramName.toStdString(), jsonValue);
    }
}

void TemplateMatchingManager::setMatchingParameter(const QString& paramName, int value)
{
    if (m_controller) {
        nlohmann::json jsonValue = value;
        m_controller->setParameter(paramName.toStdString(), jsonValue);
    }
}

void TemplateMatchingManager::setMatchingParameter(const QString& paramName, const QString& value)
{
    if (m_controller) {
        nlohmann::json jsonValue = value.toStdString();
        m_controller->setParameter(paramName.toStdString(), jsonValue);
    }
}

bool TemplateMatchingManager::saveConfiguration()
{
    if (!m_controller) {
        return false;
    }
    
    try {
        // 保存MatchingController配置
        bool result = m_controller->saveConfiguration();
        
        // 保存模板管理器配置
        QJsonObject config;
        config["activeTemplate"] = m_activeTemplate;
        
        QJsonObject roiObj;
        roiObj["x"] = m_matchingROI.x;
        roiObj["y"] = m_matchingROI.y;
        roiObj["width"] = m_matchingROI.width;
        roiObj["height"] = m_matchingROI.height;
        config["matchingROI"] = roiObj;
        
        QJsonArray templatesArray;
        for (const auto& pair : m_templates) {
            QJsonObject templateObj;
            templateObj["name"] = pair.second.name;
            templateObj["filePath"] = pair.second.filePath;
            templateObj["rotationAngle"] = pair.second.rotationAngle;
            templateObj["isActive"] = pair.second.isActive;
            
            QJsonObject roiObj;
            roiObj["x"] = pair.second.originalROI.x;
            roiObj["y"] = pair.second.originalROI.y;
            roiObj["width"] = pair.second.originalROI.width;
            roiObj["height"] = pair.second.originalROI.height;
            templateObj["originalROI"] = roiObj;
            
            templatesArray.append(templateObj);
        }
        config["templates"] = templatesArray;
        
        QJsonDocument doc(config);
        QFile file(m_configFile);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(doc.toJson());
            file.close();
            return result;
        }
        
        return false;
        
    } catch (const std::exception& e) {
        qCritical() << "Exception during configuration save:" << e.what();
        return false;
    }
}

bool TemplateMatchingManager::loadConfiguration()
{
    QFile file(m_configFile);
    if (!file.exists()) {
        return true; // 配置文件不存在是正常的
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    try {
        QByteArray data = file.readAll();
        file.close();
        
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject config = doc.object();
        
        m_activeTemplate = config["activeTemplate"].toString();
        
        QJsonObject roiObj = config["matchingROI"].toObject();
        m_matchingROI = cv::Rect(
            roiObj["x"].toInt(),
            roiObj["y"].toInt(),
            roiObj["width"].toInt(),
            roiObj["height"].toInt()
        );
        
        QJsonArray templatesArray = config["templates"].toArray();
        for (const auto& value : templatesArray) {
            QJsonObject templateObj = value.toObject();
            
            TemplateInfo info;
            info.name = templateObj["name"].toString();
            info.filePath = templateObj["filePath"].toString();
            info.rotationAngle = templateObj["rotationAngle"].toDouble();
            info.isActive = templateObj["isActive"].toBool();
            
            QJsonObject roiObj = templateObj["originalROI"].toObject();
            info.originalROI = cv::Rect(
                roiObj["x"].toInt(),
                roiObj["y"].toInt(),
                roiObj["width"].toInt(),
                roiObj["height"].toInt()
            );
            
            m_templates[info.name] = info;
        }
        
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "Exception during configuration load:" << e.what();
        return false;
    }
}

QString TemplateMatchingManager::getTemplatesDirectory() const
{
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return appDataPath + "/templates";
}

QString TemplateMatchingManager::getConfigFilePath() const
{
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return appDataPath + "/template_matching_config.json";
}

QString TemplateMatchingManager::getTemplateConfigPath(const QString& templateName) const
{
    return m_templatesDir + "/" + templateName + ".json";
}

bool TemplateMatchingManager::saveTemplateInfo(const QString& templateName, const TemplateInfo& info)
{
    QJsonObject templateObj;
    templateObj["name"] = info.name;
    templateObj["filePath"] = info.filePath;
    templateObj["rotationAngle"] = info.rotationAngle;
    templateObj["isActive"] = info.isActive;
    
    QJsonObject roiObj;
    roiObj["x"] = info.originalROI.x;
    roiObj["y"] = info.originalROI.y;
    roiObj["width"] = info.originalROI.width;
    roiObj["height"] = info.originalROI.height;
    templateObj["originalROI"] = roiObj;
    
    QJsonDocument doc(templateObj);
    QFile file(getTemplateConfigPath(templateName));
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
        return true;
    }
    
    return false;
}

TemplateMatchingManager::TemplateInfo TemplateMatchingManager::loadTemplateInfo(const QString& templateName) const
{
    TemplateInfo info;
    
    QFile file(getTemplateConfigPath(templateName));
    if (!file.open(QIODevice::ReadOnly)) {
        return info;
    }
    
    try {
        QByteArray data = file.readAll();
        file.close();
        
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject templateObj = doc.object();
        
        info.name = templateObj["name"].toString();
        info.filePath = templateObj["filePath"].toString();
        info.rotationAngle = templateObj["rotationAngle"].toDouble();
        info.isActive = templateObj["isActive"].toBool();
        
        QJsonObject roiObj = templateObj["originalROI"].toObject();
        info.originalROI = cv::Rect(
            roiObj["x"].toInt(),
            roiObj["y"].toInt(),
            roiObj["width"].toInt(),
            roiObj["height"].toInt()
        );
        
    } catch (const std::exception& e) {
        qCritical() << "Exception during template info load:" << e.what();
    }
    
    return info;
}

void TemplateMatchingManager::updateTemplatesList()
{
    // 扫描模板目录，加载所有模板信息
    QDir templatesDir(m_templatesDir);
    QStringList filters;
    filters << "*.json";
    
    QFileInfoList files = templatesDir.entryInfoList(filters, QDir::Files);
    for (const QFileInfo& fileInfo : files) {
        QString templateName = fileInfo.baseName();
        if (m_templates.find(templateName) == m_templates.end()) {
            TemplateInfo info = loadTemplateInfo(templateName);
            if (!info.name.isEmpty()) {
                m_templates[templateName] = info;
            }
        }
    }
}

void TemplateMatchingManager::createDefaultConfig(const QString& configPath)
{
    // 创建默认的MatchingController配置文件
    QJsonObject config;
    
    QJsonObject paths;
    paths["ModelRootPath"] = "templates";
    config["Paths"] = paths;
    
    QJsonObject model;
    model["ClassName"] = "default_template";
    config["Model"] = model;
    
    QJsonObject matching;
    matching["ScoreThreshold"] = 0.8;
    matching["Overlap"] = 0.4;
    matching["MinMagThreshold"] = 30.0;
    matching["Greediness"] = 0.8;
    matching["PyramidLevel"] = 3;
    matching["T"] = 2;
    matching["TopK"] = 10;
    matching["Strategy"] = 0;
    matching["RefinementSearchMode"] = "fixed";
    matching["FixedAngleWindow"] = 25.0;
    matching["ScaleSearchWindow"] = 0.1;
    config["Matching"] = matching;
    
    QJsonObject templateMaking;
    templateMaking["AngleStart"] = -180.0;
    templateMaking["AngleEnd"] = 180.0;
    templateMaking["AngleStep"] = 10.0;
    templateMaking["ScaleStart"] = 0.7;
    templateMaking["ScaleEnd"] = 1.3;
    templateMaking["ScaleStep"] = 0.05;
    templateMaking["NumFeatures"] = 100;
    templateMaking["WeakThreshold"] = 30.0;
    templateMaking["StrongThreshold"] = 60.0;
    config["TemplateMaking"] = templateMaking;
    
    QJsonDocument doc(config);
    QFile file(configPath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

void TemplateMatchingManager::onTemplateCreationFinished()
{
    // 模板创建完成的槽函数
    // 可以在这里添加额外的处理逻辑
}
