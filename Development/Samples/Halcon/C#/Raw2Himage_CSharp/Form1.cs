using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using HalconDotNet;
using MvCamCtrl.NET;
using System.Runtime.InteropServices;
using System.Threading;
using System.IO;
using System.Drawing.Imaging;
using System.Diagnostics;
using System.Collections.ObjectModel;

namespace Raw2Himage_CSharp
{
    public partial class Form1 : Form
    {
        MyCamera.MV_CC_DEVICE_INFO_LIST m_pDeviceList;
        private MyCamera m_pMyCamera;
        bool m_bGrabbing;
        HWindow m_Window;

        public Form1()
        {
            InitializeComponent();
            m_pDeviceList = new MyCamera.MV_CC_DEVICE_INFO_LIST();
            m_pMyCamera = new MyCamera();
            m_bGrabbing = false;
            m_Window = new HWindow();
            DisplayWindowsInitial();
            DeviceListAcq();
        }
/********************************************************************************************/
        private void DisplayWindowsInitial()
        {
            // ch: 定义显示的起点和宽高 || en: Definition the width and height of the display window
            HTuple hWindowRow, hWindowColumn, hWindowWidth, hWindowHeight;

            // ch: 设置显示窗口的起点和宽高 || en: Set the width and height of the display window
            hWindowRow = 0;
            hWindowColumn = 0;
            hWindowWidth = pictureBox1.Width;
            hWindowHeight = pictureBox1.Height;

            try
            {
                HTuple hWindowID = (HTuple)pictureBox1.Handle;
                m_Window.OpenWindow(hWindowRow, hWindowColumn, hWindowWidth, hWindowHeight, hWindowID, "visible", "");
            }
            catch (System.Exception ex)
            {
                MessageBox.Show(ex.ToString());
                return;
            }

        }
        private void bnEnum_Click(object sender, EventArgs e)
        {
            DeviceListAcq();
        }

        private void DeviceListAcq()
        {
            int nRet;
            // ch:创建设备列表 || en: Create device list
            System.GC.Collect();
            cbDeviceList.Items.Clear();
            nRet = MyCamera.MV_CC_EnumDevices_NET(MyCamera.MV_GIGE_DEVICE | MyCamera.MV_USB_DEVICE, ref m_pDeviceList);
            if (MyCamera.MV_OK != nRet)
            {
                MessageBox.Show("Enum Devices Fail");
                return;
            }

            // ch:在窗体列表中显示设备名 || Display the device'name on window's list
            for (int i = 0; i < m_pDeviceList.nDeviceNum; i++)
            {
                MyCamera.MV_CC_DEVICE_INFO device = (MyCamera.MV_CC_DEVICE_INFO)Marshal.PtrToStructure(m_pDeviceList.pDeviceInfo[i], typeof(MyCamera.MV_CC_DEVICE_INFO));
                if (device.nTLayerType == MyCamera.MV_GIGE_DEVICE)
                {
                    IntPtr buffer = Marshal.UnsafeAddrOfPinnedArrayElement(device.SpecialInfo.stGigEInfo, 0);
                    MyCamera.MV_GIGE_DEVICE_INFO gigeInfo = (MyCamera.MV_GIGE_DEVICE_INFO)Marshal.PtrToStructure(buffer, typeof(MyCamera.MV_GIGE_DEVICE_INFO));
                    if (gigeInfo.chUserDefinedName != "")
                    {
                        cbDeviceList.Items.Add("GigE: " + gigeInfo.chUserDefinedName + " (" + gigeInfo.chSerialNumber + ")");
                    }
                    else
                    {
                        cbDeviceList.Items.Add("GigE: " + gigeInfo.chManufacturerName + " " + gigeInfo.chModelName + " (" + gigeInfo.chSerialNumber + ")");
                    }
                }
                else if (device.nTLayerType == MyCamera.MV_USB_DEVICE)
                {
                    IntPtr buffer = Marshal.UnsafeAddrOfPinnedArrayElement(device.SpecialInfo.stUsb3VInfo, 0);
                    MyCamera.MV_USB3_DEVICE_INFO usbInfo = (MyCamera.MV_USB3_DEVICE_INFO)Marshal.PtrToStructure(buffer, typeof(MyCamera.MV_USB3_DEVICE_INFO));
                    if (usbInfo.chUserDefinedName != "")
                    {
                        cbDeviceList.Items.Add("USB: " + usbInfo.chUserDefinedName + " (" + usbInfo.chSerialNumber + ")");
                    }
                    else
                    {
                        cbDeviceList.Items.Add("USB: " + usbInfo.chManufacturerName + " " + usbInfo.chModelName + " (" + usbInfo.chSerialNumber + ")");
                    }
                }
            }

            //.ch: 选择第一项 || en: Select the first item
            if (m_pDeviceList.nDeviceNum != 0)
            {
                cbDeviceList.SelectedIndex = 0;
            }
        }

