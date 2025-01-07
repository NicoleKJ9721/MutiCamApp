/*

This program shows how to load MVFGControl.dll dynamically.

*/
#include <stdio.h>
#include <Windows.h>
#include <process.h>
#include <conio.h>
#include "MVFGDefines.h"
#include "MVFGErrorDefine.h"

#define BUFFER_NUMBER       3       // ch:申请的缓存个数 | en:Number of requested buffer
#define FILE_NAME_LEN       256     // ch:最大文件名长度 | en:Max of file name length
#define SAVE_IMAGE_NUM      10      // ch:最大存储图像数量 | en:Max number of stored images
#define TIMEOUT             1000    // ch:超时时间，单位毫秒 | en:Timeout, unit ms

typedef unsigned char*  (__stdcall * DLL_GetSDKVersion)         ();

typedef int             (__stdcall * DLL_UpdateInterfaceList)   (IN unsigned int nTLayerType, OUT bool8_t *pbChanged);
typedef int             (__stdcall * DLL_GetNumInterfaces)      (OUT unsigned int *pnNumIfaces);
typedef int             (__stdcall * DLL_GetInterfaceInfo)      (IN unsigned int nIndex, OUT MV_FG_INTERFACE_INFO *pstIfaceInfo);
typedef int             (__stdcall * DLL_OpenInterface)         (IN unsigned int nIndex, OUT IFHANDLE* phIface);
typedef int             (__stdcall * DLL_OpenInterfaceEx)       (IN unsigned int nIndex, IN int nAccess, OUT IFHANDLE* phIface);
typedef int             (__stdcall * DLL_CloseInterface)        (IN IFHANDLE hIface);
typedef int             (__stdcall * DLL_UpdateDeviceList)      (IN IFHANDLE hIface, OUT bool8_t *pbChanged);
typedef int             (__stdcall * DLL_GetNumDevices)         (IN IFHANDLE hIface, OUT unsigned int *pnNumDevices);
typedef int             (__stdcall * DLL_GetDeviceInfo)         (IN IFHANDLE hIface, IN unsigned int nIndex, OUT MV_FG_DEVICE_INFO *pstDevInfo);
typedef int             (__stdcall * DLL_OpenDevice)            (IN IFHANDLE hIface, IN unsigned int nIndex, OUT DEVHANDLE* phDevice);
typedef int             (__stdcall * DLL_CloseDevice)           (IN DEVHANDLE hDevice);
typedef int             (__stdcall * DLL_GetNumStreams)         (IN DEVHANDLE hDevice, OUT unsigned int *pnNumStreams);
typedef int             (__stdcall * DLL_OpenStream)            (IN DEVHANDLE hDevice, IN unsigned int nIndex, OUT STREAMHANDLE* phStream);
typedef int             (__stdcall * DLL_CloseStream)           (IN STREAMHANDLE hStream);
typedef int             (__stdcall * DLL_SetBufferNum)          (IN STREAMHANDLE hStream, IN unsigned int nBufferNum);
typedef int             (__stdcall * DLL_RegisterFrameCallBack) (IN STREAMHANDLE hStream, IN MV_FG_FrameCallBack cbFrame, IN void* pUser);
typedef int             (__stdcall * DLL_GetFrameBuffer)        (IN STREAMHANDLE hStream, OUT MV_FG_BUFFER_INFO* pstBufferInfo, IN unsigned int nTimeout);
typedef int             (__stdcall * DLL_ReleaseFrameBuffer)    (IN STREAMHANDLE hStream, IN MV_FG_BUFFER_INFO* pstBufferInfo);
typedef int             (__stdcall * DLL_GetBufferChunkData)    (IN STREAMHANDLE hStream, IN MV_FG_BUFFER_INFO* pstBufferInfo, IN unsigned int nIndex, OUT MV_FG_CHUNK_DATA_INFO* pstChunkDataInfo);
typedef int             (__stdcall * DLL_GetPayloadSize)        (IN STREAMHANDLE hStream, OUT unsigned int* pnPayloadSize);
typedef int             (__stdcall * DLL_AnnounceBuffer)        (IN STREAMHANDLE hStream, IN void *pBuffer, IN unsigned int nSize, IN void *pPrivate, OUT BUFFERHANDLE *phBuffer);
typedef int             (__stdcall * DLL_RevokeBuffer)          (IN STREAMHANDLE hStream, IN BUFFERHANDLE hBuffer, OUT void **pBuffer, OUT void **pPrivate);
typedef int             (__stdcall * DLL_FlushQueue)            (IN STREAMHANDLE hStream, IN MV_FG_BUFFER_QUEUE_TYPE enQueueType);
typedef int             (__stdcall * DLL_StartAcquisition)      (IN STREAMHANDLE hStream);
typedef int             (__stdcall * DLL_StopAcquisition)       (IN STREAMHANDLE hStream);
typedef int             (__stdcall * DLL_GetImageBuffer)        (IN STREAMHANDLE hStream, OUT BUFFERHANDLE *phBuffer, IN unsigned int nTimeout);
typedef int             (__stdcall * DLL_GetBufferInfo)         (IN BUFFERHANDLE hBuffer, OUT MV_FG_BUFFER_INFO* pstBufferInfo);
typedef int             (__stdcall * DLL_QueueBuffer)           (IN BUFFERHANDLE hBuffer);
typedef int             (__stdcall * DLL_GetXMLFile)            (IN PORTHANDLE hPort, IN OUT unsigned char* pData, IN unsigned int nDataSize, OUT unsigned int* pnDataLen);
typedef int             (__stdcall * DLL_GetNodeAccessMode)     (IN PORTHANDLE hPort, IN const char * strName, OUT MV_FG_NODE_ACCESS_MODE *penAccessMode);
typedef int             (__stdcall * DLL_GetNodeInterfaceType)  (IN PORTHANDLE hPort, IN const char * strName, OUT MV_FG_NODE_INTERFACE_TYPE *penInterfaceType);
typedef int             (__stdcall * DLL_GetIntValue)           (IN PORTHANDLE hPort, IN const char* strKey, OUT MV_FG_INTVALUE *pstIntValue);
typedef int             (__stdcall * DLL_SetIntValue)           (IN PORTHANDLE hPort, IN const char* strKey, IN int64_t nValue);
typedef int             (__stdcall * DLL_GetEnumValue)          (IN PORTHANDLE hPort, IN const char* strKey, OUT MV_FG_ENUMVALUE *pstEnumValue);
typedef int             (__stdcall * DLL_SetEnumValue)          (IN PORTHANDLE hPort, IN const char* strKey, IN unsigned int nValue);
typedef int             (__stdcall * DLL_SetEnumValueByString)  (IN PORTHANDLE hPort, IN const char* strKey, IN const char* strValue);
typedef int             (__stdcall * DLL_GetFloatValue)         (IN PORTHANDLE hPort, IN const char* strKey, OUT MV_FG_FLOATVALUE *pstFloatValue);
typedef int             (__stdcall * DLL_SetFloatValue)         (IN PORTHANDLE hPort, IN const char* strKey, IN float fValue);
typedef int             (__stdcall * DLL_GetBoolValue)          (IN PORTHANDLE hPort, IN const char* strKey, OUT bool8_t *pbValue);
typedef int             (__stdcall * DLL_SetBoolValue)          (IN PORTHANDLE hPort, IN const char* strKey, IN bool8_t bValue);
typedef int             (__stdcall * DLL_GetStringValue)        (IN PORTHANDLE hPort, IN const char* strKey, OUT MV_FG_STRINGVALUE *pstStringValue);
typedef int             (__stdcall * DLL_SetStringValue)        (IN PORTHANDLE hPort, IN const char* strKey, IN const char* strValue);
typedef int             (__stdcall * DLL_SetCommandValue)       (IN PORTHANDLE hPort, IN const char* strKey);
typedef int             (__stdcall * DLL_FeatureSave)           (IN PORTHANDLE hPort, IN const char* strFileName);
typedef int             (__stdcall * DLL_FeatureLoad)           (IN PORTHANDLE hPort, IN const char* strFileName);
typedef int             (__stdcall * DLL_RegisterExceptionCallBack)(IN PORTHANDLE hPort, IN MV_FG_ExceptionCallBack cbException, IN void* pUser);
typedef int             (__stdcall * DLL_ReleaseTLayerResource) (IN unsigned int nTLayerType);
typedef int				(__stdcall * DLL_Finalize)              ();

