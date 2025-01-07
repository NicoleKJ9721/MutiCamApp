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
    public partial class FormRegionSetting : Form
    {
        public enum RegionType
        {
            Region_Point = 0,
            Region_Polygon = 1,
            Region_Line = 2,
            Region_Circle = 3,
        };

        public void InitParameter()
        {
            string strType = ctrlRegionSelectComboBox.SelectedItem.ToString().Substring(0, 4);

            if (true == strType.Equals("Line", StringComparison.OrdinalIgnoreCase))
            {
                regionType = RegionType.Region_Line;
            }
            else if (true == strType.Equals("Poly", StringComparison.OrdinalIgnoreCase))
            {
                regionType = RegionType.Region_Polygon;
            }
            else if (true == strType.Equals("Circ", StringComparison.OrdinalIgnoreCase))
            {
                regionType = RegionType.Region_Circle;
            }

            if (null == device)
            {
                return;
            }

            lbCurrentRegion.Text = ctrlRegionSelectComboBox.SelectedItem.ToString();

            bool boolValue;
            device.Parameters.GetBoolValue("TempRegionEnable", out boolValue);
            cbSetEnableCheck.Checked = boolValue;

            if (boolValue)
            {
                int result = device.Parameters.SetEnumValue("RegionDisplaySelector", (uint)ctrlRegionSelectComboBox.SelectedIndex);
                if (result == MvError.MV_OK)
                {
                    device.Parameters.GetBoolValue("RegionDisplayMaxTempEnable", out boolValue);
                    cbSetMaxCheck.Checked = boolValue;

                    device.Parameters.GetBoolValue("RegionDisplayMinTempEnable", out boolValue);
                    cbSetMinCheck.Checked = boolValue;

                    device.Parameters.GetBoolValue("RegionDisplayAvgTempEnable", out boolValue);
                    cbSetAvgCheck.Checked = boolValue;
                }
            }

            if (m_bExportMode)
            {
                device.Parameters.GetBoolValue("TempRegionReflectEnable", out boolValue);
                cbSetReflectCheck.Checked = boolValue;

                IFloatValue floatValue;
                device.Parameters.GetFloatValue("TempRegionReflectance", out floatValue);
                teSetReflectance.Text = floatValue.CurValue.ToString();

                device.Parameters.GetFloatValue("TempRegionTargetDistance", out floatValue);
                teSetTargetDistance.Text = floatValue.CurValue.ToString();

                device.Parameters.GetFloatValue("TempRegionEmissivity", out floatValue);
                teSetEmissivity.Text = floatValue.CurValue.ToString();
            }

            IIntValue intValue;
            device.Parameters.GetIntValue("TempRegionPointNum", out intValue);
            teSetPointNum.Text = intValue.CurValue.ToString();

            device.Parameters.GetIntValue("TempRegionPointPositionX", out intValue);
            teSetPointX.Text = intValue.CurValue.ToString();

            device.Parameters.GetIntValue("TempRegionPointPositionY", out intValue);
            teSetPointY.Text = intValue.CurValue.ToString();

            device.Parameters.GetIntValue("TempRegionCenterPointPositionX", out intValue);
            teSetCenterX.Text = intValue.CurValue.ToString();

            device.Parameters.GetIntValue("TempRegionCenterPointPositionY", out intValue);
            teSetCenterY.Text = intValue.CurValue.ToString();

            device.Parameters.GetIntValue("TempRegionRadius", out intValue);
            teSetRadius.Text = intValue.CurValue.ToString();

            ReadEnumIntoCombo("TempRegionPointSelector", ref cbSetPointInfrx);

            switch (regionType)
            {
                case RegionType.Region_Point:
                    cbSetPointInfrx.Enabled = false;
                    teSetPointNum.Enabled = false;
                    teSetCenterX.Enabled = false;
                    teSetCenterY.Enabled = false;
                    teSetRadius.Enabled = false;
                    break;
                case RegionType.Region_Polygon:
                    teSetCenterX.Enabled = false;
                    teSetCenterY.Enabled = false;
                    teSetRadius.Enabled = false;
                    break;
                case RegionType.Region_Line:
                    teSetPointNum.Enabled = false;
                    teSetCenterX.Enabled = false;
                    teSetCenterY.Enabled = false;
                    teSetRadius.Enabled = false;
                    break;
                case RegionType.Region_Circle:
                    cbSetPointInfrx.Enabled = false;
                    teSetPointNum.Enabled = false;
                    teSetPointX.Enabled = false;
                    teSetPointY.Enabled = false;
                    break;
                default:
                    break;
            }
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

        IDevice device;
        RegionType regionType;
        ComboBox ctrlRegionSelectComboBox;
        bool m_bExportMode;

        public FormRegionSetting()
        {
            InitializeComponent();
        }

        public FormRegionSetting(ref IDevice device, ref ComboBox RegionSelectComboBox, ref bool bExportModeCheck)
            : this()
        {
            this.device = device;
            regionType = RegionType.Region_Point;
            ctrlRegionSelectComboBox = RegionSelectComboBox;
            m_bExportMode = bExportModeCheck;

            InitParameter();

            if (!m_bExportMode)
            {
                cbSetReflectCheck.Hide();
                teSetReflectance.Hide();
                teSetEmissivity.Hide();
                teSetTargetDistance.Hide();

                lbSetReflectCheck.Hide();
                lbSetReflectance.Hide();
                lbSetEmissivity.Hide();
                lbSetTargetDistance.Hide();
            }
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

        private void cbSetPointInfrx_SelectedIndexChanged(object sender, EventArgs e)
        {
            int result = device.Parameters.SetEnumValueByString("TempRegionPointSelector", cbSetPointInfrx.SelectedItem.ToString());
            if (result != MvError.MV_OK)
            {
                ShowErrorMsg("Set TempRegionPointSelector Fail!", result);
            }

            IIntValue intValue;
            device.Parameters.GetIntValue("TempRegionPointPositionX", out intValue);
            teSetPointX.Text = intValue.CurValue.ToString();

            device.Parameters.GetIntValue("TempRegionPointPositionY", out intValue);
            teSetPointY.Text = intValue.CurValue.ToString();
        }

        private void bnSetParam_Click(object sender, EventArgs e)
        {
            bool regionEnable = cbSetEnableCheck.Checked;
            int result = device.Parameters.SetBoolValue("TempRegionEnable", regionEnable);
            if (MvError.MV_OK == result && regionEnable)
            {
                result = device.Parameters.SetEnumValueByString("RegionDisplaySelector", ctrlRegionSelectComboBox.SelectedItem.ToString());
                if (result != MvError.MV_OK)
                {
                    ShowErrorMsg("Set RegionDisplaySelector Fail!", result);
                    return;
                }

                device.Parameters.SetBoolValue("RegionDisplayEnable", true);
                device.Parameters.SetBoolValue("RegionDisplayMaxTempEnable", cbSetMaxCheck.Checked);
                device.Parameters.SetBoolValue("RegionDisplayMinTempEnable", cbSetMinCheck.Checked);
                device.Parameters.SetBoolValue("RegionDisplayAvgTempEnable", cbSetAvgCheck.Checked);
            }
            else if (MvError.MV_OK != result)
            {
                ShowErrorMsg("Set TempRegionEnable Fail!", result);
                return;
            }

            if (m_bExportMode)
            {
                try
                {
                    float.Parse(teSetReflectance.Text);
                    float.Parse(teSetEmissivity.Text);
                    float.Parse(teSetTargetDistance.Text);
                }
                catch
                {
                    ShowErrorMsg("Please enter correct type!", 0);
                    return;
                }

                result = device.Parameters.SetBoolValue("TempRegionReflectEnable", cbSetReflectCheck.Checked);
                if (MvError.MV_OK != result)
                {
                    ShowErrorMsg("Set TempRegionReflectEnable Fail!", result);
                }

                result = device.Parameters.SetFloatValue("TempRegionReflectance", float.Parse(teSetReflectance.Text));
                if (MvError.MV_OK != result)
                {
                    ShowErrorMsg("Set TempRegionReflectance Fail!", result);
                }

                result = device.Parameters.SetFloatValue("TempRegionEmissivity", float.Parse(teSetEmissivity.Text));
                if (MvError.MV_OK != result)
                {
                    ShowErrorMsg("Set TempRegionEmissivity Fail!", result);
                }

                result = device.Parameters.SetFloatValue("TempRegionTargetDistance", float.Parse(teSetTargetDistance.Text));
                if (MvError.MV_OK != result)
                {
                    ShowErrorMsg("Set TempRegionTargetDistance Fail!", result);
                }
            }

            result = device.Parameters.SetCommandValue("TempControlLoad");
            if (result != MvError.MV_OK)
            {
                ShowErrorMsg("Exec TempControlLoad Fail!", result);
            }
        }

        private void bnSetPoint_Click(object sender, EventArgs e)
        {
            try
            {
                int.Parse(teSetPointNum.Text);
                int.Parse(teSetPointX.Text);
                int.Parse(teSetPointY.Text);
                int.Parse(teSetCenterX.Text);
                int.Parse(teSetCenterY.Text);
                int.Parse(teSetRadius.Text);
            }
            catch
            {
                ShowErrorMsg("Please enter correct type!", 0);
                return;
            }

            int nPrePotNum = int.Parse(teSetPointNum.Text);

            IIntValue intValue;
            device.Parameters.GetIntValue("TempRegionPointNum", out intValue);

            int result = MvError.MV_OK;
            if (nPrePotNum != intValue.CurValue)
            {
                result = device.Parameters.SetIntValue("TempRegionPointNum", int.Parse(teSetPointNum.Text));
                if (result != MvError.MV_OK)
                {
                    ShowErrorMsg("Set TempRegionPointNum Fail!", result);
                    return;
                }

                result = device.Parameters.SetCommandValue("TempControlLoad");
                if (result != MvError.MV_OK)
                {
                    ShowErrorMsg("Exec TempControlLoad Fail!", result);
                    return;
                }

                ReadEnumIntoCombo("TempRegionPointSelector", ref cbSetPointInfrx);

                device.Parameters.GetIntValue("TempRegionPointPositionX", out intValue);
                teSetPointX.Text = intValue.CurValue.ToString();

                device.Parameters.GetIntValue("TempRegionPointPositionY", out intValue);
                teSetPointY.Text = intValue.CurValue.ToString();

                return;
            }

            switch (regionType)
            {
                case RegionType.Region_Point:
                case RegionType.Region_Polygon:
                case RegionType.Region_Line:
                    {
                        result = device.Parameters.SetIntValue("TempRegionPointPositionX", int.Parse(teSetPointX.Text));
                        if (result != MvError.MV_OK)
                        {
                            ShowErrorMsg("Set TempRegionPointPositionX Fail!", result);
                        }

                        result = device.Parameters.SetIntValue("TempRegionPointPositionY", int.Parse(teSetPointY.Text));
                        if (result != MvError.MV_OK)
                        {
                            ShowErrorMsg("Set TempRegionPointPositionY Fail!", result);
                        }

                        break;
                    }
                case RegionType.Region_Circle:
                    {
                        result = device.Parameters.SetIntValue("TempRegionCenterPointPositionX", int.Parse(teSetCenterX.Text));
                        if (result != MvError.MV_OK)
                        {
                            ShowErrorMsg("Set TempRegionCenterPointPositionX Fail!", result);
                        }

                        result = device.Parameters.SetIntValue("TempRegionCenterPointPositionY", int.Parse(teSetCenterY.Text));
                        if (result != MvError.MV_OK)
                        {
                            ShowErrorMsg("Set TempRegionCenterPointPositionY Fail!", result);
                        }

                        result = device.Parameters.SetIntValue("TempRegionRadius", int.Parse(teSetRadius.Text));
                        if (result != MvError.MV_OK)
                        {
                            ShowErrorMsg("Set TempRegionRadius Fail!", result);
                        }

                        break;
                    }
                default:
                    break;
            }
            result = device.Parameters.SetCommandValue("TempControlLoad");
            if (result != MvError.MV_OK)
            {
                ShowErrorMsg("Exec TempControlLoad Fail!", result);
            }
        }
    }
}
