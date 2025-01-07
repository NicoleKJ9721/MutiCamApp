Imports System.Runtime.InteropServices
Imports System.Threading
Imports System.Net.IPAddress

Module GrabStrategies

    Dim MyCamera As CCamera = New CCamera

    Sub UpcomingThread()
        Thread.Sleep(3000)
        MyCamera.SetCommandValue("TriggerSoftware")
    End Sub

    Sub Main()
        Dim nRet As Int32 = CCamera.MV_OK
        Dim stDeviceInfoList As CCamera.MV_CC_DEVICE_INFO_LIST = New CCamera.MV_CC_DEVICE_INFO_LIST

        Do While (True)
            ' ch:枚举设备 | en:Enum device
            nRet = CCamera.EnumDevices((CCamera.MV_GIGE_DEVICE Or CCamera.MV_USB_DEVICE), stDeviceInfoList)
            If CCamera.MV_OK <> nRet Then
                Console.WriteLine("Enum Device failed:{0:x8}", nRet)
                Exit Do
            End If

            If (0 = stDeviceInfoList.nDeviceNum) Then
                Console.WriteLine("No Find Gige | Usb Device !")
                Exit Do
            End If

            '  ch:打印设备信息 | en:Print device info
            Dim i As Int32
            For i = 0 To stDeviceInfoList.nDeviceNum - 1
                Dim stDeviceInfo As CCamera.MV_CC_DEVICE_INFO = New CCamera.MV_CC_DEVICE_INFO
                stDeviceInfo = CType(Marshal.PtrToStructure(stDeviceInfoList.pDeviceInfo(i), GetType(CCamera.MV_CC_DEVICE_INFO)), CCamera.MV_CC_DEVICE_INFO)
                If (CCamera.MV_GIGE_DEVICE = stDeviceInfo.nTLayerType) Then
                    Dim stGigeInfoPtr As IntPtr = Marshal.AllocHGlobal(216)
                    Marshal.Copy(stDeviceInfo.stSpecialInfo.stGigEInfo, 0, stGigeInfoPtr, 216)
                    Dim stGigeInfo As CCamera.MV_GIGE_DEVICE_INFO
                    stGigeInfo = CType(Marshal.PtrToStructure(stGigeInfoPtr, GetType(CCamera.MV_GIGE_DEVICE_INFO)), CCamera.MV_GIGE_DEVICE_INFO)
                    Dim nIpByte1 As UInt32 = (stGigeInfo.nCurrentIp And &HFF000000) >> 24
                    Dim nIpByte2 As UInt32 = (stGigeInfo.nCurrentIp And &HFF0000) >> 16
                    Dim nIpByte3 As UInt32 = (stGigeInfo.nCurrentIp And &HFF00) >> 8
                    Dim nIpByte4 As UInt32 = (stGigeInfo.nCurrentIp And &HFF)

                    Console.WriteLine("[Dev " + Convert.ToString(i) + "]:")
                    Console.WriteLine("DevIP:" + nIpByte1.ToString() + "." + nIpByte2.ToString() + "." + nIpByte3.ToString() + "." + nIpByte4.ToString())
                    Console.WriteLine("UserDefinedName:" + stGigeInfo.chUserDefinedName)
                    Console.WriteLine("")
                Else
                    Dim stUsbInfoPtr As IntPtr = Marshal.AllocHGlobal(540)
                    Marshal.Copy(stDeviceInfo.stSpecialInfo.stUsb3VInfo, 0, stUsbInfoPtr, 540)
                    Dim stUsbInfo As CCamera.MV_USB3_DEVICE_INFO
                    stUsbInfo = CType(Marshal.PtrToStructure(stUsbInfoPtr, GetType(CCamera.MV_USB3_DEVICE_INFO)), CCamera.MV_USB3_DEVICE_INFO)

                    Console.WriteLine("[Dev " + Convert.ToString(i) + "]:")
                    Console.WriteLine("SerialNumber:" + stUsbInfo.chSerialNumber)
                    Console.WriteLine("UserDefinedName:" + stUsbInfo.chUserDefinedName)
                    Console.WriteLine("")
                End If
            Next

            Console.Write("Please input index(0-{0:d}):", stDeviceInfoList.nDeviceNum - 1)
            Dim nIndex As Int32
            Try
                nIndex = Console.ReadLine()
            Catch ex As Exception
                Console.WriteLine("Invalid input!")
                Console.WriteLine("push enter to exit")
                System.Console.ReadLine()
                End
            End Try

            If nIndex > stDeviceInfoList.nDeviceNum - 1 Then
                Console.WriteLine("Invalid input!")
                Console.WriteLine("push enter to exit")
                System.Console.ReadLine()
                End
            End If

            If nIndex < 0 Then
                Console.WriteLine("Invalid input!")
                Console.WriteLine("push enter to exit")
                System.Console.ReadLine()
                End
            End If

            Dim stdevInfo As CCamera.MV_CC_DEVICE_INFO
            stdevInfo = CType(Marshal.PtrToStructure(stDeviceInfoList.pDeviceInfo(nIndex), GetType(CCamera.MV_CC_DEVICE_INFO)), CCamera.MV_CC_DEVICE_INFO)

            ' ch:创建句柄 | en:Create handle
            nRet = MyCamera.CreateDevice(stdevInfo)
            If 0 <> nRet Then
                Console.WriteLine("Create device failed:{0:x8}", nRet)
                Exit Do
            End If

            ' ch:打开相机 | en:Open device
            nRet = MyCamera.OpenDevice()
            If 0 <> nRet Then
                Console.WriteLine("Open device failed:{0:x8}", nRet)
                Exit Do
            End If

            ' ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera)
            If stdevInfo.nTLayerType = CCamera.MV_GIGE_DEVICE Then
                Dim nPacketSize As Int32
                nPacketSize = MyCamera.GIGE_GetOptimalPacketSize()
                If nPacketSize > 0 Then
                    nRet = MyCamera.SetIntValueEx("GevSCPSPacketSize", nPacketSize)
                    If 0 <> nRet Then
                        Console.WriteLine("Warning: Set Packet Size failed:{0:x8}", nRet)
                    End If
                Else
                    Console.WriteLine("Warning: Get Packet Size failed:{0:x8}", nPacketSize)
                End If
            End If

            ' ch:设置软触发模式 | en:Set Trigger Mode and Set Trigger Source
            nRet = MyCamera.SetEnumValueByString("TriggerMode", "On")
            If 0 <> nRet Then
                Console.WriteLine("Set Trigger Mode failed:{0:x8}", nRet)
                Exit Do
            End If
            nRet = MyCamera.SetEnumValueByString("TriggerSource", "Software")
            If 0 <> nRet Then
                Console.WriteLine("Set Trigger Source failed:{0:x8}", nRet)
                Exit Do
            End If

            Dim nImageNodeNum As UInt32 = 5
            ' ch:设置缓存节点个数 | en:Set number of image node
            nRet = MyCamera.SetImageNodeNum(nImageNodeNum)
            If 0 <> nRet Then
                Console.WriteLine("Set number of image node fail:{0:x8}", nRet)
                Exit Do
            End If

            Console.WriteLine("**************************************************************************")
            Console.WriteLine("* 0.MV_GrabStrategy_OneByOne;       1.MV_GrabStrategy_LatestImagesOnly;  *")
            Console.WriteLine("* 2.MV_GrabStrategy_LatestImages;   3.MV_GrabStrategy_UpcomingImage;     *")
            Console.WriteLine("**************************************************************************")

            Console.Write("Please Intput Grab Strategy:")
            Dim nGrabStrategy As Int32
            Try
                nGrabStrategy = Console.ReadLine()
            Catch ex As Exception
                Console.WriteLine("Invalid input!")
                Exit Do
            End Try

            If nGrabStrategy = CCamera.MV_GRAB_STRATEGY.MV_GrabStrategy_UpcomingImage And CCamera.MV_USB_DEVICE = stdevInfo.nTLayerType Then
                Console.WriteLine("U3V device not support UpcomingImage")
                Exit Do
            End If

            Select Case nGrabStrategy
                Case CCamera.MV_GRAB_STRATEGY.MV_GrabStrategy_OneByOne
                    Console.WriteLine("Grab using the MV_GrabStrategy_OneByOne default strategy")
                    nRet = MyCamera.SetGrabStrategy(CCamera.MV_GRAB_STRATEGY.MV_GrabStrategy_OneByOne)
                    If 0 <> nRet Then
                        Console.WriteLine("Set Grab Strategy fail:{0:x8}", nRet)
                        Exit Do
                    End If

                Case CCamera.MV_GRAB_STRATEGY.MV_GrabStrategy_LatestImagesOnly
                    Console.WriteLine("Grab using strategy MV_GrabStrategy_LatestImagesOnly")
                    nRet = MyCamera.SetGrabStrategy(CCamera.MV_GRAB_STRATEGY.MV_GrabStrategy_LatestImagesOnly)
                    If 0 <> nRet Then
                        Console.WriteLine("Set Grab Strategy fail:{0:x8}", nRet)
                        Exit Do
                    End If

                Case CCamera.MV_GRAB_STRATEGY.MV_GrabStrategy_LatestImages
                    Console.WriteLine("Grab using strategy MV_GrabStrategy_LatestImages")
                    nRet = MyCamera.SetGrabStrategy(CCamera.MV_GRAB_STRATEGY.MV_GrabStrategy_LatestImages)
                    If 0 <> nRet Then
                        Console.WriteLine("Set Grab Strategy fail:{0:x8}", nRet)
                        Exit Do
                    End If

                    ' ch:设置输出缓存个数 | en:Set Output Queue Size
                    nRet = MyCamera.SetOutputQueueSize(2)
                    If 0 <> nRet Then
                        Console.WriteLine("Set Grab Strategy fail:{0:x8}", nRet)
                        Exit Do
                    End If

                Case CCamera.MV_GRAB_STRATEGY.MV_GrabStrategy_UpcomingImage
                    Console.WriteLine("Grab using strategy MV_GrabStrategy_UpcomingImage")
                    nRet = MyCamera.SetGrabStrategy(CCamera.MV_GRAB_STRATEGY.MV_GrabStrategy_UpcomingImage)
                    If 0 <> nRet Then
                        Console.WriteLine("Set Grab Strategy fail:{0:x8}", nRet)
                        Exit Do
                    End If

                    Dim hUpcomingThread As New System.Threading.Thread(AddressOf UpcomingThread)
                    hUpcomingThread.Start()

                Case Else
                    Console.WriteLine("Input error!Use default strategy:MV_GrabStrategy_OneByOne")
            End Select

            ' ch:开启抓图 | en:start grab image
            nRet = MyCamera.StartGrabbing()
            If 0 <> nRet Then
                Console.WriteLine("Start grabbing failed:{0:x8}", nRet)
                Exit Do
            End If

            For i = 0 To nImageNodeNum - 1
                nRet = MyCamera.SetCommandValue("TriggerSoftware")
                If 0 <> nRet Then
                    Console.WriteLine("Send Trigger Software command fail:{0:x8}", nRet)
                    Exit Do
                End If
                '如果帧率过小或TriggerDelay很大，可能会出现软触发命令没有全部起效而导致取不到数据的情况
                Threading.Thread.Sleep(500)
            Next

            Dim stOutFrame As CCamera.MV_FRAME_OUT = New CCamera.MV_FRAME_OUT
            If nGrabStrategy <> CCamera.MV_GRAB_STRATEGY.MV_GrabStrategy_UpcomingImage Then
                While (True)
                    nRet = MyCamera.GetImageBuffer(stOutFrame, 0)
                    If 0 = nRet Then
                        Console.WriteLine("Get Image Buffer:" + "Width[" + Convert.ToString(stOutFrame.stFrameInfo.nWidth) + "] , Height[" + Convert.ToString(stOutFrame.stFrameInfo.nHeight) + "] , FrameNum[" + Convert.ToString(stOutFrame.stFrameInfo.nFrameNum) + "]")
                    Else
                        Console.WriteLine("Get Image failed:{0:x8}", nRet)
                        Exit While
                    End If

                    nRet = MyCamera.FreeImageBuffer(stOutFrame)
                    If 0 <> nRet Then
                        Console.WriteLine("Free Image Buffer fail:{0:x8}", nRet)
                    End If
                End While
            Else
                nRet = MyCamera.GetImageBuffer(stOutFrame, 5000)
                If 0 = nRet Then
                    Console.WriteLine("Get Image Buffer:" + "Width[" + Convert.ToString(stOutFrame.stFrameInfo.nWidth) + "] , Height[" + Convert.ToString(stOutFrame.stFrameInfo.nHeight) + "] , FrameNum[" + Convert.ToString(stOutFrame.stFrameInfo.nFrameNum) + "]")

                    nRet = MyCamera.FreeImageBuffer(stOutFrame)
                    If 0 <> nRet Then
                        Console.WriteLine("Free Image Buffer fail:{0:x8}", nRet)
                    End If
                Else
                    Console.WriteLine("Get Image failed:{0:x8}", nRet)
                End If
            End If

            ' ch:停止抓图 | en:Stop grab image
            nRet = MyCamera.StopGrabbing()
            If 0 <> nRet Then
                Console.WriteLine("Stop grabbing failed:{0:x8}", nRet)
                Exit Do
            End If

            ' ch:关闭相机 | en:Close device
            nRet = MyCamera.CloseDevice()
            If 0 <> nRet Then
                Console.WriteLine("Close device failed:{0:x8}", nRet)
                Exit Do
            End If

            ' ch:销毁句柄 | en:Destroy handle
            nRet = MyCamera.DestroyDevice()
            If 0 <> nRet Then
                Console.WriteLine("Destroy device failed:{0:x8}", nRet)
            End If

            Exit Do
        Loop

        If 0 <> nRet Then
            ' ch:销毁句柄 | en:Destroy handle
            nRet = MyCamera.DestroyDevice()
            If 0 <> nRet Then
                Console.WriteLine("Destroy device failed:{0:x8}", nRet)
            End If
        End If

        Console.WriteLine("Press enter to exit")
        System.Console.ReadLine()
    End Sub

End Module
