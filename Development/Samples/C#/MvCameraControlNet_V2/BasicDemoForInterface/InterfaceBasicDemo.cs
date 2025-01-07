using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using System.Threading;
using System.IO;

using System.Drawing.Imaging;
using System.Diagnostics;
using System.Collections.ObjectModel;
using MvCameraControl;

namespace InterfaceBasicDemo
{
    public partial class InterfaceBasicDemo : Form
    {
        List<IInterfaceInfo> _interfaceInfoList = new List<IInterfaceInfo>();
        int iInterfaceTypeIndex = 0;
        int iInterfaceIndex = 0;
        int nRet = MvError.MV_OK;

        IInterface _ifInstance = null;
        bool m_bOpenInterface;                        // ch:是否打开采集卡 | en:Whether to open Interface

        public static CXPConfigForm CXPCfgForm = null;
        public static CMLConfigForm CMLCfgForm = null;
        public static XOFConfigForm XOFCfgForm = null;
        public static GEVConfigForm GEVCfgForm = null;

        public InterfaceBasicDemo()
        {
            InitializeComponent();
            m_bOpenInterface = false;
            cbInterfaceType.Items.Clear();
            cbInterfaceType.Items.Add("GIGE_INTERFACE");
            cbInterfaceType.Items.Add("CAMERALINK_INTERFACE");
            cbInterfaceType.Items.Add("CXP_INTERFACE");
            cbInterfaceType.Items.Add("XOF_INTERFACE");
            cbInterfaceType.SelectedIndex = 0;

            EnableControls(false);
            this.Load += new EventHandler(this.InterfaceBasicDemo_Load);
        }

        private void InterfaceBasicDemo_Load(object sender, EventArgs e)
        {
            // ch: 初始化 SDK | en: Initialize SDK
            SDKSystem.Initialize();
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

        private void EnableControls(bool bIsCameraReady)
        {
            bnOpen.Enabled = (bIsCameraReady ? true : false);
            bnClose.Enabled = ((m_bOpenInterface && bIsCameraReady) ? true : false);
            bnConfig.Enabled = ((m_bOpenInterface && bIsCameraReady) ? true : false);
        }

        private void cbInterfaceType_SelectedIndexChanged(object sender, EventArgs e)
        {
            bnClose_Click(sender, e);
            iInterfaceTypeIndex = cbInterfaceType.SelectedIndex;
            cbInterfaceList.Items.Clear();
            cbInterfaceList.Text = "";
        }

        private void bnEnum_Click(object sender, EventArgs e)
        {
            cbInterfaceList.Items.Clear();
            cbInterfaceList.Text = "";

            
            switch (iInterfaceTypeIndex)
            {
                case 0:
                    {
                        nRet = InterfaceEnumerator.EnumInterfaces(InterfaceTLayerType.MvGigEInterface, out _interfaceInfoList);
                        break;
                    }
                case 1:
                    {
                        nRet = InterfaceEnumerator.EnumInterfaces(InterfaceTLayerType.MvCameraLinkInterface, out _interfaceInfoList);
                        break;
                    }
                case 2:
                    {
                        nRet = InterfaceEnumerator.EnumInterfaces(InterfaceTLayerType.MvCXPInterface, out _interfaceInfoList);
                        break;
                    }
                case 3:
                    {
                        nRet = InterfaceEnumerator.EnumInterfaces(InterfaceTLayerType.MvXoFInterface, out _interfaceInfoList);
                        break;
                    }
            }

            // ch:在窗体列表中显示设备名 | en:Display device name in the form list
            for (int i = 0; i < _interfaceInfoList.Count; i++)
            {
                string strShowIfInfo = null;
                strShowIfInfo += "[" + i.ToString() + "]: " + _interfaceInfoList[i].DisplayName + " | " + _interfaceInfoList[i].InterfaceID + " | " + _interfaceInfoList[i].SerialNumber;
                cbInterfaceList.Items.Add(strShowIfInfo);
            }

            // ch:选择第一项 | en:Select the first item
            if (_interfaceInfoList.Count != 0)
            {
                cbInterfaceList.SelectedIndex = 0;
                EnableControls(true);
            }
        }

        private void bnOpen_Click(object sender, EventArgs e)
        {
            if (_interfaceInfoList.Count == 0 || cbInterfaceList.SelectedIndex == -1)
            {
                ShowErrorMsg("No device, please select", 0);
                return;
            }

            // ch:获取选择的设备信息 | en:Get selected device information
            
            try
            {
                _ifInstance = InterfaceFactory.CreateInterface(_interfaceInfoList[iInterfaceIndex]);
            }
            catch (MvException ex)
            {
                ShowErrorMsg("Create Interface fail!", nRet);
                return;
            }

            // ch:打开设备 | en:Open device


            nRet = _ifInstance.Open();
            if (MvError.MV_OK != nRet)
            {
                _ifInstance.Dispose();
                _ifInstance = null;
                ShowErrorMsg("Interface open fail!", nRet);
                return;
            }
			m_bOpenInterface = true;
            EnableControls(true);
        }

        private void bnClose_Click(object sender, EventArgs e)
        {
            if (_ifInstance != null)
            {
                _ifInstance.Close();
                _ifInstance.Dispose();
            }
            
            m_bOpenInterface = false;
            EnableControls(true);
        }

        private void bnConfig_Click(object sender, EventArgs e)
        {
            switch (iInterfaceTypeIndex)
            {
                case 0:
                    {
                        InterfaceBasicDemo.GEVCfgForm = null;
                        GEVCfgForm = new GEVConfigForm(_ifInstance);
                        GEVCfgForm.ShowDialog();
                        break;
                    }
                case 1:
                    {
                        InterfaceBasicDemo.CMLCfgForm = null;
                        CMLCfgForm = new CMLConfigForm(_ifInstance);
                        CMLCfgForm.ShowDialog();
                        break;
                    }
                case 2:
                    {
                        InterfaceBasicDemo.CXPCfgForm = null;
                        CXPCfgForm = new CXPConfigForm(_ifInstance);
                        CXPCfgForm.ShowDialog();
                        break;
                    }
                case 3:
                    {
                        InterfaceBasicDemo.XOFCfgForm = null;
                        XOFCfgForm = new XOFConfigForm(_ifInstance);
                        XOFCfgForm.ShowDialog();
                        break;
                    }
            }
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            bnClose_Click(sender, e);

            // ch: 反初始化SDK | en: Finalize SDK
            SDKSystem.Finalize();
        }

        private void cbInterfaceList_SelectedIndexChanged(object sender, EventArgs e)
        {
            bnClose_Click(sender, e);
            iInterfaceIndex = cbInterfaceList.SelectedIndex;
        }
    }
}
