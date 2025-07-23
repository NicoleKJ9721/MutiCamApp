#include "hikvision_camera.h"
#include <QMutexLocker>
#include <QDebug>
#include <QMetaObject>
#include <chrono>
#include <algorithm>

namespace MutiCam {
namespace Camera {

/**
 * @brief HikvisionCamera实现
 */
HikvisionCamera::HikvisionCamera(QObject* parent) 
    : ICameraController()
    , m_frameTimer(new QTimer(this)) {
    
    setParent(parent);
    
    // 初始化帧超时定时器
    m_frameTimer->setSingleShot(true);
    m_frameTimer->setInterval(5000); // 5秒超时
    QObject::connect(m_frameTimer, &QTimer::timeout, this, &HikvisionCamera::onFrameTimeout);
    
    // 初始化相机参数
    m_params.serialNumber = "";
    m_params.exposureTime = 10000;
    m_params.gain = 0;

    m_params.pixelFormat = "Mono8";
    m_params.width = 1920;
    m_params.height = 1080;
    
    // 初始化性能统计
    m_lastStatsTime = std::chrono::steady_clock::now();
    
    m_state = CameraState::Disconnected;
}

HikvisionCamera::~HikvisionCamera() {
    // 停止采集并断开连接
    if (m_isStreaming) {
        stopStreaming();
    }
    if (m_state != CameraState::Disconnected) {
        disconnect();
    }
    
    // 释放转换缓冲区
    if (m_pConvertBuffer) {
        delete[] m_pConvertBuffer;
        m_pConvertBuffer = nullptr;
    }
}

bool HikvisionCamera::connect(const std::string& serialNumber) {
    QMutexLocker locker(&m_mutex);
    
    if (m_state != CameraState::Disconnected) {
        setError("Camera is already connected");
        return false;
    }
    
    // 打开设备
    if (!openDevice(serialNumber)) {
        return false;
    }
    
    // 初始化相机
    if (!initializeCamera()) {
        closeDevice();
        return false;
    }
    
    m_params.serialNumber = serialNumber;
    m_state = CameraState::Connected;
    
    // 更新相机参数
    updateCameraParams();
    
    emit stateChanged(m_state);
    return true;
}

bool HikvisionCamera::disconnect() {
    QMutexLocker locker(&m_mutex);
    
    qDebug() << "Disconnecting camera:" << m_params.serialNumber.c_str();
    
    // 停止采集
    if (m_state == CameraState::Streaming) {
        stopStreaming();
    }
    
    // 关闭设备并释放资源
    if (m_state != CameraState::Disconnected) {
        closeDevice();
    }
    
    m_state = CameraState::Disconnected;
    emit stateChanged(m_state);
    
    qDebug() << "Camera disconnected:" << m_params.serialNumber.c_str();
    return true;
}

bool HikvisionCamera::startStreaming() {
    QMutexLocker locker(&m_mutex);
    
    if (m_state != CameraState::Connected) {
        setError("Camera is not connected");
        return false;
    }
    
    if (nullptr == m_hCamera) {
        setError("Invalid camera handle");
        return false;
    }
    
    // 开始采集
    int nRet = MV_CC_StartGrabbing(m_hCamera);
    if (MV_OK != nRet) {
        setError("Failed to start grabbing");
        return false;
    }
    
    m_isStreaming = true;
    m_frameCount = 0;
    m_lastStatsTime = std::chrono::steady_clock::now();
    
    // 启动帧超时定时器（线程安全）
    QMetaObject::invokeMethod(m_frameTimer, "start", Qt::QueuedConnection);
    
    m_state = CameraState::Streaming;
    emit stateChanged(m_state);
    
    qDebug() << "Camera streaming started:" << m_params.serialNumber.c_str();
    return true;
}

bool HikvisionCamera::stopStreaming() {
    QMutexLocker locker(&m_mutex);
    
    if (m_state != CameraState::Streaming) {
        return true;
    }
    
    // 停止采集
    if (nullptr != m_hCamera) {
        int nRet = MV_CC_StopGrabbing(m_hCamera);
        if (MV_OK != nRet) {
            qDebug() << "Failed to stop grabbing, error code:" << nRet;
        }
    }
    
    m_isStreaming = false;
    // 线程安全地停止定时器
    QMetaObject::invokeMethod(m_frameTimer, "stop", Qt::QueuedConnection);
    
    m_state = CameraState::Connected;
    emit stateChanged(m_state);
    
    qDebug() << "Camera streaming stopped:" << m_params.serialNumber.c_str();
    return true;
}

bool HikvisionCamera::setExposureTime(int exposureTime) {
    QMutexLocker locker(&m_mutex);
    
    if (m_state == CameraState::Disconnected) {
        setError("Camera is not connected");
        return false;
    }
    
    if (nullptr == m_hCamera) {
        setError("Invalid camera handle");
        return false;
    }
    
    // 设置曝光时间
    int nRet = MV_CC_SetFloatValue(m_hCamera, "ExposureTime", static_cast<float>(exposureTime));
    if (MV_OK != nRet) {
        setError("Failed to set exposure time");
        return false;
    }
    
    m_params.exposureTime = exposureTime;
    qDebug() << "Exposure time set to:" << exposureTime << "us";
    return true;
}

bool HikvisionCamera::setGain(int gain) {
    QMutexLocker locker(&m_mutex);
    
    if (m_state == CameraState::Disconnected) {
        setError("Camera is not connected");
        return false;
    }
    
    if (nullptr == m_hCamera) {
        setError("Invalid camera handle");
        return false;
    }
    
    // 设置增益
    int nRet = MV_CC_SetFloatValue(m_hCamera, "Gain", static_cast<float>(gain));
    if (MV_OK != nRet) {
        setError("Failed to set gain");
        return false;
    }
    
    m_params.gain = gain;
    qDebug() << "Gain set to:" << gain;
    return true;
}

bool HikvisionCamera::setPixelFormat(const std::string& format) {
    QMutexLocker locker(&m_mutex);
    
    if (m_state == CameraState::Disconnected) {
        setError("Camera is not connected");
        return false;
    }
    
    if (nullptr == m_hCamera) {
        setError("Invalid camera handle");
        return false;
    }
    
    // 将字符串格式转换为枚举值
    unsigned int pixelFormatValue = 0;
    if (format == "Mono8") {
        pixelFormatValue = PixelType_Gvsp_Mono8;
    } else if (format == "RGB8") {
        pixelFormatValue = PixelType_Gvsp_RGB8_Packed;
    } else if (format == "BGR8") {
        pixelFormatValue = PixelType_Gvsp_BGR8_Packed;
    } else {
        setError("Unsupported pixel format: " + format);
        return false;
    }
    
    // 设置像素格式
    int nRet = MV_CC_SetEnumValue(m_hCamera, "PixelFormat", pixelFormatValue);
    if (MV_OK != nRet) {
        setError("Failed to set pixel format");
        return false;
    }
    
    m_params.pixelFormat = format;
    qDebug() << "Pixel format set to:" << format.c_str();
    return true;
}



CameraParams HikvisionCamera::getParams() const {
    return m_params;
}

CameraState HikvisionCamera::getState() const {
    return m_state;
}

std::string HikvisionCamera::getLastError() const {
    return m_lastError;
}

cv::Mat HikvisionCamera::getLatestFrame() {
    QMutexLocker locker(&m_frameMutex);
    return m_latestFrame.clone();
}

std::vector<std::string> HikvisionCamera::enumerateDevices() {
    std::vector<std::string> devices;
    
    // 初始化SDK
    int nRet = MV_CC_Initialize();
    if (MV_OK != nRet) {
        qDebug() << "Failed to initialize MVS SDK, error code:" << nRet;
        return devices;
    }
    
    // 枚举设备
    MV_CC_DEVICE_INFO_LIST stDeviceList;
    memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
    
    nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &stDeviceList);
    if (MV_OK != nRet) {
        qDebug() << "Failed to enumerate devices, error code:" << nRet;
        MV_CC_Finalize();
        return devices;
    }
    
