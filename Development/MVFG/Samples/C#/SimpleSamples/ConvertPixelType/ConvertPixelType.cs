/*

This program shows how to use the MVFGControl method to convert the pixel format of the image.

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
        // ch:是否为Mono格式 | en:Judge whether it is Mono format
        static bool IsMonoPixelFormat(MV_FG_PIXEL_TYPE enType)
        {
            switch (enType)
            {
                case MV_FG_PIXEL_TYPE.MV_FG_PIXEL_TYPE_Mono10:
                case MV_FG_PIXEL_TYPE.MV_FG_PIXEL_TYPE_Mono10_Packed:
                case MV_FG_PIXEL_TYPE.MV_FG_PIXEL_TYPE_Mono12:
                case MV_FG_PIXEL_TYPE.MV_FG_PIXEL_TYPE_Mono12_Packed:
                case MV_FG_PIXEL_TYPE.MV_FG_PIXEL_TYPE_Mono16:
                    return true;
                default:
                    return false;
            }
        }

        // ch:是否为彩色格式 | en:Judge whether it is color format
        static bool IsColorPixelFormat(MV_FG_PIXEL_TYPE enType)
        {
            switch (enType)
            {
                case MV_FG_PIXEL_TYPE.MV_FG_PIXEL_TYPE_RGBA8_Packed:
                case MV_FG_PIXEL_TYPE.MV_FG_PIXEL_TYPE_BGRA8_Packed:
                case MV_FG_PIXEL_TYPE.MV_FG_PIXEL_TYPE_BayerGR8:
                case MV_FG_PIXEL_TYPE.MV_FG_PIXEL_TYPE_BayerRG8:
                case MV_FG_PIXEL_TYPE.MV_FG_PIXEL_TYPE_BayerGB8:
                case MV_FG_PIXEL_TYPE.MV_FG_PIXEL_TYPE_BayerBG8:
                case MV_FG_PIXEL_TYPE.MV_FG_PIXEL_TYPE_BayerGB10:
                case MV_FG_PIXEL_TYPE.MV_FG_PIXEL_TYPE_BayerGB10_Packed:
                case MV_FG_PIXEL_TYPE.MV_FG_PIXEL_TYPE_BayerBG10:
                case MV_FG_PIXEL_TYPE.MV_FG_PIXEL_TYPE_BayerBG10_Packed:
                case MV_FG_PIXEL_TYPE.MV_FG_PIXEL_TYPE_BayerRG10:
                case MV_FG_PIXEL_TYPE.MV_FG_PIXEL_TYPE_BayerRG10_Packed:
                case MV_FG_PIXEL_TYPE.MV_FG_PIXEL_TYPE_BayerGR10:
                case MV_FG_PIXEL_TYPE.MV_FG_PIXEL_TYPE_BayerGR10_Packed:
                case MV_FG_PIXEL_TYPE.MV_FG_PIXEL_TYPE_BayerGB12:
                case MV_FG_PIXEL_TYPE.MV_FG_PIXEL_TYPE_BayerGB12_Packed:
                case MV_FG_PIXEL_TYPE.MV_FG_PIXEL_TYPE_BayerBG12:
                case MV_FG_PIXEL_TYPE.MV_FG_PIXEL_TYPE_BayerBG12_Packed:
                case MV_FG_PIXEL_TYPE.MV_FG_PIXEL_TYPE_BayerRG12:
                case MV_FG_PIXEL_TYPE.MV_FG_PIXEL_TYPE_BayerRG12_Packed:
                case MV_FG_PIXEL_TYPE.MV_FG_PIXEL_TYPE_BayerGR12:
                case MV_FG_PIXEL_TYPE.MV_FG_PIXEL_TYPE_BayerGR12_Packed:
                case MV_FG_PIXEL_TYPE.MV_FG_PIXEL_TYPE_BayerGR16:
                case MV_FG_PIXEL_TYPE.MV_FG_PIXEL_TYPE_BayerRG16:
                case MV_FG_PIXEL_TYPE.MV_FG_PIXEL_TYPE_BayerGB16:
                case MV_FG_PIXEL_TYPE.MV_FG_PIXEL_TYPE_BayerBG16:
                case MV_FG_PIXEL_TYPE.MV_FG_PIXEL_TYPE_YUV422_Packed:
                case MV_FG_PIXEL_TYPE.MV_FG_PIXEL_TYPE_YUV422_YUYV_Packed:
                    return true;
                default:
                    return false;
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

                // ch:开始取流 | en:Start Acquisition
                nRet = cStream.StartAcquisition();
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    Console.WriteLine("Start acquistion failed:{0:x8}", nRet);
                    break;
                }

                const uint               nTimeOut = 1000;
                MV_FG_BUFFER_INFO        stFrameInfo = new MV_FG_BUFFER_INFO();
                MV_FG_CONVERT_PIXEL_INFO stConvertPixelInfo = new MV_FG_CONVERT_PIXEL_INFO();

                // ch:获取一帧图像 | en:Get one frame info
                nRet = cStream.GetFrameBuffer(ref stFrameInfo, nTimeOut);
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    Console.WriteLine("Get frame buffer failed:{0:x8}", nRet);
                }
                else
                {
                    Console.WriteLine("FrameNumber: " + stFrameInfo.nFrameID.ToString() +
                            ", Width: " + stFrameInfo.nWidth.ToString() + ", Height: " + stFrameInfo.nHeight.ToString());

                    MV_FG_PIXEL_TYPE enDstPixelType = MV_FG_PIXEL_TYPE.MV_FG_PIXEL_TYPE_Undefined;
                    uint             nChannelNum = 0;           // ch:像素通道数 | en:Channel number of pixel
                    string           strFileName = "Cvt.raw";
                    IntPtr           pDstConvertBuf = IntPtr.Zero;

                    // ch:彩色转成RGB8 | en:Convert color format to RGB8
                    if (IsColorPixelFormat(stFrameInfo.enPixelType))
                    {
                        nChannelNum = 3;
                        enDstPixelType = MV_FG_PIXEL_TYPE.MV_FG_PIXEL_TYPE_RGB8_Packed;
                        strFileName = "Cvt2RGB_w" + stFrameInfo.nWidth + "_h" + stFrameInfo.nHeight + ".raw";
                    }
                    // ch:黑白则换成Mono8 | en:Convert gray to Mono8
                    else if (IsMonoPixelFormat(stFrameInfo.enPixelType))
                    {
                        nChannelNum = 1;
                        enDstPixelType = MV_FG_PIXEL_TYPE.MV_FG_PIXEL_TYPE_Mono8;
                        strFileName = "Cvt2Mono_w" + stFrameInfo.nWidth + "_h" + stFrameInfo.nHeight + ".raw";
                    }
                    else
                    {
                        Console.WriteLine("Don't need to convert!");
                    }

                    if (MV_FG_PIXEL_TYPE.MV_FG_PIXEL_TYPE_Undefined != enDstPixelType)
                    {
                        if (IntPtr.Zero == pDstConvertBuf)
                        {
                            pDstConvertBuf = Marshal.AllocHGlobal(new IntPtr(stFrameInfo.nWidth * stFrameInfo.nHeight * nChannelNum));
                        }

                        if (IntPtr.Zero != pDstConvertBuf)
                        {
                            // 配置参数
                            stConvertPixelInfo.stInputImageInfo.pImageBuf = stFrameInfo.pBuffer;
                            stConvertPixelInfo.stInputImageInfo.nImageBufLen = stFrameInfo.nFilledSize;
                            stConvertPixelInfo.stInputImageInfo.nHeight = stFrameInfo.nHeight;
                            stConvertPixelInfo.stInputImageInfo.nWidth = stFrameInfo.nWidth;
                            stConvertPixelInfo.stInputImageInfo.enPixelType = stFrameInfo.enPixelType;
                            stConvertPixelInfo.stOutputImageInfo.enPixelType = enDstPixelType;
                            stConvertPixelInfo.stOutputImageInfo.pImageBuf = pDstConvertBuf;
                            stConvertPixelInfo.stOutputImageInfo.nImageBufSize = (uint)(stFrameInfo.nWidth * stFrameInfo.nHeight * nChannelNum);
                            stConvertPixelInfo.enCfaMethod = MV_FG_CFA_METHOD.MV_FG_CFA_METHOD_OPTIMAL;

                            // ch:像素格式转换 | en:Pixel format conversion
                            CImageProcess cImgProc = new CImageProcess(cStream);
                            nRet = cImgProc.ConvertPixelType(ref stConvertPixelInfo);
                            if (CErrorCode.MV_FG_SUCCESS != nRet)
                            {
                                Console.WriteLine("Convert pixel type failed:{0:x8}", nRet);
                            }
                            else
                            {
                                // ch:将图像数据保存到本地文件 | en:Save image data to local file
                                byte[] byteData = new byte[stConvertPixelInfo.stOutputImageInfo.nImageBufLen];
                                Marshal.Copy(stConvertPixelInfo.stOutputImageInfo.pImageBuf, byteData, 0, (int)stConvertPixelInfo.stOutputImageInfo.nImageBufLen);
                                FileStream pFile = null;

                                try
                                {
                                    pFile = new FileStream(strFileName, FileMode.Create);
                                    pFile.Write(byteData, 0, byteData.Length);
                                }
                                catch
                                {
                                    Console.WriteLine("Save image failed");
                                }
                                finally
                                {
                                    pFile.Close();
                                    pFile = null;
                                }
                            }
                        }
                        else
                        {
                            Console.WriteLine("Malloc memory failed");
                        }

                        // ch:将缓存放回输入队列 | en:Put the buffer back into the input queue
                        nRet = cStream.ReleaseFrameBuffer(stFrameInfo);
                        if (CErrorCode.MV_FG_SUCCESS != nRet)
                        {
                            Console.WriteLine("Release frame buffer failed:{0:x8}", nRet);
                        }
                        else
                        {
                            Console.WriteLine("Convert pixel format success!");
                        }
                    }

                    if (IntPtr.Zero != pDstConvertBuf)
                    {
                        Marshal.FreeHGlobal(pDstConvertBuf);
                    }
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
