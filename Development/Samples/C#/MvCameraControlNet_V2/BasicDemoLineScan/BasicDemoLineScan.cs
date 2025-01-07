using MvCameraControl;
using System;
using System.Collections.Generic;
using System.Threading;
using System.Windows.Forms;

namespace BasicDemoLineScan
{
    public partial class Form1 : Form
    {
        readonly DeviceTLayerType enumTLayerType = DeviceTLayerType.MvGigEDevice | DeviceTLayerType.MvUsbDevice
                | DeviceTLayerType.MvGenTLGigEDevice | DeviceTLayerType.MvGenTLCXPDevice | DeviceTLayerType.MvGenTLCameraLinkDevice | DeviceTLayerType.MvGenTLXoFDevice;

        IDevice device = null;
        List<IDeviceInfo> deviceInfos = new List<IDeviceInfo>();

        bool isGrabbing = false;
        Thread receiveThread = null;

        // ch:用于从驱动获取到的帧信息 | en:Frame info that getting image from driver
        IFrameOut frameForSave = null;
        private readonly Object lockForSaveImage = new Object();

        IEnumValue triggerSelector = null;  // 触发选项
        IEnumValue triggerMode = null;      // 触发模式
        IEnumValue triggerSource = null;    // 触发源
        IEnumValue pixelFormat = null;      // 像素格式
        IEnumValue imgCompressMode = null;  // HB模式
        IEnumValue preampGain = null;       // 模拟增益

        public Form1()
        {
            InitializeComponent();

            SDKSystem.Initialize();

            UpdateDeviceList();
            CheckForIllegalCrossThreadCalls = false;
        }

        /// <summary>
        /// // ch:显示错误信息 | en:Show error message
        /// </summary>
        /// <param name="message">ch:错误信息 | en: error message</param>
        /// <param name="errorCode">ch:错误码 | en: error code</param>
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

        /// <summary>
        /// ch:枚举设备 | en:Enum devices
        /// </summary>
        private void bnEnum_Click(object sender, EventArgs e)
        {
            UpdateDeviceList();
        }

        /// <summary>
        /// ch:刷新设备列表 | en:Update devices list
        /// </summary>
        private void UpdateDeviceList()
        {
            // ch:枚举设备列表 | en:Enumerate Device List
            cmbDeviceList.Items.Clear();

            int result = DeviceEnumerator.EnumDevices(enumTLayerType, out deviceInfos);
            if (result != MvError.MV_OK)
            {
                ShowErrorMsg("Enumerate devices fail!", result);
                return;
            }

            // ch:在窗体列表中显示设备名 | en:Display device name in the form list
            for (int i = 0; i < deviceInfos.Count; i++)
            {
                IDeviceInfo deviceInfo = deviceInfos[i];
                if (deviceInfo.UserDefinedName != "")
                {
                    cmbDeviceList.Items.Add(deviceInfo.TLayerType.ToString() + ": " + deviceInfo.UserDefinedName + " (" + deviceInfo.SerialNumber + ")");
                }
                else
                {
                    cmbDeviceList.Items.Add(deviceInfo.TLayerType.ToString() + ": " + deviceInfo.ManufacturerName + " " + deviceInfo.ModelName + " (" + deviceInfo.SerialNumber + ")");
                }
            }

            // ch:选择第一项 | en:Select the first item
            if (deviceInfos.Count > 0)
            {
                cmbDeviceList.SelectedIndex = 0;
            }
            else
            {
                ShowErrorMsg("No device", 0);
            }
            return;
        }

