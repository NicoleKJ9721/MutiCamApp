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

using System.Drawing.Imaging;
using System.Diagnostics;
using System.Collections.ObjectModel;

namespace InfraredDemo
{
    public partial class InfraredDemo : Form
    {
        [DllImport("kernel32.dll", EntryPoint = "RtlMoveMemory", SetLastError = false)]
        private static extern void CopyMemory(IntPtr dest, IntPtr src, uint count);

        // ch:判断用户自定义像素格式 | en:Determine custom pixel format
        public const Int32 CUSTOMER_PIXEL_FORMAT = unchecked((Int32)0x80000000);
        
        MyCamera.MV_CC_DEVICE_INFO_LIST m_stDeviceList = new MyCamera.MV_CC_DEVICE_INFO_LIST();
        private MyCamera m_MyCamera = new MyCamera();
        bool m_bGrabbing = false;
        Thread m_hReceiveThread = null;
        MyCamera.MV_FRAME_OUT_INFO_EX m_stFrameInfo = new MyCamera.MV_FRAME_OUT_INFO_EX();

        // ch:用于从驱动获取图像的缓存 | en:Buffer for getting image from driver
        UInt32 m_nBufSizeForDriver = 0;
        IntPtr m_BufForDriver = IntPtr.Zero;
        private static Object BufForDriverLock = new Object();

        // ch:Bitmap及其像素格式 | en:Bitmap and Pixel Format
        Bitmap m_bitmap = null;
        PixelFormat m_bitmapPixelFormat = PixelFormat.DontCare;
        IntPtr m_ConvertDstBuf = IntPtr.Zero;
        UInt32 m_nConvertDstBufLen = 0;

        bool m_bOpenDevice;                        // ch:是否打开设备 | en:Whether to open device
        bool m_bStartGrabbing;                     // ch:是否开始抓图 | en:Whether to start grabbing

        public InfraredDemo()
        {
            InitializeComponent();
            m_bOpenDevice = false;
            m_bStartGrabbing = false;
            EnableControls(false);
            this.Load += new EventHandler(this.InfraredDemo_Load);
        }

        private void InfraredDemo_Load(object sender, EventArgs e)
        {
            // ch: 初始化 SDK | en: Initialize SDK
            MyCamera.MV_CC_Initialize_NET();
        }

        public static FormRegionSetting RegionSettingForm = null;   // 区域设置界面
        public static FormAlarmSetting AlarmSettingForm = null;   // 告警设置界面

        // ch:显示错误信息 | en:Show error message
        private void ShowErrorMsg(string csMessage, int nErrorNum)
        {
            string errorMsg;
            if (nErrorNum == 0)
            {
                errorMsg = csMessage;
            }
            else
            {
                errorMsg = csMessage + ": Error =" + String.Format("{0:X}", nErrorNum);
            }

            switch (nErrorNum)
            {
                case MyCamera.MV_E_HANDLE: errorMsg += "Error or invalid handle "; break;
                case MyCamera.MV_E_SUPPORT: errorMsg += "Not supported function "; break;
                case MyCamera.MV_E_BUFOVER: errorMsg += "Cache is full "; break;
                case MyCamera.MV_E_CALLORDER: errorMsg += "Function calling order error "; break;
                case MyCamera.MV_E_PARAMETER: errorMsg += "Incorrect parameter "; break;
                case MyCamera.MV_E_RESOURCE: errorMsg += "Applying resource failed "; break;
                case MyCamera.MV_E_NODATA: errorMsg += "No data "; break;
                case MyCamera.MV_E_PRECONDITION: errorMsg += "Precondition error, or running environment changed "; break;
                case MyCamera.MV_E_VERSION: errorMsg += "Version mismatches "; break;
                case MyCamera.MV_E_NOENOUGH_BUF: errorMsg += "Insufficient memory "; break;
                case MyCamera.MV_E_ABNORMAL_IMAGE: errorMsg += "Abnormal image, maybe incomplete image because of lost packet "; break;
                case MyCamera.MV_E_UNKNOW: errorMsg += "Unknown error "; break;
                case MyCamera.MV_E_GC_GENERIC: errorMsg += "General error "; break;
                case MyCamera.MV_E_GC_ACCESS: errorMsg += "Node accessing condition error "; break;
                case MyCamera.MV_E_ACCESS_DENIED: errorMsg += "No permission "; break;
                case MyCamera.MV_E_BUSY: errorMsg += "Device is busy, or network disconnected "; break;
                case MyCamera.MV_E_NETER: errorMsg += "Network error "; break;
            }

            MessageBox.Show(errorMsg, "PROMPT");
        }

