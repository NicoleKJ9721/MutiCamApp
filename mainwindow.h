#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QVariant>
#include <QLineEdit>
#include <QResizeEvent>
#include <QTimer>
#include <QMouseEvent>
#include <QLabel>
#include <opencv2/opencv.hpp>
#include "settingsmanager.h"
#include "cameramanager.h"
#include "drawingmanager.h"

// 注册QObject*类型到元对象系统
Q_DECLARE_METATYPE(QObject*)

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    // 重写窗口大小改变事件
    void resizeEvent(QResizeEvent *event) override;
    
    // 重写事件过滤器
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    // 设置相关槽函数
    void updateUI(const QMap<QString, QVariant>& allSettings);
    void saveSettings();
    void handleEditingFinished();
    void updateWindowSize();
    
    // 相机相关槽函数
    void on_btnStartMeasure_clicked();
    void on_btnStopMeasure_clicked();
    void handleCameraConnectionChanged(CameraPosition position, bool isConnected);
    void handleCameraGrabbingChanged(CameraPosition position, bool isGrabbing);
    void handleLogMessage(const QString& message);
    
    // 绘图相关槽函数
    void on_btnDrawPoint_Ver_clicked();
    void on_btnDrawPoint_Left_clicked();
    void on_btnDrawPoint_Front_clicked();
    void on_btnDrawPoint_clicked();

private:
    Ui::MainWindow *ui;
    SettingsManager *m_settingsManager;
    CameraManager *m_cameraManager;
    DrawingManager *m_drawingManager;
    QMap<QString, QVariant> m_currentSettings;
    bool m_isUpdatingUI;
    
    // 从UI控件收集所有设置值
    QMap<QString, QVariant> collectSettingsFromUI();
    
    // 设置所有连接
    void setupConnections();
    
    // 初始化相机管理器
    void initCameraManager();
    
    // 初始化绘图管理器
    void initDrawingManager();
    
    // 连接相机
    void connectCameras();
    
    // 获取视图当前帧
    cv::Mat getCurrentFrameForView(QLabel* label);
    
    // 更新视图
    void updateView(QLabel* label, const cv::Mat& frame);
};

#endif // MAINWINDOW_H
