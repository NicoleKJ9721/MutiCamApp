/*
 * 这个示例演示如何接收采集卡事件。
 * This program shows how to receive frame grabber's events.
 */

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using MvCameraControl;

namespace Events_Interface
{
    class Events_Interface
    {
        private const InterfaceTLayerType IFLayerType = InterfaceTLayerType.MvGigEInterface | InterfaceTLayerType.MvCameraLinkInterface | InterfaceTLayerType.MvCXPInterface
            | InterfaceTLayerType.MvXoFInterface;

        static void DeviceEventGrabedHandler(object sender, DeviceEventArgs e)
        {
            Console.WriteLine("EventName[{0}], EventID[{1}]", e.EventInfo.EventName, e.EventInfo.EventID);
        }

        static void Main(string[] args)
        {
            int ret = MvError.MV_OK;
            IInterface IFInstance = null;

            SDKSystem.Initialize();

            

            try
            {
                List<IInterfaceInfo> IFInfoList;
                ret = InterfaceEnumerator.EnumInterfaces(IFLayerType, out IFInfoList);
                if (ret != MvError.MV_OK)
                {
                    Console.WriteLine("Enum interface failed:{0:x8}", ret);
                    return;
                }

                if (0 == IFInfoList.Count)
                {
                    Console.WriteLine("No interface found");
                    return;
                }

                // ch:显示采集卡信息 | en:Show interface info
                int IFIndex = 0;
                foreach (var ifInfo in IFInfoList)
                {
                    Console.WriteLine("[Interface {0}]: ", IFIndex);
                    Console.WriteLine("TLayerType: " + ifInfo.TLayerType.ToString());
                    Console.WriteLine("DisplayName: " + ifInfo.DisplayName);
                    Console.WriteLine("InterfaceID: " + ifInfo.InterfaceID);
                    Console.WriteLine("SerialNumber: " + ifInfo.SerialNumber);
                    IFIndex++;
                }

                // ch:选择采集卡 | en:Select interface
                Console.Write("Please input index(0-{0:d}):", IFInfoList.Count - 1);
                try
                {
                    IFIndex = Convert.ToInt32(Console.ReadLine());
                }
                catch
                {
                    Console.WriteLine("Invalid Index!");
                    return;
                }

                if (IFIndex < 0 || IFIndex >= IFInfoList.Count)
                {
                    Console.WriteLine("Error Index!");
                    return;
                }

                IFInstance = InterfaceFactory.CreateInterface(IFInfoList[IFIndex]);

                // ch:打开采集卡 | en:Open interface
                ret = IFInstance.Open();
                if (ret != MvError.MV_OK)
                {
                    Console.WriteLine("Open Interface failed:{0:x8}", ret);
                    return;
                }

               ret = IFInstance.Parameters.SetEnumValueByString("EventCategory", "SoftEvent");
               if (ret != MvError.MV_OK)
               {
                   Console.WriteLine("Set EventCategory failed:{0:x8}", ret);
                   return;
               }

               ret = IFInstance.Parameters.SetEnumValueByString("ChannelSelector", "Channel0");
               if (ret != MvError.MV_OK)
               {
                   Console.WriteLine("Set ChannelSelector failed:{0:x8}", ret);
                   return;
               }
            
               ret = IFInstance.Parameters.SetEnumValueByString("EventNotification", "On");
               if (ret != MvError.MV_OK)
               {
                   Console.WriteLine("Set EventNotification failed:{0:x8}", ret);
                   return;
               }

                // ch:注册回调函数 | en:Register Event callback
               IFInstance.EventGrabber.DeviceEvent += DeviceEventGrabedHandler;
               IFInstance.EventGrabber.SubscribeEvent("CardPacketReceived0");

               ret = IFInstance.Parameters.SetEnumValueByString("StreamSelector", "Stream0");
               if (ret != MvError.MV_OK)
               {
                   Console.WriteLine("Set EventNotification failed:{0:x8}", ret);
                   return;
               }

               Console.WriteLine("Press enter to exit");
               Console.ReadLine();

               IFInstance.Close();
            }
            catch (Exception e)
            {
                Console.WriteLine("Exception: " + e.Message);
            }
            finally
            {
                //ch： 释放采集卡资源  | en: Release the resources of interface
                if (IFInstance != null)
                {
                    IFInstance.Dispose();
                }
                // ch: 反初始化SDK | en: Finalize SDK
                SDKSystem.Finalize();

                Console.WriteLine("Press enter to exit");
                Console.ReadKey();
            }
        }
    }
}
