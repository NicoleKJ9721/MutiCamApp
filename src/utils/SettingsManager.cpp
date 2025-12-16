#include "SettingsManager.h"
#include <QDir>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QLineEdit>
#include <QSpinBox>
#include <QDebug>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QStringList>
#include <algorithm>

namespace {
QString resolveSettingsFilePath(const QString& providedPath)
{
    if (!providedPath.isEmpty()) {
        return providedPath;
    }
    const QString appDir = QCoreApplication::applicationDirPath();
    QStringList candidates = {
        QDir(appDir).filePath("config/settings.json"),
        QDir(appDir).filePath("../config/settings.json"),
        QDir(appDir).filePath("../../config/settings.json")
    };
    for (const QString& candidate : candidates) {
        if (QFile::exists(candidate)) {
            return candidate;
        }
    }
    return candidates.front();
}
}

SettingsManager::SettingsManager(const QString& settingsFile, QObject *parent)
    : QObject(parent)
    , m_settingsFile(resolveSettingsFilePath(settingsFile))
    , m_defaultSettings()  // 使用默认构造函数初始化
    , m_saveTimer(nullptr)
    , m_pendingUI(nullptr)
{
    // 初始化当前设置为默认设置
    m_currentSettings = m_defaultSettings;

    // 初始化延迟保存定时器
    m_saveTimer = new QTimer(this);
    m_saveTimer->setSingleShot(true);  // 单次触发
    m_saveTimer->setInterval(500);     // 500ms延迟
    connect(m_saveTimer, &QTimer::timeout, this, &SettingsManager::onSaveTimerTimeout);
    
    // 尝试加载设置文件
    if (!loadSettingsFromFile()) {
        qDebug() << "设置文件不存在或加载失败，将创建默认设置文件";
        createDefaultSettingsFile();
    }
    
    qDebug() << "SettingsManager初始化完成，设置文件路径：" << m_settingsFile;
}

SettingsManager::~SettingsManager()
{
    // 析构时可以选择自动保存设置
    // saveSettingsToFile();
}

bool SettingsManager::loadSettingsFromFile()
{
    QFile file(m_settingsFile);
    if (!file.exists()) {
        qDebug() << "设置文件不存在：" << m_settingsFile;
        return false;
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "无法打开设置文件进行读取：" << file.errorString();
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    if (data.isEmpty()) {
        qDebug() << "设置文件为空";
        return false;
    }
    
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "JSON解析错误：" << parseError.errorString();
        return false;
    }
    
    if (!doc.isObject()) {
        qDebug() << "JSON文档不是对象格式";
        return false;
    }
    
    QJsonObject jsonObj = doc.object();
    m_currentSettings = jsonToSettings(jsonObj);
    m_currentSettings = validateSettings(m_currentSettings);
    
    qDebug() << "设置文件加载成功";
    return true;
}

bool SettingsManager::saveSettingsToFile()
{
    // 确保目录存在
    QFileInfo fileInfo(m_settingsFile);
    QDir dir = fileInfo.absoluteDir();
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qDebug() << "无法创建设置目录：" << dir.absolutePath();
            return false;
        }
    }
    
    QJsonObject jsonObj = settingsToJson(m_currentSettings);
    QJsonDocument doc(jsonObj);
    
    QFile file(m_settingsFile);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "无法打开设置文件进行写入：" << file.errorString();
        return false;
    }
    
    qint64 bytesWritten = file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    
    if (bytesWritten == -1) {
        qDebug() << "写入设置文件失败";
        return false;
    }
    
    qDebug() << "设置文件保存成功";
    return true;
}

bool SettingsManager::createDefaultSettingsFile()
{
    m_currentSettings = m_defaultSettings;
    return saveSettingsToFile();
}

