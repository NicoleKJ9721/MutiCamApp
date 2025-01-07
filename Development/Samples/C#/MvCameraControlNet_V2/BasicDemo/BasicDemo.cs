using MvCameraControl;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Threading;
using System.Windows.Forms;

namespace BasicDemo
{
    public partial class Form1 : Form
    {
        readonly DeviceTLayerType enumTLayerType = DeviceTLayerType.MvGigEDevice | DeviceTLayerType.MvUsbDevice
            | DeviceTLayerType.MvGenTLGigEDevice | DeviceTLayerType.MvGenTLCXPDevice | DeviceTLayerType.MvGenTLCameraLinkDevice | DeviceTLayerType.MvGenTLXoFDevice;

        List<IDeviceInfo> deviceInfoList = new List<IDeviceInfo>();
        IDevice device = null;

        bool isGrabbing = false;        // ch:是否正在取图 | en: Grabbing flag
        bool isRecord = false;          // ch:是否正在录像 | en: Video record flag
        Thread receiveThread = null;    // ch:接收图像线程 | en: Receive image thread

        private IFrameOut frameForSave;                         // ch:获取到的帧信息, 用于保存图像 | en:Frame for save image
        private readonly object saveImageLock = new object();

        public Form1()
        {
            InitializeComponent();

            SDKSystem.Initialize();

            RefreshDeviceList();
            Control.CheckForIllegalCrossThreadCalls = false;
        }

        // ch:显示错误信息 | en:Show error message
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

        private void bnEnum_Click(object sender, EventArgs e)
        {
            RefreshDeviceList();
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
            tbFrameRate.Enabled = true;
            cbPixelFormat.Enabled = true;
            bnGetParam.Enabled = true;
            bnSetParam.Enabled = true;
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

            // ch:控件操作 | en:Control operation
            SetCtrlWhenOpen();

            // ch:获取参数 | en:Get parameters
            bnGetParam_Click(null, null);
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
            bnSaveJpg.Enabled = false;
            bnSaveTiff.Enabled = false;
            bnSavePng.Enabled = false;
            tbExposure.Enabled = false;
            tbGain.Enabled = false;
            tbFrameRate.Enabled = false;
            bnGetParam.Enabled = false;
            bnSetParam.Enabled = false;
            cbPixelFormat.Enabled = false;
            bnStartRecord.Enabled = false;
            bnStopRecord.Enabled = false;
        }

        private void bnClose_Click(object sender, EventArgs e)
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

        private void bnContinuesMode_CheckedChanged(object sender, EventArgs e)
        {
            if (bnContinuesMode.Checked)
            {
                device.Parameters.SetEnumValueByString("TriggerMode", "Off");
                cbSoftTrigger.Enabled = false;
                bnTriggerExec.Enabled = false;
            }
        }

        private void bnTriggerMode_CheckedChanged(object sender, EventArgs e)
        {
            // ch:打开触发模式 | en:Open Trigger Mode
            if (bnTriggerMode.Checked)
            {
                device.Parameters.SetEnumValueByString("TriggerMode", "On");

                // ch:触发源选择:0 - Line0; | en:Trigger source select:0 - Line0;
                //           1 - Line1;
                //           2 - Line2;
                //           3 - Line3;
                //           4 - Counter;
                //           7 - Software;
                if (cbSoftTrigger.Checked)
                {
                    device.Parameters.SetEnumValueByString("TriggerSource", "Software");
                    if (isGrabbing)
                    {
                        bnTriggerExec.Enabled = true;
                    }
                }
                else
                {
                    device.Parameters.SetEnumValueByString("TriggerSource", "Line0");
                }
                cbSoftTrigger.Enabled = true;
            }
        }

        private void SetCtrlWhenStartGrab()
        {
            bnStartGrab.Enabled = false;
            cbPixelFormat.Enabled = false;
            bnStopGrab.Enabled = true;

            if (bnTriggerMode.Checked && cbSoftTrigger.Checked)
            {
                bnTriggerExec.Enabled = true;
            }

            bnSaveBmp.Enabled = true;
            bnSaveJpg.Enabled = true;
            bnSaveTiff.Enabled = true;
            bnSavePng.Enabled = true;
            bnStartRecord.Enabled = true;
            bnStopRecord.Enabled = false;
        }

