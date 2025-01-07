using MvCameraControl;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Windows.Forms;

namespace InfraredDemo
{
    public partial class InfraredDemo : Form
    {
        List<IDeviceInfo> deviceInfoList = new List<IDeviceInfo>();
        IDevice device = null;

        bool isOpen = false;                // ch:是否打开设备 | en:Whether to open device
        bool isGrabbing = false;            // ch:是否开始抓图 | en:Whether to start grabbing
        IntPtr displayHandle = IntPtr.Zero; // ch:用于显示图像的控件句柄 | en:Handle of the image display control
        Thread m_hReceiveThread = null;

        public InfraredDemo()
        {
            InitializeComponent();
            EnableControls(false);
            this.Load += new EventHandler(this.InfraredDemo_Load);
            displayHandle = pbDisplay.Handle;
        }

        private void InfraredDemo_Load(object sender, EventArgs e)
        {
            // ch: 初始化 SDK | en: Initialize SDK
            SDKSystem.Initialize();
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
                case MvError.MV_E_HANDLE: errorMsg += "Error or invalid handle "; break;
                case MvError.MV_E_SUPPORT: errorMsg += "Not supported function "; break;
                case MvError.MV_E_BUFOVER: errorMsg += "Cache is full "; break;
                case MvError.MV_E_CALLORDER: errorMsg += "Function calling order error "; break;
                case MvError.MV_E_PARAMETER: errorMsg += "Incorrect parameter "; break;
                case MvError.MV_E_RESOURCE: errorMsg += "Applying resource failed "; break;
                case MvError.MV_E_NODATA: errorMsg += "No data "; break;
                case MvError.MV_E_PRECONDITION: errorMsg += "Precondition error, or running environment changed "; break;
                case MvError.MV_E_VERSION: errorMsg += "Version mismatches "; break;
                case MvError.MV_E_NOENOUGH_BUF: errorMsg += "Insufficient memory "; break;
                case MvError.MV_E_ABNORMAL_IMAGE: errorMsg += "Abnormal image, maybe incomplete image because of lost packet "; break;
                case MvError.MV_E_UNKNOW: errorMsg += "Unknown error "; break;
                case MvError.MV_E_GC_GENERIC: errorMsg += "General error "; break;
                case MvError.MV_E_GC_ACCESS: errorMsg += "Node accessing condition error "; break;
                case MvError.MV_E_ACCESS_DENIED: errorMsg += "No permission "; break;
                case MvError.MV_E_BUSY: errorMsg += "Device is busy, or network disconnected "; break;
                case MvError.MV_E_NETER: errorMsg += "Network error "; break;
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
            cbDeviceList.Items.Clear();
            int result = DeviceEnumerator.EnumDevices(DeviceTLayerType.MvGigEDevice | DeviceTLayerType.MvUsbDevice, out deviceInfoList);
            if (0 != result)
            {
                ShowErrorMsg("Enumerate devices fail!", 0);
                return;
            }

            // ch:在窗体列表中显示设备名 | en:Display device name in the form list
            for (int i = 0; i < deviceInfoList.Count; i++)
            {
                IDeviceInfo deviceInfo = deviceInfoList[i];
                if (deviceInfo.UserDefinedName != "")
                {
                    cbDeviceList.Items.Add(deviceInfo.TLayerType.ToString() + ": " + deviceInfo.UserDefinedName + " (" + deviceInfo.SerialNumber + ")");
                }
                else
                {
                    cbDeviceList.Items.Add(deviceInfo.TLayerType.ToString() + ": " + deviceInfo.ManufacturerName + " " + deviceInfo.ModelName + " (" + deviceInfo.SerialNumber + ")");
                }
            }

            // ch:选择第一项 | en:Select the first item
            if (deviceInfoList.Count != 0)
            {
                cbDeviceList.SelectedIndex = 0;
            }
        }

