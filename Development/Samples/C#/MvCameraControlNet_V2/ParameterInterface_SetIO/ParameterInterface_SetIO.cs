/*
 * 这个示例演示了配置采集卡常用IO的方法。
 * This sample shows how to configure commonly used IO for frame grabber.
 */

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using MvCameraControl;

namespace ParameterInterface_SetIO
{
    class ParameterInterface_SetIO
    {
        private const InterfaceTLayerType IFLayerType = InterfaceTLayerType.MvGigEInterface | InterfaceTLayerType.MvCameraLinkInterface | InterfaceTLayerType.MvCXPInterface
            | InterfaceTLayerType.MvXoFInterface;
        void Run()
        {
            IInterface IFInstance = null;

            try
            {
                List<IInterfaceInfo> IFInfoList;
                int ret = InterfaceEnumerator.EnumInterfaces(IFLayerType, out IFInfoList);
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

                IEnumValue lineSelectorEnum;
                ret = IFInstance.Parameters.GetEnumValue("LineSelector", out lineSelectorEnum);
                if(ret != MvError.MV_OK)
                {
                    Console.WriteLine("Get Line Selector failed:{0:x8}", ret);
                    return;
                }

                Console.WriteLine("Select Line Selector, including:");
                int lineSelectorIndex = -1;

                for (uint i = 0; i < lineSelectorEnum.SupportedNum; i++)
                {

                    Console.WriteLine("{0}. {1}", i, lineSelectorEnum.SupportEnumEntries[i].Symbolic);
                }

               
                try
                {
                    Console.WriteLine("Please LineSelector to set:");
                    lineSelectorIndex = Convert.ToInt32(Console.ReadLine());
                }
                catch
                {
                    Console.WriteLine("Invalid Index!");
                    return;
                }

                if (lineSelectorIndex < 0 || lineSelectorIndex >= lineSelectorEnum.SupportedNum)
                {
                    Console.WriteLine("Error Index!");
                    return;
                }

                // ch:设置输入或输出信号 | en:Set input or output signal
                ret = IFInstance.Parameters.SetEnumValue("LineSelector", lineSelectorEnum.SupportEnumEntries[lineSelectorIndex].Value);
                if (ret != MvError.MV_OK)
                {
                    Console.WriteLine("Set Line Selector failed:{0:x8}", ret);
                    return;
                }
                else
                {
                    Console.WriteLine("Set Line Selector success");
                }

                IEnumValue lineModeEnum;
                ret = IFInstance.Parameters.GetEnumValue("LineMode", out lineModeEnum);
                if (ret != MvError.MV_OK)
                {
                    Console.WriteLine("Get Line Mode failed:{0:x8}", ret);
                    return;
                }
                Console.WriteLine("Select Line Mode, including:");

                int lineModeIndex = -1;
                for (uint i = 0; i < lineModeEnum.SupportedNum; i++)
                {
                    Console.WriteLine("{0}. {1}", i, lineModeEnum.SupportEnumEntries[i].Symbolic);
                }
                try
                {
                    Console.WriteLine("Please lineMode to set:");
                    lineModeIndex = Convert.ToInt32(Console.ReadLine());
                }
                catch
                {
                    Console.WriteLine("Invalid Index!");
                    return;
                }

                if (lineModeIndex < 0 || lineModeIndex >= lineModeEnum.SupportedNum)
                {
                    Console.WriteLine("Error Index!");
                    return;
                }

                ret = IFInstance.Parameters.SetEnumValue("LineMode", lineModeEnum.SupportEnumEntries[lineModeIndex].Value);
                if (ret != MvError.MV_OK)
                {
                    Console.WriteLine("Set Line Mode failed:{0:x8}", ret);
                    return;
                }
                else
                {
                    Console.WriteLine("Set Line Mode success");
                }

                ret = IFInstance.Parameters.SetBoolValue("LineInverter", false);
                if (ret != MvError.MV_OK)
                {
                    Console.WriteLine("Set Line Inverter failed:{0:x8}", ret);
                    return;
                }
                else
                {
                    Console.WriteLine("Set Line Inverter success");
                }

                ret = IFInstance.Parameters.GetEnumValue("LineMode", out lineModeEnum);
                if (ret != MvError.MV_OK)
                {
                    Console.WriteLine("Get Line Mode failed:{0:x8}", ret);
                    return;
                }


                ret = IFInstance.Parameters.GetEnumValue("LineMode", out lineModeEnum);
                if ("Input" == lineModeEnum.CurEnumEntry.Symbolic)
                {
                    int lineDebouncerTimeNs = 100;
                    ret = IFInstance.Parameters.SetIntValue("LineDebouncerTimeNs", lineDebouncerTimeNs);
                    if (ret != MvError.MV_OK)
                    {
                        ret = IFInstance.Parameters.SetIntValue("LineDebouncerTime", lineDebouncerTimeNs);
                        if (ret != MvError.MV_OK)
                        {
                            Console.WriteLine("Set Line Debouncer Time failed:{0:x8}", ret);
                            return;
                        }
                        else
                        {
                            Console.WriteLine("Set Line Debouncer Time:{0}", lineDebouncerTimeNs);
                        }
                    }
                    else
                    {
                        Console.WriteLine("Set Line Debouncer Time:{0}", lineDebouncerTimeNs);
                    }
                }
                else if ("Output" == lineModeEnum.CurEnumEntry.Symbolic)
                {
                    IEnumValue lineSouceEnum;
                    ret = IFInstance.Parameters.GetEnumValue("LineSource", out lineSouceEnum);
                    if (ret != MvError.MV_OK)
                    {
                        Console.WriteLine("Get Line Source failed:{0:x8}", ret);
                        return;
                    }

                    Console.WriteLine("Select Line Source, including.");
                    int lineSourceIndex = -1;
                    for (uint i = 0; i < lineSouceEnum.SupportedNum; i++)
                    {
                        Console.WriteLine("{0}. {1}", i, lineSouceEnum.SupportEnumEntries[i].Symbolic);
                    }

                    try
                    {
                        lineSourceIndex = Convert.ToInt32(Console.ReadLine());
                    }
                    catch
                    {
                        Console.WriteLine("Invalid Index!");
                        return;
                    }

                    if (lineSourceIndex < 0 || lineSourceIndex >= lineSouceEnum.SupportedNum)
                    {
                        Console.WriteLine("Error Index!");
                        return;
                    }

                    ret = IFInstance.Parameters.SetEnumValue("LineSource", lineSelectorEnum.SupportEnumEntries[lineSourceIndex].Value);
                    if (ret != MvError.MV_OK)
                    {
                        Console.WriteLine("Set Line Source failed:{0:x8}", ret);
                        return;
                    }
                    else
                    {
                        Console.WriteLine("Set Line Source success");
                    }
                }

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
            }

        }
        static void Main(string[] args)
        {
            // ch: 初始化 SDK | en: Initialize SDK
            SDKSystem.Initialize();

            ParameterInterface_SetIO program = new ParameterInterface_SetIO();
            program.Run();

            Console.WriteLine("Press enter to exit");
            Console.ReadKey();

            // ch: 反初始化SDK | en: Finalize SDK
            SDKSystem.Finalize();
        }
    }
}