QJsonObject SettingsManager::settingsToJson(const Settings& settings) const
{
    QJsonObject json;
    
    // 相机参数
    json["VerCamSN"] = settings.verCamSN;
    json["LeftCamSN"] = settings.leftCamSN;
    json["FrontCamSN"] = settings.frontCamSN;
    
    // 直线查找参数
    json["CannyLineLow"] = settings.cannyLineLow;
    json["CannyLineHigh"] = settings.cannyLineHigh;
    json["LineDetThreshold"] = settings.lineDetThreshold;
    json["LineDetMinLength"] = settings.lineDetMinLength;
    json["LineDetMaxGap"] = settings.lineDetMaxGap;
    
    // 圆查找参数
    json["CannyCircleLow"] = settings.cannyCircleLow;
    json["CannyCircleHigh"] = settings.cannyCircleHigh;
    json["CircleDetParam2"] = settings.circleDetParam2;

    // 模板匹配参数（Halcon 形状模板）
    QJsonObject templateCreation;
    templateCreation["num_levels"] = settings.templateCreation.numLevels;
    templateCreation["angle_start"] = settings.templateCreation.angleStart;
    templateCreation["angle_extent"] = settings.templateCreation.angleExtent;
    templateCreation["angle_step"] = settings.templateCreation.angleStep;
    templateCreation["scale_min"] = settings.templateCreation.scaleMin;
    templateCreation["scale_max"] = settings.templateCreation.scaleMax;
    templateCreation["scale_step"] = settings.templateCreation.scaleStep;
    templateCreation["optimization"] = settings.templateCreation.optimization;
    templateCreation["metric"] = settings.templateCreation.metric;
    templateCreation["contrast"] = settings.templateCreation.contrast;
    templateCreation["min_contrast"] = settings.templateCreation.minContrast;
    json["TemplateCreation"] = templateCreation;

    QJsonObject templateMatching;
    templateMatching["angle_start"] = settings.templateMatching.angleStart;
    templateMatching["angle_extent"] = settings.templateMatching.angleExtent;
    templateMatching["scale_min"] = settings.templateMatching.scaleMin;
    templateMatching["scale_max"] = settings.templateMatching.scaleMax;
    templateMatching["min_score"] = settings.templateMatching.minScore;
    templateMatching["num_matches"] = settings.templateMatching.numMatches;
    templateMatching["max_overlap"] = settings.templateMatching.maxOverlap;
    templateMatching["sub_pixel"] = settings.templateMatching.subPixel;
    templateMatching["num_levels"] = settings.templateMatching.numLevels;
    templateMatching["greediness"] = settings.templateMatching.greediness;
    json["TemplateMatching"] = templateMatching;
    
    // UI尺寸参数
    json["UIWidth"] = settings.uiWidth;
    json["UIHeight"] = settings.uiHeight;

    // 载物台运动参数
    json["StageDefaultSpeed"] = settings.stageDefaultSpeed;
    json["StageDefaultAcceleration"] = settings.stageDefaultAcceleration;
    json["StageDefaultDeceleration"] = settings.stageDefaultDeceleration;
    json["StageStepSize"] = settings.stageStepSize;
    json["StageSoftLimitPos"] = settings.stageSoftLimitPos;
    json["StageSoftLimitNeg"] = settings.stageSoftLimitNeg;
    json["StageMaxSpeedLimit"] = settings.stageMaxSpeedLimit;
    json["StageMaxAccelerationLimit"] = settings.stageMaxAccelerationLimit;
    
    // 串口配置参数
    json["StageControllerPort"] = settings.stageControllerPort;
    json["PhysicalButtonPort"] = settings.physicalButtonPort;
    json["PhysicalButtonBaudRate"] = settings.physicalButtonBaudRate;
    json["AutoDetectSerialPorts"] = settings.autoDetectSerialPorts;

    // 标定参数
    QJsonObject verticalCalib;
    verticalCalib["pixelScale"] = settings.verticalCalibration.pixelScale;
    verticalCalib["unit"] = settings.verticalCalibration.unit;
    verticalCalib["calibrationTime"] = settings.verticalCalibration.calibrationTime;
    verticalCalib["accuracy"] = settings.verticalCalibration.accuracy;
    verticalCalib["method"] = settings.verticalCalibration.method;
    verticalCalib["isCalibrated"] = settings.verticalCalibration.isCalibrated;
    json["VerticalCalibration"] = verticalCalib;

    QJsonObject leftCalib;
    leftCalib["pixelScale"] = settings.leftCalibration.pixelScale;
    leftCalib["unit"] = settings.leftCalibration.unit;
    leftCalib["calibrationTime"] = settings.leftCalibration.calibrationTime;
    leftCalib["accuracy"] = settings.leftCalibration.accuracy;
    leftCalib["method"] = settings.leftCalibration.method;
    leftCalib["isCalibrated"] = settings.leftCalibration.isCalibrated;
    json["LeftCalibration"] = leftCalib;

    QJsonObject frontCalib;
    frontCalib["pixelScale"] = settings.frontCalibration.pixelScale;
    frontCalib["unit"] = settings.frontCalibration.unit;
    frontCalib["calibrationTime"] = settings.frontCalibration.calibrationTime;
    frontCalib["accuracy"] = settings.frontCalibration.accuracy;
    frontCalib["method"] = settings.frontCalibration.method;
    frontCalib["isCalibrated"] = settings.frontCalibration.isCalibrated;
    json["FrontCalibration"] = frontCalib;

    return json;
}

