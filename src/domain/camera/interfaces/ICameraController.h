#pragma once

#include <memory>
#include <vector>
#include <functional>
#include "../../common/Enums.h"

// 前向声明
namespace cv { class Mat; }

namespace MutiCamApp {
namespace Domain {

// 前向声明
class CameraParams;
class Frame;

/**
 * @brief 相机观察者接口
 * 用于接收相机状态变化和帧数据的通知
 */
class ICameraObserver {
public:
    virtual ~ICameraObserver() = default;
    
    /**
     * @brief 当接收到新帧时调用
     * @param frame 新的帧数据
     * @param cameraView 相机视图类型
     */
    virtual void OnFrameReceived(std::shared_ptr<Frame> frame, CameraView cameraView) = 0;
    
    /**
     * @brief 当相机状态改变时调用
     * @param status 新的相机状态
     * @param cameraView 相机视图类型
     * @param errorMessage 错误信息（如果有）
     */
    virtual void OnCameraStatusChanged(CameraStatus status, CameraView cameraView, 
                                     const std::string& errorMessage = "") = 0;
    
    /**
     * @brief 当相机参数改变时调用
     * @param params 新的相机参数
     * @param cameraView 相机视图类型
     */
    virtual void OnCameraParametersChanged(const CameraParams& params, CameraView cameraView) = 0;
};

/**
 * @brief 相机控制器接口
 * 定义了相机控制的核心功能
 */
class ICameraController {
public:
    virtual ~ICameraController() = default;
    
    // === 基础控制接口 ===
    
    /**
     * @brief 初始化相机
     * @param params 相机参数
     * @return 是否初始化成功
     */
    virtual bool Initialize(const CameraParams& params) = 0;
    
    /**
     * @brief 启动图像采集
     * @return 是否启动成功
     */
    virtual bool StartCapture() = 0;
    
    /**
     * @brief 停止图像采集
     * @return 是否停止成功
     */
    virtual bool StopCapture() = 0;
    
    /**
     * @brief 关闭相机连接
     * @return 是否关闭成功
     */
    virtual bool Disconnect() = 0;
    
    // === 状态查询接口 ===
    
    /**
     * @brief 获取相机当前状态
     * @return 相机状态
     */
    virtual CameraStatus GetStatus() const = 0;
    
    /**
     * @brief 检查相机是否已连接
     * @return 是否已连接
     */
    virtual bool IsConnected() const = 0;
    
    /**
     * @brief 检查相机是否正在采集
     * @return 是否正在采集
     */
    virtual bool IsCapturing() const = 0;
    
    // === 数据获取接口 ===
    
    /**
     * @brief 获取最新的帧数据
     * @return 最新帧数据，如果没有则返回nullptr
     */
    virtual std::shared_ptr<Frame> GetLatestFrame() = 0;
    
    /**
     * @brief 同步获取一帧数据
     * @param timeoutMs 超时时间（毫秒）
     * @return 帧数据，超时或失败返回nullptr
     */
    virtual std::shared_ptr<Frame> CaptureFrame(int timeoutMs = 1000) = 0;
    
    /**
     * @brief 获取当前帧率
     * @return 实际帧率（fps）
     */
    virtual double GetCurrentFrameRate() const = 0;
    
    // === 参数设置接口 ===
    
    /**
     * @brief 设置相机参数
     * @param params 新的参数
     * @return 是否设置成功
     */
    virtual bool SetParameters(const CameraParams& params) = 0;
    
    /**
     * @brief 获取当前相机参数
     * @return 当前参数
     */
    virtual CameraParams GetParameters() const = 0;
    
    /**
     * @brief 设置曝光时间
     * @param exposureTime 曝光时间（微秒）
     * @return 是否设置成功
     */
    virtual bool SetExposureTime(double exposureTime) = 0;
    
    /**
     * @brief 获取曝光时间
     * @return 当前曝光时间（微秒）
     */
    virtual double GetExposureTime() const = 0;
    
    /**
     * @brief 设置增益
     * @param gain 增益值
     * @return 是否设置成功
     */
    virtual bool SetGain(double gain) = 0;
    
    /**
     * @brief 获取增益
     * @return 当前增益值
     */
    virtual double GetGain() const = 0;
    
    // === 观察者模式接口 ===
    
    /**
     * @brief 注册观察者
     * @param observer 观察者对象
     */
    virtual void RegisterObserver(std::shared_ptr<ICameraObserver> observer) = 0;
    
    /**
     * @brief 注销观察者
     * @param observer 观察者对象
     */
    virtual void UnregisterObserver(std::shared_ptr<ICameraObserver> observer) = 0;
    
    /**
     * @brief 注销所有观察者
     */
    virtual void ClearObservers() = 0;
    
    // === 高级功能接口 ===
    
    /**
     * @brief 触发软件触发
     * @return 是否触发成功
     */
    virtual bool TriggerSoftware() = 0;
    
    /**
     * @brief 保存当前图像
     * @param filename 文件名
     * @return 是否保存成功
     */
    virtual bool SaveCurrentImage(const std::string& filename) = 0;
    
    /**
     * @brief 获取相机设备信息
     * @return 设备信息字符串
     */
    virtual std::string GetDeviceInfo() const = 0;
    
    /**
     * @brief 获取相机序列号
     * @return 序列号
     */
    virtual std::string GetSerialNumber() const = 0;
    
    /**
     * @brief 获取相机型号
     * @return 型号名称
     */
    virtual std::string GetModelName() const = 0;
    
    // === 错误处理接口 ===
    
    /**
     * @brief 获取最后一次错误信息
     * @return 错误信息
     */
    virtual std::string GetLastError() const = 0;
    
    /**
     * @brief 获取最后一次错误代码
     * @return 错误代码
     */
    virtual ErrorCode GetLastErrorCode() const = 0;
    
    /**
     * @brief 清除错误状态
     */
    virtual void ClearError() = 0;
    
    // === 诊断接口 ===
    
    /**
     * @brief 执行相机自检
     * @return 自检结果
     */
    virtual bool SelfDiagnosis() = 0;
    
    /**
     * @brief 获取相机温度（如果支持）
     * @return 温度值（摄氏度），不支持返回-999
     */
    virtual double GetTemperature() const = 0;
    
    /**
     * @brief 重置相机到默认状态
     * @return 是否重置成功
     */
    virtual bool ResetToDefault() = 0;
};

} // namespace Domain
} // namespace MutiCamApp 