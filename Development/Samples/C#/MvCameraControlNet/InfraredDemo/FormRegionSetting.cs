using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using MvCamCtrl.NET;

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
            string strType = ctrlRegionSelectComboBox.SelectedItem.ToString().Substring(0, 4);

            if (true == strType.Equals("Line", StringComparison.OrdinalIgnoreCase))
            {
                m_RegionType = RegionType.Region_Line;
            }
            else if (true == strType.Equals("Poly", StringComparison.OrdinalIgnoreCase))
            {
                m_RegionType = RegionType.Region_Polygon;
            }
            else if (true == strType.Equals("Circ", StringComparison.OrdinalIgnoreCase))
            {
                m_RegionType = RegionType.Region_Circle;
            }

            if (null == m_MyCamera)
            {
                return;
            }

            lbCurrentRegion.Text = ctrlRegionSelectComboBox.SelectedItem.ToString();

            bool bValue = false;
            m_MyCamera.MV_CC_GetBoolValue_NET("TempRegionEnable", ref bValue);
            cbSetEnableCheck.Checked = bValue;

            if (bValue)
            {
                int nRet = SetEnumIntoCombo("RegionDisplaySelector",ref ctrlRegionSelectComboBox);
                if (nRet == MyCamera.MV_OK)
                {
                    m_MyCamera.MV_CC_GetBoolValue_NET("RegionDisplayMaxTempEnable", ref bValue);
                    cbSetMaxCheck.Checked = bValue;

                    m_MyCamera.MV_CC_GetBoolValue_NET("RegionDisplayMinTempEnable", ref bValue);
                    cbSetMinCheck.Checked = bValue;

                    m_MyCamera.MV_CC_GetBoolValue_NET("RegionDisplayAvgTempEnable", ref bValue);
                    cbSetAvgCheck.Checked = bValue;
                }
            }

            if (m_bExportMode)
            {
                m_MyCamera.MV_CC_GetBoolValue_NET("TempRegionReflectEnable", ref bValue);
                cbSetReflectCheck.Checked = bValue;

                MyCamera.MVCC_FLOATVALUE oFloatValue = new MyCamera.MVCC_FLOATVALUE();
                m_MyCamera.MV_CC_GetFloatValue_NET("TempRegionReflectance", ref oFloatValue);
                teSetReflectance.Text = oFloatValue.fCurValue.ToString();

                m_MyCamera.MV_CC_GetFloatValue_NET("TempRegionTargetDistance", ref oFloatValue);
                teSetTargetDistance.Text = oFloatValue.fCurValue.ToString();

                m_MyCamera.MV_CC_GetFloatValue_NET("TempRegionEmissivity", ref oFloatValue);
                teSetEmissivity.Text = oFloatValue.fCurValue.ToString();
            }

            MyCamera.MVCC_INTVALUE_EX oIntValue = new MyCamera.MVCC_INTVALUE_EX();
            m_MyCamera.MV_CC_GetIntValueEx_NET("TempRegionPointNum", ref oIntValue);
            teSetPointNum.Text = oIntValue.nCurValue.ToString();

            m_MyCamera.MV_CC_GetIntValueEx_NET("TempRegionPointPositionX", ref oIntValue);
            teSetPointX.Text = oIntValue.nCurValue.ToString();

            m_MyCamera.MV_CC_GetIntValueEx_NET("TempRegionPointPositionY", ref oIntValue);
            teSetPointY.Text = oIntValue.nCurValue.ToString();

            m_MyCamera.MV_CC_GetIntValueEx_NET("TempRegionCenterPointPositionX", ref oIntValue);
            teSetCenterX.Text = oIntValue.nCurValue.ToString();

            m_MyCamera.MV_CC_GetIntValueEx_NET("TempRegionCenterPointPositionY", ref oIntValue);
            teSetCenterY.Text = oIntValue.nCurValue.ToString();

            m_MyCamera.MV_CC_GetIntValueEx_NET("TempRegionRadius", ref oIntValue);
            teSetRadius.Text = oIntValue.nCurValue.ToString();

            ReadEnumIntoCombo("TempRegionPointSelector", ref cbSetPointInfrx);

            switch (m_RegionType)
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

        MyCamera m_MyCamera;
        RegionType m_RegionType;
        ComboBox ctrlRegionSelectComboBox;
        bool m_bExportMode;

        public FormRegionSetting()
        {
            InitializeComponent();
        }

        public FormRegionSetting(ref MyCamera MyCamera, ref ComboBox RegionSelectComboBox, ref bool bExportModeCheck)
            : this()
        {
            m_MyCamera = MyCamera;
            m_RegionType = RegionType.Region_Point;
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

        private void cbSetPointInfrx_SelectedIndexChanged(object sender, EventArgs e)
        {
            int nRet = SetEnumIntoCombo("TempRegionPointSelector", ref cbSetPointInfrx);
            if (nRet != MyCamera.MV_OK)
            {
                ShowErrorMsg("Set TempRegionPointSelector Fail!", nRet);
            }

            MyCamera.MVCC_INTVALUE_EX oIntValue = new MyCamera.MVCC_INTVALUE_EX();
            m_MyCamera.MV_CC_GetIntValueEx_NET("TempRegionPointPositionX", ref oIntValue);
            teSetPointX.Text = oIntValue.nCurValue.ToString();

            m_MyCamera.MV_CC_GetIntValueEx_NET("TempRegionPointPositionY", ref oIntValue);
            teSetPointY.Text = oIntValue.nCurValue.ToString();
        }

        private void bnSetParam_Click(object sender, EventArgs e)
        {
            bool bRegionEnable = cbSetEnableCheck.Checked;
            int nRet = m_MyCamera.MV_CC_SetBoolValue_NET("TempRegionEnable", bRegionEnable);
            if (MyCamera.MV_OK == nRet && bRegionEnable)
            {
                nRet = SetEnumIntoCombo("RegionDisplaySelector", ref ctrlRegionSelectComboBox);
                if (nRet != MyCamera.MV_OK)
                {
                    ShowErrorMsg("Set RegionDisplaySelector Fail!", nRet);
                    return;
                }

                m_MyCamera.MV_CC_SetBoolValue_NET("RegionDisplayEnable", true);
                m_MyCamera.MV_CC_SetBoolValue_NET("RegionDisplayMaxTempEnable", cbSetMaxCheck.Checked);
                m_MyCamera.MV_CC_SetBoolValue_NET("RegionDisplayMinTempEnable", cbSetMinCheck.Checked);
                m_MyCamera.MV_CC_SetBoolValue_NET("RegionDisplayAvgTempEnable", cbSetAvgCheck.Checked);
            }
            else if (MyCamera.MV_OK != nRet)
            {
                ShowErrorMsg("Set TempRegionEnable Fail!", nRet);
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

                nRet = m_MyCamera.MV_CC_SetBoolValue_NET("TempRegionReflectEnable", cbSetReflectCheck.Checked);
                if (MyCamera.MV_OK != nRet)
                {
                    ShowErrorMsg("Set TempRegionReflectEnable Fail!", nRet);
                }

                nRet = m_MyCamera.MV_CC_SetFloatValue_NET("TempRegionReflectance", float.Parse(teSetReflectance.Text));
                if (MyCamera.MV_OK != nRet)
                {
                    ShowErrorMsg("Set TempRegionReflectance Fail!", nRet);
                }

                nRet = m_MyCamera.MV_CC_SetFloatValue_NET("TempRegionEmissivity", float.Parse(teSetEmissivity.Text));
                if (MyCamera.MV_OK != nRet)
                {
                    ShowErrorMsg("Set TempRegionEmissivity Fail!", nRet);
                }

                nRet = m_MyCamera.MV_CC_SetFloatValue_NET("TempRegionTargetDistance", float.Parse(teSetTargetDistance.Text));
                if (MyCamera.MV_OK != nRet)
                {
                    ShowErrorMsg("Set TempRegionTargetDistance Fail!", nRet);
                }
            }

            nRet = m_MyCamera.MV_CC_SetCommandValue_NET("TempControlLoad");
            if (nRet != MyCamera.MV_OK)
            {
                ShowErrorMsg("Exec TempControlLoad Fail!", nRet);
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

            int nRet = MyCamera.MV_OK;
            if (nPrePotNum != int.Parse(teSetPointNum.Text))
            {
                nRet = m_MyCamera.MV_CC_SetIntValueEx_NET("TempRegionPointNum", int.Parse(teSetPointNum.Text));
                if (nRet != MyCamera.MV_OK)
                {
                    ShowErrorMsg("Set TempRegionPointNum Fail!", nRet);
                    return;
                }

                nRet = m_MyCamera.MV_CC_SetCommandValue_NET("TempControlLoad");
                if (nRet != MyCamera.MV_OK)
                {
                    ShowErrorMsg("Exec TempControlLoad Fail!", nRet);
                    return;
                }

                ReadEnumIntoCombo("TempRegionPointSelector", ref cbSetPointInfrx);

                MyCamera.MVCC_INTVALUE_EX oIntValue = new MyCamera.MVCC_INTVALUE_EX();
                m_MyCamera.MV_CC_GetIntValueEx_NET("TempRegionPointPositionX", ref oIntValue);
                teSetPointX.Text = oIntValue.nCurValue.ToString();

                m_MyCamera.MV_CC_GetIntValueEx_NET("TempRegionPointPositionY", ref oIntValue);
                teSetPointY.Text = oIntValue.nCurValue.ToString();

                return;
            }

            switch (m_RegionType)
            {
                case RegionType.Region_Point:
                case RegionType.Region_Polygon:
                case RegionType.Region_Line:
                    {
                        nRet = m_MyCamera.MV_CC_SetIntValueEx_NET("TempRegionPointPositionX", int.Parse(teSetPointX.Text));
                        if (nRet != MyCamera.MV_OK)
                        {
                            ShowErrorMsg("Set TempRegionPointPositionX Fail!", nRet);
                        }

                        nRet = m_MyCamera.MV_CC_SetIntValueEx_NET("TempRegionPointPositionY", int.Parse(teSetPointY.Text));
                        if (nRet != MyCamera.MV_OK)
                        {
                            ShowErrorMsg("Set TempRegionPointPositionY Fail!", nRet);
                        }

                        break;
                    }
                case RegionType.Region_Circle:
                    {
                        nRet = m_MyCamera.MV_CC_SetIntValueEx_NET("TempRegionCenterPointPositionX", int.Parse(teSetCenterX.Text));
                        if (nRet != MyCamera.MV_OK)
                        {
                            ShowErrorMsg("Set TempRegionCenterPointPositionX Fail!", nRet);
                        }

                        nRet = m_MyCamera.MV_CC_SetIntValueEx_NET("TempRegionCenterPointPositionY", int.Parse(teSetCenterY.Text));
                        if (nRet != MyCamera.MV_OK)
                        {
                            ShowErrorMsg("Set TempRegionCenterPointPositionY Fail!", nRet);
                        }

                        nRet = m_MyCamera.MV_CC_SetIntValueEx_NET("TempRegionRadius", int.Parse(teSetRadius.Text));
                        if (nRet != MyCamera.MV_OK)
                        {
                            ShowErrorMsg("Set TempRegionRadius Fail!", nRet);
                        }

                        break;
                    }
                default:
                    break;
            }
            nRet = m_MyCamera.MV_CC_SetCommandValue_NET("TempControlLoad");
            if (nRet != MyCamera.MV_OK)
            {
                ShowErrorMsg("Exec TempControlLoad Fail!", nRet);
            }
        }
    }
}
