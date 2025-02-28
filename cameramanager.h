#ifndef CAMERAMANAGER_H
#define CAMERAMANAGER_H

#include <QObject>
#include <QMap>
#include <QMutex>
#include <QImage>
#include <QLabel>
#include "cmvcamera.h"
#include "grabimgthread.h"

// 相机位置枚举
enum CameraPosition {
    VerticalCamera = 0,
    LeftCamera = 1,
    FrontCamera = 2
};

// 相机信息结构
struct CameraInfo {
    QString serialNumber;
    CMvCamera* camera = nullptr;
    GrabImgThread* grabThread = nullptr;
    bool isConnected = false;
    bool isGrabbing = false;
};

class CameraManager : public QObject
{
    Q_OBJECT
public:
    explicit CameraManager(QObject *parent = nullptr);
    ~CameraManager();

    // 枚举设备
    bool enumDevices();
    
    // 连接相机
    bool connectCamera(CameraPosition position, const QString& serialNumber);
    
    // 断开相机
    bool disconnectCamera(CameraPosition position);
    
    // 断开所有相机
    void disconnectAllCameras();
    
    // 开始采集
    bool startGrabbing(CameraPosition position);
    
    // 停止采集
    bool stopGrabbing(CameraPosition position);
    
    // 开始所有相机采集
    bool startAllGrabbing();
    
    // 停止所有相机采集
    bool stopAllGrabbing();
    
    // 设置显示标签
    void setDisplayLabel(CameraPosition position, QLabel* mainLabel, QLabel* tabLabel);
    
    // 获取设备列表
    MV_CC_DEVICE_INFO_LIST getDeviceList() const { return m_stDevList; }
    
    // 获取相机信息
    CameraInfo getCameraInfo(CameraPosition position) const;
    
    // 检查相机是否已连接
    bool isCameraConnected(CameraPosition position) const;
    
    // 检查相机是否正在采集
    bool isCameraGrabbing(CameraPosition position) const;

signals:
    // 相机连接状态变化信号
    void cameraConnectionChanged(CameraPosition position, bool isConnected);
    
    // 相机采集状态变化信号
    void cameraGrabbingChanged(CameraPosition position, bool isGrabbing);
    
    // 图像就绪信号
    void imageReady(CameraPosition position, QImage image);
    
    // 日志信息信号
    void logMessage(const QString& message);

private slots:
    // 处理图像就绪
    void handleImageReady(QImage image, int cameraId);

private:
    // 设备列表
    MV_CC_DEVICE_INFO_LIST m_stDevList;
    
    // 相机信息映射表
    QMap<CameraPosition, CameraInfo> m_cameras;
    
    // 显示标签映射表
    QMap<CameraPosition, QPair<QLabel*, QLabel*>> m_displayLabels;
    
    // 互斥锁
    QMutex m_mutex;
    
    // 根据序列号查找设备索引
    int findDeviceIndex(const QString& serialNumber);
    
    // 更新相机连接状态
    void updateCameraConnectionStatus(CameraPosition position, bool isConnected);
    
    // 更新相机采集状态
    void updateCameraGrabbingStatus(CameraPosition position, bool isGrabbing);
};

#endif // CAMERAMANAGER_H 