    // 解析设备信息
    for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) {
        MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i];
        if (nullptr == pDeviceInfo) {
            continue;
        }
        
        std::string serialNumber;
        if (pDeviceInfo->nTLayerType == MV_GIGE_DEVICE) {
            // GigE相机
            if (pDeviceInfo->SpecialInfo.stGigEInfo.chSerialNumber[0] != '\0') {
                serialNumber = std::string((char*)pDeviceInfo->SpecialInfo.stGigEInfo.chSerialNumber);
            }
        } else if (pDeviceInfo->nTLayerType == MV_USB_DEVICE) {
            // USB相机
            if (pDeviceInfo->SpecialInfo.stUsb3VInfo.chSerialNumber[0] != '\0') {
                serialNumber = std::string((char*)pDeviceInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);
            }
        }
        
        if (!serialNumber.empty()) {
            devices.push_back(serialNumber);
        }
    }
    
    // 清理SDK
    MV_CC_Finalize();
    
    return devices;
}

bool HikvisionCamera::setTriggerMode(bool enabled) {
    QMutexLocker locker(&m_mutex);
    
    if (m_state == CameraState::Disconnected) {
        setError("Camera is not connected");
        return false;
    }
    
    if (nullptr == m_hCamera) {
        setError("Invalid camera handle");
        return false;
    }
    
    // 设置触发模式
    int nRet = MV_CC_SetEnumValue(m_hCamera, "TriggerMode", enabled ? 1 : 0);
    if (MV_OK != nRet) {
        setError("Failed to set trigger mode");
        return false;
    }
    
    if (enabled) {
        // 设置触发源为软件触发
        nRet = MV_CC_SetEnumValue(m_hCamera, "TriggerSource", MV_TRIGGER_SOURCE_SOFTWARE);
        if (MV_OK != nRet) {
            setError("Failed to set trigger source");
            return false;
        }
    }
    
    qDebug() << "Trigger mode" << (enabled ? "enabled" : "disabled");
    return true;
}

