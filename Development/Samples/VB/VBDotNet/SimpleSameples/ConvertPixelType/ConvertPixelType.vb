Imports System.Runtime.InteropServices
Imports System.Threading.Thread
Imports System.Net.IPAddress

Module ConvertPixelType

    Function IsMonoPixelFormat(ByVal enType As CCamera.MvGvspPixelType)
        Select Case enType
            Case CCamera.MvGvspPixelType.PixelType_Gvsp_Mono10
                Return True
            Case CCamera.MvGvspPixelType.PixelType_Gvsp_Mono10_Packed
                Return True
            Case CCamera.MvGvspPixelType.PixelType_Gvsp_Mono12
                Return True
            Case CCamera.MvGvspPixelType.PixelType_Gvsp_Mono12_Packed
                Return True
        End Select
        Return False
    End Function

    Function IsColorPixelFormat(ByVal enType As CCamera.MvGvspPixelType)
        Select Case enType
            Case CCamera.MvGvspPixelType.PixelType_Gvsp_BGR8_Packed
                Return True
            Case CCamera.MvGvspPixelType.PixelType_Gvsp_YUV422_Packed
                Return True
            Case CCamera.MvGvspPixelType.PixelType_Gvsp_YUV422_YUYV_Packed
                Return True
            Case CCamera.MvGvspPixelType.PixelType_Gvsp_BayerGR8
                Return True
            Case CCamera.MvGvspPixelType.PixelType_Gvsp_BayerRG8
                Return True
            Case CCamera.MvGvspPixelType.PixelType_Gvsp_BayerGB8
                Return True
            Case CCamera.MvGvspPixelType.PixelType_Gvsp_BayerBG8
                Return True
            Case CCamera.MvGvspPixelType.PixelType_Gvsp_BayerGB10
                Return True
            Case CCamera.MvGvspPixelType.PixelType_Gvsp_BayerGB10_Packed
                Return True
            Case CCamera.MvGvspPixelType.PixelType_Gvsp_BayerBG10
                Return True
            Case CCamera.MvGvspPixelType.PixelType_Gvsp_BayerBG10_Packed
                Return True
            Case CCamera.MvGvspPixelType.PixelType_Gvsp_BayerRG10
                Return True
            Case CCamera.MvGvspPixelType.PixelType_Gvsp_BayerRG10_Packed
                Return True
            Case CCamera.MvGvspPixelType.PixelType_Gvsp_BayerGR10
                Return True
            Case CCamera.MvGvspPixelType.PixelType_Gvsp_BayerGR10_Packed
                Return True
            Case CCamera.MvGvspPixelType.PixelType_Gvsp_BayerGB12
                Return True
            Case CCamera.MvGvspPixelType.PixelType_Gvsp_BayerGB12_Packed
                Return True
            Case CCamera.MvGvspPixelType.PixelType_Gvsp_BayerBG12
                Return True
            Case CCamera.MvGvspPixelType.PixelType_Gvsp_BayerBG12_Packed
                Return True
            Case CCamera.MvGvspPixelType.PixelType_Gvsp_BayerRG12
                Return True
            Case CCamera.MvGvspPixelType.PixelType_Gvsp_BayerRG12_Packed
                Return True
            Case CCamera.MvGvspPixelType.PixelType_Gvsp_BayerGR12
                Return True
            Case CCamera.MvGvspPixelType.PixelType_Gvsp_BayerGR12_Packed
                Return True
        End Select
        Return False
    End Function

    Sub Main()
        Dim MyCamera As CCamera = New CCamera
        Dim nRet As Int32 = CCamera.MV_OK
        Dim pBufForConvert As IntPtr = IntPtr.Zero

        Do While (True)
            Dim stDeviceInfoList As CCamera.MV_CC_DEVICE_INFO_LIST = New CCamera.MV_CC_DEVICE_INFO_LIST

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

            ' ch:开启取流 | en:Start grabbing
            nRet = MyCamera.StartGrabbing()
            If 0 <> nRet Then
                Console.WriteLine("Start grabbing failed:{0:x8}", nRet)
                Exit Do
            End If

            Dim stFrameOut As CCamera.MV_FRAME_OUT = New CCamera.MV_FRAME_OUT

            ' ch:抓取图像 | en:Get image
            nRet = MyCamera.GetImageBuffer(stFrameOut, 1000)
            If CCamera.MV_OK = nRet Then
                Console.WriteLine("Width:" + Convert.ToString(stFrameOut.stFrameInfo.nWidth) + " Height:" + Convert.ToString(stFrameOut.stFrameInfo.nHeight) + " FrameNum:" + Convert.ToString(stFrameOut.stFrameInfo.nFrameNum))

                Dim enType As CCamera.MvGvspPixelType = CCamera.MvGvspPixelType.PixelType_Gvsp_Undefined
                Dim nChannelNum As Int32 = 0
                If IsColorPixelFormat(stFrameOut.stFrameInfo.enPixelType) = True Then
                    enType = CCamera.MvGvspPixelType.PixelType_Gvsp_RGB8_Packed
                    nChannelNum = 3
                ElseIf IsMonoPixelFormat(stFrameOut.stFrameInfo.enPixelType) = True Then
                    enType = CCamera.MvGvspPixelType.PixelType_Gvsp_Mono8
                    nChannelNum = 1
                Else
                    Console.WriteLine("Don't need to convert!")
                End If

                Dim nWidth As Int32 = stFrameOut.stFrameInfo.nWidth
                Dim nHeight As Int32 = stFrameOut.stFrameInfo.nHeight

                If CCamera.MvGvspPixelType.PixelType_Gvsp_Undefined <> enType Then
                    If pBufForConvert = IntPtr.Zero Then
                        
                        Dim nBufForConvertSzie As Integer = nWidth * nHeight * nChannelNum
                        pBufForConvert = Marshal.AllocHGlobal(nBufForConvertSzie)
                    End If
                    Dim stConvertParam As CCamera.MV_CC_PIXEL_CONVERT_PARAM = New CCamera.MV_CC_PIXEL_CONVERT_PARAM()
                    stConvertParam.nHeight = stFrameOut.stFrameInfo.nHeight
                    stConvertParam.nWidth = stFrameOut.stFrameInfo.nWidth
                    stConvertParam.pSrcData = stFrameOut.pBufAddr
                    stConvertParam.nSrcDataLen = stFrameOut.stFrameInfo.nFrameLen
                    stConvertParam.enSrcPixelType = stFrameOut.stFrameInfo.enPixelType
                    stConvertParam.enDstPixelType = enType
                    stConvertParam.pDstBuffer = pBufForConvert
                    stConvertParam.nDstBufferSize = nWidth * nHeight * nChannelNum

                    nRet = MyCamera.ConvertPixelType(stConvertParam)
                    If CCamera.MV_OK <> nRet Then
                        Console.WriteLine("Convert failed:{0:x8}", nRet)
                        Exit Do
                    End If

                    ' ch:将图像数据保存到本地文件 | en:Save image data to local file
                    Dim m_byteImageBuffer(stConvertParam.nDstLen - 1) As Byte
                    Marshal.Copy(pBufForConvert, m_byteImageBuffer, 0, stConvertParam.nDstLen)
                    Try
                        My.Computer.FileSystem.WriteAllBytes("AfterConvert.raw", m_byteImageBuffer, False)
                    Catch ex As Exception
                    End Try
                End If
                MyCamera.FreeImageBuffer(stFrameOut)
            Else
                Console.WriteLine("Get Image failed:{0:x8}", nRet)
                Exit Do
            End If

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
                Exit Do
            End If

            Exit Do
        Loop

        If IntPtr.Zero <> pBufForConvert Then
            Marshal.FreeHGlobal(pBufForConvert)
        End If

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