        public void ReceiveThreadProcess()
        {
            int nRet;

            Graphics graphics;   // ch:使用GDI在pictureBox上绘制图像 | en:Display frame using a graphics

            while (isGrabbing)
            {
                IFrameOut frameOut;

                nRet = device.StreamGrabber.GetImageBuffer(1000, out frameOut);
                if (MvError.MV_OK == nRet)
                {
                    if (isRecord)
                    {
                        device.VideoRecorder.InputOneFrame(frameOut.Image);
                    }

                    lock (saveImageLock)
                    {
                        frameForSave = frameOut.Clone() as IFrameOut;
                    }

#if !GDI_RENDER
                    device.ImageRender.DisplayOneFrame(pictureBox1.Handle, frameOut.Image);
#else
                    // 使用GDI绘制图像
                    try
                    {
                        using (Bitmap bitmap = frameOut.Image.ToBitmap())
                        {
                            if (graphics == null)
                            {
                                graphics = pictureBox1.CreateGraphics();
                            }

                            Rectangle srcRect = new Rectangle(0, 0, bitmap.Width, bitmap.Height);
                            Rectangle dstRect = new Rectangle(0, 0, pictureBox1.Width, pictureBox1.Height);
                            graphics.DrawImage(bitmap, dstRect, srcRect, GraphicsUnit.Pixel);
                        }
                    }
                    catch (Exception e)
                    {
                        device.StreamGrabber.FreeImageBuffer(frameOut);
                        MessageBox.Show(e.Message);
                        return;
                    }
#endif


                    device.StreamGrabber.FreeImageBuffer(frameOut);
                }
                else
                {
                    if (bnTriggerMode.Checked)
                    {
                        Thread.Sleep(5);
                    }
                }
            }
        }

        private void bnStartGrab_Click(object sender, EventArgs e)
        {
            try
            {
                // ch:标志位置位true | en:Set position bit true
                isGrabbing = true;

                receiveThread = new Thread(ReceiveThreadProcess);
                receiveThread.Start();
            }
            catch (Exception ex)
            {
                MessageBox.Show("Start thread failed!, " + ex.Message);
                throw;
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

        private void cbSoftTrigger_CheckedChanged(object sender, EventArgs e)
        {
            if (cbSoftTrigger.Checked)
            {
                // ch:触发源设为软触发 | en:Set trigger source as Software
                device.Parameters.SetEnumValueByString("TriggerSource", "Software");
                if (isGrabbing)
                {
                    bnTriggerExec.Enabled = true;
                }
            }
            else
            {
                device.Parameters.SetEnumValueByString("TriggerSource", "Line0");
                bnTriggerExec.Enabled = false;
            }
        }

        private void bnTriggerExec_Click(object sender, EventArgs e)
        {
            // ch:触发命令 | en:Trigger command
            int result = device.Parameters.SetCommandValue("TriggerSoftware");
            if (result != MvError.MV_OK)
            {
                ShowErrorMsg("Trigger Software Fail!", result);
            }
        }

        private void SetCtrlWhenStopGrab()
        {
            bnStartGrab.Enabled = true;
            cbPixelFormat.Enabled = true;
            bnStopGrab.Enabled = false;
            bnTriggerExec.Enabled = false;

            bnSaveBmp.Enabled = false;
            bnSaveJpg.Enabled = false;
            bnSaveTiff.Enabled = false;
            bnSavePng.Enabled = false;
            bnStartRecord.Enabled = false;
            bnStopRecord.Enabled = false;
        }

        private void bnStopGrab_Click(object sender, EventArgs e)
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

        private int SaveImage(ImageFormatInfo imageFormatInfo)
        {
            if (frameForSave == null)
            {
                throw new Exception("No vaild image");
            }

            string imagePath = "Image_w" + frameForSave.Image.Width.ToString() + "_h" + frameForSave.Image.Height.ToString() + "_fn" + frameForSave.FrameNum.ToString() + "." + imageFormatInfo.FormatType.ToString();

            lock (saveImageLock)
            {
                return device.ImageSaver.SaveImageToFile(imagePath, frameForSave.Image, imageFormatInfo, CFAMethod.Optimal);
            }
        }

        private void bnSaveBmp_Click(object sender, EventArgs e)
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

        private void bnSaveJpg_Click(object sender, EventArgs e)
        {
            int result;

            try
            {
                ImageFormatInfo imageFormatInfo;
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

        private void bnSavePng_Click(object sender, EventArgs e)
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

        private void bnSaveTiff_Click(object sender, EventArgs e)
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
                    bnTriggerMode.Checked = true;
                    bnContinuesMode.Checked = false;

                    result = device.Parameters.GetEnumValue("TriggerSource", out enumValue);
                    if (result == MvError.MV_OK)
                    {
                        if (enumValue.CurEnumEntry.Symbolic == "TriggerSoftware")
                        {
                            cbSoftTrigger.Enabled = true;
                            cbSoftTrigger.Checked = true;
                            if (isGrabbing)
                            {
                                bnTriggerExec.Enabled = true;
                            }
                        }
                    }
                }
                else
                {
                    bnContinuesMode.Checked = true;
                    bnTriggerMode.Checked = false;
                }
            }
        }

        private void bnGetParam_Click(object sender, EventArgs e)
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

            cbPixelFormat.Items.Clear();
            IEnumValue enumValue;
            result = device.Parameters.GetEnumValue("PixelFormat", out enumValue);
            if (result == MvError.MV_OK)
            {
                foreach (var item in enumValue.SupportEnumEntries)
                {
                    cbPixelFormat.Items.Add(item.Symbolic);
                    if (item.Symbolic == enumValue.CurEnumEntry.Symbolic)
                    {
                        cbPixelFormat.SelectedIndex = cbPixelFormat.Items.Count - 1;
                    }
                }

            }
        }

