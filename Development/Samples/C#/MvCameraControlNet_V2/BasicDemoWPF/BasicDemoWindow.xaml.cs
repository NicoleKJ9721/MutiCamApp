/*
 * 这个示例演示如何在WPF程序中调用SDK。
 * This program shows how to call SDK in WPF program.
 */

using MvCameraControl;
using System;
using System.Collections.Generic;
using System.Threading;
using System.Windows;

namespace BasicDemoWPF
{
    /// <summary>
    /// Interaction logic for Window1.xaml
    /// </summary>
    public partial class BasicDemoWindow : Window
    {

        // ch:枚举的相机类型 | en:TLayerType for enumerate devices
        readonly DeviceTLayerType enumTLayerType = DeviceTLayerType.MvGigEDevice | DeviceTLayerType.MvUsbDevice
    | DeviceTLayerType.MvGenTLGigEDevice | DeviceTLayerType.MvGenTLCXPDevice | DeviceTLayerType.MvGenTLCameraLinkDevice | DeviceTLayerType.MvGenTLXoFDevice;

        List<IDeviceInfo> deviceInfoList = new List<IDeviceInfo>();
        IDevice device = null;

        bool isGrabbing = false;        // ch:是否正在取图 | en: Grabbing flag
        Thread receiveThread = null;    // ch:接收图像线程 | en: Receive image thread
        IntPtr pictureBoxHandle = IntPtr.Zero; // ch:显示图像的控件句柄 | en: Control handle for image display

        public BasicDemoWindow()
        {
            InitializeComponent();
            Closing += Window_Closing;
            Loaded += new RoutedEventHandler(BasicDemoWindow_Load);
        }

        private void BasicDemoWindow_Load(object sender, RoutedEventArgs e)
        {
            // ch: 初始化 SDK | en: Initialize SDK
            SDKSystem.Initialize();

            // ch: 枚举设备 | en: Enum Device List
            RefreshDeviceList();
        }

        /// <summary>
        /// ch:显示错误信息 | en:Show error message
        /// </summary>
        private void ShowErrorMsg(string message, int errorCode)
        {
            string errorMsg;
            if (errorCode == 0)
            {
                errorMsg = message;
            }
            else
            {
                errorMsg = message + ": Error =" + String.Format("{0:X}", errorCode);
            }

            switch (errorCode)
            {
                case MvError.MV_E_HANDLE: errorMsg += " Error or invalid handle "; break;
                case MvError.MV_E_SUPPORT: errorMsg += " Not supported function "; break;
                case MvError.MV_E_BUFOVER: errorMsg += " Cache is full "; break;
                case MvError.MV_E_CALLORDER: errorMsg += " Function calling order error "; break;
                case MvError.MV_E_PARAMETER: errorMsg += " Incorrect parameter "; break;
                case MvError.MV_E_RESOURCE: errorMsg += " Applying resource failed "; break;
                case MvError.MV_E_NODATA: errorMsg += " No data "; break;
                case MvError.MV_E_PRECONDITION: errorMsg += " Precondition error, or running environment changed "; break;
                case MvError.MV_E_VERSION: errorMsg += " Version mismatches "; break;
                case MvError.MV_E_NOENOUGH_BUF: errorMsg += " Insufficient memory "; break;
                case MvError.MV_E_UNKNOW: errorMsg += " Unknown error "; break;
                case MvError.MV_E_GC_GENERIC: errorMsg += " General error "; break;
                case MvError.MV_E_GC_ACCESS: errorMsg += " Node accessing condition error "; break;
                case MvError.MV_E_ACCESS_DENIED: errorMsg += " No permission "; break;
                case MvError.MV_E_BUSY: errorMsg += " Device is busy, or network disconnected "; break;
                case MvError.MV_E_NETER: errorMsg += " Network error "; break;
            }

            MessageBox.Show(errorMsg, "PROMPT");
        }

