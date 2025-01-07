using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using MvCamCtrl.NET;
using System.Runtime.InteropServices;
using System.IO;
using System.Threading;

namespace LensShadingCorrection
{
    class LensShadingCorrection
    {
        public static MyCamera.cbOutputExdelegate ImageCallback;
        public static MyCamera device = new MyCamera();
        static bool g_IsNeedCalib = true;
        static IntPtr g_pCalibBuf = IntPtr.Zero;
        static IntPtr g_pDstData = IntPtr.Zero;
        static uint g_nCalibBufSize = 0;
        static uint g_nDstDataSize = 0;

        static void ImageCallbackFunc(IntPtr pData, ref MyCamera.MV_FRAME_OUT_INFO_EX pFrameInfo, IntPtr pUser)
        {
            int nRet = MyCamera.MV_OK;
            Console.WriteLine("Get one frame: Width[" + Convert.ToString(pFrameInfo.nWidth) + "] , Height[" + Convert.ToString(pFrameInfo.nHeight)
                                    + "] , FrameNum[" + Convert.ToString(pFrameInfo.nFrameNum) + "]");
            //判断是否需要标定
            if (true == g_IsNeedCalib)
            {
                //LSC标定
                MyCamera.MV_CC_LSC_CALIB_PARAM stLSCCalib = new MyCamera.MV_CC_LSC_CALIB_PARAM();
                stLSCCalib.nWidth = pFrameInfo.nWidth;
                stLSCCalib.nHeight = pFrameInfo.nHeight;
                stLSCCalib.enPixelType = pFrameInfo.enPixelType;
                stLSCCalib.pSrcBuf = pData;
                stLSCCalib.nSrcBufLen = pFrameInfo.nFrameLen;

                if (g_pCalibBuf == IntPtr.Zero || g_nCalibBufSize < pFrameInfo.nWidth*pFrameInfo.nHeight*2)
                {
                    if (g_pCalibBuf != IntPtr.Zero)
                    {
                        Marshal.FreeHGlobal(g_pCalibBuf);
                        g_pCalibBuf = IntPtr.Zero;
                        g_nCalibBufSize = 0;
                    }

                    g_pCalibBuf = Marshal.AllocHGlobal((int)(pFrameInfo.nWidth * pFrameInfo.nHeight * 2));
                    if (g_pCalibBuf == IntPtr.Zero)
                    {
                        Console.WriteLine("malloc pCalibBuf failed");
                        return;
                    }
                    g_nCalibBufSize = (uint)(pFrameInfo.nWidth * pFrameInfo.nHeight * 2);
                }

                stLSCCalib.pCalibBuf = g_pCalibBuf;
                stLSCCalib.nCalibBufSize = g_nCalibBufSize;

                stLSCCalib.nSecNumW = 689;
                stLSCCalib.nSecNumH = 249;
                stLSCCalib.nPadCoef = 2;
                stLSCCalib.nCalibMethod = 2;
                stLSCCalib.nTargetGray = 100;

                //同一个相机，在场景、算法参数和图像参数都不变情况下，理论上只需要进行一次标定，可将标定表保存下来。
                //不同相机图片标定出来的标定表不可复用，当场景改变或算法参数改变或图像参数改变时，需要重新标定。
                nRet = device.MV_CC_LSCCalib_NET(ref stLSCCalib);
                if (MyCamera.MV_OK != nRet)
                {
                    Console.WriteLine("LSC Calib failed:{0:x8}", nRet);
                    return;
                }

                //保存标定表到本地
                byte[] LSCCalibData = new byte[stLSCCalib.nCalibBufLen];
                Marshal.Copy(stLSCCalib.pCalibBuf, LSCCalibData, 0, (int)stLSCCalib.nCalibBufLen);
                FileStream pFile = null;
                try
                {
                    pFile = new FileStream("./LSCCalib.bin", FileMode.Create);
                    pFile.Write(LSCCalibData, 0, LSCCalibData.Length);
                }
                catch
                {
                    Console.WriteLine("保存失败");
                }
                finally
                {
                    pFile.Close();
                }

                g_IsNeedCalib = false;
            }

            //LSC校正
            if (g_pDstData == IntPtr.Zero || g_nDstDataSize < pFrameInfo.nFrameLen)
            {
                if (g_pDstData != IntPtr.Zero)
                {
                    Marshal.FreeHGlobal(g_pDstData);
                    g_pDstData = IntPtr.Zero;
                    g_nDstDataSize = 0;
                }

                g_pDstData = Marshal.AllocHGlobal((int)pFrameInfo.nFrameLen);
                if (g_pDstData == IntPtr.Zero)
                {
                    Console.WriteLine("malloc pDstData failed");
                    return;
                }
                g_nDstDataSize = pFrameInfo.nFrameLen;
            }

            MyCamera.MV_CC_LSC_CORRECT_PARAM stLSCCorrectParam = new MyCamera.MV_CC_LSC_CORRECT_PARAM();
            stLSCCorrectParam.nWidth = pFrameInfo.nWidth;
            stLSCCorrectParam.nHeight = pFrameInfo.nHeight;
            stLSCCorrectParam.enPixelType = pFrameInfo.enPixelType;
            stLSCCorrectParam.pSrcBuf = pData;
            stLSCCorrectParam.nSrcBufLen = pFrameInfo.nFrameLen;

            stLSCCorrectParam.pDstBuf = g_pDstData;
            stLSCCorrectParam.nDstBufSize = g_nDstDataSize;

            stLSCCorrectParam.pCalibBuf = g_pCalibBuf;
            stLSCCorrectParam.nCalibBufLen = g_nCalibBufSize;
            nRet = device.MV_CC_LSCCorrect_NET(ref stLSCCorrectParam);
            if (MyCamera.MV_OK != nRet)
            {
                Console.WriteLine("LSC Correct failed:{0:x8}", nRet);
                return;
            }

            if (pFrameInfo.nFrameNum < 10)
            {
                //保存图像到文件
                MyCamera.MV_SAVE_IMG_TO_FILE_PARAM stSaveFileParam = new MyCamera.MV_SAVE_IMG_TO_FILE_PARAM();
                stSaveFileParam.enImageType = MyCamera.MV_SAVE_IAMGE_TYPE.MV_Image_Bmp;
                stSaveFileParam.enPixelType = pFrameInfo.enPixelType;
                stSaveFileParam.nWidth = pFrameInfo.nWidth;
                stSaveFileParam.nHeight = pFrameInfo.nHeight;
                stSaveFileParam.nDataLen = pFrameInfo.nFrameLen;
                stSaveFileParam.pData = pData;
                stSaveFileParam.pImagePath = "BeforeImage_w" + stSaveFileParam.nWidth.ToString() + "_" + stSaveFileParam.nHeight.ToString() + "_fn" + pFrameInfo.nFrameNum.ToString() + ".bmp";
                nRet = device.MV_CC_SaveImageToFile_NET(ref stSaveFileParam);
                if (MyCamera.MV_OK != nRet)
                {
                    Console.WriteLine("SaveImageToFile failed:{0:x8}", nRet);
                    return;
                }

                stSaveFileParam.pData = g_pDstData;
                stSaveFileParam.pImagePath = "AfterImage_w" + stSaveFileParam.nWidth.ToString() + "_" + stSaveFileParam.nHeight.ToString() + "_fn" + pFrameInfo.nFrameNum.ToString() + ".bmp";
                nRet = device.MV_CC_SaveImageToFile_NET(ref stSaveFileParam);
                if (MyCamera.MV_OK != nRet)
                {
                    Console.WriteLine("SaveImageToFile failed:{0:x8}", nRet);
                    return;
                }
            }
        }