SettingsManager::Settings SettingsManager::jsonToSettings(const QJsonObject& json) const
{
    Settings settings;
    
    // 相机参数
    settings.verCamSN = json.value("VerCamSN").toString(m_defaultSettings.verCamSN);
    settings.leftCamSN = json.value("LeftCamSN").toString(m_defaultSettings.leftCamSN);
    settings.frontCamSN = json.value("FrontCamSN").toString(m_defaultSettings.frontCamSN);
    
    // 直线查找参数
    settings.cannyLineLow = json.value("CannyLineLow").toInt(m_defaultSettings.cannyLineLow);
    settings.cannyLineHigh = json.value("CannyLineHigh").toInt(m_defaultSettings.cannyLineHigh);
    settings.lineDetThreshold = json.value("LineDetThreshold").toInt(m_defaultSettings.lineDetThreshold);
    settings.lineDetMinLength = json.value("LineDetMinLength").toInt(m_defaultSettings.lineDetMinLength);
    settings.lineDetMaxGap = json.value("LineDetMaxGap").toInt(m_defaultSettings.lineDetMaxGap);
    
    // 圆查找参数
    settings.cannyCircleLow = json.value("CannyCircleLow").toInt(m_defaultSettings.cannyCircleLow);
    settings.cannyCircleHigh = json.value("CannyCircleHigh").toInt(m_defaultSettings.cannyCircleHigh);
    settings.circleDetParam2 = json.value("CircleDetParam2").toInt(m_defaultSettings.circleDetParam2);

    // 模板匹配参数（Halcon 形状模板）
    if (json.contains("TemplateCreation") && json.value("TemplateCreation").isObject()) {
        const QJsonObject creation = json.value("TemplateCreation").toObject();
        settings.templateCreation.numLevels = creation.value("num_levels").toInt(settings.templateCreation.numLevels);
        settings.templateCreation.angleStart = creation.value("angle_start").toDouble(settings.templateCreation.angleStart);
        settings.templateCreation.angleExtent = creation.value("angle_extent").toDouble(settings.templateCreation.angleExtent);
        settings.templateCreation.angleStep = creation.value("angle_step").toDouble(settings.templateCreation.angleStep);
        settings.templateCreation.scaleMin = creation.value("scale_min").toDouble(settings.templateCreation.scaleMin);
        settings.templateCreation.scaleMax = creation.value("scale_max").toDouble(settings.templateCreation.scaleMax);
        settings.templateCreation.scaleStep = creation.value("scale_step").toDouble(settings.templateCreation.scaleStep);
        settings.templateCreation.optimization = creation.value("optimization").toString(settings.templateCreation.optimization);
        settings.templateCreation.metric = creation.value("metric").toString(settings.templateCreation.metric);
        settings.templateCreation.contrast = creation.value("contrast").toString(settings.templateCreation.contrast);
        settings.templateCreation.minContrast = creation.value("min_contrast").toString(settings.templateCreation.minContrast);
    }

    if (json.contains("TemplateMatching") && json.value("TemplateMatching").isObject()) {
        const QJsonObject matching = json.value("TemplateMatching").toObject();
        settings.templateMatching.angleStart = matching.value("angle_start").toDouble(settings.templateMatching.angleStart);
        settings.templateMatching.angleExtent = matching.value("angle_extent").toDouble(settings.templateMatching.angleExtent);
        settings.templateMatching.scaleMin = matching.value("scale_min").toDouble(settings.templateMatching.scaleMin);
        settings.templateMatching.scaleMax = matching.value("scale_max").toDouble(settings.templateMatching.scaleMax);
        settings.templateMatching.minScore = matching.value("min_score").toDouble(settings.templateMatching.minScore);
        settings.templateMatching.numMatches = matching.value("num_matches").toInt(settings.templateMatching.numMatches);
        settings.templateMatching.maxOverlap = matching.value("max_overlap").toDouble(settings.templateMatching.maxOverlap);
        settings.templateMatching.subPixel = matching.value("sub_pixel").toString(settings.templateMatching.subPixel);
        settings.templateMatching.numLevels = matching.value("num_levels").toInt(settings.templateMatching.numLevels);
        settings.templateMatching.greediness = matching.value("greediness").toDouble(settings.templateMatching.greediness);
    }
    
    // UI尺寸参数
    settings.uiWidth = json.value("UIWidth").toInt(m_defaultSettings.uiWidth);
    settings.uiHeight = json.value("UIHeight").toInt(m_defaultSettings.uiHeight);
    
    // 串口配置参数
    settings.stageControllerPort = json.value("StageControllerPort").toString(m_defaultSettings.stageControllerPort);
    settings.physicalButtonPort = json.value("PhysicalButtonPort").toString(m_defaultSettings.physicalButtonPort);
    settings.physicalButtonBaudRate = json.value("PhysicalButtonBaudRate").toInt(m_defaultSettings.physicalButtonBaudRate);
    settings.autoDetectSerialPorts = json.value("AutoDetectSerialPorts").toBool(m_defaultSettings.autoDetectSerialPorts);

    // 载物台运动参数
    settings.stageDefaultSpeed = json.value("StageDefaultSpeed").toDouble(m_defaultSettings.stageDefaultSpeed);
    settings.stageDefaultAcceleration = json.value("StageDefaultAcceleration").toDouble(m_defaultSettings.stageDefaultAcceleration);
    settings.stageDefaultDeceleration = json.value("StageDefaultDeceleration").toDouble(m_defaultSettings.stageDefaultDeceleration);
    settings.stageStepSize = json.value("StageStepSize").toDouble(m_defaultSettings.stageStepSize);
    settings.stageSoftLimitPos = json.value("StageSoftLimitPos").toDouble(m_defaultSettings.stageSoftLimitPos);
    settings.stageSoftLimitNeg = json.value("StageSoftLimitNeg").toDouble(m_defaultSettings.stageSoftLimitNeg);
    settings.stageMaxSpeedLimit = json.value("StageMaxSpeedLimit").toDouble(m_defaultSettings.stageMaxSpeedLimit);
    settings.stageMaxAccelerationLimit = json.value("StageMaxAccelerationLimit").toDouble(m_defaultSettings.stageMaxAccelerationLimit);

    // 标定参数
    if (json.contains("VerticalCalibration")) {
        QJsonObject verticalCalib = json.value("VerticalCalibration").toObject();
        settings.verticalCalibration.pixelScale = verticalCalib.value("pixelScale").toDouble(1.0);
        settings.verticalCalibration.unit = verticalCalib.value("unit").toString("μm");
        settings.verticalCalibration.calibrationTime = verticalCalib.value("calibrationTime").toString("");
        settings.verticalCalibration.accuracy = verticalCalib.value("accuracy").toDouble(0.0);
        settings.verticalCalibration.method = verticalCalib.value("method").toString("单点标定");
        settings.verticalCalibration.isCalibrated = verticalCalib.value("isCalibrated").toBool(false);
    }

    if (json.contains("LeftCalibration")) {
        QJsonObject leftCalib = json.value("LeftCalibration").toObject();
        settings.leftCalibration.pixelScale = leftCalib.value("pixelScale").toDouble(1.0);
        settings.leftCalibration.unit = leftCalib.value("unit").toString("μm");
        settings.leftCalibration.calibrationTime = leftCalib.value("calibrationTime").toString("");
        settings.leftCalibration.accuracy = leftCalib.value("accuracy").toDouble(0.0);
        settings.leftCalibration.method = leftCalib.value("method").toString("单点标定");
        settings.leftCalibration.isCalibrated = leftCalib.value("isCalibrated").toBool(false);
    }

    if (json.contains("FrontCalibration")) {
        QJsonObject frontCalib = json.value("FrontCalibration").toObject();
        settings.frontCalibration.pixelScale = frontCalib.value("pixelScale").toDouble(1.0);
        settings.frontCalibration.unit = frontCalib.value("unit").toString("μm");
        settings.frontCalibration.calibrationTime = frontCalib.value("calibrationTime").toString("");
        settings.frontCalibration.accuracy = frontCalib.value("accuracy").toDouble(0.0);
        settings.frontCalibration.method = frontCalib.value("method").toString("单点标定");
        settings.frontCalibration.isCalibrated = frontCalib.value("isCalibrated").toBool(false);
    }

    return settings;
}