        private void RefreshDeviceList()
        {
            // ch:创建设备列表 | en:Create Device List
            cbDeviceList.Items.Clear();
            int nRet = DeviceEnumerator.EnumDevices(enumTLayerType, out deviceInfoList);
            if (nRet != MvError.MV_OK)
            {
                ShowErrorMsg("Enumerate devices fail!", nRet);
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
        private void bnEnum_Click(object sender, RoutedEventArgs e)
        {
            RefreshDeviceList();
        }

        private void bnOpen_Click(object sender, RoutedEventArgs e)
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

            // ch:控件操作 | en:Control operation
            SetCtrlWhenOpen();

            // ch:获取参数 | en:Get parameters
            bnGetParam_Click(null, null);
        }

        /// ch:控件操作 | en:Control operation
        private void SetCtrlWhenOpen()
        {
            bnOpen.IsEnabled = false;
            bnClose.IsEnabled = true;

            bnStartGrab.IsEnabled = true;
            bnStopGrab.IsEnabled = false;
            bnContinuesMode.IsEnabled = true;
            bnContinuesMode.IsChecked = true;
            bnTriggerMode.IsEnabled = true;
            cbSoftTrigger.IsEnabled = false;
            bnTriggerExec.IsEnabled = false;

            tbExposure.IsEnabled = true;
            tbGain.IsEnabled = true;
            tbFrameRate.IsEnabled = true;
            bnGetParam.IsEnabled = true;
            bnSetParam.IsEnabled = true;
        }

        private void bnClose_Click(object sender, RoutedEventArgs e)
        {
            // ch:取流标志位清零 | en:Reset flow flag bit
            if (isGrabbing == true)
            {
                bnStopGrab_Click(sender, e);
            }

            // ch:关闭设备 | en:Close Device
            if (device != null)
            {
                device.Close();
                device.Dispose();
            }

            // ch:控件操作 | en:Control Operation
            SetCtrlWhenClose();
        }

        private void SetCtrlWhenClose()
        {
            bnOpen.IsEnabled = true;

            bnClose.IsEnabled = false;
            bnStartGrab.IsEnabled = false;
            bnStopGrab.IsEnabled = false;
            bnContinuesMode.IsEnabled = false;
            bnTriggerMode.IsEnabled = false;
            cbSoftTrigger.IsEnabled = false;
            bnTriggerExec.IsEnabled = false;

            tbExposure.IsEnabled = false;
            tbGain.IsEnabled = false;
            tbFrameRate.IsEnabled = false;
            bnGetParam.IsEnabled = false;
            bnSetParam.IsEnabled = false;
        }

        private void bnContinuesMode_Checked(object sender, RoutedEventArgs e)
        {
            device.Parameters.SetEnumValueByString("TriggerMode", "Off");
            cbSoftTrigger.IsEnabled = false;
            bnTriggerExec.IsEnabled = false;
        }

        private void bnTriggerMode_Checked(object sender, RoutedEventArgs e)
        {
            // ch:打开触发模式 | en:Open Trigger Mode
            device.Parameters.SetEnumValueByString("TriggerMode", "On");

            // ch:触发源选择:0 - Line0; | en:Trigger source select:0 - Line0;
            //           1 - Line1;
            //           2 - Line2;
            //           3 - Line3;
            //           4 - Counter;
            //           7 - Software;
            if ((bool)cbSoftTrigger.IsChecked)
            {
                device.Parameters.SetEnumValueByString("TriggerSource", "Software");
                if (isGrabbing)
                {
                    bnTriggerExec.IsEnabled = true;
                }
            }
            else
            {
                device.Parameters.SetEnumValueByString("TriggerSource", "Line0");
            }
            cbSoftTrigger.IsEnabled = true;
            cbSoftTrigger.IsChecked = true;
        }

        public void ReceiveThreadProcess()
        {
            IFrameOut frameOut = null;
            int result = MvError.MV_OK;

            while (isGrabbing)
            {
                result = device.StreamGrabber.GetImageBuffer(1000, out frameOut);
                if (result == MvError.MV_OK)
                {
                    device.ImageRender.DisplayOneFrame(pictureBoxHandle, frameOut.Image);

                    device.StreamGrabber.FreeImageBuffer(frameOut);
                }
            }
        }

        private void bnStartGrab_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                // ch:标志位置位true | en:Set position bit true
                isGrabbing = true;
                pictureBoxHandle = displayArea.Handle;

                receiveThread = new Thread(ReceiveThreadProcess);
                receiveThread.Start();
            }
            catch (Exception ex)
            {
                MessageBox.Show("Start thread failed!, " + ex.Message);
                return;
            }

            // ch:开始采集 | en:Start Grabbing
            int result = device.StreamGrabber.StartGrabbing();
            if (result != MvError.MV_OK)
            {
                isGrabbing = false;
                receiveThread.Join();
                ShowErrorMsg("Start Grabbing Fail!", result);
                return;
            }

            // ch:控件操作 | en:Control Operation
            SetCtrlWhenStartGrab();
        }

        private void SetCtrlWhenStartGrab()
        {
            bnStartGrab.IsEnabled = false;
            bnStopGrab.IsEnabled = true;

            if ((bool)bnTriggerMode.IsChecked)
            {
                cbSoftTrigger.IsEnabled = true;
                if ((bool)cbSoftTrigger.IsChecked)
                {
                    bnTriggerExec.IsEnabled = true;
                }
            }
        }

        private void bnStopGrab_Click(object sender, RoutedEventArgs e)
        {
            // ch:标志位设为false | en:Set flag bit false
            isGrabbing = false;
            receiveThread.Join();

            // ch:停止采集 | en:Stop Grabbing
            int result = device.StreamGrabber.StopGrabbing();
            if (result != MvError.MV_OK)
            {
                ShowErrorMsg("Stop Grabbing Fail!", result);
            }


            // ch:控件操作 | en:Control Operation
            SetCtrlWhenStopGrab();
        }

