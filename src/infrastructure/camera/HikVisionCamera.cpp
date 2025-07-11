#include "HikVisionCamera.h"
#include <QDebug>
#include <QMutexLocker>

// 静态成员初始化
MV_CC_DEVICE_INFO_LIST HikVisionCamera::s_deviceList = {0};
bool HikVisionCamera::s_devicesEnumerated = false;

HikVisionCamera::HikVisionCamera(const QString& serialNumber)
    : m_hCamera(nullptr)
    , m_serialNumber(serialNumber)
    , m_isOpened(false)
    , m_isGrabbing(false)
    , m_nPayloadSize(0)
{
    qDebug() << "创建海康相机实例，序列号:" << serialNumber;
    
    // 如果提供了序列号，尝试自动打开相机
    if (!serialNumber.isEmpty()) {
        if (!openCameraBySerialNumber(serialNumber)) {
            qWarning() << "无法自动打开序列号为" << serialNumber << "的相机";
        }
    }
}

HikVisionCamera::~HikVisionCamera()
{
    closeCamera();
}

bool HikVisionCamera::enumerateDevices()
{
    qDebug() << "开始枚举海康相机设备...";
    
    // 枚举GigE设备
    MV_CC_DEVICE_INFO_LIST gigeDevices = {0};
    int ret = MV_CC_EnumDevices(MV_GIGE_DEVICE, &gigeDevices);
    if (ret != MV_OK) {
        qWarning() << "枚举GigE设备失败，错误码:" << QString("0x%1").arg(ret, 8, 16, QLatin1Char('0'));
    } else {
        qDebug() << "找到" << gigeDevices.nDeviceNum << "个GigE设备";
    }
    
    // 枚举USB设备
    MV_CC_DEVICE_INFO_LIST usbDevices = {0};
    ret = MV_CC_EnumDevices(MV_USB_DEVICE, &usbDevices);
    if (ret != MV_OK) {
        qWarning() << "枚举USB设备失败，错误码:" << QString("0x%1").arg(ret, 8, 16, QLatin1Char('0'));
    } else {
        qDebug() << "找到" << usbDevices.nDeviceNum << "个USB设备";
    }
    
    // 合并设备列表
    s_deviceList.nDeviceNum = gigeDevices.nDeviceNum + usbDevices.nDeviceNum;
    
    if (s_deviceList.nDeviceNum == 0) {
        qWarning() << "未找到任何相机设备！";
        return false;
    }
    
    // 复制GigE设备信息
    for (unsigned int i = 0; i < gigeDevices.nDeviceNum; i++) {
        s_deviceList.pDeviceInfo[i] = gigeDevices.pDeviceInfo[i];
    }
    
    // 复制USB设备信息
    for (unsigned int i = 0; i < usbDevices.nDeviceNum; i++) {
        s_deviceList.pDeviceInfo[gigeDevices.nDeviceNum + i] = usbDevices.pDeviceInfo[i];
    }
    
    s_devicesEnumerated = true;
    
    // 打印设备信息
    qDebug() << "总共找到" << s_deviceList.nDeviceNum << "个设备:";
    for (unsigned int i = 0; i < s_deviceList.nDeviceNum; i++) {
        QString deviceInfo = getDeviceSerialNumber(i);
        qDebug() << "  设备" << i << "序列号:" << deviceInfo;
    }
    
    return true;
}

int HikVisionCamera::getDeviceCount()
{
    if (!s_devicesEnumerated) {
        enumerateDevices();
    }
    return s_deviceList.nDeviceNum;
}

QString HikVisionCamera::getDeviceSerialNumber(int index)
{
    if (!s_devicesEnumerated) {
        enumerateDevices();
    }
    
    if (index < 0 || index >= static_cast<int>(s_deviceList.nDeviceNum)) {
        return QString();
    }
    
    MV_CC_DEVICE_INFO* pDeviceInfo = s_deviceList.pDeviceInfo[index];
    if (pDeviceInfo->nTLayerType == MV_GIGE_DEVICE) {
        // GigE设备
        QString serialNumber;
        for (int i = 0; i < sizeof(pDeviceInfo->SpecialInfo.stGigEInfo.chSerialNumber); i++) {
            char ch = pDeviceInfo->SpecialInfo.stGigEInfo.chSerialNumber[i];
            if (ch == 0) break;
            serialNumber += ch;
        }
        return serialNumber;
    } else if (pDeviceInfo->nTLayerType == MV_USB_DEVICE) {
        // USB设备
        QString serialNumber;
        for (int i = 0; i < sizeof(pDeviceInfo->SpecialInfo.stUsb3VInfo.chSerialNumber); i++) {
            char ch = pDeviceInfo->SpecialInfo.stUsb3VInfo.chSerialNumber[i];
            if (ch == 0) break;
            serialNumber += ch;
        }
        return serialNumber;
    }
    
    return QString();
}

