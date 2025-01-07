/*

This program shows how to use the MVFGControl method to set IO signal.

*/
using System;
using System.Collections.Generic;
using MvFGCtrlC.NET;
using System.Runtime.InteropServices;
using System.IO;

namespace SetIOSignal
{
    class SetIOSignal
    {
        static void Main(string[] args)
        {
            int         nRet = CErrorCode.MV_FG_SUCCESS;
            CSystem     cSystem = new CSystem();            // ch:操作采集卡 | en:Interface operations
            CInterface  cInterface = null;                  // ch:操作采集卡和设备 | en:Interface and device operation

            do
            {
                // ch:枚举采集卡 | en:Enum interface
                bool bChanged = false;
                nRet = cSystem.UpdateInterfaceList(
                    CParamDefine.MV_FG_CAMERALINK_INTERFACE | CParamDefine.MV_FG_GEV_INTERFACE | CParamDefine.MV_FG_CXP_INTERFACE | CParamDefine.MV_FG_XoF_INTERFACE,
                    ref bChanged);
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    Console.WriteLine("Enum interface failed:{0:x8}", nRet);
                    break;
                }

                // ch:获取采集卡数量 | en:Get interface num
                uint nInterfaceNum = 0;        // ch:采集卡数量 | en:Interface number
                nRet = cSystem.GetNumInterfaces(ref nInterfaceNum);
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    Console.WriteLine("Get interface number failed:{0:x8}", nRet);
                    break;
                }
                if (0 == nInterfaceNum)
                {
                    Console.WriteLine("No interface found");
                    break;
                }

                // ch:显示采集卡信息 | en:Show interface info
                MV_FG_INTERFACE_INFO stIfInfo = new MV_FG_INTERFACE_INFO();
                for (uint i = 0; i < nInterfaceNum; i++)
                {
                    // ch:获取采集卡信息 | en:Get interface info
                    nRet = cSystem.GetInterfaceInfo(i, ref stIfInfo);
                    if (CErrorCode.MV_FG_SUCCESS != nRet)
                    {
                        Console.WriteLine("Get interface info failed:{0:x8}", nRet);
                        break;
                    }

                    if (CParamDefine.MV_FG_GEV_INTERFACE == stIfInfo.nTLayerType)
                    {
                        // ch:Byte数组转换为结构体 | en:Convert byte array to structure
                        MV_GEV_INTERFACE_INFO stGevIFInfo = (MV_GEV_INTERFACE_INFO)CAdditional.ByteToStruct(
                            stIfInfo.SpecialInfo.stGevIfInfo, typeof(MV_GEV_INTERFACE_INFO));
                        Console.WriteLine("[GEV]No." + i.ToString() + " Interface:");
                        Console.WriteLine("\tDisplayName: " + stGevIFInfo.chDisplayName);
                        Console.WriteLine("\tInterfaceID: " + stGevIFInfo.chInterfaceID);
                        Console.WriteLine("\tSerialNumber: " + stGevIFInfo.chSerialNumber);
                    }
                    else if (CParamDefine.MV_FG_CXP_INTERFACE == stIfInfo.nTLayerType)
                    {
                        // ch:Byte数组转换为结构体 | en:Convert byte array to structure
                        MV_CXP_INTERFACE_INFO stCxpIFInfo = (MV_CXP_INTERFACE_INFO)CAdditional.ByteToStruct(
                            stIfInfo.SpecialInfo.stCXPIfInfo, typeof(MV_CXP_INTERFACE_INFO));
                        Console.WriteLine("[CXP]No." + i.ToString() + " Interface:");
                        Console.WriteLine("\tDisplayName: " + stCxpIFInfo.chDisplayName);
                        Console.WriteLine("\tInterfaceID: " + stCxpIFInfo.chInterfaceID);
                        Console.WriteLine("\tSerialNumber: " + stCxpIFInfo.chSerialNumber);
                    }
                    else if (CParamDefine.MV_FG_CAMERALINK_INTERFACE == stIfInfo.nTLayerType)
                    {
                        // ch:Byte数组转换为结构体 | en:Convert byte array to structure
                        MV_CML_INTERFACE_INFO stCmlIFInfo = (MV_CML_INTERFACE_INFO)CAdditional.ByteToStruct(
                            stIfInfo.SpecialInfo.stCMLIfInfo, typeof(MV_CML_INTERFACE_INFO));
                        Console.WriteLine("[CML]No." + i.ToString() + " Interface:");
                        Console.WriteLine("\tDisplayName: " + stCmlIFInfo.chDisplayName);
                        Console.WriteLine("\tInterfaceID: " + stCmlIFInfo.chInterfaceID);
                        Console.WriteLine("\tSerialNumber: " + stCmlIFInfo.chSerialNumber);
                    }
                    else if (CParamDefine.MV_FG_XoF_INTERFACE == stIfInfo.nTLayerType)
                    {
                        // ch:Byte数组转换为结构体 | en:Convert byte array to structure
                        MV_XoF_INTERFACE_INFO stXoFIFInfo = (MV_XoF_INTERFACE_INFO)CAdditional.ByteToStruct(
                            stIfInfo.SpecialInfo.stXoFIfInfo, typeof(MV_XoF_INTERFACE_INFO));
                        Console.WriteLine("[XoF]No." + i.ToString() + " Interface:");
                        Console.WriteLine("\tDisplayName: " + stXoFIFInfo.chDisplayName);
                        Console.WriteLine("\tInterfaceID: " + stXoFIFInfo.chInterfaceID);
                        Console.WriteLine("\tSerialNumber: " + stXoFIFInfo.chSerialNumber);
                    }
                    else
                    {
                        Console.WriteLine("Unknown interface type.");
                        nRet = CErrorCode.MV_FG_ERR_INVALID_VALUE;
                        break;
                    }
                }

                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    break;
                }

