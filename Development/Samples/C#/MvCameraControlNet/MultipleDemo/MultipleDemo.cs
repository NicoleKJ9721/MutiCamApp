using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using MvCamCtrl.NET;
using System.Runtime.InteropServices;
using System.Threading;
using System.IO;

namespace MultipleDemo
{
    public partial class MultipleDemo : Form
    {
        [DllImport("kernel32.dll", EntryPoint = "RtlMoveMemory", SetLastError = false)]
        private static extern void CopyMemory(IntPtr dest, IntPtr src, uint count);

        public UInt32[] m_nSaveImageBufSize = new UInt32[4] { 0 , 0, 0, 0};
        public IntPtr[] m_pSaveImageBuf = new IntPtr[4] { IntPtr.Zero, IntPtr.Zero, IntPtr.Zero, IntPtr.Zero };
        private Object[] m_BufForSaveImageLock = new Object[4];
        MyCamera.MV_FRAME_OUT_INFO_EX[] m_stFrameInfo = new MyCamera.MV_FRAME_OUT_INFO_EX[4];

        MyCamera.cbOutputExdelegate cbImage;
        MyCamera.MV_CC_DEVICE_INFO_LIST m_pDeviceList;
        private MyCamera[] m_pMyCamera;
        MyCamera.MV_CC_DEVICE_INFO[] m_pDeviceInfo;
        bool m_bGrabbing;
        int m_nCanOpenDeviceNum;        // ch:设备使用数量 | en:Used Device Number
        int m_nDevNum;        // ch:在线设备数量 | en:Online Device Number
        int[] m_nFrames;      // ch:帧数 | en:Frame Number
        bool m_bTimerFlag;     // ch:定时器开始计时标志位 | en:Timer Start Timing Flag Bit
        IntPtr []m_hDisplayHandle;

        public MultipleDemo()
        {
            InitializeComponent();
            m_pDeviceList = new MyCamera.MV_CC_DEVICE_INFO_LIST();
            m_bGrabbing = false;
            m_nCanOpenDeviceNum = 0;
            m_nDevNum = 0;
            m_pMyCamera = new MyCamera[4];
            m_pDeviceInfo = new MyCamera.MV_CC_DEVICE_INFO[4];
            m_nFrames = new int[4];
            cbImage = new MyCamera.cbOutputExdelegate(ImageCallBack);
            for (int i = 0; i < 4; ++i)
            {
                m_BufForSaveImageLock[i] = new Object();
            }

            m_bTimerFlag = false;
            m_hDisplayHandle = new IntPtr[4];
            this.Load += new EventHandler(this.MultipleDemo_Load);
        }

        private void MultipleDemo_Load(object sender, EventArgs e)
        {
            // ch: 初始化 SDK | en: Initialize SDK
            MyCamera.MV_CC_Initialize_NET();

            // ch: 枚举设备列表 | en: Enum Devices
            DeviceListAcq();
        }

        public void ResetMember()
        {
            m_pDeviceList = new MyCamera.MV_CC_DEVICE_INFO_LIST();
            m_bGrabbing = false;
            m_nCanOpenDeviceNum = 0;
            m_nDevNum = 0;
            DeviceListAcq();
            m_nFrames = new int[4];
            cbImage = new MyCamera.cbOutputExdelegate(ImageCallBack);
            m_bTimerFlag = false;
            m_hDisplayHandle = new IntPtr[4];
            m_pDeviceInfo = new MyCamera.MV_CC_DEVICE_INFO[4];
        }

        // ch:枚举设备 | en:Create Device List
        private void DeviceListAcq()
        {
            System.GC.Collect();
            int nRet = MyCamera.MV_CC_EnumDevices_NET(MyCamera.MV_GIGE_DEVICE | MyCamera.MV_USB_DEVICE, ref m_pDeviceList);
            if (0 != nRet)
            {
                richTextBox.Text += "Enumerate devices fail!\r\n";
                return;
            }

            m_nDevNum = (int)m_pDeviceList.nDeviceNum;
            tbDevNum.Text = m_nDevNum.ToString("d");
        }

