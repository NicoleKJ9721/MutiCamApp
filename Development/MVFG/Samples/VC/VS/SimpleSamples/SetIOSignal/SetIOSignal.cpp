/*

This program shows how to use the MVFGControl method to set IO signal.
[注] 需要保存文件的示例程序在部分环境下需以管理员权限执行，否则会有异常
[PS] Sample programs that need to save files need to be executed with administrator privileges \
     in some environments, otherwise there will be exceptions
     
*/
#include <stdio.h>
#include <Windows.h>
#include <process.h>
#include <conio.h>
#include "MVFGControl.h"

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

// ch:打印采集卡信息 | en:Print interface info
bool PrintInterfaceInfo(unsigned int nInterfaceNum)
{
    int nRet = 0;

    for (unsigned int i = 0; i < nInterfaceNum; i++)
    {
        MV_FG_INTERFACE_INFO stInterfaceInfo = { 0 };

        nRet = MV_FG_GetInterfaceInfo(i, &stInterfaceInfo);
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

int main()
{
    int             nRet = 0;
    IFHANDLE        hInterface = NULL;

    do
    {
        // ch:枚举采集卡 | en:Enum interface
        bool bChanged = false;
        nRet = MV_FG_UpdateInterfaceList(MV_FG_CXP_INTERFACE | MV_FG_GEV_INTERFACE | MV_FG_CAMERALINK_INTERFACE | MV_FG_XoF_INTERFACE, &bChanged);
        if (MV_FG_SUCCESS != nRet)
        {
            printf("Update interface list failed! %#x\n", nRet);
            break;
        }

        // ch:获取采集卡数量 | en:Get interface num
        unsigned int nInterfaceNum = 0;
        nRet = MV_FG_GetNumInterfaces(&nInterfaceNum);
        if (MV_FG_SUCCESS != nRet || 0 == nInterfaceNum)
        {
            printf("No interface found! return = %d, number = %d\n", nRet, nInterfaceNum);
            break;
        }

        // ch:显示采集卡信息 | en:Show interface info
        if (false == PrintInterfaceInfo(nInterfaceNum))
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

        // ch:打开采集卡，获得采集卡句柄 | en:Open interface with specified permissions, get handle
        nRet = MV_FG_OpenInterface((unsigned int)nInterfaceIndex, &hInterface);
        if (MV_FG_SUCCESS != nRet)
        {
            printf("Open No.%d interface failed! %#x\n", nInterfaceIndex, nRet);
            break;
        }

        MV_FG_ENUMVALUE stEnumValue = {0};
        nRet = MV_FG_GetEnumValue(hInterface, "LineSelector", &stEnumValue);
        if (MV_FG_SUCCESS != nRet)
        {
            printf("Get Line Selector failed! %#x\n", nRet);
            break;
        }
        printf("Select Line Selector, including.\n");
        int nLineSelectorIndex = -1;
        for (unsigned int i = 0; i < stEnumValue.nSupportedNum; i++)
        {
            printf("%d. %s\n", i, stEnumValue.strSymbolic[i]);
        }
        scanf_s("%d", &nLineSelectorIndex);
        if (nLineSelectorIndex < 0 || nLineSelectorIndex >= stEnumValue.nSupportedNum)
        {
            printf("Invalid index.\nQuit.\n");
            break;
        }
        // ch:设置输入或输出信号 | en:Set input or output signal
        nRet = MV_FG_SetEnumValue(hInterface, "LineSelector", stEnumValue.nSupportValue[nLineSelectorIndex]);
        if (MV_FG_SUCCESS != nRet)
        {
            printf("Set Line Selector failed! %#x\n", nRet);
            break;
        }
        else
        {
            printf("Set Line Selector success!\n");
        }

        memset(&stEnumValue, 0, sizeof(stEnumValue));
        nRet = MV_FG_GetEnumValue(hInterface, "LineMode", &stEnumValue);
        if (MV_FG_SUCCESS != nRet)
        {
            printf("Get Line Mode failed! %#x\n", nRet);
            break;
        }
        printf("Select Line Mode, including.\n");
        int nLineModeIndex = -1;
        for (unsigned int i = 0; i < stEnumValue.nSupportedNum; i++)
        {
            printf("%d. %s\n", i, stEnumValue.strSymbolic[i]);
        }
        scanf_s("%d", &nLineModeIndex);
        if (nLineModeIndex < 0 || nLineModeIndex >= stEnumValue.nSupportedNum)
        {
            printf("Invalid index.\nQuit.\n");
            break;
        }
        nRet = MV_FG_SetEnumValue(hInterface, "LineMode", stEnumValue.nSupportValue[nLineModeIndex]);
        if (MV_FG_SUCCESS != nRet)
        {
            printf("Set Line Mode failed! %#x\n", nRet);
            break;
        }
        else
        {
            printf("Set Line Mode success!\n");
        }

        nRet = MV_FG_SetBoolValue(hInterface, "LineInverter", false);
        if (MV_FG_SUCCESS != nRet)
        {
            printf("Set Line Inverter failed! %#x\n", nRet);
            break;
        }
        else
        {
            printf("Set Line Inverter success!\n");
        }

        memset(&stEnumValue, 0, sizeof(stEnumValue));
        nRet = MV_FG_GetEnumValue(hInterface, "LineMode", &stEnumValue);
        if (MV_FG_SUCCESS != nRet)
        {
            printf("Get Line Mode failed! %#x\n", nRet);
            break;
        }
        if (0 == strcmp("Input", stEnumValue.strCurSymbolic))
        {
            int nLineDebouncerTimeNs = 100;
            nRet = MV_FG_SetIntValue(hInterface, "LineDebouncerTimeNs", nLineDebouncerTimeNs);
            if (MV_FG_SUCCESS != nRet)
            {
                nRet = MV_FG_SetIntValue(hInterface, "LineDebouncerTime", nLineDebouncerTimeNs);
                if (MV_FG_SUCCESS != nRet)
                {
                    printf("Set Line Debouncer Time failed! %#x\n", nRet);
                    break;
                }
                else
                {
                    printf("Set Line Debouncer Time to %d!\n", nLineDebouncerTimeNs);
                }
            }
            else
            {
                printf("Set Line Debouncer Time to %d!\n", nLineDebouncerTimeNs);
            }
        }
        else if (0 == strcmp("Output", stEnumValue.strCurSymbolic))
        {
            memset(&stEnumValue, 0, sizeof(stEnumValue));
            nRet = MV_FG_GetEnumValue(hInterface, "LineSource", &stEnumValue);
            if (MV_FG_SUCCESS != nRet)
            {
                printf("Get Line Source failed! %#x\n", nRet);
                break;
            }
            printf("Select Line Source, including.\n");
            int nLineSourceIndex = -1;
            for (unsigned int i = 0; i < stEnumValue.nSupportedNum; i++)
            {
                printf("%d. %s\n", i, stEnumValue.strSymbolic[i]);
            }
            scanf_s("%d", &nLineSourceIndex);
            if (nLineSourceIndex < 0 || nLineSourceIndex >= stEnumValue.nSupportedNum)
            {
                printf("Invalid index.\nQuit.\n");
                break;
            }
            nRet = MV_FG_SetEnumValue(hInterface, "LineSource", stEnumValue.nSupportValue[nLineSourceIndex]);
            if (MV_FG_SUCCESS != nRet)
            {
                printf("Set Line Source failed! %#x\n", nRet);
                break;
            }
            else
            {
                printf("Set Line Source success!\n");
            }
        }
    } while (0);

    // ch:关闭采集卡 | en:Close interface
    if (NULL != hInterface)
    {
        nRet = MV_FG_CloseInterface(hInterface);
        if (MV_FG_SUCCESS != nRet)
        {
            printf("Close interface failed! %#x\n", nRet);
        }
        hInterface = NULL;
    }

    printf("Press any key to exit.\n");
    WaitForKeyPress();

    return 0;
}
