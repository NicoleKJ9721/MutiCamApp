using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using MvCameraControl;
using System.Net;

namespace GigE_ForceIP
{
    public partial class GigE_ForceIP : Form
    {
        List<IDeviceInfo> _devInfoList;
        IDevice _device = null;

        public GigE_ForceIP()
        {
            InitializeComponent();
            this.Load += new EventHandler(GigE_ForceIP_Load);
        }

        private void enumButton_Click(object sender, EventArgs e)
        {
            DeviceListAcq();
        }

        private void DeviceListAcq()
        {
            int result = 0;

            // ch:创建设备列表 | en:Create Device List
            deviceListComboBox.Items.Clear();
            result = DeviceEnumerator.EnumDevices(DeviceTLayerType.MvGigEDevice, out _devInfoList);
            if (MvError.MV_OK != result)
            {
                ShowErrorMsg("DeviceList Acquire Failed!", result);
                return;
            }

            // ch:在窗体列表中显示设备名 | en:Display device name in the form list
            for (int i = 0; i < _devInfoList.Count; i++)
            {
                if (_devInfoList[i].TLayerType == DeviceTLayerType.MvGigEDevice)
                {
                    if (_devInfoList[i].UserDefinedName.Length > 0)
                    {
                        deviceListComboBox.Items.Add("GEV: " + _devInfoList[i].UserDefinedName + " (" + _devInfoList[i].SerialNumber + ")");
                    }
                    else
                    {
                        deviceListComboBox.Items.Add("GEV: " + _devInfoList[i].ManufacturerName + " " + _devInfoList[i].ModelName + "(" + _devInfoList[i].SerialNumber + ")");
                    }
                }
            }

            if (_devInfoList != null)
            {
                deviceListComboBox.SelectedIndex = 0;
            }
        }

        private void GigE_ForceIP_Load(object sender, EventArgs e)
        {
            // ch: 初始化 SDK | en: Initialize SDK
            SDKSystem.Initialize();

            // ch: 枚举设备 | en: Enum Device List
            DeviceListAcq();
        }


        private void GigE_ForceIP_Closing(object sender, FormClosingEventArgs e)
        {
            if (_device != null)
            {
                _device.Dispose();
                _device = null;
            }
            // ch: 反初始化SDK | en: Finalize SDK
            SDKSystem.Finalize();
        }

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

        private void setButton_Click(object sender, EventArgs e)
        {
            if (_devInfoList.Count == 0)
            {
                ShowErrorMsg("No Device", 0);
                return;
            }

            // ch:IP转换 | en:IP conversion
            IPAddress clsIpAddr;
            if (false == IPAddress.TryParse(ipTextBox.Text, out clsIpAddr))
            {
                ShowErrorMsg("Please enter correct IP", 0);
                return;
            }
            long nIp = IPAddress.NetworkToHostOrder(clsIpAddr.Address);

            // ch:掩码转换 | en:Mask conversion
            IPAddress clsSubMask;
            if (false == IPAddress.TryParse(subnetTextBox.Text, out clsSubMask))
            {
                ShowErrorMsg("Please enter correct IP", 0);
                return;
            }
            long nSubMask = IPAddress.NetworkToHostOrder(clsSubMask.Address);

            // ch:网关转换 | en:Gateway conversion
            IPAddress clsDefaultWay;
            if (false == IPAddress.TryParse(gatewayTextBox.Text, out clsDefaultWay))
            {
                ShowErrorMsg("Please enter correct IP", 0);
                return;
            }
            long nDefaultWay = IPAddress.NetworkToHostOrder(clsDefaultWay.Address);

            if (_devInfoList == null || _devInfoList.Count == 0 || _devInfoList[deviceListComboBox.SelectedIndex] ==null)
            {
                return;
            }

            int ret = MvError.MV_OK;
            // ch:创建设备 | en:Create device
            _device = DeviceFactory.CreateDevice(_devInfoList[deviceListComboBox.SelectedIndex]);
            IGigEDevice gigeDevice = _device as IGigEDevice;
            // ch:判断设备IP是否可达 | en: If device ip is accessible
            bool accessible = DeviceEnumerator.IsDeviceAccessible(_devInfoList[deviceListComboBox.SelectedIndex], DeviceAccessMode.AccessExclusive);
            if (accessible)
            {
               

                ret = gigeDevice.SetIpConfig(IpConfigType.Static);
                if (MvError.MV_OK != ret)
                {
                    ShowErrorMsg("Set Ip config fail", ret);
                    gigeDevice.Dispose();
                    _device = null;
                    return;
                }

                ret = gigeDevice.ForceIp((uint)(nIp >> 32), (uint)(nSubMask >> 32), (uint)(nDefaultWay >> 32));
                if (MvError.MV_OK != ret)
                {
                    ShowErrorMsg("ForceIp fail", ret);
                    gigeDevice.Dispose();
                    _device = null;
                    return;
                }
            }
            else
            {
                ret = gigeDevice.ForceIp((uint)(nIp >> 32), (uint)(nSubMask >> 32), (uint)(nDefaultWay >> 32));
                if (MvError.MV_OK != ret)
                {
                    ShowErrorMsg("ForceIp fail", ret);
                    gigeDevice.Dispose();
                    _device = null;
                    return;
                }
                gigeDevice.Dispose();

               IDeviceInfo deviceInfo = _devInfoList[deviceListComboBox.SelectedIndex];
               IGigEDeviceInfo gigeDevInfo = deviceInfo as IGigEDeviceInfo;

               uint nIp1 = ((gigeDevInfo.NetExport & 0xff000000) >> 24);
               uint nIp2 = ((gigeDevInfo.NetExport & 0x00ff0000) >> 16);
               uint nIp3 = ((gigeDevInfo.NetExport & 0x0000ff00) >> 8);
               uint nIp4 = (gigeDevInfo.NetExport & 0x000000ff);
               string netExportIp = nIp1.ToString() + "." + nIp2.ToString() + "." + nIp3.ToString() + "." + nIp4.ToString();
                //ch:需要重新创建句柄，设置为静态IP方式进行保存 | en:  Need to recreate the handle and set it to static IP mode for saving
                //ch: 创建设备 | en: Create device
               _device = DeviceFactory.CreateDeviceByIp(ipTextBox.Text, netExportIp);
               if (null == _device)
                {
                    ShowErrorMsg("Create handle fail", 0);
                    return;
                }
                gigeDevice = _device as IGigEDevice;
                ret = gigeDevice.SetIpConfig(IpConfigType.Static);
                if (MvError.MV_OK != ret)
                {
                    ShowErrorMsg("Set Ip config fail", ret);
                    gigeDevice.Dispose();
                    _device = null;
                    return;
                }
            }
            ShowErrorMsg("IP Set Succeed!", 0);
        }

