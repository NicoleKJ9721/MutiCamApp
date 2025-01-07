Imports System.Runtime.InteropServices
Imports System.Threading.Thread
Imports System.Net.IPAddress

Module SavePonitCloudData_3D

    Public Sub Main()
        'Dim GetPixelSize As Sub
        Dim MyCamera As CCamera = New CCamera
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
            If (CCamera.MV_OK <> nRet) Then
                Console.WriteLine("Create device failed:{0:x8}", nRet)
                Exit Do
            End If

            ' ch:打开相机 | en:Open device
            nRet = MyCamera.OpenDevice()
            If (CCamera.MV_OK <> nRet) Then
                Console.WriteLine("Open device failed:{0:x8}", nRet)
                Exit Do
            End If

            ' ch:判断设备是否是设置的3D格式 | en:Judge Whether the device is set to 3D format
            Dim EnumValue As CCamera.MVCC_ENUMVALUE = New CCamera.MVCC_ENUMVALUE()
            nRet = MyCamera.GetEnumValue("PixelFormat", EnumValue)
            If (CCamera.MV_OK <> nRet) Then
                Console.WriteLine("Get the Camera format fail:{0:x8}", nRet)
                Exit Do
            End If

            Dim nPixelFormat As Int32 = EnumValue.nCurValue And &H7FFFFFFF
            If (EnumValue.nCurValue And &H80000000) = 2147483648 Then '2147483648 = &H80000000
                nPixelFormat = nPixelFormat Or &H80000000
            End If

            If (nPixelFormat <> CCamera.MvGvspPixelType.PixelType_Gvsp_Coord3D_ABC32 _
            And nPixelFormat <> CCamera.MvGvspPixelType.PixelType_Gvsp_Coord3D_ABC32f _
            And nPixelFormat <> CCamera.MvGvspPixelType.PixelType_Gvsp_Coord3D_AB32 _
            And nPixelFormat <> CCamera.MvGvspPixelType.PixelType_Gvsp_Coord3D_AB32f _
            And nPixelFormat <> CCamera.MvGvspPixelType.PixelType_Gvsp_Coord3D_AC32 _
            And nPixelFormat <> CCamera.MvGvspPixelType.PixelType_Gvsp_Coord3D_AC32f) Then
                Console.WriteLine("This is not a supported 3D format!")
                nRet = CCamera.MV_E_SUPPORT
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

            ' ch:获取包大小 || en: Get Payload Size
            Dim stParam As CCamera.MVCC_INTVALUE_EX = New CCamera.MVCC_INTVALUE_EX()
            nRet = MyCamera.GetIntValueEx("PayloadSize", stParam)
            If (CCamera.MV_OK <> nRet) Then
                Console.WriteLine("Get PayloadSize failed:{0:x8}", nRet)
                Exit Do
            End If
            Dim nPayloadSize As Int32 = stParam.nCurValue

            ' ch:开启取流 | en:Start grabbing
            nRet = MyCamera.StartGrabbing()
            If 0 <> nRet Then
                Console.WriteLine("Start grabbing failed:{0:x8}", nRet)
                Exit Do
            End If

            '申请足够大的缓存，用于保存获取到的图像
            Dim nImageNum As Int32 = 100
            Dim bSaveImageBuf(nPayloadSize * nImageNum) As Byte
            Dim nSaveImageSize As Int32 = nPayloadSize * nImageNum

            '已获取的总图片大小
            Dim nSaveDataLen As Int32 = 0

            Dim stOutFrame As CCamera.MV_FRAME_OUT = New CCamera.MV_FRAME_OUT

            For i = 0 To nImageNum - 1
                '仅支持3D格式的图像
                nRet = MyCamera.GetImageBuffer(stOutFrame, 1000)
                If CCamera.MV_OK = nRet Then
                    Console.WriteLine("Width:" + Convert.ToString(stOutFrame.stFrameInfo.nWidth) + " Height:" + Convert.ToString(stOutFrame.stFrameInfo.nHeight) + " FrameNum:" + Convert.ToString(stOutFrame.stFrameInfo.nFrameNum))

                    If nSaveImageSize > (nSaveDataLen + stOutFrame.stFrameInfo.nFrameLen) Then
                        Marshal.Copy(stOutFrame.pBufAddr, bSaveImageBuf, nSaveDataLen, stOutFrame.stFrameInfo.nFrameLen)
                        nSaveDataLen += stOutFrame.stFrameInfo.nFrameLen
                    End If

                    nRet = MyCamera.FreeImageBuffer(stOutFrame)
                    If CCamera.MV_OK <> nRet Then
                        Console.WriteLine("Free Image Buffer failed:{0:x8}", nRet)
                    End If
                Else
                    Console.WriteLine("Get Image failed:{0:x8}", nRet)
                End If
            Next

            Dim stSavePoCloudPar As CCamera.MV_SAVE_POINT_CLOUD_PARAM = New CCamera.MV_SAVE_POINT_CLOUD_PARAM

            stSavePoCloudPar.nLinePntNum = stOutFrame.stFrameInfo.nWidth
            stSavePoCloudPar.nLineNum = stOutFrame.stFrameInfo.nHeight * nImageNum

            Dim bDstImageBuf(stSavePoCloudPar.nLineNum * stSavePoCloudPar.nLinePntNum * (16 * 3 + 4) + 2048) As Byte
            Dim nDstImageSize As Int32 = stSavePoCloudPar.nLineNum * stSavePoCloudPar.nLinePntNum * (16 * 3 + 4) + 2048

            stSavePoCloudPar.enPointCloudFileType = CCamera.MV_SAVE_POINT_CLOUD_FILE_TYPE.MV_PointCloudFile_PLY
            stSavePoCloudPar.enSrcPixelType = stOutFrame.stFrameInfo.enPixelType
            stSavePoCloudPar.nSrcDataLen = nSaveDataLen

            Dim hSrcData As GCHandle = GCHandle.Alloc(bSaveImageBuf, GCHandleType.Pinned)
            stSavePoCloudPar.pSrcData = hSrcData.AddrOfPinnedObject()

            stSavePoCloudPar.nDstBufSize = nDstImageSize
            Dim hDstData As GCHandle = GCHandle.Alloc(bDstImageBuf, GCHandleType.Pinned)
            stSavePoCloudPar.pDstBuf = hDstData.AddrOfPinnedObject()

            nRet = MyCamera.SavePointCloudData(stSavePoCloudPar)
            If (CCamera.MV_OK <> nRet) Then
                Console.WriteLine("Save point cloud data failed:{0:x8}", nRet)
                Exit Do
            End If

            Dim bData(stSavePoCloudPar.nDstBufLen) As Byte
            Marshal.Copy(stSavePoCloudPar.pDstBuf, bData, 0, stSavePoCloudPar.nDstBufLen)

            My.Computer.FileSystem.WriteAllBytes("PointCloudData.ply", bData, False)
            Console.WriteLine("Save point cloud data succeed")

            hSrcData.Free()
            hDstData.Free()

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
