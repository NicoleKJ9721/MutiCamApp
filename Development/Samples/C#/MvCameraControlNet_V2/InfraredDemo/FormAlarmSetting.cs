using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using MvCameraControl;

namespace InfraredDemo
{
    public partial class FormAlarmSetting : Form
    {
        IDevice device;
        ComboBox ctrlRegionSelectComboBox;

        public void InitParameter()
        {
            if (null == device)
            {
                return;
            }

            string currentRegion = ctrlRegionSelectComboBox.SelectedItem.ToString();
            lbCurrentRegion.Text = currentRegion;

            int result = device.Parameters.SetEnumValue("TempRegionAlarmRuleSelector", (uint)ctrlRegionSelectComboBox.SelectedIndex);
            if (result != MvError.MV_OK)
            {
                ShowErrorMsg("Read TempRegionAlarmRuleSelector Fail!", result);
                return;
            }

            bool boolValue;
            device.Parameters.GetBoolValue("TempRegionAlarmRuleEnable", out boolValue);
            cbSetAlarmEnableCheck.Checked = boolValue;

            XmlAccessMode accessMode;
            device.Parameters.GetNodeAccessMode("TempRegionAlarmRuleEnable", out accessMode);
            if (accessMode != XmlAccessMode.RW)
            {
                cbSetAlarmEnableCheck.Enabled = false;
            }

            IFloatValue floatValue;
            device.Parameters.GetFloatValue("TempRegionAlarmReferenceValue", out floatValue);
            teSetAlarmReference.Text = floatValue.CurValue.ToString();

            device.Parameters.GetFloatValue("TempRegionAlarmRecoveryABSValue", out floatValue);
            teSetAlarmAbs.Text = floatValue.CurValue.ToString();

            ReadEnumIntoCombo("TempRegionAlarmRuleSource", ref cbSetAlarmSource);
            ReadEnumIntoCombo("TempRegionAlarmRuleCondition", ref cbSetAlarmCondition);
        }

        public FormAlarmSetting()
        {
            InitializeComponent();
        }

        public FormAlarmSetting(ref IDevice device, ref ComboBox RegionSelectComboBox)
            : this()
        {
            this.device = device;
            ctrlRegionSelectComboBox = RegionSelectComboBox;
            InitParameter();
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

        private int ReadEnumIntoCombo(string strKey, ref ComboBox ctrlComboBox)
        {
            IEnumValue enumValue;
            int result = device.Parameters.GetEnumValue(strKey, out enumValue);
            if (MvError.MV_OK != result)
            {
                return result;
            }
            ctrlComboBox.Items.Clear();
            for (int i = 0; i < enumValue.SupportedNum; ++i)
            {
                ctrlComboBox.Items.Add(enumValue.SupportEnumEntries[i].Symbolic);
                if (enumValue.CurEnumEntry.Value == enumValue.SupportEnumEntries[i].Value)
                {
                    ctrlComboBox.SelectedIndex = i;
                }
            }
            return MvError.MV_OK;
        }

        private void bnOK_Click(object sender, EventArgs e)
        {
            int result = MvError.MV_OK;
            if (cbSetAlarmEnableCheck.Enabled)
            {
                device.Parameters.SetBoolValue("TempRegionAlarmRuleEnable", cbSetAlarmEnableCheck.Checked);
                if (result != MvError.MV_OK)
                {
                    ShowErrorMsg("Set TempRegionAlarmRuleEnable Fail!", result);
                    return;
                }

                if (cbSetAlarmEnableCheck.Checked)
                {
                    device.Parameters.SetBoolValue("RegionDisplayAlarmEnable", true);
                }
            }

            try
            {
                float.Parse(teSetAlarmReference.Text);
                float.Parse(teSetAlarmAbs.Text);
            }
            catch
            {
                ShowErrorMsg("Please enter correct type!", 0);
                return;
            }

            result = device.Parameters.SetEnumValueByString("TempRegionAlarmRuleSource", cbSetAlarmSource.SelectedItem.ToString());
            if (result != MvError.MV_OK)
            {
                ShowErrorMsg("Set TempRegionAlarmRuleSource Fail!", result);
            }

            result = device.Parameters.SetEnumValueByString("TempRegionAlarmRuleCondition", cbSetAlarmCondition.SelectedItem.ToString());
            if (result != MvError.MV_OK)
            {
                ShowErrorMsg("Set TempRegionAlarmRuleCondition Fail!", result);
            }

            result = device.Parameters.SetFloatValue("TempRegionAlarmReferenceValue", float.Parse(teSetAlarmReference.Text));
            if (result != MvError.MV_OK)
            {
                ShowErrorMsg("Set TempRegionAlarmReferenceValue Fail!", result);
            }

            result = device.Parameters.SetFloatValue("TempRegionAlarmRecoveryABSValue", float.Parse(teSetAlarmAbs.Text));
            if (result != MvError.MV_OK)
            {
                ShowErrorMsg("Set TempRegionAlarmRecoveryABSValue Fail!", result);
            }

            result = device.Parameters.SetCommandValue("TempControlLoad");
            if (result != MvError.MV_OK)
            {
                ShowErrorMsg("Exec TempControlLoad Fail!", result);
            }
            this.Hide();
        }

        private void bnCancel_Click(object sender, EventArgs e)
        {
            this.Hide();
        }
    }
}