        MyCamera.cbOutputExdelegate ImageCallback;
        private void bnOpen_Click(object sender, EventArgs e)
        {
            if (m_pDeviceList.nDeviceNum == 0 || cbDeviceList.SelectedIndex == -1)
            {
                MessageBox.Show("No device,please select");
                return;
            }
            int nRet = -1;

            //ch:获取选择的设备信息 | en:Get selected device information
            MyCamera.MV_CC_DEVICE_INFO device =
                (MyCamera.MV_CC_DEVICE_INFO)Marshal.PtrToStructure(m_pDeviceList.pDeviceInfo[cbDeviceList.SelectedIndex],
                                                              typeof(MyCamera.MV_CC_DEVICE_INFO));

            nRet = m_pMyCamera.MV_CC_CreateDevice_NET(ref device);
            if (MyCamera.MV_OK != nRet)
            {
                return;
            }

            // ch:打开设备 | en:Open device
            nRet = m_pMyCamera.MV_CC_OpenDevice_NET();
            if (MyCamera.MV_OK != nRet)
            {
                MessageBox.Show("Open Device Fail");
                return;
            }

            // ch:设置触发模式为off || en:set trigger mode as off
            m_pMyCamera.MV_CC_SetEnumValue_NET("AcquisitionMode", 2);
            m_pMyCamera.MV_CC_SetEnumValue_NET("TriggerMode", 0);

            SetCtrlWhenOpen();

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
        }

        private void bnClose_Click(object sender, EventArgs e)
        {
            if (m_bGrabbing)
            {
                m_bGrabbing = false;
                // ch:停止抓图 || en:Stop grab image
                m_pMyCamera.MV_CC_StopGrabbing_NET();

                // ch: 控件操作 || en: Control operation
                SetCtrlWhenStopGrab();
            }

            // ch:关闭设备 || en: Close device
            m_pMyCamera.MV_CC_CloseDevice_NET();

            // ch: 控件操作 || en: Control operation
            SetCtrlWhenClose();

            m_bGrabbing = false;
        }

        private void bnContinuesMode_CheckedChanged(object sender, EventArgs e)
        {
            int nRet = MyCamera.MV_OK;
            if (bnContinuesMode.Checked)
            {
                nRet = m_pMyCamera.MV_CC_SetEnumValue_NET("TriggerMode", 0);
                if (nRet != MyCamera.MV_OK)
                {
                    MessageBox.Show("Set TriggerMode Fail");
                    return;
                }
                cbSoftTrigger.Enabled = false;
                bnTriggerExec.Enabled = false;
            }
        }

        private void bnTriggerMode_CheckedChanged(object sender, EventArgs e)
        {
            int nRet = MyCamera.MV_OK;
            if (bnTriggerMode.Checked)
            {
                nRet = m_pMyCamera.MV_CC_SetEnumValue_NET("TriggerMode", 1);
                if (nRet != MyCamera.MV_OK)
                {
                    MessageBox.Show("Set TriggerMode Fail");
                    return;
                }

                // ch: 触发源选择:0 - Line0 || en :TriggerMode select;
                //           1 - Line1;
                //           2 - Line2;
                //           3 - Line3;
                //           4 - Counter;
                //           7 - Software;
                if (cbSoftTrigger.Checked)
                {
                    nRet = m_pMyCamera.MV_CC_SetEnumValue_NET("TriggerSource", 7);
                    if (nRet != MyCamera.MV_OK)
                    {
                        MessageBox.Show("Set TriggerSource Fail");
                        return;
                    }
                    if (m_bGrabbing)
                    {
                        bnTriggerExec.Enabled = true;
                    }
                }
                else
                {
                    nRet = m_pMyCamera.MV_CC_SetEnumValue_NET("TriggerSource", 0);
                    if (nRet != MyCamera.MV_OK)
                    {
                        MessageBox.Show("Set TriggerSource Fail");
                        return;
                    }
                }
                cbSoftTrigger.Enabled = true;
            }
        }