        private void SetCtrlWhenOpen()
        {
            bnOpen.Enabled = false;
            bnClose.Enabled = true;
            bnStartGrab.Enabled = true;
            bnStopGrab.Enabled = false;
            bnContinuesMode.Enabled = true;
            bnContinuesMode.Checked = true;
            bnTriggerMode.Enabled = true;
            cbSoftTrigger.Enabled = false;
            bnTriggerExec.Enabled = false;

            tbExposure.Enabled = true;
            tbGain.Enabled = true;
            bnSetParam.Enabled = true;
        }

        // ch:初始化、打开相机 | en:Initialization and open devices
        private void bnOpen_Click(object sender, EventArgs e)
        {
            bool bOpened =false;
            // ch:判断输入格式是否正确 | en:Determine whether the input format is correct
            try
            {
                int.Parse(tbUseNum.Text);
            }
            catch
            {
                richTextBox.Text += "Please enter correct format!\r\n";
                return;
            }
            // ch:获取使用设备的数量 | en:Get Used Device Number
            int nCameraUsingNum = int.Parse(tbUseNum.Text);
            // ch:参数检测 | en:Parameters inspection
            if (nCameraUsingNum <= 0)
            {
                nCameraUsingNum = 1;
            }
            if (nCameraUsingNum > 4)
            {
                nCameraUsingNum = 4;
            }

            for (int i = 0, j = 0; j < m_nDevNum; j++)
            {
                //ch:获取选择的设备信息 | en:Get Selected Device Information
                MyCamera.MV_CC_DEVICE_INFO device =
                    (MyCamera.MV_CC_DEVICE_INFO)Marshal.PtrToStructure(m_pDeviceList.pDeviceInfo[j], typeof(MyCamera.MV_CC_DEVICE_INFO));

                string StrTemp = "";
                string strUserDefinedName = "";

                if (device.nTLayerType == MyCamera.MV_GIGE_DEVICE)
                {
                    MyCamera.MV_GIGE_DEVICE_INFO_EX gigeInfo = (MyCamera.MV_GIGE_DEVICE_INFO_EX)MyCamera.ByteToStruct(device.SpecialInfo.stGigEInfo, typeof(MyCamera.MV_GIGE_DEVICE_INFO_EX));

                    if ((gigeInfo.chUserDefinedName.Length > 0) && (gigeInfo.chUserDefinedName[0] != '\0'))
                    {
                        if (MyCamera.IsTextUTF8(gigeInfo.chUserDefinedName))
                        {
                            strUserDefinedName = Encoding.UTF8.GetString(gigeInfo.chUserDefinedName).TrimEnd('\0');
                        }
                        else
                        {
                            strUserDefinedName = Encoding.Default.GetString(gigeInfo.chUserDefinedName).TrimEnd('\0');
                        }
                        StrTemp = "GEV: " + strUserDefinedName + " (" + gigeInfo.chSerialNumber + ")";
                    }
                    else
                    {
                        StrTemp = "GEV: " + gigeInfo.chManufacturerName + " " + gigeInfo.chModelName + " (" + gigeInfo.chSerialNumber + ")";
                    }
                }
                else if (device.nTLayerType == MyCamera.MV_USB_DEVICE)
                {
                    MyCamera.MV_USB3_DEVICE_INFO_EX usbInfo = (MyCamera.MV_USB3_DEVICE_INFO_EX)MyCamera.ByteToStruct(device.SpecialInfo.stUsb3VInfo, typeof(MyCamera.MV_USB3_DEVICE_INFO_EX));
                    
                    if ((usbInfo.chUserDefinedName.Length > 0) && (usbInfo.chUserDefinedName[0] != '\0'))
                    {
                        if (MyCamera.IsTextUTF8(usbInfo.chUserDefinedName))
                        {
                            strUserDefinedName = Encoding.UTF8.GetString(usbInfo.chUserDefinedName).TrimEnd('\0');
                        }
                        else
                        {
                            strUserDefinedName = Encoding.Default.GetString(usbInfo.chUserDefinedName).TrimEnd('\0');
                        }

                        StrTemp = "U3V: " + strUserDefinedName + " (" + usbInfo.chSerialNumber + ")";
                    }
                    else
                    {
                        StrTemp = "U3V: " + usbInfo.chManufacturerName + " " + usbInfo.chModelName + " (" + usbInfo.chSerialNumber + ")";
                    }
                }

                //ch:打开设备 | en:Open Device
                if (null == m_pMyCamera[i])
                {
                    m_pMyCamera[i] = new MyCamera();
                    if (null == m_pMyCamera[i])
                    {
                        return ;
                    }
                }

                int nRet = m_pMyCamera[i].MV_CC_CreateDevice_NET(ref device);
                if (MyCamera.MV_OK != nRet)
                {
                    return;
                }

                nRet = m_pMyCamera[i].MV_CC_OpenDevice_NET();
                if (MyCamera.MV_OK != nRet)
                {
                    richTextBox.Text += String.Format("Open Device[{0}] failed! nRet=0x{1}\r\n", StrTemp, nRet.ToString("X"));
                    continue;
                }
                else
                {
                    richTextBox.Text += String.Format("Open Device[{0}] success!\r\n", StrTemp);
                    
                    m_nCanOpenDeviceNum++;
                    m_pDeviceInfo[i] = device;
                    // ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera)
                    if (device.nTLayerType == MyCamera.MV_GIGE_DEVICE)
                    {
                        int nPacketSize = m_pMyCamera[i].MV_CC_GetOptimalPacketSize_NET();
                        if (nPacketSize > 0)
                        {
                            nRet = m_pMyCamera[i].MV_CC_SetIntValueEx_NET("GevSCPSPacketSize", nPacketSize);
                            if (nRet != MyCamera.MV_OK)
                            {
                                richTextBox.Text += String.Format("Set Packet Size failed! nRet=0x{0}\r\n", nRet.ToString("X"));
                            }
                        }
                        else
                        {
                            richTextBox.Text += String.Format("Get Packet Size failed! nRet=0x{0}\r\n", nPacketSize.ToString("X"));
                        }
                    }

                    m_pMyCamera[i].MV_CC_SetEnumValue_NET("TriggerMode", (uint)MyCamera.MV_CAM_TRIGGER_MODE.MV_TRIGGER_MODE_OFF);
                    m_pMyCamera[i].MV_CC_RegisterImageCallBackEx_NET(cbImage, (IntPtr)i);
                    bOpened = true;
                    if (m_nCanOpenDeviceNum == nCameraUsingNum)
                    {
                        break;
                    }
                    i++;
                }
            }

            // ch:只要有一台设备成功打开 | en:As long as there is a device successfully opened
            if (bOpened)
            {
                tbUseNum.Text = m_nCanOpenDeviceNum.ToString();
                SetCtrlWhenOpen();
            }
        }

