/*
This example shows the user how to control multiple interfaces and multiple devices at the same time.
This example covers enumeration module, control module and stream module.
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

namespace MultipleCamera
{
    public partial class MultipleCamera : Form
    {
        public const UInt32 LIST_NUM = 2;
        public const UInt32 MAX_DEVICE_NUM = 4;
        public const UInt32 BUFFER_NUMBER = 3;

        CSystem m_cSystem = new CSystem();                      // ch:操作采集卡 | en:Interface operations
        CInterface[] m_cInterface = {null, null};               // ch:操作采集卡和设备 | en:Interface and device operation
        CDevice[,] m_cDevice = {{null, null, null, null}, {null, null, null, null}};                     // ch:操作设备和流 | en:Device and stream operation
        CStream[,] m_cStream = { { null, null, null, null }, { null, null, null, null } };              // ch:操作流和缓存 | en:Stream and buffer operation
        
        uint m_nInterfaceNum = 0;                              // ch:采集卡数量 | en:Interface number
        bool[] m_bIsIFOpen = {false, false};                   // ch:采集卡是否打开 | en:Whether to open interface
        bool[] m_bIsDeviceOpen = {false, false};               // ch:设备是否打开 | en:Whether to open device
        bool m_bIsGrabbing = false;                            // ch:是否在抓图 | en:Whether to start grabbing

        public const UInt32 TRIGGER_MODE_ON = 1;               // ch:触发模式开 | en:Trigger mode on
        public const UInt32 TRIGGER_MODE_OFF = 0;              // ch:触发模式关 | en:Trigger mode off

        //bool m_bThreadState = false;        // ch:线程状态 | en:Thread state
        Thread[,] m_hGrabThread = { { null, null, null, null }, { null, null, null, null } };               // ch:取流线程 | en:Grabbing thread
        bool[,] m_bGrabThreadFlag = { { false, false, false, false }, { false, false, false, false } };     // ch:取流线程标志 | en:Grabbing thread flag
        

        int m_nCurListIndex   = -1;
        int m_nCurCameraIndex = -1;

       // delegate void ShowDisplayError(int nRet);
   

        ComboBox[] m_cmbInterfaceList;
        CheckBox[,] m_chkDeviceList;
        PictureBox[,] m_pctDisplay;

        public MultipleCamera()
        {
            InitializeComponent();
            EnableControls(false);
            Control.CheckForIllegalCrossThreadCalls = false;
            m_cmbInterfaceList = new ComboBox[2] { cmbInterfaceListFirst, cmbInterfaceListSecond };
            m_chkDeviceList = new CheckBox[2, 4] { {checkBox1, checkBox2, checkBox3, checkBox4}, {checkBox5, checkBox6, checkBox7, checkBox8} };
            m_pctDisplay = new PictureBox[2, 4] { { pictureBox1, pictureBox2, pictureBox3, pictureBox4 }, { pictureBox5, pictureBox6, pictureBox7, pictureBox8 } };
        }

        private void EnableControls(bool bIsCameraReady)
        {
            this.btnEnumInterface.Enabled = !(m_bIsIFOpen[0] || m_bIsIFOpen[1]);
            this.btnOpenInterface.Enabled = (m_nInterfaceNum > 0) && !(m_bIsIFOpen[0] || m_bIsIFOpen[1]);
            this.btnCloseInterface.Enabled = (m_bIsIFOpen[0] || m_bIsIFOpen[1]) && !(m_bIsDeviceOpen[0] || m_bIsDeviceOpen[1]);

            this.btnEnumDevice.Enabled = (m_bIsIFOpen[0] || m_bIsIFOpen[1]) && !(m_bIsDeviceOpen[0] || m_bIsDeviceOpen[1]);
            this.btnOpenDevice.Enabled = (m_bIsIFOpen[0] || m_bIsIFOpen[1]) && (m_nInterfaceNum > 0) && !(m_bIsDeviceOpen[0] || m_bIsDeviceOpen[1]) && bIsCameraReady;
            this.btnCloseDevice.Enabled = (m_bIsIFOpen[0] || m_bIsIFOpen[1]) && (m_bIsDeviceOpen[0] || m_bIsDeviceOpen[1]) && bIsCameraReady;

            this.btnStartGrab.Enabled = (m_bIsDeviceOpen[0] || m_bIsDeviceOpen[1]) && !m_bIsGrabbing && bIsCameraReady;
            this.btnStopGrab.Enabled = (m_bIsDeviceOpen[0] || m_bIsDeviceOpen[1]) && m_bIsGrabbing;

            this.cmbInterfaceListFirst.Enabled = (m_nInterfaceNum > 0) && !(m_bIsIFOpen[0] || m_bIsIFOpen[1]);
            this.cmbInterfaceListSecond.Enabled = (m_nInterfaceNum > 0) && !(m_bIsIFOpen[0] || m_bIsIFOpen[1]);

          
        }

        
        private void btnEnumInterface_Click(object sender, EventArgs e)
        {
            int nRet = 0;
            bool bChanged = false;

            // ch:枚举采集卡 | en:Enum interface
            nRet = m_cSystem.UpdateInterfaceList(
                CParamDefine.MV_FG_CAMERALINK_INTERFACE | CParamDefine.MV_FG_GEV_INTERFACE | CParamDefine.MV_FG_CXP_INTERFACE | CParamDefine.MV_FG_XoF_INTERFACE,
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
                EnableControls(false);
                return;
            }
            if (0 == m_nInterfaceNum)
            {
                MessageBox.Show("No interface found");
                EnableControls(false);
                return;
            }

            if (bChanged)
            {
                for (uint i = 0; i < LIST_NUM; i++)
                {
                    m_cmbInterfaceList[i].Items.Clear();
                    this.m_cmbInterfaceList[i].ResetText();
                }


                // ch:向下拉框添加采集卡信息 | en:Add interface info in Combo
                MV_FG_INTERFACE_INFO stIfInfo = new MV_FG_INTERFACE_INFO();
                for (uint i = 0; i < m_nInterfaceNum; i++)
                {
                    // ch:获取采集卡信息 | en:Get interface info
                    nRet = m_cSystem.GetInterfaceInfo(i, ref stIfInfo);
                    if (CErrorCode.MV_FG_SUCCESS != nRet)
                    {
                        for (uint j = 0; j < LIST_NUM; j++)
                        {
                            m_cmbInterfaceList[j].Items.Clear();
                            this.m_cmbInterfaceList[j].ResetText();
                        }
                        MessageBox.Show("Get interface info failed, ErrorCode:" + nRet.ToString("X"));
                        return;
                    }

                    string strShowIfInfo = null;
                    switch (stIfInfo.nTLayerType)
                    {
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

                    for (uint k = 0; k < LIST_NUM; k++)
                    {
                        this.m_cmbInterfaceList[k].Items.Add(strShowIfInfo);

                    }
                        
                }
            }

            if (m_nInterfaceNum > 0)
            {
                for (uint i = 0; i < LIST_NUM; i++)
                {
                    this.m_cmbInterfaceList[i].SelectedIndex = 0;
                }
            }

            EnableControls(false);

        }

        private void btnOpenInterface_Click(object sender, EventArgs e)
        {
            int nRet = 0;

            for (uint i = 0; i < LIST_NUM; i++)
            {
                if (this.m_cmbInterfaceList[i].Items.Count <= 0 || this.m_cmbInterfaceList[i].SelectedIndex < 0)
                {
                    MessageBox.Show("No interface");
                    return;
                }
            }

            // ch:打开采集卡，获得采集卡句柄 | en:Open interface, get handle
            nRet = m_cSystem.OpenInterface(Convert.ToUInt32(this.m_cmbInterfaceList[0].SelectedIndex), out m_cInterface[0]);
            if (CErrorCode.MV_FG_SUCCESS != nRet)
            {
                MessageBox.Show("List 1:Open Interface failed, ErrorCode:" + nRet.ToString("X"));
                return;
            }

            m_bIsIFOpen[0] = true;

            if (this.m_cmbInterfaceList[0].SelectedIndex != this.m_cmbInterfaceList[1].SelectedIndex)
            {
                nRet = m_cSystem.OpenInterface(Convert.ToUInt32(this.m_cmbInterfaceList[1].SelectedIndex), out m_cInterface[1]);
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    MessageBox.Show("List 2:Open Interface failed, ErrorCode:" + nRet.ToString("X"));
                    return;
                }

            }
            m_bIsIFOpen[1] = true;
            EnableControls(false);
        }

        private void btnCloseInterface_Click(object sender, EventArgs e)
        {
            int nRet = 0;
            m_bIsGrabbing = false;
            if (null == m_cInterface[0] && null == m_cInterface[1])
            {
                return;
            }

            if (m_bIsDeviceOpen[0] || m_bIsDeviceOpen[1])
            {
                btnCloseDevice_Click(sender, e);
            }

            // ch:关闭采集卡 | en:Close interface
            if (null != m_cInterface[0])
            {
                nRet = m_cInterface[0].CloseInterface();
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    MessageBox.Show("List 1:Close interface failed, ErrorCode:" + nRet.ToString("X"));
                    return;
                }

            }
         
            if (null != m_cInterface[1])
            {
                nRet = m_cInterface[1].CloseInterface();
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    MessageBox.Show("List 2:Close interface failed, ErrorCode:" + nRet.ToString("X"));
                    return;
                }

            }

            for (uint i = 0; i < LIST_NUM; i++)
            {
                m_cInterface[i] = null;
                m_bIsIFOpen[i]  = false;

            }

            EnableControls(false);
        }

        private void btnEnumDevice_Click(object sender, EventArgs e)
        {
            int nRet = 0;
            bool[] bChanged = { false, false };
            uint[] nDeviceNum = {0, 0};

            // ch:枚举采集卡上的相机 | en:Enum camera of interface
            nRet = m_cInterface[0].UpdateDeviceList(ref bChanged[0]);
            if (CErrorCode.MV_FG_SUCCESS != nRet)
            {
                MessageBox.Show("List 1:Update device list failed, ErrorCode:" + nRet.ToString("X"));
                return;
            }

            // ch:获取设备数量 | en:Get device number
            nRet = m_cInterface[0].GetNumDevices(ref nDeviceNum[0]);
            if (CErrorCode.MV_FG_SUCCESS != nRet)
            {
                MessageBox.Show("List 1:Get devices number failed, ErrorCode:" + nRet.ToString("X"));
                EnableControls(false);
                return;
            }

            if (null != m_cInterface[1])
            {
                nRet = m_cInterface[1].UpdateDeviceList(ref bChanged[1]);
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    MessageBox.Show("List 2:Update device list failed, ErrorCode:" + nRet.ToString("X"));
                    return;
                }

                nRet = m_cInterface[1].GetNumDevices(ref nDeviceNum[1]);
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    MessageBox.Show("List 2:Get devices number failed, ErrorCode:" + nRet.ToString("X"));
                    EnableControls(false);
                    return;
                }

            }


            if (0 == nDeviceNum[0] && 0 == nDeviceNum[1])
            {
                MessageBox.Show("No Device found");
                EnableControls(false);
                return;
            }

            if (bChanged[0] || bChanged[1])
            {

                MV_FG_DEVICE_INFO stDeviceInfo = new MV_FG_DEVICE_INFO();
                for (uint i = 0; i < (nDeviceNum[0] + nDeviceNum[1]); i++)
                {
                    // ch:获取设备信息 | en:Get device info
                    if (i < nDeviceNum[0])
                    {
                        nRet = m_cInterface[0].GetDeviceInfo(i, ref stDeviceInfo);
                        if (CErrorCode.MV_FG_SUCCESS != nRet)
                        {
                            MessageBox.Show("List 1:Get device info failed, ErrorCode:" + nRet.ToString("X"));
                            return;
                        }
                    }
                    else
                    {
                        nRet = m_cInterface[1].GetDeviceInfo(i - nDeviceNum[0], ref stDeviceInfo);
                        if (CErrorCode.MV_FG_SUCCESS != nRet)
                        {
                            MessageBox.Show("List 2:Get device info failed, ErrorCode:" + nRet.ToString("X"));
                            return;
                        }
                    }
                    

                    string strShowDevInfo = null;
                    switch (stDeviceInfo.nDevType)
                    {
                        case CParamDefine.MV_FG_GEV_DEVICE:
                            {
                                MV_GEV_DEVICE_INFO stGevDevInfo = (MV_GEV_DEVICE_INFO)CAdditional.ByteToStruct(
                                    stDeviceInfo.DevInfo.stGEVDevInfo, typeof(MV_GEV_DEVICE_INFO));
                                strShowDevInfo += "[GEV]" + " | " +
                                    stGevDevInfo.chModelName + " | " + stGevDevInfo.chSerialNumber;
                                break;
                            }
                        case CParamDefine.MV_FG_CXP_DEVICE:
                            {
                                MV_CXP_DEVICE_INFO stCxpDevInfo = (MV_CXP_DEVICE_INFO)CAdditional.ByteToStruct(
                                    stDeviceInfo.DevInfo.stCXPDevInfo, typeof(MV_CXP_DEVICE_INFO));
                                strShowDevInfo += "[CXP]" +  " | " +
                                    stCxpDevInfo.chModelName + " | " + stCxpDevInfo.chSerialNumber;
                                break;
                            }
                        case CParamDefine.MV_FG_CAMERALINK_DEVICE:
                            {
                                MV_CML_DEVICE_INFO stCmlDevInfo = (MV_CML_DEVICE_INFO)CAdditional.ByteToStruct(
                                    stDeviceInfo.DevInfo.stCMLDevInfo, typeof(MV_CML_DEVICE_INFO));
                                strShowDevInfo += "[CML]" +  " | " +
                                    stCmlDevInfo.chModelName + " | " + stCmlDevInfo.chSerialNumber;
                                break;
                            }
                        case CParamDefine.MV_FG_XoF_DEVICE:
                            {
                                MV_XoF_DEVICE_INFO stXoFDevInfo = (MV_XoF_DEVICE_INFO)CAdditional.ByteToStruct(
                                    stDeviceInfo.DevInfo.stXoFDevInfo, typeof(MV_XoF_DEVICE_INFO));
                                strShowDevInfo += "[XoF]" +  " | " +
                                    stXoFDevInfo.chModelName + " | " + stXoFDevInfo.chSerialNumber;
                                break;
                            }
                        default:
                            {
                                strShowDevInfo += "Unknown device[" + i.ToString() + "]";
                                break;
                            }
                    }
                    
                    

                    if (i < nDeviceNum[0])
                    {
                        this.m_chkDeviceList[0, i].Enabled = true;
                        this.m_chkDeviceList[0, i].Text = strShowDevInfo;

                    }
                    else
                    {
                        this.m_chkDeviceList[1, i - nDeviceNum[0]].Enabled = true;
                        this.m_chkDeviceList[1, i - nDeviceNum[0]].Text = strShowDevInfo;

                    }

                   

                }

                // 不可选中无相机信息的复选框，重置相机名
                for (uint j = 0; j < LIST_NUM; j++)
                {
                    for (uint k = nDeviceNum[j]; k < MAX_DEVICE_NUM; k++)
                    {
                        this.m_chkDeviceList[j, k].Enabled = false;
                        this.m_chkDeviceList[j, k].Text = "Cam" + (k + 1).ToString();
                        
                    }
                }  



            }

            for (int i = 0; i < LIST_NUM; i++)
            {
                listBox1.Items.Add("List" + (i + 1).ToString() + ":Total Find" + nDeviceNum[i].ToString() + "devices!");
                
            }

            EnableControls(true);

        }

        private void btnOpenDevice_Click(object sender, EventArgs e)
        {
            if (true == m_bIsDeviceOpen[0] || true == m_bIsDeviceOpen[1])
            {
                return;
            }

            bool[] bHaveCheck = {false, false};

            for (int i = 0; i < LIST_NUM; i++)
            {
                for (uint nDeviceIndex = 0; nDeviceIndex < MAX_DEVICE_NUM; nDeviceIndex++)
                {
                    if (true == m_chkDeviceList[i, nDeviceIndex].Checked && null != m_cInterface[i]) //已勾选的相机
                    {
                        bHaveCheck[i] = true;
                        int nRet = m_cInterface[i].OpenDevice(nDeviceIndex, out m_cDevice[i,nDeviceIndex]);
                        if (CErrorCode.MV_FG_SUCCESS != nRet)
                        {
                            listBox1.Items.Add("List" + (i + 1).ToString() + ":Open device failed! DevIndex[" + nDeviceIndex + "] nRet" + nRet.ToString("X"));
                            m_cDevice[i, nDeviceIndex] = null;
                            continue;
                        }
                        else
                        {
                            listBox1.Items.Add("List" + (i + 1).ToString() + ":Open device success! DevIndex[" + nDeviceIndex + "]");
                            m_bIsDeviceOpen[i] = true;
                        }

                        // ch:设置连续采集模式 | en:Set Continuous Aquisition Mode
                        CParam cDeviceParam = new CParam(m_cDevice[i, nDeviceIndex]);
                        cDeviceParam.SetEnumValue("AcquisitionMode", 2);  // 0 - SingleFrame, 2 - Continuous
                        cDeviceParam.SetEnumValue("TriggerMode", TRIGGER_MODE_OFF);
                    }
                }
            }

            for (int i = 0; i < LIST_NUM; i++)
            {
                if (true == m_bIsDeviceOpen[i])
                {
                    EnableControls(true);
                    
                    for (uint j = 0; j < MAX_DEVICE_NUM; j++)
                    {
                        m_chkDeviceList[i, j].Enabled = false;
                    }
                
                    
                }
                else
                {
                    if (false == bHaveCheck[i])
                    {
                        listBox1.Items.Add("List" + (i + 1).ToString() + ":Unchecked device!");
                    }
                    else
                    {
                        listBox1.Items.Add("List" + (i + 1).ToString() + ":No device opened successfully!");
                    }

                }
        
            }


        }

        private void btnCloseDevice_Click(object sender, EventArgs e)
        {
            if (false == m_bIsDeviceOpen[0] && false == m_bIsDeviceOpen[1])
            {
                return;
            }

            btnStopGrab_Click(sender, e);
            int nRet = 0;

            for (int i = 0; i < LIST_NUM; i++)
            {
                for (int j = 0; j < MAX_DEVICE_NUM; j++)
                {
                    if (null != m_cDevice[i, j])
                    {
                        nRet =  m_cDevice[i, j].CloseDevice();
                        if (CErrorCode.MV_FG_SUCCESS != nRet)
                        {
                           // PrintMessage("List %d:Close device fail! DevIndex[%d], nRet[%#x]\r\n", i + 1, j + 1, nRet);
                            listBox1.Items.Add("List" + (j + 1).ToString() + ":Close device fail! DevIndex[" + (j + 1).ToString() + "],  nRet" + nRet.ToString("X"));
                        }
                        m_cDevice[i, j] = null;
                    }

                    m_chkDeviceList[i, j].Enabled = true;


                }

            }

            m_bIsDeviceOpen[0] = false;
            m_bIsDeviceOpen[1] = false;
            EnableControls(true);
        }

        private void btnStartGrab_Click(object sender, EventArgs e)
        {
            if ((false == m_bIsDeviceOpen[0] && false == m_bIsDeviceOpen[1]) || true == m_bIsGrabbing)
            {        
                return;
            }

            int nRet = 0;

            for (int i = 0; i < LIST_NUM; i++)
            {
                for (int j = 0; j < MAX_DEVICE_NUM; j++)
                {
                    if (null != m_cDevice[i, j])
                    {
                        // ch:获取流通道个数 | en:Get number of stream
                        uint nStreamNum = 0;
                        nRet = m_cDevice[i, j].GetNumStreams(ref nStreamNum);
                        if (CErrorCode.MV_FG_SUCCESS != nRet || 0 == nStreamNum)
                        {
                            listBox1.Items.Add("List" + (j + 1).ToString() + ":No stream available! DevIndex[" + (j + 1).ToString() + "],  nRet" + nRet.ToString("X"));
                            continue;
                        }

                        // ch:打开流通道(目前只支持单个通道) | en:Open stream(Only a single stream is supported now)
                        nRet = m_cDevice[i, j].OpenStream(0, out m_cStream[i, j]);
                        if (CErrorCode.MV_FG_SUCCESS!= nRet)
                        {
                            listBox1.Items.Add("List" + (j + 1).ToString() + ":Open Stream failed! DevIndex[" + (j + 1).ToString() + "],  nRet" + nRet.ToString("X"));
                            continue;
                        }

                        // ch:设置SDK内部缓存数量 | en:Set internal buffer number
                        nRet = m_cStream[i, j].SetBufferNum(BUFFER_NUMBER);
                        if (CErrorCode.MV_FG_SUCCESS != nRet)
                        {
                            listBox1.Items.Add("List" + (j + 1).ToString() + ":Set buffer number failed! DevIndex[" + (j + 1).ToString() + "],  nRet" + nRet.ToString("X"));
                            nRet = m_cStream[i, j].CloseStream();
                            if (CErrorCode.MV_FG_SUCCESS != nRet)
                            {
                                MessageBox.Show("List" + (i + 1).ToString() + "Close stream failed!  nRet" + nRet.ToString("X"));
                                
                            }
                            m_cStream[i, j] = null;
                            continue;
                        }


                        // ch:开始取流 | en:Start Acquisition
                        nRet = m_cStream[i, j].StartAcquisition();
                        if (CErrorCode.MV_FG_SUCCESS != nRet)
                        {
                            listBox1.Items.Add("List" + (j + 1).ToString() + ":Start Acquisition failed! DevIndex[" + (j + 1).ToString() + "],  nRet" + nRet.ToString("X"));
                            nRet = m_cStream[i, j].CloseStream();
                            if (CErrorCode.MV_FG_SUCCESS != nRet)
                            {
                                MessageBox.Show("List" + (i + 1).ToString() + "Close stream failed!  nRet" + nRet.ToString("X"));
                            }
                            m_cStream[i, j] = null;
                            continue;
                        }
                        m_nCurListIndex   = i;
                        m_nCurCameraIndex = j;
                        m_bIsGrabbing = true;

                        // ch:创建取流线程 | en:Create acquistion thread
                        m_hGrabThread[i, j] = new Thread(ReceiveThreadProcess);
                        if (null == m_hGrabThread[i, j])
                        {
                            m_bIsGrabbing = false;
                            MessageBox.Show("Create thread failed");
                            return;
                        }
                        m_hGrabThread[i, j].Start();
                        m_bGrabThreadFlag[i, j] = true;
                    }
                }
            }
    
            EnableControls(true);

        }

        private void btnStopGrab_Click(object sender, EventArgs e)
        {
            if ((false == m_bIsDeviceOpen[0] && false == m_bIsDeviceOpen[1]) || false == m_bIsGrabbing)
            {        
                return;
            }

            int nRet = 0;

            for (int i = 0; i < LIST_NUM; i++)
            {
                for (int j = 0; j < MAX_DEVICE_NUM; j++)
                {
                    if(null == m_hGrabThread[i, j])
                    {
                        continue;
                    }

                    m_bGrabThreadFlag[i, j] = false;

                    m_hGrabThread[i, j].Join();
                    if (null != m_cStream[i, j])
                    {
                        nRet = m_cStream[i, j].StopAcquisition();
                        if (CErrorCode.MV_FG_SUCCESS != nRet)
                        {
                            listBox1.Items.Add("List" + (j + 1).ToString() + ":Stop grabbing failed! DevIndex[" + (j + 1).ToString() + "],  nRet" + nRet.ToString("X"));
                            return;
                        }

                        nRet = m_cStream[i, j].CloseStream();
                        if (CErrorCode.MV_FG_SUCCESS != nRet)
                        {
                            MessageBox.Show("List" + (i + 1).ToString() + ":Close stream failed!  nRet" + nRet.ToString("X"));
                            return;
                        }
                        m_cStream[i, j] = null;

                    }
                }
            }

            m_bIsGrabbing = false;
    
            EnableControls(true);

        }

        public void ReceiveThreadProcess()
        {
            const uint nTimeout = 1000;
            int nCurListIndex = m_nCurListIndex;
            int nCurCameraIndex = m_nCurCameraIndex;

            m_nCurListIndex = -1;
            m_nCurCameraIndex = -1;

            MV_FG_BUFFER_INFO stFrameInfo = new MV_FG_BUFFER_INFO();    // 图像信息
            MV_FG_INPUT_IMAGE_INFO stDisplayFrameInfo = new MV_FG_INPUT_IMAGE_INFO();   // 显示信息

            CImageProcess cImgProc = new CImageProcess(m_cStream[nCurListIndex, nCurCameraIndex]);
            int nRet = 0;

            while (m_bIsGrabbing && false != m_bGrabThreadFlag[nCurListIndex, nCurCameraIndex])
            {
                nRet = m_cStream[nCurListIndex, nCurCameraIndex].GetFrameBuffer(ref stFrameInfo, nTimeout);
                if (CErrorCode.MV_FG_SUCCESS != nRet)
                {
                    MessageBox.Show("Get Frame Buffer fail! DevIndex[" + nCurCameraIndex.ToString() + "]  nRet" + nRet.ToString("X"));
                    continue;
                }

                if (null != stFrameInfo.pBuffer)
                {

                    stDisplayFrameInfo.pImageBuf = stFrameInfo.pBuffer;
                    stDisplayFrameInfo.nImageBufLen = stFrameInfo.nFilledSize;
                    stDisplayFrameInfo.nWidth = stFrameInfo.nWidth;
                    stDisplayFrameInfo.nHeight = stFrameInfo.nHeight;
                    stDisplayFrameInfo.enPixelType = stFrameInfo.enPixelType;

                    nRet = cImgProc.DisplayOneFrame(m_pctDisplay[nCurListIndex, nCurCameraIndex].Handle, ref stDisplayFrameInfo);
                    if (CErrorCode.MV_FG_SUCCESS != nRet)
                    {
                        MessageBox.Show("Display OneFrame failed! DevIndex[" + nCurCameraIndex.ToString() + "]  nRet" + nRet.ToString("X"));
                    }

                    nRet = m_cStream[nCurListIndex, nCurCameraIndex].ReleaseFrameBuffer(stFrameInfo);
                    if (CErrorCode.MV_FG_SUCCESS != nRet)
                    {
                        MessageBox.Show("GRelease frame buffer failed!  nRet" + nRet.ToString("X"));
                    }
                }
            }

        }


       
       
    }
}
