#include "settingsmanager.h"
#include <QDir>
#include <QCoreApplication>
#include <QFile>
#include <QDebug>

SettingsManager::SettingsManager(QObject *parent)
    : QObject{parent}
{
    // 创建Settings目录（在应用程序路径下）
    QDir dir(QCoreApplication::applicationDirPath());
    if (!dir.exists("Settings"))
    {
        dir.mkdir("Settings");
    }

    // 设置文件路径
    m_settingsFile = dir.absoluteFilePath("Settings/settings.ini");
    
    // 检查资源文件路径是否正确
    qDebug() << "检查资源是否存在:" << QFile::exists(":/settings.ini");
    
    // 如果设置文件不存在，从资源中复制默认设置
    QFile settingsFile(m_settingsFile);
    if (!settingsFile.exists()) {
        // 注意这里使用的路径，根据你的截图应该是 :/settings.ini
        QFile defaultSettings(":/settings.ini");
        if (defaultSettings.open(QIODevice::ReadOnly)) {
            qDebug() << "成功打开资源文件";
            QByteArray defaultContent = defaultSettings.readAll();
            defaultSettings.close();
            
            if (settingsFile.open(QIODevice::WriteOnly)) {
                qDebug() << "成功创建设置文件" << m_settingsFile;
                settingsFile.write(defaultContent);
                settingsFile.close();
            } else {
                qDebug() << "无法创建设置文件:" << settingsFile.errorString();
            }
        } else {
            qDebug() << "无法打开资源文件:" << defaultSettings.errorString();
        }
    }
    
    // 创建设置对象
    m_settings = new QSettings(m_settingsFile, QSettings::IniFormat);
}

SettingsManager::~SettingsManager()
{
    delete m_settings;
}

void SettingsManager::loadSettings()
{
    QMap<QString, QVariant> allSettings;
    
    // 加载相机参数
    allSettings["Camera/VerCamSN"] = m_settings->value("Camera/VerCamSN", "Vir1");
    allSettings["Camera/LeftCamSN"] = m_settings->value("Camera/LeftCamSN", "Vir2");
    allSettings["Camera/FrontCamSN"] = m_settings->value("Camera/FrontCamSN", "Vir3");
    
    // 加载线条检测参数
    allSettings["LineDetection/CannyLineLow"] = m_settings->value("LineDetection/CannyLineLow", 50);
    allSettings["LineDetection/CannyLineHigh"] = m_settings->value("LineDetection/CannyLineHigh", 150);
    allSettings["LineDetection/LineDetThreshold"] = m_settings->value("LineDetection/LineDetThreshold", 50);
    allSettings["LineDetection/LineDetMinLength"] = m_settings->value("LineDetection/LineDetMinLength", 100);
    allSettings["LineDetection/LineDetMaxGap"] = m_settings->value("LineDetection/LineDetMaxGap", 10);
    
    // 加载圆形检测参数
    allSettings["CircleDetection/CannyCircleLow"] = m_settings->value("CircleDetection/CannyCircleLow", 50);
    allSettings["CircleDetection/CannyCircleHigh"] = m_settings->value("CircleDetection/CannyCircleHigh", 150);
    allSettings["CircleDetection/CircleDetParam2"] = m_settings->value("CircleDetection/CircleDetParam2", 30);
    
    // 加载UI参数
    allSettings["UI/Width"] = m_settings->value("UI/Width", 800);
    allSettings["UI/Height"] = m_settings->value("UI/Height", 600);
    
    // 保存到成员变量
    m_allSettings = allSettings;
    
    // 发出加载完成信号
    emit settingsLoaded(allSettings);
}

void SettingsManager::saveSettings(const QMap<QString, QVariant>& allSettings)
{
    // 保存所有参数
    QMapIterator<QString, QVariant> i(allSettings);
    while (i.hasNext()) {
        i.next();
        m_settings->setValue(i.key(), i.value());
    }
    
    // 确保写入文件
    m_settings->sync();
    
    // 更新成员变量
    m_allSettings = allSettings;
}