bool HikvisionCamera::softwareTrigger() {
    QMutexLocker locker(&m_mutex);
    
    if (m_state != CameraState::Streaming) {
        setError("Camera is not streaming");
        return false;
    }
    
    if (nullptr == m_hCamera) {
        setError("Invalid camera handle");
        return false;
    }
    
    // 执行软件触发
    int nRet = MV_CC_TriggerSoftwareExecute(m_hCamera);
    if (MV_OK != nRet) {
        setError("Failed to execute software trigger");
        return false;
    }
    
    qDebug() << "Software trigger executed";
    return true;
}

void HikvisionCamera::onFrameTimeout() {
    qDebug() << "Frame timeout for camera:" << m_params.serialNumber.c_str();
    emit errorOccurred("Frame acquisition timeout");
}

bool HikvisionCamera::openDevice(const std::string& serialNumber) {
    // 初始化SDK
    int nRet = MV_CC_Initialize();
    if (MV_OK != nRet) {
        setError("Failed to initialize MVS SDK");
        return false;
    }
    
    // 枚举设备
    MV_CC_DEVICE_INFO_LIST stDeviceList;
    memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
    
    nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &stDeviceList);
    if (MV_OK != nRet) {
        setError("Failed to enumerate devices");
        MV_CC_Finalize();
        return false;
    }
    
    // 查找指定序列号的设备
    MV_CC_DEVICE_INFO* pTargetDevice = nullptr;
    for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) {
        MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i];
        if (nullptr == pDeviceInfo) {
            continue;
        }
        
        std::string currentSerial;
        if (pDeviceInfo->nTLayerType == MV_GIGE_DEVICE) {
            if (pDeviceInfo->SpecialInfo.stGigEInfo.chSerialNumber[0] != '\0') {
                currentSerial = std::string((char*)pDeviceInfo->SpecialInfo.stGigEInfo.chSerialNumber);
            }
        } else if (pDeviceInfo->nTLayerType == MV_USB_DEVICE) {
            if (pDeviceInfo->SpecialInfo.stUsb3VInfo.chSerialNumber[0] != '\0') {
                currentSerial = std::string((char*)pDeviceInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);
            }
        }
        
        if (currentSerial == serialNumber) {
            pTargetDevice = pDeviceInfo;
            break;
        }
    }
    
    if (nullptr == pTargetDevice) {
        setError("Device with specified serial number not found");
        MV_CC_Finalize();
        return false;
    }
    
    // 创建设备句柄
    nRet = MV_CC_CreateHandle(&m_hCamera, pTargetDevice);
    if (MV_OK != nRet) {
        setError("Failed to create device handle");
        MV_CC_Finalize();
        return false;
    }
    
    // 打开设备
    nRet = MV_CC_OpenDevice(m_hCamera);
    if (MV_OK != nRet) {
        setError("Failed to open device");
        MV_CC_DestroyHandle(m_hCamera);
        m_hCamera = nullptr;
        MV_CC_Finalize();
        return false;
    }
    
    return true;
}

