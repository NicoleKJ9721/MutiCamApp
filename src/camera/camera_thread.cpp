#include "camera_thread.h"
#include <QDebug>
#include <QMutexLocker>
#include <chrono>
#include <thread>

namespace MutiCam {
namespace Camera {

CameraThread::CameraThread(std::shared_ptr<ICameraController> camera, QObject* parent)
    : QThread(parent)
    , m_camera(camera)
    , m_capturing(false)
    , m_paused(false)
    , m_stopRequested(false)
    , m_totalFrameCount(0)
{
    if (!m_camera) {
        qWarning() << "CameraThread: Invalid camera instance";
        return;
    }
    
    // 连接相机信号
    connect(m_camera.get(), &ICameraController::frameReady,
            this, &CameraThread::onCameraFrameReady, Qt::DirectConnection);
    connect(m_camera.get(), &ICameraController::errorOccurred,
            this, &CameraThread::onCameraError, Qt::QueuedConnection);
    
    qDebug() << "CameraThread created for camera";
}

CameraThread::~CameraThread() {
    qDebug() << "CameraThread destroying...";
    
    // 停止采集
    stopCapture();
    
    // 等待线程结束
    if (isRunning()) {
        quit();
        wait(3000);
        if (isRunning()) {
            terminate();
            wait(1000);
        }
    }
    
    qDebug() << "CameraThread destroyed";
}

void CameraThread::startCapture() {
    QMutexLocker locker(&m_mutex);
    
    if (m_capturing.load()) {
        qDebug() << "Camera thread is already capturing";
        return;
    }
    
    if (!m_camera) {
        emit errorOccurred("Invalid camera instance");
        return;
    }
    
    qDebug() << "Starting camera thread capture...";
    
    // 重置状态
    m_stopRequested = false;
    m_paused = false;
    m_capturing = true;
    
    // 启动线程
    if (!isRunning()) {
        start();
    }
    
    emit captureStateChanged(true);
    qDebug() << "Camera thread capture started";
}

void CameraThread::stopCapture() {
    QMutexLocker locker(&m_mutex);
    
    if (!m_capturing.load()) {
        qDebug() << "Camera thread is not capturing";
        return;
    }
    
    qDebug() << "Stopping camera thread capture...";
    
    // 设置停止标志
    m_stopRequested = true;
    m_capturing = false;
    
    // 唤醒可能暂停的线程
    m_pauseCondition.wakeAll();
    
    emit captureStateChanged(false);
    qDebug() << "Camera thread capture stopped";
}

void CameraThread::pauseCapture() {
    QMutexLocker locker(&m_mutex);
    
    if (!m_capturing.load()) {
        qDebug() << "Camera thread is not capturing, cannot pause";
        return;
    }
    
    qDebug() << "Pausing camera thread capture...";
    m_paused = true;
}

void CameraThread::resumeCapture() {
    QMutexLocker locker(&m_mutex);
    
    if (!m_capturing.load()) {
        qDebug() << "Camera thread is not capturing, cannot resume";
        return;
    }
    
    if (!m_paused.load()) {
        qDebug() << "Camera thread is not paused";
        return;
    }
    
    qDebug() << "Resuming camera thread capture...";
    m_paused = false;
    m_pauseCondition.wakeAll();
}

bool CameraThread::isCapturing() const {
    return m_capturing.load();
}

uint64_t CameraThread::getTotalFrameCount() const {
    return m_totalFrameCount.load();
}

void CameraThread::run() {
    qDebug() << "Camera thread started";
    
    while (!m_stopRequested.load()) {
        // 检查暂停状态
        if (m_paused.load()) {
            QMutexLocker locker(&m_mutex);
            m_pauseCondition.wait(&m_mutex);
            continue;
        }
        
        // 线程主要工作是等待相机回调
        // 实际的图像处理在onCameraFrameReady中进行
        msleep(1); // 短暂休眠，避免CPU占用过高
    }
    
    qDebug() << "Camera thread finished";
}

void CameraThread::onCameraFrameReady(const cv::Mat& frame) {
    // 检查采集状态
    if (!m_capturing.load() || m_paused.load()) {
        return;
    }

    // 更新帧计数
    m_totalFrameCount++;

    static int threadFrameCount = 0;
    threadFrameCount++;
    if (threadFrameCount % 30 == 0) { // 每30帧打印一次
        qDebug() << "CameraThread接收到帧，总帧数：" << m_totalFrameCount;
    }

    // 发出帧就绪信号
    emit frameReady(frame);
}

void CameraThread::onCameraError(const QString& error) {
    qWarning() << "Camera thread received error:" << error;
    emit errorOccurred(error);
}



} // namespace Camera
} // namespace MutiCam