SettingsManager::Settings SettingsManager::validateSettings(const Settings& settings) const
{
    Settings validatedSettings = settings;
    
    // 验证Canny参数范围 (0-255)
    validatedSettings.cannyLineLow = qBound(0, settings.cannyLineLow, 255);
    validatedSettings.cannyLineHigh = qBound(0, settings.cannyLineHigh, 255);
    validatedSettings.cannyCircleLow = qBound(0, settings.cannyCircleLow, 255);
    validatedSettings.cannyCircleHigh = qBound(0, settings.cannyCircleHigh, 255);
    
    // 确保高阈值大于低阈值
    if (validatedSettings.cannyLineHigh <= validatedSettings.cannyLineLow) {
        validatedSettings.cannyLineHigh = validatedSettings.cannyLineLow + 50;
    }
    if (validatedSettings.cannyCircleHigh <= validatedSettings.cannyCircleLow) {
        validatedSettings.cannyCircleHigh = validatedSettings.cannyCircleLow + 50;
    }
    
    // 验证其他参数范围
    validatedSettings.lineDetThreshold = qBound(1, settings.lineDetThreshold, 1000);
    validatedSettings.lineDetMinLength = qBound(1, settings.lineDetMinLength, 1000);
    validatedSettings.lineDetMaxGap = qBound(0, settings.lineDetMaxGap, 100);
    validatedSettings.circleDetParam2 = qBound(1, settings.circleDetParam2, 200);
    
    // 验证UI尺寸
    validatedSettings.uiWidth = qBound(1100, settings.uiWidth, 4000);
    validatedSettings.uiHeight = qBound(700, settings.uiHeight, 3000);

    // 验证载物台运动参数
    auto clampDouble = [](double value, double minValue, double maxValue) {
        return std::clamp(value, minValue, maxValue);
    };
    validatedSettings.stageDefaultSpeed = clampDouble(settings.stageDefaultSpeed,
                                                     AxisControl::Constants::MIN_SPEED,
                                                     AxisControl::Constants::MAX_SPEED);
    validatedSettings.stageDefaultAcceleration = clampDouble(settings.stageDefaultAcceleration,
                                                            AxisControl::Constants::MIN_ACCELERATION,
                                                            AxisControl::Constants::MAX_ACCELERATION);
    validatedSettings.stageDefaultDeceleration = clampDouble(settings.stageDefaultDeceleration,
                                                            AxisControl::Constants::MIN_ACCELERATION,
                                                            AxisControl::Constants::MAX_ACCELERATION);
    validatedSettings.stageStepSize = clampDouble(settings.stageStepSize, 0.01, 10000.0);
    validatedSettings.stageSoftLimitPos = clampDouble(settings.stageSoftLimitPos,
                                                      AxisControl::Constants::MIN_POSITION,
                                                      AxisControl::Constants::MAX_POSITION);
    validatedSettings.stageSoftLimitNeg = clampDouble(settings.stageSoftLimitNeg,
                                                      AxisControl::Constants::MIN_POSITION,
                                                      AxisControl::Constants::MAX_POSITION);
    if (validatedSettings.stageSoftLimitNeg > validatedSettings.stageSoftLimitPos) {
        std::swap(validatedSettings.stageSoftLimitNeg, validatedSettings.stageSoftLimitPos);
    }
    validatedSettings.stageMaxSpeedLimit = clampDouble(settings.stageMaxSpeedLimit,
                                                       AxisControl::Constants::MIN_SPEED,
                                                       AxisControl::Constants::MAX_SPEED);
    validatedSettings.stageMaxAccelerationLimit = clampDouble(settings.stageMaxAccelerationLimit,
                                                              AxisControl::Constants::MIN_ACCELERATION,
                                                              AxisControl::Constants::MAX_ACCELERATION);

    // 验证模板创建/匹配参数
    validatedSettings.templateCreation.numLevels = qBound(0, settings.templateCreation.numLevels, 20);
    validatedSettings.templateCreation.angleExtent = clampDouble(settings.templateCreation.angleExtent, 0.0, 3600.0);
    validatedSettings.templateCreation.angleStep = clampDouble(settings.templateCreation.angleStep, 0.001, 360.0);
    validatedSettings.templateCreation.scaleMin = clampDouble(settings.templateCreation.scaleMin, 0.001, 1000.0);
    validatedSettings.templateCreation.scaleMax = clampDouble(settings.templateCreation.scaleMax, 0.001, 1000.0);
    if (validatedSettings.templateCreation.scaleMax < validatedSettings.templateCreation.scaleMin) {
        std::swap(validatedSettings.templateCreation.scaleMin, validatedSettings.templateCreation.scaleMax);
    }
    validatedSettings.templateCreation.scaleStep = clampDouble(settings.templateCreation.scaleStep, 0.0001, 100.0);

    validatedSettings.templateMatching.angleExtent = clampDouble(settings.templateMatching.angleExtent, 0.0, 3600.0);
    validatedSettings.templateMatching.scaleMin = clampDouble(settings.templateMatching.scaleMin, 0.001, 1000.0);
    validatedSettings.templateMatching.scaleMax = clampDouble(settings.templateMatching.scaleMax, 0.001, 1000.0);
    if (validatedSettings.templateMatching.scaleMax < validatedSettings.templateMatching.scaleMin) {
        std::swap(validatedSettings.templateMatching.scaleMin, validatedSettings.templateMatching.scaleMax);
    }
    validatedSettings.templateMatching.minScore = clampDouble(settings.templateMatching.minScore, 0.0, 1.0);
    validatedSettings.templateMatching.numMatches = qBound(1, settings.templateMatching.numMatches, 1000);
    validatedSettings.templateMatching.maxOverlap = clampDouble(settings.templateMatching.maxOverlap, 0.0, 1.0);
    validatedSettings.templateMatching.numLevels = qBound(0, settings.templateMatching.numLevels, 20);
    validatedSettings.templateMatching.greediness = clampDouble(settings.templateMatching.greediness, 0.0, 1.0);
    
    return validatedSettings;
}

