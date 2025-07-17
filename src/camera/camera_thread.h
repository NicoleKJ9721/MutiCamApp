#pragma once

#include "camera_controller.h"
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QTimer>
#include <memory>
#include <atomic>
#include <opencv2/opencv.hpp>

namespace MutiCam {
namespace Camera {

/**
 * @brief 相机线程类，负责在独立线程中进行图像采集
 * 
 * 该类继承自QThread，为每个相机提供独立的采集线程，
 * 避免多相机采集时的阻塞问题。
 */
class CameraThread : public QThread {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param camera 相机控制器实例
     * @param parent 父对象
     */
    explicit CameraThread(std::shared_ptr<ICameraController> camera, QObject* parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~CameraThread() override;
    
    /**
     * @brief 开始图像采集
     */
    void startCapture();
    
    /**
     * @brief 停止图像采集
     */
    void stopCapture();
    
    /**
     * @brief 暂停图像采集
     */
    void pauseCapture();
    
    /**
     * @brief 恢复图像采集
     */
    void resumeCapture();
    
    /**
     * @brief 获取采集状态
     * @return true表示正在采集，false表示已停止
     */
    bool isCapturing() const;
    
    /**
     * @brief 获取总帧数
     * @return 总帧数
     */
    uint64_t getTotalFrameCount() const;

signals:
    /**
     * @brief 新帧就绪信号
     * @param frame 图像帧
     */
    void frameReady(const cv::Mat& frame);
    
    /**
     * @brief 采集状态变化信号
     * @param capturing 是否正在采集
     */
    void captureStateChanged(bool capturing);
    

    
    /**
     * @brief 错误发生信号
     * @param error 错误信息
     */
    void errorOccurred(const QString& error);

protected:
    /**
     * @brief 线程运行函数
     */
    void run() override;

private slots:
    /**
     * @brief 处理相机帧就绪信号
     * @param frame 图像帧
     */
    void onCameraFrameReady(const cv::Mat& frame);
    
    /**
     * @brief 处理相机错误信号
     * @param error 错误信息
     */
    void onCameraError(const QString& error);

private:
    std::shared_ptr<ICameraController> m_camera;    ///< 相机控制器
    
    std::atomic<bool> m_capturing;                  ///< 采集状态
    std::atomic<bool> m_paused;                     ///< 暂停状态
    std::atomic<bool> m_stopRequested;              ///< 停止请求
    
    mutable QMutex m_mutex;                         ///< 线程安全锁
    QWaitCondition m_pauseCondition;                ///< 暂停条件变量
    
    // 帧计数
    std::atomic<uint64_t> m_totalFrameCount;        ///< 总帧数
    
    
};

} // namespace Camera
} // namespace MutiCam