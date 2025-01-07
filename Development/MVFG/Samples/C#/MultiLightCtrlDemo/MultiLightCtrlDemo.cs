/*
This example shows the method how to reconstruct images from the device that support "MultiLightControl" feature.
[PS] Sample programs that need to save files need to be executed with administrator privileges \
     in some environments, otherwise there will be exceptions
     Sample programs currently support cameralink, xof interfaces and devices.
*/
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using MvFGCtrlC.NET;
using System.Runtime.InteropServices;
using System.Threading;
using System.IO;

using System.Drawing.Imaging;
using System.Diagnostics;
using System.Collections.ObjectModel;

namespace BasicDemo
{
    public partial class Form1 : Form
    {
        // ch:判断用户自定义像素格式 | en:Determine custom pixel format
        public const Int32 CUSTOMER_PIXEL_FORMAT = unchecked((Int32)0x80000000);

        public const UInt32 MAX_EXPOSURE_NUM = 4;   // 分时频闪的最大灯数

        public const UInt32 TRIGGER_MODE_ON  = 1;   // ch:触发模式开 | en:Trigger mode on
        public const UInt32 TRIGGER_MODE_OFF = 0;   // ch:触发模式关 | en:Trigger mode off

        CSystem     m_cSystem = new CSystem();            // ch:操作采集卡 | en:Interface operations
        CInterface  m_cInterface = null;                  // ch:操作采集卡和设备 | en:Interface and device operation
        CDevice     m_cDevice = null;                     // ch:操作设备和流 | en:Device and stream operation
        CStream     m_cStream = null;                     // ch:操作流和缓存 | en:Stream and buffer operation

        uint m_nInterfaceNum = 0;                   // ch:采集卡数量 | en:Interface number
        bool m_bIsIFOpen = false;                   // ch:采集卡是否打开 | en:Whether to open interface
        bool m_bIsDeviceOpen = false;               // ch:设备是否打开 | en:Whether to open device
        bool m_bIsGrabbing = false;                 // ch:是否在抓图 | en:Whether to start grabbing
        uint m_nTriggerMode = TRIGGER_MODE_OFF;     // ch:触发模式 | en:Trigger Mode
        uint m_nLineNum = 1;                        // ch:线数 | en:Line Number

        bool    m_bThreadState = false;        // ch:线程状态 | en:Thread state
        Thread  m_hGrabThread = null;          // ch:取流线程 | en:Grabbing thread

        delegate void ShowDisplayError(int nRet);

        public Form1()
        {
            InitializeComponent();
            Control.CheckForIllegalCrossThreadCalls = false;

            EnableControls();
        }

        private void EnableControls()
        {
            this.btnEnumInterface.Enabled = !m_bIsIFOpen;
            this.btnOpenInterface.Enabled = (m_nInterfaceNum > 0) && !m_bIsIFOpen;
            this.btnCloseInterface.Enabled = m_bIsIFOpen && !m_bIsDeviceOpen;

            this.btnEnumDevice.Enabled = m_bIsIFOpen && !m_bIsDeviceOpen;
            this.btnOpenDevice.Enabled = m_bIsIFOpen && (m_nInterfaceNum > 0) && !m_bIsDeviceOpen;
            this.btnCloseDevice.Enabled = m_bIsIFOpen && m_bIsDeviceOpen;

            this.bnContinuesMode.Enabled = m_bIsDeviceOpen;
            this.bnTriggerMode.Enabled = m_bIsDeviceOpen;

            this.btnStartGrab.Enabled = m_bIsDeviceOpen && !m_bIsGrabbing;
            this.btnStopGrab.Enabled = m_bIsDeviceOpen && m_bIsGrabbing;

            this.cbSoftTrigger.Enabled = m_bIsDeviceOpen && (m_nTriggerMode == TRIGGER_MODE_ON);
            this.btnTriggerExec.Enabled = this.cbSoftTrigger.Checked && this.bnTriggerMode.Checked && m_bIsGrabbing;
            this.comboBoxMultiLightControl.Enabled = m_bIsDeviceOpen && !m_bIsGrabbing;
        }

