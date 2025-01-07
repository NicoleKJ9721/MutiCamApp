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
    public partial class CMLConfigForm : Form
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
            teImageHeight.Enabled = true;
            teFrameTimeoutTime.Enabled = true;
            ReadEnumIntoCombo("StreamSelector", ref cbStreamSelector);

            ReadEnumIntoCombo("CameraType", ref cbCameraType);

            IIntValue intValue;
            _ifInstance.Parameters.GetIntValue("ImageHeight", out intValue);
            teImageHeight.Text = intValue.CurValue.ToString();
            _ifInstance.Parameters.GetIntValue("FrameTimeoutTime", out intValue);
            teFrameTimeoutTime.Text = intValue.CurValue.ToString();

            ReadEnumIntoCombo("StreamPartialImageControl", ref cbStreamPartialImageControl);

            bIni = true;
        }


        public CMLConfigForm()
        {
            InitializeComponent();
        }

        public CMLConfigForm(IInterface ifInstance)
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

        private void cbCameraType_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (false == bIni)
            {
                return;
            }

            int ret = SetEnumIntoCombo("CameraType", ref cbCameraType);
            if (ret != MvError.MV_OK)
            {
                ShowErrorMsg("Set CameraType Fail!", ret);
            }
        }

        private void cbStreamPartialImageControl_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (false == bIni)
            {
                return;
            }

            int ret = SetEnumIntoCombo("StreamPartialImageControl", ref cbStreamPartialImageControl);
            if (ret != MvError.MV_OK)
            {
                ShowErrorMsg("Set StreamPartialImageControl Fail!", ret);
            }
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
                int.Parse(teFrameTimeoutTime.Text);
            }
            catch
            {
                ShowErrorMsg("Please enter correct type!", 0);
                return;
            }

            int ret = _ifInstance.Parameters.SetIntValue("ImageHeight", int.Parse(teImageHeight.Text));
            if (MvError.MV_OK != ret)
            {
                ShowErrorMsg("Set ImageHeight Fail!", ret);
            }

            ret = _ifInstance.Parameters.SetIntValue("FrameTimeoutTime", int.Parse(teFrameTimeoutTime.Text));
            if (MvError.MV_OK != ret)
            {
                ShowErrorMsg("Set FrameTimeoutTime Fail!", ret);
            }
        }
    }
}
