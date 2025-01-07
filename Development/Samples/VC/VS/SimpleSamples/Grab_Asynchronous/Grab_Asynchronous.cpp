#include <stdio.h>
#include <Windows.h>
#include <conio.h>
#include <process.h>
#include <iostream>
#include "MvCameraControl.h"

using namespace std;

#define MV_Mutex CRITICAL_SECTION

MV_Mutex Mutex;

void MV_InitializeMutex(MV_Mutex *Mutex)
{
	InitializeCriticalSection(Mutex);
	return;
}

void MV_DeleteMutex(MV_Mutex *Mutex)
{
	DeleteCriticalSection(Mutex);
	return;
}

void MV_EnterMutex(MV_Mutex *Mutex)
{
	EnterCriticalSection(Mutex);
	return;
}

void MV_LeaveMutex(MV_Mutex *Mutex)
{
	LeaveCriticalSection(Mutex);
	return;
}

class CMVMutex
{
public:
	CMVMutex(MV_Mutex Mutex)
	{
		m_Mutex = Mutex;
		MV_InitializeMutex(&m_Mutex);
	}
	~CMVMutex()
	{
		MV_DeleteMutex(&m_Mutex);
	}
	void _Lock()
	{
		MV_EnterMutex(&m_Mutex);
	}
	void _Unlock()
	{
		MV_LeaveMutex(&m_Mutex);
	}

private:
	MV_Mutex    m_Mutex;
};



bool g_bExit = false;  //线程结束标记
#define Max_Count 5       //队列 缓冲区个数； 根据实际情况调节
int m_nImageSize = 0;    // 图像大小
CMVMutex *g_mutex = NULL;  //互斥锁


typedef struct _stImageNode_
{
    unsigned char* pData;
    unsigned int nFrameLen;

    unsigned short nWidth;
    unsigned short nHeight;
    unsigned int nFrameNum;

}stImageNode;



class ArrayQueue
{
public:
    ArrayQueue()
    {
        this->size = 0;
        this->start = 0;
        this->end = 0;
        this->Queue = NULL;
        this->Qlen = 0;
    }
    

    ~ArrayQueue()
    {
        g_mutex->_Lock();

        for (int i = 0; i< Qlen; i++)
        {
            Queue[i].nFrameNum = 0;
            Queue[i].nHeight = 0;
            Queue[i].nWidth = 0;
            Queue[i].nFrameLen = 0;  
			if (Queue[i].pData)
			{
				free(Queue[i].pData);
				Queue[i].pData = NULL;
			}           

            printf(" free ArrayQueue [%d] !\r\n",i);
        }

        delete []Queue;
        Queue = NULL;

        size = 0;
        start = 0;
        end = 0;

        g_mutex->_Unlock();

    }

    // 队列初始化
    int Init(int nBufCount, int DefaultImagelen)
    {
        int nRet = 0 ;

        this->Queue = new (std::nothrow)stImageNode[nBufCount];
		if (this->Queue == NULL)
		{
			return  MV_E_RESOURCE;
		}
        this->Qlen = nBufCount;


        for (int i = 0; i< nBufCount; i++)
        {
            Queue[i].nFrameNum = 0;
            Queue[i].nHeight = 0;
            Queue[i].nWidth = 0;
            Queue[i].nFrameLen = 0;          
            Queue[i].pData = (unsigned char*)malloc(DefaultImagelen);
            if(NULL ==  Queue[i].pData)
            {
                return  MV_E_RESOURCE;
            }
        }

        return 0;

    }

    // 数据放入队列
    int push(int nFrameNum, int nWidth, int nHeight, unsigned char *pData, int nFrameLen)
    {
        g_mutex->_Lock();

        if (size==Qlen)
        {
            g_mutex->_Unlock();
            return MV_E_BUFOVER;
        }


        size++;
        Queue[end].nFrameNum = nFrameNum;
        Queue[end].nHeight = nHeight;
        Queue[end].nWidth = nWidth;
        Queue[end].nFrameLen = nFrameLen;

        if (NULL !=  Queue[end].pData  && NULL != pData)
        {
            memcpy(Queue[end].pData, pData, nFrameLen);
        }

        end = end == Qlen - 1 ? 0 : end + 1;
        g_mutex->_Unlock();

        return 0;
    }

