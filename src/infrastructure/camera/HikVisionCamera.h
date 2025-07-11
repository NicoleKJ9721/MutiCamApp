#ifndef HIKVISION_CAMERA_H
#define HIKVISION_CAMERA_H

#include <QString>
#include <QMutex>
#include <memory>
#include "MvCameraControl.h"

class HikVisionCamera {
public:
    explicit HikVisionCamera(const QString& serialNumber = "");
    ~HikVisionCamera();

    // 相机控制方法
    bool openCamera();
    bool openCameraBySerialNumber(const QString& serialNumber);
    bool startGrabbing();
    bool stopGrabbing();
    void closeCamera();
    
    // 图像获取
    bool getFrame(unsigned char*& pImageBuf, unsigned int& nImageSize, 
                  unsigned int& nWidth, unsigned int& nHeight);
    void releaseFrame();
    
    // 设备信息
    bool isConnected() const;
    QString getSerialNumber() const;
    bool checkDeviceStatus();
    
    // 相机参数设置
    bool setExposureTime(float exposureTime);
    bool setPixelFormat(const QString& format);
    float getExposureTime();
    QString getPixelFormat();

    // 静态方法
    static bool enumerateDevices();
    static int getDeviceCount();
    static QString getDeviceSerialNumber(int index);
    
private:
    void* m_hCamera;              // 相机句柄
    QString m_serialNumber;       // 相机序列号
    bool m_isOpened;             // 是否已打开
    bool m_isGrabbing;           // 是否正在采集
    unsigned int m_nPayloadSize; // 载荷大小
    
    // 线程安全
    mutable QMutex m_mutex;
    
    // 静态设备列表
    static MV_CC_DEVICE_INFO_LIST s_deviceList;
    static bool s_devicesEnumerated;
    
    // 私有方法
    bool createCamera(int deviceIndex);
    int findDeviceBySerialNumber(const QString& serialNumber);
    void printDeviceInfo();
};

#endif // HIKVISION_CAMERA_H 