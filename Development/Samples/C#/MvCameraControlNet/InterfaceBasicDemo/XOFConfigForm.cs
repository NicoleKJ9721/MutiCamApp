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
    public partial class XOFConfigForm : Form
    {
        MyCamera m_MyCamera;

        bool bIni = false;

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

        public void InitParameter()
        {
            teImageHeight.Enabled = true;
            ReadEnumIntoCombo("StreamSelector", ref cbStreamSelector);

            MyCamera.MVCC_STRINGVALUE oStringValue = new MyCamera.MVCC_STRINGVALUE();
            m_MyCamera.MV_CC_GetStringValue_NET("CurrentStreamDevice", ref oStringValue);
            teCurrentStreamDevice.Text = oStringValue.chCurValue;

            bool bValue = false;
            m_MyCamera.MV_CC_GetBoolValue_NET("MinFrameDelay", ref bValue);
            cbMinFrameDelay.Checked = bValue;

            ReadEnumIntoCombo("CameraType", ref cbCameraType);

            MyCamera.MVCC_INTVALUE_EX oIntValue = new MyCamera.MVCC_INTVALUE_EX();
            m_MyCamera.MV_CC_GetIntValueEx_NET("ImageHeight", ref oIntValue);
            teImageHeight.Text = oIntValue.nCurValue.ToString();
            m_MyCamera.MV_CC_GetIntValueEx_NET("FrameTimeoutTime", ref oIntValue);
            teFrameTimeoutTime.Text = oIntValue.nCurValue.ToString();

            ReadEnumIntoCombo("PartialImageOutputMode", ref cbPartialImageOutputMode);

            bIni = true;
        }

        public XOFConfigForm()
        {
            InitializeComponent();
        }

        public XOFConfigForm(ref MyCamera MyCamera)
            : this()
        {
            m_MyCamera = MyCamera;

            InitParameter();
        }

        private void bnGetParameter_Click(object sender, EventArgs e)
        {
            InitParameter();
        }

        private void bnSetParameter_Click(object sender, EventArgs e)
        {
            try
            {
                int.Parse(teImageHeight.Text);
            }
            catch
            {
                ShowErrorMsg("Please enter correct type!", 0);
                return;
            }

            int nRet = m_MyCamera.MV_CC_SetIntValueEx_NET("ImageHeight", int.Parse(teImageHeight.Text));
            if (MyCamera.MV_OK != nRet)
            {
                ShowErrorMsg("Set ImageHeight Fail!", nRet);
            }
        }

        private void cbStreamSelector_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (false == bIni)
            {
                return;
            }

            int nRet = SetEnumIntoCombo("StreamSelector", ref cbStreamSelector);
            if (nRet != MyCamera.MV_OK)
            {
                ShowErrorMsg("Set StreamSelector Fail!", nRet);
            }
        }

        private void cbMinFrameDelay_CheckedChanged(object sender, EventArgs e)
        {
            if (false == bIni)
            {
                return;
            }

            bool bCheck = cbMinFrameDelay.Checked;

            int nRet = m_MyCamera.MV_CC_SetBoolValue_NET("MinFrameDelay", bCheck);
            if (nRet != MyCamera.MV_OK)
            {
                ShowErrorMsg("Set MinFrameDelay Fail!", nRet);
                return;
            }
        }

        private void cbCameraType_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (false == bIni)
            {
                return;
            }

            int nRet = SetEnumIntoCombo("CameraType", ref cbCameraType);
            if (nRet != MyCamera.MV_OK)
            {
                ShowErrorMsg("Set CameraType Fail!", nRet);
            }
        }

        private void cbPartialImageOutputMode_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (false == bIni)
            {
                return;
            }

            int nRet = SetEnumIntoCombo("PartialImageOutputMode", ref cbPartialImageOutputMode);
            if (nRet != MyCamera.MV_OK)
            {
                ShowErrorMsg("Set PartialImageOutputMode Fail!", nRet);
            }
        }
    }
}