    // 数据从队列中取出
    int  poll(int &nFrameNum, int &nHeight, int &nWidth, unsigned char *pData, int &nFrameLen)
    {

        g_mutex->_Lock();

        if (size == 0)
        {
            g_mutex->_Unlock();
            return MV_E_NODATA ;
        }


        nFrameNum =Queue[start].nFrameNum;
        nHeight =Queue[start].nHeight;
        nWidth =Queue[start].nWidth;
        nFrameLen =Queue[start].nFrameLen;

        if (NULL !=  pData && NULL != Queue[start].pData)
        {
            memcpy( pData,Queue[start].pData, nFrameLen);
        }

        size--;
        start = start == Qlen - 1 ? 0 : start + 1;

        g_mutex->_Unlock();


        return 0;
    }


private:
    stImageNode *Queue;
    int size;
    int start;
    int end;
    int Qlen;
};


ArrayQueue * m_queue = NULL;      //线程通信队列



// ch:等待按键输入 | en:Wait for key press
void WaitForKeyPress(void)
{
	while(!_kbhit())
	{
		Sleep(10);
	}
	_getch();
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
	else
	{
		printf("Not support.\n");
	}

	return true;
}

void __stdcall ImageCallBackEx(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser)
{
	if (NULL ==  pFrameInfo ||  NULL ==  pData)
	{
        printf("ImageCallBackEx Input Param invalid.\n");
        return;
    }

    int nRet = MV_OK;
    nRet = m_queue->push(pFrameInfo->nFrameNum, pFrameInfo->nWidth, pFrameInfo->nHeight,pData, pFrameInfo->nFrameLen);//固定数组实现队列故是10 9 8 7 6 5 4 3 2 1
    if (MV_OK != nRet)
    {
        printf("Add Image [%d] to list  failed. \r\n", pFrameInfo->nFrameNum);
    }
    else
    {
        printf("Add Image [%d] to list  success. \r\n", pFrameInfo->nFrameNum);
    }

    return;
}


