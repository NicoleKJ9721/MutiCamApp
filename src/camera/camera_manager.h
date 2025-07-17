#pragma once

#include "camera_controller.h"
#include "camera_thread.h"
#include <QObject>
#include <QMutex>
#include <memory>
#include <map>
#include <vector>
#include <string>

namespace MutiCam {
namespace Camera {

/**
 * @brief 相机管理器类
 * 
 * 负责管理多个相机实例，提供统一的相机控制接口，
 * 包括相机的添加、移除、批量操作等功能。
 */
class CameraManager : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit CameraManager(QObject* parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~CameraManager() override;
    
    /**
     * @brief 添加相机
     * @param cameraId 相机ID
     * @param serialNumber 相机序列号
     * @return 成功返回true，失败返回false
     */
    bool addCamera(const std::string& cameraId, const std::string& serialNumber);
    
    /**
     * @brief 移除相机
     * @param cameraId 相机ID
     * @return 成功返回true，失败返回false
     */
    bool removeCamera(const std::string& cameraId);
    
    /**
     * @brief 获取相机实例
     * @param cameraId 相机ID
     * @return 相机控制器指针，如果不存在返回nullptr
     */
    std::shared_ptr<ICameraController> getCamera(const std::string& cameraId);
    
    /**
     * @brief 获取相机线程实例
     * @param cameraId 相机ID
     * @return 相机线程指针，如果不存在返回nullptr
     */
    std::shared_ptr<CameraThread> getCameraThread(const std::string& cameraId);
    
    /**
     * @brief 获取所有相机ID列表
     * @return 相机ID列表
     */
    std::vector<std::string> getCameraIds() const;
    
    /**
     * @brief 获取相机数量
     * @return 相机数量
     */
    size_t getCameraCount() const;
    
    /**
     * @brief 检查相机是否已连接
     * @param cameraId 相机ID
     * @return 已连接返回true，否则返回false
     */
    bool isCameraConnected(const std::string& cameraId) const;
    
    /**
     * @brief 检查相机是否正在流式传输
     * @param cameraId 相机ID
     * @return 正在流式传输返回true，否则返回false
     */
    bool isCameraStreaming(const std::string& cameraId) const;
    
    /**
     * @brief 启动所有相机
     * @return 全部启动成功返回true，否则返回false
     */
    bool startAllCameras();
    
    /**
     * @brief 停止所有相机
     * @return 全部停止成功返回true，否则返回false
     */
    bool stopAllCameras();
    
    /**
     * @brief 断开所有相机连接
     */
    void disconnectAllCameras();
    
    /**
     * @brief 获取所有相机状态
     * @return 相机ID到状态的映射
     */
    std::map<std::string, CameraState> getAllCameraStates() const;
    
    /**
     * @brief 枚举可用设备
     * @return 设备序列号列表
     */
    static std::vector<std::string> enumerateDevices();
    

    
    /**
     * @brief 获取相机统计信息
     * @param cameraId 相机ID
     * @return 统计信息字符串
     */
    QString getCameraStats(const std::string& cameraId) const;

signals:
    /**
     * @brief 相机添加信号
     * @param cameraId 相机ID
     */
    void cameraAdded(const QString& cameraId);
    
    /**
     * @brief 相机移除信号
     * @param cameraId 相机ID
     */
    void cameraRemoved(const QString& cameraId);
    
    /**
     * @brief 相机状态变化信号
     * @param cameraId 相机ID
     * @param state 新状态
     */
    void cameraStateChanged(const QString& cameraId, CameraState state);
    
    /**
     * @brief 相机错误信号
     * @param cameraId 相机ID
     * @param error 错误信息
     */
    void cameraError(const QString& cameraId, const QString& error);
    
    /**
     * @brief 相机帧就绪信号
     * @param cameraId 相机ID
     * @param frame 图像帧
     */
    void cameraFrameReady(const QString& cameraId, const cv::Mat& frame);
    


private:
    mutable QMutex m_managerMutex;                                      ///< 管理器互斥锁
    std::map<std::string, std::shared_ptr<ICameraController>> m_cameras; ///< 相机实例映射
    std::map<std::string, std::shared_ptr<CameraThread>> m_cameraThreads; ///< 相机线程映射
    
    /**
     * @brief 连接相机线程信号
     * @param cameraId 相机ID
     * @param cameraThread 相机线程
     */
    void connectCameraThreadSignals(const std::string& cameraId, 
                                   std::shared_ptr<CameraThread> cameraThread);
};

} // namespace Camera
} // namespace MutiCam