/*

This program shows how to use the MVFGControl method to Save or Load config files.

*/
using System;
using System.Collections.Generic;
using MvFGCtrlC.NET;
using System.Runtime.InteropServices;
using System.IO;

namespace ConvertPixelType
{
    class ConvertPixelType
    {

        static void Main(string[] args)
        {
            int         nRet = CErrorCode.MV_FG_SUCCESS;
            CSystem     cSystem = new CSystem();            // ch:操作采集卡 | en:Interface operations
            CInterface  cInterface = null;                  // ch:操作采集卡和设备 | en:Interface and device operation
            CDevice     cDevice = null;                     // ch:操作设备和流 | en:Device and stream operation
            CStream     cStream = null;                     // ch:操作流和缓存 | en:Stream and buffer operation

            do
            {
                // ch:枚举采集卡 | en:Enum interface
                bool bChanged = false;
                /* ch:提供一份由GEV采集卡导出的配置文件用于演示 OpenInterfaceWithConfig功能，
                      用户要使用其它类型的卡可自行打开以下枚举类型并导出对应的配置文件
                      [注]配置文件导入时会对DeviceFirmwareVersion节点做校验，推荐用户用自己导出的配置文件替换Demo.hcf
                   en:Provide a configuration file exported by the GEV FramGrabber card to demonstrate the OpenInterfaceWithConfig function,
                      If users want to use other types of cards, they can open the following enumeration types themselves and export the corresponding configuration files
                      [Note] When importing the configuration file, the DeviceFirmwareVersion node will be verified. It is recommended that users replace Demo.hcf with their own exported configuration file
                */
                nRet = cSystem.UpdateInterfaceList(
                    CParamDefine.MV_FG_GEV_INTERFACE /*| CParamDefine.MV_FG_CXP_INTERFACE | CParamDefine.MV_FG_XoF_INTERFACE | CParamDefine.MV_FG_CAMERALINK_INTERFACE*/,
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

                Console.WriteLine("******");
                Console.WriteLine("\t\t Chose Method To Open Interface");
                Console.WriteLine("\t\t 0. Open Interface With 'Demo.hcf'");
                Console.WriteLine("\t\t 1. Open Interface and Save Config");
                Console.WriteLine("******");
                int nMethod = -1;
                bool bOpenWithConfig = false;

                try
                {
                    nMethod = Convert.ToInt32(Console.ReadLine());
                }
                catch
                {
                    Console.WriteLine("Invalid index!");
                    break;
                }

                switch (nMethod)
                {
                case 0:
                    {
                        // ch:以指定权限打开采集卡，获得采集卡句柄 | en:Open interface with specified permissions, get handle
                        nRet = cSystem.OpenInterface((uint)nInterfaceIndex, "Demo.hcf", out cInterface);
                        bOpenWithConfig = true;
                    }
                    break;
                case 1:
                default:
                    {
                        // ch:打开采集卡，获得采集卡句柄 | en:Open interface, get handle
                        nRet = cSystem.OpenInterface((uint)nInterfaceIndex, out cInterface);
                    }
                    break;
                }
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    Console.WriteLine("Open No.{0} interface failed!:{1:x8}", nInterfaceIndex, nRet);
                    break;
                }

                if (!bOpenWithConfig)
                {
                    string strIFFeatureFile = "Interface" + nInterfaceIndex.ToString() + ".hcf";
                    Console.WriteLine("\t\tInterface FeatureSave Start");

                    // ch:对采集卡执行FeatureSave | en: Execute FeatureSave on the Interface
                    CParam cParam = new CParam(cInterface);
                    nRet = cParam.FeatureSave(strIFFeatureFile);
                    if (CErrorCode.MV_FG_SUCCESS != nRet)
                    {
                        Console.WriteLine("Feature Save interface failed!:{0:x8}", nRet);
                        break;
                    }

                    Console.WriteLine("\t\tInterface FeatureSave Success");
                    Console.WriteLine("\t\tInterface FeatureLoad Start");

                    // ch:对采集卡执行FeatureLoad | en: Execute FeatureLoad on the Interface
                    nRet = cParam.FeatureSave(strIFFeatureFile);
                    if (CErrorCode.MV_FG_SUCCESS != nRet)
                    {
                        Console.WriteLine("Feature Load Interface feature failed!:{0:x8}", nRet);
                        break;
                    }
                    Console.WriteLine("\t\tInterface FeatureLoad Success");
                }

                // ch:枚举采集卡上的相机 | en:Enum camera of interface
                nRet = cInterface.UpdateDeviceList(ref bChanged);
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    Console.WriteLine("Enum device failed:{0:x8}", nRet);
                    break;
                }

                // ch:获取设备数量 | en:Get device number
                uint nDeviceNum = 0;        // ch:设备数量 | en:Device number
                nRet = cInterface.GetNumDevices(ref nDeviceNum);
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    Console.WriteLine("Get device number failed:{0:x8}", nRet);
                    break;
                }
                if (0 == nDeviceNum)
                {
                    Console.WriteLine("No device found");
                    break;
                }