int HikVisionCamera::findDeviceBySerialNumber(const QString& serialNumber)
{
    if (!s_devicesEnumerated) {
        enumerateDevices();
    }
    
    for (int i = 0; i < static_cast<int>(s_deviceList.nDeviceNum); i++) {
        if (getDeviceSerialNumber(i) == serialNumber) {
            return i;
        }
    }
    
    return -1; // 未找到
}

bool HikVisionCamera::openCamera()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_isOpened) {
        qWarning() << "相机已经打开";
        return true;
    }
    
    // 如果有序列号，按序列号打开
    if (!m_serialNumber.isEmpty()) {
        return openCameraBySerialNumber(m_serialNumber);
    }
    
    // 否则打开第一个设备
    if (getDeviceCount() == 0) {
        qWarning() << "没有找到任何相机设备";
        return false;
    }
    
    return createCamera(0);
}

bool HikVisionCamera::openCameraBySerialNumber(const QString& serialNumber)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_isOpened && m_serialNumber == serialNumber) {
        return true;
    }
    
    // 关闭当前相机
    if (m_isOpened) {
        closeCamera();
    }
    
    // 查找设备
    int deviceIndex = findDeviceBySerialNumber(serialNumber);
    if (deviceIndex == -1) {
        qWarning() << "未找到序列号为" << serialNumber << "的设备";
        return false;
    }
    
    m_serialNumber = serialNumber;
    return createCamera(deviceIndex);
}

bool HikVisionCamera::createCamera(int deviceIndex)
{
    if (deviceIndex < 0 || deviceIndex >= static_cast<int>(s_deviceList.nDeviceNum)) {
        qWarning() << "设备索引超出范围:" << deviceIndex;
        return false;
    }
    
    // 创建相机句柄
    int ret = MV_CC_CreateHandle(&m_hCamera, s_deviceList.pDeviceInfo[deviceIndex]);
    if (ret != MV_OK) {
        qWarning() << "创建相机句柄失败，错误码:" << QString("0x%1").arg(ret, 8, 16, QLatin1Char('0'));
        return false;
    }
    
    // 打开相机
    ret = MV_CC_OpenDevice(m_hCamera);
    if (ret != MV_OK) {
        qWarning() << "打开相机设备失败，错误码:" << QString("0x%1").arg(ret, 8, 16, QLatin1Char('0'));
        MV_CC_DestroyHandle(m_hCamera);
        m_hCamera = nullptr;
        return false;
    }
    
    // 获取载荷大小
    MVCC_INTVALUE stIntValue = {0};
    ret = MV_CC_GetIntValue(m_hCamera, "PayloadSize", &stIntValue);
    if (ret != MV_OK) {
        qWarning() << "获取载荷大小失败，错误码:" << QString("0x%1").arg(ret, 8, 16, QLatin1Char('0'));
        MV_CC_CloseDevice(m_hCamera);
        MV_CC_DestroyHandle(m_hCamera);
        m_hCamera = nullptr;
        return false;
    }
    m_nPayloadSize = stIntValue.nCurValue;
    
    m_isOpened = true;
    qDebug() << "成功打开相机，序列号:" << m_serialNumber << "载荷大小:" << m_nPayloadSize;
    
    return true;
}

bool HikVisionCamera::startGrabbing()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_isOpened) {
        qWarning() << "相机未打开，无法开始采集";
        return false;
    }
    
    if (m_isGrabbing) {
        qWarning() << "相机已在采集中";
        return true;
    }
    
    // 开始取流
    int ret = MV_CC_StartGrabbing(m_hCamera);
    if (ret != MV_OK) {
        qWarning() << "开始采集失败，错误码:" << QString("0x%1").arg(ret, 8, 16, QLatin1Char('0'));
        return false;
    }
    
    m_isGrabbing = true;
    qDebug() << "开始图像采集，相机序列号:" << m_serialNumber;
    
    return true;
}

bool HikVisionCamera::stopGrabbing()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_isGrabbing) {
        return true;
    }
    
    int ret = MV_CC_StopGrabbing(m_hCamera);
    if (ret != MV_OK) {
        qWarning() << "停止采集失败，错误码:" << QString("0x%1").arg(ret, 8, 16, QLatin1Char('0'));
        return false;
    }
    
    m_isGrabbing = false;
    qDebug() << "停止图像采集，相机序列号:" << m_serialNumber;
    
    return true;
}

