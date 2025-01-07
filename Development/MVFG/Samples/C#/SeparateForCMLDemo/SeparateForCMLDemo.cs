/*

 * This example shows the how to grab image without communicate with camera through serial port.

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

        CSystem     m_cSystem = new CSystem();            // ch:操作采集卡 | en:Interface operations
        CInterface  m_cInterface = null;                  // ch:操作采集卡和设备 | en:Interface and device operation
        CDevice     m_cDevice = null;                     // ch:操作设备和流 | en:Device and stream operation
        CStream     m_cStream = null;                     // ch:操作流和缓存 | en:Stream and buffer operation

        uint m_nInterfaceNum = 0;                   // ch:采集卡数量 | en:Interface number
        bool m_bIsIFOpen = false;                   // ch:采集卡是否打开 | en:Whether to open interface
        bool m_bIsDeviceOpen = false;               // ch:设备是否打开 | en:Whether to open device
        bool m_bIsGrabbing = false;                 // ch:是否在抓图 | en:Whether to start grabbing

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

            this.btnStartGrab.Enabled = m_bIsDeviceOpen && !m_bIsGrabbing;
            this.btnStopGrab.Enabled = m_bIsDeviceOpen && m_bIsGrabbing;

            this.groupBoxIFParams.Enabled = m_bIsIFOpen;
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
            nRet = m_cSystem.UpdateInterfaceList(CParamDefine.MV_FG_CAMERALINK_INTERFACE, ref bChanged);
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
                        case CParamDefine.MV_FG_CAMERALINK_INTERFACE:
                            {
                                MV_CML_INTERFACE_INFO stCmlIFInfo = (MV_CML_INTERFACE_INFO)CAdditional.ByteToStruct(
                                    stIfInfo.SpecialInfo.stCMLIfInfo, typeof(MV_CML_INTERFACE_INFO));
                                strShowIfInfo += "CML[" + i.ToString() + "]: " + stCmlIFInfo.chDisplayName + " | " +
                                    stCmlIFInfo.chInterfaceID + " | " + stCmlIFInfo.chSerialNumber;
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

        private void InitInterfaceParam()
        {
            if (null == m_cInterface)
            {
                return;
            }

            int nRet = CErrorCode.MV_FG_SUCCESS;

            CParam cParam = new CParam(m_cInterface);
            MV_FG_INTVALUE stIntValue = new MV_FG_INTVALUE();
            MV_FG_ENUMVALUE stEnumValue = new MV_FG_ENUMVALUE();

            nRet = cParam.GetIntValue("CameraWidth", ref stIntValue);
            if (CErrorCode.MV_FG_SUCCESS == nRet)
            {
                this.textBoxWidth.Text = Convert.ToString(stIntValue.nCurValue);
            }

            nRet = cParam.GetIntValue("CameraHeight", ref stIntValue);
            if (CErrorCode.MV_FG_SUCCESS == nRet)
            {
                this.textBoxHeight.Text = Convert.ToString(stIntValue.nCurValue);
            }

            this.comboBoxPixelFormat.Items.Clear();
            nRet = cParam.GetEnumValue("CameraPixelFormat", ref stEnumValue);
            if (CErrorCode.MV_FG_SUCCESS == nRet)
            {
                for (int i = 0; i < stEnumValue.nSupportedNum; i++)
                {
                    this.comboBoxPixelFormat.Items.Add(stEnumValue.strSymbolicArray[i].strInfo);
                }
                this.comboBoxPixelFormat.SelectedItem = stEnumValue.strCurSymbolic;
            }

            this.comboBoxPixelSize.Items.Clear();
            nRet = cParam.GetEnumValue("CameraPixelSize", ref stEnumValue);
            if (CErrorCode.MV_FG_SUCCESS == nRet)
            {
                for (int i = 0; i < stEnumValue.nSupportedNum; i++)
                {
                    this.comboBoxPixelSize.Items.Add(stEnumValue.strSymbolicArray[i].strInfo);
                }
                this.comboBoxPixelSize.SelectedItem = stEnumValue.strCurSymbolic;
            }

            this.comboBoxTap.Items.Clear();
            nRet = cParam.GetEnumValue("CameraTapGeometry", ref stEnumValue);
            if (CErrorCode.MV_FG_SUCCESS == nRet)
            {
                for (int i = 0; i < stEnumValue.nSupportedNum; i++)
                {
                    this.comboBoxTap.Items.Add(stEnumValue.strSymbolicArray[i].strInfo);
                }
                this.comboBoxTap.SelectedItem = stEnumValue.strCurSymbolic;
            }

            this.comboBoxClConfig.Items.Clear();
            nRet = cParam.GetEnumValue("CameraClConfiguration", ref stEnumValue);
            if (CErrorCode.MV_FG_SUCCESS == nRet)
            {
                for (int i = 0; i < stEnumValue.nSupportedNum; i++)
                {
                    this.comboBoxClConfig.Items.Add(stEnumValue.strSymbolicArray[i].strInfo);
                }
                this.comboBoxClConfig.SelectedItem = stEnumValue.strCurSymbolic;
            }
        }

        private void btnOpenInterface_Click(object sender, EventArgs e)
        {
            if (this.cmbInterfaceList.Items.Count <= 0 || this.cmbInterfaceList.SelectedIndex < 0)
            {
                MessageBox.Show("No interface");
                return;
            }

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

            // ch:开启采集卡上的分离方案使能 | en:Enable seperate scheme on interface.
            CParam cParam = new CParam(m_cInterface);
            nRet = cParam.SetBoolValue("CameraDeviceEnable", true);
            if (CErrorCode.MV_FG_SUCCESS != nRet)
            {
                m_cInterface.CloseInterface();
                m_cInterface = null;

                MessageBox.Show("Enable CameraDeviceEnable failed, ErrorCode:" + nRet.ToString("X"));
                return;
            }

            // ch:初始化界面参数 | en:Init interface params.
            InitInterfaceParam();

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
                        case CParamDefine.MV_FG_CAMERALINK_DEVICE:
                            {
                                MV_CML_DEVICE_INFO stCmlDevInfo = (MV_CML_DEVICE_INFO)CAdditional.ByteToStruct(
                                    stDeviceInfo.DevInfo.stCMLDevInfo, typeof(MV_CML_DEVICE_INFO));
                                strShowDevInfo += "CML[" + i.ToString() + "]: " + stCmlDevInfo.chUserDefinedName + " | " +
                                    stCmlDevInfo.chModelName + " | " + stCmlDevInfo.chSerialNumber;
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

            m_bIsGrabbing = false;
            // ch:停止取流 | en:Stop Acquisition
            int nRet = m_cStream.StopAcquisition();
            if (CErrorCode.MV_FG_SUCCESS != nRet)
            {
                MessageBox.Show("Stop acquistion failed, ErrorCode:" + nRet.ToString("X"));
                return;
            }

            // ch:关闭流通道 | en:Close stream channel
            nRet = m_cStream.CloseStream();
            if (CErrorCode.MV_FG_SUCCESS != nRet)
            {
                MessageBox.Show("Close stream channel failed, ErrorCode:" + nRet.ToString("X"));
            }
            m_cStream = null;

            EnableControls();
            
            // ch:标志位设为false | en:Set flag bit false
            m_bThreadState = false;
            m_hGrabThread.Join();


        }

        private void DisplayError(int nRet)
        {
            MessageBox.Show("Display failed, ErrorCode:" + nRet.ToString("X"));
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
                            btnStopGrab_Click(this, new EventArgs());

                            break;
                        }

                        m_cStream.ReleaseFrameBuffer(stFrameInfo);
                    }
                }
                else
                {
                    Thread.Sleep(5);
                }
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

        private void textBoxWidth_Validated(object sender, EventArgs e)
        {
            if (null == m_cInterface)
            {
                return;
            }

            CParam cParam = new CParam(m_cInterface);
            Int64 nNewValue = Convert.ToInt64(this.textBoxWidth.Text);
            int nRet = cParam.SetIntValue("CameraWidth", nNewValue);
            if (CErrorCode.MV_FG_SUCCESS != nRet)
            {
                MessageBox.Show("Set CameraWidth failed, ErrorCode:" + nRet.ToString("X"));
            }
        }

        private void textBoxHeight_Validated(object sender, EventArgs e)
        {
            if (null == m_cInterface)
            {
                return;
            }

            CParam cParam = new CParam(m_cInterface);
            Int64 nNewValue = Convert.ToInt64(this.textBoxHeight.Text);
            int nRet = cParam.SetIntValue("CameraHeight", nNewValue);
            if (CErrorCode.MV_FG_SUCCESS != nRet)
            {
                MessageBox.Show("Set CameraHeight failed, ErrorCode:" + nRet.ToString("X"));
            }
        }

        private void comboBoxPixelFormat_SelectionChangeCommitted(object sender, EventArgs e)
        {
            if (null == m_cInterface)
            {
                return;
            }

            CParam cParam = new CParam(m_cInterface);
            string strSelectedPixelFormat = this.comboBoxPixelFormat.Text;
            int nRet = cParam.SetEnumValueByString("CameraPixelFormat", strSelectedPixelFormat);
            if (CErrorCode.MV_FG_SUCCESS != nRet)
            {
                MessageBox.Show("Set CameraPixelFormat failed, ErrorCode:" + nRet.ToString("X"));
            }
        }

        private void comboBoxPixelSize_SelectionChangeCommitted(object sender, EventArgs e)
        {
            if (null == m_cInterface)
            {
                return;
            }

            CParam cParam = new CParam(m_cInterface);
            string strSelectedValue = this.comboBoxPixelSize.Text;
            int nRet = cParam.SetEnumValueByString("CameraPixelSize", strSelectedValue);
            if (CErrorCode.MV_FG_SUCCESS != nRet)
            {
                MessageBox.Show("Set CameraPixelSize failed, ErrorCode:" + nRet.ToString("X"));
            }
        }

        private void comboBoxTap_SelectionChangeCommitted(object sender, EventArgs e)
        {
            if (null == m_cInterface)
            {
                return;
            }

            CParam cParam = new CParam(m_cInterface);
            string strSelectedValue = this.comboBoxTap.Text;
            int nRet = cParam.SetEnumValueByString("CameraTapGeometry", strSelectedValue);
            if (CErrorCode.MV_FG_SUCCESS != nRet)
            {
                MessageBox.Show("Set CameraTapGeometry failed, ErrorCode:" + nRet.ToString("X"));
            }
        }

        private void comboBoxClConfig_SelectionChangeCommitted(object sender, EventArgs e)
        {
            if (null == m_cInterface)
            {
                return;
            }

            CParam cParam = new CParam(m_cInterface);
            string strSelectedValue = this.comboBoxClConfig.Text;
            int nRet = cParam.SetEnumValueByString("CameraClConfiguration", strSelectedValue);
            if (CErrorCode.MV_FG_SUCCESS != nRet)
            {
                MessageBox.Show("Set CameraClConfiguration failed, ErrorCode:" + nRet.ToString("X"));
            }
        }
    }
}
