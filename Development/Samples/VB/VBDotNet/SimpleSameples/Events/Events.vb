Imports System.Runtime.InteropServices
Imports System.Threading.Thread
Imports System.Net.IPAddress

Module Events
    Dim MyCamera As CCamera = New CCamera

    Private Sub cbEventdelegateFunc(ByRef pEventInfo As CCamera.MV_EVENT_OUT_INFO, ByVal pUser As IntPtr)
        Dim Info As String
        Info = "EventName[" & pEventInfo.EventName & "] , EventID[" & Convert.ToString(pEventInfo.nEventID) & "]"
        Console.WriteLine(Info)
    End Sub

    Sub Main()
        Dim nRet As Int32 = CCamera.MV_OK
        Dim stDeviceInfoList As CCamera.MV_CC_DEVICE_INFO_LIST = New CCamera.MV_CC_DEVICE_INFO_LIST
        Dim cbCallback As CCamera.cbEventdelegateEx = New CCamera.cbEventdelegateEx(AddressOf cbEventdelegateFunc)

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

            ' ch:打印设备信息 | en:Print device info
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

                    Console.WriteLine("[Dev " & Convert.ToString(i) & "]:")
                    Console.WriteLine("DevIP:" & nIpByte1.ToString() & "." & nIpByte2.ToString() & "." & nIpByte3.ToString() & "." & nIpByte4.ToString())
                    Console.WriteLine("UserDefinedName:" & stGigeInfo.chUserDefinedName)
                    Console.WriteLine("")
                Else
                    Dim stUsbInfoPtr As IntPtr = Marshal.AllocHGlobal(540)
                    Marshal.Copy(stDeviceInfo.stSpecialInfo.stUsb3VInfo, 0, stUsbInfoPtr, 540)
                    Dim stUsbInfo As CCamera.MV_USB3_DEVICE_INFO
                    stUsbInfo = CType(Marshal.PtrToStructure(stUsbInfoPtr, GetType(CCamera.MV_USB3_DEVICE_INFO)), CCamera.MV_USB3_DEVICE_INFO)

                    Console.WriteLine("[Dev " & Convert.ToString(i) & "]:")
                    Console.WriteLine("SerialNumber:" & stUsbInfo.chSerialNumber)
                    Console.WriteLine("UserDefinedName:" & stUsbInfo.chUserDefinedName)
                    Console.WriteLine("")
                End If
            Next

            Console.Write("Please input index(0-{0:d}):", stDeviceInfoList.nDeviceNum - 1)
            Dim nIndex As Int32
            nIndex = Console.ReadLine()

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

            nRet = MyCamera.SetEnumValueByString("EventSelector", "ExposureEnd")
            If 0 <> nRet Then
                Console.WriteLine("Set Event Selector failed:{0:x8}", nRet)
                Exit Do
            End If

            nRet = MyCamera.SetEnumValueByString("EventNotification", "On")
            If 0 <> nRet Then
                Console.WriteLine("Set Event Notification failed:{0:x8}", nRet)
                Exit Do
            End If

            ' ch:注册抓图回调 | en:Register image callback
            nRet = MyCamera.RegisterEventCallBackEx("ExposureEnd", cbCallback, 0)
            If CCamera.MV_OK <> nRet Then
                Console.WriteLine("Register image callback failed:{0:x8}", nRet)
                Exit Do
            End If

            ' ch:开启取流 | en:Start grabbing
            nRet = MyCamera.StartGrabbing()
            If 0 <> nRet Then
                Console.WriteLine("Start grabbing failed:{0:x8}", nRet)
                Exit Do
            End If

            Console.WriteLine("push enter to exit")
            System.Console.ReadLine()

            ' ch:停止取流 | en:Stop grabbing
            nRet = MyCamera.StopGrabbing()
            If 0 <> nRet Then
                Console.WriteLine("Stop Grabbing failed:{0:x8}", nRet)
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
