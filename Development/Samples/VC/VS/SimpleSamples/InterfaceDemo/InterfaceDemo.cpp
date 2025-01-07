#include <stdio.h>
#include <Windows.h>
#include <process.h>
#include <conio.h>
#include "MvCameraControl.h"

void* hInterface = NULL;

// ch:等待按键输入 | en:Wait for key press
void WaitForKeyPress(void)
{
	while(!_kbhit())
	{
		Sleep(10);
	}
	_getch();
}

bool PrintInterfaceInfo(MV_INTERFACE_INFO* pstInterfaceInfo)
{
	if (NULL == pstInterfaceInfo)
	{
		printf("The Pointer of pstInterfaceInfo is NULL!\n");
		return false;
	}
	printf("Display name: %s\n",pstInterfaceInfo->chDisplayName);
	printf("Serial number: %s\n",pstInterfaceInfo->chSerialNumber);
	printf("model name: %s\n",pstInterfaceInfo->chModelName);
	printf("\n");

	return true;
}

void Set_Get_Enum(const char* str)
{
	MVCC_ENUMVALUE stEnumValue = {0};
	MVCC_ENUMENTRY stEnumentryInfo = {0};

	int nRet = MV_CC_GetEnumValue(hInterface,str, &stEnumValue);
	if (MV_OK != nRet)
	{
		printf("Get %s Fail! nRet [0x%x]\n", str, nRet);
		return;
	}

	stEnumentryInfo.nValue = stEnumValue.nCurValue;
	nRet = MV_CC_GetEnumEntrySymbolic(hInterface,str, &stEnumentryInfo);
	if (MV_OK != nRet)
	{
		printf("Get %s Fail! nRet [0x%x]\n", str,nRet);
		return;
	}
	else
	{
		printf("Get %s = [%s] Success!\n",str,stEnumentryInfo.chSymbolic);
	}

	nRet = MV_CC_SetEnumValue(hInterface,str,stEnumValue.nCurValue);
	if (MV_OK != nRet)
	{
		printf("Set %s Fail! nRet [0x%x]\n", str,nRet);
		return;
	}
	else
	{
		printf("Set %s = [%s] Success!\n",str,stEnumentryInfo.chSymbolic);
	}
}

void Set_Get_Bool(const char* str)
{
	bool bValue = false;
	int nRet = MV_CC_GetBoolValue(hInterface,str, &bValue);
	if (MV_OK != nRet)
	{
		printf("Get %s Fail! nRet [0x%x]\n", str,nRet);
		return;
	}
	else
	{
		printf("Get %s =  [%d] Success!\n",str,bValue);
	}

	nRet = MV_CC_SetBoolValue(hInterface,str, bValue);
	if (MV_OK != nRet)
	{
		printf("Set %s Fail! nRet [0x%x]\n", str,nRet);
		return;
	}
	else
	{
		printf("Set %s = [%d] Success!\n",str,bValue);
	}
}

void Set_Get_Int(const char* str)
{
	MVCC_INTVALUE_EX stIntValue;
	int nRet = MV_CC_GetIntValueEx(hInterface,str,&stIntValue);
	if (MV_OK != nRet)
	{
		printf("Get %s Fail! nRet [0x%x]\n", str,nRet);
		return;
	}
	else
	{
		printf("Get %s =  [%d] Success!\n",str,stIntValue.nCurValue);
	}
	
	nRet = MV_CC_SetIntValueEx(hInterface,str,stIntValue.nCurValue);
	if (MV_OK != nRet)
	{
		printf("Set %s Fail! nRet [0x%x]\n", str,nRet);
		return;
	}
	else
	{
		printf("Set %s = [%d] Success!\n",str,stIntValue.nCurValue);
	}
}

void Set_Get_String(const char* str)
{
	MVCC_STRINGVALUE StringValue;
	int nRet = MV_CC_GetStringValue(hInterface,str, &StringValue);
	if (MV_OK != nRet)
	{
		printf("Get %s Fail! nRet [0x%x]\n", str,nRet);
		return;
	}
	else
	{
		printf("Get %s =  [%s] Success!\n",str,StringValue.chCurValue);
	}

	nRet = MV_CC_SetStringValue(hInterface,str, StringValue.chCurValue);
	if (MV_OK != nRet)
	{
		printf("Set %s Fail! nRet [0x%x]\n", str,nRet);
		return;
	}
	else
	{
		printf("Set %s = [%s] Success!\n",str,StringValue.chCurValue);
	}
}

void Set_Get_Float(const char* str)
{
	MVCC_FLOATVALUE FloatValue;
	int nRet = MV_CC_GetFloatValue(hInterface,str, &FloatValue);
	if (MV_OK != nRet)
	{
		printf("Get %s Fail! nRet [0x%x]\n", str,nRet);
		return;
	}
	else
	{
		printf("Get %s =  [%f] Success!\n",str,FloatValue.fCurValue);
	}

	nRet = MV_CC_SetFloatValue(hInterface,str, FloatValue.fCurValue);
	if (MV_OK != nRet)
	{
		printf("Set %s Fail! nRet [0x%x]\n", str,nRet);
		return;
	}
	else
	{
		printf("Set %s = [%f] Success!\n",str,FloatValue.fCurValue);
	}
}

