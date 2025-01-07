using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using MvCamCtrl.NET;
using System.Runtime.InteropServices;
using System.IO;

namespace ConnectSpecCamera
{
    class ConnectSpecCamera
    {
        static void Main(string[] args)
        {
            MyCamera.MV_CC_DEVICE_INFO stDevInfo = new MyCamera.MV_CC_DEVICE_INFO();
            stDevInfo.nTLayerType = MyCamera.MV_GIGE_DEVICE;
            MyCamera.MV_GIGE_DEVICE_INFO_EX stGigEDev= new MyCamera.MV_GIGE_DEVICE_INFO_EX();
            int nRet = MyCamera.MV_OK;
            MyCamera device = new MyCamera();

            // ch: 初始化 SDK | en: Initialize SDK
            MyCamera.MV_CC_Initialize_NET();

            do
            {
                Console.Write("Please input Device Ip : ");
                string strCurrentIp = Convert.ToString(Console.ReadLine());// ch:需要连接的相机ip(根据实际填充) 
                                                        // en:The camera IP that needs to be connected (based on actual padding)
                Console.Write("Please input Net Export Ip : ");
                string strNetExport = Convert.ToString(Console.ReadLine());   // ch:相机对应的网卡ip(根据实际填充) 
                                                        // en:The pc IP that needs to be connected (based on actual padding)
                var parts = strCurrentIp.Split('.');
                try
                {
                    int nIp1 = Convert.ToInt32(parts[0]);
                    int nIp2 = Convert.ToInt32(parts[1]);
                    int nIp3 = Convert.ToInt32(parts[2]);
                    int nIp4 = Convert.ToInt32(parts[3]);
                    stGigEDev.nCurrentIp = (uint)((nIp1 << 24) | (nIp2 << 16) | (nIp3 << 8) | nIp4);

                    parts = strNetExport.Split('.');
                    nIp1 = Convert.ToInt32(parts[0]);
                    nIp2 = Convert.ToInt32(parts[1]);
                    nIp3 = Convert.ToInt32(parts[2]);
                    nIp4 = Convert.ToInt32(parts[3]);
                    stGigEDev.nNetExport = (uint)((nIp1 << 24) | (nIp2 << 16) | (nIp3 << 8) | nIp4);
                }
                catch
                {
                    Console.Write("Invalid Input!\n");
                    break;
                }

                // stGigEDev结构体转为stDevInfo.SpecialInfo.stGigEInfo(Byte[])
                IntPtr stGigeInfoPtr = Marshal.AllocHGlobal(Marshal.SizeOf(stGigEDev));
                Marshal.StructureToPtr(stGigEDev, stGigeInfoPtr, false);
                stDevInfo.SpecialInfo.stGigEInfo = new Byte[Marshal.SizeOf(stDevInfo.SpecialInfo)];
                Marshal.Copy(stGigeInfoPtr, stDevInfo.SpecialInfo.stGigEInfo, 0, Marshal.SizeOf(stDevInfo.SpecialInfo));
                Marshal.Release(stGigeInfoPtr);

                // ch:创建设备 | en: Create device
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

                // ch:开启抓图 || en: start grab image
                nRet = device.MV_CC_StartGrabbing_NET();
                if (MyCamera.MV_OK != nRet)
                {
                    Console.WriteLine("Start grabbing failed:{0:x8}", nRet);
                    break;
                }

                int nCount = 0;
                MyCamera.MV_FRAME_OUT stFrameOut = new MyCamera.MV_FRAME_OUT();
                while (nCount++ != 10)
                {
                    nRet = device.MV_CC_GetImageBuffer_NET(ref stFrameOut, 1000);
                    // ch:获取一帧图像 | en:Get one image
                    if (MyCamera.MV_OK == nRet)
                    {
                        Console.WriteLine("Get Image Buffer: Width[{0}] , Height[{1}] , FrameNum[{2}]"
                            , stFrameOut.stFrameInfo.nWidth, stFrameOut.stFrameInfo.nHeight, stFrameOut.stFrameInfo.nFrameNum);

                        MyCamera.MV_SAVE_IMG_TO_FILE_PARAM stSaveFileParam = new MyCamera.MV_SAVE_IMG_TO_FILE_PARAM();
                        
                        stSaveFileParam.nWidth = stFrameOut.stFrameInfo.nWidth;
                        stSaveFileParam.nHeight = stFrameOut.stFrameInfo.nHeight;
                        stSaveFileParam.enPixelType = stFrameOut.stFrameInfo.enPixelType;
                        stSaveFileParam.pData = stFrameOut.pBufAddr;
                        stSaveFileParam.nDataLen = stFrameOut.stFrameInfo.nFrameLen;

                        stSaveFileParam.enImageType = MyCamera.MV_SAVE_IAMGE_TYPE.MV_Image_Bmp;
                        stSaveFileParam.iMethodValue = 2;
                        stSaveFileParam.pImagePath = "Image_w" + stSaveFileParam.nWidth.ToString() + "_h" + stSaveFileParam.nHeight.ToString() + "_fn" + stFrameOut.stFrameInfo.nFrameNum.ToString() + ".bmp";

                        nRet = device.MV_CC_SaveImageToFile_NET(ref stSaveFileParam);
                        if (MyCamera.MV_OK != nRet)
                        {
                            Console.WriteLine("Save Image failed:{0:x8}", nRet);
                        }
                        device.MV_CC_FreeImageBuffer_NET(ref stFrameOut);
                    }
                    else
                    {
                        Console.WriteLine("Get Image failed:{0:x8}", nRet);
                        break;
                    }
                }

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
