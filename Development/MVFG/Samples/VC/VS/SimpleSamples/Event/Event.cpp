/*

This program shows how to use the MVFGControl method to register and get specified events.

*/
#include <Windows.h>
#include <conio.h>
#include <stdio.h>
#include <process.h>
#include "MVFGControl.h"

bool g_bExit = FALSE;
#define BUFFER_NUMBER           3       // 申请的缓存个数

static int nEventNum = 0;

void __stdcall EventCallBack(MV_FG_EVENT_INFO* pstEventInfo, void* pUser)
{
    nEventNum++;

    if (NULL != pstEventInfo)
    {
        printf("%d Event: name %s  id 0x%x time %lld \r\n", nEventNum, pstEventInfo->EventName, pstEventInfo->nEventID, pstEventInfo->nTimestamp);
    }
}

void __stdcall ImageCallBack(MV_FG_BUFFER_INFO* pstBufferInfo, void* pUser)
{
    if (NULL != pstBufferInfo)
    {
        printf("FrameNumber:%2I64d%, Width:%d, Height:%d, nDevTimeStamp[%I64d]\n", pstBufferInfo->nFrameID, pstBufferInfo->nWidth, pstBufferInfo->nHeight, pstBufferInfo->nDevTimeStamp);
    }
}

// 等待按键输入
void WaitForKeyPress(void)
{
    while(!_kbhit())
    {
        Sleep(10);
    }
    _getch();
}

// 清空标准输入中的残留数据
void ClearStdin(void)
{
    char c = '0';
    
    do
    {
        c = getchar();
        if (c == '\n' ||c == EOF)
        {
            break;
        }
    }
    while(TRUE);
}

