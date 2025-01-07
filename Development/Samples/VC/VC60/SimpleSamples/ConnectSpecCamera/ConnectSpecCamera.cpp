#include <stdio.h>
#include <Windows.h>
#include <process.h>
#include <conio.h>
#include "MvSdkExport.h"
#pragma comment(lib, "MvSdkExport.lib")

bool g_bExit = false;

// ch:等待按键输入 | en:Wait for key press
void WaitForKeyPress(void)
{
    while(!_kbhit())
    {
        Sleep(10);
    }
    _getch();
}

DWORD _stdcall WorkThread(void* pUser)
{
    int nRet = MV_OK;
    MV_FRAME_OUT stImageInfo = {0};

    while(1)
    {
        nRet = MV_CC_GetImageBuffer(pUser, &stImageInfo, 1000);
        if (nRet == MV_OK)
        {
            printf("Get Image Buffer: Width[%d], Height[%d], FrameNum[%d]\n", 
                stImageInfo.stFrameInfo.nWidth, stImageInfo.stFrameInfo.nHeight, stImageInfo.stFrameInfo.nFrameNum);

            nRet = MV_CC_FreeImageBuffer(pUser, &stImageInfo);
            if(nRet != MV_OK)
            {
                printf("Free Image Buffer fail! nRet [0x%x]\n", nRet);
            }
        }
        else
        {
			printf("Get Image fail! nRet [0x%x]\n", nRet);
        }
        if(g_bExit)
        {
            break;
        }
    }

    return 0;
}

int main()
{
    int nRet = MV_OK;
    void* handle = NULL;
    MV_CC_DEVICE_INFO stDevInfo = {0};
    MV_GIGE_DEVICE_INFO stGigEDev = {0};
    
    // ch:需要连接的相机ip(根据实际填充) | en:The camera IP that needs to be connected (based on actual padding)
    printf("Please input Current Camera Ip : ");
    char nCurrentIp[128];
    scanf("%s", &nCurrentIp);
    // ch:相机对应的网卡ip(根据实际填充) | en:The pc IP that needs to be connected (based on actual padding)
    printf("Please input Net Export Ip : ");
    char nNetExport[128];
    scanf("%s", &nNetExport);
    unsigned int nIp1, nIp2, nIp3, nIp4, nIp;

    sscanf(nCurrentIp, "%d.%d.%d.%d", &nIp1, &nIp2, &nIp3, &nIp4);
    nIp = (nIp1 << 24) | (nIp2 << 16) | (nIp3 << 8) | nIp4;
    stGigEDev.nCurrentIp = nIp;

    sscanf(nNetExport, "%d.%d.%d.%d", &nIp1, &nIp2, &nIp3, &nIp4);
    nIp = (nIp1 << 24) | (nIp2 << 16) | (nIp3 << 8) | nIp4;
    stGigEDev.nNetExport = nIp;

    stDevInfo.nTLayerType = MV_GIGE_DEVICE;// ch:仅支持GigE相机 | en:Only support GigE camera
    stDevInfo.SpecialInfo.stGigEInfo = stGigEDev;

    do 
    {
        // ch:选择设备并创建句柄 | en:Select device and create handle
        nRet = MV_CC_CreateHandle(&handle, &stDevInfo);
        if (MV_OK != nRet)
        {
            printf("Create Handle fail! nRet[0x%x]\n", nRet);
            break;
        }

        // ch:打开设备 | en:Open device
        nRet = MV_CC_OpenDevice(handle,MV_ACCESS_Exclusive,0);
        if (MV_OK != nRet)
        {
            printf("Open Device fail! nRet [0x%x]\n", nRet);
            break;
        }

		// ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera)
        if (stDevInfo.nTLayerType == MV_GIGE_DEVICE)
        {
            int nPacketSize = MV_CC_GetOptimalPacketSize(handle);
            if (nPacketSize > 0)
            {
                nRet = MV_CC_SetIntValue(handle,"GevSCPSPacketSize",nPacketSize);
                if(nRet != MV_OK)
                {
                    printf("Warning: Set Packet Size fail nRet [0x%x]!", nRet);
                }
            }
            else
            {
                printf("Warning: Get Packet Size fail nRet [0x%x]!", nPacketSize);
            }
        }

        // ch:设置触发模式为off | en:Set trigger mode as off
        nRet = MV_CC_SetEnumValue(handle, "TriggerMode", MV_TRIGGER_MODE_OFF);
        if (MV_OK != nRet)
        {
            printf("Set Trigger Mode fail! nRet [0x%x]\n", nRet);
            break;
        }

        // ch:开始取流 | en:Start grab image
        nRet = MV_CC_StartGrabbing(handle);
        if (MV_OK != nRet)
        {
            printf("Start Grabbing fail! nRet [0x%x]\n", nRet);
            break;
        }

        void* hThreadHandle = (void*) CreateThread( NULL , 0 , WorkThread , handle, 0 , NULL );
        if (NULL == hThreadHandle)
        {
            break;
        }

        printf("Press a key to stop grabbing.\n");
        WaitForKeyPress();

        g_bExit = true;
        Sleep(1000);

        // ch:停止取流 | en:Stop grab image
        nRet = MV_CC_StopGrabbing(handle);
        if (MV_OK != nRet)
        {
            printf("Stop Grabbing fail! nRet [0x%x]\n", nRet);
            break;
        }

        // ch:关闭设备 | en:Close device
        nRet = MV_CC_CloseDevice(handle);
        if (MV_OK != nRet)
        {
            printf("Close Device fail! nRet [0x%x]\n", nRet);
            break;
        }
		
        // ch:销毁句柄 | en:Destroy handle
        nRet = MV_CC_DestroyHandle(handle);
        if (MV_OK != nRet)
        {
            printf("Destroy Handle fail! nRet [0x%x]\n", nRet);
            break;
        }
        handle = NULL;
    } while (0);
    
	
    if (nRet != MV_OK)
    {
        if (handle != NULL)
        {
            MV_CC_DestroyHandle(handle);
            handle = NULL;
        }
    }
	
    printf("Press a key to exit.\n");
    WaitForKeyPress();
	
    return 0;
}
