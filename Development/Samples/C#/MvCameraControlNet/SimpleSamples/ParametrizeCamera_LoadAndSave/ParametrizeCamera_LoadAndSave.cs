using System;
using System.Collections.Generic;
using MvCamCtrl.NET;
using System.Runtime.InteropServices;
using System.IO;
using System.Text;

namespace ParametrizeCamera_LoadAndSave
{
    class ParametrizeCamera_LoadAndSave
    {
        static void Main(string[] args)
        {
            MyCamera device = new MyCamera();
            int nRet = MyCamera.MV_OK;

            // ch: 初始化 SDK | en: Initialize SDK
            MyCamera.MV_CC_Initialize_NET();

            do{
                // ch:枚举设备 | en:Enum deivce
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

                Console.WriteLine("Start export the camera properties to the file");
                Console.WriteLine("Wait......");
                // ch:将相机属性导出到文件中
                // en:Export the camera properties to the file
                nRet = device.MV_CC_FeatureSave_NET("CameraFile");
                if(MyCamera.MV_OK != nRet)
                {
                    Console.WriteLine("FeatureSave failed!");
                    break;
                }
                Console.WriteLine("Finish export the camera properties to the file\n");

                Console.WriteLine("Start import the camera properties from the file");
                Console.WriteLine("Wait......");
                // ch:从文件中导入相机属性
                // en:Import the camera properties from the file
                nRet = device.MV_CC_FeatureLoad_NET("CameraFile");
                if (MyCamera.MV_OK != nRet)
                {
                    Console.WriteLine("FeatureLoad failed!");
                    break;
                }
                Console.WriteLine("Finish import the camera properties from the file");

                // ch:关闭设备 | en:Close device
                nRet = device.MV_CC_CloseDevice_NET();
                if (MyCamera.MV_OK != nRet)
                {
                    Console.WriteLine("Close device failed{0:x8}", nRet);
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
