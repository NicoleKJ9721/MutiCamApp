/*

This program shows how to use the MVFGControl method to set trigger function from Camera Link interface.

*/
using System;
using System.Collections.Generic;
using MvFGCtrlC.NET;
using System.Runtime.InteropServices;
using System.IO;
using System.Threading;

namespace CameraLinkTrigger
{
    class CameraLinkTrigger
    {
        // ch:帧信息回调函数 | en:Frame info callback
        static void ImageCallbackFunc(ref MV_FG_BUFFER_INFO stBufferInfo, IntPtr pUser)
        {
            if (null != stBufferInfo.pBuffer)
            {
                Console.WriteLine("FrameNumber: " + Convert.ToInt64(stBufferInfo.nFrameID).ToString() + ", Width: " +
                    stBufferInfo.nWidth.ToString() + ", Height: " + stBufferInfo.nHeight.ToString());
            }
        }

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
                nRet = cSystem.UpdateInterfaceList(
                    CParamDefine.MV_FG_CAMERALINK_INTERFACE,
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
                    // ch:Byte数组转换为结构体 | en:Convert byte array to structure
                    MV_CML_INTERFACE_INFO stCmlIFInfo = (MV_CML_INTERFACE_INFO)CAdditional.ByteToStruct(
                        stIfInfo.SpecialInfo.stCMLIfInfo, typeof(MV_CML_INTERFACE_INFO));
                    Console.WriteLine("[CML]No." + i.ToString() + " Interface:");
                    Console.WriteLine("\tDisplayName: " + stCmlIFInfo.chDisplayName);
                    Console.WriteLine("\tInterfaceID: " + stCmlIFInfo.chInterfaceID);
                    Console.WriteLine("\tSerialNumber: " + stCmlIFInfo.chSerialNumber);
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
                bool bCameraControl = false;
                int nIndex = -1;
                Console.WriteLine("Use Camera Control or not, 1.Yes, 2.No");
                try
                {
                    nIndex = Convert.ToInt32(Console.ReadLine());
                }
                catch
                {
                    Console.WriteLine("Invalid Index!");
                    break;
                }
                if (nIndex < 1 || nIndex > 2)
                {
                    Console.WriteLine("Error index");
                    break;
                }
                else if (1 == nIndex)
                {
                    bCameraControl = true;
                }

                MV_FG_ENUMVALUE stEnumValue = new MV_FG_ENUMVALUE();

                if (bCameraControl)
                {
                    nRet = cParam.SetEnumValueByString("CameraType", "LineScan");
                    if (CErrorCode.MV_FG_SUCCESS != nRet)
                    {
                        Console.WriteLine("Set Camera Type failed:{0:x8}", nRet);
                        break;
                    }
                    else
                    {
                        Console.WriteLine("Set Camera Type success");
                    }
                    // ch:固定CC通道为CC1 | en:Fixed CCSelector equal to CC1
                    nRet = cParam.SetEnumValueByString("CCSelector", "CC1");
                    if (CErrorCode.MV_FG_SUCCESS != nRet)
                    {
                        Console.WriteLine("Set Camera Control Selector failed:{0:x8}", nRet);
                        break;
                    }
                    else
                    {
                        Console.WriteLine("Set Camera Control Selector success");
                    }

                    nRet = cParam.GetEnumValue("CCSource", ref stEnumValue);
                    if (CErrorCode.MV_FG_SUCCESS != nRet)
                    {
                        Console.WriteLine("Get Camera Control Source failed:{0:x8}", nRet);
                        break;
                    }
                    Console.WriteLine("Select Camera Control Source, including.");
                    int nCCSourceIndex = -1;
                    for (uint i = 0; i < stEnumValue.nSupportedNum; i++)
                    {
                        Console.WriteLine("{0}. {1}", i, stEnumValue.strSymbolicArray[i].strInfo);
                    }
                    try
                    {
                        nCCSourceIndex = Convert.ToInt32(Console.ReadLine());
                    }
                    catch
                    {
                        Console.WriteLine("Invalid Index!");
                        break;
                    }

                    if (nCCSourceIndex < 0 || nCCSourceIndex >= stEnumValue.nSupportedNum)
                    {
                        Console.WriteLine("Error Index!");
                        break;
                    }
                    // ch:通过采集卡设置相机控制触发源 | en:Set Camera Control trigger source from interface
                    nRet = cParam.SetEnumValue("CCSource", stEnumValue.nSupportValueArray[nCCSourceIndex]);
                    if (CErrorCode.MV_FG_SUCCESS != nRet)
                    {
                        Console.WriteLine("Set Camera Control Source failed:{0:x8}", nRet);
                        break;
                    }
                    else
                    {
                        Console.WriteLine("Set Camera Control Source success");
                    }
                }
                else
                {
                    nRet = cParam.GetEnumValue("StreamTriggerSource", ref stEnumValue);
                    if (CErrorCode.MV_FG_SUCCESS != nRet)
                    {
                        Console.WriteLine("Get Stream Trigger Source failed:{0:x8}", nRet);
                        break;
                    }
                    Console.WriteLine("Select Stream Trigger Source, including.");
                    int nTriggerSourceIndex = -1;
                    for (uint i = 0; i < stEnumValue.nSupportedNum; i++)
                    {
                        Console.WriteLine("{0}. {1}", i, stEnumValue.strSymbolicArray[i].strInfo);
                    }
                    try
                    {
                        nTriggerSourceIndex = Convert.ToInt32(Console.ReadLine());
                    }
                    catch
                    {
                        Console.WriteLine("Invalid Index!");
                        break;
                    }

                    if (nTriggerSourceIndex < 0 || nTriggerSourceIndex >= stEnumValue.nSupportedNum)
                    {
                        Console.WriteLine("Error Index!");
                        break;
                    }

                    // ch:通过采集卡设置触发源 | en:Set trigger source from interface
                    nRet = cParam.SetEnumValue("StreamTriggerSource", stEnumValue.nSupportValueArray[nTriggerSourceIndex]);
                    if (CErrorCode.MV_FG_SUCCESS != nRet)
                    {
                        Console.WriteLine("Set Stream Trigger Source failed:{0:x8}", nRet);
                        break;
                    }
                    else
                    {
                        Console.WriteLine("Set Stream Trigger Source success");
                    }
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
                    // ch:Byte数组转换为结构体 | en:Convert byte array to structure
                    MV_CML_DEVICE_INFO stCmlDevInfo = (MV_CML_DEVICE_INFO)CAdditional.ByteToStruct(
                        stDeviceInfo.DevInfo.stCMLDevInfo, typeof(MV_CML_DEVICE_INFO));
                    Console.WriteLine("[CML]No." + i.ToString() + " Device:");
                    Console.WriteLine("\tUserDefinedName: " + stCmlDevInfo.chUserDefinedName);
                    Console.WriteLine("\tModelName: " + stCmlDevInfo.chModelName);
                    Console.WriteLine("\tSerialNumber: " + stCmlDevInfo.chSerialNumber);
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

                CParam cDeviceParam = new CParam(cDevice);      // ch:操作采集卡或设备参数配置 | en:Interface or device config operation

                if (bCameraControl)
                {
                    // ch:打开设备触发模式 | en:Open device trigger mode
                    nRet = cDeviceParam.SetEnumValueByString("TriggerMode", "On");
                    if (CErrorCode.MV_FG_SUCCESS != nRet)
                    {
                        Console.WriteLine("Turn on device trigger mode failed:{0:x8}", nRet);
                        break;
                    }
                    // ch:固定设备触发源为CC1 | en:Fixed device trigger source equal to CC1
                    nRet = cDeviceParam.SetEnumValueByString("TriggerSource", "CC1");
                    if (CErrorCode.MV_FG_SUCCESS != nRet)
                    {
                        Console.WriteLine("Set device trigger source failed:{0:x8}", nRet);
                        break;
                    }
                    else
                    {
                        Console.WriteLine("Set device trigger source success");
                    }
                }
                else
                {
                    // ch:关闭设备触发模式 | en:Close device trigger mode
                    nRet = cDeviceParam.SetEnumValueByString("TriggerMode", "Off");
                    if (CErrorCode.MV_FG_SUCCESS != nRet)
                    {
                        Console.WriteLine("Turn off device trigger mode failed:{0:x8}", nRet);
                        break;
                    }
                }

                // ch:获取流通道个数 | en:Get number of stream
                uint nStreamNum = 0;
                nRet = cDevice.GetNumStreams(ref nStreamNum);
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    Console.WriteLine("Get stream number failed:{0:x8}", nRet);
                    break;
                }
                if (0 == nStreamNum)
                {
                    Console.WriteLine("No stream available");
                    break;
                }

                // ch:打开流通道(目前只支持单个通道) | en:Open stream(Only a single stream is supported now)
                nRet = cDevice.OpenStream(0, out cStream);
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    Console.WriteLine("Open stream failed:{0:x8}", nRet);
                    break;
                }