bool SettingsManager::updateSettings(const Settings& settings)
{
    m_currentSettings = validateSettings(settings);
    bool success = saveSettingsToFile();
    
    if (success) {
        qDebug() << "设置已更新并保存";
    } else {
        qDebug() << "设置更新失败";
    }
    
    return success;
}

void SettingsManager::resetToDefaults()
{
    m_currentSettings = m_defaultSettings;
    qDebug() << "设置已重置为默认值";
}

void SettingsManager::saveSettingsDelayed(QObject* ui)
{
    if (!ui) {
        return;
    }

    // 保存UI指针
    m_pendingUI = ui;

    // 重启定时器（如果已经在运行，会重新开始计时）
    m_saveTimer->start();
}

void SettingsManager::onSaveTimerTimeout()
{
    if (m_pendingUI) {
        // 执行实际的保存操作
        saveSettingsFromUI(m_pendingUI);
        m_pendingUI = nullptr;
    }
}

bool SettingsManager::loadSettingsToUI(QObject* ui)
{
    if (!ui) {
        qDebug() << "UI对象为空，无法加载设置";
        return false;
    }

    try {
        // 加载相机参数
        QLineEdit* ledVerCamSN = ui->findChild<QLineEdit*>("ledVerCamSN");
        QLineEdit* ledLeftCamSN = ui->findChild<QLineEdit*>("ledLeftCamSN");
        QLineEdit* ledFrontCamSN = ui->findChild<QLineEdit*>("ledFrontCamSN");

        if (ledVerCamSN) ledVerCamSN->setText(m_currentSettings.verCamSN);
        if (ledLeftCamSN) ledLeftCamSN->setText(m_currentSettings.leftCamSN);
        if (ledFrontCamSN) ledFrontCamSN->setText(m_currentSettings.frontCamSN);

        // 加载直线查找参数
        QLineEdit* ledCannyLineLow = ui->findChild<QLineEdit*>("ledCannyLineLow");
        QLineEdit* ledCannyLineHigh = ui->findChild<QLineEdit*>("ledCannyLineHigh");
        QLineEdit* ledLineDetThreshold = ui->findChild<QLineEdit*>("ledLineDetThreshold");
        QLineEdit* ledLineDetMinLength = ui->findChild<QLineEdit*>("ledLineDetMinLength");
        QLineEdit* ledLineDetMaxGap = ui->findChild<QLineEdit*>("ledLineDetMaxGap");

        if (ledCannyLineLow) ledCannyLineLow->setText(QString::number(m_currentSettings.cannyLineLow));
        if (ledCannyLineHigh) ledCannyLineHigh->setText(QString::number(m_currentSettings.cannyLineHigh));
        if (ledLineDetThreshold) ledLineDetThreshold->setText(QString::number(m_currentSettings.lineDetThreshold));
        if (ledLineDetMinLength) ledLineDetMinLength->setText(QString::number(m_currentSettings.lineDetMinLength));
        if (ledLineDetMaxGap) ledLineDetMaxGap->setText(QString::number(m_currentSettings.lineDetMaxGap));

        // 加载圆查找参数
        QLineEdit* ledCannyCircleLow = ui->findChild<QLineEdit*>("ledCannyCircleLow");
        QLineEdit* ledCannyCircleHigh = ui->findChild<QLineEdit*>("ledCannyCircleHigh");
        QLineEdit* ledCircleDetParam2 = ui->findChild<QLineEdit*>("ledCircleDetParam2");

        if (ledCannyCircleLow) ledCannyCircleLow->setText(QString::number(m_currentSettings.cannyCircleLow));
        if (ledCannyCircleHigh) ledCannyCircleHigh->setText(QString::number(m_currentSettings.cannyCircleHigh));
        if (ledCircleDetParam2) ledCircleDetParam2->setText(QString::number(m_currentSettings.circleDetParam2));

        // 加载UI尺寸参数
        QLineEdit* ledUIWidth = ui->findChild<QLineEdit*>("ledUIWidth");
        QLineEdit* ledUIHeight = ui->findChild<QLineEdit*>("ledUIHeight");

        if (ledUIWidth) ledUIWidth->setText(QString::number(m_currentSettings.uiWidth));
        if (ledUIHeight) ledUIHeight->setText(QString::number(m_currentSettings.uiHeight));

        // 加载运动参数
        QSpinBox* spinSpeed = ui->findChild<QSpinBox*>("spinBoxSpeed");
        QSpinBox* spinAccel = ui->findChild<QSpinBox*>("spinBoxAccel");
        QSpinBox* spinMaxSpeed = ui->findChild<QSpinBox*>("spinBoxMaxSpeed");
        QSpinBox* spinMaxAccel = ui->findChild<QSpinBox*>("spinBoxMaxAccel");

        if (spinSpeed) spinSpeed->setValue(static_cast<int>(m_currentSettings.stageDefaultSpeed));
        if (spinAccel) spinAccel->setValue(static_cast<int>(m_currentSettings.stageDefaultAcceleration));
        if (spinMaxSpeed) {
            spinMaxSpeed->setValue(static_cast<int>(m_currentSettings.stageMaxSpeedLimit));
            spinMaxSpeed->setMaximum(static_cast<int>(AxisControl::Constants::MAX_SPEED));
        }
        if (spinMaxAccel) {
            spinMaxAccel->setValue(static_cast<int>(m_currentSettings.stageMaxAccelerationLimit));
            spinMaxAccel->setMaximum(static_cast<int>(AxisControl::Constants::MAX_ACCELERATION));
        }
        if (spinSpeed && spinMaxSpeed) {
            spinSpeed->setMaximum(spinMaxSpeed->value());
        }
        if (spinAccel && spinMaxAccel) {
            spinAccel->setMaximum(spinMaxAccel->value());
        }

        qDebug() << "设置已加载到UI";
        emit settingsLoaded(true);
        return true;

    } catch (const std::exception& e) {
        qDebug() << "加载设置到UI时发生异常：" << e.what();
        emit settingsLoaded(false);
        return false;
    }
}