        private void SetCtrlWhenStartGrab()
        {

            bnStartGrab.Enabled = false;
            bnStopGrab.Enabled = true;

            if (bnTriggerMode.Checked && cbSoftTrigger.Checked)
            {
                bnTriggerExec.Enabled = true;
            }
        }

        private bool IsMonoPixelFormat(MyCamera.MvGvspPixelType enType)
        {
            switch (enType)
            {
                case MyCamera.MvGvspPixelType.PixelType_Gvsp_Mono8:
                case MyCamera.MvGvspPixelType.PixelType_Gvsp_Mono10:
                case MyCamera.MvGvspPixelType.PixelType_Gvsp_Mono10_Packed:
                case MyCamera.MvGvspPixelType.PixelType_Gvsp_Mono12:
                case MyCamera.MvGvspPixelType.PixelType_Gvsp_Mono12_Packed:
                    return true;
                default:
                    return false;
            }
        }

        private bool IsColorPixelFormat(MyCamera.MvGvspPixelType enType)
        {
            switch (enType)
            {
                case MyCamera.MvGvspPixelType.PixelType_Gvsp_RGB8_Packed:
                case MyCamera.MvGvspPixelType.PixelType_Gvsp_BGR8_Packed:
                case MyCamera.MvGvspPixelType.PixelType_Gvsp_RGBA8_Packed:
                case MyCamera.MvGvspPixelType.PixelType_Gvsp_BGRA8_Packed:
                case MyCamera.MvGvspPixelType.PixelType_Gvsp_YUV422_Packed:
                case MyCamera.MvGvspPixelType.PixelType_Gvsp_YUV422_YUYV_Packed:
                case MyCamera.MvGvspPixelType.PixelType_Gvsp_BayerGR8:
                case MyCamera.MvGvspPixelType.PixelType_Gvsp_BayerRG8:
                case MyCamera.MvGvspPixelType.PixelType_Gvsp_BayerGB8:
                case MyCamera.MvGvspPixelType.PixelType_Gvsp_BayerBG8:
                case MyCamera.MvGvspPixelType.PixelType_Gvsp_BayerGB10:
                case MyCamera.MvGvspPixelType.PixelType_Gvsp_BayerGB10_Packed:
                case MyCamera.MvGvspPixelType.PixelType_Gvsp_BayerBG10:
                case MyCamera.MvGvspPixelType.PixelType_Gvsp_BayerBG10_Packed:
                case MyCamera.MvGvspPixelType.PixelType_Gvsp_BayerRG10:
                case MyCamera.MvGvspPixelType.PixelType_Gvsp_BayerRG10_Packed:
                case MyCamera.MvGvspPixelType.PixelType_Gvsp_BayerGR10:
                case MyCamera.MvGvspPixelType.PixelType_Gvsp_BayerGR10_Packed:
                case MyCamera.MvGvspPixelType.PixelType_Gvsp_BayerGB12:
                case MyCamera.MvGvspPixelType.PixelType_Gvsp_BayerGB12_Packed:
                case MyCamera.MvGvspPixelType.PixelType_Gvsp_BayerBG12:
                case MyCamera.MvGvspPixelType.PixelType_Gvsp_BayerBG12_Packed:
                case MyCamera.MvGvspPixelType.PixelType_Gvsp_BayerRG12:
                case MyCamera.MvGvspPixelType.PixelType_Gvsp_BayerRG12_Packed:
                case MyCamera.MvGvspPixelType.PixelType_Gvsp_BayerGR12:
                case MyCamera.MvGvspPixelType.PixelType_Gvsp_BayerGR12_Packed:
                    return true;
                default:
                    return false;
            }
        }

        public void HalconDisplay(HTuple hWindow, HObject Hobj, HTuple hHeight, HTuple hWidth)
        {
            // ch: 显示 || display
            try
            {
                HOperatorSet.SetPart(hWindow, 0, 0, hHeight - 1, hWidth - 1);// ch: 使图像显示适应窗口大小 || en: Make the image adapt the window size
            }
            catch (System.Exception ex)
            {
                MessageBox.Show(ex.ToString());
                return;
            }
            if (hWindow == null)
            {
                return;
            }
            try
            {
                HOperatorSet.DispObj(Hobj, hWindow);// ch 显示 || en: display
            }
            catch (System.Exception ex)
            {
                MessageBox.Show(ex.ToString());
                return;
            }
            return;
        }

