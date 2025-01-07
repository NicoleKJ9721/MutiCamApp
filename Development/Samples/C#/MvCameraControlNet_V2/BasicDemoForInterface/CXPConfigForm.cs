using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using MvCameraControl;

namespace InterfaceBasicDemo
{
    public partial class CXPConfigForm : Form
    {
        IInterface _ifInstance;

        bool bIni = false;

        private int ReadEnumIntoCombo(string strKey, ref ComboBox ctrlComboBox)
        {
            IEnumValue enumValue;
            int ret = _ifInstance.Parameters.GetEnumValue(strKey, out enumValue);
            if (ret != MvError.MV_OK)
            {
                return ret;
            }

            ctrlComboBox.Items.Clear();
            for (int i = 0; i < enumValue.SupportedNum; ++i)
            {
                ctrlComboBox.Items.Add(enumValue.SupportEnumEntries[i].Symbolic);
            }

            int nIndex = ctrlComboBox.FindString(enumValue.CurEnumEntry.Symbolic);
            if (nIndex >= 0)
            {
                ctrlComboBox.SelectedIndex = nIndex;
            }

            return MvError.MV_OK;
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

        public int SetEnumIntoCombo(string strKey, ref ComboBox ctrlComboBox)
        {
            string str = ctrlComboBox.SelectedItem.ToString();
            IEnumValue enumValue;
            int ret = _ifInstance.Parameters.GetEnumValue(strKey, out enumValue);
            if (ret != MvError.MV_OK)
            {
                return ret;
            }

            for (int i = 0; i < enumValue.SupportedNum; ++i)
            {

                if (str.Equals(enumValue.SupportEnumEntries[i].Symbolic, StringComparison.OrdinalIgnoreCase))
                {
                    ret = _ifInstance.Parameters.SetEnumValue(strKey, enumValue.SupportEnumEntries[i].Value);
                    if (ret != MvError.MV_OK)
                    {
                        return ret;
                    }

                    break;
                }
            }
            return ret;
        }

        public void InitParameter()
        {
            if (null == _ifInstance)
            {
                return;
            }

            teIspGamma.Enabled = true;

            ReadEnumIntoCombo("StreamSelector", ref cbStreamSelector);


            IStringValue stringValue;
            _ifInstance.Parameters.GetStringValue("CurrentStreamDevice", out stringValue);
            teCurrentStreamDevice.Text = stringValue.CurValue;

            IIntValue intValue;
            _ifInstance.Parameters.GetIntValue("StreamEnableStatus", out intValue);
            teStreamEnableStatus.Text = intValue.CurValue.ToString();

            bool bValue = false;
            _ifInstance.Parameters.GetBoolValue("BayerCFAEnable", out bValue);
            cbBayerCFAEnable.Checked = bValue;

            _ifInstance.Parameters.GetBoolValue("IspGammaEnable", out bValue);
            cbIspGammaEnable.Checked = bValue;

            IFloatValue floatValue;
            _ifInstance.Parameters.GetFloatValue("IspGamma", out floatValue);
            teIspGamma.Text = floatValue.CurValue.ToString();

            bIni = true;
        }

        public CXPConfigForm()
        {
            InitializeComponent();
        }

        public CXPConfigForm(IInterface ifInstance)
            : this()
        {
            _ifInstance = ifInstance;

            InitParameter();
        }

        private void cbStreamSelector_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (false == bIni)
            {
                return;
            }

            int ret = SetEnumIntoCombo("StreamSelector", ref cbStreamSelector);
            if (ret != MvError.MV_OK)
            {
                ShowErrorMsg("Set StreamSelector Fail!", ret);
            }
        }

        private void cbBayerCFAEnable_CheckedChanged(object sender, EventArgs e)
        {
            bool bCheck = cbBayerCFAEnable.Checked;

            int ret = _ifInstance.Parameters.SetBoolValue("BayerCFAEnable", bCheck);
            if (ret != MvError.MV_OK)
            {
                ShowErrorMsg("Set BayerCFAEnable Fail!", ret);
                return;
            }

        }

        private void cbIspGammaEnable_CheckedChanged(object sender, EventArgs e)
        {
            bool bCheck = cbIspGammaEnable.Checked;

            int ret =_ifInstance.Parameters.SetBoolValue("IspGammaEnable", bCheck);
            if (ret != MvError.MV_OK)
            {
                ShowErrorMsg("Set IspGammaEnable Fail!", ret);
                return;
            }
        }

        private void bnSetParameter_Click(object sender, EventArgs e)
        {
            try
            {
                float.Parse(teIspGamma.Text);
            }
            catch
            {
                ShowErrorMsg("Please enter correct type!", 0);
                return;
            }

            int ret = _ifInstance.Parameters.SetFloatValue("IspGamma", float.Parse(teIspGamma.Text));
            if (MvError.MV_OK != ret)
            {
                ShowErrorMsg("Set IspGamma Fail!", ret);
            }
        }

        private void bnGetParameter_Click(object sender, EventArgs e)
        {
            InitParameter();
        }
    }
}