        private void SetCtrlWhenClose()
        {
            bnOpen.Enabled = true;
            bnClose.Enabled = false;
            bnStartGrab.Enabled = false;
            bnStopGrab.Enabled = false;
            bnContinuesMode.Enabled = false;
            bnTriggerMode.Enabled = false;
            cbSoftTrigger.Enabled = false;
            bnTriggerExec.Enabled = false;

            bnSaveBmp.Enabled = false;
            tbExposure.Enabled = false;
            tbGain.Enabled = false;
            bnSetParam.Enabled = false;
        }

        // ch:关闭相机 | en:Close Device
        private void bnClose_Click(object sender, EventArgs e)
        {
            for (int i = 0; i < m_nCanOpenDeviceNum; ++i)
            {
                int nRet;

                nRet = m_pMyCamera[i].MV_CC_CloseDevice_NET();
                if (MyCamera.MV_OK != nRet)
                {
                    return;
                }

                nRet = m_pMyCamera[i].MV_CC_DestroyDevice_NET();
                if (MyCamera.MV_OK != nRet)
                {
                    return;
                }
            }

            //控件操作 ch: | en:Control Operation
            SetCtrlWhenClose();
            // ch:取流标志位清零 | en:Zero setting grabbing flag bit
            m_bGrabbing = false;
            // ch:重置成员变量 | en:Reset member variable
            ResetMember();
        }