bool g_bExit = false;               // ch:停止取流 | en:Stop grabbing

// ch:线程参数 | en:Thread parameter
struct MultiThrParam
{
    void*       pUser;      // ch:用户传入的参数 | en:User param
    HINSTANCE   hDll;       // ch:Dll句柄 | en:Dll handle
};

// ch:等待按键输入 | en:Wait for key press
void WaitForKeyPress(void)
{
    while(!_kbhit())
    {
        Sleep(10);
    }
    _getch();
}

// ch:清空标准输入中的残留数据 | en:Flush stdin
void ClearStdin(void)
{
    char c = '\0';

    while (1)
    {
        c = getchar();
        if ('\n' == c || EOF == c)
        {
            break;
        }
    }
}

// ch:保存图像原始数据 | en:Save original image data
void SaveRawImage(int nImageNo, MV_FG_BUFFER_INFO* pstImageInfo)
{
    if (pstImageInfo)
    {
        char szFileName[FILE_NAME_LEN] = { 0 };

        sprintf_s(szFileName, FILE_NAME_LEN, "Image_w%d_h%d_n%d.raw", pstImageInfo->nWidth, pstImageInfo->nHeight, nImageNo);

        FILE* pImageFile = NULL;
        if ((0 != fopen_s(&pImageFile, szFileName, "wb")) || (NULL == pImageFile))
        {
            return;
        }

        fwrite(pstImageInfo->pBuffer, 1, pstImageInfo->nFilledSize, pImageFile);
        fclose(pImageFile);
    }
}