        static void Main(string[] args)
        {
            int nRet = MyCamera.MV_OK;

            // ch: 初始化 SDK | en: Initialize SDK
            MyCamera.MV_CC_Initialize_NET();

            do
            {
                // ch:枚举设备 | en:Enum device
                MyCamera.MV_CC_DEVICE_INFO_LIST stDevList = new MyCamera.MV_CC_DEVICE_INFO_LIST();
                nRet = MyCamera.MV_CC_EnumDevices_NET(MyCamera.MV_GIGE_DEVICE | MyCamera.MV_USB_DEVICE, ref stDevList);
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
                nRet = device.MV_CC_SetEnumValue_NET("TriggerMode", 0);
                if (MyCamera.MV_OK != nRet)
                {
                    Console.WriteLine("Set TriggerMode failed:{0:x8}", nRet);
                    break;
                }

                //判断是否可以本地导入
                if (true == File.Exists("./LSCCalib.bin"))
                {
                    FileStream fs = new FileStream("./LSCCalib.bin", FileMode.Open);
                    byte[] data = new byte[fs.Length];

                    if (g_pCalibBuf == IntPtr.Zero || g_nCalibBufSize < fs.Length)
                    {
                        if (g_pCalibBuf != IntPtr.Zero)
                        {
                            Marshal.FreeHGlobal(g_pCalibBuf);
                            g_pCalibBuf = IntPtr.Zero;
                            g_nCalibBufSize = 0;
                        }

                        g_pCalibBuf = Marshal.AllocHGlobal((int)fs.Length);
                        if (g_pCalibBuf == IntPtr.Zero)
                        {
                            Console.WriteLine("malloc pCalibBuf failed");
                            break;
                        }
                        g_nCalibBufSize = (uint)fs.Length;
                    }

                    fs.Read(data, 0, data.Length);
                    fs.Close();

                    Marshal.Copy(data, 0, g_pCalibBuf, (Int32)g_nCalibBufSize);

                    g_IsNeedCalib = false;
                }

                // ch:注册回调函数 | en:Register image callback
                ImageCallback = new MyCamera.cbOutputExdelegate(ImageCallbackFunc);
                nRet = device.MV_CC_RegisterImageCallBackEx_NET(ImageCallback, IntPtr.Zero);
                if (MyCamera.MV_OK != nRet)
                {
                    Console.WriteLine("Register image callback failed!");
                    break;
                }

                // ch:开启抓图 || en: start grab image
                nRet = device.MV_CC_StartGrabbing_NET();
                if (MyCamera.MV_OK != nRet)
                {
                    Console.WriteLine("Start grabbing failed:{0:x8}", nRet);
                    break;
                }

                Console.WriteLine("Press enter to exit");
                Console.ReadLine();

                // ch:停止抓图 | en:Stop grabbing
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

            if (g_pCalibBuf != IntPtr.Zero)
            {
                Marshal.FreeHGlobal(g_pCalibBuf);
                g_pCalibBuf = IntPtr.Zero;
                g_nCalibBufSize = 0;
            }

            if (g_pDstData != IntPtr.Zero)
            {
                Marshal.FreeHGlobal(g_pDstData);
                g_pDstData = IntPtr.Zero;
                g_nDstDataSize = 0;
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
