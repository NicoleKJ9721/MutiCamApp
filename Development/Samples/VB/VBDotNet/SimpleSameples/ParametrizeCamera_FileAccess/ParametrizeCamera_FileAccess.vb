Imports System.Runtime.InteropServices
Imports System.Threading
Imports System.Net.IPAddress

Module ParametrizeCamera_FileAccess

    Dim MyCamera As CCamera = New CCamera
    Dim g_nRet As Int32 = CCamera.MV_OK
    Dim g_nMode As UInt32 = 0

    Sub FileAccessProgress()
        Dim nRet As Int32 = CCamera.MV_OK
        Dim stFileAccessProgress As CCamera.MV_CC_FILE_ACCESS_PROGRESS = New CCamera.MV_CC_FILE_ACCESS_PROGRESS()

        While (True)
            'ch:获取文件存取进度 |en:Get progress of file access
            nRet = MyCamera.GetFileAccessProgress(stFileAccessProgress)
            Console.WriteLine("State = {0:x8},Completed = {1},Total = {2}", nRet, stFileAccessProgress.nCompleted, stFileAccessProgress.nTotal)
            If (nRet <> CCamera.MV_OK Or (stFileAccessProgress.nCompleted <> 0 And stFileAccessProgress.nCompleted = stFileAccessProgress.nTotal)) Then
                Return
            End If

            Thread.Sleep(50)
        End While
    End Sub

    Sub FileAccessThread()
        Dim stFileAccess As CCamera.MV_CC_FILE_ACCESS = New CCamera.MV_CC_FILE_ACCESS()

        stFileAccess.pUserFileName = "UserSet1.bin"
        stFileAccess.pDevFileName = "UserSet1"
        If 1 = g_nMode Then
            'ch:读模式 |en:Read mode
            g_nRet = MyCamera.FileAccessRead(stFileAccess)
            If (CCamera.MV_OK <> g_nRet) Then
                Console.WriteLine("File Access Read failed:{0:x8}", g_nRet)
            End If
        End If

        If 2 = g_nMode Then
            'ch:写模式 |en:Write mode
            g_nRet = MyCamera.FileAccessWrite(stFileAccess)
            If (CCamera.MV_OK <> g_nRet) Then
                Console.WriteLine("File Access Write failed:{0:x8}", g_nRet)
            End If
        End If
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

            'ch:读模式 |en:Read mode
            Console.WriteLine("Read to file")
            g_nMode = 1

            Dim hReadHandle As New System.Threading.Thread(AddressOf FileAccessThread)
            hReadHandle.Start()

            Thread.Sleep(5)

            Dim hReadProgressHandle As New System.Threading.Thread(AddressOf FileAccessProgress)
            hReadProgressHandle.Start()

            hReadProgressHandle.Join()
            hReadHandle.Join()
            If 0 = g_nRet Then
                Console.WriteLine("File Access Read Success")
            End If

            Console.WriteLine("")

            'ch:写模式 |en:Write mode
            Console.WriteLine("Write to file")
            g_nMode = 2

            Dim hWriteHandle As New System.Threading.Thread(AddressOf FileAccessThread)
            hWriteHandle.Start()

            Thread.Sleep(5)

            Dim hWriteProgressHandle As New System.Threading.Thread(AddressOf FileAccessProgress)
            hWriteProgressHandle.Start()

            hWriteProgressHandle.Join()
            hWriteHandle.Join()
            If 0 = g_nRet Then
                Console.WriteLine("File Access Write Success")
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