// ch:取流线程 | en:Grabbing image data thread
unsigned int __stdcall GrabbingThread(void* pUser)
{
    if (pUser)
    {
        MultiThrParam*      pstThreadParam  = (MultiThrParam*)pUser;
        MV_FG_BUFFER_INFO   stFrameInfo     = { 0 };        // 图像信息
        int                 nSaveImage      = 0;            // 保存的图像数量
        int                 nRet            = 0;

        DLL_StartAcquisition DLLStartAcquisition = (DLL_StartAcquisition)GetProcAddress(pstThreadParam->hDll, "MV_FG_StartAcquisition");
        DLL_GetFrameBuffer DLLGetFrameBuffer = (DLL_GetFrameBuffer)GetProcAddress(pstThreadParam->hDll, "MV_FG_GetFrameBuffer");
        DLL_ReleaseFrameBuffer DLLReleaseFrameBuffer = (DLL_ReleaseFrameBuffer)GetProcAddress(pstThreadParam->hDll, "MV_FG_ReleaseFrameBuffer");
        DLL_StopAcquisition DLLStopAcquisition = (DLL_StopAcquisition)GetProcAddress(pstThreadParam->hDll, "MV_FG_StopAcquisition");

        // ch:开始取流 | en:Start Acquisition
        nRet = DLLStartAcquisition(pstThreadParam->pUser);
        if (MV_FG_SUCCESS != nRet)
        {
            printf("Start acquistion failed! %#x\n", nRet);
            return nRet;
        }
        g_bExit = false;

        while (!g_bExit)
        {
            // ch:获取一帧图像缓存信息 | en:Get one frame buffer's info
            nRet = DLLGetFrameBuffer(pstThreadParam->pUser, &stFrameInfo, TIMEOUT);
            if (MV_FG_SUCCESS != nRet)
            {
                printf("Get frame buffer info failed! %#x\n", nRet);
                continue;
            }
            else
            {
                printf("FrameNumber:%2I64d%, Width:%d, Height:%d\n", stFrameInfo.nFrameID, stFrameInfo.nWidth, stFrameInfo.nHeight);

                if ((NULL != stFrameInfo.pBuffer) && (0 < stFrameInfo.nFilledSize) && (SAVE_IMAGE_NUM > nSaveImage))
                {
                    SaveRawImage(++nSaveImage, &stFrameInfo);
                }
            }

            // ch:将缓存放回输入队列 | en:Put the buffer back into the input queue
            nRet = DLLReleaseFrameBuffer(pstThreadParam->pUser, &stFrameInfo);
            if (MV_FG_SUCCESS != nRet)
            {
                printf("Release frame buffer failed! %#x\n", nRet);
                break;
            }
        }

        // ch:停止取流 | en:Stop Acquisition
        nRet = DLLStopAcquisition(pstThreadParam->pUser);
        if (MV_FG_SUCCESS != nRet)
        {
            printf("Stop acquisition failed! %#x\n", nRet);
            return nRet;
        }
    }

    return MV_FG_SUCCESS;
}

