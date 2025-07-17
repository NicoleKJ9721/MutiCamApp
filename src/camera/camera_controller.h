#pragma once

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <opencv2/opencv.hpp>
#include <memory>
#include <string>

namespace MutiCam {
namespace Camera {

/**
 * @brief 相机状态枚举
 */
enum class CameraState {
    Disconnected,   // 未连接
    Connected,      // 已连接
    Streaming,      // 正在采集
    Error          // 错误状态
};

/**
 * @brief 相机参数结构
 */
struct CameraParams {
    std::string serialNumber;    // 相机序列号
    int exposureTime;           // 曝光时间(微秒)
    int gain;                   // 增益
    std::string pixelFormat;    // 像素格式
    int width;                  // 图像宽度
    int height;                 // 图像高度

};

/**
 * @brief 相机控制器接口
 * 提供相机的基本控制功能，包括连接、参数设置、图像采集等
 */
class ICameraController : public QObject {
    Q_OBJECT

public:
    virtual ~ICameraController() = default;
    
    // 基础控制接口
    virtual bool connect(const std::string& serialNumber) = 0;
    virtual bool disconnect() = 0;
    virtual bool startStreaming() = 0;
    virtual bool stopStreaming() = 0;
    
    // 参数设置接口
    virtual bool setExposureTime(int exposureTime) = 0;
    virtual bool setGain(int gain) = 0;
    virtual bool setPixelFormat(const std::string& format) = 0;

    
    // 参数获取接口
    virtual CameraParams getParams() const = 0;
    virtual CameraState getState() const = 0;
    virtual std::string getLastError() const = 0;
    
    // 图像获取接口
    virtual cv::Mat getLatestFrame() = 0;
    
signals:
    void frameReady(const cv::Mat& frame);           // 新帧就绪信号
    void stateChanged(CameraState newState);        // 状态变化信号
    void errorOccurred(const QString& errorMsg);    // 错误发生信号
    
protected:
    mutable QMutex m_mutex;                         // 线程安全保护
    CameraState m_state = CameraState::Disconnected;
    CameraParams m_params;
    std::string m_lastError;
};

// CameraThread类定义在camera_thread.h中
class CameraThread;

} // namespace Camera
} // namespace MutiCam