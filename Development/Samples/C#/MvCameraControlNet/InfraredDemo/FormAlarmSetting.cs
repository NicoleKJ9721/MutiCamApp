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
    public partial class FormAlarmSetting : Form
    {
        MyCamera m_MyCamera;
        ComboBox ctrlRegionSelectComboBox;

        public int SetEnumIntoCombo(string strKey, ref ComboBox ctrlComboBox)
        {
            string str=ctrlComboBox.SelectedItem.ToString();
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
            if (null == m_MyCamera)
            {
                return;
            }

            lbCurrentRegion.Text = ctrlRegionSelectComboBox.SelectedItem.ToString();

            int nRet = SetEnumIntoCombo("TempRegionAlarmRuleSelector", ref ctrlRegionSelectComboBox);
            if (nRet != MyCamera.MV_OK)
            {
                ShowErrorMsg("Read TempRegionAlarmRuleSelector Fail!", nRet);
                return;
            }

            bool bValue = false;
            m_MyCamera.MV_CC_GetBoolValue_NET("TempRegionAlarmRuleEnable", ref bValue);
            cbSetAlarmEnableCheck.Checked = bValue;

            MyCamera.MV_XML_AccessMode oAccessMode = MyCamera.MV_XML_AccessMode.AM_NA;
            m_MyCamera.MV_XML_GetNodeAccessMode_NET("TempRegionAlarmRuleEnable", ref oAccessMode);
            if (oAccessMode != MyCamera.MV_XML_AccessMode.AM_RW)
            {
                cbSetAlarmEnableCheck.Enabled = false;
            }

            MyCamera.MVCC_FLOATVALUE oFloatValue = new MyCamera.MVCC_FLOATVALUE();
            m_MyCamera.MV_CC_GetFloatValue_NET("TempRegionAlarmReferenceValue", ref oFloatValue);
            teSetAlarmReference.Text = oFloatValue.fCurValue.ToString();

            m_MyCamera.MV_CC_GetFloatValue_NET("TempRegionAlarmRecoveryABSValue", ref oFloatValue);
            teSetAlarmAbs.Text = oFloatValue.fCurValue.ToString();

            ReadEnumIntoCombo("TempRegionAlarmRuleSource", ref cbSetAlarmSource);
            ReadEnumIntoCombo("TempRegionAlarmRuleCondition", ref cbSetAlarmCondition);
        }

        public FormAlarmSetting()
        {
            InitializeComponent();
        }

        public FormAlarmSetting(ref MyCamera MyCamera, ref ComboBox RegionSelectComboBox)
            : this()
        {
            m_MyCamera = MyCamera;
            //m_strRegion = strRegion;
            ctrlRegionSelectComboBox = RegionSelectComboBox;
            InitParameter();
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

        private void bnOK_Click(object sender, EventArgs e)
        {
            int nRet = MyCamera.MV_OK;
            if (cbSetAlarmEnableCheck.Enabled)
            {
                m_MyCamera.MV_CC_SetBoolValue_NET("TempRegionAlarmRuleEnable", cbSetAlarmEnableCheck.Checked);
                if (nRet != MyCamera.MV_OK)
                {
                    ShowErrorMsg("Set TempRegionAlarmRuleEnable Fail!", nRet);
                    return;
                }

                if (cbSetAlarmEnableCheck.Checked)
                {
                    m_MyCamera.MV_CC_SetBoolValue_NET("RegionDisplayAlarmEnable", true);
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

            nRet = SetEnumIntoCombo("TempRegionAlarmRuleSource", ref cbSetAlarmSource);
            if (nRet != MyCamera.MV_OK)
            {
                ShowErrorMsg("Set TempRegionAlarmRuleSource Fail!", nRet);
            }

            nRet = SetEnumIntoCombo("TempRegionAlarmRuleCondition", ref cbSetAlarmCondition);
            if (nRet != MyCamera.MV_OK)
            {
                ShowErrorMsg("Set TempRegionAlarmRuleCondition Fail!", nRet);
            }

            nRet = m_MyCamera.MV_CC_SetFloatValue_NET("TempRegionAlarmReferenceValue", float.Parse(teSetAlarmReference.Text));
            if (nRet != MyCamera.MV_OK)
            {
                ShowErrorMsg("Set TempRegionAlarmReferenceValue Fail!", nRet);
            }

            nRet = m_MyCamera.MV_CC_SetFloatValue_NET("TempRegionAlarmRecoveryABSValue", float.Parse(teSetAlarmAbs.Text));
            if (nRet != MyCamera.MV_OK)
            {
                ShowErrorMsg("Set TempRegionAlarmRecoveryABSValue Fail!", nRet);
            }

            nRet = m_MyCamera.MV_CC_SetCommandValue_NET("TempControlLoad");
            if (nRet != MyCamera.MV_OK)
            {
                ShowErrorMsg("Exec TempControlLoad Fail!", nRet);
            }
            this.Hide();
        }

        private void bnCancel_Click(object sender, EventArgs e)
        {
            this.Hide();
        }
    }
}
