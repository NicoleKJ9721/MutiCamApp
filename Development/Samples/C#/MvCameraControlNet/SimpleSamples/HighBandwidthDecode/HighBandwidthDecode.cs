using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using MvCamCtrl.NET;
using System.Runtime.InteropServices;
using System.IO;
using System.Threading;

namespace HighBandwidthDecode
{
    class HighBandwidthDecode
    {
        static void Main(string[] args)
        {
            int nRet = MyCamera.MV_OK;

            MyCamera device = new MyCamera();
            IntPtr pBufForDecode = IntPtr.Zero;

            // ch: 初始化 SDK | en: Initialize SDK
            MyCamera.MV_CC_Initialize_NET();

            do
            {
                // ch:枚举设备 | en:Enum device
                MyCamera.MV_CC_DEVICE_INFO_LIST stDevList = new MyCamera.MV_CC_DEVICE_INFO_LIST();
                nRet = MyCamera.MV_CC_EnumDevices_NET(MyCamera.MV_GIGE_DEVICE | MyCamera.MV_USB_DEVICE, ref stDevList);
                //nRet = MyCamera.MV_CC_EnumDevices_NET(MyCamera.MV_GIGE_DEVICE, ref stDevList);
                if (MyCamera.MV_OK != nRet)
                {
                    Console.WriteLine("Enum device failed:{0:x8}", nRet);
                    break;
                }
                Console.WriteLine("Enum device count : " + Convert.ToString(stDevList.nDeviceNum));
                if (0 == stDevList.nDeviceNum)
                {
                    break;
                }

                MyCamera.MV_CC_DEVICE_INFO stDevInfo;                            // 通用设备信息

                // ch:打印设备信息 en:Print device info
                for (Int32 i = 0; i < stDevList.nDeviceNum; i++)
                {
                    stDevInfo = (MyCamera.MV_CC_DEVICE_INFO)Marshal.PtrToStructure(stDevList.pDeviceInfo[i], typeof(MyCamera.MV_CC_DEVICE_INFO));

                    if (MyCamera.MV_GIGE_DEVICE == stDevInfo.nTLayerType)
                    {
                        MyCamera.MV_GIGE_DEVICE_INFO_EX stGigEDeviceInfo = (MyCamera.MV_GIGE_DEVICE_INFO_EX)MyCamera.ByteToStruct(stDevInfo.SpecialInfo.stGigEInfo, typeof(MyCamera.MV_GIGE_DEVICE_INFO_EX));
                        uint nIp1 = ((stGigEDeviceInfo.nCurrentIp & 0xff000000) >> 24);
                        uint nIp2 = ((stGigEDeviceInfo.nCurrentIp & 0x00ff0000) >> 16);
                        uint nIp3 = ((stGigEDeviceInfo.nCurrentIp & 0x0000ff00) >> 8);
                        uint nIp4 = (stGigEDeviceInfo.nCurrentIp & 0x000000ff);
                        Console.WriteLine("[device " + i.ToString() + "]:");
                        Console.WriteLine("DevIP:" + nIp1 + "." + nIp2 + "." + nIp3 + "." + nIp4);
                        Console.WriteLine("ModelName:" + stGigEDeviceInfo.chModelName + "\n");
                    }
                    else if (MyCamera.MV_USB_DEVICE == stDevInfo.nTLayerType)
                    {
                        MyCamera.MV_USB3_DEVICE_INFO_EX stUsb3DeviceInfo = (MyCamera.MV_USB3_DEVICE_INFO_EX)MyCamera.ByteToStruct(stDevInfo.SpecialInfo.stUsb3VInfo, typeof(MyCamera.MV_USB3_DEVICE_INFO_EX));
                        Console.WriteLine("[device " + i.ToString() + "]:");
                        Console.WriteLine("SerialNumber:" + stUsb3DeviceInfo.chSerialNumber);
                        Console.WriteLine("ModelName:" + stUsb3DeviceInfo.chModelName + "\n");
                    }
                }

                Int32 nDevIndex = 0;
                Console.Write("Please input index(0-{0:d}):", stDevList.nDeviceNum - 1);
                try
                {
                    nDevIndex = Convert.ToInt32(Console.ReadLine());
                }
                catch
                {
                    Console.Write("Invalid Input!\n");
                    break;
                }

                if (nDevIndex > stDevList.nDeviceNum - 1 || nDevIndex < 0)
                {
                    Console.Write("Input Error!\n");
                    break;
                }
                stDevInfo = (MyCamera.MV_CC_DEVICE_INFO)Marshal.PtrToStructure(stDevList.pDeviceInfo[nDevIndex], typeof(MyCamera.MV_CC_DEVICE_INFO));

                // ch:创建设备 | en:Create device
                nRet = device.MV_CC_CreateDevice_NET(ref stDevInfo);
                if (MyCamera.MV_OK != nRet)
                {
                    Console.WriteLine("Create device failed:{0:x8}", nRet);
                    break;
                }

                // ch:打开设备 | en:Open device
                nRet = device.MV_CC_OpenDevice_NET();
                if (MyCamera.MV_OK != nRet)
                {
                    Console.WriteLine("Open device failed:{0:x8}", nRet);
                    break;
                }

                // ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera)
                if (stDevInfo.nTLayerType == MyCamera.MV_GIGE_DEVICE)
                {
                    int nPacketSize = device.MV_CC_GetOptimalPacketSize_NET();
                    if (nPacketSize > 0)
                    {
                        nRet = device.MV_CC_SetIntValueEx_NET("GevSCPSPacketSize", nPacketSize);
                        if (nRet != MyCamera.MV_OK)
                        {
                            Console.WriteLine("Warning: Set Packet Size failed {0:x8}", nRet);
                        }
                    }
                    else
                    {
                        Console.WriteLine("Warning: Get Packet Size failed {0:x8}", nPacketSize);
                    }
                }

                // ch:设置触发模式为off || en:set trigger mode as off
                if (MyCamera.MV_OK != device.MV_CC_SetEnumValue_NET("TriggerMode", 0))
                {
                    Console.WriteLine("Set TriggerMode failed:{0:x8}", nRet);
                    break;
                }

                // ch:获取数据包大小 | en:Get payload size
                MyCamera.MVCC_INTVALUE stParam = new MyCamera.MVCC_INTVALUE();
                nRet = device.MV_CC_GetIntValue_NET("PayloadSize", ref stParam);
                if (MyCamera.MV_OK != nRet)
                {
                    Console.WriteLine("Get IntValue failed:{0:x8}", nRet);
                    break;
                }
                UInt32 nPayloadSize = stParam.nCurValue;

                // ch:开启抓图 | en:start grab
                nRet = device.MV_CC_StartGrabbing_NET();
                if (MyCamera.MV_OK != nRet)
                {
                    Console.WriteLine("Start grabbing failed:{0:x8}", nRet);
                    break;
                }

                MyCamera.MV_FRAME_OUT stImageInfo = new MyCamera.MV_FRAME_OUT();
                UInt32 nImageNum = 10;
                string chImageName;

                MyCamera.MV_CC_HB_DECODE_PARAM stDecodeParam = new MyCamera.MV_CC_HB_DECODE_PARAM();
                for (UInt32 i = 0; i < nImageNum; i++)
                {
                    nRet = device.MV_CC_GetImageBuffer_NET(ref stImageInfo, 1000);
                    if (MyCamera.MV_OK == nRet)
                    {
                        Console.WriteLine("Get One Frame:" + "Width[" + Convert.ToString(stImageInfo.stFrameInfo.nWidth) + "] , Height[" + Convert.ToString(stImageInfo.stFrameInfo.nHeight)
                            + "] , FrameNum[" + Convert.ToString(stImageInfo.stFrameInfo.nFrameNum) + "], PixelFormat[" + Convert.ToString(stImageInfo.stFrameInfo.enPixelType) + "]");
                        stDecodeParam.pSrcBuf = stImageInfo.pBufAddr;
                        stDecodeParam.nSrcLen = stImageInfo.stFrameInfo.nFrameLen;
                        if (pBufForDecode == IntPtr.Zero)
                        {
                            pBufForDecode = Marshal.AllocHGlobal((int)(nPayloadSize));
                        }
                        stDecodeParam.pDstBuf = pBufForDecode;
                        stDecodeParam.nDstBufSize = nPayloadSize;
                        nRet = device.MV_CC_HB_Decode_NET(ref stDecodeParam);
                        if (MyCamera.MV_OK != nRet)
                        {
                            Console.WriteLine("Decode failed:{0:x8}", nRet);
                            break;
                        }
                        device.MV_CC_FreeImageBuffer_NET(ref stImageInfo);
                        chImageName = "Image_w" + stDecodeParam.nWidth.ToString() + "_h" + stDecodeParam.nHeight.ToString() + "_fn" + stImageInfo.stFrameInfo.nFrameNum.ToString() + ".raw";
                        byte[] data = new byte[stDecodeParam.nDstBufLen];
                        Marshal.Copy(stDecodeParam.pDstBuf, data, 0, (int)stDecodeParam.nDstBufLen);
                        FileStream pFile = null;
                        try
                        {
                            pFile = new FileStream(chImageName, FileMode.Create);
                            pFile.Write(data, 0, data.Length);
                        }
                        catch
                        {
                            Console.WriteLine("保存失败");
                        }
                        finally
                        {
                            pFile.Close();
                        }
                        Console.WriteLine("Decode succeed!");
                    }
                    else
                    {
                        Console.WriteLine("Get Image failed:{0:x8}", nRet);
                    }

                }

                // ch:停止抓图 | en:Stop grab image
                nRet = device.MV_CC_StopGrabbing_NET();
                if (MyCamera.MV_OK != nRet)
                {
                    Console.WriteLine("Stop grabbing failed:{0:x8}", nRet);
                    break;
                }

                // ch:关闭设备 | en:Close device
                nRet = device.MV_CC_CloseDevice_NET();
                if (MyCamera.MV_OK != nRet)
                {
                    Console.WriteLine("Close device failed:{0:x8}", nRet);
                    break;
                }

                // ch:销毁设备 | en:Destroy device
                nRet = device.MV_CC_DestroyDevice_NET();
                if (MyCamera.MV_OK != nRet)
                {
                    Console.WriteLine("Destroy device failed:{0:x8}", nRet);
                    break;
                }
            } while (false);
            if (pBufForDecode != IntPtr.Zero)
            {
                Marshal.FreeHGlobal(pBufForDecode);
            }

            if (MyCamera.MV_OK != nRet)
            {
                // ch:销毁设备 | en:Destroy device
                nRet = device.MV_CC_DestroyDevice_NET();
                if (MyCamera.MV_OK != nRet)
                {
                    Console.WriteLine("Destroy device failed:{0:x8}", nRet);
                }
            }

            // ch: 反初始化SDK | en: Finalize SDK
            MyCamera.MV_CC_Finalize_NET();

            Console.WriteLine("Press enter to exit");
            Console.ReadKey();
        }
    }
}
