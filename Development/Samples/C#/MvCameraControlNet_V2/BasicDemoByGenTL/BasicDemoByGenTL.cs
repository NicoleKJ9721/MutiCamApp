using MvCameraControl;
using System;
using System.Collections.Generic;
using System.Threading;
using System.Windows.Forms;

namespace BasicDemoByGenTL
{
    public partial class Form1 : Form
    {
        List<IGenTLIFInfo> interfaceInfos = new List<IGenTLIFInfo>();
        List<IGenTLDevInfo> deviceInfos = new List<IGenTLDevInfo>();
        private IDevice device = null;
        bool isGrabbing = false;
        Thread receiveThread = null;

        public Form1()
        {
            InitializeComponent();

            SDKSystem.Initialize();

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

        private void btnEnumInterface_Click(object sender, EventArgs e)
        {
            cmbDeviceList.Items.Clear();
            cmbInterfaceList.Items.Clear();
            cmbDeviceList.Text = "";
            cmbInterfaceList.Text = "";
            OpenFileDialog fileDialog = new OpenFileDialog();
            if (null == fileDialog)
            {
                ShowErrorMsg("Open File Dialog Fail!", MvError.MV_E_RESOURCE);
                return;
            }

            //ch:选择要导入的Cti文件 | en:Select a cti file
            fileDialog.Filter = "Cti文件(*.cti)|*.cti";
            fileDialog.ShowDialog();

            int result = GenTLManager.EnumInterfacesByGenTL(fileDialog.FileName, out interfaceInfos);
            if (0 != result)
            {
                ShowErrorMsg("Enumerate interfaces fail!", result);
                return;
            }

            if(interfaceInfos.Count <= 0)
            {
                ShowErrorMsg("No interfaces!", 0);
                return;
            }
            for (Int32 i = 0; i < interfaceInfos.Count; i++ )
            {
                cmbInterfaceList.Items.Add("TLType:" + interfaceInfos[i].TLType + " " + interfaceInfos[i].InterfaceID + " " + interfaceInfos[i].DisplayName);
            }
            
            cmbInterfaceList.SelectedIndex = 0;
            btnEnumDevice.Enabled = true;
        }

        private void btnEnumDevice_Click(object sender, EventArgs e)
        {
            DeviceListAcq();

            bnOpen.Enabled = true;
        }

        private void DeviceListAcq()
        {
            cmbDeviceList.Items.Clear();

            IGenTLIFInfo ifInfo = interfaceInfos[cmbInterfaceList.SelectedIndex];

            // ch:枚举设备列表 | en:Enumerate Device List
            int result = GenTLManager.EnumDevicesByGenTL(ifInfo, out deviceInfos);
            if (result != MvError.MV_OK)
            {
                ShowErrorMsg("Enumerate devices fail!", 0);
                return;
            }

            // ch:在窗体列表中显示设备名 | en:Display device name in the form list
            for (int i = 0; i < deviceInfos.Count; i++)
            {
                IGenTLDevInfo deviceInfo = deviceInfos[i];

                if (deviceInfo.UserDefinedName != "")
                {
                    cmbDeviceList.Items.Add("Dev: " + deviceInfo.UserDefinedName + " (" + deviceInfo.SerialNumber + ")");
                }
                else
                {
                    cmbDeviceList.Items.Add("Dev: " + deviceInfo.VendorName + " " + deviceInfo.ModelName + " (" + deviceInfo.SerialNumber + ")");
                }
            }

            // ch:选择第一项 | en:Select the first item
            if (deviceInfos.Count != 0)
            {
                cmbDeviceList.SelectedIndex = 0;
            }
        }

        private void SetCtrlWhenOpen()
        {
            btnEnumInterface.Enabled = false;
            btnEnumDevice.Enabled = false;
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

        private void bnOpen_Click(object sender, EventArgs e)
        {
            if (deviceInfos.Count == 0 || cmbDeviceList.SelectedIndex == -1)
            {
                ShowErrorMsg("No device, please select", 0);
                return;
            }

            // ch:获取选择的设备信息 | en:Get selected device information
            IGenTLDevInfo deviceInfo = deviceInfos[cmbDeviceList.SelectedIndex];

            try
            {
                // ch:创建设备 | en:Create device
                device = DeviceFactory.CreateDeviceByGenTL(deviceInfo);
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

                ShowErrorMsg("Device open fail!", result);
                return;
            }

            // ch:设置采集连续模式 | en:Set Continues Aquisition Mode
            device.Parameters.SetEnumValueByString("AcquisitionMode", "Continuous");
            device.Parameters.SetEnumValueByString("TriggerMode", "Off");

            // ch:控件操作 | en:Control operation
            SetCtrlWhenOpen();
        }

        private void SetCtrlWhenClose()
        {
            btnEnumInterface.Enabled = true;
            btnEnumDevice.Enabled = true;

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
                    device.Parameters.SetEnumValueByString("TriggerMode", "On");
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
            bnStopGrab.Enabled = true;

            if (bnTriggerMode.Checked && cbSoftTrigger.Checked)
            {
                bnTriggerExec.Enabled = true;
            }
        }

        public void ReceiveThreadProcess()
        {
            IFrameOut frameOut;
            int result = MvError.MV_OK;

            while (isGrabbing)
            {
                result = device.StreamGrabber.GetImageBuffer(1000, out frameOut);
                if (result == MvError.MV_OK)
                {
                    device.ImageRender.DisplayOneFrame(pictureBox1.Handle, frameOut.Image);

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
            // ch:标志位置位true | en:Set position bit true
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
            bnStopGrab.Enabled = false;

            bnTriggerExec.Enabled = false;
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

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            bnClose_Click(sender, e);

            SDKSystem.Finalize();
        }
    }
}