static  unsigned int __stdcall WorkThread(void* pUser)
{
	int nRet = MV_OK;

	int nWidth = 0;
	int nHeight = 0;
	int nFrameLen = 0;
	int nFrameNum = 0;
    unsigned char *pOutData = (unsigned char *) malloc(m_nImageSize);
    if (NULL == pOutData)
    {
        printf("WorkThread malloc size [%d] failed. \r\n",m_nImageSize);
        return  MV_E_RESOURCE;
    }
    printf("WorkThread Begin . \r\n");


	while(true != g_bExit)
	{
        // 变量初始化
        nWidth = 0;
        nHeight = 0;
        nFrameLen = 0;
        nFrameNum = 0;

        nRet = m_queue->poll(nFrameNum,nHeight,nWidth,pOutData,nFrameLen);
        if (MV_OK != nRet)
        {
            printf("Poll failed, maybe no data. \r\n");
			Sleep(2);
            continue;
        }
        else
        {
            printf("Get nWidth [%d] nHeight [%d] nFrameNum [%d]  \r\n",nWidth,nHeight,nFrameNum);

             //根据实际场景需求，对图像进行 处理   
#if 0
            FILE* fp = NULL;
            char szFileName[256] = {0};
            sprintf(szFileName, "Image_%d_width_%d_height_%d_Len_%d.raw",nFrameNum,nWidth,nHeight,nFrameLen);
            fp = fopen(szFileName, "wb+");
            if (fp == NULL)
            {
                return MV_E_RESOURCE;
            }
            fwrite(pOutData, 1, nFrameLen, fp);
            fclose(fp);
            fp = NULL;
#endif
        }
	}

    printf("WorkThread exit . \r\n");

    if (pOutData)
    {
        free(pOutData);
        pOutData = NULL;
    }

	return 0;
}
int main()
{

	int nRet = MV_OK;
	void* handle = NULL;
	
	do 
	{
		g_mutex = new (std::nothrow)CMVMutex(Mutex);  //互斥锁
		if (g_mutex == NULL)
		{
			printf("g_mutex is null! \n");
			break;
		}

		// ch:初始化SDK | en:Initialize SDK
		nRet = MV_CC_Initialize();
		if (MV_OK != nRet)
		{
			printf("Initialize SDK fail! nRet [0x%x]\n", nRet);
			break;
		}

		// ch:枚举设备 | Enum device
		MV_CC_DEVICE_INFO_LIST stDeviceList;
		memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
		nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &stDeviceList);
		if (MV_OK != nRet)
		{
			printf("Enum Devices fail! nRet [0x%x]\n", nRet);
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

		// ch:选择设备并创建句柄 | Select device and create handle
		nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[nIndex]);
		if (MV_OK != nRet)
		{
			printf("Create Handle fail! nRet [0x%x]\n", nRet);
			break;
		}

		// ch:打开设备 | Open device
		nRet = MV_CC_OpenDevice(handle);
		if (MV_OK != nRet)
		{
			printf("Open Device fail! nRet [0x%x]\n", nRet);
			break;
		}

        int nWidth = 0;
        int nHeight = 0;

        MVCC_INTVALUE_EX stIntEx  = {0}; 
        nRet = MV_CC_GetIntValueEx(handle, "Width", &stIntEx);
        if (MV_OK != nRet)
        {
            printf("Get IntValue fail! nRet [0x%x]\n", nRet);
            break;
        }
        nWidth = stIntEx.nCurValue;

        
        memset(&stIntEx, 0, sizeof(MVCC_INTVALUE_EX));
        nRet = MV_CC_GetIntValueEx(handle, "Height", &stIntEx);
        if (MV_OK != nRet)
        {
            printf("Get IntValue fail! nRet [0x%x]\n", nRet);
            break;
        }
        nHeight = stIntEx.nCurValue;
             
        
        nRet = MV_CC_GetIntValueEx(handle, "PayloadSize", &stIntEx);
		if (MV_OK != nRet)
		{
			printf("Get IntValue fail! nRet [0x%x], use [%d] replace.\n", nRet, nHeight*nWidth * 3);
            m_nImageSize =  nHeight*nWidth*3; 
		}
        else
        {
           m_nImageSize =  stIntEx.nCurValue;
        }

        //初始化队列
		m_queue = new (std::nothrow)ArrayQueue();
		if (m_queue==NULL)
		{
			printf("m_queue is null! \n");
			break;
		}
        nRet = m_queue->Init(Max_Count,m_nImageSize);
        if (MV_OK != nRet)
        {
            printf("ArrayQueue init fail! nRet [0x%x]\n", nRet);
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

		// ch:设置触发模式为off | eb:Set trigger mode as off
		nRet = MV_CC_SetEnumValue(handle, "TriggerMode", MV_TRIGGER_MODE_OFF);
		if (MV_OK != nRet)
		{
			printf("Set Trigger Mode fail! nRet [0x%x]\n", nRet);
			break;
		}

		// ch:注册抓图回调 | en:Register image callback
		nRet = MV_CC_RegisterImageCallBackEx(handle, ImageCallBackEx, handle);
		if (MV_OK != nRet)
		{
			printf("Register Image CallBack fail! nRet [0x%x]\n", nRet);
			break;
		}

		unsigned int nThreadID = 0;
		void* hThreadHandle = (void*) _beginthreadex( NULL , 0 , WorkThread , handle, 0 , &nThreadID );
		if (NULL == hThreadHandle)
		{
			printf("Start work thread fail! \n");
			break;
		}

		// ch:开始取流 | en:Start grab image
		nRet = MV_CC_StartGrabbing(handle);
		if (MV_OK != nRet)
		{
			printf("Start Grabbing fail! nRet [0x%x]\n", nRet);
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

		// ch:注销抓图回调 | en:Unregister image callback
		nRet = MV_CC_RegisterImageCallBackEx(handle, NULL, NULL);
		if (MV_OK != nRet)
		{
			printf("Unregister Image CallBack fail! nRet [0x%x]\n", nRet);
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

    if (m_queue)
    {
        delete m_queue;
        m_queue = NULL;
    }

	printf("free buffer done\n");
	
	if (handle != NULL)
	{
		MV_CC_DestroyHandle(handle);
		handle = NULL;
	}
	

	// ch:反初始化SDK | en:Finalize SDK
	MV_CC_Finalize();

	if(g_mutex)
	{
		delete g_mutex;
		g_mutex = NULL;
	}

	printf("Press a key to exit.\n");
	WaitForKeyPress();

	return 0;
}