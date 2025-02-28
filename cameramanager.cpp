#include "cameramanager.h"
#include <QDebug>
#include <QMutexLocker>
#include <QMessageBox>
#include <QApplication>
#include <QThread>
#include "drawingmanager.h"

CameraManager::CameraManager(QObject *parent)
    : QObject{parent}
{
    // 初始化相机信息
    m_cameras[VerticalCamera] = CameraInfo();
    m_cameras[LeftCamera] = CameraInfo();
    m_cameras[FrontCamera] = CameraInfo();
    
    // 初始化设备列表
    memset(&m_stDevList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
}

CameraManager::~CameraManager()
{
    // 断开所有相机连接
    disconnectAllCameras();
}

bool CameraManager::enumDevices()
{
    QMutexLocker locker(&m_mutex);
    
    // 清空设备列表
    memset(&m_stDevList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
    
    // 枚举设备
    int nRet = CMvCamera::EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &m_stDevList);
    if (MV_OK != nRet)
    {
        emit logMessage(QString("枚举设备失败，错误码: %1").arg(nRet));
        return false;
    }
    
    // 输出设备信息
    emit logMessage(QString("找到 %1 个设备").arg(m_stDevList.nDeviceNum));
    
    return true;
}

int CameraManager::findDeviceIndex(const QString& serialNumber)
{
    // 遍历设备列表，查找匹配的序列号
    for (unsigned int i = 0; i < m_stDevList.nDeviceNum; i++)
    {
        MV_CC_DEVICE_INFO* pDeviceInfo = m_stDevList.pDeviceInfo[i];
        if (NULL == pDeviceInfo)
        {
            continue;
        }
        
        // 获取序列号
        QString deviceSN;
        if (pDeviceInfo->nTLayerType == MV_GIGE_DEVICE)
        {
            // GigE相机
            deviceSN = QString(reinterpret_cast<char*>(pDeviceInfo->SpecialInfo.stGigEInfo.chSerialNumber));
        }
        else if (pDeviceInfo->nTLayerType == MV_USB_DEVICE)
        {
            // USB相机
            deviceSN = QString(reinterpret_cast<char*>(pDeviceInfo->SpecialInfo.stUsb3VInfo.chSerialNumber));
        }
        
        // 比较序列号
        if (deviceSN == serialNumber)
        {
            return i;
        }
    }
    
    return -1;
}

bool CameraManager::connectCamera(CameraPosition position, const QString& serialNumber)
{
    QMutexLocker locker(&m_mutex);
    
    // 检查相机是否已连接
    if (m_cameras[position].isConnected)
    {
        // 如果已连接且序列号相同，则直接返回成功
        if (m_cameras[position].serialNumber == serialNumber)
        {
            return true;
        }
        
        // 如果序列号不同，则先断开连接
        disconnectCamera(position);
    }
    
    // 查找设备索引
    int deviceIndex = findDeviceIndex(serialNumber);
    if (deviceIndex < 0)
    {
        emit logMessage(QString("未找到序列号为 %1 的设备").arg(serialNumber));
        return false;
    }
    
    // 创建相机对象
    m_cameras[position].camera = new CMvCamera();
    
    // 连接相机
    int nRet = m_cameras[position].camera->Open(m_stDevList.pDeviceInfo[deviceIndex]);
    if (MV_OK != nRet)
    {
        emit logMessage(QString("连接相机失败，错误码: %1").arg(nRet));
        delete m_cameras[position].camera;
        m_cameras[position].camera = nullptr;
        return false;
    }
    
    // 创建图像抓取线程
    m_cameras[position].grabThread = new GrabImgThread(static_cast<int>(position), this);
    m_cameras[position].grabThread->setCameraPtr(m_cameras[position].camera);
    
    // 连接信号和槽
    connect(m_cameras[position].grabThread, &GrabImgThread::signal_imageReady, 
            this, &CameraManager::handleImageReady);
    
    // 更新相机信息
    m_cameras[position].serialNumber = serialNumber;
    updateCameraConnectionStatus(position, true);
    
    emit logMessage(QString("相机 %1 连接成功").arg(serialNumber));
    
    return true;
}

bool CameraManager::disconnectCamera(CameraPosition position)
{
    QMutexLocker locker(&m_mutex);
    
    // 检查相机是否已连接
    if (!m_cameras[position].isConnected)
    {
        return true;
    }
    
    // 停止采集
    if (m_cameras[position].isGrabbing)
    {
        stopGrabbing(position);
    }
    
    // 断开线程连接
    if (m_cameras[position].grabThread)
    {
        disconnect(m_cameras[position].grabThread, &GrabImgThread::signal_imageReady, 
                   this, &CameraManager::handleImageReady);
        
        // 等待线程结束
        if (m_cameras[position].grabThread->isRunning())
        {
            m_cameras[position].grabThread->terminate();
            m_cameras[position].grabThread->wait();
        }
        
        delete m_cameras[position].grabThread;
        m_cameras[position].grabThread = nullptr;
    }
    
    // 关闭相机
    if (m_cameras[position].camera)
    {
        m_cameras[position].camera->Close();
        delete m_cameras[position].camera;
        m_cameras[position].camera = nullptr;
    }
    
    // 更新相机信息
    QString serialNumber = m_cameras[position].serialNumber;
    m_cameras[position].serialNumber.clear();
    updateCameraConnectionStatus(position, false);
    
    emit logMessage(QString("相机 %1 断开连接").arg(serialNumber));
    
    return true;
}

void CameraManager::disconnectAllCameras()
{
    // 断开所有相机连接
    disconnectCamera(VerticalCamera);
    disconnectCamera(LeftCamera);
    disconnectCamera(FrontCamera);
}

bool CameraManager::startGrabbing(CameraPosition position)
{
    QMutexLocker locker(&m_mutex);
    
    // 检查相机是否已连接
    if (!m_cameras[position].isConnected)
    {
        emit logMessage(QString("相机未连接，无法开始采集"));
        return false;
    }
    
    // 检查相机是否已在采集
    if (m_cameras[position].isGrabbing)
    {
        return true;
    }
    
    // 开始采集
    int nRet = m_cameras[position].camera->StartGrabbing();
    if (MV_OK != nRet)
    {
        emit logMessage(QString("开始采集失败，错误码: %1").arg(nRet));
        return false;
    }
    
    // 启动图像抓取线程
    m_cameras[position].grabThread->start();
    
    // 更新相机采集状态
    updateCameraGrabbingStatus(position, true);
    
    emit logMessage(QString("相机 %1 开始采集").arg(m_cameras[position].serialNumber));
    
    return true;
}

bool CameraManager::stopGrabbing(CameraPosition position)
{
    QMutexLocker locker(&m_mutex);
    
    // 检查相机是否已连接
    if (!m_cameras[position].isConnected)
    {
        return true;
    }
    
    // 检查相机是否已在采集
    if (!m_cameras[position].isGrabbing)
    {
        return true;
    }
    
    // 停止采集
    int nRet = m_cameras[position].camera->StopGrabbing();
    if (MV_OK != nRet)
    {
        emit logMessage(QString("停止采集失败，错误码: %1").arg(nRet));
        return false;
    }
    
    // 停止图像抓取线程
    if (m_cameras[position].grabThread->isRunning())
    {
        m_cameras[position].grabThread->terminate();
        m_cameras[position].grabThread->wait();
    }
    
    // 更新相机采集状态
    updateCameraGrabbingStatus(position, false);
    
    emit logMessage(QString("相机 %1 停止采集").arg(m_cameras[position].serialNumber));
    
    return true;
}

bool CameraManager::startAllGrabbing()
{
    // 开始所有相机采集
    bool result = true;
    
    if (m_cameras[VerticalCamera].isConnected)
    {
        result &= startGrabbing(VerticalCamera);
    }
    
    if (m_cameras[LeftCamera].isConnected)
    {
        result &= startGrabbing(LeftCamera);
    }
    
    if (m_cameras[FrontCamera].isConnected)
    {
        result &= startGrabbing(FrontCamera);
    }
    
    return result;
}

bool CameraManager::stopAllGrabbing()
{
    // 停止所有相机采集
    bool result = true;
    
    if (m_cameras[VerticalCamera].isConnected)
    {
        result &= stopGrabbing(VerticalCamera);
    }
    
    if (m_cameras[LeftCamera].isConnected)
    {
        result &= stopGrabbing(LeftCamera);
    }
    
    if (m_cameras[FrontCamera].isConnected)
    {
        result &= stopGrabbing(FrontCamera);
    }
    
    return result;
}

void CameraManager::setDisplayLabel(CameraPosition position, QLabel* mainLabel, QLabel* tabLabel)
{
    QMutexLocker locker(&m_mutex);
    
    // 设置显示标签
    m_displayLabels[position] = qMakePair(mainLabel, tabLabel);
}

CameraInfo CameraManager::getCameraInfo(CameraPosition position) const
{
    return m_cameras[position];
}

bool CameraManager::isCameraConnected(CameraPosition position) const
{
    return m_cameras[position].isConnected;
}

bool CameraManager::isCameraGrabbing(CameraPosition position) const
{
    return m_cameras[position].isGrabbing;
}

void CameraManager::handleImageReady(QImage image, int cameraId)
{
    // 将cameraId转换为CameraPosition
    CameraPosition position = static_cast<CameraPosition>(cameraId);
    
    // 发送图像就绪信号
    emit imageReady(position, image);
    
    // 更新显示标签
    if (m_displayLabels.contains(position))
    {
        QLabel* mainLabel = m_displayLabels[position].first;
        QLabel* tabLabel = m_displayLabels[position].second;
        
        // 获取原始的cv::Mat帧
        GrabImgThread* thread = m_cameras[position].grabThread;
        if (thread)
        {
            cv::Mat frame = thread->getLastFrame();
            
            if (mainLabel)
            {
                // 如果有绘图管理器，使用它来更新显示
                DrawingManager* drawingManager = qobject_cast<DrawingManager*>(mainLabel->property("drawingManager").value<QObject*>());
                if (drawingManager && !frame.empty())
                {
                    // 使用绘图管理器更新显示，保持绘制内容
                    drawingManager->updateDisplay(mainLabel, frame.clone());
                }
                else
                {
                    // 保持QLabel大小不变，只缩放图像
                    QSize labelSize = mainLabel->size();
                    mainLabel->setPixmap(QPixmap::fromImage(image).scaled(
                        labelSize, 
                        Qt::KeepAspectRatio, 
                        Qt::SmoothTransformation));
                    
                    // 确保QLabel大小策略正确
                    mainLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
                    mainLabel->setScaledContents(false);
                    
                    // 保存原始帧到QLabel的属性中
                    if (!frame.empty())
                    {
                        mainLabel->setProperty("currentFrame", QVariant::fromValue(frame.clone()));
                    }
                }
            }
            
            if (tabLabel)
            {
                // 如果有绘图管理器，使用它来更新显示
                DrawingManager* drawingManager = qobject_cast<DrawingManager*>(tabLabel->property("drawingManager").value<QObject*>());
                if (drawingManager && !frame.empty())
                {
                    // 使用绘图管理器更新显示，保持绘制内容
                    drawingManager->updateDisplay(tabLabel, frame.clone());
                }
                else
                {
                    // 保持QLabel大小不变，只缩放图像
                    QSize labelSize = tabLabel->size();
                    tabLabel->setPixmap(QPixmap::fromImage(image).scaled(
                        labelSize, 
                        Qt::KeepAspectRatio, 
                        Qt::SmoothTransformation));
                    
                    // 确保QLabel大小策略正确
                    tabLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
                    tabLabel->setScaledContents(false);
                    
                    // 保存原始帧到QLabel的属性中
                    if (!frame.empty())
                    {
                        tabLabel->setProperty("currentFrame", QVariant::fromValue(frame.clone()));
                    }
                }
            }
        }
    }
}

void CameraManager::updateCameraConnectionStatus(CameraPosition position, bool isConnected)
{
    // 更新相机连接状态
    m_cameras[position].isConnected = isConnected;
    
    // 发送相机连接状态变化信号
    emit cameraConnectionChanged(position, isConnected);
}

void CameraManager::updateCameraGrabbingStatus(CameraPosition position, bool isGrabbing)
{
    // 更新相机采集状态
    m_cameras[position].isGrabbing = isGrabbing;
    
    // 发送相机采集状态变化信号
    emit cameraGrabbingChanged(position, isGrabbing);
} 