        // ch:关闭设备 | en:Close Device
        private void CloseDevice()
        {
            if (m_bThreadState)
            {
                m_bThreadState = false;
                m_hGrabThread.Join();
            }

            int nRet = 0;

            if (m_bIsGrabbing)
            {
                // ch:停止取流 | en:Stop Acquisition
                nRet = m_cStream.StopAcquisition();
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    MessageBox.Show("Stop acquistion failed, ErrorCode:" + nRet.ToString("X"));
                }
                m_bIsGrabbing = false;
            }

            if (null != m_cStream)
            {
                // ch:关闭流通道 | en:Close stream channel
                nRet = m_cStream.CloseStream();
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    MessageBox.Show("Close stream channel failed, ErrorCode:" + nRet.ToString("X"));
                }
                m_cStream = null;
            }

            if (null != m_cDevice)
            {
                // ch:关闭设备 | en:Close device
                nRet = m_cDevice.CloseDevice();
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    MessageBox.Show("Close device failed, ErrorCode:" + nRet.ToString("X"));
                }
                m_cDevice = null;
            }

            m_bIsDeviceOpen = false;
        }

        private void btnEnumInterface_Click(object sender, EventArgs e)
        {
            int  nRet = 0;
            bool bChanged = false;

            // ch:枚举采集卡 | en:Enum interface
            nRet = m_cSystem.UpdateInterfaceList(
                CParamDefine.MV_FG_CAMERALINK_INTERFACE | CParamDefine.MV_FG_XoF_INTERFACE,
                ref bChanged);
            if (CErrorCode.MV_FG_SUCCESS != nRet)
            {
                MessageBox.Show("Enum Interface failed, ErrorCode:" + nRet.ToString("X"));
                return;
            }
            m_nInterfaceNum = 0;

            // ch:获取采集卡数量 | en:Get interface num
            nRet = m_cSystem.GetNumInterfaces(ref m_nInterfaceNum);
            if (CErrorCode.MV_FG_SUCCESS != nRet)
            {
                MessageBox.Show("Get interface number failed, ErrorCode:" + nRet.ToString("X"));
                EnableControls();
                return;
            }
            if (0 == m_nInterfaceNum)
            {
                MessageBox.Show("No interface found");
                EnableControls();
                return;
            }

            if (bChanged)
            {
                cmbInterfaceList.Items.Clear();
                this.cmbInterfaceList.ResetText();

                // ch:向下拉框添加采集卡信息 | en:Add interface info in Combo
                MV_FG_INTERFACE_INFO stIfInfo = new MV_FG_INTERFACE_INFO();
                for (uint i = 0; i < m_nInterfaceNum; i++)
                {
                    // ch:获取采集卡信息 | en:Get interface info
                    nRet = m_cSystem.GetInterfaceInfo(i, ref stIfInfo);
                    if (CErrorCode.MV_FG_SUCCESS != nRet)
                    {
                        cmbInterfaceList.Items.Clear();
                        this.cmbInterfaceList.ResetText();
                        MessageBox.Show("Get interface info failed, ErrorCode:" + nRet.ToString("X"));
                        return;
                    }

                    string strShowIfInfo = null;
                    switch (stIfInfo.nTLayerType)
                    {
                        // Maybe support CXP or GEV interfaces and devices in the future, so reserve these codes
                        case CParamDefine.MV_FG_GEV_INTERFACE:
                            {
                                MV_GEV_INTERFACE_INFO stGevIFInfo = (MV_GEV_INTERFACE_INFO)CAdditional.ByteToStruct(
                                    stIfInfo.SpecialInfo.stGevIfInfo, typeof(MV_GEV_INTERFACE_INFO));
                                strShowIfInfo += "GEV[" + i.ToString() + "]: " + stGevIFInfo.chDisplayName + " | " +
                                    stGevIFInfo.chInterfaceID + " | " + stGevIFInfo.chSerialNumber;
                                break;
                            }
                        case CParamDefine.MV_FG_CXP_INTERFACE:
                            {
                                MV_CXP_INTERFACE_INFO stCxpIFInfo = (MV_CXP_INTERFACE_INFO)CAdditional.ByteToStruct(
                                    stIfInfo.SpecialInfo.stCXPIfInfo, typeof(MV_CXP_INTERFACE_INFO));
                                strShowIfInfo += "CXP[" + i.ToString() + "]: " + stCxpIFInfo.chDisplayName + " | " +
                                    stCxpIFInfo.chInterfaceID + " | " + stCxpIFInfo.chSerialNumber;
                                break;
                            }
                        case CParamDefine.MV_FG_CAMERALINK_INTERFACE:
                            {
                                MV_CML_INTERFACE_INFO stCmlIFInfo = (MV_CML_INTERFACE_INFO)CAdditional.ByteToStruct(
                                    stIfInfo.SpecialInfo.stCMLIfInfo, typeof(MV_CML_INTERFACE_INFO));
                                strShowIfInfo += "CML[" + i.ToString() + "]: " + stCmlIFInfo.chDisplayName + " | " +
                                    stCmlIFInfo.chInterfaceID + " | " + stCmlIFInfo.chSerialNumber;
                                break;
                            }
                        case CParamDefine.MV_FG_XoF_INTERFACE:
                            {
                                MV_XoF_INTERFACE_INFO stXoFIFInfo = (MV_XoF_INTERFACE_INFO)CAdditional.ByteToStruct(
                                    stIfInfo.SpecialInfo.stXoFIfInfo, typeof(MV_XoF_INTERFACE_INFO));
                                strShowIfInfo += "XoF[" + i.ToString() + "]: " + stXoFIFInfo.chDisplayName + " | " +
                                    stXoFIFInfo.chInterfaceID + " | " + stXoFIFInfo.chSerialNumber;
                                break;
                            }
                        default:
                            {
                                strShowIfInfo += "Unknown interface[" + i.ToString() + "]";
                                break;
                            }
                    }
                    this.cmbInterfaceList.Items.Add(strShowIfInfo);
                }
            }

            if (m_nInterfaceNum > 0)
            {
                this.cmbInterfaceList.SelectedIndex = 0;
            }

            EnableControls();
        }

        private void btnOpenInterface_Click(object sender, EventArgs e)
        {
            if (this.cmbInterfaceList.Items.Count <= 0 || this.cmbInterfaceList.SelectedIndex < 0)
            {
                MessageBox.Show("No interface");
                return;
            }

            // 如果已经打开，则先关闭采集卡
            if (null != m_cInterface)
            {
                m_cInterface.CloseInterface();
                m_cInterface = null;
            }

            // ch:打开采集卡，获得采集卡句柄 | en:Open interface, get handle
            int nRet = m_cSystem.OpenInterface(Convert.ToUInt32(this.cmbInterfaceList.SelectedIndex), out m_cInterface);
            if (CErrorCode.MV_FG_SUCCESS != nRet)
            {
                MessageBox.Show("Open Interface failed, ErrorCode:" + nRet.ToString("X"));
                return;
            }

            m_bIsIFOpen = true;
            EnableControls();
        }

        private void btnCloseInterface_Click(object sender, EventArgs e)
        {
            if (null == m_cInterface)
            {
                return;
            }

            if (m_bIsDeviceOpen)
            {
                btnCloseDevice_Click(sender, e);
            }

            // ch:关闭采集卡 | en:Close interface
            int nRet = m_cInterface.CloseInterface();
            if (CErrorCode.MV_FG_SUCCESS != nRet)
            {
                MessageBox.Show("Close interface failed, ErrorCode:" + nRet.ToString("X"));
                return;
            }
            m_cInterface = null;
            m_bIsIFOpen = false;

            EnableControls();
        }

        private void btnEnumDevice_Click(object sender, EventArgs e)
        {
            int  nRet = 0;
            bool bChanged = false;
            uint nDeviceNum = 0;

            // ch:枚举采集卡上的相机 | en:Enum camera of interface
            nRet = m_cInterface.UpdateDeviceList(ref bChanged);
            if (CErrorCode.MV_FG_SUCCESS != nRet)
            {
                MessageBox.Show("Update device list failed, ErrorCode:" + nRet.ToString("X"));
                return;
            }

            // ch:获取设备数量 | en:Get device number
            nRet = m_cInterface.GetNumDevices(ref nDeviceNum);
            if (CErrorCode.MV_FG_SUCCESS != nRet)
            {
                MessageBox.Show("Get devices number failed, ErrorCode:" + nRet.ToString("X"));
                EnableControls();
                return;
            }
            if (0 == nDeviceNum)
            {
                MessageBox.Show("No Device found");
                EnableControls();
                return;
            }

            if (bChanged)
            {
                cmbDeviceList.Items.Clear();
                this.cmbDeviceList.ResetText();

                // ch:向下拉框添加设备信息 | en:Add device info in Combo
                MV_FG_DEVICE_INFO stDeviceInfo = new MV_FG_DEVICE_INFO();
                for (uint i = 0; i < nDeviceNum; i++)
                {
                    // ch:获取设备信息 | en:Get device info
                    nRet = m_cInterface.GetDeviceInfo(i, ref stDeviceInfo);
                    if (CErrorCode.MV_FG_SUCCESS != nRet)
                    {
                        cmbDeviceList.Items.Clear();
                        this.cmbDeviceList.ResetText();
                        MessageBox.Show("Get device info failed, ErrorCode:" + nRet.ToString("X"));
                        return;
                    }

                    string strShowDevInfo = null;
                    switch (stDeviceInfo.nDevType)
                    {
                        // Maybe support CXP or GEV interfaces and devices in the future, so reserve these codes
                        case CParamDefine.MV_FG_GEV_DEVICE:
                            {
                                MV_GEV_DEVICE_INFO stGevDevInfo = (MV_GEV_DEVICE_INFO)CAdditional.ByteToStruct(
                                    stDeviceInfo.DevInfo.stGEVDevInfo, typeof(MV_GEV_DEVICE_INFO));
                                strShowDevInfo += "GEV[" + i.ToString() + "]: " + stGevDevInfo.chUserDefinedName + " | " +
                                    stGevDevInfo.chModelName + " | " + stGevDevInfo.chSerialNumber;
                                break;
                            }
                        case CParamDefine.MV_FG_CXP_DEVICE:
                            {
                                MV_CXP_DEVICE_INFO stCxpDevInfo = (MV_CXP_DEVICE_INFO)CAdditional.ByteToStruct(
                                    stDeviceInfo.DevInfo.stCXPDevInfo, typeof(MV_CXP_DEVICE_INFO));
                                strShowDevInfo += "CXP[" + i.ToString() + "]: " + stCxpDevInfo.chUserDefinedName + " | " +
                                    stCxpDevInfo.chModelName + " | " + stCxpDevInfo.chSerialNumber;
                                break;
                            }
                        case CParamDefine.MV_FG_CAMERALINK_DEVICE:
                            {
                                MV_CML_DEVICE_INFO stCmlDevInfo = (MV_CML_DEVICE_INFO)CAdditional.ByteToStruct(
                                    stDeviceInfo.DevInfo.stCMLDevInfo, typeof(MV_CML_DEVICE_INFO));
                                strShowDevInfo += "CML[" + i.ToString() + "]: " + stCmlDevInfo.chUserDefinedName + " | " +
                                    stCmlDevInfo.chModelName + " | " + stCmlDevInfo.chSerialNumber;
                                break;
                            }
                        case CParamDefine.MV_FG_XoF_DEVICE:
                            {
                                MV_XoF_DEVICE_INFO stXoFDevInfo = (MV_XoF_DEVICE_INFO)CAdditional.ByteToStruct(
                                    stDeviceInfo.DevInfo.stXoFDevInfo, typeof(MV_XoF_DEVICE_INFO));
                                strShowDevInfo += "XoF[" + i.ToString() + "]: " + stXoFDevInfo.chUserDefinedName + " | " +
                                    stXoFDevInfo.chModelName + " | " + stXoFDevInfo.chSerialNumber;
                                break;
                            }
                        default:
                            {
                                strShowDevInfo += "Unknown device[" + i.ToString() + "]";
                                break;
                            }
                    }
                    this.cmbDeviceList.Items.Add(strShowDevInfo);
                }
            }

            if (nDeviceNum > 0)
            {
                this.cmbDeviceList.SelectedIndex = 0;
            }

            EnableControls();
        }

        private void GetMultiLightControl()
        {
            if (null == m_cDevice)
            {
                return;
            }

            int nRet = CErrorCode.MV_FG_SUCCESS;

            MV_FG_ENUMVALUE stEnumValue = new MV_FG_ENUMVALUE();
            CParam cParam = new CParam(m_cDevice);
            this.comboBoxMultiLightControl.Items.Clear();
            nRet = cParam.GetEnumValue("MultiLightControl", ref stEnumValue);
            if (CErrorCode.MV_FG_SUCCESS == nRet)
            {
                for (int i = 0; i < stEnumValue.nSupportedNum; i++)
                {
                    this.comboBoxMultiLightControl.Items.Add(stEnumValue.strSymbolicArray[i].strInfo);
                }
                this.comboBoxMultiLightControl.Text = stEnumValue.strCurSymbolic;
                m_nLineNum = stEnumValue.nCurValue & 0xF;
                if (0 == m_nLineNum)
                {
                    m_nLineNum = 1;
                }
            }
            else
            {
                MessageBox.Show("Not support Multi Light Control, ErrorCode:" + nRet.ToString("X"));
                m_nLineNum = 1;
            }
        }
        private void btnOpenDevice_Click(object sender, EventArgs e)
        {
            if (this.cmbDeviceList.Items.Count <= 0 || this.cmbDeviceList.SelectedIndex < 0)
            {
                MessageBox.Show("No device");
                return;
            }

            // 如果已经打开，则先关闭通道
            if (null != m_cStream)
            {
                m_cStream.CloseStream();
                m_cStream = null;
            }

            // 如果已经打开，则先关闭设备
            if (null != m_cDevice)
            {
                m_cDevice.CloseDevice();
                m_cDevice = null;
            }

            // ch:打开设备，获得设备句柄 | en:Open device, get handle
            int nRet = m_cInterface.OpenDevice(Convert.ToUInt32(this.cmbDeviceList.SelectedIndex), out m_cDevice);
            if (CErrorCode.MV_FG_SUCCESS != nRet)
            {
                MessageBox.Show("Open device failed, ErrorCode:" + nRet.ToString("X"));
                return;
            }

            GetMultiLightControl();

            m_bIsDeviceOpen = true;

            // ch:设置连续采集模式 | en:Set Continuous Aquisition Mode
            CParam cDeviceParam = new CParam(m_cDevice);
            cDeviceParam.SetEnumValue("AcquisitionMode", 2);  // 0 - SingleFrame, 2 - Continuous
            cDeviceParam.SetEnumValue("TriggerMode", TRIGGER_MODE_OFF);

            bnContinuesMode.Checked = true;
            bnTriggerMode.Checked = false;
            m_nTriggerMode = TRIGGER_MODE_OFF;

            EnableControls();
        }

        private void btnCloseDevice_Click(object sender, EventArgs e)
        {
            CloseDevice();

            EnableControls();
        }

        private void bnContinuesMode_CheckedChanged(object sender, EventArgs e)
        {
            if (bnContinuesMode.Checked)
            {
                CParam cDeviceParam = new CParam(m_cDevice);

                // ch:关闭触发模式 | en:Turn off Trigger Mode
                int nRet = cDeviceParam.SetEnumValue("TriggerMode", TRIGGER_MODE_OFF);
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    MessageBox.Show("Turn off trigger mode failed, ErrorCode:" + nRet.ToString("X"));
                    return;
                }
                m_nTriggerMode = TRIGGER_MODE_OFF;

                EnableControls();
            }
        }

        private void bnTriggerMode_CheckedChanged(object sender, EventArgs e)
        {
            if (bnTriggerMode.Checked)
            {
                CParam cDeviceParam = new CParam(m_cDevice);

                // ch:打开触发模式 | en:Open Trigger Mode
                int nRet = cDeviceParam.SetEnumValue("TriggerMode", TRIGGER_MODE_ON);
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    MessageBox.Show("Turn on trigger mode failed, ErrorCode:" + nRet.ToString("X"));
                    return;
                }
                m_nTriggerMode = TRIGGER_MODE_ON;

                // ch:触发源选择:0 - Line0; | en:Trigger source select:0 - Line0;
                //           1 - Line1;
                //           2 - Line2;
                //           3 - Line3;
                //           4 - Counter;
                //           7 - Software;
                if (cbSoftTrigger.Checked)
                {
                    cDeviceParam.SetEnumValue("TriggerSource", (uint)7);
                }
                else
                {
                    cDeviceParam.SetEnumValue("TriggerSource", (uint)0);
                }

                EnableControls();
            }
        }

        private void btnStartGrab_Click(object sender, EventArgs e)
        {
            if (!m_bIsDeviceOpen)
            {
                MessageBox.Show("Please open device first");
                return;
            }

            if (m_bIsGrabbing)
            {
                return;
            }

            // ch:获取流通道个数 | en:Get number of stream
            uint nStreamNum = 0;
            int nRet = m_cDevice.GetNumStreams(ref nStreamNum);
            if (CErrorCode.MV_FG_SUCCESS != nRet)
            {
                MessageBox.Show("Get streams number failed, ErrorCode:" + nRet.ToString("X"));
                return;
            }
            if (0 == nStreamNum)
            {
                MessageBox.Show("No valid stream channel");
                return;
            }

            // ch:打开流通道(目前只支持单个通道) | en:Open stream(Only a single stream is supported now)
            nRet = m_cDevice.OpenStream(0, out m_cStream);
            if (CErrorCode.MV_FG_SUCCESS != nRet)
            {
                MessageBox.Show("Open stream failed, ErrorCode:" + nRet.ToString("X"));
                return;
            }

            // ch:设置SDK内部缓存数量 | en:Set internal buffer number
            const uint nBufNum = 5;
            nRet = m_cStream.SetBufferNum(nBufNum);
            if (CErrorCode.MV_FG_SUCCESS != nRet)
            {
                MessageBox.Show("Set buffer number failed, ErrorCode:" + nRet.ToString("X"));
                return;
            }

            // ch:创建取流线程 | en:Create acquistion thread
            m_bThreadState = true;
            m_hGrabThread = new Thread(ReceiveThreadProcess);
            if (null == m_hGrabThread)
            {
                m_bThreadState = false;
                MessageBox.Show("Create thread failed");
                return;
            }
            m_hGrabThread.Start();

            // ch:开始取流 | en:Start Acquisition
            nRet = m_cStream.StartAcquisition();
            if (CErrorCode.MV_FG_SUCCESS != nRet)
            {
                m_bThreadState = false;
                MessageBox.Show("Start acquistion failed, ErrorCode:" + nRet.ToString("X"));
                return;
            }
            m_bIsGrabbing = true;

            EnableControls();
        }

        private void btnStopGrab_Click(object sender, EventArgs e)
        {
            if (false == m_bIsDeviceOpen || false == m_bIsGrabbing)
            {
                return;
            }

            // ch:标志位设为false | en:Set flag bit false
            m_bThreadState = false;
            m_hGrabThread.Join();

            // ch:停止取流 | en:Stop Acquisition
            int nRet = m_cStream.StopAcquisition();
            if (CErrorCode.MV_FG_SUCCESS != nRet)
            {
                MessageBox.Show("Stop acquistion failed, ErrorCode:" + nRet.ToString("X"));
                return;
            }
            m_bIsGrabbing = false;

            // ch:关闭流通道 | en:Close stream channel
            nRet = m_cStream.CloseStream();
            if (CErrorCode.MV_FG_SUCCESS != nRet)
            {
                MessageBox.Show("Close stream channel failed, ErrorCode:" + nRet.ToString("X"));
            }
            m_cStream = null;

            EnableControls();
        }

        private void DisplayError(int nRet)
        {
            MessageBox.Show("Display failed, ErrorCode:" + nRet.ToString("X"));

            btnStopGrab_Click(this, new EventArgs());
        }

        public void ReceiveThreadProcess()
        {
            CImageProcess cImgProc = new CImageProcess(m_cStream);
            MV_FG_BUFFER_INFO stFrameInfo = new MV_FG_BUFFER_INFO();              // ch:图像信息 | en:Frame info
            MV_FG_INPUT_IMAGE_INFO stDisplayInfo = new MV_FG_INPUT_IMAGE_INFO();       // ch:显示的图像信息 | en:Display frame info
            MV_FG_RECONSTRUCT_INFO stReconstructInfo = new MV_FG_RECONSTRUCT_INFO(0);   // ch:图像重组信息 | en:Image reconstruction info
            Byte[][] pImageBufferList = new byte[MAX_EXPOSURE_NUM][];
            UInt32 nImageBufferSize = 0;

            const uint nTimeout = 1000;
            int nRet = 0;

            IntPtr[] ptrPictureBoxHandle = new IntPtr[MAX_EXPOSURE_NUM];
            ptrPictureBoxHandle[0] = pictureBox1.Handle;
            ptrPictureBoxHandle[1] = pictureBox2.Handle;
            ptrPictureBoxHandle[2] = pictureBox3.Handle;
            ptrPictureBoxHandle[3] = pictureBox4.Handle;

            while (m_bThreadState)
            {
                if (m_bIsGrabbing)
                {
                    // ch:获取一帧图像缓存信息 | en:Get one frame buffer's info
                    nRet = m_cStream.GetFrameBuffer(ref stFrameInfo, nTimeout);
                    if (CErrorCode.MV_FG_SUCCESS == nRet)
                    {
                        if (RemoveCustomPixelFormats(stFrameInfo.enPixelType))
                        {
                            m_cStream.ReleaseFrameBuffer(stFrameInfo);
                            continue;
                        }

                        // ch:开始图像重组 | en:Start image reconstruct
                        if (m_nLineNum != 1)
                        {
                            // ch:如果图像大小由小变大需要重新分配缓存 | en:Adjust buffer size while image size has changed
                            uint nNeedSize = stFrameInfo.nFilledSize / m_nLineNum;
                            if (nImageBufferSize < nNeedSize)
                            {
                                for (int i = 0; i < m_nLineNum; i++)
                                {
                                    pImageBufferList[i] = null;

                                    pImageBufferList[i] = new byte[nNeedSize];
                                    if (null == pImageBufferList[i])
                                    {
                                        break;
                                    }

                                    stReconstructInfo.stOutputImageInfo[i].pImageBuf = Marshal.UnsafeAddrOfPinnedArrayElement(pImageBufferList[i], 0);
                                    stReconstructInfo.stOutputImageInfo[i].nImageBufSize = nNeedSize;
                                }
                                nImageBufferSize = nNeedSize;
                            }

                            stReconstructInfo.stInputImageInfo.nWidth = stFrameInfo.nWidth;
                            stReconstructInfo.stInputImageInfo.nHeight = stFrameInfo.nHeight;
                            stReconstructInfo.stInputImageInfo.enPixelType = stFrameInfo.enPixelType;
                            stReconstructInfo.stInputImageInfo.pImageBuf = stFrameInfo.pBuffer;
                            stReconstructInfo.stInputImageInfo.nImageBufLen = stFrameInfo.nFilledSize;
                            if (2 == m_nLineNum)
                            {
                                stReconstructInfo.enReconstructMode = MV_FG_RECONSTRUCT_MODE.RECONSTRUCT_MODE_SPLIT_BY_LINE_2;
                            }
                            else if (3 == m_nLineNum)
                            {
                                stReconstructInfo.enReconstructMode = MV_FG_RECONSTRUCT_MODE.RECONSTRUCT_MODE_SPLIT_BY_LINE_3;
                            }
                            else if (4 == m_nLineNum)
                            {
                                stReconstructInfo.enReconstructMode = MV_FG_RECONSTRUCT_MODE.RECONSTRUCT_MODE_SPLIT_BY_LINE_4;
                            }

                            nRet = cImgProc.ReconstructImage(ref stReconstructInfo);
                            if (CErrorCode.MV_FG_SUCCESS != nRet)
                            {
                                m_cStream.ReleaseFrameBuffer(stFrameInfo);
                                continue;
                            }

                            for (int i = 0; i < m_nLineNum; i++)
                            {
                                // 配置显示图像的参数
                                stDisplayInfo.nWidth = stReconstructInfo.stOutputImageInfo[i].nWidth;
                                stDisplayInfo.nHeight = stReconstructInfo.stOutputImageInfo[i].nHeight;
                                stDisplayInfo.enPixelType = stReconstructInfo.stOutputImageInfo[i].enPixelType;
                                stDisplayInfo.pImageBuf = stReconstructInfo.stOutputImageInfo[i].pImageBuf;
                                stDisplayInfo.nImageBufLen = stReconstructInfo.stOutputImageInfo[i].nImageBufLen;
                                nRet = cImgProc.DisplayOneFrame(ptrPictureBoxHandle[i], ref stDisplayInfo);
                                if (CErrorCode.MV_FG_SUCCESS != nRet)
                                {
                                    m_cStream.ReleaseFrameBuffer(stFrameInfo);
                                    this.Invoke(new ShowDisplayError(DisplayError), new object[] { nRet });
                                    break;
                                }
                            }
                        }
                        else
                        {
                            // 配置显示图像的参数
                            stDisplayInfo.nWidth = stFrameInfo.nWidth;
                            stDisplayInfo.nHeight = stFrameInfo.nHeight;
                            stDisplayInfo.enPixelType = stFrameInfo.enPixelType;
                            stDisplayInfo.pImageBuf = stFrameInfo.pBuffer;
                            stDisplayInfo.nImageBufLen = stFrameInfo.nFilledSize;
                            nRet = cImgProc.DisplayOneFrame(pictureBox1.Handle, ref stDisplayInfo);
                            if (CErrorCode.MV_FG_SUCCESS != nRet)
                            {
                                m_cStream.ReleaseFrameBuffer(stFrameInfo);
                                this.Invoke(new ShowDisplayError(DisplayError), new object[] { nRet });
                                break;
                            }
                        }

                        m_cStream.ReleaseFrameBuffer(stFrameInfo);
                    }
                    else
                    {
                        if (TRIGGER_MODE_ON == m_nTriggerMode)
                        {
                            Thread.Sleep(5);
                        }
                    }
                }
                else
                {
                    Thread.Sleep(5);
                }
            }
        }

        private void cbSoftTrigger_CheckedChanged(object sender, EventArgs e)
        {
            CParam cDeviceParam = new CParam(m_cDevice);

            // ch:触发源设置 | en:Set trigger source
            // ch:触发源选择:0 - Line0; | en:Trigger source select:0 - Line0;
            //           1 - Line1;
            //           2 - Line2;
            //           3 - Line3;
            //           4 - Counter;
            //           7 - Software;
            if (cbSoftTrigger.Checked)
            {
                int nRet = cDeviceParam.SetEnumValue("TriggerSource", (uint)7);
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    MessageBox.Show("Set software trigger source failed, ErrorCode:" + nRet.ToString("X"));
                    return;
                }
            }
            else
            {
                int nRet = cDeviceParam.SetEnumValue("TriggerSource", (uint)0);
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    MessageBox.Show("Set trigger source failed, ErrorCode:" + nRet.ToString("X"));
                    return;
                }
            }

            EnableControls();
        }

        private void bnTriggerExec_Click(object sender, EventArgs e)
        {
            if (true != m_bIsGrabbing)
            {
                return;
            }

            CParam cDeviceParam = new CParam(m_cDevice);

            // ch:触发命令 | en:Trigger command
            int nRet = cDeviceParam.SetCommandValue("TriggerSoftware");
            if (CErrorCode.MV_FG_SUCCESS != nRet)
            {
                MessageBox.Show("Trigger software failed, ErrorCode:" + nRet.ToString("X"));
            }
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            btnCloseDevice_Click(sender, e);
            btnCloseInterface_Click(sender, e);
        }

        // ch:去除自定义的像素格式 | en:Remove custom pixel formats
        private bool RemoveCustomPixelFormats(MV_FG_PIXEL_TYPE enPixelFormat)
        {
            Int32 nResult = ((int)enPixelFormat) & CUSTOMER_PIXEL_FORMAT;
            if (CUSTOMER_PIXEL_FORMAT == nResult)
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        private void ParamsChanged(object sender, EventArgs e)
        {
            int nRet = CErrorCode.MV_FG_SUCCESS;
            string strPrefixComboBox = "comboBox";
            CParam cParam = new CParam(m_cDevice);
            Control control = sender as Control;
            string strControlName = control.Name;
            string strNodeName = strControlName.Substring(strPrefixComboBox.Length);
            string strValue = control.Text;
            nRet = cParam.SetEnumValueByString(strNodeName, strValue);

            if (null == strNodeName)
            {
                return;
            }

            if (CErrorCode.MV_FG_SUCCESS != nRet)
            {
                MessageBox.Show("Not support Multi Light Control, ErrorCode:" + nRet.ToString("X"));
                return;
            }
            else
            {
                GetMultiLightControl();
            }
        }
    }
}