        /// <summary>
        /// ch:打开设备 | en:Open device
        /// </summary>
        private void bnOpen_Click(object sender, System.EventArgs e)
        {
            if (0 == deviceInfos.Count || -1 == cmbDeviceList.SelectedIndex)
            {
                ShowErrorMsg("No device, please enumerate device", 0);
                return;
            }

            // ch:获取选择的设备信息 | en:Get selected device information
            IDeviceInfo deviceInfo = deviceInfos[cmbDeviceList.SelectedIndex];

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
                device.Dispose();
                device = null;

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

            // ch:获取参数 | en:Get parameters
            GetImageCompressionMode();
            GetPreampGain();
            GetTriggerMode();
            GetTriggerSelector();
            GetTriggerSource();
            GetPixelFormat();
            bnGetParam_Click(null, null);

            // ch:控件操作 | en:Control operation
            btnOpen.Enabled = false;
            btnClose.Enabled = true;
            btnStartGrab.Enabled = true;
            btnStopGrab.Enabled = false;
            btnTriggerExec.Enabled = false;
            btnGetParam.Enabled = true;
            btnSetParam.Enabled = true;
            cmbDeviceList.Enabled = false;
        }

        /// <summary>
        /// ch:获取虚拟增益模式 | en:Get PreampGain
        /// </summary>
        private void GetPreampGain()
        {
            cmbPreampGain.Items.Clear();

            int result = device.Parameters.GetEnumValue("PreampGain", out preampGain);
            if (result == MvError.MV_OK)
            {
                for (int i = 0; i < preampGain.SupportedNum; i++)
                {
                    cmbPreampGain.Items.Add(preampGain.SupportEnumEntries[i].Symbolic);
                    if (preampGain.SupportEnumEntries[i].Symbolic == preampGain.CurEnumEntry.Symbolic)
                    {
                        cmbPreampGain.SelectedIndex = i;
                    }
                }
                cmbPreampGain.Enabled = true;
            }
        }

        /// <summary>
        /// ch:获取HB模式 | en:Get ImageCompressionMode
        /// </summary>
        private void GetImageCompressionMode()
        {
            cmbHBMode.Items.Clear();

            int result = device.Parameters.GetEnumValue("ImageCompressionMode", out imgCompressMode);
            if (result == MvError.MV_OK)
            {
                for (int i = 0; i < imgCompressMode.SupportedNum; i++)
                {
                    cmbHBMode.Items.Add(imgCompressMode.SupportEnumEntries[i].Symbolic);
                    if (imgCompressMode.SupportEnumEntries[i].Symbolic == imgCompressMode.CurEnumEntry.Symbolic)
                    {
                        cmbHBMode.SelectedIndex = i;
                    }
                }
                cmbHBMode.Enabled = true;
            }
            else
            {
                cmbHBMode.Enabled = false;
            }
        }

        /// <summary>
        /// ch:获取像素格式 | en:Get PixelFormat
        /// </summary>
        private void GetPixelFormat()
        {
            cmbPixelFormat.Items.Clear();

            int result = device.Parameters.GetEnumValue("PixelFormat", out pixelFormat);
            if (result == MvError.MV_OK)
            {
                for (int i = 0; i < pixelFormat.SupportedNum; i++)
                {
                    cmbPixelFormat.Items.Add(pixelFormat.SupportEnumEntries[i].Symbolic);
                    if (pixelFormat.SupportEnumEntries[i].Symbolic == pixelFormat.CurEnumEntry.Symbolic)
                    {
                        cmbPixelFormat.SelectedIndex = i;
                    }
                }
                cmbPixelFormat.Enabled = true;
            }
        }

        /// <summary>
        /// ch:获取触发选项 | en:Get TriggerSelector
        /// </summary>
        private void GetTriggerSelector()
        {
            cmbTriggerSelector.Items.Clear();
            int result = device.Parameters.GetEnumValue("TriggerSelector", out triggerSelector);
            if (result == MvError.MV_OK)
            {
                for (int i = 0; i < triggerSelector.SupportedNum; i++)
                {
                    cmbTriggerSelector.Items.Add(triggerSelector.SupportEnumEntries[i].Symbolic);
                    if (triggerSelector.SupportEnumEntries[i].Symbolic == triggerSelector.CurEnumEntry.Symbolic)
                    {
                        cmbTriggerSelector.SelectedIndex = i;
                    }
                }
                cmbTriggerSelector.Enabled = true;
            }
        }

