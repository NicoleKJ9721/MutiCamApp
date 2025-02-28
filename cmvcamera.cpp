#include "cmvcamera.h"
#include <QDebug>

CMvCamera::CMvCamera()
{
    m_hDevHandle = NULL;
}

CMvCamera::~CMvCamera()
{
    if (m_hDevHandle)
    {
        MV_CC_DestroyHandle(m_hDevHandle);
        m_hDevHandle = NULL;
    }
}

// 获取SDK版本号
int CMvCamera::GetSDKVersion()
{
    return MV_CC_GetSDKVersion();
}

// 枚举设备
int CMvCamera::EnumDevices(unsigned int nTLayerType, MV_CC_DEVICE_INFO_LIST* pstDevList)
{
    return MV_CC_EnumDevices(nTLayerType, pstDevList);
}

// 判断设备是否可达
bool CMvCamera::IsDeviceAccessible(MV_CC_DEVICE_INFO* pstDevInfo, unsigned int nAccessMode)
{
    return MV_CC_IsDeviceAccessible(pstDevInfo, nAccessMode) == true;
}

// 打开设备
int CMvCamera::Open(MV_CC_DEVICE_INFO* pstDeviceInfo)
{
    if (NULL == pstDeviceInfo)
    {
        return MV_E_PARAMETER;
    }

    if (m_hDevHandle)
    {
        return MV_E_CALLORDER;
    }

    int nRet = MV_CC_CreateHandle(&m_hDevHandle, pstDeviceInfo);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    nRet = MV_CC_OpenDevice(m_hDevHandle);
    if (MV_OK != nRet)
    {
        MV_CC_DestroyHandle(m_hDevHandle);
        m_hDevHandle = NULL;
        return nRet;
    }

    return MV_OK;
}

// 关闭设备
int CMvCamera::Close()
{
    if (NULL == m_hDevHandle)
    {
        return MV_E_HANDLE;
    }

    MV_CC_CloseDevice(m_hDevHandle);

    int nRet = MV_CC_DestroyHandle(m_hDevHandle);
    m_hDevHandle = NULL;

    return nRet;
}

// 判断相机是否处于连接状态
bool CMvCamera::IsDeviceConnected()
{
    if (NULL == m_hDevHandle)
    {
        return false;
    }

    return MV_CC_IsDeviceConnected(m_hDevHandle);
}

// 注册图像数据回调
int CMvCamera::RegisterImageCallBack(void(__stdcall* cbOutput)(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser), void* pUser)
{
    if (NULL == m_hDevHandle)
    {
        return MV_E_HANDLE;
    }

    return MV_CC_RegisterImageCallBackEx(m_hDevHandle, cbOutput, pUser);
}

// 开启抓图
int CMvCamera::StartGrabbing()
{
    if (NULL == m_hDevHandle)
    {
        return MV_E_HANDLE;
    }

    return MV_CC_StartGrabbing(m_hDevHandle);
}

// 停止抓图
int CMvCamera::StopGrabbing()
{
    if (NULL == m_hDevHandle)
    {
        return MV_E_HANDLE;
    }

    return MV_CC_StopGrabbing(m_hDevHandle);
}

// 主动获取一帧图像数据
int CMvCamera::GetImageBuffer(MV_FRAME_OUT* pFrame, int nMsec)
{
    if (NULL == m_hDevHandle || NULL == pFrame)
    {
        return MV_E_PARAMETER;
    }

    return MV_CC_GetImageBuffer(m_hDevHandle, pFrame, nMsec);
}

// 释放图像缓存
int CMvCamera::FreeImageBuffer(MV_FRAME_OUT* pFrame)
{
    if (NULL == m_hDevHandle || NULL == pFrame)
    {
        return MV_E_PARAMETER;
    }

    return MV_CC_FreeImageBuffer(m_hDevHandle, pFrame);
}

