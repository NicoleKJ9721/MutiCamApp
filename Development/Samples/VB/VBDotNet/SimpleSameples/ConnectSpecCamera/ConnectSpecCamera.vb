Imports System.Runtime.InteropServices
Imports System.Threading.Thread
Imports System.Net.IPAddress

Module ConnectSpecCamera
    Sub Main()
        Dim MyCamera As CCamera = New CCamera
        Dim nRet As Int32 = CCamera.MV_OK

        Dim stDevInfo As CCamera.MV_CC_DEVICE_INFO = New CCamera.MV_CC_DEVICE_INFO
        stDevInfo.nTLayerType = CCamera.MV_GIGE_DEVICE
        Dim stGigEDev As CCamera.MV_GIGE_DEVICE_INFO = New CCamera.MV_GIGE_DEVICE_INFO

        ' ch:需要连接的相机ip(根据实际填充) | en:The camera IP that needs to be connected (based on actual padding)
        Console.WriteLine("Please input Device Ip : ")
        Dim strCurrentIp As String
        strCurrentIp = Console.ReadLine()

        ' ch:相机对应的网卡ip(根据实际填充) | en:The pc IP that needs to be connected (based on actual padding)
        Console.WriteLine("Please Net Export Ip : ")
        Dim strNetExport As String
        strNetExport = Console.ReadLine()
        Try
            Dim nIp1 As UInt32 = Convert.ToUInt32(Split(strCurrentIp, ".")(0))
            Dim nIp2 As UInt32 = Convert.ToUInt32(Split(strCurrentIp, ".")(1))
            Dim nIp3 As UInt32 = Convert.ToUInt32(Split(strCurrentIp, ".")(2))
            Dim nIp4 As UInt32 = Convert.ToUInt32(Split(strCurrentIp, ".")(3))
            stGigEDev.nCurrentIp = ((nIp1 << 24) Or (nIp2 << 16) Or (nIp3 << 8) Or nIp4)

            nIp1 = Convert.ToInt32(Split(strNetExport, ".")(0))
            nIp2 = Convert.ToInt32(Split(strNetExport, ".")(1))
            nIp3 = Convert.ToInt32(Split(strNetExport, ".")(2))
            nIp4 = Convert.ToInt32(Split(strNetExport, ".")(3))
            stGigEDev.nNetExport = ((nIp1 << 24) Or (nIp2 << 16) Or (nIp3 << 8) Or nIp4)
        Catch ex As Exception
            Console.WriteLine("Invalid input!")
            Console.WriteLine("push enter to exit")
            System.Console.ReadLine()
            End
        End Try
        Dim stGigeInfoPtr As IntPtr = Marshal.AllocHGlobal(216)
        Marshal.StructureToPtr(stGigEDev, stGigeInfoPtr, False)
        ReDim stDevInfo.stSpecialInfo.stGigEInfo(539)
        Marshal.Copy(stGigeInfoPtr, stDevInfo.stSpecialInfo.stGigEInfo, 0, 540)

        Do While (True)
            ' ch:创建句柄 | en:Create handle
            nRet = MyCamera.CreateDevice(stDevInfo)
            If 0 <> nRet Then
                Console.WriteLine("Create device failed:{0:x8}", nRet)
                Exit Do
            End If

            ' ch:打开相机 | en:Open devic
            nRet = MyCamera.OpenDevice()
            If 0 <> nRet Then
                Console.WriteLine("Open device failed:{0:x8}", nRet)
                Exit Do
            End If

            ' ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera)
            If stDevInfo.nTLayerType = CCamera.MV_GIGE_DEVICE Then
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

            Dim nCount As Int32 = 0
            Dim stFrameOut As CCamera.MV_FRAME_OUT = New CCamera.MV_FRAME_OUT

            ' ch:抓取图像 | en:Get image
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

        Console.WriteLine("push enter to exit")
        System.Console.ReadLine()
    End Sub
End Module