        public void ReceiveImageWorkThread(object obj)
        {
            int nRet = MyCamera.MV_OK;
            MyCamera device = obj as MyCamera;
            MyCamera.MV_FRAME_OUT stFrameOut = new MyCamera.MV_FRAME_OUT();

            IntPtr pImageBuf = IntPtr.Zero;
            int nImageBufSize = 0;

            HObject Hobj = new HObject();
            IntPtr pTemp = IntPtr.Zero;

            while (m_bGrabbing)
            {
                nRet = device.MV_CC_GetImageBuffer_NET(ref stFrameOut, 1000);
                if (MyCamera.MV_OK == nRet)
                {
                    if (IsColorPixelFormat(stFrameOut.stFrameInfo.enPixelType))
                    {
                        if (stFrameOut.stFrameInfo.enPixelType == MyCamera.MvGvspPixelType.PixelType_Gvsp_RGB8_Packed)
                        {
                            pTemp = stFrameOut.pBufAddr;
                        }
                        else
                        {
                            if(IntPtr.Zero == pImageBuf || nImageBufSize < (stFrameOut.stFrameInfo.nWidth * stFrameOut.stFrameInfo.nHeight * 3))
                            {
                                if (pImageBuf != IntPtr.Zero)
                                {
                                    Marshal.FreeHGlobal(pImageBuf);
                                    pImageBuf = IntPtr.Zero;
                                }

                                pImageBuf = Marshal.AllocHGlobal((int)stFrameOut.stFrameInfo.nWidth * stFrameOut.stFrameInfo.nHeight * 3);
                                if (IntPtr.Zero == pImageBuf)
                                {
                                    break;
                                }
                                nImageBufSize = stFrameOut.stFrameInfo.nWidth * stFrameOut.stFrameInfo.nHeight * 3;
                            }

                            MyCamera.MV_PIXEL_CONVERT_PARAM stPixelConvertParam = new MyCamera.MV_PIXEL_CONVERT_PARAM();

                            stPixelConvertParam.pSrcData = stFrameOut.pBufAddr;//源数据
                            stPixelConvertParam.nWidth = stFrameOut.stFrameInfo.nWidth;//图像宽度
                            stPixelConvertParam.nHeight = stFrameOut.stFrameInfo.nHeight;//图像高度
                            stPixelConvertParam.enSrcPixelType = stFrameOut.stFrameInfo.enPixelType;//源数据的格式
                            stPixelConvertParam.nSrcDataLen = stFrameOut.stFrameInfo.nFrameLen;

                            stPixelConvertParam.nDstBufferSize = (uint)nImageBufSize;
                            stPixelConvertParam.pDstBuffer = pImageBuf;//转换后的数据
                            stPixelConvertParam.enDstPixelType = MyCamera.MvGvspPixelType.PixelType_Gvsp_RGB8_Packed;
                            nRet = device.MV_CC_ConvertPixelType_NET(ref stPixelConvertParam);//格式转换
                            if (MyCamera.MV_OK != nRet)
                            {
                                break;
                            }
                            pTemp = pImageBuf;
                        }

                        try
                        {
                            HOperatorSet.GenImageInterleaved(out Hobj, (HTuple)pTemp, (HTuple)"rgb", (HTuple)stFrameOut.stFrameInfo.nWidth, (HTuple)stFrameOut.stFrameInfo.nHeight, -1, "byte", 0, 0, 0, 0, -1, 0);
                        }
                        catch (System.Exception ex)
                        {
                            MessageBox.Show(ex.ToString());
                            break;
                        }
                    }
                    else if (IsMonoPixelFormat(stFrameOut.stFrameInfo.enPixelType))
                    {
                        if (stFrameOut.stFrameInfo.enPixelType == MyCamera.MvGvspPixelType.PixelType_Gvsp_Mono8)
                        {
                            pTemp = stFrameOut.pBufAddr;
                        }
                        else
                        {
                            if (IntPtr.Zero == pImageBuf || nImageBufSize < (stFrameOut.stFrameInfo.nWidth * stFrameOut.stFrameInfo.nHeight))
                            {
                                if (pImageBuf != IntPtr.Zero)
                                {
                                    Marshal.FreeHGlobal(pImageBuf);
                                    pImageBuf = IntPtr.Zero;
                                }

                                pImageBuf = Marshal.AllocHGlobal((int)stFrameOut.stFrameInfo.nWidth * stFrameOut.stFrameInfo.nHeight);
                                if (IntPtr.Zero == pImageBuf)
                                {
                                    break;
                                }
                                nImageBufSize = stFrameOut.stFrameInfo.nWidth * stFrameOut.stFrameInfo.nHeight;
                            }

                            MyCamera.MV_PIXEL_CONVERT_PARAM stPixelConvertParam = new MyCamera.MV_PIXEL_CONVERT_PARAM();

                            stPixelConvertParam.pSrcData = stFrameOut.pBufAddr;//源数据
                            stPixelConvertParam.nWidth = stFrameOut.stFrameInfo.nWidth;//图像宽度
                            stPixelConvertParam.nHeight = stFrameOut.stFrameInfo.nHeight;//图像高度
                            stPixelConvertParam.enSrcPixelType = stFrameOut.stFrameInfo.enPixelType;//源数据的格式
                            stPixelConvertParam.nSrcDataLen = stFrameOut.stFrameInfo.nFrameLen;

                            stPixelConvertParam.nDstBufferSize = (uint)nImageBufSize;
                            stPixelConvertParam.pDstBuffer = pImageBuf;//转换后的数据
                            stPixelConvertParam.enDstPixelType = MyCamera.MvGvspPixelType.PixelType_Gvsp_Mono8;
                            nRet = device.MV_CC_ConvertPixelType_NET(ref stPixelConvertParam);//格式转换
                            if (MyCamera.MV_OK != nRet)
                            {
                                break;
                            }
                            pTemp = pImageBuf;
                        }
                        try
                        {
                            HOperatorSet.GenImage1Extern(out Hobj, "byte", stFrameOut.stFrameInfo.nWidth, stFrameOut.stFrameInfo.nHeight, pTemp, IntPtr.Zero);
                        }
                        catch (System.Exception ex)
                        {
                            MessageBox.Show(ex.ToString());
                            break;
                        }
                    }
                    else
                    {
                        device.MV_CC_FreeImageBuffer_NET(ref stFrameOut);
                        continue;
                    }
                    HalconDisplay(m_Window, Hobj, stFrameOut.stFrameInfo.nHeight, stFrameOut.stFrameInfo.nWidth);

                    device.MV_CC_FreeImageBuffer_NET(ref stFrameOut);
                }
                else
                {
                    continue;
                }
            }

            if (pImageBuf != IntPtr.Zero)
            {
                Marshal.FreeHGlobal(pImageBuf);
                pImageBuf = IntPtr.Zero;
            }
        }