bool SettingsManager::saveSettingsFromUI(QObject* ui)
{
    if (!ui) {
        qDebug() << "UI对象为空，无法保存设置";
        return false;
    }

    try {
        Settings newSettings = m_currentSettings;

        // 获取相机参数
        QLineEdit* ledVerCamSN = ui->findChild<QLineEdit*>("ledVerCamSN");
        QLineEdit* ledLeftCamSN = ui->findChild<QLineEdit*>("ledLeftCamSN");
        QLineEdit* ledFrontCamSN = ui->findChild<QLineEdit*>("ledFrontCamSN");

        if (ledVerCamSN) newSettings.verCamSN = ledVerCamSN->text();
        if (ledLeftCamSN) newSettings.leftCamSN = ledLeftCamSN->text();
        if (ledFrontCamSN) newSettings.frontCamSN = ledFrontCamSN->text();

        // 获取直线查找参数
        QLineEdit* ledCannyLineLow = ui->findChild<QLineEdit*>("ledCannyLineLow");
        QLineEdit* ledCannyLineHigh = ui->findChild<QLineEdit*>("ledCannyLineHigh");
        QLineEdit* ledLineDetThreshold = ui->findChild<QLineEdit*>("ledLineDetThreshold");
        QLineEdit* ledLineDetMinLength = ui->findChild<QLineEdit*>("ledLineDetMinLength");
        QLineEdit* ledLineDetMaxGap = ui->findChild<QLineEdit*>("ledLineDetMaxGap");

        if (ledCannyLineLow) newSettings.cannyLineLow = ledCannyLineLow->text().toInt();
        if (ledCannyLineHigh) newSettings.cannyLineHigh = ledCannyLineHigh->text().toInt();
        if (ledLineDetThreshold) newSettings.lineDetThreshold = ledLineDetThreshold->text().toInt();
        if (ledLineDetMinLength) newSettings.lineDetMinLength = ledLineDetMinLength->text().toInt();
        if (ledLineDetMaxGap) newSettings.lineDetMaxGap = ledLineDetMaxGap->text().toInt();

        // 获取圆查找参数
        QLineEdit* ledCannyCircleLow = ui->findChild<QLineEdit*>("ledCannyCircleLow");
        QLineEdit* ledCannyCircleHigh = ui->findChild<QLineEdit*>("ledCannyCircleHigh");
        QLineEdit* ledCircleDetParam2 = ui->findChild<QLineEdit*>("ledCircleDetParam2");

        if (ledCannyCircleLow) newSettings.cannyCircleLow = ledCannyCircleLow->text().toInt();
        if (ledCannyCircleHigh) newSettings.cannyCircleHigh = ledCannyCircleHigh->text().toInt();
        if (ledCircleDetParam2) newSettings.circleDetParam2 = ledCircleDetParam2->text().toInt();

        // 获取UI尺寸参数
        QLineEdit* ledUIWidth = ui->findChild<QLineEdit*>("ledUIWidth");
        QLineEdit* ledUIHeight = ui->findChild<QLineEdit*>("ledUIHeight");

        if (ledUIWidth) newSettings.uiWidth = ledUIWidth->text().toInt();
        if (ledUIHeight) newSettings.uiHeight = ledUIHeight->text().toInt();

        // 获取运动参数
        QSpinBox* spinSpeed = ui->findChild<QSpinBox*>("spinBoxSpeed");
        QSpinBox* spinAccel = ui->findChild<QSpinBox*>("spinBoxAccel");
        QSpinBox* spinMaxSpeed = ui->findChild<QSpinBox*>("spinBoxMaxSpeed");
        QSpinBox* spinMaxAccel = ui->findChild<QSpinBox*>("spinBoxMaxAccel");

        if (spinSpeed) newSettings.stageDefaultSpeed = spinSpeed->value();
        if (spinAccel) {
            newSettings.stageDefaultAcceleration = spinAccel->value();
            newSettings.stageDefaultDeceleration = spinAccel->value();
        }
        if (spinMaxSpeed) newSettings.stageMaxSpeedLimit = spinMaxSpeed->value();
        if (spinMaxAccel) newSettings.stageMaxAccelerationLimit = spinMaxAccel->value();

        // 验证并保存设置
        m_currentSettings = validateSettings(newSettings);
        bool success = saveSettingsToFile();

        if (success) {
            qDebug() << "设置已实时保存到文件";
        } else {
            qDebug() << "设置保存失败";
        }
        return success;

    } catch (const std::exception& e) {
        qDebug() << "从UI保存设置时发生异常：" << e.what();
        return false;
    }
}