        private void bnOpen_Click(object sender, EventArgs e)
        {
            if (deviceInfoList.Count == 0 || cbDeviceList.SelectedIndex == -1)
            {
                ShowErrorMsg("No device, please select", 0);
                return;
            }

            // ch:获取选择的设备信息 | en:Get selected device information
            IDeviceInfo deviceInfo = deviceInfoList[cbDeviceList.SelectedIndex];

            try
            {
                // ch:打开设备 | en:Open device
                device = DeviceFactory.CreateDevice(deviceInfo);
            }
            catch (Exception ex)
            {
                MessageBox.Show("Create Device fail!" + ex.Message);
                return;
            }

            int result = device.Open();
            if (result != MvError.MV_OK)
            {
                ShowErrorMsg("Open Device fail!", result);
                return;
            }

            //ch: 判断是否为gige设备 | en: Determine whether it is a GigE device
            if (device is IGigEDevice)
            {
                //ch: 转换为gigE设备 | en: Convert to Gige device
                IGigEDevice gigEDevice = device as IGigEDevice;

                // ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera)
                int optionPacketSize;
                result = gigEDevice.GetOptimalPacketSize(out optionPacketSize);
                if (result != MvError.MV_OK)
                {
                    ShowErrorMsg("Warning: Get Packet Size failed!", result);
                }
                else
                {
                    result = device.Parameters.SetIntValue("GevSCPSPacketSize", (long)optionPacketSize);
                    if (result != MvError.MV_OK)
                    {
                        ShowErrorMsg("Warning: Set Packet Size failed!", result);
                    }
                }
            }

            // ch:设置采集连续模式 | en:Set Continues Aquisition Mode
            device.Parameters.SetEnumValueByString("AcquisitionMode", "Continuous");
            device.Parameters.SetEnumValueByString("TriggerMode", "Off");

            isOpen = true;
            EnableControls(true);
            bnGetParameter_Click(null, null);
        }

        private void EnableControls(bool bIsCameraReady)
        {
            bnOpen.Enabled = (isOpen ? false : (bIsCameraReady ? true : false));
            bnClose.Enabled = ((isOpen && bIsCameraReady) ? true : false);
            bnStartGrab.Enabled = ((isGrabbing && bIsCameraReady) ? false : (isOpen ? true : false));
            bnStopGrab.Enabled = (isGrabbing ? true : false);
            cbPixelFormat.Enabled = ((isGrabbing && bIsCameraReady) ? false : (isOpen ? true : false));
            cbDisplaySource.Enabled = (isOpen ? true : false);
            cbLegendCheck.Enabled = (isOpen ? true : false);
            cbRegionSelect.Enabled = (isOpen ? true : false);
            bnRegionSetting.Enabled = (isOpen ? true : false);
            bnWarningSetting.Enabled = (isOpen ? true : false);
            teTransmissivity.Enabled = (isOpen ? true : false);
            teTargetDistance.Enabled = (isOpen ? true : false);
            teEmissivity.Enabled = (isOpen ? true : false);
            cbMeasureRange.Enabled = (isOpen ? true : false);
            bnGetParameter.Enabled = (isOpen ? true : false);
            bnSetParameter.Enabled = (isOpen ? true : false);
            cbPaletteMode.Enabled = (isOpen ? true : false);
            cbExportModeCheck.Enabled = (isOpen ? true : false);
        }

        private int ReadEnumIntoCombo(string strKey, ref ComboBox ctrlComboBox)
        {
            IEnumValue enumValue;
            int result = device.Parameters.GetEnumValue(strKey, out enumValue);
            if (MvError.MV_OK != result)
            {
                return result;
            }
            ctrlComboBox.Items.Clear();
            for (int i = 0; i < enumValue.SupportedNum; ++i)
            {
                ctrlComboBox.Items.Add(enumValue.SupportEnumEntries[i].Symbolic);
                if (enumValue.CurEnumEntry.Value == enumValue.SupportEnumEntries[i].Value)
                {
                    ctrlComboBox.SelectedIndex = i;
                }
            }
            return MvError.MV_OK;
        }