        // ch:连续采集 | en:
        private void bnContinuesMode_CheckedChanged(object sender, EventArgs e)
        {
            if (bnContinuesMode.Checked)
            {
                for (int i = 0; i < m_nCanOpenDeviceNum; ++i)
                {
                    m_pMyCamera[i].MV_CC_SetEnumValue_NET("TriggerMode", (uint)MyCamera.MV_CAM_TRIGGER_MODE.MV_TRIGGER_MODE_OFF);
                    cbSoftTrigger.Enabled = false;
                    bnTriggerExec.Enabled = false;
                    bnSaveBmp.Enabled = true;
                }
            }
        }

        // ch:打开触发模式 | en:Open Trigger Mode
        private void bnTriggerMode_CheckedChanged(object sender, EventArgs e)
        {
            if (bnTriggerMode.Checked)
            {
                for (int i = 0; i < m_nCanOpenDeviceNum; ++i)
                {
                    m_pMyCamera[i].MV_CC_SetEnumValue_NET("TriggerMode", (uint)MyCamera.MV_CAM_TRIGGER_MODE.MV_TRIGGER_MODE_ON);

                    // ch:触发源选择:0 - Line0; | en:Trigger source select:0 - Line0;
                    //           1 - Line1;
                    //           2 - Line2;
                    //           3 - Line3;
                    //           4 - Counter;
                    //           7 - Software;
                    if (cbSoftTrigger.Checked)
                    {
                        m_pMyCamera[i].MV_CC_SetEnumValue_NET("TriggerSource", (uint)MyCamera.MV_CAM_TRIGGER_SOURCE.MV_TRIGGER_SOURCE_SOFTWARE);
                        if (m_bGrabbing)
                        {
                            bnTriggerExec.Enabled = true;
                        }
                    }
                    else
                    {
                        m_pMyCamera[i].MV_CC_SetEnumValue_NET("TriggerSource", (uint)MyCamera.MV_CAM_TRIGGER_SOURCE.MV_TRIGGER_SOURCE_LINE0);
                    }
                    cbSoftTrigger.Enabled = true;
                    bnSaveBmp.Enabled = false;
                }
            }
        }

        private void SetCtrlWhenStartGrab()
        {
            bnStartGrab.Enabled = false;
            bnStopGrab.Enabled = true;
            bnClose.Enabled = false;
 
            if (bnTriggerMode.Checked && cbSoftTrigger.Checked)
            {
                bnTriggerExec.Enabled = true;
            }

            bnSaveBmp.Enabled = true;
        }

        // ch:取流回调函数 | en:Aquisition Callback Function
        private void ImageCallBack(IntPtr pData, ref MyCamera.MV_FRAME_OUT_INFO_EX pFrameInfo, IntPtr pUser)
        {
            int nIndex=(int) pUser;

            // ch:抓取的帧数 | en:Aquired Frame Number
            ++m_nFrames[nIndex];

            lock (m_BufForSaveImageLock[nIndex])
            {
                if (m_pSaveImageBuf[nIndex] == IntPtr.Zero || pFrameInfo.nFrameLen > m_nSaveImageBufSize[nIndex])
                {
                    if (m_pSaveImageBuf[nIndex] != IntPtr.Zero)
                    {
                        Marshal.Release(m_pSaveImageBuf[nIndex]);
                        m_pSaveImageBuf[nIndex] = IntPtr.Zero;
                    }

                    m_pSaveImageBuf[nIndex] = Marshal.AllocHGlobal((Int32)pFrameInfo.nFrameLen);
                    if (m_pSaveImageBuf[nIndex] == IntPtr.Zero)
                    {
                        return;
                    }
                    m_nSaveImageBufSize[nIndex] = pFrameInfo.nFrameLen;
                }

                m_stFrameInfo[nIndex] = pFrameInfo;
                CopyMemory(m_pSaveImageBuf[nIndex], pData, pFrameInfo.nFrameLen);
            }

            MyCamera.MV_DISPLAY_FRAME_INFO stDisplayInfo = new MyCamera.MV_DISPLAY_FRAME_INFO();
            stDisplayInfo.hWnd = m_hDisplayHandle[nIndex];
            stDisplayInfo.pData = pData;
            stDisplayInfo.nDataLen = pFrameInfo.nFrameLen;
            stDisplayInfo.nWidth = pFrameInfo.nWidth;
            stDisplayInfo.nHeight = pFrameInfo.nHeight;
            stDisplayInfo.enPixelType = pFrameInfo.enPixelType;

            m_pMyCamera[nIndex].MV_CC_DisplayOneFrame_NET(ref stDisplayInfo);
       }