void HikVisionCamera::closeCamera()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_isGrabbing) {
        stopGrabbing();
    }
    
    if (m_hCamera != nullptr) {
        MV_CC_CloseDevice(m_hCamera);
        MV_CC_DestroyHandle(m_hCamera);
        m_hCamera = nullptr;
    }
    
    m_isOpened = false;
    qDebug() << "关闭相机，序列号:" << m_serialNumber;
}

bool HikVisionCamera::getFrame(unsigned char*& pImageBuf, unsigned int& nImageSize, 
                              unsigned int& nWidth, unsigned int& nHeight)
{
    if (!m_isGrabbing) {
        qDebug() << "相机未在采集状态，序列号:" << m_serialNumber;
        return false;
    }
    
    MV_FRAME_OUT_INFO_EX stImageInfo = {0};
    pImageBuf = new unsigned char[m_nPayloadSize];
    
    int ret = MV_CC_GetOneFrameTimeout(m_hCamera, pImageBuf, m_nPayloadSize, &stImageInfo, 1000);
    if (ret != MV_OK) {
        qWarning() << "获取图像帧失败，序列号:" << m_serialNumber 
                   << "错误码:" << QString("0x%1").arg(ret, 8, 16, QLatin1Char('0'));
        delete[] pImageBuf;
        pImageBuf = nullptr;
        return false;
    }
    
    nImageSize = stImageInfo.nFrameLen;
    nWidth = stImageInfo.nWidth;
    nHeight = stImageInfo.nHeight;
    
    qDebug() << "成功获取图像帧，序列号:" << m_serialNumber 
             << "尺寸:" << nWidth << "x" << nHeight 
             << "数据长度:" << nImageSize;
    
    return true;
}

void HikVisionCamera::releaseFrame()
{
    // 在getFrame中使用new分配的内存需要在这里释放
    // 但由于我们在getFrame中直接返回指针，
    // 释放操作需要在调用方进行
}

bool HikVisionCamera::isConnected() const
{
    QMutexLocker locker(&m_mutex);
    return m_isOpened;
}

QString HikVisionCamera::getSerialNumber() const
{
    return m_serialNumber;
}

bool HikVisionCamera::checkDeviceStatus()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_isOpened) {
        return false;
    }
    
    // 尝试获取设备信息来检查连接状态
    MVCC_INTVALUE stIntValue = {0};
    int ret = MV_CC_GetIntValue(m_hCamera, "PayloadSize", &stIntValue);
    return (ret == MV_OK);
}

bool HikVisionCamera::setExposureTime(float exposureTime)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_isOpened) {
        return false;
    }
    
    int ret = MV_CC_SetFloatValue(m_hCamera, "ExposureTime", exposureTime);
    return (ret == MV_OK);
}

float HikVisionCamera::getExposureTime()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_isOpened) {
        return 0.0f;
    }
    
    MVCC_FLOATVALUE stFloatValue = {0};
    int ret = MV_CC_GetFloatValue(m_hCamera, "ExposureTime", &stFloatValue);
    if (ret == MV_OK) {
        return stFloatValue.fCurValue;
    }
    
    return 0.0f;
}

bool HikVisionCamera::setPixelFormat(const QString& format)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_isOpened) {
        return false;
    }
    
    // 简化版本，这里可以根据需要扩展
    unsigned int pixelFormat = PixelType_Gvsp_Mono8; // 默认格式
    
    if (format == "RGB8") {
        pixelFormat = PixelType_Gvsp_RGB8_Packed;
    } else if (format == "Mono8") {
        pixelFormat = PixelType_Gvsp_Mono8;
    }
    
    int ret = MV_CC_SetEnumValue(m_hCamera, "PixelFormat", pixelFormat);
    return (ret == MV_OK);
}

QString HikVisionCamera::getPixelFormat()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_isOpened) {
        return QString();
    }
    
    MVCC_ENUMVALUE stEnumValue = {0};
    int ret = MV_CC_GetEnumValue(m_hCamera, "PixelFormat", &stEnumValue);
    if (ret == MV_OK) {
        switch (stEnumValue.nCurValue) {
            case PixelType_Gvsp_Mono8:
                return "Mono8";
            case PixelType_Gvsp_RGB8_Packed:
                return "RGB8";
            default:
                return "Unknown";
        }
    }
    
    return QString();
} 