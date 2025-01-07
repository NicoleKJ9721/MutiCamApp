/*

This program shows how to use the user's own buffers to grab image.

*/
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using MvFGCtrlC.NET;
using System.Runtime.InteropServices;
using System.IO;
using System.Threading;

namespace GrabImage
{
    class GrabImage
    {
        static bool g_bExit = false;
        const int BUFFER_NUMBER = 3;       // ch:申请的缓存个数 | en:Number of requested buffer

        public static void GrabbingThread(object obj)
        {
            if (null != obj)
            {
                int                 nRet = 0;
                const uint          nTimeout = 1000;
                CStream             cStream = obj as CStream;
                CBuffer cBuffer = null;
                MV_FG_BUFFER_INFO   stFrameInfo = new MV_FG_BUFFER_INFO();    // 图像信息

                // ch:开始取流 | en:Start Acquisition
                nRet = cStream.StartAcquisition();
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    Console.WriteLine("Start acquistion failed:{0:x8}", nRet);
                    return;
                }
                g_bExit = false;

                while (!g_bExit)
                {
                    // ch:获取一帧图像缓存信息 | en:Get one frame buffer's info
                    nRet = cStream.GetImageBuffer(out cBuffer, nTimeout);
                    if (CErrorCode.MV_FG_SUCCESS != nRet)
                    {
                        Console.WriteLine("Get image buffer failed:{0:x8}", nRet);
                        continue;
                    }
                    else
                    {
                        nRet = cBuffer.GetBufferInfo(ref stFrameInfo);
                        if (CErrorCode.MV_FG_SUCCESS == nRet)
                        {
                            Console.WriteLine("FrameNumber: " + stFrameInfo.nFrameID.ToString() +
                                ", Width: " + stFrameInfo.nWidth.ToString() + ", Height: " + stFrameInfo.nHeight.ToString());
                        }

                        cBuffer.QueueBuffer();
                    }
                }

                // ch:停止取流 | en:Stop Acquisition
                nRet = cStream.StopAcquisition();
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    Console.WriteLine("Stop acquistion failed:{0:x8}", nRet);
                    return;
                }
            }
        }

        static void Main(string[] args)
        {
            int         nRet = CErrorCode.MV_FG_SUCCESS;
            CSystem     cSystem = new CSystem();            // ch:操作采集卡 | en:Interface operations
            CInterface  cInterface = null;                  // ch:操作采集卡和设备 | en:Interface and device operation
            CDevice     cDevice = null;                     // ch:操作设备和流 | en:Device and stream operation
            CStream     cStream = null;                     // ch:操作流和缓存 | en:Stream and buffer operation

            IntPtr[]    ptrBuffers = new IntPtr[BUFFER_NUMBER]; // ch:待注册的缓存 | en:Buffers to be announced
            CBuffer[] cBuffers = new CBuffer[BUFFER_NUMBER];    // ch:注册的缓存实例 | en:Instances of announced buffers

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

                CParam cDeviceParam = new CParam(cDevice);      // ch:操作采集卡或设备参数配置 | en:Interface or device config operation

                // ch:关闭触发模式 | en:Close trigger mode
                nRet = cDeviceParam.SetEnumValueByString("TriggerMode", "Off");
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    Console.WriteLine("Turn off trigger mode failed:{0:x8}", nRet);
                    break;
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

                // ch:获取流通道图像大小 | en:Get payload size of stream
                UInt32 nPayloadSize = 0;
                nRet = cStream.GetPayloadSize(ref nPayloadSize);
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    Console.WriteLine("Get payload size failed! {0:x8}", nRet);
                    break;
                }

                // ch:注册缓存 | en:Announce buffer
                for (uint i = 0; i < BUFFER_NUMBER; i++)
                {
                    // ch:分配图像缓存 | en:Allocate image memory
                    ptrBuffers[i] = CSystem.MV_MEM_Alloc(nPayloadSize, 64);
                    if (IntPtr.Zero == ptrBuffers[i])
                    {
                        Console.WriteLine("Allocate buffer failed!");
                        nRet = CErrorCode.MV_FG_ERR_RESOURCE_EXHAUSTED;
                        break;
                    }

                    // ch:向SDK注册缓存 | en:Announce buffer for SDK
                    nRet = cStream.AnnounceBuffer(ptrBuffers[i], nPayloadSize, IntPtr.Zero, out cBuffers[i]);
                    if (CErrorCode.MV_FG_SUCCESS != nRet)
                    {
                        Console.WriteLine("Announce buffer failed! {0:x8}", nRet);
                        break;
                    }
                }
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    break;
                }

                // ch:刷新缓存队列，用于取流 | en:Flush buffer for grabbing image
                nRet = cStream.FlushQueue(MV_FG_BUFFER_QUEUE_TYPE.MV_FG_BUFFER_QUEUE_ALL_TO_INPUT);
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    Console.WriteLine("Flush queue: all to input failed! %#x\n", nRet);
                    break;
                }

                // ch:创建取流线程 | en:Create acquistion thread
                Thread hGrabThread = new Thread(GrabbingThread);
                if (null == hGrabThread)
                {
                    Console.WriteLine("Create thread failed");
                    break;
                }
                hGrabThread.Start(cStream);

                Console.WriteLine("Press Enter to stop acquisition.");
                Console.ReadKey();

                g_bExit = true;
                hGrabThread.Join();

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