        // ch:控件操作 | en:Control Operation
        private void SetCtrlWhenStopGrab()
        {
            bnStartGrab.IsEnabled = true;
            bnStopGrab.IsEnabled = false;

            cbSoftTrigger.IsEnabled = false;
            bnTriggerExec.IsEnabled = false;
        }

        private void bnTriggerExec_Click(object sender, RoutedEventArgs e)
        {
            // ch:触发命令 | en:Trigger command
            int result = device.Parameters.SetCommandValue("TriggerSoftware");
            if (result != MvError.MV_OK)
            {
                ShowErrorMsg("Trigger Software Fail!", result);
            }
        }

        private void cbSoftTrigger_Checked(object sender, RoutedEventArgs e)
        {
            // ch:触发源设为软触发 | en:Set trigger source as Software
            device.Parameters.SetEnumValueByString("TriggerSource", "Software");
            if (isGrabbing)
            {
                bnTriggerExec.IsEnabled = true;
            }
        }

        private void cbSoftTrigger_Unchecked(object sender, RoutedEventArgs e)
        {
            device.Parameters.SetEnumValueByString("TriggerSource", "Line0");
            bnTriggerExec.IsEnabled = false;
        }

        /// <summary>
        /// ch:获取触发模式 | en:Get Trigger Mode
        /// </summary>
        private void GetTriggerMode()
        {
            IEnumValue enumValue;
            int result = device.Parameters.GetEnumValue("TriggerMode", out enumValue);
            if (result == MvError.MV_OK)
            {
                if (enumValue.CurEnumEntry.Symbolic == "On")
                {
                    bnTriggerMode.IsChecked = true;
                    bnContinuesMode.IsChecked = false;

                    result = device.Parameters.GetEnumValue("TriggerSource", out enumValue);
                    if (result == MvError.MV_OK)
                    {
                        if (enumValue.CurEnumEntry.Symbolic == "TriggerSoftware")
                        {
                            cbSoftTrigger.IsEnabled = true;
                            cbSoftTrigger.IsChecked = true;
                            if (isGrabbing)
                            {
                                bnTriggerExec.IsEnabled = true;
                            }
                        }
                    }
                }
                else
                {
                    bnContinuesMode.IsChecked = true;
                    bnTriggerMode.IsChecked = false;
                }
            }
        }

        private void bnGetParam_Click(object sender, RoutedEventArgs e)
        {
            GetTriggerMode();

            IFloatValue floatValue;
            int result = device.Parameters.GetFloatValue("ExposureTime", out floatValue);
            if (result == MvError.MV_OK)
            {
                tbExposure.Text = floatValue.CurValue.ToString("F1");
            }

            result = device.Parameters.GetFloatValue("Gain", out floatValue);
            if (result == MvError.MV_OK)
            {
                tbGain.Text = floatValue.CurValue.ToString("F1");
            }

            result = device.Parameters.GetFloatValue("ResultingFrameRate", out floatValue);
            if (result == MvError.MV_OK)
            {
                tbFrameRate.Text = floatValue.CurValue.ToString("F1");
            }

            IEnumValue enumValue;
            result = device.Parameters.GetEnumValue("PixelFormat", out enumValue);
            if (result == MvError.MV_OK)
            {
                tbPixelFormat.Text = enumValue.CurEnumEntry.Symbolic;
            }
        }

        private void bnSetParam_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                float.Parse(tbExposure.Text);
                float.Parse(tbGain.Text);
                float.Parse(tbFrameRate.Text);
            }
            catch
            {
                ShowErrorMsg("Please enter correct type!", 0);
                return;
            }

            device.Parameters.SetEnumValue("ExposureAuto", 0);
            int result = device.Parameters.SetFloatValue("ExposureTime", float.Parse(tbExposure.Text));
            if (result != MvError.MV_OK)
            {
                ShowErrorMsg("Set Exposure Time Fail!", result);
            }

            device.Parameters.SetEnumValue("GainAuto", 0);
            result = device.Parameters.SetFloatValue("Gain", float.Parse(tbGain.Text));
            if (result != MvError.MV_OK)
            {
                ShowErrorMsg("Set Gain Fail!", result);
            }

            result = device.Parameters.SetFloatValue("AcquisitionFrameRate", float.Parse(tbFrameRate.Text));
            if (result != MvError.MV_OK)
            {
                ShowErrorMsg("Set Frame Rate Fail!", result);
            }
        }

        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            bnClose_Click(null, null);

            // ch: 反初始化SDK | en: Finalize SDK
            SDKSystem.Finalize();
        }
    }
}