        private void bnStartGrab_Click(object sender, EventArgs e)
        {
            int nRet;
            m_hDisplayHandle[0] = pictureBox1.Handle;
            m_hDisplayHandle[1] = pictureBox2.Handle;
            m_hDisplayHandle[2] = pictureBox3.Handle;
            m_hDisplayHandle[3] = pictureBox4.Handle;

            // ch:开始采集 | en:Start Grabbing
            for (int i = 0; i < m_nCanOpenDeviceNum; i++)
            {
                m_nFrames[i] = 0;
                m_stFrameInfo[i].nFrameLen = 0;//取流之前先清除帧长度
                m_stFrameInfo[i].enPixelType = MyCamera.MvGvspPixelType.PixelType_Gvsp_Undefined;
                nRet = m_pMyCamera[i].MV_CC_StartGrabbing_NET();
                if (MyCamera.MV_OK != nRet)
                {
                    richTextBox.Text += String.Format("No.{0} Start Grabbing failed! nRet=0x{1}\r\n", (i + 1).ToString(), nRet.ToString("X"));
                }
            }

            //ch:开始计时  | en:Start Timing
            m_bTimerFlag = true;     
            // ch:控件操作 | en:Control Operation
            SetCtrlWhenStartGrab();
            // ch:标志位置位true | en:Set Position Bit true
            m_bGrabbing = true;
        }

        private void cbSoftTrigger_CheckedChanged(object sender, EventArgs e)
        {
            if (cbSoftTrigger.Checked)
            {
                // ch:触发源设为软触发 | en:Set Trigger Source As Software
                for (int i = 0; i < m_nCanOpenDeviceNum; ++i)
                {
                    m_pMyCamera[i].MV_CC_SetEnumValue_NET("TriggerSource", (uint)MyCamera.MV_CAM_TRIGGER_SOURCE.MV_TRIGGER_SOURCE_SOFTWARE);
                }
                if (m_bGrabbing)
                {
                    bnTriggerExec.Enabled = true;
                }
            }
            else
            {
                bnTriggerExec.Enabled = false;
            }
        }

        // ch:触发命令 | en:Trigger Command
        private void bnTriggerExec_Click(object sender, EventArgs e)
        {
            int nRet;

            for (int i = 0; i < m_nCanOpenDeviceNum; ++i)
            {
                nRet = m_pMyCamera[i].MV_CC_SetCommandValue_NET("TriggerSoftware");
                if (MyCamera.MV_OK != nRet)
                {
                    richTextBox.Text += String.Format("No.{0} Set software trigger failed! nRet=0x{1}\r\n", (i + 1).ToString(), nRet.ToString("X"));
                }
            }
        }

        private void SetCtrlWhenStopGrab()
        {
            bnStartGrab.Enabled = true;
            bnStopGrab.Enabled = false;
            bnClose.Enabled = true;

            bnTriggerExec.Enabled = false;
            bnSaveBmp.Enabled = false;
        }

        //停止采集 ch: | en:Stop Grabbing
        private void bnStopGrab_Click(object sender, EventArgs e)
        {
            for (int i = 0; i < m_nCanOpenDeviceNum; ++i)
            {
                m_pMyCamera[i].MV_CC_StopGrabbing_NET();
            }
            //ch:标志位设为false  | en:Set Flag Bit false
            m_bGrabbing = false;
            // ch:停止计时 | en:Stop Timing
            m_bTimerFlag = false;

            // ch:控件操作 | en:Control Operation
            SetCtrlWhenStopGrab();
        }