// ch:打印采集卡信息 | en:Print interface info
bool PrintInterfaceInfo(HINSTANCE hDll, unsigned int nInterfaceNum)
{
    if (NULL == hDll)
    {
        printf("Parameter error!\n");
        return false;
    }

    int nRet = 0;

    DLL_GetInterfaceInfo DLLGetInterfaceInfo = (DLL_GetInterfaceInfo)GetProcAddress(hDll, "MV_FG_GetInterfaceInfo");

    for (unsigned int i = 0; i < nInterfaceNum; i++)
    {
        MV_FG_INTERFACE_INFO stInterfaceInfo = { 0 };

        nRet = DLLGetInterfaceInfo(i, &stInterfaceInfo);
        if (MV_FG_SUCCESS != nRet)
        {
            printf("Get info of No.%d interface failed! %#x\n", i, nRet);
            return false;
        }

        switch (stInterfaceInfo.nTLayerType)
        {
        case MV_FG_CXP_INTERFACE:
            {
                printf("[CXP]No.%d Interface: \n\tDisplayName: %s\n\tInterfaceID: %s\n\tSerialNumber:%s\n", i, 
                    stInterfaceInfo.IfaceInfo.stCXPIfaceInfo.chDisplayName, 
                    stInterfaceInfo.IfaceInfo.stCXPIfaceInfo.chInterfaceID, 
                    stInterfaceInfo.IfaceInfo.stCXPIfaceInfo.chSerialNumber);
                break;
            }
        case MV_FG_GEV_INTERFACE:
            {
                printf("[GEV]No.%d Interface: \n\tDisplayName: %s\n\tInterfaceID: %s\n\tSerialNumber:%s\n", i, 
                    stInterfaceInfo.IfaceInfo.stGEVIfaceInfo.chDisplayName, 
                    stInterfaceInfo.IfaceInfo.stGEVIfaceInfo.chInterfaceID, 
                    stInterfaceInfo.IfaceInfo.stGEVIfaceInfo.chSerialNumber);
                break;
            }
        case MV_FG_CAMERALINK_INTERFACE:
            {
                printf("[CML]No.%d Interface: \n\tDisplayName: %s\n\tInterfaceID: %s\n\tSerialNumber:%s\n", i, 
                    stInterfaceInfo.IfaceInfo.stCMLIfaceInfo.chDisplayName, 
                    stInterfaceInfo.IfaceInfo.stCMLIfaceInfo.chInterfaceID, 
                    stInterfaceInfo.IfaceInfo.stCMLIfaceInfo.chSerialNumber);
                break;
            }
        case MV_FG_XoF_INTERFACE:
            {
                printf("[XoF]No.%d Interface: \n\tDisplayName: %s\n\tInterfaceID: %s\n\tSerialNumber:%s\n", i, 
                    stInterfaceInfo.IfaceInfo.stXoFIfaceInfo.chDisplayName, 
                    stInterfaceInfo.IfaceInfo.stXoFIfaceInfo.chInterfaceID, 
                    stInterfaceInfo.IfaceInfo.stXoFIfaceInfo.chSerialNumber);
                break;
            }
        default:
            {
                printf("Unknown interface type.\n");
                return false;
            }
        }
    }

    return true;
}

