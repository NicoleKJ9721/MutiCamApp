
Imports System.Runtime.InteropServices
Imports System.Net.IPAddress

Public Class ForceIp
    Dim MyCamera As CCamera = New CCamera

    Dim m_stDeviceInfoList As CCamera.MV_CC_DEVICE_INFO_LIST = New CCamera.MV_CC_DEVICE_INFO_LIST
    Dim m_stDeviceInfo As CCamera.MV_CC_DEVICE_INFO = New CCamera.MV_CC_DEVICE_INFO
    Dim m_nDeviceIndex As UInt32
    Dim m_bIsOpen As Boolean

    Private Sub ButtonEnumDevice_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles ButtonEnumDevice.Click
        ComboBoxDeviceList.Items.Clear()
        ComboBoxDeviceList.SelectedIndex = -1

        Dim nRet As Int32 = CCamera.MV_OK
        nRet = CCamera.EnumDevices((CCamera.MV_GIGE_DEVICE), m_stDeviceInfoList)
        If CCamera.MV_OK <> nRet Then
            MsgBox("Fail to enumerate devices!" + Convert.ToString(nRet))
            Return
        End If


        If (0 = m_stDeviceInfoList.nDeviceNum) Then
            MsgBox("No Find Gige | Usb Device !")
            Return
        End If

        Dim Info As String
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

                Info = "DEV[" + Convert.ToString(i) + "] NAME[" + stGigeInfo.chUserDefinedName + "]IP[" + nIpByte1.ToString() + "." + nIpByte2.ToString() + "." + nIpByte3.ToString() + "." + nIpByte4.ToString() + "]"
                ComboBoxDeviceList.Items.Add(Info)
            Else
                Dim stUsbInfoPtr As IntPtr = Marshal.AllocHGlobal(540)
                Marshal.Copy(stDeviceInfo.stSpecialInfo.stUsb3VInfo, 0, stUsbInfoPtr, 540)
                Dim stUsbInfo As CCamera.MV_USB3_DEVICE_INFO
                stUsbInfo = CType(Marshal.PtrToStructure(stUsbInfoPtr, GetType(CCamera.MV_USB3_DEVICE_INFO)), CCamera.MV_USB3_DEVICE_INFO)

                Info = "DEV[" + Convert.ToString(i) + "] NAME[" + stUsbInfo.chUserDefinedName + "]Model[" + stUsbInfo.chSerialNumber + "]"
                ComboBoxDeviceList.Items.Add(Info)
            End If
        Next i

        If (0 < m_stDeviceInfoList.nDeviceNum) Then
            ComboBoxDeviceList.SelectedIndex = 0
        End If
        BtnSetIp.Enabled = True
    End Sub

    Private Sub ComboBoxDeviceList_SelectedIndexChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles ComboBoxDeviceList.SelectedIndexChanged
        m_nDeviceIndex = ComboBoxDeviceList.SelectedIndex

        Dim stDeviceInfo As CCamera.MV_CC_DEVICE_INFO = New CCamera.MV_CC_DEVICE_INFO
        stDeviceInfo = CType(Marshal.PtrToStructure(m_stDeviceInfoList.pDeviceInfo(m_nDeviceIndex), GetType(CCamera.MV_CC_DEVICE_INFO)), CCamera.MV_CC_DEVICE_INFO)
        Dim stGigeInfoPtr As IntPtr = Marshal.AllocHGlobal(216)
        Marshal.Copy(stDeviceInfo.stSpecialInfo.stGigEInfo, 0, stGigeInfoPtr, 216)
        Dim stGigeInfo As CCamera.MV_GIGE_DEVICE_INFO
        stGigeInfo = CType(Marshal.PtrToStructure(stGigeInfoPtr, GetType(CCamera.MV_GIGE_DEVICE_INFO)), CCamera.MV_GIGE_DEVICE_INFO)

        ' ch:显示ip | en:Display ip
        Dim nIpByte1 As UInt32 = (stGigeInfo.nCurrentIp And &HFF000000) >> 24
        Dim nIpByte2 As UInt32 = (stGigeInfo.nCurrentIp And &HFF0000) >> 16
        Dim nIpByte3 As UInt32 = (stGigeInfo.nCurrentIp And &HFF00) >> 8
        Dim nIpByte4 As UInt32 = (stGigeInfo.nCurrentIp And &HFF)

        Label4.Text = nIpByte1.ToString() + "." + nIpByte2.ToString() + "." + nIpByte3.ToString() + ".0---" + nIpByte1.ToString() + "." + nIpByte2.ToString() + "." + nIpByte3.ToString() + ".255"

        TextBoxSetIp.Text = nIpByte1.ToString() + "." + nIpByte2.ToString() + "." + nIpByte3.ToString() + "." + nIpByte4.ToString()

        ' ch:显示子网掩码 | en:Display subnet mask
        nIpByte1 = (stGigeInfo.nCurrentSubNetMask And &HFF000000) >> 24
        nIpByte2 = (stGigeInfo.nCurrentSubNetMask And &HFF0000) >> 16
        nIpByte3 = (stGigeInfo.nCurrentSubNetMask And &HFF00) >> 8
        nIpByte4 = (stGigeInfo.nCurrentSubNetMask And &HFF)

        TextBoxSetMask.Text = nIpByte1.ToString() + "." + nIpByte2.ToString() + "." + nIpByte3.ToString() + "." + nIpByte4.ToString()

        ' ch:显示默认网关 | en:Display default gateway
        nIpByte1 = (stGigeInfo.nDefultGateWay And &HFF000000) >> 24
        nIpByte2 = (stGigeInfo.nDefultGateWay And &HFF0000) >> 16
        nIpByte3 = (stGigeInfo.nDefultGateWay And &HFF00) >> 8
        nIpByte4 = (stGigeInfo.nDefultGateWay And &HFF)

        TextBoxSetDefaultWay.Text = nIpByte1.ToString() + "." + nIpByte2.ToString() + "." + nIpByte3.ToString() + "." + nIpByte4.ToString()

    End Sub

    Private Sub ButtonSetIp_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles BtnSetIp.Click
        Dim ctrlIpAddress As System.Net.IPAddress = System.Net.IPAddress.None
        Dim ctrlMaskAddress As System.Net.IPAddress = System.Net.IPAddress.None
        Dim ctrlDefaultWayAddress As System.Net.IPAddress = System.Net.IPAddress.None

        ' ch:检查ip | en:Check ip
        Dim bIsValidIp As Boolean = System.Net.IPAddress.TryParse(TextBoxSetIp.Text, ctrlIpAddress)
        If (Not bIsValidIp) Then
            MsgBox("Invalid IP address")
            Return
        End If

        ' ch:检查掩码 | en:Check mask
        bIsValidIp = System.Net.IPAddress.TryParse(TextBoxSetMask.Text, ctrlMaskAddress)
        If (Not bIsValidIp) Then
            MsgBox("Invalid mask")
            Return
        End If

        ' ch:检查网关 | en:Check gateway
        bIsValidIp = System.Net.IPAddress.TryParse(TextBoxSetDefaultWay.Text, ctrlDefaultWayAddress)
        If (Not bIsValidIp) Then
            MsgBox("Invalid Gateway")
            Return
        End If

        Dim nRet As Int32 = CCamera.MV_OK
        ' ch:创建句柄 | en:Create Handle
        m_stDeviceInfo = CType(Marshal.PtrToStructure(m_stDeviceInfoList.pDeviceInfo(m_nDeviceIndex), GetType(CCamera.MV_CC_DEVICE_INFO)), CCamera.MV_CC_DEVICE_INFO)
        nRet = MyCamera.CreateDevice(m_stDeviceInfo)
        If CCamera.MV_OK <> nRet Then
            MsgBox("Invalid gateway!")
        End If

        ' ch:ip转换 | en:ip conversion
        Dim nIp As UInt32 = Convert.ToUInt32(ctrlIpAddress.Address)
        Dim nIpByte1 As UInt32 = (nIp And &HFF000000) >> 24
        Dim nIpByte2 As UInt32 = (nIp And &HFF0000) >> 16
        Dim nIpByte3 As UInt32 = (nIp And &HFF00) >> 8
        Dim nIpByte4 As UInt32 = (nIp And &HFF)

        Dim nIpConvert As UInt32 = (nIpByte4 << 24) + (nIpByte3 << 16) + (nIpByte2 << 8) + nIpByte1

        ' ch:掩码转换 | en:Mask conversion
        Dim nMask As UInt32 = Convert.ToUInt32(ctrlMaskAddress.Address)
        Dim nMaskByte1 As UInt32 = (nMask And &HFF000000) >> 24
        Dim nMaskByte2 As UInt32 = (nMask And &HFF0000) >> 16
        Dim nMaskByte3 As UInt32 = (nMask And &HFF00) >> 8
        Dim nMaskByte4 As UInt32 = (nMask And &HFF)

        Dim nMaskConvert As UInt32 = (nMaskByte4 << 24) + (nMaskByte3 << 16) + (nMaskByte2 << 8) + nMaskByte1

        ' ch:默认网关转换 | en:Default gateway conversion
        Dim nDefaultWay As UInt32 = Convert.ToUInt32(ctrlDefaultWayAddress.Address)
        Dim nDefaultWayByte1 As UInt32 = (nDefaultWay And &HFF000000) >> 24
        Dim nDefaultWayByte2 As UInt32 = (nDefaultWay And &HFF0000) >> 16
        Dim nDefaultWayByte3 As UInt32 = (nDefaultWay And &HFF00) >> 8
        Dim nDefaultWayByte4 As UInt32 = (nDefaultWay And &HFF)
        Dim nDefaultWayConvert As UInt32 = (nDefaultWayByte4 << 24) + (nDefaultWayByte3 << 16) + (nDefaultWayByte2 << 8) + nDefaultWayByte1

        ' ch:判断设备IP是否可达
        Dim bAccessible As Boolean = CCamera.IsDeviceAccessible(m_stDeviceInfo, CCamera.MV_ACCESS_Exclusive)
        If bAccessible Then
            nRet = MyCamera.GIGE_SetIpConfig(CCamera.MV_IP_CFG_STATIC)
            If CCamera.MV_OK <> nRet Then
                MsgBox("Set Ip config fail", nRet)
                MyCamera.DestroyDevice()
                Return
            End If

            nRet = MyCamera.GIGE_ForceIpEx(nIpConvert, nMaskConvert, nDefaultWayConvert)
            If CCamera.MV_OK <> nRet Then
                MsgBox("ForceIp fail", nRet)
                MyCamera.DestroyDevice()
                Return
            End If
        Else
            nRet = MyCamera.GIGE_ForceIpEx(nIpConvert, nMaskConvert, nDefaultWayConvert)
            If CCamera.MV_OK <> nRet Then
                MsgBox("ForceIp fail", nRet)
                MyCamera.DestroyDevice()
                Return
            End If
            MyCamera.DestroyDevice()

            'ch:需要重新创建句柄，设置为静态IP方式进行保存
            Dim stGigEDev As CCamera.MV_GIGE_DEVICE_INFO = New CCamera.MV_GIGE_DEVICE_INFO
            stGigEDev.nCurrentIp = nIpConvert
            stGigEDev.nCurrentSubNetMask = nMaskConvert
            stGigEDev.nDefultGateWay = nDefaultWayConvert

            ' stGigEDev结构体转为stDevInfo.stSpecialInfo.stGigEInfo(Byte[])
            Dim stGigeInfoPtr As IntPtr = Marshal.AllocHGlobal(216)
            Marshal.StructureToPtr(stGigEDev, stGigeInfoPtr, False)
            ReDim m_stDeviceInfo.stSpecialInfo.stGigEInfo(539)
            Marshal.Copy(stGigeInfoPtr, m_stDeviceInfo.stSpecialInfo.stGigEInfo, 0, 540)
            Marshal.Release(stGigeInfoPtr)

            nRet = MyCamera.CreateDevice(m_stDeviceInfo)
            If CCamera.MV_OK <> nRet Then
                MsgBox("Create handle fail", nRet)
                Return
            End If
            nRet = MyCamera.GIGE_SetIpConfig(CCamera.MV_IP_CFG_STATIC)
            If CCamera.MV_OK <> nRet Then
                MsgBox("Set Ip config fail", nRet)
                MyCamera.DestroyDevice()
                Return
            End If
        End If
        MsgBox("IP Set Succeed!", 0)
        MyCamera.DestroyDevice()
        GC.Collect()
    End Sub

    Private Sub ForceIp_FormClosing(ByVal sender As System.Object, ByVal e As System.Windows.Forms.FormClosingEventArgs) Handles MyBase.FormClosing
        ' ch:关闭设备 | en:Close device
        Dim nRet As Int32 = CCamera.MV_OK
        nRet = MyCamera.CloseDevice()
        ' ch:销毁句柄 | en:Destroy handle
        MyCamera.DestroyDevice()
    End Sub
End Class