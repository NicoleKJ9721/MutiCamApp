#include "SettingsManager.h"
#include <QDir>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QLineEdit>
#include <QDebug>
#include <QStandardPaths>
#include <QCoreApplication>

SettingsManager::SettingsManager(const QString& settingsFile, QObject *parent)
    : QObject(parent)
    , m_settingsFile(settingsFile)
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
    
    // UI尺寸参数
    json["UIWidth"] = settings.uiWidth;
    json["UIHeight"] = settings.uiHeight;

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
    
    // UI尺寸参数
    settings.uiWidth = json.value("UIWidth").toInt(m_defaultSettings.uiWidth);
    settings.uiHeight = json.value("UIHeight").toInt(m_defaultSettings.uiHeight);

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
    validatedSettings.uiWidth = qBound(800, settings.uiWidth, 4000);
    validatedSettings.uiHeight = qBound(600, settings.uiHeight, 3000);
    
    return validatedSettings;
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
        Settings newSettings;

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