// ch:打印设备信息 | en:Print device info
bool PrintDeviceInfo(HINSTANCE hDll, IFHANDLE hInterface, unsigned int nDeviceNum)
{
    if (NULL == hDll)
    {
        printf("Parameter error!\n");
        return false;
    }

    int nRet = 0;

    DLL_GetDeviceInfo DLLGetDeviceInfo = (DLL_GetDeviceInfo)GetProcAddress(hDll, "MV_FG_GetDeviceInfo");

    for (unsigned int i = 0; i < nDeviceNum; i++)
    {
        MV_FG_DEVICE_INFO stDeviceInfo = { 0 };

        nRet = DLLGetDeviceInfo(hInterface, i, &stDeviceInfo);
        if (MV_FG_SUCCESS != nRet)
        {
            printf("Get info of No.%d device failed! %#x\n", i, nRet);
            return false;
        }

        switch (stDeviceInfo.nDevType)
        {
        case MV_FG_CXP_DEVICE:
            {
                printf("[CXP]No.%d Device: \n\tUserDefinedName: %s\n\tModelName: %s\n\tSerialNumber: %s\n", i, 
                    stDeviceInfo.DevInfo.stCXPDevInfo.chUserDefinedName,
                    stDeviceInfo.DevInfo.stCXPDevInfo.chModelName,
                    stDeviceInfo.DevInfo.stCXPDevInfo.chSerialNumber);
                break;
            }
        case MV_FG_GEV_DEVICE:
            {
                printf("[GEV]No.%d Device: \n\tUserDefinedName: %s\n\tModelName: %s\n\tSerialNumber: %s\n", i, 
                    stDeviceInfo.DevInfo.stGEVDevInfo.chUserDefinedName,
                    stDeviceInfo.DevInfo.stGEVDevInfo.chModelName,
                    stDeviceInfo.DevInfo.stGEVDevInfo.chSerialNumber);
                break;
            }
        case MV_FG_CAMERALINK_DEVICE:
            {
                printf("[CML]No.%d Device: \n\tUserDefinedName: %s\n\tModelName: %s\n\tSerialNumber: %s\n", i, 
                    stDeviceInfo.DevInfo.stCMLDevInfo.chUserDefinedName,
                    stDeviceInfo.DevInfo.stCMLDevInfo.chModelName,
                    stDeviceInfo.DevInfo.stCMLDevInfo.chSerialNumber);
                break;
            }
        case MV_FG_XoF_DEVICE:
            {
                printf("[XoF]No.%d Device: \n\tUserDefinedName: %s\n\tModelName: %s\n\tSerialNumber: %s\n", i, 
                    stDeviceInfo.DevInfo.stXoFDevInfo.chUserDefinedName,
                    stDeviceInfo.DevInfo.stXoFDevInfo.chModelName,
                    stDeviceInfo.DevInfo.stXoFDevInfo.chSerialNumber);
                break;
            }
        default:
            {
                printf("Unknown device type.\n");
                return false;
            }
        }
    }

    return true;
}