                // ch:选择采集卡 | en:Select interface
                int nInterfaceIndex = -1;
                Console.Write("Please input index(0-{0:d}):", nInterfaceNum - 1);
                try
                {
                    nInterfaceIndex = Convert.ToInt32(Console.ReadLine());
                }
                catch
                {
                    Console.WriteLine("Invalid Index!");
                    break;
                }

                if (nInterfaceIndex < 0 || nInterfaceIndex >= nInterfaceNum)
                {
                    Console.WriteLine("Error Index!");
                    break;
                }

                // ch:打开采集卡，获得采集卡句柄 | en:Open interface, get handle
                nRet = cSystem.OpenInterface((uint)nInterfaceIndex, out cInterface);
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    Console.WriteLine("Open Interface failed:{0:x8}", nRet);
                    break;
                }

                CParam cParam = new CParam(cInterface);
                MV_FG_ENUMVALUE stEnumValue = new MV_FG_ENUMVALUE();
                nRet = cParam.GetEnumValue("LineSelector", ref stEnumValue);
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    Console.WriteLine("Get Line Selector failed:{0:x8}", nRet);
                    break;
                }
                Console.WriteLine("Select Line Selector, including.");
                int nLineSelectorIndex = -1;
                for (uint i = 0; i < stEnumValue.nSupportedNum; i++)
                {
                    Console.WriteLine("{0}. {1}", i, stEnumValue.strSymbolicArray[i].strInfo);
                }
                try
                {
                    nLineSelectorIndex = Convert.ToInt32(Console.ReadLine());
                }
                catch
                {
                    Console.WriteLine("Invalid Index!");
                    break;
                }

                if (nLineSelectorIndex < 0 || nLineSelectorIndex >= stEnumValue.nSupportedNum)
                {
                    Console.WriteLine("Error Index!");
                    break;
                }

                // ch:设置输入或输出信号 | en:Set input or output signal
                nRet = cParam.SetEnumValue("LineSelector", stEnumValue.nSupportValueArray[nLineSelectorIndex]);
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    Console.WriteLine("Set Line Selector failed:{0:x8}", nRet);
                    break;
                }
                else
                {
                    Console.WriteLine("Set Line Selector success");
                }

                nRet = cParam.GetEnumValue("LineMode", ref stEnumValue);
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    Console.WriteLine("Get Line Mode failed:{0:x8}", nRet);
                    break;
                }
                Console.WriteLine("Select Line Mode, including.");
                int nLineModeIndex = -1;
                for (uint i = 0; i < stEnumValue.nSupportedNum; i++)
                {
                    Console.WriteLine("{0}. {1}", i, stEnumValue.strSymbolicArray[i].strInfo);
                }
                try
                {
                    nLineModeIndex = Convert.ToInt32(Console.ReadLine());
                }
                catch
                {
                    Console.WriteLine("Invalid Index!");
                    break;
                }

                if (nLineModeIndex < 0 || nLineModeIndex >= stEnumValue.nSupportedNum)
                {
                    Console.WriteLine("Error Index!");
                    break;
                }

                nRet = cParam.SetEnumValue("LineMode", stEnumValue.nSupportValueArray[nLineModeIndex]);
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    Console.WriteLine("Set Line Mode failed:{0:x8}", nRet);
                    break;
                }
                else
                {
                    Console.WriteLine("Set Line Mode success");
                }

                nRet = cParam.SetBoolValue("LineInverter", false);
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    Console.WriteLine("Set Line Inverter failed:{0:x8}", nRet);
                    break;
                }
                else
                {
                    Console.WriteLine("Set Line Inverter success");
                }

                nRet = cParam.GetEnumValue("LineMode", ref stEnumValue);
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    Console.WriteLine("Get Line Mode failed:{0:x8}", nRet);
                    break;
                }

                if ("Input" == stEnumValue.strCurSymbolic)
                {
                    int nLineDebouncerTimeNs = 100;
                    nRet = cParam.SetIntValue("LineDebouncerTimeNs", nLineDebouncerTimeNs);
                    if (CErrorCode.MV_FG_SUCCESS != nRet)
                    {
                        nRet = cParam.SetIntValue("LineDebouncerTime", nLineDebouncerTimeNs);
                        if (CErrorCode.MV_FG_SUCCESS != nRet)
                        {
                            Console.WriteLine("Set Line Debouncer Time failed:{0:x8}", nRet);
                            break;
                        }
                        else
                        {
                            Console.WriteLine("Set Line Debouncer Time:{0}", nLineDebouncerTimeNs);
                        }
                    }
                    else
                    {
                        Console.WriteLine("Set Line Debouncer Time:{0}", nLineDebouncerTimeNs);
                    }
                }
                else if ("Output" == stEnumValue.strCurSymbolic)
                {
                    nRet = cParam.GetEnumValue("LineSource", ref stEnumValue);
                    if (CErrorCode.MV_FG_SUCCESS != nRet)
                    {
                        Console.WriteLine("Get Line Source failed:{0:x8}", nRet);
                        break;
                    }
                    Console.WriteLine("Select Line Source, including.");
                    int nLineSourceIndex = -1;
                    for (uint i = 0; i < stEnumValue.nSupportedNum; i++)
                    {
                        Console.WriteLine("{0}. {1}", i, stEnumValue.strSymbolicArray[i].strInfo);
                    }
                    try
                    {
                        nLineSourceIndex = Convert.ToInt32(Console.ReadLine());
                    }
                    catch
                    {
                        Console.WriteLine("Invalid Index!");
                        break;
                    }

                    if (nLineSourceIndex < 0 || nLineSourceIndex >= stEnumValue.nSupportedNum)
                    {
                        Console.WriteLine("Error Index!");
                        break;
                    }

                    nRet = cParam.SetEnumValue("LineSource", stEnumValue.nSupportValueArray[nLineSourceIndex]);
                    if (CErrorCode.MV_FG_SUCCESS != nRet)
                    {
                        Console.WriteLine("Set Line Source failed:{0:x8}", nRet);
                        break;
                    }
                    else
                    {
                        Console.WriteLine("Set Line Source success");
                    }
                }
            } while (false);

            // ch:关闭采集卡 | en:Close interface
            if (null != cInterface)
            {
                nRet = cInterface.CloseInterface();
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    Console.WriteLine("Close interface failed:{0:x8}", nRet);
                }
                cInterface = null;
            }

            Console.WriteLine("Press enter to exit.");
            Console.ReadKey();
        }
    }
}
