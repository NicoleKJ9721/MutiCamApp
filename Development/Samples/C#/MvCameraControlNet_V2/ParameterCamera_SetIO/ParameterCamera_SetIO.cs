/*
 * 这个示例演示了配置相机常用IO的方法。
 * This sample shows how to configure commonly used IO for cameras.
 */

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using MvCameraControl;

namespace ParameterCamera_SetIO
{
    class ParameterCamera_SetIO
    {
        const DeviceTLayerType devLayerType = DeviceTLayerType.MvGigEDevice | DeviceTLayerType.MvUsbDevice | DeviceTLayerType.MvGenTLCameraLinkDevice
        | DeviceTLayerType.MvGenTLCXPDevice | DeviceTLayerType.MvGenTLXoFDevice;

        static void Main(string[] args)
        {
            int ret = MvError.MV_OK;
            IDevice device = null;

            // ch: 初始化 SDK | en: Initialize SDK
            SDKSystem.Initialize();

            try
            {
                List<IDeviceInfo> devInfoList;

                // ch:枚举设备 | en:Enum device
                ret = DeviceEnumerator.EnumDevices(devLayerType, out devInfoList);
                if (ret != MvError.MV_OK)
                {
                    Console.WriteLine("Enum device failed:{0:x8}", ret);
                    return;
                }

                Console.WriteLine("Enum device count : {0}", devInfoList.Count);

                if (0 == devInfoList.Count)
                {
                    return;
                }

                // ch:打印设备信息 en:Print device info
                int devIndex = 0;
                foreach (var devInfo in devInfoList)
                {
                    Console.WriteLine("[Device {0}]:", devIndex);
                    if (devInfo.TLayerType == DeviceTLayerType.MvGigEDevice || devInfo.TLayerType == DeviceTLayerType.MvVirGigEDevice || devInfo.TLayerType == DeviceTLayerType.MvGenTLGigEDevice)
                    {
                        IGigEDeviceInfo gigeDevInfo = devInfo as IGigEDeviceInfo;
                        uint nIp1 = ((gigeDevInfo.CurrentIp & 0xff000000) >> 24);
                        uint nIp2 = ((gigeDevInfo.CurrentIp & 0x00ff0000) >> 16);
                        uint nIp3 = ((gigeDevInfo.CurrentIp & 0x0000ff00) >> 8);
                        uint nIp4 = (gigeDevInfo.CurrentIp & 0x000000ff);
                        Console.WriteLine("DevIP: {0}.{1}.{2}.{3}", nIp1, nIp2, nIp3, nIp4);
                    }

                    Console.WriteLine("ModelName:" + devInfo.ModelName);
                    Console.WriteLine("SerialNumber:" + devInfo.SerialNumber);
                    Console.WriteLine();
                    devIndex++;
                }

                Console.Write("Please input index(0-{0:d}):", devInfoList.Count - 1);

                devIndex = Convert.ToInt32(Console.ReadLine());
                Console.WriteLine();
                if (devIndex > devInfoList.Count - 1 || devIndex < 0)
                {
                    Console.Write("Input Error!\n");
                    return;
                }

                // ch:创建设备 | en:Create device
                device = DeviceFactory.CreateDevice(devInfoList[devIndex]);

                // ch:打开设备 | en:Open device
                ret = device.Open();
                if (ret != MvError.MV_OK)
                {
                    Console.WriteLine("Open device failed:{0:x8}", ret);
                    return;
                }

                Console.WriteLine("----------LineSelector------------");
                //ch: 获取LineSelector | en: get LineSelector
                IEnumValue lineSelectors;
                ret = device.Parameters.GetEnumValue("LineSelector", out lineSelectors);
                if (ret != MvError.MV_OK)
                {
                    Console.WriteLine("Get LineSelector failed:{0:x8}", ret);                  
                }
                else
                {
                    Console.WriteLine("LineSelector current value:{0}", lineSelectors.CurEnumEntry.Value);
                    Console.WriteLine("Supported lineSelector number:{0}", lineSelectors.SupportedNum);
                    foreach (IEnumEntry selectorValue in lineSelectors.SupportEnumEntries)
                    {
                        Console.WriteLine("Supported lineSelector value:{0}", selectorValue.Value);
                    }
                    Console.WriteLine();
                    //ch: 设置LineSelector | en: set LineSelector
                    Console.Write("Please lineSelector to set:");
                    int lineSelector = Convert.ToInt32(Console.ReadLine());

                    ret = device.Parameters.SetEnumValue("LineSelector", (uint)lineSelector);
                    if (ret != MvError.MV_OK)
                    {
                        Console.WriteLine("Set LineSelector failed:{0:x8}", ret);
                    }
                    else
                    {
                        Console.WriteLine("Set LineSelector OK!");
                    }
                }
                Console.WriteLine();

                Console.WriteLine("----------LineMode------------");
                //ch: 获取LineMode | en: get LineMode
               IEnumValue lineModes;
               ret = device.Parameters.GetEnumValue("LineMode", out lineModes);
               if (ret != MvError.MV_OK)
               {
                   Console.WriteLine("Get LineSelector failed:{0:x8}", ret);
               }
               else
               {
                   Console.WriteLine("LineMode current value:{0}", lineModes.CurEnumEntry.Value);
                   Console.WriteLine("Supported lineMode number:{0}", lineModes.SupportedNum);
                   foreach (IEnumEntry modeValue in lineModes.SupportEnumEntries)
                   {
                       Console.WriteLine("Supported lineMode value:{0}", modeValue.Value);
                   }
                   Console.WriteLine();
                   //ch: 设置LineMode | en: set LineMode
                   //ch: 设置LineSelector | en: set LineSelector
                   Console.Write("Please lineMode to set:");
                   int lineMode = Convert.ToInt32(Console.ReadLine());

                   ret = device.Parameters.SetEnumValue("LineMode", (uint)lineMode);
                   if (ret != MvError.MV_OK)
                   {
                       Console.WriteLine("Set lineMode failed:{0:x8}", ret);
                   }
                   else
                   {
                       Console.WriteLine("Set lineMode OK!");
                   }
               }

               // ch:关闭设备 | en:Close device
               ret = device.Close();
               if (ret != MvError.MV_OK)
               {
                   Console.WriteLine("Close device failed:{0:x8}", ret);
                   return;
               }

               // ch:销毁设备 | en:Destroy device
               device.Dispose();
               device = null;
            }
            catch (Exception e)
            {
                Console.Write("Exception: " + e.Message);
            }
            finally
            {
                // ch:销毁设备 | en:Destroy device
                if (device != null || ret != MvError.MV_OK)
                {
                    device.Dispose();
                    device = null;
                }

                // ch: 反初始化SDK | en: Finalize SDK
                SDKSystem.Finalize();

                Console.WriteLine("Press enter to exit");
                Console.ReadKey();
            }
        }
    }
}
