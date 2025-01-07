Imports System.Runtime.InteropServices
Imports System.Threading.Thread
Imports System.Net.IPAddress

Module MultiCast

    Sub Main()
        Dim MyCamera As CCamera = New CCamera
        Dim nRet As Int32 = CCamera.MV_OK
        Dim stDeviceInfoList As CCamera.MV_CC_DEVICE_INFO_LIST = New CCamera.MV_CC_DEVICE_INFO_LIST

        Do While (True)
            ' ch:枚举设备 | en:Enum device
            nRet = CCamera.EnumDevices((CCamera.MV_GIGE_DEVICE), stDeviceInfoList)
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

            '  ch:创建句柄 | en:Create handle
            nRet = MyCamera.CreateDevice(stdevInfo)
            If 0 <> nRet Then
                Console.WriteLine("Create device failed:{0:x8}", nRet)
                Exit Do
            End If

            ' ch:查询用户使用的模式
            ' Query the user for the mode to use.
            Dim monitorMode As Boolean = False
            Dim key As String

            ' ch:询问用户启动多播控制应用程序或多播监控应用程序
            ' Ask the user to launch: the multicast controlling application or the multicast monitoring application.
            Console.WriteLine("Start multicast sample in (c)ontrol or in (m)onitor mode? (c/m)")
            key = Convert.ToString(Console.ReadLine())
            While ((key <> "c") And (key <> "m") And (key <> "C") And (key <> "M"))
            End While
            monitorMode = (key = "m") Or (key = "M")


            ' ch:打开相机 | en:Open device
            If (monitorMode) Then
                nRet = MyCamera.OpenDevice(CCamera.MV_ACCESS_Monitor, 0)
                If (CCamera.MV_OK <> nRet) Then
                    Console.WriteLine("Open device failed:{0:x8}", nRet)
                    Exit Do
                End If
            Else
                nRet = MyCamera.OpenDevice(CCamera.MV_ACCESS_Control, 0)
                If (CCamera.MV_OK <> nRet) Then
                    Console.WriteLine("Open device failed:{0:x8}", nRet)
                    Exit Do
                End If
            End If

            ' ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera)
            If stdevInfo.nTLayerType = CCamera.MV_GIGE_DEVICE And monitorMode = False Then
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

            ' 指定组播ip
            Dim strIp As String = "239.0.1.23"
            Dim nIp1 As UInt32 = Convert.ToUInt32(Split(strIp, ".")(0))
            Dim nIp2 As UInt32 = Convert.ToUInt32(Split(strIp, ".")(1))
            Dim nIp3 As UInt32 = Convert.ToUInt32(Split(strIp, ".")(2))
            Dim nIp4 As UInt32 = Convert.ToUInt32(Split(strIp, ".")(3))
            Dim nIp As UInt32 = ((nIp1 << 24) Or (nIp2 << 16) Or (nIp3 << 8) Or nIp4)

            ' ch:可指定端口号作为组播组端口 | en:multicast port
            Dim stTransmissionType As CCamera.MV_TRANSMISSION_TYPE = New CCamera.MV_TRANSMISSION_TYPE()
            stTransmissionType.enTransmissionType = CCamera.MV_GIGE_TRANSMISSION_TYPE.MV_GIGE_TRANSTYPE_MULTICAST
            stTransmissionType.nDestIp = nIp
            stTransmissionType.nDestPort = 8787

            nRet = MyCamera.GIGE_SetTransmissionType(stTransmissionType)
            If (CCamera.MV_OK <> nRet) Then
                Console.WriteLine("MV_GIGE_SetTransmissionType fail! nRet [%x]\n", nRet)
                Exit Do
            End If

            ' ch:开启取流 | en:Start grabbing
            nRet = MyCamera.StartGrabbing()
            If 0 <> nRet Then
                Console.WriteLine("Start grabbing failed:{0:x8}", nRet)
            End If

            ' ch:抓取图像 | en:Get image
            Dim nCount As Int32 = 0
            Dim stFrameOut As CCamera.MV_FRAME_OUT = New CCamera.MV_FRAME_OUT

            Do While nCount <> 10
                nCount = nCount + 1
                nRet = MyCamera.GetImageBuffer(stFrameOut, 1000)
                If CCamera.MV_OK = nRet Then
                    Console.WriteLine("Width:" + Convert.ToString(stFrameOut.stFrameInfo.nWidth) + " Height:" + Convert.ToString(stFrameOut.stFrameInfo.nHeight) + " FrameNum:" + Convert.ToString(stFrameOut.stFrameInfo.nFrameNum))

                    Dim stSaveParam As CCamera.MV_SAVE_IMG_TO_FILE_PARAM = New CCamera.MV_SAVE_IMG_TO_FILE_PARAM()
                    stSaveParam.enImageType = CCamera.MV_SAVE_IAMGE_TYPE.MV_Image_Bmp
                    stSaveParam.enPixelType = stFrameOut.stFrameInfo.enPixelType
                    stSaveParam.pData = stFrameOut.pBufAddr
                    stSaveParam.nDataLen = stFrameOut.stFrameInfo.nFrameLen
                    stSaveParam.nHeight = stFrameOut.stFrameInfo.nHeight
                    stSaveParam.nWidth = stFrameOut.stFrameInfo.nWidth
                    stSaveParam.iMethodValue = 2
                    stSaveParam.pImagePath = "Image_w" + stSaveParam.nWidth.ToString() + "_h" + stSaveParam.nHeight.ToString() + "_fn" + stFrameOut.stFrameInfo.nFrameNum.ToString() + ".bmp"
                    nRet = MyCamera.SaveImageToFile(stSaveParam)
                    If CCamera.MV_OK <> nRet Then
                        Console.WriteLine("Save Image failed:{0:x8}", nRet)
                    End If
                    MyCamera.FreeImageBuffer(stFrameOut)
                    Continue Do
                Else
                    Console.WriteLine("Get Image failed:{0:x8}", nRet)
                End If
            Loop

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