        private void bnEnum_Click(object sender, EventArgs e)
        {
            DeviceListAcq();
            EnableControls(true);
        }

        private void DeviceListAcq()
        {
            // ch:创建设备列表 | en:Create Device List
            System.GC.Collect();
            cbDeviceList.Items.Clear();
            m_stDeviceList.nDeviceNum = 0;
            int nRet = MyCamera.MV_CC_EnumDevices_NET(MyCamera.MV_GIGE_DEVICE | MyCamera.MV_USB_DEVICE, ref m_stDeviceList);
            if (0 != nRet)
            {
                ShowErrorMsg("Enumerate devices fail!", 0);
                return;
            }

            // ch:在窗体列表中显示设备名 | en:Display device name in the form list
            for (int i = 0; i < m_stDeviceList.nDeviceNum; i++)
            {
                MyCamera.MV_CC_DEVICE_INFO device = (MyCamera.MV_CC_DEVICE_INFO)Marshal.PtrToStructure(m_stDeviceList.pDeviceInfo[i], typeof(MyCamera.MV_CC_DEVICE_INFO));
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
                        cbDeviceList.Items.Add("GEV: " + strUserDefinedName + " (" + gigeInfo.chSerialNumber + ")");
                    }
                    else
                    {
                        cbDeviceList.Items.Add("GEV: " + gigeInfo.chManufacturerName + " " + gigeInfo.chModelName + " (" + gigeInfo.chSerialNumber + ")");
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
                        cbDeviceList.Items.Add("U3V: " + strUserDefinedName + " (" + usbInfo.chSerialNumber + ")");
                    }
                    else
                    {
                        cbDeviceList.Items.Add("U3V: " + usbInfo.chManufacturerName + " " + usbInfo.chModelName + " (" + usbInfo.chSerialNumber + ")");
                    }
                }
            }