// 显示一帧图像
int CMvCamera::DisplayOneFrame(MV_DISPLAY_FRAME_INFO* pDisplayInfo)
{
    if (NULL == m_hDevHandle || NULL == pDisplayInfo)
    {
        return MV_E_PARAMETER;
    }

    return MV_CC_DisplayOneFrame(m_hDevHandle, pDisplayInfo);
}

// 设置SDK内部图像缓存节点个数
int CMvCamera::SetImageNodeNum(unsigned int nNum)
{
    if (NULL == m_hDevHandle)
    {
        return MV_E_HANDLE;
    }

    return MV_CC_SetImageNodeNum(m_hDevHandle, nNum);
}

// 获取设备信息
int CMvCamera::GetDeviceInfo(MV_CC_DEVICE_INFO* pstDevInfo)
{
    if (NULL == m_hDevHandle || NULL == pstDevInfo)
    {
        return MV_E_PARAMETER;
    }

    return MV_CC_GetDeviceInfo(m_hDevHandle, pstDevInfo);
}

// 获取GEV相机的统计信息
int CMvCamera::GetGevAllMatchInfo(MV_MATCH_INFO_NET_DETECT* pMatchInfoNetDetect)
{
    if (NULL == m_hDevHandle || NULL == pMatchInfoNetDetect)
    {
        return MV_E_PARAMETER;
    }

    MV_ALL_MATCH_INFO struMatchInfo = {0};
    struMatchInfo.nType = MV_MATCH_TYPE_NET_DETECT;
    struMatchInfo.pInfo = pMatchInfoNetDetect;
    struMatchInfo.nInfoSize = sizeof(MV_MATCH_INFO_NET_DETECT);
    
    return MV_CC_GetAllMatchInfo(m_hDevHandle, &struMatchInfo);
}

// 获取U3V相机的统计信息
int CMvCamera::GetU3VAllMatchInfo(MV_MATCH_INFO_USB_DETECT* pMatchInfoUSBDetect)
{
    if (NULL == m_hDevHandle || NULL == pMatchInfoUSBDetect)
    {
        return MV_E_PARAMETER;
    }

    MV_ALL_MATCH_INFO struMatchInfo = {0};
    struMatchInfo.nType = MV_MATCH_TYPE_USB_DETECT;
    struMatchInfo.pInfo = pMatchInfoUSBDetect;
    struMatchInfo.nInfoSize = sizeof(MV_MATCH_INFO_USB_DETECT);
    
    return MV_CC_GetAllMatchInfo(m_hDevHandle, &struMatchInfo);
}

// 获取Int型参数
int CMvCamera::GetIntValue(IN const char* strKey, OUT MVCC_INTVALUE_EX *pIntValue)
{
    if (NULL == m_hDevHandle || NULL == strKey || NULL == pIntValue)
    {
        return MV_E_PARAMETER;
    }

    return MV_CC_GetIntValueEx(m_hDevHandle, strKey, pIntValue);
}

// 设置Int型参数
int CMvCamera::SetIntValue(IN const char* strKey, IN int64_t nValue)
{
    if (NULL == m_hDevHandle || NULL == strKey)
    {
        return MV_E_PARAMETER;
    }

    return MV_CC_SetIntValueEx(m_hDevHandle, strKey, nValue);
}

// 获取Enum型参数
int CMvCamera::GetEnumValue(IN const char* strKey, OUT MVCC_ENUMVALUE *pEnumValue)
{
    if (NULL == m_hDevHandle || NULL == strKey || NULL == pEnumValue)
    {
        return MV_E_PARAMETER;
    }

    return MV_CC_GetEnumValue(m_hDevHandle, strKey, pEnumValue);
}

// 设置Enum型参数
int CMvCamera::SetEnumValue(IN const char* strKey, IN unsigned int nValue)
{
    if (NULL == m_hDevHandle || NULL == strKey)
    {
        return MV_E_PARAMETER;
    }

    return MV_CC_SetEnumValue(m_hDevHandle, strKey, nValue);
}