int main()
{
    HINSTANCE   MVFGCtrlDll = NULL;     // ch:MVFGControl.dll的句柄 | en:Handle of MVFGControl.dll

    // ch:以默认方式加载动态库 | en:Load dynamic libraries by default
    MVFGCtrlDll = LoadLibrary("MVFGControl.dll");
    if (NULL == MVFGCtrlDll)
    {
        DWORD errCode = GetLastError();
        printf("Error code! [%ld]\n",errCode);
        printf("Press any key to exit.\n");
        WaitForKeyPress();
        return -1;
    }

    int             nRet = 0;
    IFHANDLE        hInterface = NULL;
    DEVHANDLE       hDevice = NULL;
    STREAMHANDLE    hStream = NULL;

    do 
    {
        // ch:枚举采集卡 | en:Enum interface
        bool bChanged = false;
        DLL_UpdateInterfaceList DLLUpdateInterfaceList = (DLL_UpdateInterfaceList)GetProcAddress(MVFGCtrlDll, "MV_FG_UpdateInterfaceList");
        nRet = DLLUpdateInterfaceList(MV_FG_CXP_INTERFACE | MV_FG_GEV_INTERFACE | MV_FG_CAMERALINK_INTERFACE | MV_FG_XoF_INTERFACE, &bChanged);
        if (MV_FG_SUCCESS != nRet)
        {
            printf("Update interface list failed! %#x\n", nRet);
            break;
        }

        // ch:获取采集卡数量 | en:Get interface num
        unsigned int nInterfaceNum = 0;
        DLL_GetNumInterfaces DLLGetNumInterfaces = (DLL_GetNumInterfaces)GetProcAddress(MVFGCtrlDll, "MV_FG_GetNumInterfaces");
        nRet = DLLGetNumInterfaces(&nInterfaceNum);
        if (MV_FG_SUCCESS != nRet || 0 == nInterfaceNum)
        {
            printf("No interface found! return = %d, number = %d\n", nRet, nInterfaceNum);
            break;
        }

        // ch:显示采集卡信息 | en:Show interface info
        if (false == PrintInterfaceInfo(MVFGCtrlDll, nInterfaceNum))
        {
            break;
        }

        // ch:选择采集卡 | en:Select interface
        int nInterfaceIndex = -1;
        printf("Select an interface: ");
        scanf_s("%d", &nInterfaceIndex);
        ClearStdin();

        if (nInterfaceIndex < 0 || nInterfaceIndex >= (int)nInterfaceNum)
        {
            printf("Invalid interface index.\nQuit.\n");
            break;
        }

        // ch:以指定权限打开采集卡，获得采集卡句柄 | en:Open interface with specified permissions, get handle
        DLL_OpenInterfaceEx DLLOpenInterfaceEx = (DLL_OpenInterfaceEx)GetProcAddress(MVFGCtrlDll, "MV_FG_OpenInterfaceEx");
        nRet = DLLOpenInterfaceEx((unsigned int)nInterfaceIndex, MV_FG_ACCESS_CONTROL, &hInterface);
        if (MV_FG_SUCCESS != nRet)
        {
            printf("Open No.%d interface failed! %#x\n", nInterfaceIndex, nRet);
            break;
        }

        // ch:枚举采集卡上的相机 | en:Enum camera of interface
        DLL_UpdateDeviceList DLLUpdateDeviceList = (DLL_UpdateDeviceList)GetProcAddress(MVFGCtrlDll, "MV_FG_UpdateDeviceList");
        nRet = DLLUpdateDeviceList(hInterface, &bChanged);
        if (MV_FG_SUCCESS != nRet)
        {
            printf("Update device list failed! %#x\n", nRet);
            break;
        }

        // ch:获取设备数量 | en:Get device number
        unsigned int nDeviceNum = 0;
        DLL_GetNumDevices DLLGetNumDevices = (DLL_GetNumDevices)GetProcAddress(MVFGCtrlDll, "MV_FG_GetNumDevices");
        nRet = DLLGetNumDevices(hInterface, &nDeviceNum);
        if (MV_FG_SUCCESS != nRet || 0 == nDeviceNum)
        {
            printf("No device found! return = %d, number = %d\n", nRet, nDeviceNum);
            break;
        }

        // ch:显示设备信息 | en:Show device info
        if (false == PrintDeviceInfo(MVFGCtrlDll, hInterface, nDeviceNum))
        {
            break;
        }

        // ch:选择设备 | en:Select device
        int nDeviceIndex = -1;
        printf("Select a device: ");
        scanf_s("%d", &nDeviceIndex);
        ClearStdin();

        if (nDeviceIndex < 0 || nDeviceIndex >= (int)nDeviceNum)
        {
            printf("Invalid device index.\nQuit.\n");
            break;
        }

        // ch:打开设备，获得设备句柄 | en:Open device, get handle
        DLL_OpenDevice DLLOpenDevice = (DLL_OpenDevice)GetProcAddress(MVFGCtrlDll, "MV_FG_OpenDevice");
        nRet = DLLOpenDevice(hInterface, (unsigned int)nDeviceIndex, &hDevice);
        if (MV_FG_SUCCESS != nRet)
        {
            printf("Open No.%d device failed! %#x\n", nDeviceIndex, nRet);
            hDevice = NULL;
            break;
        }

        // ch:关闭触发模式 | en:Close trigger mode
        DLL_SetEnumValueByString DLLSetEnumValueByString = (DLL_SetEnumValueByString)GetProcAddress(MVFGCtrlDll, "MV_FG_SetEnumValueByString");
        nRet = DLLSetEnumValueByString(hDevice, "TriggerMode", "Off");
        if (MV_FG_SUCCESS != nRet)
        {
            printf("Turn off trigger mode failed! %#x\n", nRet);
            break;
        }

        // ch:获取流通道个数 | en:Get number of stream
        unsigned int nStreamNum = 0;
        DLL_GetNumStreams DLLGetNumStreams = (DLL_GetNumStreams)GetProcAddress(MVFGCtrlDll, "MV_FG_GetNumStreams");
        nRet = DLLGetNumStreams(hDevice, &nStreamNum);
        if (MV_FG_SUCCESS != nRet || 0 == nStreamNum)
        {
            printf("No stream available! return = %d, number = %d\n", nRet, nStreamNum);
            break;
        }

        // ch:打开流通道(目前只支持单个通道) | en:Open stream(Only a single stream is supported now)
        DLL_OpenStream DLLOpenStream = (DLL_OpenStream)GetProcAddress(MVFGCtrlDll, "MV_FG_OpenStream");
        nRet = DLLOpenStream(hDevice, 0, &hStream);
        if (MV_FG_SUCCESS != nRet)
        {
            printf("Open stream failed! %#x\n", nRet);
            break;
        }

        // ch:设置SDK内部缓存数量 | en:Set internal buffer number
        DLL_SetBufferNum DLLSetBufferNum = (DLL_SetBufferNum)GetProcAddress(MVFGCtrlDll, "MV_FG_SetBufferNum");
        nRet = DLLSetBufferNum(hStream, BUFFER_NUMBER);
        if (MV_FG_SUCCESS != nRet)
        {
            printf("Set buffer number failed! %#x\n", nRet);
            break;
        }

        // ch:创建取流线程 | en:Create acquistion thread
        MultiThrParam stThreadParam = { 0 };
        stThreadParam.pUser = hStream;
        stThreadParam.hDll  = MVFGCtrlDll;
        void* hThreadHandle = (void*)_beginthreadex(NULL, 0, GrabbingThread, (void*)&stThreadParam, 0, NULL);
        if (NULL == hThreadHandle)
        {
            printf("Create thread failed!\n");
            break;
        }

        printf("Press any key to stop acquisition.\n");
        WaitForKeyPress();

        // ch:关闭取流线程 | en:Close acquistion thread
        g_bExit = true;
        WaitForSingleObject(hThreadHandle, INFINITE);
        CloseHandle(hThreadHandle);
        hThreadHandle = NULL;
    } while (0);

    // ch:关闭流通道 | en:Close Stream
    if (NULL != hStream)
    {
        DLL_CloseStream DLLCloseStream = (DLL_CloseStream)GetProcAddress(MVFGCtrlDll, "MV_FG_CloseStream");
        nRet = DLLCloseStream(hStream);
        if (MV_FG_SUCCESS != nRet)
        {
            printf("Close stream failed! %#x\n", nRet);
        }
        hStream = NULL;
    }

    // ch:关闭设备 | en:Close device
    if (NULL != hDevice)
    {
        DLL_CloseDevice DLLCloseDevice = (DLL_CloseDevice)GetProcAddress(MVFGCtrlDll, "MV_FG_CloseDevice");
        nRet = DLLCloseDevice(hDevice);
        if (MV_FG_SUCCESS != nRet)
        {
            printf("Close device failed! %#x\n", nRet);
        }
        hDevice = NULL;
    }

    // ch:关闭采集卡 | en:Close interface
    if (NULL != hInterface)
    {
        DLL_CloseInterface DLLCloseInterface = (DLL_CloseInterface)GetProcAddress(MVFGCtrlDll, "MV_FG_CloseInterface");
        nRet = DLLCloseInterface(hInterface);
        if (MV_FG_SUCCESS != nRet)
        {
            printf("Close interface failed! %#x\n", nRet);
        }
        hInterface = NULL;
    }

    DLL_ReleaseTLayerResource DLLReleaseTLayerResource = (DLL_ReleaseTLayerResource)GetProcAddress(MVFGCtrlDll, "MV_FG_ReleaseTLayerResource");
    DLLReleaseTLayerResource(MV_FG_CXP_INTERFACE | MV_FG_GEV_INTERFACE | MV_FG_CAMERALINK_INTERFACE | MV_FG_XoF_INTERFACE);

	DLL_Finalize DLLFinalize = (DLL_Finalize)GetProcAddress(MVFGCtrlDll, "MV_FG_Finalize");
	DLLFinalize();
	
	FreeLibrary(MVFGCtrlDll);

    printf("Press any key to exit.\n");
    WaitForKeyPress();

    return 0;
}
