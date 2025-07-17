#include "camera_manager.h"
#include "hikvision_camera.h"
#include "camera_thread.h"
#include <QMutex>
#include <QMutexLocker>
#include <QDebug>

namespace MutiCam {
namespace Camera {

/**
 * @brief CameraManager实现
 */
CameraManager::CameraManager(QObject* parent) : QObject(parent) {
    qDebug() << "CameraManager initialized";
}

CameraManager::~CameraManager() {
    qDebug() << "CameraManager destroying...";
    
    // 首先停止所有相机采集
    stopAllCameras();
    
    // 停止并等待所有相机线程结束
    for (auto& pair : m_cameraThreads) {
        if (pair.second) {
            qDebug() << "Stopping camera thread:" << QString::fromStdString(pair.first);
            
            // 停止采集
            pair.second->stopCapture();
            
            // 退出线程
            pair.second->quit();
            
            // 等待线程结束，最多等待5秒
            if (!pair.second->wait(5000)) {
                qWarning() << "Camera thread" << QString::fromStdString(pair.first) << "did not finish within timeout, terminating...";
                pair.second->terminate();
                pair.second->wait(1000); // 再等待1秒
            }
            
            qDebug() << "Camera thread stopped:" << QString::fromStdString(pair.first);
        }
    }
    
    // 断开所有相机连接
    disconnectAllCameras();
    
    // 清理资源
    m_cameraThreads.clear();
    m_cameras.clear();
    
    qDebug() << "CameraManager destroyed";
}

bool CameraManager::addCamera(const std::string& cameraId, const std::string& serialNumber) {
    QMutexLocker locker(&m_managerMutex);
    
    // 检查相机ID是否已存在
    if (m_cameras.find(cameraId) != m_cameras.end()) {
        qDebug() << "Camera ID already exists:" << QString::fromStdString(cameraId);
        return false;
    }
    
    // 创建海康相机实例
    auto camera = std::make_shared<HikvisionCamera>(this);
    
    // 连接相机
    if (!camera->connect(serialNumber)) {
        qDebug() << "Failed to connect camera:" << QString::fromStdString(serialNumber);
        return false;
    }
    
    // 创建相机线程
    auto cameraThread = std::make_shared<CameraThread>(camera, this);
    
    // 添加到管理列表
    m_cameras[cameraId] = camera;
    m_cameraThreads[cameraId] = cameraThread;
    
    // 连接信号
    QObject::connect(camera.get(), &ICameraController::stateChanged, 
                    this, [this, cameraId](CameraState state) {
        emit cameraStateChanged(QString::fromStdString(cameraId), state);
    });
    
    QObject::connect(camera.get(), &ICameraController::errorOccurred,
                    this, [this, cameraId](const QString& error) {
        emit cameraError(QString::fromStdString(cameraId), error);
    });
    
    // 连接线程信号
     connectCameraThreadSignals(cameraId, cameraThread);
    
    emit cameraAdded(QString::fromStdString(cameraId));
    qDebug() << "Camera added successfully:" << QString::fromStdString(cameraId);
    
    return true;
}

bool CameraManager::removeCamera(const std::string& cameraId) {
    QMutexLocker locker(&m_managerMutex);
    
    auto cameraIt = m_cameras.find(cameraId);
    if (cameraIt == m_cameras.end()) {
        qDebug() << "Camera not found:" << QString::fromStdString(cameraId);
        return false;
    }
    
    // 停止相机线程
    auto threadIt = m_cameraThreads.find(cameraId);
    if (threadIt != m_cameraThreads.end() && threadIt->second) {
        threadIt->second->stopCapture();
        threadIt->second->quit();
        threadIt->second->wait(3000);
        m_cameraThreads.erase(threadIt);
    }
    
    // 断开相机连接
    if (cameraIt->second) {
        cameraIt->second->disconnect();
    }
    
    // 从管理器中移除
    m_cameras.erase(cameraIt);
    
    emit cameraRemoved(QString::fromStdString(cameraId));
    qDebug() << "Camera removed successfully:" << QString::fromStdString(cameraId);
    
    return true;
}

std::shared_ptr<ICameraController> CameraManager::getCamera(const std::string& cameraId) {
    QMutexLocker locker(&m_managerMutex);
    
    auto it = m_cameras.find(cameraId);
    if (it != m_cameras.end()) {
        return std::static_pointer_cast<ICameraController>(it->second);
    }
    
    qDebug() << "Camera not found:" << QString::fromStdString(cameraId);
    return nullptr;
}

bool CameraManager::startAllCameras() {
    QMutexLocker locker(&m_managerMutex);
    
    qDebug() << "Starting all cameras...";
    bool allStarted = true;
    
    for (auto& pair : m_cameras) {
        if (pair.second) {
            if (pair.second->startStreaming()) {
                qDebug() << "Camera started:" << QString::fromStdString(pair.first);
                
                // 启动对应的线程
                auto threadIt = m_cameraThreads.find(pair.first);
                if (threadIt != m_cameraThreads.end() && threadIt->second) {
                    threadIt->second->startCapture();
                }
            } else {
                qDebug() << "Failed to start camera:" << QString::fromStdString(pair.first);
                allStarted = false;
            }
        }
    }
    
    qDebug() << "All cameras start result:" << allStarted;
    return allStarted;
}

bool CameraManager::stopAllCameras() {
    QMutexLocker locker(&m_managerMutex);
    
    qDebug() << "Stopping all cameras...";
    bool allStopped = true;
    
    for (auto& pair : m_cameras) {
        if (pair.second) {
            // 停止对应的线程
            auto threadIt = m_cameraThreads.find(pair.first);
            if (threadIt != m_cameraThreads.end() && threadIt->second) {
                threadIt->second->stopCapture();
            }
            
            if (pair.second->stopStreaming()) {
                qDebug() << "Camera stopped:" << QString::fromStdString(pair.first);
            } else {
                qDebug() << "Failed to stop camera:" << QString::fromStdString(pair.first);
                allStopped = false;
            }
        }
    }
    
    qDebug() << "All cameras stop result:" << allStopped;
    return allStopped;
}

void CameraManager::disconnectAllCameras() {
    QMutexLocker locker(&m_managerMutex);
    
    qDebug() << "Disconnecting all cameras...";
    
    for (auto& pair : m_cameras) {
        if (pair.second) {
            pair.second->disconnect();
            qDebug() << "Camera disconnected:" << QString::fromStdString(pair.first);
        }
    }
    
    m_cameras.clear();
    qDebug() << "All cameras disconnected";
}

std::vector<std::string> CameraManager::getCameraIds() const {
    QMutexLocker locker(&m_managerMutex);
    
    std::vector<std::string> ids;
    for (const auto& pair : m_cameras) {
        ids.push_back(pair.first);
    }
    
    return ids;
}

size_t CameraManager::getCameraCount() const {
    QMutexLocker locker(&m_managerMutex);
    return m_cameras.size();
}

bool CameraManager::isCameraConnected(const std::string& cameraId) const {
    QMutexLocker locker(&m_managerMutex);
    
    auto it = m_cameras.find(cameraId);
    if (it != m_cameras.end() && it->second) {
        return it->second->getState() != CameraState::Disconnected;
    }
    
    return false;
}

bool CameraManager::isCameraStreaming(const std::string& cameraId) const {
    QMutexLocker locker(&m_managerMutex);
    
    auto it = m_cameras.find(cameraId);
    if (it != m_cameras.end() && it->second) {
        return it->second->getState() == CameraState::Streaming;
    }
    
    return false;
}

std::map<std::string, CameraState> CameraManager::getAllCameraStates() const {
    QMutexLocker locker(&m_managerMutex);
    
    std::map<std::string, CameraState> states;
    for (const auto& pair : m_cameras) {
        states[pair.first] = pair.second->getState();
    }
    
    return states;
}

std::vector<std::string> CameraManager::enumerateDevices() {
    qDebug() << "Enumerating camera devices...";
    
    // 调用海康威视相机的设备枚举功能
    auto devices = HikvisionCamera::enumerateDevices();
    
    qDebug() << "Found" << devices.size() << "camera devices";
    for (const auto& device : devices) {
        qDebug() << "Device:" << QString::fromStdString(device);
    }
    
    return devices;
}

std::shared_ptr<CameraThread> CameraManager::getCameraThread(const std::string& cameraId) {
    QMutexLocker locker(&m_managerMutex);
    
    auto it = m_cameraThreads.find(cameraId);
    if (it != m_cameraThreads.end()) {
        return it->second;
    }
    
    qDebug() << "Camera thread not found:" << QString::fromStdString(cameraId);
    return nullptr;
}



QString CameraManager::getCameraStats(const std::string& cameraId) const {
    QMutexLocker locker(&m_managerMutex);
    
    auto cameraIt = m_cameras.find(cameraId);
    auto threadIt = m_cameraThreads.find(cameraId);
    
    if (cameraIt == m_cameras.end() || threadIt == m_cameraThreads.end()) {
        return QString("Camera not found: %1").arg(QString::fromStdString(cameraId));
    }
    
    QString stats;
    stats += QString("Camera ID: %1\n").arg(QString::fromStdString(cameraId));
    
    if (cameraIt->second) {
        auto state = cameraIt->second->getState();
        QString stateStr;
        switch (state) {
            case CameraState::Disconnected: stateStr = "Disconnected"; break;
            case CameraState::Connected: stateStr = "Connected"; break;
            case CameraState::Streaming: stateStr = "Streaming"; break;
            default: stateStr = "Unknown"; break;
        }
        stats += QString("State: %1\n").arg(stateStr);
        
        auto params = cameraIt->second->getParams();
        stats += QString("Resolution: %1x%2\n").arg(params.width).arg(params.height);
        stats += QString("Exposure: %1 μs\n").arg(params.exposureTime);
        stats += QString("Gain: %1\n").arg(params.gain);

    }
    
    if (threadIt->second) {
        stats += QString("Total Frames: %1\n").arg(threadIt->second->getTotalFrameCount());
        stats += QString("Capturing: %1\n").arg(threadIt->second->isCapturing() ? "Yes" : "No");
    }
    
    return stats;
}

void CameraManager::connectCameraThreadSignals(const std::string& cameraId, 
                                              std::shared_ptr<CameraThread> cameraThread) {
    if (!cameraThread) {
        return;
    }
    
    // 连接帧就绪信号
    QObject::connect(cameraThread.get(), &CameraThread::frameReady,
                    this, [this, cameraId](const cv::Mat& frame) {
        emit cameraFrameReady(QString::fromStdString(cameraId), frame);
    });
    

    
    // 连接错误信号
    QObject::connect(cameraThread.get(), &CameraThread::errorOccurred,
                    this, [this, cameraId](const QString& error) {
        emit cameraError(QString::fromStdString(cameraId), error);
    });
}

} // namespace Camera
} // namespace MutiCam