                // ch:设置SDK内部缓存数量 | en:Set internal buffer number
                const uint nBufNum = 3;
                nRet = cStream.SetBufferNum(nBufNum);
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    Console.WriteLine("Set buffer number failed:{0:x8}", nRet);
                    break;
                }

                CStream.ImageDelegate DelegateImageCallBack = new CStream.ImageDelegate(ImageCallbackFunc);
                // ch:注册帧缓存信息回调函数 | en:Register frame info callback
                nRet = cStream.RegisterImageCallBack(DelegateImageCallBack, IntPtr.Zero, true);
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    Console.WriteLine("Register image callback failed:{0:x8}", nRet);
                    break;
                }

                // ch:开始取流 | en:Start Acquisition
                nRet = cStream.StartAcquisition();
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    Console.WriteLine("Start acquistion failed:{0:x8}", nRet);
                    break;
                }

                Console.WriteLine("Press Enter to stop acquisition.");
                if (!bCameraControl)
                {
                    nRet = cParam.GetEnumValue("StreamTriggerSource", ref stEnumValue);
                    if (CErrorCode.MV_FG_SUCCESS != nRet)
                    {
                        Console.WriteLine("Get Stream Trigger Source failed:{0:x8}", nRet);
                        break;
                    }
                    if ("SoftwareSignal0" == stEnumValue.strCurSymbolic)
                    {
                        bool bExitLoop = false;
                        while (!bExitLoop) {
                            // ch:通过采集卡软触发一次 | en:Software trigger once from interface
                            nRet = cParam.SetCommandValue("StreamSoftwareTrigger");
                            if (CErrorCode.MV_FG_SUCCESS != nRet)
                            {
                                Console.WriteLine("Software Trigger once failed:{0:x8}", nRet);
                            }
                            else
                            {
                                Console.WriteLine("Software Trigger once success", nRet);
                            }
                            Thread.Sleep(2000);
                            if (Console.KeyAvailable)
                            {
                                Console.ReadKey();
                                bExitLoop = true;
                            }
                        }
                    }
                    else
                    {
                        Console.ReadKey();
                    }
                }
                else
                {
                    Console.ReadKey();
                }

                // ch:停止取流 | en:Stop Acquisition
                nRet = cStream.StopAcquisition();
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    Console.WriteLine("Stop acquistion failed:{0:x8}", nRet);
                    break;
                }
            } while (false);

            // ch:关闭流通道 | en:Close Stream
            if (null != cStream)
            {
                nRet = cStream.CloseStream();
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    Console.WriteLine("Close stream failed:{0:x8}", nRet);
                }
                cStream = null;
            }

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