        private void bnGetParameter_Click(object sender, EventArgs e)
        {
            int result = ReadEnumIntoCombo("PixelFormat", ref cbPixelFormat);
            if (MvError.MV_OK != result)
            {
                ShowErrorMsg("Get PixelFormat Fail!", result);
                return;
            }

            result = ReadEnumIntoCombo("OverScreenDisplayProcessor", ref cbDisplaySource);
            if (MvError.MV_OK != result)
            {
                ShowErrorMsg("Get OverScreenDisplayProcessor Fail!", result);
                return;
            }

            result = ReadEnumIntoCombo("PalettesMode", ref cbPaletteMode);
            if (MvError.MV_OK != result)
            {
                ShowErrorMsg("Get PalettesMode Fail!", result);
                return;
            }

            bool boolValue = false;
            result = device.Parameters.GetBoolValue("LegendDisplayEnable", out boolValue);
            if (MvError.MV_OK != result)
            {
                ShowErrorMsg("Get LegendDisplayEnable Fail!", result);
                return;
            }
            else
            {
                cbLegendCheck.Checked = boolValue;
            }

            result = device.Parameters.GetBoolValue("MtExpertMode", out boolValue);
            if (MvError.MV_OK != result)
            {
                ShowErrorMsg("Get MtExpertMode Fail!", result);
                return;
            }
            else
            {
                cbExportModeCheck.Checked = boolValue;
            }

            result = ReadEnumIntoCombo("TempRegionSelector", ref cbRegionSelect);
            if (MvError.MV_OK != result)
            {
                ShowErrorMsg("Get TempRegionSelector Fail!", result);
                return;
            }

            result = ReadEnumIntoCombo("TempMeasurementRange", ref cbMeasureRange);
            if (MvError.MV_OK != result)
            {
                ShowErrorMsg("Get TempMeasurementRange Fail!", result);
                return;
            }

            IIntValue intValue;
            result = device.Parameters.GetIntValue("AtmosphericTransmissivity", out intValue);
            if (MvError.MV_OK != result)
            {
                ShowErrorMsg("Get AtmosphericTransmissivity Fail!", result);
            }
            else
            {
                teTransmissivity.Text = intValue.CurValue.ToString();
            }

            IFloatValue floatValue;
            result = device.Parameters.GetFloatValue("TargetDistance", out floatValue);
            if (MvError.MV_OK != result)
            {
                ShowErrorMsg("Get TargetDistance Fail!", result);
                return;
            }
            else
            {
                teTargetDistance.Text = floatValue.CurValue.ToString();
            }

            result = device.Parameters.GetFloatValue("FullScreenEmissivity", out floatValue);
            if (MvError.MV_OK != result)
            {
                ShowErrorMsg("Get FullScreenEmissivity!", result);
                return;
            }
            else
            {
                teEmissivity.Text = floatValue.CurValue.ToString();
            }
        }

        private void ReceiveThreadProcess()
        {
            IFrameOut frameOut;
            int result = MvError.MV_OK;

            while (isGrabbing)
            {
                // ch:获取图像 | en:Get image
                result = device.StreamGrabber.GetImageBuffer(1000, out frameOut);
                if (result == MvError.MV_OK)
                {
                    // ch:显示图像 | en:Display image
                    device.ImageRender.DisplayOneFrame(displayHandle, frameOut.Image);

                    // ch:释放图像 | en:Free image
                    device.StreamGrabber.FreeImageBuffer(frameOut);
                }
            }
        }

        private void bnStartGrab_Click(object sender, EventArgs e)
        {
            if (false == isOpen || true == isGrabbing || null == device)
            {
                return;
            }

            // ch:标志位置true | en:Set position bit true
            isGrabbing = true;

            m_hReceiveThread =  new Thread(ReceiveThreadProcess);
            m_hReceiveThread.Start();

            // ch:开始采集 | en:Start Grabbing
            int result = device.StreamGrabber.StartGrabbing();
            if (MvError.MV_OK != result)
            {
                isGrabbing = false;
                m_hReceiveThread.Join();
                ShowErrorMsg("Start Grabbing Fail!", result);
                return;
            }

            isGrabbing = true;
            EnableControls(true);
        }

        private void bnClose_Click(object sender, EventArgs e)
        {
            // ch:取流标志位清零 | en:Reset flow flag bit
            if (isGrabbing == true)
            {
                isGrabbing = false;
                m_hReceiveThread.Join();
            }

            // ch:关闭设备 | en:Close Device
            if (device != null)
            {
                device.Close();
                device.Dispose();
            }

            isGrabbing = false;
            isOpen = false;

            // ch:控件操作 | en:Control Operation
            EnableControls(true);
        }

        private void bnStopGrab_Click(object sender, EventArgs e)
        {
            if (false == isOpen || false == isGrabbing || null == device)
            {
                return;
            }

            // ch:标志位设为false | en:Set flag bit false
            isGrabbing = false;
            m_hReceiveThread.Join();

            // ch:停止采集 | en:Stop Grabbing
            int result = device.StreamGrabber.StopGrabbing();
            if (result != MvError.MV_OK)
            {
                ShowErrorMsg("Stop Grabbing Fail!", result);
            }

            isGrabbing = false;
            // ch:控件操作 | en:Control Operation
            EnableControls(true);
        }

        private void cbPixelFormat_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (false == isGrabbing)
            {
                int result = device.Parameters.SetEnumValueByString("PixelFormat", cbPixelFormat.SelectedItem.ToString());
                if (result != MvError.MV_OK)
                {
                    ShowErrorMsg("Set PixelFormat Fail!", result);
                }
            }
        }

