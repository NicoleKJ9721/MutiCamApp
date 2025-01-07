/*

This program shows how to use the MVFGControl method to open chunk mode and parse chunk data.

*/
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using MvFGCtrlC.NET;
using System.Runtime.InteropServices;
using System.IO;

namespace ChunkData
{
    class ChunkData
    {
        static uint CHUNK_ID_TIMESTAMP_LITTLE = unchecked(0xa5a50101);   // ch:时间戳 | en:Timestamp
        static uint CHUNK_ID_EXPOSURE_LITTLE = unchecked(0xa5a50103);   // ch:曝光 | en:Exposure

        // ch:帧信息回调函数 | en:Frame info callback
        static void ImageCallbackFunc(ref MV_FG_BUFFER_INFO stBufferInfo, IntPtr pUser)
        {
            if (null != stBufferInfo.pBuffer)
            {
                Console.WriteLine("FrameNumber: " + stBufferInfo.nFrameID.ToString() + ", Width: " +
                    stBufferInfo.nWidth.ToString() + ", Height: " + stBufferInfo.nHeight.ToString());

                int                     nRet = 0;
                GCHandle                handle = GCHandle.FromIntPtr(pUser);
                CStream                 cStreamTemp = handle.Target as CStream;
                uint                    nChunkNum = stBufferInfo.nNumChunks;                // Chunk个数
                MV_FG_CHUNK_DATA_INFO   stChunkDataInfo = new MV_FG_CHUNK_DATA_INFO();      // Chunk信息

                // ch:打印chunk信息 | en:Print chunk info
                Console.WriteLine("********************");
                for (uint i = 0; i < nChunkNum; i++)
                {
                    nRet = cStreamTemp.GetBufferChunkData(stBufferInfo, i, ref stChunkDataInfo);
                    if (CErrorCode.MV_FG_SUCCESS != nRet)
                    {
                        Console.WriteLine("Get chunk data failed:{0:x8}", nRet);
                        Console.WriteLine("********************");
                        break;
                    }

                    if (CHUNK_ID_TIMESTAMP_LITTLE == stChunkDataInfo.nChunkID)
                    {
                        UInt32 nTimeStamp = Convert.ToUInt32(Marshal.ReadInt32(stChunkDataInfo.pChunkData));
                        Console.WriteLine("Chunk ID[{0:x8}], Chunk length[{1}], Chunk data[{2}]",
                            stChunkDataInfo.nChunkID, stChunkDataInfo.nChunkLen, nTimeStamp);
                    }
                    else if (CHUNK_ID_EXPOSURE_LITTLE == stChunkDataInfo.nChunkID)
                    {
                        float fExposure = Convert.ToSingle(Marshal.ReadInt32(stChunkDataInfo.pChunkData));
                        Console.WriteLine("Chunk ID[{0:x8}], Chunk length[{1}], Chunk data[{2:f}]",
                            stChunkDataInfo.nChunkID, stChunkDataInfo.nChunkLen, fExposure);
                    }
                }
                Console.WriteLine("********************");
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

                // ch:开启Chunk Mode | en:Open Chunk Mode
                nRet = cDeviceParam.SetBoolValue("ChunkModeActive", true);
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    Console.WriteLine("Set chunk mode failed:{0:x8}", nRet);
                    break;
                }

                // ch:Chunk Selector设为Exposure | en: Chunk Selector set as Exposure
                nRet = cDeviceParam.SetEnumValueByString("ChunkSelector", "Exposure");
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    Console.WriteLine("Set exposure chunk failed:{0:x8}", nRet);
                    break;
                }

                // ch:开启Chunk Enable | en:Open Chunk Enable
                nRet = cDeviceParam.SetBoolValue("ChunkEnable", true);
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    Console.WriteLine("Set exposure chunk enable failed:{0:x8}", nRet);
                    break;
                }

                // ch:Chunk Selector设为Timestamp | en: Chunk Selector set as Timestamp
                nRet = cDeviceParam.SetEnumValueByString("ChunkSelector", "Timestamp");
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    Console.WriteLine("Set timestamp chunk failed:{0:x8}", nRet);
                    break;
                }

                // ch:开启Chunk Enable | en:Open Chunk Enable
                nRet = cDeviceParam.SetBoolValue("ChunkEnable", true);
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    Console.WriteLine("Set timestamp chunk enable failed:{0:x8}", nRet);
                    break;
                }

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

                // ch:设置SDK内部缓存数量 | en:Set internal buffer number
                const uint nBufNum = 3;
                nRet = cStream.SetBufferNum(nBufNum);
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    Console.WriteLine("Set buffer number failed:{0:x8}", nRet);
                    break;
                }

                // ch:注册帧缓存信息回调函数 | en:Register frame info callback
                GCHandle handle = GCHandle.Alloc(cStream);
                CStream.ImageDelegate DelegateImageCallBack = new CStream.ImageDelegate(ImageCallbackFunc);
                // ch:注册帧缓存信息回调函数 | en:Register frame info callback
                nRet = cStream.RegisterImageCallBack(DelegateImageCallBack, GCHandle.ToIntPtr(handle), true);
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
                Console.ReadKey();

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
