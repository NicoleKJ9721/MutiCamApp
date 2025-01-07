/*
This example shows the user the basic functionality of the MVFGControl method.
This example covers enumeration module, control module and stream module.
This is also an example of how to save images.
[PS] Sample programs currently support cameralink, cxp, xof interfaces and devices.
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
        bool m_bLineInputPolarity = false;          // ch:是否拥有线路输入极性节点 | en:Whether to have LineInputPolarity node
        bool m_bCCSelector = false;                 // ch:是否拥有相机控制选择节点 | en:Whether to have CCSelector node
        bool m_bCCSource = false;                   // ch:是否拥有相机控制源节点 | en:Whether to have CCSource node
        bool m_bScanMode = false;                   // ch:是否拥有扫描模式节点 | en:Whether to have ScanMode node
        bool m_bTriggerActivation = false;          // ch:是否拥有触发极性节点 | en:Whether to have TriggerActivation node
        uint m_nTriggerMode = TRIGGER_MODE_OFF;     // ch:触发模式 | en:Trigger Mode

        bool    m_bThreadState = false;        // ch:线程状态 | en:Thread state
        Thread  m_hGrabThread = null;          // ch:取流线程 | en:Grabbing thread

        delegate void ShowDisplayError(int nRet);

        public Form1()
        {
            InitializeComponent();
            Control.CheckForIllegalCrossThreadCalls = false;

            EnableControls();
        }

        private void EnableIFControls()
        {
            this.groupBoxInterfaceParams.Enabled = m_bIsIFOpen;

            if (m_bIsIFOpen)
            {
                this.groupBoxInterfaceParams.Enabled = m_bIsIFOpen;

                this.textBoxImageHeight.Enabled = (this.comboBoxCameraType.Text == "LineScan" && !m_bIsGrabbing);
                this.comboBoxLineMode.Enabled = m_bIsIFOpen;
                this.comboBoxLineInputPolarity.Enabled = (this.comboBoxLineMode.Text == "Input") && m_bLineInputPolarity;
                this.comboBoxEncoderSelector.Enabled = m_bIsIFOpen;
                this.comboBoxEncoderSourceA.Enabled = m_bIsIFOpen;
                this.comboBoxEncoderSourceB.Enabled = m_bIsIFOpen;
                this.comboBoxEncoderTrigger.Enabled = m_bIsIFOpen;
                this.comboBoxCCSelector.Enabled = m_bIsIFOpen && m_bCCSelector;
                this.comboBoxCCSource.Enabled = m_bIsIFOpen && m_bCCSource;
            }
        }

        private void EnableDevControls()
        {
            this.groupBoxDeviceParams.Enabled = m_bIsDeviceOpen;

            if (m_bIsDeviceOpen)
            {
                string strTriggerMode = this.comboBoxTriggerMode.Text;
                string strTriggerSource = this.comboBoxTriggerSource.Text;
                this.btnTriggerExec.Enabled = ((strTriggerMode == "On") && (strTriggerSource == "Software"));
                if (m_bScanMode)
                {
                    this.textBoxHeight.Enabled = (this.comboBoxScanMode.Text == "FrameScan" && !m_bIsGrabbing);
                }
                else
                {
                    this.textBoxHeight.Enabled = !m_bIsGrabbing;
                }
                this.textBoxWidth.Enabled = !m_bIsGrabbing;
                this.comboBoxScanMode.Enabled = !m_bIsGrabbing && m_bScanMode;
                this.comboBoxTriggerActivation.Enabled = m_bTriggerActivation;
                this.comboBoxPixelFormat.Enabled = !m_bIsGrabbing;
            }
        }

        private void EnableControls()
        {
            this.btnEnumInterface.Enabled = !m_bIsIFOpen;
            this.btnOpenInterface.Enabled = (m_nInterfaceNum > 0) && !m_bIsIFOpen;
            this.btnCloseInterface.Enabled = m_bIsIFOpen && !m_bIsDeviceOpen;

            this.btnEnumDevice.Enabled = m_bIsIFOpen && !m_bIsDeviceOpen;
            this.btnOpenDevice.Enabled = m_bIsIFOpen && (m_nInterfaceNum > 0) && !m_bIsDeviceOpen;
            this.btnCloseDevice.Enabled = m_bIsIFOpen && m_bIsDeviceOpen;

            this.btnStartGrab.Enabled = m_bIsDeviceOpen && !m_bIsGrabbing;
            this.btnStopGrab.Enabled = m_bIsDeviceOpen && m_bIsGrabbing;

            this.btnTriggerExec.Enabled = m_bIsGrabbing;

            EnableIFControls();

            EnableDevControls();
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
            nRet = m_cSystem.UpdateInterfaceList(CParamDefine.MV_FG_CAMERALINK_INTERFACE | CParamDefine.MV_FG_CXP_INTERFACE | CParamDefine.MV_FG_XoF_INTERFACE, ref bChanged);
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
                        // Maybe support GEV interfaces and devices in the future, so reserve these codes
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

        private void GetInterfaceParams()
        {
            if (null == m_cInterface)
            {
                return;
            }

            int nRet = CErrorCode.MV_FG_SUCCESS;

            CParam cParam = new CParam(m_cInterface);
            MV_FG_ENUMVALUE stEnumValue = new MV_FG_ENUMVALUE();
            MV_FG_INTVALUE stIntValue = new MV_FG_INTVALUE();

            this.comboBoxCameraType.Items.Clear();
            nRet = cParam.GetEnumValue("CameraType", ref stEnumValue);
            if (CErrorCode.MV_FG_SUCCESS == nRet)
            {
                for (int i = 0; i < stEnumValue.nSupportedNum; i++)
                {
                    this.comboBoxCameraType.Items.Add(stEnumValue.strSymbolicArray[i].strInfo);
                }
                this.comboBoxCameraType.Text = stEnumValue.strCurSymbolic;
            }

            nRet = cParam.GetIntValue("ImageHeight", ref stIntValue);
            if (CErrorCode.MV_FG_SUCCESS == nRet)
            {
                this.textBoxImageHeight.Text = Convert.ToString(stIntValue.nCurValue);
            }

            this.comboBoxLineSelector.Items.Clear();
            nRet = cParam.GetEnumValue("LineSelector", ref stEnumValue);
            if (CErrorCode.MV_FG_SUCCESS == nRet)
            {
                for (int i = 0; i < stEnumValue.nSupportedNum; i++)
                {
                    this.comboBoxLineSelector.Items.Add(stEnumValue.strSymbolicArray[i].strInfo);
                }
                this.comboBoxLineSelector.Text = stEnumValue.strCurSymbolic;
            }

            this.comboBoxLineMode.Items.Clear();
            nRet = cParam.GetEnumValue("LineMode", ref stEnumValue);
            if (CErrorCode.MV_FG_SUCCESS == nRet)
            {
                for (int i = 0; i < stEnumValue.nSupportedNum; i++)
                {
                    this.comboBoxLineMode.Items.Add(stEnumValue.strSymbolicArray[i].strInfo);
                }
                this.comboBoxLineMode.Text = stEnumValue.strCurSymbolic;
            }

            this.comboBoxLineInputPolarity.Items.Clear();
            nRet = cParam.GetEnumValue("LineInputPolarity", ref stEnumValue);
            if (CErrorCode.MV_FG_SUCCESS == nRet)
            {
                for (int i = 0; i < stEnumValue.nSupportedNum; i++)
                {
                    this.comboBoxLineInputPolarity.Items.Add(stEnumValue.strSymbolicArray[i].strInfo);
                }
                this.comboBoxLineInputPolarity.Text = stEnumValue.strCurSymbolic;
                m_bLineInputPolarity = true;
            }
            else
            {
                m_bLineInputPolarity = false;
            }

            this.comboBoxEncoderSelector.Items.Clear();
            nRet = cParam.GetEnumValue("EncoderSelector", ref stEnumValue);
            if (CErrorCode.MV_FG_SUCCESS == nRet)
            {
                for (int i = 0; i < stEnumValue.nSupportedNum; i++)
                {
                    this.comboBoxEncoderSelector.Items.Add(stEnumValue.strSymbolicArray[i].strInfo);
                }
                this.comboBoxEncoderSelector.Text = stEnumValue.strCurSymbolic;
            }

            this.comboBoxEncoderSourceA.Items.Clear();
            nRet = cParam.GetEnumValue("EncoderSourceA", ref stEnumValue);
            if (CErrorCode.MV_FG_SUCCESS == nRet)
            {
                for (int i = 0; i < stEnumValue.nSupportedNum; i++)
                {
                    this.comboBoxEncoderSourceA.Items.Add(stEnumValue.strSymbolicArray[i].strInfo);
                }
                this.comboBoxEncoderSourceA.Text = stEnumValue.strCurSymbolic;
            }

            this.comboBoxEncoderSourceB.Items.Clear();
            nRet = cParam.GetEnumValue("EncoderSourceB", ref stEnumValue);
            if (CErrorCode.MV_FG_SUCCESS == nRet)
            {
                for (int i = 0; i < stEnumValue.nSupportedNum; i++)
                {
                    this.comboBoxEncoderSourceB.Items.Add(stEnumValue.strSymbolicArray[i].strInfo);
                }
                this.comboBoxEncoderSourceB.Text = stEnumValue.strCurSymbolic;
            }

            this.comboBoxEncoderTrigger.Items.Clear();
            nRet = cParam.GetEnumValue("EncoderTriggerMode", ref stEnumValue);
            if (CErrorCode.MV_FG_SUCCESS == nRet)
            {
                for (int i = 0; i < stEnumValue.nSupportedNum; i++)
                {
                    this.comboBoxEncoderTrigger.Items.Add(stEnumValue.strSymbolicArray[i].strInfo);
                }
                this.comboBoxEncoderTrigger.Text = stEnumValue.strCurSymbolic;
            }

            this.comboBoxCCSelector.Items.Clear();
            nRet = cParam.GetEnumValue("CCSelector", ref stEnumValue);
            if (CErrorCode.MV_FG_SUCCESS == nRet)
            {
                for (int i = 0; i < stEnumValue.nSupportedNum; i++)
                {
                    this.comboBoxCCSelector.Items.Add(stEnumValue.strSymbolicArray[i].strInfo);
                }
                this.comboBoxCCSelector.Text = stEnumValue.strCurSymbolic;
                m_bCCSelector = true;
            }
            else
            {
                m_bCCSelector = false;
            }

            this.comboBoxCCSource.Items.Clear();
            nRet = cParam.GetEnumValue("CCSource", ref stEnumValue);
            if (CErrorCode.MV_FG_SUCCESS == nRet)
            {
                for (int i = 0; i < stEnumValue.nSupportedNum; i++)
                {
                    this.comboBoxCCSource.Items.Add(stEnumValue.strSymbolicArray[i].strInfo);
                }
                this.comboBoxCCSource.Text = stEnumValue.strCurSymbolic;
                m_bCCSource = true;
            }
            else
            {
                m_bCCSource = false;
            }
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

            GetInterfaceParams();

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

            this.cmbDeviceList.Items.Clear();

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
                        // Maybe support GEV interfaces and devices in the future, so reserve these codes
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

        private void GetDeviceParams()
        {
            if (null == m_cDevice)
            {
                return;
            }

            int nRet = CErrorCode.MV_FG_SUCCESS;

            MV_FG_ENUMVALUE stEnumValue = new MV_FG_ENUMVALUE();
            MV_FG_INTVALUE stIntValue = new MV_FG_INTVALUE();
            CParam cParam = new CParam(m_cDevice);
            nRet = cParam.GetIntValue("Width", ref stIntValue);
            if (CErrorCode.MV_FG_SUCCESS == nRet)
            {
                this.textBoxWidth.Text = Convert.ToString(stIntValue.nCurValue);
            }

            nRet = cParam.GetIntValue("Height", ref stIntValue);
            if (CErrorCode.MV_FG_SUCCESS == nRet)
            {
                this.textBoxHeight.Text = Convert.ToString(stIntValue.nCurValue);
            }

            this.comboBoxPixelFormat.Items.Clear();
            nRet = cParam.GetEnumValue("PixelFormat", ref stEnumValue);
            if (CErrorCode.MV_FG_SUCCESS == nRet)
            {
                for (int i = 0; i < stEnumValue.nSupportedNum; i++)
                {
                    this.comboBoxPixelFormat.Items.Add(stEnumValue.strSymbolicArray[i].strInfo);
                }
                this.comboBoxPixelFormat.Text = stEnumValue.strCurSymbolic;
            }

            this.comboBoxScanMode.Items.Clear();
            nRet = cParam.GetEnumValue("ScanMode", ref stEnumValue);
            if (CErrorCode.MV_FG_SUCCESS == nRet)
            {
                for (int i = 0; i < stEnumValue.nSupportedNum; i++)
                {
                    this.comboBoxScanMode.Items.Add(stEnumValue.strSymbolicArray[i].strInfo);
                }
                this.comboBoxScanMode.Text = stEnumValue.strCurSymbolic;
                m_bScanMode = true;
            }
            else
            {
                m_bScanMode = false;
            }

            this.comboBoxTriggerMode.Items.Clear();
            nRet = cParam.GetEnumValue("TriggerMode", ref stEnumValue);
            if (CErrorCode.MV_FG_SUCCESS == nRet)
            {
                for (int i = 0; i < stEnumValue.nSupportedNum; i++)
                {
                    this.comboBoxTriggerMode.Items.Add(stEnumValue.strSymbolicArray[i].strInfo);
                }
                this.comboBoxTriggerMode.Text = stEnumValue.strCurSymbolic;
            }

            this.comboBoxTriggerSource.Items.Clear();
            nRet = cParam.GetEnumValue("TriggerSource", ref stEnumValue);
            if (CErrorCode.MV_FG_SUCCESS == nRet)
            {
                for (int i = 0; i < stEnumValue.nSupportedNum; i++)
                {
                    this.comboBoxTriggerSource.Items.Add(stEnumValue.strSymbolicArray[i].strInfo);
                }
                this.comboBoxTriggerSource.Text = stEnumValue.strCurSymbolic;
            }

            this.comboBoxTriggerActivation.Items.Clear();
            nRet = cParam.GetEnumValue("TriggerActivation", ref stEnumValue);
            if (CErrorCode.MV_FG_SUCCESS == nRet)
            {
                for (int i = 0; i < stEnumValue.nSupportedNum; i++)
                {
                    this.comboBoxTriggerActivation.Items.Add(stEnumValue.strSymbolicArray[i].strInfo);
                }
                this.comboBoxTriggerActivation.Text = stEnumValue.strCurSymbolic;
                m_bTriggerActivation = true;
            }
            else
            {
                m_bTriggerActivation = false;
            }

            this.comboBoxTriggerSelector.Items.Clear();
            nRet = cParam.GetEnumValue("TriggerSelector", ref stEnumValue);
            if (CErrorCode.MV_FG_SUCCESS == nRet)
            {
                for (int i = 0; i < stEnumValue.nSupportedNum; i++)
                {
                    this.comboBoxTriggerSelector.Items.Add(stEnumValue.strSymbolicArray[i].strInfo);
                }
                this.comboBoxTriggerSelector.Text = stEnumValue.strCurSymbolic;
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

            GetDeviceParams();

            m_bIsDeviceOpen = true;
            EnableControls();
        }

        private void btnCloseDevice_Click(object sender, EventArgs e)
        {
            CloseDevice();

            EnableControls();
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
            CImageProcess           cImgProc = new CImageProcess(m_cStream);
            MV_FG_BUFFER_INFO       stFrameInfo = new MV_FG_BUFFER_INFO();          // ch:图像信息 | en:Frame info
            MV_FG_INPUT_IMAGE_INFO  stDisplayInfo = new MV_FG_INPUT_IMAGE_INFO();   // ch:显示的图像信息 | en:Display frame info
            const uint              nTimeout = 1000;
            int                     nRet = 0;

            while (m_bThreadState)
            {
                if (m_bIsGrabbing)
                {
                    // ch:获取一帧图像缓存信息 | en:Get one frame buffer's info
                    nRet = m_cStream.GetFrameBuffer(ref stFrameInfo, nTimeout);
                    if (CErrorCode.MV_FG_SUCCESS == nRet)
                    {
                        // 自定义格式不支持显示
                        if (RemoveCustomPixelFormats(stFrameInfo.enPixelType))
                        {
                            m_cStream.ReleaseFrameBuffer(stFrameInfo);
                            continue;
                        }

                        // 配置显示图像的参数
                        stDisplayInfo.nWidth        = stFrameInfo.nWidth;
                        stDisplayInfo.nHeight       = stFrameInfo.nHeight;
                        stDisplayInfo.enPixelType   = stFrameInfo.enPixelType;
                        stDisplayInfo.pImageBuf     = stFrameInfo.pBuffer;
                        stDisplayInfo.nImageBufLen  = stFrameInfo.nFilledSize;
                        nRet = cImgProc.DisplayOneFrame(pictureBox1.Handle, ref stDisplayInfo);
                        if (CErrorCode.MV_FG_SUCCESS != nRet)
                        {
                            m_cStream.ReleaseFrameBuffer(stFrameInfo);
                            this.Invoke(new ShowDisplayError(DisplayError), new object[]{nRet});
                            break;
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
            string strPrefixTextBox = "textBox";

            bool bIsInterfaceParam = true; // ch:是否是采集卡参数 | en: Does the param belongs to interface?
            CParam cParam = null;
            string strNodeName = null;
            string strValue = null;
            Int64 nValue = 0;

            Control control = sender as Control;
            string strControlName = control.Name;

            bIsInterfaceParam = (control.Parent.Name == "groupBoxInterfaceParams");
            if (bIsInterfaceParam)
            {
                cParam = new CParam(m_cInterface);
            }
            else
            {
                cParam = new CParam(m_cDevice);
            }

            if (strControlName.Contains(strPrefixComboBox))
            {
                ComboBox comboBox = sender as ComboBox;
                strNodeName =  strControlName.Substring(strPrefixComboBox.Length);
                strValue = control.Text;

                nRet = cParam.SetEnumValueByString(strNodeName, strValue);
            }
            else if (strControlName.Contains(strPrefixTextBox))
            {
                strNodeName = strControlName.Substring(strPrefixTextBox.Length);
                nValue = Convert.ToInt64(control.Text);

                nRet = cParam.SetIntValue(strNodeName, nValue);
            }

            if (null == strNodeName)
            {
                return;
            }

            if (CErrorCode.MV_FG_SUCCESS != nRet)
            {
                MessageBox.Show(String.Format("Set param {0} failed, ErrorCode:0x{1:X8}", strNodeName, nRet));
            }

            if (bIsInterfaceParam)
            {
                GetInterfaceParams();
                EnableIFControls();
            }
            else
            {
                GetDeviceParams();
                EnableDevControls();
            }

        }

    }
}
