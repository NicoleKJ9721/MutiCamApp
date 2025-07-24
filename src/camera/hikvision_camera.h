#pragma once

#include "camera_controller.h"
#include "MvCameraControl.h"
#include <QTimer>
#include <QMutex>
#include <atomic>

namespace MutiCam {
namespace Camera {

/**
 * @brief 海康威视相机控制器实现
 * 基于海康威视MVS SDK的相机控制器具体实现
 */
class HikvisionCamera : public ICameraController {
    Q_OBJECT
    
public:
    explicit HikvisionCamera(QObject* parent = nullptr);
    ~HikvisionCamera() override;
    
    // 实现ICameraController接口
    bool connect(const std::string& serialNumber) override;
    bool disconnect() override;
    bool startStreaming() override;
    bool stopStreaming() override;
    
    bool setExposureTime(int exposureTime) override;
    bool setGain(int gain) override;
    bool setPixelFormat(const std::string& format) override;

    
    CameraParams getParams() const override;
    CameraState getState() const override;
    std::string getLastError() const override;
    
    cv::Mat getLatestFrame() override;
    
    // 海康特有功能
    static std::vector<std::string> enumerateDevices();
    bool setTriggerMode(bool enabled);
    bool softwareTrigger();
    
private slots:
    void onFrameTimeout();  // 帧超时处理
    
private:
    // 内部方法
    bool openDevice(const std::string& serialNumber);
    bool closeDevice();
    bool initializeCamera();
    void updateCameraParams();
    void setError(const std::string& error);
    
    // 静态回调函数
    static void __stdcall imageCallbackEx(unsigned char* pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser);
    void processFrame(unsigned char* pData, MV_FRAME_OUT_INFO_EX* pFrameInfo);

    // SDK管理
    static bool initializeSDK();
    static void finalizeSDK();
    static bool isSDKInitialized();

    // 静态成员变量
    static std::atomic<bool> s_sdkInitialized;
    static std::atomic<int> s_instanceCount;
    static QMutex s_sdkMutex;
    
    // 成员变量
    void* m_hCamera = nullptr;              // 相机句柄
    std::atomic<bool> m_isStreaming{false}; // 采集状态
    cv::Mat m_latestFrame;                  // 最新帧缓存
    QMutex m_frameMutex;                    // 帧数据保护
    QTimer* m_frameTimer;                   // 帧超时定时器
    
    // 图像转换缓冲区
    unsigned char* m_pConvertBuffer = nullptr;
    unsigned int m_nConvertBufferSize = 0;
    
    // 性能统计
    std::atomic<uint64_t> m_frameCount{0};
    std::chrono::steady_clock::time_point m_lastStatsTime;
};



} // namespace Camera
} // namespace MutiCam