int main()
{
	int nRet = MV_OK;

	printf("[0]: GIGE Interface\n");
	printf("[1]: CAMERALINK Interface\n");
	printf("[2]: CXP Interface\n");
	printf("[3]: XOF Interface\n\n");

	do 
	{
		// ch:初始化SDK | en:Initialize SDK
		nRet = MV_CC_Initialize();
		if (MV_OK != nRet)
		{
			printf("Initialize SDK fail! nRet [0x%x]\n", nRet);
			break;
		}

		MV_INTERFACE_INFO_LIST stInterfaceInfoList={0};

		unsigned int nType = 0;
		printf("Please Input Enum Interfaces Type(0-%d):", 3);
		scanf_s("%d", &nType);

		switch(nType)
		{
		case 0:
			{
				nRet = MV_CC_EnumInterfaces(MV_GIGE_INTERFACE, &stInterfaceInfoList);
				break;
			}
		case 1:
			{
				nRet = MV_CC_EnumInterfaces(MV_CAMERALINK_INTERFACE, &stInterfaceInfoList);
				break;
			}
		case 2:
			{
				nRet = MV_CC_EnumInterfaces(MV_CXP_INTERFACE, &stInterfaceInfoList);
				break;
			}
		case 3:
			{
				nRet = MV_CC_EnumInterfaces(MV_XOF_INTERFACE, &stInterfaceInfoList);
				break;
			}
		default:
			{
				printf("Input error!\n");
				break;
			}
		}

		//枚举采集卡
		if (MV_OK != nRet)
		{
			printf("Enum Interfaces fail! nRet [0x%x]\n", nRet);
			break;
		}
		
		if (stInterfaceInfoList.nInterfaceNum > 0)
		{
			for (unsigned int i = 0; i < stInterfaceInfoList.nInterfaceNum; i++)
			{
				printf("[Interface %d]:\n", i);
				MV_INTERFACE_INFO* pstInterfaceInfo = stInterfaceInfoList.pInterfaceInfos[i];
				if (NULL == pstInterfaceInfo)
				{
					break;
				} 
				PrintInterfaceInfo(pstInterfaceInfo);            
			}
			printf("Enum Interfaces success!\n\n");
		} 
		else
		{
			printf("Find No Interface!\n");
			break;
		}

		printf("Please Input Interfaces index(0-%d):", stInterfaceInfoList.nInterfaceNum-1);
		unsigned int nIndex = 0;
		scanf_s("%d", &nIndex);

		if (nIndex >= stInterfaceInfoList.nInterfaceNum)
		{
			printf("Input error!\n");
			break;
		}

		//创建采集卡句柄
		nRet = MV_CC_CreateInterface(&hInterface, stInterfaceInfoList.pInterfaceInfos[nIndex]);
		if (MV_OK == nRet)
		{
			printf("Create Interface success!\n");
		}
		else
		{
			printf("Create Interface Handle fail! nRet [0x%x]\n", nRet);
			break;
		}

		//打开采集卡
		nRet = MV_CC_OpenInterface(hInterface, "NULL");
		if (MV_OK == nRet)
		{
			printf("Open Interface success!\n");
		}
		else
		{
			printf("Open Interface fail! nRet [0x%x]\n", nRet);
			break;
		}

		//采集卡属性操作,不同类型卡对各自常用属性进行获取和设置
		switch(nType)
		{
		case 0:
			{
				//MV_GIGE_INTERFACE卡属性获取和设置操作
				Set_Get_Enum("StreamSelector");
				Set_Get_Enum("TimerSelector");
				Set_Get_Enum("TimerTriggerSource");
				Set_Get_Enum("TimerTriggerActivation");
				Set_Get_Bool("HBDecompression");
				Set_Get_Int("TimerDuration");
				Set_Get_Int("TimerDelay");
				Set_Get_Int("TimerFrequency");
				break;
			}
		case 1:
			{
				//MV_CAMERALINK_INTERFACE卡属性操作
				Set_Get_Enum("StreamSelector");
				Set_Get_Enum("CameraType");
				Set_Get_Enum("StreamPartialImageControl");
				Set_Get_Int("ImageHeight");
				Set_Get_Int("FrameTimeoutTime");
				break;
			}
		case 2:
			{
				//MV_CXP_INTERFACE卡属性操作
				Set_Get_Enum("StreamSelector");
				Set_Get_String("CurrentStreamDevice");
				Set_Get_Int("StreamEnableStatus");
				Set_Get_Bool("BayerCFAEnable");
				Set_Get_Bool("IspGammaEnable");
				Set_Get_Float("IspGamma");
				break;
			}
		case 3:
			{
				//MV_XOF_INTERFACE卡属性操作
				Set_Get_Enum("StreamSelector");
				Set_Get_String("CurrentStreamDevice");
				Set_Get_Int("StreamEnableStatus");
				Set_Get_Bool("BayerCFAEnable");
				Set_Get_Bool("IspGammaEnable");
				Set_Get_Float("IspGamma");
				break;
			}
		default:
			{
				printf("Input error!\n");
				break;
			}
		}

		//关闭采集卡
		nRet = MV_CC_CloseInterface(hInterface);
		if (MV_OK == nRet)
		{
			printf("Close Interface success!\n");
		}
		else
		{
			printf("Close Interface Handle fail! nRet [0x%x]\n", nRet);
			break;
		}

		//销毁采集卡句柄
		nRet = MV_CC_DestroyInterface(hInterface);
		if (MV_OK == nRet)
		{
			printf("Destroy Interface success!\n");
		}
		else
		{
			printf("Destroy Interface Handle fail! nRet [0x%x]\n", nRet);
			break;
		}
		hInterface = NULL;
	} while (0);

	if (hInterface != NULL)
	{
		MV_CC_CloseInterface(hInterface);
		MV_CC_DestroyInterface(hInterface);
		hInterface = NULL;
	}

	// ch:反初始化SDK | en:Finalize SDK
	MV_CC_Finalize();

	printf("Press a key to exit.\n");
	WaitForKeyPress();
}