int main(int argc, char** argv)
{
    int             nRet = MV_FG_SUCCESS;
    IFHANDLE        hInterface = NULL;
    DEVHANDLE       hDevice = NULL;
    STREAMHANDLE    hStream = NULL;

    do 
    {
        // 枚举采集卡
        bool bChanged = false;
        nRet = MV_FG_UpdateInterfaceList(MV_FG_CXP_INTERFACE |  MV_FG_GEV_INTERFACE | MV_FG_CAMERALINK_INTERFACE | MV_FG_XoF_INTERFACE, &bChanged);
        if (MV_FG_SUCCESS != nRet)
        {
            printf("Update Interface List error, %#x\n", nRet);
            break;
        }

        // 获取采集卡个数
        unsigned int nInterfaceNumber = 0;
        nRet = MV_FG_GetNumInterfaces(&nInterfaceNumber);
        if (MV_FG_SUCCESS != nRet || 0 == nInterfaceNumber)
        {
            printf("No Interface found\n");
            break;
        }

        // 打印采集卡信息
        for (unsigned int i = 0; i < nInterfaceNumber; i++)
        {
            MV_FG_INTERFACE_INFO stInterfaceInfo = {0};

            nRet = MV_FG_GetInterfaceInfo(i, &stInterfaceInfo);
            if (MV_FG_SUCCESS != nRet)
            {
                printf("Get info of No.%d Interface error, %#x\n", i, nRet);
                break;
            }

            if (stInterfaceInfo.nTLayerType == MV_FG_CXP_INTERFACE)
            {
                printf("[CXP]No.%d Interface: \n\tDisplayName: %s\n\tInterfaceID: %s\n\tSerialNumber:%s\n", i, 
                    stInterfaceInfo.IfaceInfo.stCXPIfaceInfo.chDisplayName, 
                    stInterfaceInfo.IfaceInfo.stCXPIfaceInfo.chInterfaceID, 
                    stInterfaceInfo.IfaceInfo.stCXPIfaceInfo.chSerialNumber);
            }
            else if (stInterfaceInfo.nTLayerType == MV_FG_GEV_INTERFACE)
            {
                printf("[GEV]No.%d Interface: \n\tDisplayName: %s\n\tInterfaceID: %s\n\tSerialNumber:%s\n", i, 
                    stInterfaceInfo.IfaceInfo.stGEVIfaceInfo.chDisplayName, 
                    stInterfaceInfo.IfaceInfo.stGEVIfaceInfo.chInterfaceID, 
                    stInterfaceInfo.IfaceInfo.stGEVIfaceInfo.chSerialNumber);
            }
            else if (stInterfaceInfo.nTLayerType == MV_FG_CAMERALINK_INTERFACE)
            {
                printf("[CML]No.%d Interface: \n\tDisplayName: %s\n\tInterfaceID: %s\n\tSerialNumber:%s\n", i, 
                    stInterfaceInfo.IfaceInfo.stCMLIfaceInfo.chDisplayName, 
                    stInterfaceInfo.IfaceInfo.stCMLIfaceInfo.chInterfaceID, 
                    stInterfaceInfo.IfaceInfo.stCMLIfaceInfo.chSerialNumber);
            }
            else if (stInterfaceInfo.nTLayerType == MV_FG_XoF_INTERFACE)
            {
                printf("[XoF]No.%d Interface: \n\tDisplayName: %s\n\tInterfaceID: %s\n\tSerialNumber:%s\n", i, 
                    stInterfaceInfo.IfaceInfo.stXoFIfaceInfo.chDisplayName, 
                    stInterfaceInfo.IfaceInfo.stXoFIfaceInfo.chInterfaceID, 
                    stInterfaceInfo.IfaceInfo.stXoFIfaceInfo.chSerialNumber);
            }
        }
        if (MV_FG_SUCCESS != nRet)
        {
            break;
        }

        // 让用户选择采集卡，获取采集卡下标
        int nSelectedInterfaceIndex = -1;
        printf("Select an interface: ");
        scanf_s("%d", &nSelectedInterfaceIndex);
        ClearStdin();

        if ((nSelectedInterfaceIndex < 0) || (nSelectedInterfaceIndex >= (int)nInterfaceNumber))
        {
            printf("invalid interface index, Quit\n");
            break;
        }

        // 打开采集卡，返回采集卡句柄
        nRet = MV_FG_OpenInterface(nSelectedInterfaceIndex, &hInterface);
        if (MV_FG_SUCCESS != nRet)
        {
            printf("Open No.%d Interface error, %#x\n", nSelectedInterfaceIndex, nRet);
            break;
        }

        // 注册采集卡帧开始事件回调
        nRet = MV_FG_RegisterEventCallBack(hInterface, "ReceiveImageFrameStart0", EventCallBack, NULL);
        if (MV_FG_SUCCESS != nRet)
        {
            printf("MV_FG_RegisterEventCallBack event %s error, %#x\n", "ReceiveImageFrameStart0", nRet);
            break;
        }

        // 设置事件类型为流事件(可自行设置EventCategory相机节点下的其他事件类型)
        nRet = MV_FG_SetEnumValueByString(hInterface, "EventCategory", "StreamEvent");
        if (MV_FG_SUCCESS != nRet)
        {
            printf("MV_FG_SetEnumValueByString EventCategory %s error, %#x\n", "StreamEvent", nRet);
            break;
        }

        // 设置事件通道(Channel0-Channel3)
        nRet = MV_FG_SetEnumValueByString(hInterface, "ChannelSelector", "Channel0");
        if (MV_FG_SUCCESS != nRet)
        {
            printf("MV_FG_SetEnumValueByString ChannelSelector %s error, %#x\n", "Channel0", nRet);
            break;
        }

        // 设置具体事件(可自行设置EventSelector相机节点下的其他具体事件)
        nRet = MV_FG_SetEnumValueByString(hInterface, "EventSelector", "ReceiveImageFrameStart0");
        if (MV_FG_SUCCESS != nRet)
        {
            printf("MV_FG_SetEnumValueByString EventSelector %s error, %#x\n", "ReceiveImageFrameStart0", nRet);
            break;
        }

        // 打开事件通知
        nRet = MV_FG_SetEnumValueByString(hInterface, "EventNotification", "On");
        if (MV_FG_SUCCESS != nRet)
        {
            printf("MV_FG_SetEnumValueByString EventNotification %s error, %#x\n", "On", nRet);
            break;
        }

        // 枚举采集卡上的相机
        nRet = MV_FG_UpdateDeviceList(hInterface, &bChanged);
        if (MV_FG_SUCCESS != nRet)
        {
            printf("Update Device list error, %#x\n", nRet);
            break;
        }

        // 获取并打印设备信息
        unsigned int nDeviceNumber = 0;
        nRet = MV_FG_GetNumDevices(hInterface, &nDeviceNumber);
        if (MV_FG_SUCCESS != nRet || 0 == nDeviceNumber)
        {
            printf("No devices found, %#x\n", nRet);
            break;
        }

        for (unsigned int i = 0; i < nDeviceNumber; i++)
        {
            MV_FG_DEVICE_INFO stDeviceInfo = {0};

            nRet = MV_FG_GetDeviceInfo(hInterface, i, &stDeviceInfo);
            if (MV_FG_SUCCESS != nRet)
            {
                printf("Get Info of No.%u Device error, %#x\n", i, nRet);
                break;
            }

            if (stDeviceInfo.nDevType == MV_FG_CXP_DEVICE)
            {
                printf("[CXP]No.%d Device: \n\tUserDefinedName: %s\n\tModelName: %s\n\tSerialNumber: %s\n", i, 
                    stDeviceInfo.DevInfo.stCXPDevInfo.chUserDefinedName,
                    stDeviceInfo.DevInfo.stCXPDevInfo.chModelName,
                    stDeviceInfo.DevInfo.stCXPDevInfo.chSerialNumber);
            }
            else if (stDeviceInfo.nDevType == MV_FG_GEV_DEVICE)
            {
                printf("[GEV]No.%d Device: \n\tUserDefinedName: %s\n\tModelName: %s\n\tSerialNumber: %s\n", i, 
                    stDeviceInfo.DevInfo.stGEVDevInfo.chUserDefinedName,
                    stDeviceInfo.DevInfo.stGEVDevInfo.chModelName,
                    stDeviceInfo.DevInfo.stGEVDevInfo.chSerialNumber);
            }
            else if (stDeviceInfo.nDevType == MV_FG_CAMERALINK_DEVICE)
            {
                printf("[CML]No.%d Device: \n\tUserDefinedName: %s\n\tModelName: %s\n\tSerialNumber: %s\n", i, 
                    stDeviceInfo.DevInfo.stCMLDevInfo.chUserDefinedName,
                    stDeviceInfo.DevInfo.stCMLDevInfo.chModelName,
                    stDeviceInfo.DevInfo.stCMLDevInfo.chSerialNumber);
            }
            else if (stDeviceInfo.nDevType == MV_FG_XoF_DEVICE)
            {
                printf("[XoF]No.%d Device: \n\tUserDefinedName: %s\n\tModelName: %s\n\tSerialNumber: %s\n", i, 
                    stDeviceInfo.DevInfo.stXoFDevInfo.chUserDefinedName,
                    stDeviceInfo.DevInfo.stXoFDevInfo.chModelName,
                    stDeviceInfo.DevInfo.stXoFDevInfo.chSerialNumber);
            }
        }
        if (MV_FG_SUCCESS != nRet)
        {
            break;
        }

        // 让用户选择设备
        int nSelectedDeviceIndex = -1;
        printf("Select a device: ");
        scanf_s("%d", &nSelectedDeviceIndex);
        ClearStdin();

        if ((nSelectedDeviceIndex < 0) || (nSelectedDeviceIndex >= (int)nDeviceNumber))
        {
            printf("invalid device index, Quit\n");
            break;
        }

        // 打开设备，获取设备句柄
        nRet = MV_FG_OpenDevice(hInterface, nSelectedDeviceIndex, &hDevice);
        if (MV_FG_SUCCESS != nRet)
        {
            printf("Open device error, %#x\n", nRet);
            hDevice = NULL;
            break;
        }

        // 关闭触发模式
        nRet = MV_FG_SetEnumValueByString(hDevice, "TriggerMode", "Off");
        if (MV_FG_SUCCESS != nRet)
        {
            printf("Turn off TriggerMode failed, %#x\n", nRet);
            break;
        }

        // 获取流通道个数
        unsigned int nStreamNumber = 0;
        nRet = MV_FG_GetNumStreams(hDevice, &nStreamNumber);
        if (MV_FG_SUCCESS != nRet || 0 == nStreamNumber)
        {
            printf("No Stream available\n");
            break;
        }

        // 打开流通道(目前只支持单个流通道)
        nRet = MV_FG_OpenStream(hDevice, 0, &hStream);
        if (MV_FG_SUCCESS != nRet)
        {
            printf("Open Stream error, %#x\n", nRet);
            break;
        }

        // 设置取图缓存节点个数
        nRet = MV_FG_SetBufferNum(hStream, BUFFER_NUMBER);
        if (MV_FG_SUCCESS != nRet)
        {
            printf("Set buffer number error, %#x\n", nRet);
            break;
        }

        // 注册图像事件
        nRet = MV_FG_RegisterFrameCallBack(hStream, ImageCallBack, &hStream);
        if (MV_FG_SUCCESS != nRet)
        {
            printf("Register frame callback error, %#x\n", nRet);
            break;
        }

        // 开始取图
        nRet = MV_FG_StartAcquisition(hStream);
        if (MV_FG_SUCCESS != nRet)
        {
            printf("Start acquisition error, %#x\n", nRet);
            break;
        }

        printf("Press any key to stop acquisition.\n");
        WaitForKeyPress();

        // 停止取图
        nRet = MV_FG_StopAcquisition(hStream);
        if (MV_FG_SUCCESS != nRet)
        {
            printf("Stop acquisition error, %#x\n", nRet);
            break;
        }

        g_bExit = true;
    } while (0);

    // 释放相关资源
    if (NULL != hStream)
    {
        // 关闭流通道
        nRet = MV_FG_CloseStream(hStream);
        if (MV_FG_SUCCESS != nRet)
        {
            printf("Close Stream error, %#x\n", nRet);
        }
        hStream = NULL;
    }

    // 关闭设备
    if (NULL != hDevice)
    {
        nRet = MV_FG_CloseDevice(hDevice);
        if (MV_FG_SUCCESS != nRet)
        {
            printf("Close device error, %#x\n", nRet);
        }
        hDevice = NULL;
    }

    // 关闭采集卡
    if (NULL != hInterface)
    {
        nRet = MV_FG_CloseInterface(hInterface);
        if (MV_FG_SUCCESS != nRet)
        {
            printf("Close Interface error, %#x\n", nRet);
        }
        hInterface = NULL;
    }

    printf("Press any key to exit.\n");
    WaitForKeyPress();

    return 0;
}