        private void bnSetParam_Click(object sender, EventArgs e)
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

        /// <summary>
        /// ch:程序关闭事件，释放SDK资源 | en:FormClosing, dispose SDK resources
        /// </summary>
        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            bnClose_Click(sender, e);

            SDKSystem.Finalize();
        }

        private void SetCtrlWhenStartRecord()
        {
            bnStartRecord.Enabled = false;
            bnStopRecord.Enabled = true;
        }

        private void bnStartRecord_Click(object sender, EventArgs e)
        {
            if (false == isGrabbing)
            {
                ShowErrorMsg("Not Start Grabbing", 0);
                return;
            }

            IIntValue intValue;
            IEnumValue enumValue;

            uint width;
            uint height;
            MvGvspPixelType pixelType;

            int result;

            result = device.Parameters.GetIntValue("Width", out intValue);
            if (result != MvError.MV_OK)
            {
                ShowErrorMsg("Get Width failed!", result);
                return;
            }
            width = (uint)intValue.CurValue;

            result = device.Parameters.GetIntValue("Height", out intValue);
            if (result != MvError.MV_OK)
            {
                ShowErrorMsg("Get Height failed!", result);
                return;
            }
            height = (uint)intValue.CurValue;

            result = device.Parameters.GetEnumValue("PixelFormat", out enumValue);
            if (result != MvError.MV_OK)
            {
                ShowErrorMsg("Get PixelFormat failed!", result);
                return;
            }
            pixelType = (MvGvspPixelType)enumValue.CurEnumEntry.Value;

            // ch:开始录像 | en:Start record
            RecordParam recordParam;
            recordParam.Width = width;
            recordParam.Height = height;
            recordParam.PixelType = pixelType;
            recordParam.FrameRate = float.Parse(tbFrameRate.Text);
            recordParam.BitRate = 1000;
            recordParam.FormatType = VideoFormatType.AVI;

            result = device.VideoRecorder.StartRecord("./Record.avi", recordParam);
            if (result != MvError.MV_OK)
            {
                ShowErrorMsg("Start Record Fail!", result);
                return;
            }

            isRecord = true;

            SetCtrlWhenStartRecord();
        }

        private void SetCtrlWhenStopRecord()
        {
            bnStartRecord.Enabled = true;
            bnStopRecord.Enabled = false;
        }

        private void bnStopRecord_Click(object sender, EventArgs e)
        {
            if (false == isGrabbing)
            {
                ShowErrorMsg("Not Start Grabbing", 0);
                return;
            }

            int result = device.VideoRecorder.StopRecord();
            if (result != MvError.MV_OK)
            {
                ShowErrorMsg("Stop Record Fail!", result);
            }

            isRecord = false;
            SetCtrlWhenStopRecord();
        }

        private void cbPixelFormat_SelectionChangeCommitted(object sender, EventArgs e)
        {
            int result = device.Parameters.SetEnumValueByString("PixelFormat", cbPixelFormat.Text);
            if (result != MvError.MV_OK)
            {
                ShowErrorMsg("Set PixelFormat failed!", result);
            }
        }
    }
}