bool HikvisionCamera::closeDevice() {
    if (nullptr == m_hCamera) {
        return true;
    }
    
    // 关闭设备
    int nRet = MV_CC_CloseDevice(m_hCamera);
    if (MV_OK != nRet) {
        qDebug() << "Failed to close device, error code:" << nRet;
    }
    
    // 销毁句柄
    nRet = MV_CC_DestroyHandle(m_hCamera);
    if (MV_OK != nRet) {
        qDebug() << "Failed to destroy handle, error code:" << nRet;
    }
    
    m_hCamera = nullptr;
    
    // 清理SDK
    MV_CC_Finalize();
    
    return true;
}

bool HikvisionCamera::initializeCamera() {
    if (nullptr == m_hCamera) {
        setError("Invalid camera handle");
        return false;
    }
    
    // 设置触发模式为关闭
    int nRet = MV_CC_SetEnumValue(m_hCamera, "TriggerMode", 0);
    if (MV_OK != nRet) {
        setError("Failed to set trigger mode");
        return false;
    }
    
    // 注册图像回调
    nRet = MV_CC_RegisterImageCallBackEx(m_hCamera, imageCallbackEx, this);
    if (MV_OK != nRet) {
        setError("Failed to register image callback");
        return false;
    }
    
    return true;
}

void HikvisionCamera::updateCameraParams() {
    if (nullptr == m_hCamera) {
        return;
    }
    
    // 获取图像宽度
    MVCC_INTVALUE stParam;
    memset(&stParam, 0, sizeof(MVCC_INTVALUE));
    int nRet = MV_CC_GetIntValue(m_hCamera, "Width", &stParam);
    if (MV_OK == nRet) {
        m_params.width = static_cast<int>(stParam.nCurValue);
    }
    
    // 获取图像高度
    memset(&stParam, 0, sizeof(MVCC_INTVALUE));
    nRet = MV_CC_GetIntValue(m_hCamera, "Height", &stParam);
    if (MV_OK == nRet) {
        m_params.height = static_cast<int>(stParam.nCurValue);
    }
    
    // 获取曝光时间
    MVCC_FLOATVALUE stFloatValue;
    memset(&stFloatValue, 0, sizeof(MVCC_FLOATVALUE));
    nRet = MV_CC_GetFloatValue(m_hCamera, "ExposureTime", &stFloatValue);
    if (MV_OK == nRet) {
        m_params.exposureTime = static_cast<int>(stFloatValue.fCurValue);
    }
    
    // 获取增益
    memset(&stFloatValue, 0, sizeof(MVCC_FLOATVALUE));
    nRet = MV_CC_GetFloatValue(m_hCamera, "Gain", &stFloatValue);
    if (MV_OK == nRet) {
        m_params.gain = static_cast<int>(stFloatValue.fCurValue);
    }
    

}