            // ch:选择第一项 | en:Select the first item
            if (m_stDeviceList.nDeviceNum != 0)
            {
                cbDeviceList.SelectedIndex = 0;
            }
        }

        public int SetEnumIntoCombo(string strKey, ref ComboBox ctrlComboBox)
        {
            string str = ctrlComboBox.SelectedItem.ToString();
            MyCamera.MVCC_ENUMENTRY stEnumInfo = new MyCamera.MVCC_ENUMENTRY();
            MyCamera.MVCC_ENUMVALUE stEnumValue = new MyCamera.MVCC_ENUMVALUE();
            int nRet = m_MyCamera.MV_CC_GetEnumValue_NET(strKey, ref stEnumValue);
            if (MyCamera.MV_OK != nRet)
            {
                return nRet;
            }
            for (int i = 0; i < stEnumValue.nSupportedNum; ++i)
            {
                stEnumInfo.nValue = stEnumValue.nSupportValue[i];
                nRet = m_MyCamera.MV_CC_GetEnumEntrySymbolic_NET(strKey, ref stEnumInfo);
                if (MyCamera.MV_OK == nRet && str.Equals(Encoding.Default.GetString(stEnumInfo.chSymbolic), StringComparison.OrdinalIgnoreCase))
                {
                    nRet = m_MyCamera.MV_CC_SetEnumValue_NET(strKey, stEnumInfo.nValue);
                    if (MyCamera.MV_OK != nRet)
                    {
                        return nRet;
                    }
                    break;
                }
            }
            return nRet;
        }

        private void bnOpen_Click(object sender, EventArgs e)
        {
            if (m_stDeviceList.nDeviceNum == 0 || cbDeviceList.SelectedIndex == -1)
            {
                ShowErrorMsg("No device, please select", 0);
                return;
            }

            // ch:获取选择的设备信息 | en:Get selected device information
            MyCamera.MV_CC_DEVICE_INFO device =
                (MyCamera.MV_CC_DEVICE_INFO)Marshal.PtrToStructure(m_stDeviceList.pDeviceInfo[cbDeviceList.SelectedIndex],
                                                              typeof(MyCamera.MV_CC_DEVICE_INFO));

            // ch:打开设备 | en:Open device
            if (null == m_MyCamera)
            {
                m_MyCamera = new MyCamera();
                if (null == m_MyCamera)
                {
                    ShowErrorMsg("Applying resource fail!", MyCamera.MV_E_RESOURCE);
                    return;
                }
            }

            int nRet = m_MyCamera.MV_CC_CreateDevice_NET(ref device);
            if (MyCamera.MV_OK != nRet)
            {
                ShowErrorMsg("Create device fail!", nRet);
                return;
            }

            nRet = m_MyCamera.MV_CC_OpenDevice_NET();
            if (MyCamera.MV_OK != nRet)
            {
                m_MyCamera.MV_CC_DestroyDevice_NET();
                ShowErrorMsg("Device open fail!", nRet);
                return;
            }

            // ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera)
            if (device.nTLayerType == MyCamera.MV_GIGE_DEVICE)
            {
                int nPacketSize = m_MyCamera.MV_CC_GetOptimalPacketSize_NET();
                if (nPacketSize > 0)
                {
                    nRet = m_MyCamera.MV_CC_SetIntValueEx_NET("GevSCPSPacketSize", nPacketSize);
                    if (nRet != MyCamera.MV_OK)
                    {
                        ShowErrorMsg("Set Packet Size failed!", nRet);
                    }
                }
                else
                {
                    ShowErrorMsg("Get Packet Size failed!", nPacketSize);
                }
            }

            m_MyCamera.MV_CC_SetEnumValue_NET("TriggerMode", (uint)MyCamera.MV_CAM_TRIGGER_MODE.MV_TRIGGER_MODE_OFF);

            m_bOpenDevice = true;
            EnableControls(true);
            bnGetParameter_Click(null, null);
        }

        private void EnableControls(bool bIsCameraReady)
        {
            bnOpen.Enabled = (m_bOpenDevice ? false : (bIsCameraReady ? true : false));
            bnClose.Enabled = ((m_bOpenDevice && bIsCameraReady) ? true : false);
            bnStartGrab.Enabled = ((m_bStartGrabbing && bIsCameraReady) ? false : (m_bOpenDevice ? true : false));
            bnStopGrab.Enabled = (m_bStartGrabbing ? true : false);
            cbPixelFormat.Enabled = ((m_bStartGrabbing && bIsCameraReady) ? false : (m_bOpenDevice ? true : false));
            cbDisplaySource.Enabled = (m_bOpenDevice ? true : false);
            cbLegendCheck.Enabled = (m_bOpenDevice ? true : false);
            cbRegionSelect.Enabled = (m_bOpenDevice ? true : false);
            bnRegionSetting.Enabled = (m_bOpenDevice ? true : false);
            bnWarningSetting.Enabled = (m_bOpenDevice ? true : false);
            teTransmissivity.Enabled = (m_bOpenDevice ? true : false);
            teTargetDistance.Enabled = (m_bOpenDevice ? true : false);
            teEmissivity.Enabled = (m_bOpenDevice ? true : false);
            cbMeasureRange.Enabled = (m_bOpenDevice ? true : false);
            bnGetParameter.Enabled = (m_bOpenDevice ? true : false);
            bnSetParameter.Enabled = (m_bOpenDevice ? true : false);
            cbPaletteMode.Enabled = (m_bOpenDevice ? true : false);
            cbExportModeCheck.Enabled = (m_bOpenDevice ? true : false);
        }

        private int ReadEnumIntoCombo(string strKey, ref ComboBox ctrlComboBox)
        {
            MyCamera.MVCC_ENUMENTRY stEnumInfo = new MyCamera.MVCC_ENUMENTRY();
            MyCamera.MVCC_ENUMVALUE stEnumValue = new MyCamera.MVCC_ENUMVALUE();
            int nRet = m_MyCamera.MV_CC_GetEnumValue_NET(strKey, ref stEnumValue);
            if (MyCamera.MV_OK != nRet)
            {
                return nRet;
            }
            ctrlComboBox.Items.Clear();
            int nIndex = -1;
            for (int i = 0; i < stEnumValue.nSupportedNum; ++i)
            {
                stEnumInfo.nValue = stEnumValue.nSupportValue[i];
                nRet = m_MyCamera.MV_CC_GetEnumEntrySymbolic_NET(strKey, ref stEnumInfo);
                if (MyCamera.MV_OK == nRet)
                {
                    ctrlComboBox.Items.Add(Encoding.Default.GetString(stEnumInfo.chSymbolic));
                }
                if (stEnumInfo.nValue == stEnumValue.nCurValue)
                {
                    nIndex = ctrlComboBox.FindString(Encoding.Default.GetString(stEnumInfo.chSymbolic));
                }
                if (nIndex >= 0)
                {
                    ctrlComboBox.SelectedIndex = nIndex;
                }
            }
            return MyCamera.MV_OK;
        }

        private void bnGetParameter_Click(object sender, EventArgs e)
        {
            int nRet = ReadEnumIntoCombo("PixelFormat", ref cbPixelFormat);
            if (MyCamera.MV_OK != nRet)
            {
                ShowErrorMsg("Get PixelFormat Fail!", nRet);
                return;
            }

            nRet = ReadEnumIntoCombo("OverScreenDisplayProcessor", ref cbDisplaySource);
            if (MyCamera.MV_OK != nRet)
            {
                ShowErrorMsg("Get OverScreenDisplayProcessor Fail!", nRet);
                return;
            }

            nRet = ReadEnumIntoCombo("PalettesMode", ref cbPaletteMode);
            if (MyCamera.MV_OK != nRet)
            {
                ShowErrorMsg("Get PalettesMode Fail!", nRet);
                return;
            }

            bool bValue = false;
            nRet = m_MyCamera.MV_CC_GetBoolValue_NET("LegendDisplayEnable", ref bValue);
            if (MyCamera.MV_OK != nRet)
            {
                ShowErrorMsg("Get LegendDisplayEnable Fail!", nRet);
                return;
            }
            else
            {
                cbLegendCheck.Checked = bValue;
            }

            nRet = m_MyCamera.MV_CC_GetBoolValue_NET("MtExpertMode", ref bValue);
            if (MyCamera.MV_OK != nRet)
            {
                ShowErrorMsg("Get MtExpertMode Fail!", nRet);
                return;
            }
            else
            {
                cbExportModeCheck.Checked = bValue;
            }

            nRet = ReadEnumIntoCombo("TempRegionSelector", ref cbRegionSelect);
            if (MyCamera.MV_OK != nRet)
            {
                ShowErrorMsg("Get TempRegionSelector Fail!", nRet);
                return;
            }

            nRet = ReadEnumIntoCombo("TempMeasurementRange", ref cbMeasureRange);
            if (MyCamera.MV_OK != nRet)
            {
                ShowErrorMsg("Get TempMeasurementRange Fail!", nRet);
                return;
            }

            MyCamera.MVCC_INTVALUE_EX oIntValue = new MyCamera.MVCC_INTVALUE_EX();
            nRet = m_MyCamera.MV_CC_GetIntValueEx_NET("AtmosphericTransmissivity", ref oIntValue);
            if (MyCamera.MV_OK != nRet)
            {
                ShowErrorMsg("Get AtmosphericTransmissivity Fail!", nRet);
            }
            else
            {
                teTransmissivity.Text = oIntValue.nCurValue.ToString();
            }

            MyCamera.MVCC_FLOATVALUE oFloatValue = new MyCamera.MVCC_FLOATVALUE();
            nRet = m_MyCamera.MV_CC_GetFloatValue_NET("TargetDistance", ref oFloatValue);
            if (MyCamera.MV_OK != nRet)
            {
                ShowErrorMsg("Get TargetDistance Fail!", nRet);
                return;
            }
            else
            {
                teTargetDistance.Text = oFloatValue.fCurValue.ToString();
            }

            nRet = m_MyCamera.MV_CC_GetFloatValue_NET("FullScreenEmissivity", ref oFloatValue);
            if (MyCamera.MV_OK != nRet)
            {
                ShowErrorMsg("Get FullScreenEmissivity!", nRet);
                return;
            }
            else
            {
                teEmissivity.Text = oFloatValue.fCurValue.ToString();
            }
        }

        // ch:像素类型是否为Mono格式 | en:If Pixel Type is Mono 
        private Boolean IsMono(UInt32 enPixelType)
        {
            switch (enPixelType)
            {
                case (UInt32)MyCamera.MvGvspPixelType.PixelType_Gvsp_Mono1p:
                case (UInt32)MyCamera.MvGvspPixelType.PixelType_Gvsp_Mono2p:
                case (UInt32)MyCamera.MvGvspPixelType.PixelType_Gvsp_Mono4p:
                case (UInt32)MyCamera.MvGvspPixelType.PixelType_Gvsp_Mono8:
                case (UInt32)MyCamera.MvGvspPixelType.PixelType_Gvsp_Mono8_Signed:
                case (UInt32)MyCamera.MvGvspPixelType.PixelType_Gvsp_Mono10:
                case (UInt32)MyCamera.MvGvspPixelType.PixelType_Gvsp_Mono10_Packed:
                case (UInt32)MyCamera.MvGvspPixelType.PixelType_Gvsp_Mono12:
                case (UInt32)MyCamera.MvGvspPixelType.PixelType_Gvsp_Mono12_Packed:
                case (UInt32)MyCamera.MvGvspPixelType.PixelType_Gvsp_Mono14:
                case (UInt32)MyCamera.MvGvspPixelType.PixelType_Gvsp_Mono16:
                    return true;
                default:
                    return false;
            }
        }

        // ch:取图前的必要操作步骤 | en:Necessary operation before grab
        private Int32 NecessaryOperBeforeGrab()
        {
            // ch:取图像宽 | en:Get Iamge Width
            MyCamera.MVCC_INTVALUE_EX stWidth = new MyCamera.MVCC_INTVALUE_EX();
            int nRet = m_MyCamera.MV_CC_GetIntValueEx_NET("Width", ref stWidth);
            if (MyCamera.MV_OK != nRet)
            {
                ShowErrorMsg("Get Width Info Fail!", nRet);
                return nRet;
            }
            // ch:取图像高 | en:Get Iamge Height
            MyCamera.MVCC_INTVALUE_EX stHeight = new MyCamera.MVCC_INTVALUE_EX();
            nRet = m_MyCamera.MV_CC_GetIntValueEx_NET("Height", ref stHeight);
            if (MyCamera.MV_OK != nRet)
            {
                ShowErrorMsg("Get Height Info Fail!", nRet);
                return nRet;
            }
            // ch:取像素格式 | en:Get Pixel Format
            MyCamera.MVCC_ENUMVALUE stPixelFormat = new MyCamera.MVCC_ENUMVALUE();
            nRet = m_MyCamera.MV_CC_GetEnumValue_NET("PixelFormat", ref stPixelFormat);
            if (MyCamera.MV_OK != nRet)
            {
                ShowErrorMsg("Get Pixel Format Fail!", nRet);
                return nRet;
            }

            // ch:设置bitmap像素格式，申请相应大小内存 | en:Set Bitmap Pixel Format, alloc memory
            if ((Int32)MyCamera.MvGvspPixelType.PixelType_Gvsp_Undefined == stPixelFormat.nCurValue)
            {
                ShowErrorMsg("Unknown Pixel Format!", MyCamera.MV_E_UNKNOW);
                return MyCamera.MV_E_UNKNOW;
            }
            else if (IsMono(stPixelFormat.nCurValue))
            {
                m_bitmapPixelFormat = PixelFormat.Format8bppIndexed;

                if (IntPtr.Zero != m_ConvertDstBuf)
                {
                    Marshal.Release(m_ConvertDstBuf);
                    m_ConvertDstBuf = IntPtr.Zero;
                }

                // Mono8为单通道
                m_nConvertDstBufLen = (UInt32)(stWidth.nCurValue * stHeight.nCurValue);
                m_ConvertDstBuf = Marshal.AllocHGlobal((Int32)m_nConvertDstBufLen);
                if (IntPtr.Zero == m_ConvertDstBuf)
                {
                    ShowErrorMsg("Malloc Memory Fail!", MyCamera.MV_E_RESOURCE);
                    return MyCamera.MV_E_RESOURCE;
                }
            }
            else
            {
                m_bitmapPixelFormat = PixelFormat.Format24bppRgb;

                if (IntPtr.Zero != m_ConvertDstBuf)
                {
                    Marshal.FreeHGlobal(m_ConvertDstBuf);
                    m_ConvertDstBuf = IntPtr.Zero;
                }

                // RGB为三通道
                m_nConvertDstBufLen = (UInt32)(3 * stWidth.nCurValue * stHeight.nCurValue);
                m_ConvertDstBuf = Marshal.AllocHGlobal((Int32)m_nConvertDstBufLen);
                if (IntPtr.Zero == m_ConvertDstBuf)
                {
                    ShowErrorMsg("Malloc Memory Fail!", MyCamera.MV_E_RESOURCE);
                    return MyCamera.MV_E_RESOURCE;
                }
            }

            // 确保释放保存了旧图像数据的bitmap实例，用新图像宽高等信息new一个新的bitmap实例
            if (null != m_bitmap)
            {
                m_bitmap.Dispose();
                m_bitmap = null;
            }
            m_bitmap = new Bitmap((Int32)stWidth.nCurValue, (Int32)stHeight.nCurValue, m_bitmapPixelFormat);

            // ch:Mono8格式，设置为标准调色板 | en:Set Standard Palette in Mono8 Format
            if (PixelFormat.Format8bppIndexed == m_bitmapPixelFormat)
            {
                ColorPalette palette = m_bitmap.Palette;
                for (int i = 0; i < palette.Entries.Length; i++)
                {
                    palette.Entries[i] = Color.FromArgb(i, i, i);
                }
                m_bitmap.Palette = palette;
            }

            return MyCamera.MV_OK;
        }

        public void ReceiveThreadProcess()
        {
            MyCamera.MV_FRAME_OUT stFrameInfo = new MyCamera.MV_FRAME_OUT();
            MyCamera.MV_DISPLAY_FRAME_INFO stDisplayInfo = new MyCamera.MV_DISPLAY_FRAME_INFO();
            MyCamera.MV_PIXEL_CONVERT_PARAM stConvertInfo = new MyCamera.MV_PIXEL_CONVERT_PARAM();
            int nRet = MyCamera.MV_OK;

            while (m_bGrabbing)
            {
                nRet = m_MyCamera.MV_CC_GetImageBuffer_NET(ref stFrameInfo, 1000);
                if (nRet == MyCamera.MV_OK)
                {
                    lock (BufForDriverLock)
                    {
                        if (m_BufForDriver == IntPtr.Zero || stFrameInfo.stFrameInfo.nFrameLen > m_nBufSizeForDriver)
                        {
                            if (m_BufForDriver != IntPtr.Zero)
                            {
                                Marshal.Release(m_BufForDriver);
                                m_BufForDriver = IntPtr.Zero;
                            }

                            m_BufForDriver = Marshal.AllocHGlobal((Int32)stFrameInfo.stFrameInfo.nFrameLen);
                            if (m_BufForDriver == IntPtr.Zero)
                            {
                                return;
                            }
                            m_nBufSizeForDriver = stFrameInfo.stFrameInfo.nFrameLen;
                        }

                        m_stFrameInfo = stFrameInfo.stFrameInfo;
                        CopyMemory(m_BufForDriver, stFrameInfo.pBufAddr, stFrameInfo.stFrameInfo.nFrameLen);

                        // ch:转换像素格式 | en:Convert Pixel Format
                        stConvertInfo.nWidth = stFrameInfo.stFrameInfo.nWidth;
                        stConvertInfo.nHeight = stFrameInfo.stFrameInfo.nHeight;
                        stConvertInfo.enSrcPixelType = stFrameInfo.stFrameInfo.enPixelType;
                        stConvertInfo.pSrcData = stFrameInfo.pBufAddr;
                        stConvertInfo.nSrcDataLen = stFrameInfo.stFrameInfo.nFrameLen;
                        stConvertInfo.pDstBuffer = m_ConvertDstBuf;
                        stConvertInfo.nDstBufferSize = m_nConvertDstBufLen;
                        if (PixelFormat.Format8bppIndexed == m_bitmap.PixelFormat)
                        {
                            stConvertInfo.enDstPixelType = MyCamera.MvGvspPixelType.PixelType_Gvsp_Mono8;
                            m_MyCamera.MV_CC_ConvertPixelType_NET(ref stConvertInfo);
                        }
                        else
                        {
                            stConvertInfo.enDstPixelType = MyCamera.MvGvspPixelType.PixelType_Gvsp_BGR8_Packed;
                            m_MyCamera.MV_CC_ConvertPixelType_NET(ref stConvertInfo);
                        }

                        // ch:保存Bitmap数据 | en:Save Bitmap Data
                        BitmapData bitmapData = m_bitmap.LockBits(new Rectangle(0, 0, stConvertInfo.nWidth, stConvertInfo.nHeight), ImageLockMode.ReadWrite, m_bitmap.PixelFormat);
                        CopyMemory(bitmapData.Scan0, stConvertInfo.pDstBuffer, (UInt32)(bitmapData.Stride * m_bitmap.Height));
                        m_bitmap.UnlockBits(bitmapData);
                    }

                    stDisplayInfo.hWnd = pbDisplay.Handle;
                    stDisplayInfo.pData = stFrameInfo.pBufAddr;
                    stDisplayInfo.nDataLen = stFrameInfo.stFrameInfo.nFrameLen;
                    stDisplayInfo.nWidth = stFrameInfo.stFrameInfo.nWidth;
                    stDisplayInfo.nHeight = stFrameInfo.stFrameInfo.nHeight;
                    stDisplayInfo.enPixelType = stFrameInfo.stFrameInfo.enPixelType;
                    m_MyCamera.MV_CC_DisplayOneFrame_NET(ref stDisplayInfo);

                    m_MyCamera.MV_CC_FreeImageBuffer_NET(ref stFrameInfo);
                }
            }
        }

        private void bnStartGrab_Click(object sender, EventArgs e)
        {
            if (false == m_bOpenDevice || true == m_bStartGrabbing || null == m_MyCamera)
            {
                return;
            }

            // ch:前置配置 | en:pre-operation
            int nRet = NecessaryOperBeforeGrab();
            if (MyCamera.MV_OK != nRet)
            {
                return;
            }

            // ch:标志位置true | en:Set position bit true
            m_bGrabbing = true;

            m_stFrameInfo.nFrameLen = 0;//取流之前先清除帧长度
            m_stFrameInfo.enPixelType = MyCamera.MvGvspPixelType.PixelType_Gvsp_Undefined;

            m_hReceiveThread = new Thread(ReceiveThreadProcess);
            m_hReceiveThread.Start();

            // ch:开始采集 | en:Start Grabbing
            nRet = m_MyCamera.MV_CC_StartGrabbing_NET();
            if (MyCamera.MV_OK != nRet)
            {
                m_bGrabbing = false;
                m_hReceiveThread.Join();
                ShowErrorMsg("Start Grabbing Fail!", nRet);
                return;
            }

            m_bStartGrabbing = true;
            EnableControls(true);
        }

        private void bnClose_Click(object sender, EventArgs e)
        {
            // ch:取流标志位清零 | en:Reset flow flag bit
            if (m_bGrabbing == true)
            {
                m_bGrabbing = false;
                m_hReceiveThread.Join();
            }

            if (m_BufForDriver != IntPtr.Zero)
            {
                Marshal.Release(m_BufForDriver);
            }

            // ch:关闭设备 | en:Close Device
            m_MyCamera.MV_CC_CloseDevice_NET();
            m_MyCamera.MV_CC_DestroyDevice_NET();

            m_bStartGrabbing = false;
            m_bOpenDevice = false;

            // ch:控件操作 | en:Control Operation
            EnableControls(true);
        }

        private void bnStopGrab_Click(object sender, EventArgs e)
        {
            if (false == m_bOpenDevice || false == m_bStartGrabbing || null == m_MyCamera)
            {
                return;
            }

            // ch:标志位设为false | en:Set flag bit false
            m_bGrabbing = false;
            m_hReceiveThread.Join();

            // ch:停止采集 | en:Stop Grabbing
            int nRet = m_MyCamera.MV_CC_StopGrabbing_NET();
            if (nRet != MyCamera.MV_OK)
            {
                ShowErrorMsg("Stop Grabbing Fail!", nRet);
            }

            m_bStartGrabbing = false;
            // ch:控件操作 | en:Control Operation
            EnableControls(true);
        }

        private void cbPixelFormat_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (false == m_bStartGrabbing)
            {
                int nRet = SetEnumIntoCombo("PixelFormat", ref cbPixelFormat);
                if (nRet != MyCamera.MV_OK)
                {
                    ShowErrorMsg("Set PixelFormat Fail!", nRet);
                }
            }
        }

        private void cbDisplaySource_SelectedIndexChanged(object sender, EventArgs e)
        {
            int nRet = SetEnumIntoCombo("OverScreenDisplayProcessor", ref cbDisplaySource);
            if (nRet != MyCamera.MV_OK)
            {
                ShowErrorMsg("Set OverScreenDisplayProcessor Fail!", nRet);
            }
        }

        private void cbPaletteMode_SelectedIndexChanged(object sender, EventArgs e)
        {
            int nRet = SetEnumIntoCombo("PalettesMode", ref cbPaletteMode);
            if (nRet != MyCamera.MV_OK)
            {
                ShowErrorMsg("Set PaletteMode Fail!", nRet);
            }
        }

        private void cbLegendCheck_CheckedChanged(object sender, EventArgs e)
        {
            bool bLegendCheck = cbLegendCheck.Checked;

            int nRet = m_MyCamera.MV_CC_SetBoolValue_NET("LegendDisplayEnable", bLegendCheck);
            if (nRet != MyCamera.MV_OK)
            {
                ShowErrorMsg("Set LegendDisplayEnable Fail!", nRet);
                return;
            }

            nRet = m_MyCamera.MV_CC_SetCommandValue_NET("TempControlLoad");
            if (nRet != MyCamera.MV_OK)
            {
                ShowErrorMsg("Exec TempControlLoad Fail!", nRet);
            }
        }

        private void cbExportModeCheck_CheckedChanged(object sender, EventArgs e)
        {
            bool bExportModeCheck = cbExportModeCheck.Checked;

            int nRet = m_MyCamera.MV_CC_SetBoolValue_NET("MtExpertMode", bExportModeCheck);
            if (nRet != MyCamera.MV_OK)
            {
                ShowErrorMsg("Set ExpertMode Fail!", nRet);
                return;
            }

            nRet = m_MyCamera.MV_CC_SetCommandValue_NET("TempControlLoad");
            if (nRet != MyCamera.MV_OK)
            {
                ShowErrorMsg("Exec TempControlLoad Fail!", nRet);
            }
        }

        private void cbRegionSelect_SelectedIndexChanged(object sender, EventArgs e)
        {
            int nRet = SetEnumIntoCombo("TempRegionSelector", ref cbRegionSelect);
            if (nRet != MyCamera.MV_OK)
            {
                ShowErrorMsg("Set TempRegionSelector Fail!", nRet);
            }
        }

        private void cbMeasureRange_SelectedIndexChanged(object sender, EventArgs e)
        {
            int nRet = SetEnumIntoCombo("TempMeasurementRange", ref cbMeasureRange);
            if (nRet != MyCamera.MV_OK)
            {
                ShowErrorMsg("Set TempMeasurementRange Fail!", nRet);
            }
        }

        private void bnSetParameter_Click(object sender, EventArgs e)
        {
            try
            {
                int.Parse(teTransmissivity.Text);
                float.Parse(teTargetDistance.Text);
                float.Parse(teEmissivity.Text);
            }
            catch
            {
                ShowErrorMsg("Please enter correct type!", 0);
                return;
            }

            int nRet = m_MyCamera.MV_CC_SetIntValueEx_NET("AtmosphericTransmissivity", int.Parse(teTransmissivity.Text));
            if (nRet != MyCamera.MV_OK)
            {
                ShowErrorMsg("Set AtmosphericTransmissivity Fail!", nRet);
            }

            nRet = m_MyCamera.MV_CC_SetFloatValue_NET("TargetDistance", float.Parse(teTargetDistance.Text));
            if (nRet != MyCamera.MV_OK)
            {
                ShowErrorMsg("Set TargetDistance Fail!", nRet);
            }

            nRet = m_MyCamera.MV_CC_SetFloatValue_NET("FullScreenEmissivity", float.Parse(teEmissivity.Text) + 0.000001F);
            if (nRet != MyCamera.MV_OK)
            {
                ShowErrorMsg("Set FullScreenEmissivity Fail!", nRet);
            }
        }

        private void bnRegionSetting_Click(object sender, EventArgs e)
        {
            bool bExportModeCheck = cbExportModeCheck.Checked;
            InfraredDemo.RegionSettingForm = new FormRegionSetting(ref m_MyCamera, ref cbRegionSelect, ref bExportModeCheck);

            InfraredDemo.RegionSettingForm.Show();
            RegionSettingForm.Show();
        }

        private void bnWarningSetting_Click(object sender, EventArgs e)
        {
            InfraredDemo.AlarmSettingForm = new FormAlarmSetting(ref m_MyCamera, ref cbRegionSelect);
            InfraredDemo.AlarmSettingForm.Show();
            AlarmSettingForm.Show();
        }

        private void InfraredDemo_Closing(object sender, FormClosingEventArgs e)
        {
            bnClose_Click(sender, null);

            // ch: 反初始化SDK | en: Finalize SDK
            MyCamera.MV_CC_Finalize_NET();
        }
    }
}