        private void bnStartGrab_Click(object sender, EventArgs e)
        {
            // ch:开启抓图 | en:start grab
            int nRet = m_pMyCamera.MV_CC_StartGrabbing_NET();
            if (MyCamera.MV_OK != nRet)
            {
                MessageBox.Show("Start Grabbing Fail");
                return;
            }
            m_bGrabbing = true;

            Thread hReceiveImageThreadHandle = new Thread(ReceiveImageWorkThread);
            hReceiveImageThreadHandle.Start(m_pMyCamera);

            // ch: 控件操作 || en: Control operation
            SetCtrlWhenStartGrab();
        }

        private void cbSoftTrigger_CheckedChanged(object sender, EventArgs e)
        {
            if (cbSoftTrigger.Checked)
            {

                // ch: 触发源设为软触发 || en: set trigger mode as Software
                m_pMyCamera.MV_CC_SetEnumValue_NET("TriggerSource", 7);
                if (m_bGrabbing)
                {
                    bnTriggerExec.Enabled = true;
                }
            }
            else
            {
                m_pMyCamera.MV_CC_SetEnumValue_NET("TriggerSource", 0);
                bnTriggerExec.Enabled = false;
            }
        }

        private void bnTriggerExec_Click(object sender, EventArgs e)
        {
            // ch: 触发命令 || en: Trigger command
            int nRet = m_pMyCamera.MV_CC_SetCommandValue_NET("TriggerSoftware");
            if (MyCamera.MV_OK != nRet)
            {
                MessageBox.Show("Trigger Fail");
            }
        }

        private void SetCtrlWhenStopGrab()
        {
            bnStartGrab.Enabled = true;
            bnStopGrab.Enabled = false;

            bnTriggerExec.Enabled = false;
        }

        private void bnStopGrab_Click(object sender, EventArgs e)
        {
            // ch:停止抓图 || en:Stop grab image
            int nRet = m_pMyCamera.MV_CC_StopGrabbing_NET();
            if (nRet != MyCamera.MV_OK)
            {
                MessageBox.Show("Stop Grabbing Fail");
            }
            m_bGrabbing = false;
            // ch: 控件操作 || en: Control operation
            SetCtrlWhenStopGrab();
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            bnClose_Click(sender, e);
        }
    }
}