// 设置Enum型参数（使用字符串）
int CMvCamera::SetEnumValueByString(IN const char* strKey, IN const char* sValue)
{
    if (NULL == m_hDevHandle || NULL == strKey)
    {
        return MV_E_PARAMETER;
    }

    return MV_CC_SetEnumValueByString(m_hDevHandle, strKey, sValue);
}

// 获取Enum型参数的符号
int CMvCamera::GetEnumEntrySymbolic(IN const char* strKey, IN MVCC_ENUMENTRY* pstEnumEntry)
{
    if (NULL == m_hDevHandle || NULL == strKey || NULL == pstEnumEntry)
    {
        return MV_E_PARAMETER;
    }

    return MV_CC_GetEnumEntrySymbolic(m_hDevHandle, strKey, pstEnumEntry);
}

// 获取Float型参数
int CMvCamera::GetFloatValue(IN const char* strKey, OUT MVCC_FLOATVALUE *pFloatValue)
{
    if (NULL == m_hDevHandle || NULL == strKey || NULL == pFloatValue)
    {
        return MV_E_PARAMETER;
    }

    return MV_CC_GetFloatValue(m_hDevHandle, strKey, pFloatValue);
}

// 设置Float型参数
int CMvCamera::SetFloatValue(IN const char* strKey, IN float fValue)
{
    if (NULL == m_hDevHandle || NULL == strKey)
    {
        return MV_E_PARAMETER;
    }

    return MV_CC_SetFloatValue(m_hDevHandle, strKey, fValue);
}

// 获取Bool型参数
int CMvCamera::GetBoolValue(IN const char* strKey, OUT bool *pbValue)
{
    if (NULL == m_hDevHandle || NULL == strKey || NULL == pbValue)
    {
        return MV_E_PARAMETER;
    }

    return MV_CC_GetBoolValue(m_hDevHandle, strKey, pbValue);
}

// 设置Bool型参数
int CMvCamera::SetBoolValue(IN const char* strKey, IN bool bValue)
{
    if (NULL == m_hDevHandle || NULL == strKey)
    {
        return MV_E_PARAMETER;
    }

    return MV_CC_SetBoolValue(m_hDevHandle, strKey, bValue);
}

// 获取String型参数
int CMvCamera::GetStringValue(IN const char* strKey, MVCC_STRINGVALUE *pStringValue)
{
    if (NULL == m_hDevHandle || NULL == strKey || NULL == pStringValue)
    {
        return MV_E_PARAMETER;
    }

    return MV_CC_GetStringValue(m_hDevHandle, strKey, pStringValue);
}

// 设置String型参数
int CMvCamera::SetStringValue(IN const char* strKey, IN const char* strValue)
{
    if (NULL == m_hDevHandle || NULL == strKey)
    {
        return MV_E_PARAMETER;
    }

    return MV_CC_SetStringValue(m_hDevHandle, strKey, strValue);
}

// 执行Command型命令
int CMvCamera::CommandExecute(IN const char* strKey)
{
    if (NULL == m_hDevHandle || NULL == strKey)
    {
        return MV_E_PARAMETER;
    }

    return MV_CC_SetCommandValue(m_hDevHandle, strKey);
}

// 探测网络最佳包大小
int CMvCamera::GetOptimalPacketSize(unsigned int* pOptimalPacketSize)
{
    if (NULL == m_hDevHandle || NULL == pOptimalPacketSize)
    {
        return MV_E_PARAMETER;
    }

    int nRet = MV_CC_GetOptimalPacketSize(m_hDevHandle);
    if (nRet < 0)
    {
        return nRet;
    }

    *pOptimalPacketSize = (unsigned int)nRet;

    return MV_OK;
}