        // ch:点击保存图片按钮 | en:Click on Save Image Button
        private void bnSaveBmp_Click(object sender, EventArgs e)
        {
            for (int i = 0; i < m_nCanOpenDeviceNum; ++i)
            {
                MyCamera.MV_SAVE_IMG_TO_FILE_PARAM stSaveParam = new MyCamera.MV_SAVE_IMG_TO_FILE_PARAM();
                lock (m_BufForSaveImageLock[i])
                {
                    if (m_stFrameInfo[i].nFrameLen == 0)
                    {
                        richTextBox.Text += String.Format("No.{0} save image failed! No data!\r\n", (i + 1).ToString());
                        continue;
                    }
                    stSaveParam.enImageType = MyCamera.MV_SAVE_IAMGE_TYPE.MV_Image_Bmp;
                    stSaveParam.enPixelType = m_stFrameInfo[i].enPixelType;
                    stSaveParam.pData = m_pSaveImageBuf[i];
                    stSaveParam.nDataLen = m_stFrameInfo[i].nFrameLen;
                    stSaveParam.nHeight = m_stFrameInfo[i].nHeight;
                    stSaveParam.nWidth = m_stFrameInfo[i].nWidth;
                    stSaveParam.pImagePath = "Dev"+i.ToString()+"_Image_w" + stSaveParam.nWidth.ToString() + "_h" + stSaveParam.nHeight.ToString() + "_fn" + m_stFrameInfo[i].nFrameNum.ToString() + ".bmp";
                    //stSaveParam.nQuality = 80;//存Jpeg时有效
                    int nRet = m_pMyCamera[i].MV_CC_SaveImageToFile_NET(ref stSaveParam);
                    if (MyCamera.MV_OK != nRet)
                    {
                        richTextBox.Text += String.Format("No.{0} save image failed! nRet=0x{1}\r\n", (i + 1).ToString(), nRet.ToString("X"));
                    }
                    else
                    {
                        richTextBox.Text += String.Format("No.{0} save image Succeed!\r\n", (i + 1).ToString());
                    }
                }
            }
        }

        // ch:设置曝光时间和增益 | en:Set Exposure Time and Gain
        private void bnSetParam_Click(object sender, EventArgs e)
        {
            try
            {
                float.Parse(tbExposure.Text);
                float.Parse(tbGain.Text);
            }
            catch
            {
                richTextBox.Text += String.Format("Please Enter Correct Type.\r\n");
                return;
            }
            if (float.Parse(tbExposure.Text) < 0 || float.Parse(tbGain.Text) < 0)
            {
                richTextBox.Text += String.Format("Set ExposureTime or Gain fail,Because ExposureTime or Gain less than zero.\r\n");
                return;
            }

            int nRet;
            for (int i = 0; i < m_nCanOpenDeviceNum; ++i)
            {
                bool bSuccess = true;
                m_pMyCamera[i].MV_CC_SetEnumValue_NET("ExposureAuto", 0);

                nRet = m_pMyCamera[i].MV_CC_SetFloatValue_NET("ExposureTime", float.Parse(tbExposure.Text));
                if (nRet != MyCamera.MV_OK)
                {
                    richTextBox.Text += String.Format("No.{0} Set Exposure Time Failed! nRet=0x{1}\r\n", (i + 1).ToString(), nRet.ToString("X"));
                    bSuccess = false;
                }

                m_pMyCamera[i].MV_CC_SetEnumValue_NET("GainAuto", 0);
                nRet = m_pMyCamera[i].MV_CC_SetFloatValue_NET("Gain", float.Parse(tbGain.Text));
                if (nRet != MyCamera.MV_OK)
                {
                    richTextBox.Text += String.Format("No.{0} Set Gain Failed! nRet=0x{1}\r\n", (i + 1).ToString(), nRet.ToString("X"));
                    bSuccess = false;
                }

                if (bSuccess)
                {
                    richTextBox.Text += String.Format("No.{0} Set Parameters Succeed!\r\n", (i + 1).ToString());
                }
            }
        }