        /// <summary>
        /// ch:获取触发模式 | en:Get TriggerMode
        /// </summary>
        private void GetTriggerMode()
        {
            cmbTriggerMode.Items.Clear();
            int result = device.Parameters.GetEnumValue("TriggerMode", out triggerMode);
            if (result == MvError.MV_OK)
            {
                for (int i = 0; i < triggerMode.SupportedNum; i++)
                {
                    cmbTriggerMode.Items.Add(triggerMode.SupportEnumEntries[i].Symbolic);
                    if (triggerMode.SupportEnumEntries[i].Symbolic == triggerMode.CurEnumEntry.Symbolic)
                    {
                        cmbTriggerMode.SelectedIndex = i;
                    }
                }
                cmbTriggerMode.Enabled = true;
            }
        }

        /// <summary>
        /// ch:获取触发源 | en:Get TriggerSource
        /// </summary>
        private void GetTriggerSource()
        {
            cmbTriggerSource.Items.Clear();
            int result = device.Parameters.GetEnumValue("TriggerSource", out triggerSource);
            if (result == MvError.MV_OK)
            {
                for (int i = 0; i < triggerSource.SupportedNum; i++)
                {
                    cmbTriggerSource.Items.Add(triggerSource.SupportEnumEntries[i].Symbolic);
                    if (triggerSource.SupportEnumEntries[i].Value == triggerSource.CurEnumEntry.Value)
                    {
                        cmbTriggerSource.SelectedIndex = i;
                    }
                }
                cmbTriggerSource.Enabled = true;
            }
        }

