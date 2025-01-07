Imports System.Runtime.InteropServices
Imports System.Threading.Thread
Imports System.Net.IPAddress
Imports System.IO
Imports System.Text

Public Class BasicDemo
    Dim MyCamera As CCamera = New CCamera

    ' ch:用于从驱动获取图像的缓存 | en:Buffer to get image from driver
    Dim m_nBufSizeForDriver As UInt32 = 1000 * 1000 * 3
    Dim m_pBufForDriver(m_nBufSizeForDriver) As Byte

    ' ch:成员变量，用于控制相机 | en:Member variable to control camera
    Dim m_stDeviceInfoList As CCamera.MV_CC_DEVICE_INFO_LIST = New CCamera.MV_CC_DEVICE_INFO_LIST
    Dim m_nDeviceIndex As UInt32
    Dim m_bIsGrabbing As Boolean = False
    Dim m_hGrabHandle As System.Threading.Thread
    Dim m_stFrameInfoEx As CCamera.MV_FRAME_OUT_INFO_EX = New CCamera.MV_FRAME_OUT_INFO_EX()
    Dim m_ReadWriteLock As System.Threading.ReaderWriterLock

    ''' <summary>
    ''' ch:枚举设备按钮操作 | en:Button operation for device enum
    ''' </summary>
    ''' <param name="sender"></param>
    ''' <param name="e"></param>
    ''' <remarks></remarks>
    Private Sub ButtonEnumDevice_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles ButtonEnumDevice.Click
        '清空设备列表 | en:Clear up device list
        ComboBoxDeviceList.Items.Clear()
        ComboBoxDeviceList.SelectedIndex = -1

        Dim Info As String
        Dim nRet As Int32 = CCamera.MV_OK
        ' ch:枚举设备 | en:Enumerate devices
        nRet = CCamera.EnumDevices((CCamera.MV_GIGE_DEVICE Or CCamera.MV_USB_DEVICE), m_stDeviceInfoList)
        If CCamera.MV_OK <> nRet Then
            MsgBox("Fail to enumerate devices" & Convert.ToString(nRet))
            Return
        End If

        If (0 = m_stDeviceInfoList.nDeviceNum) Then
            MsgBox("No Find Device !")
            Return
        End If

        ' ch:将设备信息放到设备列表中 | en:Put device information in the device list
        Dim i As Int32
        For i = 0 To m_stDeviceInfoList.nDeviceNum - 1
            Dim stDeviceInfo As CCamera.MV_CC_DEVICE_INFO = New CCamera.MV_CC_DEVICE_INFO
            stDeviceInfo = CType(Marshal.PtrToStructure(m_stDeviceInfoList.pDeviceInfo(i), GetType(CCamera.MV_CC_DEVICE_INFO)), CCamera.MV_CC_DEVICE_INFO)

            If (CCamera.MV_GIGE_DEVICE = stDeviceInfo.nTLayerType) Then
                Dim stGigeInfoPtr As IntPtr = Marshal.AllocHGlobal(216)
                Marshal.Copy(stDeviceInfo.stSpecialInfo.stGigEInfo, 0, stGigeInfoPtr, 216)
                Dim stGigeInfo As CCamera.MV_GIGE_DEVICE_INFO
                stGigeInfo = CType(Marshal.PtrToStructure(stGigeInfoPtr, GetType(CCamera.MV_GIGE_DEVICE_INFO)), CCamera.MV_GIGE_DEVICE_INFO)
                Dim nIpByte1 As UInt32 = (stGigeInfo.nCurrentIp And &HFF000000) >> 24
                Dim nIpByte2 As UInt32 = (stGigeInfo.nCurrentIp And &HFF0000) >> 16
                Dim nIpByte3 As UInt32 = (stGigeInfo.nCurrentIp And &HFF00) >> 8
                Dim nIpByte4 As UInt32 = (stGigeInfo.nCurrentIp And &HFF)
                
                If (stGigeInfo.chUserDefinedName = "") Then
                    Info = "DEV:[" & Convert.ToString(i) & "] " & stGigeInfo.chModelName & "(" & nIpByte1.ToString() & "." & nIpByte2.ToString() & "." & nIpByte3.ToString() & "." & nIpByte4.ToString() & ")"
                Else
                    Info = "DEV:[" & Convert.ToString(i) & "] " & stGigeInfo.chUserDefinedName & "(" & nIpByte1.ToString() & "." & nIpByte2.ToString() & "." & nIpByte3.ToString() & "." & nIpByte4.ToString() & ")"
                End If
                ComboBoxDeviceList.Items.Add(Info)
            Else
                Dim stUsbInfoPtr As IntPtr = Marshal.AllocHGlobal(540)
                Marshal.Copy(stDeviceInfo.stSpecialInfo.stUsb3VInfo, 0, stUsbInfoPtr, 540)
                Dim stUsbInfo As CCamera.MV_USB3_DEVICE_INFO
                stUsbInfo = CType(Marshal.PtrToStructure(stUsbInfoPtr, GetType(CCamera.MV_USB3_DEVICE_INFO)), CCamera.MV_USB3_DEVICE_INFO)
                Dim strUserDefinedName As String = ""

                'If (CCamera.IsTextUTF8(stUsbInfo.chUserDefinedName)) Then
                '    strUserDefinedName = UTF8Encoding.UTF8.GetString(stUsbInfo.chUserDefinedName).TrimEnd("\0")
                'Else
                '    strUserDefinedName = Encoding.GetEncoding("GB2312").GetString(stUsbInfo.chUserDefinedName).TrimEnd("\0")
                'End If

                If (stUsbInfo.chUserDefinedName = "") Then
                    Info = "U3V:[" & Convert.ToString(i) & "] " & stUsbInfo.chModelName & "(" & stUsbInfo.chSerialNumber & ")"
                Else
                    Info = "U3V:[" & Convert.ToString(i) & "] " & stUsbInfo.chUserDefinedName & "(" & stUsbInfo.chSerialNumber & ")"
                End If

                ComboBoxDeviceList.Items.Add(Info)
            End If
        Next i

        If (0 < m_stDeviceInfoList.nDeviceNum) Then
            ComboBoxDeviceList.SelectedIndex = 0
        End If

        ' ch:设置各个控件的可见度 | en:Set visibility of controls
        GroupBoxInit.Enabled = True
        GroupBoxDeviceControl.Enabled = False
        GroupBoxGrabImage.Enabled = False
        GroupBoxImageSave.Enabled = False
        GroupBoxParam.Enabled = False

        ComboBoxDeviceList.Enabled = True
        ButtonEnumDevice.Enabled = True
        ButtonOpenDevice.Enabled = False
        ButtonCloseDevice.Enabled = False
        RadioButtonTriggerOff.Enabled = False
        RadioButtonTriggerOn.Enabled = False
        ButtonStartGrabbing.Enabled = False
        ButtonStopGrabbing.Enabled = False
        CheckBoxSoftware.Enabled = False
        ButtonSoftwareOnce.Enabled = False
        ButtonSaveBmp.Enabled = False
        ButtonSaveJpg.Enabled = False
        ButtonSaveTiff.Enabled = False
        ButtonSavePng.Enabled = False
        TextBoxExposureTime.Enabled = False
        TextBoxGain.Enabled = False
        TextBoxFrameRate.Enabled = False
        ButtonParamGet.Enabled = False
        ButtonParamSet.Enabled = False
    End Sub

    ' ch:打开设备按钮操作 | en:Button operation for opening device
    Private Sub ButtonOpenDevice_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles ButtonOpenDevice.Click
        ' ch:创建句柄 | en:Create handle
        Dim nRet As Int32 = CCamera.MV_OK
        Dim stDeviceInfo As CCamera.MV_CC_DEVICE_INFO
        stDeviceInfo = CType(Marshal.PtrToStructure(m_stDeviceInfoList.pDeviceInfo(m_nDeviceIndex), GetType(CCamera.MV_CC_DEVICE_INFO)), CCamera.MV_CC_DEVICE_INFO)
        nRet = MyCamera.CreateDevice(stDeviceInfo)
        If CCamera.MV_OK <> nRet Then
            MsgBox("Fail to create handle")
            Return
        End If

        ' ch:打开设备 | en:Open device
        nRet = MyCamera.OpenDevice()
        If CCamera.MV_OK <> nRet Then
            MyCamera.DestroyDevice()
            MsgBox("Open device failed")
            Return
        End If

        ' ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera)
        If stDeviceInfo.nTLayerType = CCamera.MV_GIGE_DEVICE Then
            Dim nPacketSize As Int32
            nPacketSize = MyCamera.GIGE_GetOptimalPacketSize()
            If nPacketSize > 0 Then
                nRet = MyCamera.SetIntValueEx("GevSCPSPacketSize", nPacketSize)
                If 0 <> nRet Then
                    MsgBox("Set Packet Size failed")
                End If
            Else
                MsgBox("Get Packet Size failed")
            End If
        End If

        ' ch:获取触发模式 | en:Acquire trigger mode
        Dim stTriggerMode As CCamera.MVCC_ENUMVALUE = New CCamera.MVCC_ENUMVALUE
        nRet = MyCamera.GetEnumValue("TriggerMode", stTriggerMode)
        If CCamera.MV_OK <> nRet Then
            MsgBox("Fail to acquire trigger mode")
            Return
        End If
        If 0 = stTriggerMode.nCurValue Then
            RadioButtonTriggerOff.Checked = True
            RadioButtonTriggerOn.Checked = False
        Else
            RadioButtonTriggerOff.Checked = False
            RadioButtonTriggerOn.Checked = True
        End If

        ' ch:获取触发源 | en:Acquire trigger source
        Dim stTriggerSource As CCamera.MVCC_ENUMVALUE = New CCamera.MVCC_ENUMVALUE
        nRet = MyCamera.GetEnumValue("TriggerSource", stTriggerSource)
        If CCamera.MV_OK <> nRet Then
            MsgBox("Fail to acquire trigger source")
            Return
        End If
        If CCamera.MV_CAM_TRIGGER_SOURCE.MV_TRIGGER_SOURCE_SOFTWARE = stTriggerSource.nCurValue Then
            CheckBoxSoftware.Checked = True
        Else
            CheckBoxSoftware.Checked = False
        End If

        ' ch:获取曝光时间 | en:Acquire exposure time 
        Dim stExposureTime As CCamera.MVCC_FLOATVALUE = New CCamera.MVCC_FLOATVALUE
        nRet = MyCamera.GetFloatValue("ExposureTime", stExposureTime)
        If CCamera.MV_OK <> nRet Then
            MsgBox("Fail to acquire exposure time")
        End If
        TextBoxExposureTime.Text = Convert.ToString(stExposureTime.fCurValue)
        ' ch:获取增益 | en:Acquire gain
        Dim stGain As CCamera.MVCC_FLOATVALUE = New CCamera.MVCC_FLOATVALUE
        nRet = MyCamera.GetFloatValue("Gain", stGain)
        If CCamera.MV_OK <> nRet Then
            MsgBox("Fail to acquire gain")
        End If
        TextBoxGain.Text = Convert.ToString(stGain.fCurValue)
        ' ch:获取帧率 | en:Acquire frame rate
        Dim stFrameRate As CCamera.MVCC_FLOATVALUE = New CCamera.MVCC_FLOATVALUE
        nRet = MyCamera.GetFloatValue("ResultingFrameRate", stFrameRate)
        If CCamera.MV_OK <> nRet Then
            MsgBox("Fail to acquire frame rate")
        End If
        TextBoxFrameRate.Text = Convert.ToString(stFrameRate.fCurValue)

        ' ch:模式控制 | en:Mode control
        If RadioButtonTriggerOn.Checked Then
            RadioButtonTriggerOn.Enabled = False
            RadioButtonTriggerOff.Enabled = True
        Else
            RadioButtonTriggerOn.Enabled = True
            RadioButtonTriggerOff.Enabled = False
        End If

        If (RadioButtonTriggerOn.Checked) Then
            CheckBoxSoftware.Enabled = True
        Else
            CheckBoxSoftware.Enabled = False
        End If

        If (RadioButtonTriggerOn.Checked And CheckBoxSoftware.Checked) Then
            ButtonSoftwareOnce.Enabled = True
        Else
            ButtonSoftwareOnce.Enabled = False
        End If

        ' ch:设置各个控件的可见性 | en:Set visibility of controls
        GroupBoxDeviceControl.Enabled = True
        GroupBoxGrabImage.Enabled = True
        GroupBoxImageSave.Enabled = True
        GroupBoxParam.Enabled = True

        ComboBoxDeviceList.Enabled = False
        ButtonOpenDevice.Enabled = False
        ButtonCloseDevice.Enabled = True
        ButtonStartGrabbing.Enabled = True
        ButtonStopGrabbing.Enabled = False
        ButtonSaveBmp.Enabled = False
        ButtonSaveJpg.Enabled = False
        ButtonSaveTiff.Enabled = False
        ButtonSavePng.Enabled = False
        TextBoxExposureTime.Enabled = True
        TextBoxGain.Enabled = True
        TextBoxFrameRate.Enabled = True
        ButtonParamGet.Enabled = True
        ButtonParamSet.Enabled = True

    End Sub

    ' ch:关闭设备 | en:Close device
    Private Sub ButtonCloseDevice_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles ButtonCloseDevice.Click
        If m_bIsGrabbing Then
            m_bIsGrabbing = False
            m_hGrabHandle.Join()
        End If

        Dim nRet As Int32 = CCamera.MV_OK
        nRet = MyCamera.CloseDevice()
        If CCamera.MV_OK <> nRet Then
            MsgBox("Fail to close device")
            Return
        End If

        nRet = MyCamera.DestroyDevice()
        If CCamera.MV_OK <> nRet Then
            MsgBox("Fail to destroy handle")
            Return
        End If

        TextBoxExposureTime.Text = "0"
        TextBoxGain.Text = "0"
        TextBoxFrameRate.Text = "0"

        ' ch:设置各个控件的可见性 | en:Set visibility of controls
        ComboBoxDeviceList.Enabled = True
        ButtonOpenDevice.Enabled = True
        ButtonCloseDevice.Enabled = False

        GroupBoxGrabImage.Enabled = False
        ButtonStartGrabbing.Enabled = False
        ButtonStopGrabbing.Enabled = False
        ButtonSoftwareOnce.Enabled = False

        GroupBoxImageSave.Enabled = False
        GroupBoxParam.Enabled = False

    End Sub

    ' ch:刚加载时的初始化 | en:Initialization for loading
    Private Sub Thread_Load(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles MyBase.Load
        Control.CheckForIllegalCrossThreadCalls = False
        ' ch:设置各个控件的可见性 | en:Set visibility of controls
        GroupBoxInit.Enabled = True
        GroupBoxDeviceControl.Enabled = False
        GroupBoxGrabImage.Enabled = False
        GroupBoxImageSave.Enabled = False
        GroupBoxParam.Enabled = False

        ComboBoxDeviceList.Enabled = False
        ButtonEnumDevice.Enabled = True
        ButtonOpenDevice.Enabled = False
        ButtonCloseDevice.Enabled = False
        RadioButtonTriggerOff.Enabled = False
        RadioButtonTriggerOn.Enabled = False
        ButtonStartGrabbing.Enabled = False
        ButtonStopGrabbing.Enabled = False
        CheckBoxSoftware.Enabled = False
        ButtonSoftwareOnce.Enabled = False
        ButtonSaveBmp.Enabled = False
        ButtonSaveJpg.Enabled = False
        ButtonSaveTiff.Enabled = False
        ButtonSavePng.Enabled = False
        TextBoxExposureTime.Enabled = False
        TextBoxGain.Enabled = False
        TextBoxFrameRate.Enabled = False
        ButtonParamGet.Enabled = False
        ButtonParamSet.Enabled = False
    End Sub

    ' ch:设置连续采集 | en:Set continuous acquisition
    Private Sub RadioButtonTriggerOff_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles RadioButtonTriggerOff.CheckedChanged
        ' ch:关闭触发 | en:Close trigger mode
        Dim nRet As Int32 = CCamera.MV_OK
        nRet = MyCamera.SetEnumValue("TriggerMode", CCamera.MV_CAM_TRIGGER_MODE.MV_TRIGGER_MODE_OFF)
        If CCamera.MV_OK <> nRet Then
            MsgBox("Fail to close trigger mode")
        End If

        RadioButtonTriggerOff.Enabled = False
        RadioButtonTriggerOn.Enabled = True
        If (RadioButtonTriggerOn.Checked) Then
            CheckBoxSoftware.Enabled = True
        Else
            CheckBoxSoftware.Enabled = False
        End If

        If (RadioButtonTriggerOn.Checked And CheckBoxSoftware.Checked) Then
            ButtonSoftwareOnce.Enabled = True
        Else
            ButtonSoftwareOnce.Enabled = False
        End If

    End Sub

    ' ch:设置触发模式 | en:Set trigger mode
    Private Sub RadioButtonTriggerOn_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles RadioButtonTriggerOn.CheckedChanged
        ' ch:开启触发 | en:Open trigger mode
        Dim nRet As Int32 = CCamera.MV_OK
        nRet = MyCamera.SetEnumValue("TriggerMode", CCamera.MV_CAM_TRIGGER_MODE.MV_TRIGGER_MODE_ON)
        If CCamera.MV_OK <> nRet Then
            MsgBox("Fail to close trigger mode")
        End If

        RadioButtonTriggerOff.Enabled = True
        RadioButtonTriggerOn.Enabled = False
        If (RadioButtonTriggerOn.Checked) Then
            CheckBoxSoftware.Enabled = True
        Else
            CheckBoxSoftware.Enabled = False
        End If

        If (RadioButtonTriggerOn.Checked And CheckBoxSoftware.Checked) Then
            ButtonSoftwareOnce.Enabled = True
        Else
            ButtonSoftwareOnce.Enabled = False
        End If

    End Sub

    Sub ReceiveThreadProcess()
        Dim stFrameOut As CCamera.MV_FRAME_OUT = New CCamera.MV_FRAME_OUT()
        Dim stDisplayInfo As CCamera.MV_DISPLAY_FRAME_INFO = New CCamera.MV_DISPLAY_FRAME_INFO()

        While (m_bIsGrabbing)
            Dim nRet = MyCamera.GetImageBuffer(stFrameOut, 1000)
            If CCamera.MV_OK = nRet Then

                m_ReadWriteLock.AcquireWriterLock(System.Threading.Timeout.Infinite)
                If stFrameOut.stFrameInfo.nFrameLen > m_nBufSizeForDriver Then
                    m_nBufSizeForDriver = stFrameOut.stFrameInfo.nFrameLen
                    ReDim m_pBufForDriver(m_nBufSizeForDriver)
                End If

                m_stFrameInfoEx = stFrameOut.stFrameInfo
                Marshal.Copy(stFrameOut.pBufAddr, m_pBufForDriver, 0, stFrameOut.stFrameInfo.nFrameLen)
                m_ReadWriteLock.ReleaseWriterLock() ' 释放写入锁定

                stDisplayInfo.hWnd = PictureBoxDisplay.Handle
                stDisplayInfo.pData = stFrameOut.pBufAddr
                stDisplayInfo.nDataLen = stFrameOut.stFrameInfo.nFrameLen
                stDisplayInfo.nWidth = stFrameOut.stFrameInfo.nWidth
                stDisplayInfo.nHeight = stFrameOut.stFrameInfo.nHeight
                stDisplayInfo.enPixelType = stFrameOut.stFrameInfo.enPixelType
                MyCamera.DisplayOneFrame(stDisplayInfo)

                MyCamera.FreeImageBuffer(stFrameOut)
            Else
                If RadioButtonTriggerOn.Checked Then
                    Threading.Thread.Sleep(5)
                End If
            End If
        End While

    End Sub

    ' ch:开启采集 | en:Start grabbing
    Private Sub ButtonStartGrabbing_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles ButtonStartGrabbing.Click
        m_bIsGrabbing = True
        m_ReadWriteLock = New Threading.ReaderWriterLock()
        m_hGrabHandle = New Threading.Thread(AddressOf ReceiveThreadProcess)
        m_hGrabHandle.Start()

        m_stFrameInfoEx.nFrameLen = 0 '//取流之前先清除帧长度
        Dim nRet As Int32
        nRet = MyCamera.StartGrabbing()
        If CCamera.MV_OK <> nRet Then
            m_bIsGrabbing = False
            m_hGrabHandle.Join()
            MsgBox("Fail to start grabbing")
        End If

        ButtonStartGrabbing.Enabled = False
        ButtonStopGrabbing.Enabled = True
        ButtonSaveBmp.Enabled = True
        ButtonSaveJpg.Enabled = True
        ButtonSaveTiff.Enabled = True
        ButtonSavePng.Enabled = True

    End Sub

    ' ch:停止采集 | en:Stop grabbing
    Private Sub ButtonStopGrabbing_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles ButtonStopGrabbing.Click
        m_bIsGrabbing = False
        m_hGrabHandle.Join()

        Dim nRet As Int32
        nRet = MyCamera.StopGrabbing()
        If CCamera.MV_OK <> nRet Then
            MsgBox("Fail to stop grabbing")
        End If
        ButtonStartGrabbing.Enabled = True
        ButtonStopGrabbing.Enabled = False
        ButtonSaveBmp.Enabled = False
        ButtonSaveJpg.Enabled = False
        ButtonSaveTiff.Enabled = False
        ButtonSavePng.Enabled = False

    End Sub

    ' ch:软触发模式 | en:Software trigger mode
    Private Sub CheckBoxSoftware_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles CheckBoxSoftware.CheckedChanged
        If (CheckBoxSoftware.Checked) Then
            ' ch:设置软触发 | en:Set software trigger
            Dim nRet As Int32
            nRet = MyCamera.SetEnumValue("TriggerSource", CCamera.MV_CAM_TRIGGER_SOURCE.MV_TRIGGER_SOURCE_SOFTWARE)
            If CCamera.MV_OK <> nRet Then
                MsgBox("Fail to set software trigger")
            End If
        Else
            ' ch:设置硬触发 | en:Set hardware trigger
            Dim nRet As Int32
            nRet = MyCamera.SetEnumValue("TriggerSource", CCamera.MV_CAM_TRIGGER_SOURCE.MV_TRIGGER_SOURCE_LINE0)
            If CCamera.MV_OK <> nRet Then
                MsgBox("Fail to set hardware trigger")
            End If
        End If

        CheckBoxSoftware.Enabled = True
        If (RadioButtonTriggerOn.Checked And CheckBoxSoftware.Checked) Then
            ButtonSoftwareOnce.Enabled = True
        Else
            ButtonSoftwareOnce.Enabled = False
        End If

    End Sub

    ' ch:软触发一次 | en:Software trigger once
    Private Sub ButtonSoftwareOnce_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles ButtonSoftwareOnce.Click
        Dim nRet As Int32
        nRet = MyCamera.SetCommandValue("TriggerSoftware")
        If CCamera.MV_OK <> nRet Then
            MsgBox("Fail to software trigger once")
        End If
    End Sub

    ' ch:保存bmp图片 | en:Save bmp image
    Private Sub ButtonSaveBmp_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles ButtonSaveBmp.Click
        If False = m_bIsGrabbing Then
            MsgBox("Not Start Grabbing!")
            Return
        End If

        Dim stSaveImageParam As CCamera.MV_SAVE_IMG_TO_FILE_PARAM = New CCamera.MV_SAVE_IMG_TO_FILE_PARAM()

        m_ReadWriteLock.AcquireWriterLock(System.Threading.Timeout.Infinite)
        If m_stFrameInfoEx.nFrameLen = 0 Then
            m_ReadWriteLock.ReleaseWriterLock() ' 释放写入锁定
            MsgBox("Save Bmp Fail!")
            Return
        End If

        Dim nRet As Int32 = CCamera.MV_OK
        Dim pData As IntPtr = Marshal.UnsafeAddrOfPinnedArrayElement(m_pBufForDriver, 0)
        stSaveImageParam.pData = pData
        stSaveImageParam.nDataLen = m_stFrameInfoEx.nFrameLen
        stSaveImageParam.enPixelType = m_stFrameInfoEx.enPixelType
        stSaveImageParam.nWidth = m_stFrameInfoEx.nWidth
        stSaveImageParam.nHeight = m_stFrameInfoEx.nHeight
        stSaveImageParam.enImageType = CCamera.MV_SAVE_IAMGE_TYPE.MV_Image_Bmp
        stSaveImageParam.iMethodValue = 2
        stSaveImageParam.pImagePath = "Image_w" & stSaveImageParam.nWidth.ToString() & "_h" & stSaveImageParam.nHeight.ToString() & "_fn" & m_stFrameInfoEx.nFrameNum.ToString() & ".bmp"
        nRet = MyCamera.SaveImageToFile(stSaveImageParam)
        If (CCamera.MV_OK <> nRet) Then
            m_ReadWriteLock.ReleaseWriterLock() ' 释放写入锁定
            MsgBox("Save Image fail!")
            Return
        End If
        m_ReadWriteLock.ReleaseWriterLock() ' 释放写入锁定

        MsgBox("Save BMP succeed")
        Return

    End Sub

    ' ch:保存jpg图片 | en:Save jpg image
    Private Sub ButtonSaveJpg_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles ButtonSaveJpg.Click
        If False = m_bIsGrabbing Then
            MsgBox("Not Start Grabbing!")
            Return
        End If

        Dim stSaveImageParam As CCamera.MV_SAVE_IMG_TO_FILE_PARAM = New CCamera.MV_SAVE_IMG_TO_FILE_PARAM()

        m_ReadWriteLock.AcquireWriterLock(System.Threading.Timeout.Infinite)
        If m_stFrameInfoEx.nFrameLen = 0 Then
            m_ReadWriteLock.ReleaseWriterLock() ' 释放写入锁定
            MsgBox("Save Jpeg Fail!")
            Return
        End If

        Dim nRet As Int32
        Dim pData As IntPtr = Marshal.UnsafeAddrOfPinnedArrayElement(m_pBufForDriver, 0)
        stSaveImageParam.pData = pData
        stSaveImageParam.nDataLen = m_stFrameInfoEx.nFrameLen
        stSaveImageParam.enPixelType = m_stFrameInfoEx.enPixelType
        stSaveImageParam.nWidth = m_stFrameInfoEx.nWidth
        stSaveImageParam.nHeight = m_stFrameInfoEx.nHeight
        stSaveImageParam.enImageType = CCamera.MV_SAVE_IAMGE_TYPE.MV_Image_Jpeg
        stSaveImageParam.iMethodValue = 2
        stSaveImageParam.nQuality = 80
        stSaveImageParam.pImagePath = "Image_w" & stSaveImageParam.nWidth.ToString() & "_h" & stSaveImageParam.nHeight.ToString() & "_fn" & m_stFrameInfoEx.nFrameNum.ToString() & ".jpg"
        nRet = MyCamera.SaveImageToFile(stSaveImageParam)
        If (CCamera.MV_OK <> nRet) Then
            m_ReadWriteLock.ReleaseWriterLock() ' 释放写入锁定
            MsgBox("Save Image fail!")
            Return
        End If
        m_ReadWriteLock.ReleaseWriterLock() ' 释放写入锁定

        MsgBox("Save Jpeg succeed")
        Return
    End Sub

    Private Sub ButtonSavePng_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles ButtonSavePng.Click
        If False = m_bIsGrabbing Then
            MsgBox("Not Start Grabbing!")
            Return
        End If

        Dim stSaveImageParam As CCamera.MV_SAVE_IMG_TO_FILE_PARAM = New CCamera.MV_SAVE_IMG_TO_FILE_PARAM()

        m_ReadWriteLock.AcquireWriterLock(System.Threading.Timeout.Infinite)
        If m_stFrameInfoEx.nFrameLen = 0 Then
            m_ReadWriteLock.ReleaseWriterLock() ' 释放写入锁定
            MsgBox("Save Png Fail!")
            Return
        End If

        Dim nRet As Int32
        Dim pData As IntPtr = Marshal.UnsafeAddrOfPinnedArrayElement(m_pBufForDriver, 0)
        stSaveImageParam.pData = pData
        stSaveImageParam.nDataLen = m_stFrameInfoEx.nFrameLen
        stSaveImageParam.enPixelType = m_stFrameInfoEx.enPixelType
        stSaveImageParam.nWidth = m_stFrameInfoEx.nWidth
        stSaveImageParam.nHeight = m_stFrameInfoEx.nHeight
        stSaveImageParam.enImageType = CCamera.MV_SAVE_IAMGE_TYPE.MV_Image_Png
        stSaveImageParam.nQuality = 8
        stSaveImageParam.iMethodValue = 2
        stSaveImageParam.pImagePath = "Image_w" & stSaveImageParam.nWidth.ToString() & "_h" & stSaveImageParam.nHeight.ToString() & "_fn" & m_stFrameInfoEx.nFrameNum.ToString() & ".png"
        nRet = MyCamera.SaveImageToFile(stSaveImageParam)
        If (CCamera.MV_OK <> nRet) Then
            m_ReadWriteLock.ReleaseWriterLock() ' 释放写入锁定
            MsgBox("Save Image fail!")
            Return
        End If
        m_ReadWriteLock.ReleaseWriterLock() ' 释放写入锁定

        MsgBox("Save Png succeed")
        Return
    End Sub

    Private Sub ButtonSaveTiff_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles ButtonSaveTiff.Click
        If False = m_bIsGrabbing Then
            MsgBox("Not Start Grabbing!")
            Return
        End If

        Dim stSaveImageParam As CCamera.MV_SAVE_IMG_TO_FILE_PARAM = New CCamera.MV_SAVE_IMG_TO_FILE_PARAM()

        m_ReadWriteLock.AcquireWriterLock(System.Threading.Timeout.Infinite)
        If m_stFrameInfoEx.nFrameLen = 0 Then
            m_ReadWriteLock.ReleaseWriterLock() ' 释放写入锁定
            MsgBox("Save Tiff Fail!")
            Return
        End If

        Dim nRet As Int32
        Dim pData As IntPtr = Marshal.UnsafeAddrOfPinnedArrayElement(m_pBufForDriver, 0)
        stSaveImageParam.pData = pData
        stSaveImageParam.nDataLen = m_stFrameInfoEx.nFrameLen
        stSaveImageParam.enPixelType = m_stFrameInfoEx.enPixelType
        stSaveImageParam.nWidth = m_stFrameInfoEx.nWidth
        stSaveImageParam.nHeight = m_stFrameInfoEx.nHeight
        stSaveImageParam.enImageType = CCamera.MV_SAVE_IAMGE_TYPE.MV_Image_Tif
        stSaveImageParam.iMethodValue = 2
        stSaveImageParam.pImagePath = "Image_w" & stSaveImageParam.nWidth.ToString() & "_h" & stSaveImageParam.nHeight.ToString() & "_fn" & m_stFrameInfoEx.nFrameNum.ToString() & ".tif"
        nRet = MyCamera.SaveImageToFile(stSaveImageParam)
        If (CCamera.MV_OK <> nRet) Then
            m_ReadWriteLock.ReleaseWriterLock() ' 释放写入锁定
            MsgBox("Save Image fail!")
            Return
        End If
        m_ReadWriteLock.ReleaseWriterLock() ' 释放写入锁定

        MsgBox("Save Tiff succeed")
        Return
    End Sub

    ' ch:获取参数 | en:Get parameters
    Private Sub ButtonParamGet_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles ButtonParamGet.Click
        ' ch:获取曝光时间 | en:Get exposure time
        Dim nRet As Int32
        Dim stExposureTime As CCamera.MVCC_FLOATVALUE = New CCamera.MVCC_FLOATVALUE
        nRet = MyCamera.GetFloatValue("ExposureTime", stExposureTime)
        If CCamera.MV_OK <> nRet Then
            MsgBox("Fail to get exposure time")
        End If
        TextBoxExposureTime.Text = Convert.ToString(stExposureTime.fCurValue)
        ' ch:获取增益 | en:Get gain
        Dim stGain As CCamera.MVCC_FLOATVALUE = New CCamera.MVCC_FLOATVALUE
        nRet = MyCamera.GetFloatValue("Gain", stGain)
        If CCamera.MV_OK <> nRet Then
            MsgBox("Fail to get gain")
        End If
        TextBoxGain.Text = Convert.ToString(stGain.fCurValue)
        ' ch:获取帧率 | en:Get frame rate
        Dim stFrameRate As CCamera.MVCC_FLOATVALUE = New CCamera.MVCC_FLOATVALUE
        nRet = MyCamera.GetFloatValue("ResultingFrameRate", stFrameRate)
        If CCamera.MV_OK <> nRet Then
            MsgBox("Fail to get frame rate")
        End If
        TextBoxFrameRate.Text = Convert.ToString(stFrameRate.fCurValue)

    End Sub

    ' ch:设置参数 | en:Set Parameters
    Private Sub ButtonParamSet_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles ButtonParamSet.Click
        Dim fExposureTime As Single = 0
        Dim fGain As Single = 0
        Dim fFrameRate As Single = 0
        Try
            fExposureTime = Convert.ToSingle(TextBoxExposureTime.Text)
            fGain = Convert.ToSingle(TextBoxGain.Text)
            fFrameRate = Convert.ToSingle(TextBoxFrameRate.Text)
        Catch
            MsgBox("Incorrect parameter format")
            Return
        Finally

        End Try

        Dim nRet As Int32
        ' ch:将自动曝光和自动增益关闭 | en:Close auto-exposure and auto-gain
        If RadioButtonTriggerOff.Checked Then
            nRet = MyCamera.SetEnumValue("ExposureAuto", CCamera.MV_CAM_EXPOSURE_AUTO_MODE.MV_EXPOSURE_AUTO_MODE_OFF)
            If CCamera.MV_OK <> nRet Then
                MsgBox("Fail to close auto-exposure")
            End If
        End If

        nRet = MyCamera.SetEnumValue("GainAuto", CCamera.MV_CAM_GAIN_MODE.MV_GAIN_MODE_OFF)
        If CCamera.MV_OK <> nRet Then
            MsgBox("Fail to close auto-gain")
        End If

        ' ch:设置曝光时间 | en:Set exposure time
        nRet = MyCamera.SetFloatValue("ExposureTime", fExposureTime)
        If CCamera.MV_OK <> nRet Then
            MsgBox("Fail to set exposure time")
        End If

        ' ch:设置增益 | en:Set gain
        nRet = MyCamera.SetFloatValue("Gain", fGain)
        If CCamera.MV_OK <> nRet Then
            MsgBox("Fail to set gain")
        End If

        ' ch:设置帧率 | en:Set frame rate
        nRet = MyCamera.SetBoolValue("AcquisitionFrameRateEnable", True)
        If CCamera.MV_OK <> nRet Then
            MsgBox("Set frame rate enable fail")
        End If
        nRet = MyCamera.SetFloatValue("AcquisitionFrameRate", fFrameRate)
        If CCamera.MV_OK <> nRet Then
            MsgBox("Fail to set frame rate")
        End If

    End Sub

    ' ch:选择不同相机 | en:Select different cameras
    Private Sub ComboBoxDeviceList_SelectedIndexChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles ComboBoxDeviceList.SelectedIndexChanged
        m_nDeviceIndex = ComboBoxDeviceList.SelectedIndex

        ' ch:设置各个控件的可见性 | en:Set visibility of controls
        GroupBoxInit.Enabled = True
        GroupBoxDeviceControl.Enabled = True
        GroupBoxGrabImage.Enabled = False
        GroupBoxImageSave.Enabled = False
        GroupBoxParam.Enabled = False

        ComboBoxDeviceList.Enabled = True
        ButtonEnumDevice.Enabled = True
        ButtonOpenDevice.Enabled = True
        ButtonCloseDevice.Enabled = False
        RadioButtonTriggerOff.Enabled = False
        RadioButtonTriggerOn.Enabled = False
        ButtonStartGrabbing.Enabled = False
        ButtonStopGrabbing.Enabled = False
        CheckBoxSoftware.Enabled = False
        ButtonSoftwareOnce.Enabled = False
        ButtonSaveBmp.Enabled = False
        ButtonSaveJpg.Enabled = False
        ButtonSaveTiff.Enabled = False
        ButtonSavePng.Enabled = False
        TextBoxExposureTime.Enabled = False
        TextBoxGain.Enabled = False
        TextBoxFrameRate.Enabled = False
        ButtonParamGet.Enabled = False
        ButtonParamSet.Enabled = False
    End Sub

    Private Sub Thread_FormClosing(ByVal sender As System.Object, ByVal e As System.Windows.Forms.FormClosingEventArgs) Handles MyBase.FormClosing
        If m_bIsGrabbing Then
            m_bIsGrabbing = False
            m_hGrabHandle.Join()
        End If

        ' ch:关闭设备 | en:Close device
        Dim nRet As Int32 = CCamera.MV_OK
        nRet = MyCamera.CloseDevice()
        ' ch:销毁句柄 | en:Destroy handle
        MyCamera.DestroyDevice()
    End Sub
End Class