void HikvisionCamera::setError(const std::string& error) {
    m_lastError = error;
    qDebug() << "HikvisionCamera Error:" << error.c_str();
}

void __stdcall HikvisionCamera::imageCallbackEx(unsigned char* pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser) {
    // 简化实现
    if (pUser) {
        HikvisionCamera* camera = static_cast<HikvisionCamera*>(pUser);
        camera->processFrame(pData, pFrameInfo);
    }
}

void HikvisionCamera::processFrame(unsigned char* pData, MV_FRAME_OUT_INFO_EX* pFrameInfo) {
    if (!pData || !pFrameInfo || !m_isStreaming) {
        return;
    }

    static int callbackCount = 0;
    callbackCount++;
    if (callbackCount % 30 == 0) { // 每30帧打印一次
        qDebug() << "相机回调函数被调用，序列号：" << m_params.serialNumber.c_str()
                 << "帧大小：" << pFrameInfo->nWidth << "x" << pFrameInfo->nHeight;
    }
    
    try {
        // 重置帧超时定时器（线程安全）
        QMetaObject::invokeMethod(m_frameTimer, "start", Qt::QueuedConnection);
        
        // 创建OpenCV图像
        cv::Mat frame;
        
        // 根据像素格式处理图像数据
        if (pFrameInfo->enPixelType == PixelType_Gvsp_Mono8) {
            frame = cv::Mat(pFrameInfo->nHeight, pFrameInfo->nWidth, CV_8UC1, pData).clone();
        } else if (pFrameInfo->enPixelType == PixelType_Gvsp_RGB8_Packed) {
            frame = cv::Mat(pFrameInfo->nHeight, pFrameInfo->nWidth, CV_8UC3, pData).clone();
            cv::cvtColor(frame, frame, cv::COLOR_RGB2BGR);
        } else {
            // 对于其他格式，尝试转换为Mono8
            unsigned int nConvertSize = pFrameInfo->nWidth * pFrameInfo->nHeight;
            if (m_nConvertBufferSize < nConvertSize) {
                if (m_pConvertBuffer) {
                    delete[] m_pConvertBuffer;
                }
                m_pConvertBuffer = new unsigned char[nConvertSize];
                m_nConvertBufferSize = nConvertSize;
            }
            
            MV_CC_PIXEL_CONVERT_PARAM stConvertParam = {0};
            stConvertParam.nWidth = pFrameInfo->nWidth;
            stConvertParam.nHeight = pFrameInfo->nHeight;
            stConvertParam.pSrcData = pData;
            stConvertParam.nSrcDataLen = pFrameInfo->nFrameLen;
            stConvertParam.enSrcPixelType = pFrameInfo->enPixelType;
            stConvertParam.enDstPixelType = PixelType_Gvsp_Mono8;
            stConvertParam.pDstBuffer = m_pConvertBuffer;
            stConvertParam.nDstBufferSize = m_nConvertBufferSize;
            
            int nRet = MV_CC_ConvertPixelType(m_hCamera, &stConvertParam);
            if (MV_OK == nRet) {
                frame = cv::Mat(pFrameInfo->nHeight, pFrameInfo->nWidth, CV_8UC1, m_pConvertBuffer).clone();
            } else {
                qDebug() << "Failed to convert pixel format, error code:" << nRet;
                return;
            }
        }
        
        if (!frame.empty()) {
            // 更新最新帧
            {
                QMutexLocker locker(&m_frameMutex);
                m_latestFrame = frame;
            }
            
            // 更新帧计数
            m_frameCount++;
            
            // 发射帧就绪信号
            emit frameReady(frame);
        }
    } catch (const std::exception& e) {
        qDebug() << "Exception in processFrame:" << e.what();
        setError(std::string("Frame processing error: ") + e.what());
    }
}

} // namespace Camera
} // namespace MutiCam