        /// <summary>
        /// ch:关闭设备 | en:Close device
        /// </summary>
        private void bnClose_Click(object sender, System.EventArgs e)
        {
            // ch:取流标志位清零 | en:Reset flow flag bit
            if (isGrabbing == true)
            {
                isGrabbing = false;
                receiveThread.Join();
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
            btnOpen.Enabled = true;
            btnClose.Enabled = false;
            btnStartGrab.Enabled = false;
            btnStopGrab.Enabled = false;
            btnTriggerExec.Enabled = false;
            cmbDeviceList.Enabled = true;

            btnSaveBmp.Enabled = false;
            btnSaveJpg.Enabled = false;
            btnSaveTiff.Enabled = false;
            btnSavePng.Enabled = false;
            tbExposure.Enabled = false;
            btnGetParam.Enabled = false;
            btnSetParam.Enabled = false;
            cmbPixelFormat.Enabled = false;
            cmbHBMode.Enabled = false;
            cmbPreampGain.Enabled = false;
            cmbTriggerSource.Enabled = false;
            cmbTriggerSelector.Enabled = false;
            cmbTriggerMode.Enabled = false;
            tbExposure.Enabled = false;
            tbDigitalShift.Enabled = false;
            tbAcqLineRate.Enabled = false;
            chkLineRateEnable.Enabled = false;
        }

        /// <summary>
        /// ch:接收图像线程 | en:Receive image thread process
        /// </summary>
        public void ReceiveThreadProcess()
        {
            IFrameOut frameOut = null;
            int result = MvError.MV_OK;

            while (isGrabbing)
            {
                result = device.StreamGrabber.GetImageBuffer(1000, out frameOut);
                if (result == MvError.MV_OK)
                {
                    // ch:保存图像数据用于保存图像文件 | en:Save frame info for save image
                    lock (lockForSaveImage)
                    {
                        frameForSave = frameOut.Clone() as IFrameOut;
                    }

                    // ch:渲染图像数据 | en:Display frame
                    device.ImageRender.DisplayOneFrame(pictureBox1.Handle, frameOut.Image);

                    // ch:释放帧信息 | en:Free frame info
                    device.StreamGrabber.FreeImageBuffer(frameOut);
                }
                else
                {
                    if (cmbTriggerMode.SelectedText == "On")
                    {
                        Thread.Sleep(5);
                    }
                }
            }
        }

        /// <summary>
        /// ch:开始采集 | en:Start grab
        /// </summary>
        private void bnStartGrab_Click(object sender, System.EventArgs e)
        {
            // ch:标志位置位 true | en:Set position bit true
            isGrabbing = true;

            receiveThread = new Thread(ReceiveThreadProcess);
            receiveThread.Start();

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
            btnStartGrab.Enabled = false;
            btnStopGrab.Enabled = true;

            if ((cmbTriggerMode.Text == "On") && (cmbTriggerSource.Text == "Software") && isGrabbing)
            {
                btnTriggerExec.Enabled = true;
            }

            btnSaveBmp.Enabled = true;
            btnSaveJpg.Enabled = true;
            btnSaveTiff.Enabled = true;
            btnSavePng.Enabled = true;
            cmbPixelFormat.Enabled = false;
            cmbHBMode.Enabled = false;
        }

        /// <summary>
        /// ch:软触发执行一次 | en:Trigger once by software
        /// </summary>
        private void bnTriggerExec_Click(object sender, System.EventArgs e)
        {
            // ch:触发命令 | en:Trigger command
            int result = device.Parameters.SetCommandValue("TriggerSoftware");
            if (result != MvError.MV_OK)
            {
                ShowErrorMsg("Trigger Software Fail!", result);
            }
        }

        /// <summary>
        /// ch:停止采集 | en:Stop Grab
        /// </summary>
        private void bnStopGrab_Click(object sender, System.EventArgs e)
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

        private void SetCtrlWhenStopGrab()
        {
            btnStartGrab.Enabled = true;
            btnStopGrab.Enabled = false;
            btnTriggerExec.Enabled = false;
            btnSaveBmp.Enabled = false;
            btnSaveJpg.Enabled = false;
            btnSaveTiff.Enabled = false;
            btnSavePng.Enabled = false;
            cmbPixelFormat.Enabled = true;
            cmbHBMode.Enabled = true;
        }

        /// <summary>
        /// ch:保存图像 | en:Save image
        /// </summary>
        /// <param name="imageFormatInfo">ch:图像格式信息 | en:Image format info </param>
        /// <returns></returns>
        private int SaveImage(ImageFormatInfo imageFormatInfo)
        {
            if (frameForSave == null)
            {
                throw new Exception("No vaild image");
            }

            string imagePath = "Image_w" + frameForSave.Image.Width.ToString() + "_h" + frameForSave.Image.Height.ToString() + "_fn" + frameForSave.FrameNum.ToString() + "." + imageFormatInfo.FormatType.ToString();

            lock (lockForSaveImage)
            {
                return device.ImageSaver.SaveImageToFile(imagePath, frameForSave.Image, imageFormatInfo, CFAMethod.Optimal);
            }
        }

        /// <summary>
        /// ch:保存Bmp文件 | en:Save Bmp image
        /// </summary>
        private void bnSaveBmp_Click(object sender, System.EventArgs e)
        {
            int result;

            try
            {
                ImageFormatInfo imageFormatInfo = new ImageFormatInfo();
                imageFormatInfo.FormatType = ImageFormatType.Bmp;

                result = SaveImage(imageFormatInfo);
                if (result != MvError.MV_OK)
                {
                    ShowErrorMsg("Save Image Fail!", result);
                    return;
                }
                else
                {
                    ShowErrorMsg("Save Image Succeed!", 0);
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show("Save Image Failed, " + ex.Message);
                return;
            }
        }

        /// <summary>
        /// ch:保存Jpeg文件 | en:Save Jpeg image
        /// </summary>
        private void bnSaveJpg_Click(object sender, System.EventArgs e)
        {
            int result;

            try
            {
                ImageFormatInfo imageFormatInfo = new ImageFormatInfo();
                imageFormatInfo.FormatType = ImageFormatType.Jpeg;
                imageFormatInfo.JpegQuality = 80;

                result = SaveImage(imageFormatInfo);
                if (result != MvError.MV_OK)
                {
                    ShowErrorMsg("Save Image Fail!", result);
                    return;
                }
                else
                {
                    ShowErrorMsg("Save Image Succeed!", 0);
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show("Save Image Failed, " + ex.Message);
                return;
            }
        }

        /// <summary>
        /// ch:保存Tiff格式文件 | en:Save Tiff image
        /// </summary>
        private void bnSaveTiff_Click(object sender, System.EventArgs e)
        {
            int result;
            try
            {
                ImageFormatInfo imageFormatInfo = new ImageFormatInfo();
                imageFormatInfo.FormatType = ImageFormatType.Tiff;

                result = SaveImage(imageFormatInfo);
                if (result != MvError.MV_OK)
                {
                    ShowErrorMsg("Save Image Fail!", result);
                    return;
                }
                else
                {
                    ShowErrorMsg("Save Image Succeed!", 0);
                }

            }
            catch (Exception ex)
            {
                MessageBox.Show("Save Image Failed, " + ex.Message);
                return;
            }
        }

        /// <summary>
        /// ch:保存PNG格式文件 | en:Save PNG image
        /// </summary>
        private void bnSavePng_Click(object sender, System.EventArgs e)
        {
            int result;

            try
            {
                ImageFormatInfo imageFormatInfo = new ImageFormatInfo();
                imageFormatInfo.FormatType = ImageFormatType.Png;

                result = SaveImage(imageFormatInfo);
                if (result != MvError.MV_OK)
                {
                    ShowErrorMsg("Save Image Fail!", result);
                    return;
                }
                else
                {
                    ShowErrorMsg("Save Image Succeed!", 0);
                }

            }
            catch (Exception ex)
            {
                MessageBox.Show("Save Image Failed, " + ex.Message);
                return;
            }
        }

        /// <summary>
        /// ch:设置参数 | en:Set Parameters
        /// </summary>
        private void bnSetParam_Click(object sender, System.EventArgs e)
        {
            int result = MvError.MV_OK;

            // ch:设置曝光 | en:Set ExposureTime
            if (tbExposure.Enabled)
            {
                try
                {
                    float.Parse(tbExposure.Text);
                    device.Parameters.SetEnumValue("ExposureAuto", 0);
                    result = device.Parameters.SetFloatValue("ExposureTime", float.Parse(tbExposure.Text));
                    if (result != MvError.MV_OK)
                    {
                        ShowErrorMsg("Set Exposure Time Fail!", result);
                    }
                }
                catch
                {
                    ShowErrorMsg("Please enter ExposureTime correct", 0);
                }
            }

            // ch:设置数字增益 | en:Set DigitalShift
            if (tbDigitalShift.Enabled)
            {
                try
                {
                    float.Parse(tbDigitalShift.Text);
                    device.Parameters.SetBoolValue("DigitalShiftEnable", true);
                    result = device.Parameters.SetFloatValue("DigitalShift", float.Parse(tbDigitalShift.Text));
                    if (result != MvError.MV_OK)
                    {
                        ShowErrorMsg("Set Digital Shift Fail!", result);
                    }
                }
                catch
                {
                    ShowErrorMsg("Please enter DigitalShift correct", 0);
                }
            }

            // ch:设置行频设定值 | en:Set AcquisitionLineRate
            if (tbAcqLineRate.Enabled)
            {
                try
                {
                    int.Parse(tbAcqLineRate.Text);
                    result = device.Parameters.SetIntValue("AcquisitionLineRate", int.Parse(tbAcqLineRate.Text));
                    if (result != MvError.MV_OK)
                    {
                        ShowErrorMsg("Set Acquisition Line Rate Fail!", result);
                    }
                }
                catch
                {
                    ShowErrorMsg("Please enter AcquisitionLineRate correct", 0);
                }
            }
        }

        /// <summary>
        /// ch:获取参数 | en:Get Parameters
        /// </summary>
        private void bnGetParam_Click(object sender, System.EventArgs e)
        {
            // ch:获取曝光参数 | en:Get ExposureTime
            IFloatValue exposureTime = null;
            int result = device.Parameters.GetFloatValue("ExposureTime", out exposureTime);
            if (result == MvError.MV_OK)
            {
                tbExposure.Text = exposureTime.CurValue.ToString("F2");
                tbExposure.Enabled = true;
            }

            // ch:获取数字增益参数 | en:Get DigitalShift
            IFloatValue digitalShift = null;
            result = device.Parameters.GetFloatValue("DigitalShift", out digitalShift);
            if (result == MvError.MV_OK)
            {
                tbDigitalShift.Text = digitalShift.CurValue.ToString("F2");
                tbDigitalShift.Enabled = true;
            }

            // ch:获取行频使能开关 | en:Get AcquisitionLineRateEnable
            bool acqLineRateEnable = false;
            result = device.Parameters.GetBoolValue("AcquisitionLineRateEnable", out acqLineRateEnable);
            if (result == MvError.MV_OK)
            {
                chkLineRateEnable.Enabled = true;
                chkLineRateEnable.Checked = acqLineRateEnable;
            }

            // ch:获取行频设置值 | en:Get AcquisitionLineRate
            IIntValue acqLineRate = null;
            result = device.Parameters.GetIntValue("AcquisitionLineRate", out acqLineRate);
            if (result == MvError.MV_OK)
            {
                tbAcqLineRate.Text = acqLineRate.CurValue.ToString();
                tbAcqLineRate.Enabled = true;
            }

            // ch:获取行频实际值 | en:Get ResultingLineRate
            IIntValue resultLineRate = null;
            result = device.Parameters.GetIntValue("ResultingLineRate", out resultLineRate);
            if (result == MvError.MV_OK)
            {
                tbResLineRate.Text = resultLineRate.CurValue.ToString();
                tbResLineRate.Enabled = true;
            }
        }

        private void cbTriggerSelector_SelectedIndexChanged(object sender, EventArgs e)
        {
            int result = device.Parameters.SetEnumValue("TriggerSelector", triggerSelector.SupportEnumEntries[cmbTriggerSelector.SelectedIndex].Value);
            if (result != MvError.MV_OK)
            {
                ShowErrorMsg("Set Trigger Selector Failed", result);
                for (int i = 0; i < triggerSelector.SupportedNum; i++)
                {
                    if (triggerSelector.SupportEnumEntries[i].Value == triggerSelector.CurEnumEntry.Value)
                    {
                        cmbTriggerSelector.SelectedIndex = i;
                        return;
                    }
                }
            }

            GetTriggerMode();
            GetTriggerSource();
        }

        private void cbTiggerMode_SelectedIndexChanged(object sender, EventArgs e)
        {
            int result = device.Parameters.SetEnumValue("TriggerMode", (uint)triggerMode.SupportEnumEntries[cmbTriggerMode.SelectedIndex].Value);
            if (result != MvError.MV_OK)
            {
                ShowErrorMsg("Set Trigger Mode Failed", result);
                for (int i = 0; i < triggerMode.SupportedNum; i++)
                {
                    if (triggerMode.SupportEnumEntries[i].Value == triggerMode.CurEnumEntry.Value)
                    {
                        cmbTriggerMode.SelectedIndex = i;
                        return;
                    }
                }
            }

            GetTriggerSource();

            if ((cmbTriggerMode.Text == "On" && cmbTriggerSource.Text == "Software") && isGrabbing)
            {
                btnTriggerExec.Enabled = true;
            }
            else
            {
                btnTriggerExec.Enabled = false;
            }
        }

        private void cbTriggerSource_SelectedIndexChanged(object sender, EventArgs e)
        {
            int result = device.Parameters.SetEnumValue("TriggerSource", triggerSource.SupportEnumEntries[cmbTriggerSource.SelectedIndex].Value);
            if (result != MvError.MV_OK)
            {
                ShowErrorMsg("Set Trigger Source Failed", result);
                for (int i = 0; i < triggerSource.SupportedNum; i++)
                {
                    if (triggerSource.SupportEnumEntries[i].Value == triggerSource.CurEnumEntry.Value)
                    {
                        cmbTriggerSource.SelectedIndex = i;
                        return;
                    }
                }
            }

            if ((cmbTriggerMode.Text == "On" && cmbTriggerSource.Text == "Software") && isGrabbing)
            {
                btnTriggerExec.Enabled = true;
            }
            else
            {
                btnTriggerExec.Enabled = false;
            }
        }

        private void cbPixelFormat_SelectedIndexChanged(object sender, EventArgs e)
        {
            // ch:设置像素格式 | en:Set PixelFormat
            int result = device.Parameters.SetEnumValue("PixelFormat", pixelFormat.SupportEnumEntries[cmbPixelFormat.SelectedIndex].Value);
            if (result != MvError.MV_OK)
            {
                ShowErrorMsg("Set PixelFormat Fail!", result);
                for (int i = 0; i < pixelFormat.SupportedNum; i++)
                {
                    if (pixelFormat.SupportEnumEntries[i].Value == pixelFormat.CurEnumEntry.Value)
                    {
                        cmbPixelFormat.SelectedIndex = i;
                        return;
                    }
                }
            }
            GetImageCompressionMode();
        }

        private void cbHBMode_SelectedIndexChanged(object sender, EventArgs e)
        {
            // ch:设置无损压缩模式 | en:Set ImageCompressionMode
            int result = device.Parameters.SetEnumValue("ImageCompressionMode", imgCompressMode.SupportEnumEntries[cmbHBMode.SelectedIndex].Value);
            if (result != MvError.MV_OK)
            {
                ShowErrorMsg("Set ImageCompressionMode Fail!", result);
                for (int i = 0; i < imgCompressMode.SupportedNum; i++)
                {
                    if (imgCompressMode.SupportEnumEntries[i].Value == imgCompressMode.CurEnumEntry.Value)
                    {
                        cmbHBMode.SelectedIndex = i;
                        return;
                    }
                }
            }
        }

        private void cbPreampGain_SelectedIndexChanged(object sender, EventArgs e)
        {
            int result = device.Parameters.SetEnumValue("PreampGain", preampGain.SupportEnumEntries[cmbPreampGain.SelectedIndex].Value);
            if (result != MvError.MV_OK)
            {
                ShowErrorMsg("Set PreampGain Fail!", result);
                for (int i = 0; i < preampGain.SupportedNum; i++)
                {
                    if (preampGain.SupportEnumEntries[i].Value == preampGain.CurEnumEntry.Value)
                    {
                        cmbPreampGain.SelectedIndex = i;
                        return;
                    }
                }
            }
        }

        private void chkLineRateEnable_CheckedChanged(object sender, EventArgs e)
        {
            if (chkLineRateEnable.Checked)
            {
                device.Parameters.SetBoolValue("AcquisitionLineRateEnable", true);
            }
            else
            {
                device.Parameters.SetBoolValue("AcquisitionLineRateEnable", false);
            }
        }

        /// <summary>
        /// ch:窗口关闭事件 | en: FormClosing event
        /// </summary>
        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            bnClose_Click(sender, e);

            SDKSystem.Finalize();
        }
    }
}
