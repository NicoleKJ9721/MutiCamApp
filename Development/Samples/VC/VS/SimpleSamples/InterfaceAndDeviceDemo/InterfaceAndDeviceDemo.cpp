#include <stdio.h>
#include <Windows.h>
#include <process.h>
#include <conio.h>
#include "MvCameraControl.h"

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

static  unsigned int __stdcall WorkThread(void* pUser)
{
	int nRet = MV_OK;
	MV_FRAME_OUT stOutFrame = {0};

	while(true)
	{
		nRet = MV_CC_GetImageBuffer(pUser, &stOutFrame, 1000);
		if (nRet == MV_OK)
		{
			printf("Get Image Buffer: Width[%d], Height[%d], FrameNum[%d],enPixelType[%x]\n",
				stOutFrame.stFrameInfo.nWidth, stOutFrame.stFrameInfo.nHeight, stOutFrame.stFrameInfo.nFrameNum,stOutFrame.stFrameInfo.enPixelType);

			nRet = MV_CC_FreeImageBuffer(pUser, &stOutFrame);
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

bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo)
{
	if (NULL == pstMVDevInfo)
	{
		printf("The Pointer of pstMVDevInfo is NULL!\n");
		return false;
	}
	if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE)
	{
		int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24);
		int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16);
		int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8);
		int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

		// ch:打印当前相机ip和用户自定义名字 | en:print current ip and user defined name
		printf("CurrentIp: %d.%d.%d.%d\n" , nIp1, nIp2, nIp3, nIp4);
		printf("UserDefinedName: %s\n\n" , pstMVDevInfo->SpecialInfo.stGigEInfo.chUserDefinedName);
	}
	else if (pstMVDevInfo->nTLayerType == MV_USB_DEVICE)
	{
		printf("UserDefinedName: %s\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName);
		printf("Serial Number: %s\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);
		printf("Device Number: %d\n\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.nDeviceNumber);
	}
	else if (pstMVDevInfo->nTLayerType == MV_GENTL_CAMERALINK_DEVICE)
	{
		printf("UserDefinedName: %s\n", pstMVDevInfo->SpecialInfo.stCMLInfo.chUserDefinedName);
		printf("Serial Number: %s\n", pstMVDevInfo->SpecialInfo.stCMLInfo.chSerialNumber);
		printf("Model Name: %s\n\n", pstMVDevInfo->SpecialInfo.stCMLInfo.chModelName);
	}
	else if (pstMVDevInfo->nTLayerType == MV_GENTL_CXP_DEVICE)
	{
		printf("UserDefinedName: %s\n", pstMVDevInfo->SpecialInfo.stCXPInfo.chUserDefinedName);
		printf("Serial Number: %s\n", pstMVDevInfo->SpecialInfo.stCXPInfo.chSerialNumber);
		printf("Model Name: %s\n\n", pstMVDevInfo->SpecialInfo.stCXPInfo.chModelName);
	}
	else if (pstMVDevInfo->nTLayerType == MV_GENTL_XOF_DEVICE)
	{
		printf("UserDefinedName: %s\n", pstMVDevInfo->SpecialInfo.stXoFInfo.chUserDefinedName);
		printf("Serial Number: %s\n", pstMVDevInfo->SpecialInfo.stXoFInfo.chSerialNumber);
		printf("Model Name: %s\n\n", pstMVDevInfo->SpecialInfo.stXoFInfo.chModelName);
	}
	else
	{
		printf("Not support.\n");
	}

	return true;
}

int main()
{
	int nRet = MV_OK;
	void* handle = NULL;

	printf("[0]: Enum GIGE Interface Devices\n");
	printf("[1]: Enum CAMERALINK Interface Devices\n");
	printf("[2]: Enum CXP Interface Devices\n");
	printf("[3]: Enum XOF Interface Devices\n\n");

	do 
	{
		// ch:初始化SDK | en:Initialize SDK
		nRet = MV_CC_Initialize();
		if (MV_OK != nRet)
		{
			printf("Initialize SDK fail! nRet [0x%x]\n", nRet);
			break;
		}

		MV_CC_DEVICE_INFO_LIST stDeviceList={0};

		unsigned int nType = 0;
		printf("Please Input Enum Interfaces Type(0-%d):", 3);
		scanf_s("%d", &nType);

		switch(nType)
		{
		case 0:
			{
				nRet = MV_CC_EnumDevices(MV_GENTL_GIGE_DEVICE,&stDeviceList);
				break;
			}
		case 1:
			{
				nRet = MV_CC_EnumDevices(MV_GENTL_CAMERALINK_DEVICE,&stDeviceList);
				break;
			}
		case 2:
			{
				nRet = MV_CC_EnumDevices(MV_GENTL_CXP_DEVICE,&stDeviceList);
				break;
			}
		case 3:
			{
				nRet = MV_CC_EnumDevices(MV_GENTL_XOF_DEVICE,&stDeviceList);
				break;
			}
		default:
			{
				printf("Input error!\n");
				break;
			}
		}

		//枚举采集卡设备
		if (MV_OK != nRet)
		{
			printf("Enum Interfaces Devices fail! nRet [0x%x]\n", nRet);
			break;
		}

		if (stDeviceList.nDeviceNum > 0)
		{
			for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++)
			{
				printf("[device %d]:\n", i);
				MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i];
				if (NULL == pDeviceInfo)
				{
					break;
				} 
				PrintDeviceInfo(pDeviceInfo);            
			}  
		} 
		else
		{
			printf("Find No Devices!\n");
			break;
		}

		printf("Please Input camera index(0-%d):", stDeviceList.nDeviceNum-1);
		unsigned int nIndex = 0;
		scanf_s("%d", &nIndex);

		if (nIndex >= stDeviceList.nDeviceNum)
		{
			printf("Input error!\n");
			break;
		}

		// ch:选择设备并创建句柄 | en:Select device and create handle
		nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[nIndex]);
		if (MV_OK != nRet)
		{
			printf("Create Handle fail! nRet [0x%x]\n", nRet);
			break;
		}

		// ch:打开设备 | en:Open device
		nRet = MV_CC_OpenDevice(handle);
		if (MV_OK != nRet)
		{
			printf("Open Device fail! nRet [0x%x]\n", nRet);
			break;
		}

		// ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera)
		if (stDeviceList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE)
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
		nRet = MV_CC_SetEnumValue(handle, "TriggerMode", 0);
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

		unsigned int nThreadID = 0;
		void* hThreadHandle = (void*) _beginthreadex( NULL , 0 , WorkThread , handle, 0 , &nThreadID );
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

		// ch:关闭设备 | Close device
		nRet = MV_CC_CloseDevice(handle);
		if (MV_OK != nRet)
		{
			printf("ClosDevice fail! nRet [0x%x]\n", nRet);
			break;
		}

		// ch:销毁句柄 | Destroy handle
		nRet = MV_CC_DestroyHandle(handle);
		if (MV_OK != nRet)
		{
			printf("Destroy Handle fail! nRet [0x%x]\n", nRet);
			break;
		}
		handle = NULL;
		
	} while (0);

	if (handle != NULL)
	{
		MV_CC_DestroyHandle(handle);
		handle = NULL;
	}
	
	// ch:反初始化SDK | en:Finalize SDK
	MV_CC_Finalize();

	printf("Press a key to exit.\n");
	WaitForKeyPress();

}