// 读取图像到OpenCV的Mat对象
int CMvCamera::ReadBuffer(OUT Mat& image)
{
    if (NULL == m_hDevHandle)
    {
        return MV_E_HANDLE;
    }
    
    // 增加缓冲区大小，从4MB增加到16MB
    MV_FRAME_OUT_INFO_EX stImageInfo = {0};
    memset(&stImageInfo, 0, sizeof(MV_FRAME_OUT_INFO_EX));
    unsigned char* pData = (unsigned char*)malloc(sizeof(unsigned char) * 1024 * 1024 * 16);
    if (pData == NULL)
    {
        return MV_E_RESOURCE;
    }
    
    unsigned int nDataSize = 1024 * 1024 * 16;
    
    // 增加超时时间，从1000ms增加到2000ms
    int nRet = MV_CC_GetOneFrameTimeout(m_hDevHandle, pData, nDataSize, &stImageInfo, 2000);
    if (MV_OK != nRet)
    {
        free(pData);
        return nRet;
    }
    
    // 打印图像信息，帮助调试
    qDebug() << "获取到图像: 宽=" << stImageInfo.nWidth << " 高=" << stImageInfo.nHeight 
             << " 像素格式=" << stImageInfo.enPixelType << " 数据大小=" << stImageInfo.nFrameLen;
    
    // 转换为OpenCV格式
    try {
        if (stImageInfo.enPixelType == PixelType_Gvsp_Mono8)
        {
            // 黑白图像
            image = Mat(stImageInfo.nHeight, stImageInfo.nWidth, CV_8UC1, pData).clone();
        }
        else if (stImageInfo.enPixelType == PixelType_Gvsp_RGB8_Packed)
        {
            // RGB图像
            image = Mat(stImageInfo.nHeight, stImageInfo.nWidth, CV_8UC3, pData).clone();
        }
        else
        {
            // 其他格式，需要转换
            // 这里仅处理常见的几种格式，实际应用中可能需要处理更多格式
            if (stImageInfo.enPixelType == PixelType_Gvsp_BayerRG8 || 
                stImageInfo.enPixelType == PixelType_Gvsp_BayerGR8 ||
                stImageInfo.enPixelType == PixelType_Gvsp_BayerGB8 ||
                stImageInfo.enPixelType == PixelType_Gvsp_BayerBG8)
            {
                // 拜耳格式转RGB
                Mat src(stImageInfo.nHeight, stImageInfo.nWidth, CV_8UC1, pData);
                
                // 根据具体的拜耳格式选择不同的转换方式
                int cvtCode = COLOR_BayerRG2RGB;
                if (stImageInfo.enPixelType == PixelType_Gvsp_BayerRG8)
                    cvtCode = COLOR_BayerRG2RGB;
                else if (stImageInfo.enPixelType == PixelType_Gvsp_BayerGR8)
                    cvtCode = COLOR_BayerGR2RGB;
                else if (stImageInfo.enPixelType == PixelType_Gvsp_BayerGB8)
                    cvtCode = COLOR_BayerGB2RGB;
                else if (stImageInfo.enPixelType == PixelType_Gvsp_BayerBG8)
                    cvtCode = COLOR_BayerBG2RGB;
                
                cvtColor(src, image, cvtCode);
                image = image.clone(); // 确保数据被复制
            }
            else
            {
                // 对于虚拟相机，可能需要特殊处理
                // 尝试将数据作为RGB格式处理
                qDebug() << "尝试将未知格式作为RGB处理";
                if (stImageInfo.nFrameLen >= stImageInfo.nWidth * stImageInfo.nHeight * 3)
                {
                    image = Mat(stImageInfo.nHeight, stImageInfo.nWidth, CV_8UC3, pData).clone();
                }
                else if (stImageInfo.nFrameLen >= stImageInfo.nWidth * stImageInfo.nHeight)
                {
                    image = Mat(stImageInfo.nHeight, stImageInfo.nWidth, CV_8UC1, pData).clone();
                }
                else
                {
                    free(pData);
                    return MV_E_SUPPORT;
                }
            }
        }
    }
    catch (cv::Exception& e) {
        qDebug() << "OpenCV异常:" << e.what();
        free(pData);
        return MV_E_UNKNOW;
    }
    
    // 释放缓存
    free(pData);
    
    return MV_OK;
} 