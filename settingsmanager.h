#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QSettings>
#include <QMap>
#include <QVariant>

class SettingsManager : public QObject
{
    Q_OBJECT
public:
    explicit SettingsManager(QObject *parent = nullptr);
    ~SettingsManager();

    void loadSettings();
    void saveSettings(const QMap<QString, QVariant>& allSettings);

private:
    QString m_settingsFile;
    QSettings* m_settings;
    
    // 存储所有设置的映射
    QMap<QString, QVariant> m_allSettings;
    
signals:
    // 当设置加载完成时发出信号，传递所有设置
    void settingsLoaded(const QMap<QString, QVariant>& allSettings);
};

#endif // SETTINGSMANAGER_H