                // ch:显示设备信息 | en:Show device info
                MV_FG_DEVICE_INFO stDeviceInfo = new MV_FG_DEVICE_INFO();
                for (uint i = 0; i < nDeviceNum; i++)
                {
                    // ch:获取设备信息 | en:Get device info
                    nRet = cInterface.GetDeviceInfo(i, ref stDeviceInfo);
                    if (CErrorCode.MV_FG_SUCCESS != nRet)
                    {
                        Console.WriteLine("Get device info failed:{0:x8}", nRet);
                        break;
                    }

                    if (CParamDefine.MV_FG_GEV_DEVICE == stDeviceInfo.nDevType)
                    {
                        // ch:Byte数组转换为结构体 | en:Convert byte array to structure
                        MV_GEV_DEVICE_INFO stGevDevInfo = (MV_GEV_DEVICE_INFO)CAdditional.ByteToStruct(
                            stDeviceInfo.DevInfo.stGEVDevInfo, typeof(MV_GEV_DEVICE_INFO));
                        Console.WriteLine("[GEV]No." + i.ToString() + " Device:");
                        Console.WriteLine("\tUserDefinedName: " + stGevDevInfo.chUserDefinedName);
                        Console.WriteLine("\tModelName: " + stGevDevInfo.chModelName);
                        Console.WriteLine("\tSerialNumber: " + stGevDevInfo.chSerialNumber);
                    }
                    else if (CParamDefine.MV_FG_CXP_DEVICE == stDeviceInfo.nDevType)
                    {
                        // ch:Byte数组转换为结构体 | en:Convert byte array to structure
                        MV_CXP_DEVICE_INFO stCxpDevInfo = (MV_CXP_DEVICE_INFO)CAdditional.ByteToStruct(
                            stDeviceInfo.DevInfo.stCXPDevInfo, typeof(MV_CXP_DEVICE_INFO));
                        Console.WriteLine("[CXP]No." + i.ToString() + " Device:");
                        Console.WriteLine("\tUserDefinedName: " + stCxpDevInfo.chUserDefinedName);
                        Console.WriteLine("\tModelName: " + stCxpDevInfo.chModelName);
                        Console.WriteLine("\tSerialNumber: " + stCxpDevInfo.chSerialNumber);
                    }
                    else if (CParamDefine.MV_FG_CAMERALINK_DEVICE == stDeviceInfo.nDevType)
                    {
                        // ch:Byte数组转换为结构体 | en:Convert byte array to structure
                        MV_CML_DEVICE_INFO stCmlDevInfo = (MV_CML_DEVICE_INFO)CAdditional.ByteToStruct(
                            stDeviceInfo.DevInfo.stCMLDevInfo, typeof(MV_CML_DEVICE_INFO));
                        Console.WriteLine("[CML]No." + i.ToString() + " Device:");
                        Console.WriteLine("\tUserDefinedName: " + stCmlDevInfo.chUserDefinedName);
                        Console.WriteLine("\tModelName: " + stCmlDevInfo.chModelName);
                        Console.WriteLine("\tSerialNumber: " + stCmlDevInfo.chSerialNumber);
                    }
                    else if (CParamDefine.MV_FG_XoF_DEVICE == stDeviceInfo.nDevType)
                    {
                        // ch:Byte数组转换为结构体 | en:Convert byte array to structure
                        MV_XoF_DEVICE_INFO stXoFDevInfo = (MV_XoF_DEVICE_INFO)CAdditional.ByteToStruct(
                            stDeviceInfo.DevInfo.stXoFDevInfo, typeof(MV_XoF_DEVICE_INFO));
                        Console.WriteLine("[XoF]No." + i.ToString() + " Device:");
                        Console.WriteLine("\tUserDefinedName: " + stXoFDevInfo.chUserDefinedName);
                        Console.WriteLine("\tModelName: " + stXoFDevInfo.chModelName);
                        Console.WriteLine("\tSerialNumber: " + stXoFDevInfo.chSerialNumber);
                    }
                    else
                    {
                        Console.WriteLine("Unknown device type.");
                        nRet = CErrorCode.MV_FG_ERR_INVALID_VALUE;
                        break;
                    }
                }

                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    break;
                }

                // ch:选择设备 | en:Select device
                int nDeviceIndex = -1;
                Console.Write("Please input index(0-{0:d}):", nDeviceNum - 1);
                try
                {
                    nDeviceIndex = Convert.ToInt32(Console.ReadLine());
                }
                catch
                {
                    Console.WriteLine("Invalid Index!");
                    break;
                }

                if (nDeviceIndex < 0 || nDeviceIndex >= nDeviceNum)
                {
                    Console.WriteLine("Error Index!");
                    break;
                }

                // ch:打开设备，获得设备句柄 | en:Open device, get handle
                nRet = cInterface.OpenDevice((uint)nDeviceIndex, out cDevice);
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    Console.WriteLine("Open device failed:{0:x8}", nRet);
                    break;
                }

                Console.WriteLine("\t\tDevice FeatureSave Start");

                // ch:操作采集卡或设备参数配置 | en:Interface or device config operation
                CParam cDeviceParam = new CParam(cDevice);
                string strDevFeatureFile = "Device" + nDeviceIndex.ToString() + ".hcf";
                nRet = cDeviceParam.FeatureSave(strDevFeatureFile);
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    Console.WriteLine("Device FeatureSave failed:{0:x8}", nRet);
                    break;
                }
                Console.WriteLine("\t\tDevice FeatureSave success");
                Console.WriteLine("\t\tDevice FeatureLoad Start");

                nRet = cDeviceParam.FeatureLoad(strDevFeatureFile);
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    Console.WriteLine("Device FeatureLoad failed:{0:x8}", nRet);
                    break;
                }
                Console.WriteLine("\t\tDevice FeatureLoad success");

            } while (false);

            // ch:关闭设备 | en:Close device
            if (null != cDevice)
            {
                nRet = cDevice.CloseDevice();
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    Console.WriteLine("Close device failed:{0:x8}", nRet);
                }
                cDevice = null;
            }

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
