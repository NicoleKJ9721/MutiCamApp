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

namespace InterfaceBasicDemo
{
    public partial class InterfaceBasicDemo : Form
    {
        [DllImport("kernel32.dll", EntryPoint = "RtlMoveMemory", SetLastError = false)]
        private static extern void CopyMemory(IntPtr dest, IntPtr src, uint count);

        MyCamera.MV_INTERFACE_INFO_LIST m_stInterfaceInfoList = new MyCamera.MV_INTERFACE_INFO_LIST();
        private MyCamera m_MyCamera = new MyCamera();
        int iInterfaceTypeIndex = 0;
        int iInterfaceIndex = 0;
        int nRet = MyCamera.MV_OK;

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
            MyCamera.MV_CC_Initialize_NET();
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
                        nRet = MyCamera.MV_CC_EnumInterfaces_NET(MyCamera.MV_GIGE_INTERFACE, ref m_stInterfaceInfoList);
                        break;
                    }
                case 1:
                    {
                        nRet = MyCamera.MV_CC_EnumInterfaces_NET(MyCamera.MV_CAMERALINK_INTERFACE, ref m_stInterfaceInfoList);
                        break;
                    }
                case 2:
                    {
                        nRet = MyCamera.MV_CC_EnumInterfaces_NET(MyCamera.MV_CXP_INTERFACE, ref m_stInterfaceInfoList);
                        break;
                    }
                case 3:
                    {
                        nRet = MyCamera.MV_CC_EnumInterfaces_NET(MyCamera.MV_XOF_INTERFACE, ref m_stInterfaceInfoList);
                        break;
                    }
            }

            // ch:在窗体列表中显示设备名 | en:Display device name in the form list
            for (int i = 0; i < m_stInterfaceInfoList.nInterfaceNum; i++)
            {
                MyCamera.MV_INTERFACE_INFO pstInterfaceInfo = (MyCamera.MV_INTERFACE_INFO)Marshal.PtrToStructure(m_stInterfaceInfoList.pInterfaceInfo[i], typeof(MyCamera.MV_INTERFACE_INFO));
                string strShowIfInfo = null;
                strShowIfInfo += "[" + i.ToString() + "]: " + pstInterfaceInfo.chDisplayName + " | " + pstInterfaceInfo.chInterfaceID + " | " + pstInterfaceInfo.chSerialNumber;
                cbInterfaceList.Items.Add(strShowIfInfo);
            }

            // ch:选择第一项 | en:Select the first item
            if (m_stInterfaceInfoList.nInterfaceNum != 0)
            {
                cbInterfaceList.SelectedIndex = 0;
                EnableControls(true);
            }
        }

        private void bnOpen_Click(object sender, EventArgs e)
        {
            if (m_stInterfaceInfoList.nInterfaceNum == 0 || cbInterfaceList.SelectedIndex == -1)
            {
                ShowErrorMsg("No device, please select", 0);
                return;
            }

            // ch:获取选择的设备信息 | en:Get selected device information
            MyCamera.MV_INTERFACE_INFO pstInterfaceInfo =
                (MyCamera.MV_INTERFACE_INFO)Marshal.PtrToStructure(m_stInterfaceInfoList.pInterfaceInfo[iInterfaceIndex], typeof(MyCamera.MV_INTERFACE_INFO));

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

            int nRet = m_MyCamera.MV_CC_CreateInterface_NET(ref pstInterfaceInfo);
            if (MyCamera.MV_OK != nRet)
            {
                ShowErrorMsg("Create Interface fail!", nRet);
                return;
            }

            nRet = m_MyCamera.MV_CC_OpenInterface_NET("");
            if (MyCamera.MV_OK != nRet)
            {
                m_MyCamera.MV_CC_DestroyInterface_NET();
                ShowErrorMsg("Interface open fail!", nRet);
                return;
            }
			m_bOpenInterface = true;
            EnableControls(true);
        }

        private void bnClose_Click(object sender, EventArgs e)
        {
            m_MyCamera.MV_CC_CloseInterface_NET();
            m_MyCamera.MV_CC_DestroyInterface_NET();
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
                        GEVCfgForm = new GEVConfigForm(ref m_MyCamera);
                        GEVCfgForm.ShowDialog();
                        break;
                    }
                case 1:
                    {
                        InterfaceBasicDemo.CMLCfgForm = null;
                        CMLCfgForm = new CMLConfigForm(ref m_MyCamera);
                        CMLCfgForm.ShowDialog();
                        break;
                    }
                case 2:
                    {
                        InterfaceBasicDemo.CXPCfgForm = null;
                        CXPCfgForm = new CXPConfigForm(ref m_MyCamera);
                        CXPCfgForm.ShowDialog();
                        break;
                    }
                case 3:
                    {
                        InterfaceBasicDemo.XOFCfgForm = null;
                        XOFCfgForm = new XOFConfigForm(ref m_MyCamera);
                        XOFCfgForm.ShowDialog();
                        break;
                    }
            }
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            bnClose_Click(sender, e);

            // ch: 反初始化SDK | en: Finalize SDK
            MyCamera.MV_CC_Finalize_NET();
        }

        private void cbInterfaceList_SelectedIndexChanged(object sender, EventArgs e)
        {
            bnClose_Click(sender, e);
            iInterfaceIndex = cbInterfaceList.SelectedIndex;
        }
    }
}