        private void deviceListComboBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (_devInfoList.Count == 0)
            {
                ShowErrorMsg("No Device", 0);
                return;
            }
            IDeviceInfo deviceInfo = _devInfoList[deviceListComboBox.SelectedIndex];
            IGigEDeviceInfo gigeDevInfo = deviceInfo as IGigEDeviceInfo;

            // ch:网口IP | en:Net IP
            UInt32 nNetIp1 = (gigeDevInfo.NetExport & 0xFF000000) >> 24;
            UInt32 nNetIp2 = (gigeDevInfo.NetExport & 0x00FF0000) >> 16;
            UInt32 nNetIp3 = (gigeDevInfo.NetExport & 0x0000FF00) >> 8;
            UInt32 nNetIp4 = (gigeDevInfo.NetExport & 0x000000FF);

            // ch:显示IP | en:Display IP
            uint nIp1 = ((gigeDevInfo.CurrentIp & 0xff000000) >> 24);
            uint nIp2 = ((gigeDevInfo.CurrentIp & 0x00ff0000) >> 16);
            uint nIp3 = ((gigeDevInfo.CurrentIp & 0x0000ff00) >> 8);
            uint nIp4 = (gigeDevInfo.CurrentIp & 0x000000ff);

            rangeLabel.Text = nNetIp1.ToString() + "." + nNetIp2.ToString() + "." + nNetIp3.ToString() + "." 
                + "0" + "~" + nNetIp1.ToString() + "." + nNetIp2.ToString() + "." + nNetIp3.ToString() + "." + "255";

            ipTextBox.Text = nIp1.ToString() + "." + nIp2.ToString() + "." + nIp3.ToString() + "." + nIp4.ToString();

            // ch:显示掩码 | en:Display mask
            nIp1 = (gigeDevInfo.CurrentSubNetMask & 0xFF000000) >> 24;
            nIp2 = (gigeDevInfo.CurrentSubNetMask & 0x00FF0000) >> 16;
            nIp3 = (gigeDevInfo.CurrentSubNetMask & 0x0000FF00) >> 8;
            nIp4 = (gigeDevInfo.CurrentSubNetMask & 0x000000FF);

            subnetTextBox.Text = nIp1.ToString() + "." + nIp2.ToString() + "." + nIp3.ToString() + "." + nIp4.ToString();

            // ch:显示网关 | en:Display gateway
            nIp1 = (gigeDevInfo.DefultGateWay & 0xFF000000) >> 24;
            nIp2 = (gigeDevInfo.DefultGateWay & 0x00FF0000) >> 16;
            nIp3 = (gigeDevInfo.DefultGateWay & 0x0000FF00) >> 8;
            nIp4 = (gigeDevInfo.DefultGateWay & 0x000000FF);

            gatewayTextBox.Text = nIp1.ToString() + "." + nIp2.ToString() + "." + nIp3.ToString() + "." + nIp4.ToString();
        }

    }
}