        // ch:获取丢帧数 | en:Get Throw Frame Number
        private string GetLostFrame(int nIndex) 
        {
            MyCamera.MV_ALL_MATCH_INFO pstInfo = new MyCamera.MV_ALL_MATCH_INFO();
            if (m_pDeviceInfo[nIndex].nTLayerType == MyCamera.MV_GIGE_DEVICE)
            {
                MyCamera.MV_MATCH_INFO_NET_DETECT MV_NetInfo = new MyCamera.MV_MATCH_INFO_NET_DETECT();
                pstInfo.nInfoSize = (uint)System.Runtime.InteropServices.Marshal.SizeOf(typeof(MyCamera.MV_MATCH_INFO_NET_DETECT));
                pstInfo.nType = MyCamera.MV_MATCH_TYPE_NET_DETECT;
                int size = Marshal.SizeOf(MV_NetInfo);
                pstInfo.pInfo = Marshal.AllocHGlobal(size);
                Marshal.StructureToPtr(MV_NetInfo, pstInfo.pInfo, false);

                m_pMyCamera[nIndex].MV_CC_GetAllMatchInfo_NET(ref pstInfo);
                MV_NetInfo = (MyCamera.MV_MATCH_INFO_NET_DETECT)Marshal.PtrToStructure(pstInfo.pInfo, typeof(MyCamera.MV_MATCH_INFO_NET_DETECT));

                string sTemp = MV_NetInfo.nLostFrameCount.ToString();
                Marshal.FreeHGlobal(pstInfo.pInfo);
                return sTemp;
            }
            else if (m_pDeviceInfo[nIndex].nTLayerType == MyCamera.MV_USB_DEVICE)
            {
                MyCamera.MV_MATCH_INFO_USB_DETECT MV_NetInfo = new MyCamera.MV_MATCH_INFO_USB_DETECT();
                pstInfo.nInfoSize = (uint)System.Runtime.InteropServices.Marshal.SizeOf(typeof(MyCamera.MV_MATCH_INFO_USB_DETECT));
                pstInfo.nType = MyCamera.MV_MATCH_TYPE_USB_DETECT;
                int size = Marshal.SizeOf(MV_NetInfo);
                pstInfo.pInfo = Marshal.AllocHGlobal(size);
                Marshal.StructureToPtr(MV_NetInfo, pstInfo.pInfo, false);

                m_pMyCamera[nIndex].MV_CC_GetAllMatchInfo_NET(ref pstInfo);
                MV_NetInfo = (MyCamera.MV_MATCH_INFO_USB_DETECT)Marshal.PtrToStructure(pstInfo.pInfo, typeof(MyCamera.MV_MATCH_INFO_USB_DETECT));

                string sTemp = MV_NetInfo.nErrorFrameCount.ToString();
                Marshal.FreeHGlobal(pstInfo.pInfo);
                return sTemp;
            }
            else
            {
                return "0";
            }
        }

        // ch:定时器,1秒运行一次 | en:Timer, run once a second
        private void timer1_Tick(object sender, EventArgs e)
        {
            if (m_bTimerFlag)
            {
                if (m_nCanOpenDeviceNum > 0)
                {
                    tbGrabFrame1.Text = m_nFrames[0].ToString();
                    tbLostFrame1.Text = GetLostFrame(0);
                }
                if (m_nCanOpenDeviceNum > 1)
                {
                    tbGrabFrame2.Text = m_nFrames[1].ToString();
                    tbLostFrame2.Text = GetLostFrame(1);
                }
                if (m_nCanOpenDeviceNum > 2)
                {
                    tbGrabFrame3.Text = m_nFrames[2].ToString();
                    tbLostFrame3.Text = GetLostFrame(2);
                }
                if (m_nCanOpenDeviceNum > 3)
                {
                    tbGrabFrame4.Text = m_nFrames[3].ToString();
                    tbLostFrame4.Text = GetLostFrame(3);
                }
            }
        }

        private void MultipleDemo_FormClosing(object sender, FormClosingEventArgs e)
        {
            bnClose_Click(sender, e);

            // ch: 反初始化SDK | en: Finalize SDK
            MyCamera.MV_CC_Finalize_NET();
        }

        private void richTextBox_DoubleClick(object sender, EventArgs e)
        {
            richTextBox.Clear();
        }

        private void richTextBox_TextChanged(object sender, EventArgs e)
        {
            richTextBox.SelectionStart = richTextBox.Text.Length;
            richTextBox.ScrollToCaret();
        }
    }
}