        private void cbDisplaySource_SelectedIndexChanged(object sender, EventArgs e)
        {
            int result = device.Parameters.SetEnumValueByString("OverScreenDisplayProcessor", cbDisplaySource.SelectedItem.ToString());
            if (result != MvError.MV_OK)
            {
                ShowErrorMsg("Set OverScreenDisplayProcessor Fail!", result);
            }
        }

        private void cbPaletteMode_SelectedIndexChanged(object sender, EventArgs e)
        {
            int result = device.Parameters.SetEnumValueByString("PalettesMode", cbPaletteMode.SelectedItem.ToString());
            if (result != MvError.MV_OK)
            {
                ShowErrorMsg("Set PalettesMode Fail!", result);
            }
        }

        private void cbLegendCheck_CheckedChanged(object sender, EventArgs e)
        {
            bool bLegendCheck = cbLegendCheck.Checked;

            int result = device.Parameters.SetBoolValue("LegendDisplayEnable", bLegendCheck);
            if (result != MvError.MV_OK)
            {
                ShowErrorMsg("Set LegendDisplayEnable Fail!", result);
                return;
            }

            result = device.Parameters.SetCommandValue("TempControlLoad");
            if (result != MvError.MV_OK)
            {
                ShowErrorMsg("Exec TempControlLoad Fail!", result);
            }
        }

        private void cbExportModeCheck_CheckedChanged(object sender, EventArgs e)
        {
            bool bExportModeCheck = cbExportModeCheck.Checked;

            int result = device.Parameters.SetBoolValue("MtExpertMode", bExportModeCheck);
            if (result != MvError.MV_OK)
            {
                ShowErrorMsg("Set ExpertMode Fail!", result);
                return;
            }

            result = device.Parameters.SetCommandValue("TempControlLoad");
            if (result != MvError.MV_OK)
            {
                ShowErrorMsg("Exec TempControlLoad Fail!", result);
            }
        }

        private void cbRegionSelect_SelectedIndexChanged(object sender, EventArgs e)
        {
            int result = device.Parameters.SetEnumValueByString("TempRegionSelector", cbRegionSelect.SelectedItem.ToString());
            if (result != MvError.MV_OK)
            {
                ShowErrorMsg("Set TempRegionSelector Fail!", result);
            }
        }

        private void cbMeasureRange_SelectedIndexChanged(object sender, EventArgs e)
        {
            int result = device.Parameters.SetEnumValueByString("TempMeasurementRange", cbMeasureRange.SelectedItem.ToString());
            if (result != MvError.MV_OK)
            {
                ShowErrorMsg("Set TempMeasurementRange Fail!", result);
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

            int result = device.Parameters.SetIntValue("AtmosphericTransmissivity", long.Parse(teTransmissivity.Text));
            if (result != MvError.MV_OK)
            {
                ShowErrorMsg("Set AtmosphericTransmissivity Fail!", result);
            }

            result = device.Parameters.SetFloatValue("TargetDistance", float.Parse(teTargetDistance.Text));
            if (result != MvError.MV_OK)
            {
                ShowErrorMsg("Set TargetDistance Fail!", result);
            }

            result = device.Parameters.SetFloatValue("FullScreenEmissivity", float.Parse(teEmissivity.Text) + 0.000001F);
            if (result != MvError.MV_OK)
            {
                ShowErrorMsg("Set FullScreenEmissivity Fail!", result);
            }
        }

        private void bnRegionSetting_Click(object sender, EventArgs e)
        {
            if (cbRegionSelect.SelectedIndex < 0)
            {
                ShowErrorMsg("No Region is selected", MvError.MV_OK);
                return;
            }

            bool bExportModeCheck = cbExportModeCheck.Checked;
            InfraredDemo.RegionSettingForm = new FormRegionSetting(ref device, ref cbRegionSelect, ref bExportModeCheck);

            InfraredDemo.RegionSettingForm.Show();
            RegionSettingForm.Show();
        }

        private void bnWarningSetting_Click(object sender, EventArgs e)
        {
            if (cbRegionSelect.SelectedIndex < 0)
            {
                ShowErrorMsg("No Region is selected", MvError.MV_OK);
                return;
            }

            InfraredDemo.AlarmSettingForm = new FormAlarmSetting(ref device, ref cbRegionSelect);
            InfraredDemo.AlarmSettingForm.Show();
            AlarmSettingForm.Show();
        }

        private void InfraredDemo_Closing(object sender, FormClosingEventArgs e)
        {
            bnClose_Click(sender, null);

            // ch: 反初始化SDK | en: Finalize SDK
            SDKSystem.Finalize();
        }
    }
}
