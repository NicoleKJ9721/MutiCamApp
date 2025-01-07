
// InfraredDemoDlg.cpp : implementation file
#include "stdafx.h"
#include "RegionSettingDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CInfraredDemoDlg dialog
CRegionSettingDlg::CRegionSettingDlg(CMvCamera *pcMyCamera, const CString &strRegion, bool bExportMode, CWnd* pParent /*=NULL*/)
	: CDialog(CRegionSettingDlg::IDD, pParent)
	, m_pcMyCamera(pcMyCamera)
	, m_RegionType(Region_Point)
	, m_strRegion(strRegion)
	, m_bExportMode(bExportMode)
	, m_fReflectance(0)
	, m_fEmissivity(0)
	, m_fTargetDistance(0)
	, m_nPointNum(0)
	, m_nPointX(0)
	, m_nPointY(0)
	, m_nCenterX(0)
	, m_nCenterY(0)
	, m_nRadius(0)
{
	CString strType = strRegion.Left(4);
	if (strType.CompareNoCase(L"Line") == 0)
	{
		m_RegionType = Region_Line;
	}
	else if (strType.CompareNoCase(L"Poly") == 0)
	{
		m_RegionType = Region_Polygon;
	}
	else if (strType.CompareNoCase(L"Circ") == 0)
	{
		m_RegionType = Region_Circle;
	}

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRegionSettingDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SET_POINTINFRX_COMBO, m_ctrlPointIndexCombo);
	DDX_Control(pDX, IDC_SET_ENBLE_CHECK, m_ctrlRegionEnableCheck);
	DDX_Control(pDX, IDC_SET_MAX_CHECK, m_ctrlMaxCheck);
	DDX_Control(pDX, IDC_SET_MIN_CHECK, m_ctrlMinCheck);
	DDX_Control(pDX, IDC_SET_AVG_CHECK, m_ctrlAvgCheck);
	DDX_Control(pDX, IDC_SET_REFLECT_CHECK, m_ctrlReflectEnableCheck);
	DDX_Text(pDX, IDC_SET_REFLECTANCE_EDIT, m_fReflectance);
	DDX_Text(pDX, IDC_SET_EMISSIVITY_EDIT, m_fEmissivity);
	DDX_Text(pDX, IDC_SET_TARGETDIST_EDIT, m_fTargetDistance);
	DDX_Text(pDX, IDC_SET_POINTNUM_EDIT, m_nPointNum);
	DDX_Text(pDX, IDC_SET_POINTX_EDIT, m_nPointX);
	DDX_Text(pDX, IDC_SET_POINTY_EDIT, m_nPointY);
	DDX_Text(pDX, IDC_SET_CENTERX_EDIT, m_nCenterX);
	DDX_Text(pDX, IDC_SET_CENTERY_EDIT, m_nCenterY);
	DDX_Text(pDX, IDC_SET_RADIUS_EDIT, m_nRadius);
}

BEGIN_MESSAGE_MAP(CRegionSettingDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()

	ON_BN_CLICKED(IDC_SET_SETPARAM_BUTTON, &CRegionSettingDlg::OnBnClickedSaveParameter)
	ON_BN_CLICKED(IDC_SET_SETPOINT_BUTTON, &CRegionSettingDlg::OnBnClickedPointInfo)
	ON_CBN_SELCHANGE(IDC_SET_POINTINFRX_COMBO, &CRegionSettingDlg::OnPointIndexChanged)

    ON_WM_CLOSE()
END_MESSAGE_MAP()


// CInfraredDemoDlg message handlers
BOOL CRegionSettingDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	// when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	InitParameter();

	if (!m_bExportMode)
	{
		GetDlgItem(IDC_SET_REFLECT_CHECK)->ShowWindow(FALSE);
		GetDlgItem(IDC_SET_REFLECTANCE_EDIT)->ShowWindow(FALSE);
		GetDlgItem(IDC_SET_EMISSIVITY_EDIT)->ShowWindow(FALSE);
		GetDlgItem(IDC_SET_TARGETDIST_EDIT)->ShowWindow(FALSE);

		GetDlgItem(IDC_SET_REFLECTENABLE_STATIC)->ShowWindow(FALSE);
		GetDlgItem(IDC_SET_REFLECTANCE_STATIC)->ShowWindow(FALSE);
		GetDlgItem(IDC_SET_TARGETDIST_STATIC)->ShowWindow(FALSE);
		GetDlgItem(IDC_SET_EMISSIVITY_STATIC)->ShowWindow(FALSE);
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}


// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
void CRegionSettingDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CRegionSettingDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// ch:按钮使能 | en:Enable control


BOOL CRegionSettingDlg::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN&&pMsg->wParam == VK_ESCAPE)
    {
        return TRUE;
    }

    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
    {
        return TRUE;
    }

    return CDialog::PreTranslateMessage(pMsg);
}

void CRegionSettingDlg::InitParameter()
{
	if (!m_pcMyCamera)
	{
		return;
	}

	GetDlgItem(IDC_SET_REGION_STATIC)->SetWindowText(m_strRegion);

	bool bValue = false;
	m_pcMyCamera->GetBoolValue("TempRegionEnable", &bValue);
	m_ctrlRegionEnableCheck.SetCheck(bValue);

	if (bValue)
	{
		int nRet = SetEnumIntoDevice("RegionDisplaySelector", m_strRegion);
		if (MV_OK == nRet)
		{
			m_pcMyCamera->GetBoolValue("RegionDisplayMaxTempEnable", &bValue);
			m_ctrlMaxCheck.SetCheck(bValue);

			m_pcMyCamera->GetBoolValue("RegionDisplayMinTempEnable", &bValue);
			m_ctrlMinCheck.SetCheck(bValue);

			m_pcMyCamera->GetBoolValue("RegionDisplayAvgTempEnable", &bValue);
			m_ctrlAvgCheck.SetCheck(bValue);
		}
	}



	if (m_bExportMode)
	{
		m_pcMyCamera->GetBoolValue("TempRegionReflectEnable", &bValue);
		m_ctrlReflectEnableCheck.SetCheck(bValue);

		MVCC_FLOATVALUE oFloatValue = { 0 };
		m_pcMyCamera->GetFloatValue("TempRegionReflectance", &oFloatValue);
		m_fReflectance = oFloatValue.fCurValue;

		m_pcMyCamera->GetFloatValue("TempRegionTargetDistance", &oFloatValue);
		m_fTargetDistance = oFloatValue.fCurValue;

		m_pcMyCamera->GetFloatValue("TempRegionEmissivity", &oFloatValue);
		m_fEmissivity = oFloatValue.fCurValue;
	}


	MVCC_INTVALUE_EX oIntValue = { 0 };
	m_pcMyCamera->GetIntValue("TempRegionPointNum", &oIntValue);
	m_nPointNum = oIntValue.nCurValue;

	m_pcMyCamera->GetIntValue("TempRegionPointPositionX", &oIntValue);
	m_nPointX = oIntValue.nCurValue;

	m_pcMyCamera->GetIntValue("TempRegionPointPositionY", &oIntValue);
	m_nPointY = oIntValue.nCurValue;

	m_pcMyCamera->GetIntValue("TempRegionCenterPointPositionX", &oIntValue);
	m_nCenterX = oIntValue.nCurValue;

	m_pcMyCamera->GetIntValue("TempRegionCenterPointPositionY", &oIntValue);
	m_nCenterY = oIntValue.nCurValue;

	m_pcMyCamera->GetIntValue("TempRegionRadius", &oIntValue);
	m_nRadius = oIntValue.nCurValue;

	ReadEnumIntoCombo("TempRegionPointSelector", m_ctrlPointIndexCombo);

	switch (m_RegionType)
	{
	case CRegionSettingDlg::Region_Point:
		m_ctrlPointIndexCombo.EnableWindow(false);
		GetDlgItem(IDC_SET_POINTNUM_EDIT)->EnableWindow(false);
		GetDlgItem(IDC_SET_CENTERX_EDIT)->EnableWindow(false);
		GetDlgItem(IDC_SET_CENTERY_EDIT)->EnableWindow(false);
		GetDlgItem(IDC_SET_RADIUS_EDIT)->EnableWindow(false);
		break;
	case CRegionSettingDlg::Region_Polygon:
		GetDlgItem(IDC_SET_CENTERX_EDIT)->EnableWindow(false);
		GetDlgItem(IDC_SET_CENTERY_EDIT)->EnableWindow(false);
		GetDlgItem(IDC_SET_RADIUS_EDIT)->EnableWindow(false);
		break;
	case CRegionSettingDlg::Region_Line:
		GetDlgItem(IDC_SET_POINTNUM_EDIT)->EnableWindow(false);
		GetDlgItem(IDC_SET_CENTERX_EDIT)->EnableWindow(false);
		GetDlgItem(IDC_SET_CENTERY_EDIT)->EnableWindow(false);
		GetDlgItem(IDC_SET_RADIUS_EDIT)->EnableWindow(false);
		break;
	case CRegionSettingDlg::Region_Circle:
		m_ctrlPointIndexCombo.EnableWindow(false);
		GetDlgItem(IDC_SET_POINTNUM_EDIT)->EnableWindow(false);
		GetDlgItem(IDC_SET_POINTX_EDIT)->EnableWindow(false);
		GetDlgItem(IDC_SET_POINTY_EDIT)->EnableWindow(false);
		break;
	default:
		break;
	}


	UpdateData(FALSE);
}

int CRegionSettingDlg::ReadEnumIntoCombo(const char* strKey, CComboBox &ctrlComboBox)
{
	MVCC_ENUMVALUE stEnumValue = { 0 };
	MVCC_ENUMENTRY stEnumInfo = { 0 };

	int nRet = m_pcMyCamera->GetEnumValue(strKey, &stEnumValue);
	if (nRet != MV_OK)
	{
		return nRet;
	}


	ctrlComboBox.ResetContent();

	int nIndex = -1;
	for (unsigned int i = 0; i < stEnumValue.nSupportedNum; ++i)
	{
		stEnumInfo.nValue = stEnumValue.nSupportValue[i];
		nRet = m_pcMyCamera->GetEnumEntrySymbolic(strKey, &stEnumInfo);
		if (nRet == MV_OK)
		{
			ctrlComboBox.AddString((CString)stEnumInfo.chSymbolic);
		}

		if (stEnumInfo.nValue == stEnumValue.nCurValue)
		{
			nIndex = ctrlComboBox.GetCount() - 1;
		}
	}

	if (nIndex >= 0)
	{
		ctrlComboBox.SetCurSel(nIndex);
	}

	return MV_OK;
}

int CRegionSettingDlg::SetEnumIntoDevice(const char* strKey, const CString &strValue)
{
	LPCWSTR _lpw = NULL;
	int _convert = 0;
	UINT _acp = ATL::_AtlGetConversionACP();
	return m_pcMyCamera->SetEnumValueByString(strKey, W2A(strValue));
}

void CRegionSettingDlg::OnBnClickedSaveParameter()
{
	UpdateData(TRUE);

	bool bRegionEnable = m_ctrlRegionEnableCheck.GetCheck() > 0;
	int nRet = m_pcMyCamera->SetBoolValue("TempRegionEnable", bRegionEnable);

	if (MV_OK == nRet && bRegionEnable)
	{
		nRet = SetEnumIntoDevice("RegionDisplaySelector", m_strRegion);
		if (MV_OK != nRet)
		{
			ShowErrorMsg(TEXT("Set RegionDisplaySelector Fail"), nRet);
			return;
		}

		m_pcMyCamera->SetBoolValue("RegionDisplayEnable", true);
		m_pcMyCamera->SetBoolValue("RegionDisplayMaxTempEnable", m_ctrlMaxCheck.GetCheck() > 0);
		m_pcMyCamera->SetBoolValue("RegionDisplayMinTempEnable", m_ctrlMinCheck.GetCheck() > 0);
		m_pcMyCamera->SetBoolValue("RegionDisplayAvgTempEnable", m_ctrlAvgCheck.GetCheck() > 0);
	}
	else if (MV_OK != nRet)
	{
		ShowErrorMsg(TEXT("Set TempRegionEnable Fail"), nRet);
		return;
	}

	if (m_bExportMode)
	{
		nRet = m_pcMyCamera->SetBoolValue("TempRegionReflectEnable", m_ctrlReflectEnableCheck.GetCheck() > 0);
		if (MV_OK != nRet)
		{
			ShowErrorMsg(TEXT("Set TempRegionReflectEnable Fail"), nRet);
		}

		nRet = m_pcMyCamera->SetFloatValue("TempRegionReflectance", m_fReflectance);
		if (MV_OK != nRet)
		{
			ShowErrorMsg(TEXT("Set TempRegionReflectance Fail"), nRet);
		}

		nRet = m_pcMyCamera->SetFloatValue("TempRegionEmissivity", m_fEmissivity);
		if (MV_OK != nRet)
		{
			ShowErrorMsg(TEXT("Set TempRegionEmissivity Fail"), nRet);
		}

		nRet = m_pcMyCamera->SetFloatValue("TempRegionTargetDistance", m_fTargetDistance);
		if (MV_OK != nRet)
		{
			ShowErrorMsg(TEXT("Set TempRegionTargetDistance Fail"), nRet);
		}
	}


	nRet = m_pcMyCamera->CommandExecute("TempControlLoad");
	if (nRet != MV_OK)
	{
		ShowErrorMsg(TEXT("Exec TempControlLoad Fail"), nRet);
	}
}

void CRegionSettingDlg::OnBnClickedPointInfo()
{
	int64_t nPrePotNum = m_nPointNum;
	UpdateData(TRUE);

	int nRet = MV_OK;
	if (nPrePotNum != m_nPointNum)
	{
		nRet = m_pcMyCamera->SetIntValue("TempRegionPointNum", m_nPointNum);
		if (nRet != MV_OK)
		{
			ShowErrorMsg(TEXT("Set TempRegionPointNum Fail"), nRet);
			return;
		}

		nRet = m_pcMyCamera->CommandExecute("TempControlLoad");
		if (nRet != MV_OK)
		{
			ShowErrorMsg(TEXT("Exec TempControlLoad Fail"), nRet);
			return;
		}

		ReadEnumIntoCombo("TempRegionPointSelector", m_ctrlPointIndexCombo);

		MVCC_INTVALUE_EX oIntValue = { 0 };
		m_pcMyCamera->GetIntValue("TempRegionPointPositionX", &oIntValue);
		m_nPointX = oIntValue.nCurValue;

		m_pcMyCamera->GetIntValue("TempRegionPointPositionY", &oIntValue);
		m_nPointY = oIntValue.nCurValue;

		UpdateData(FALSE);

		return;
	}


	switch (m_RegionType)
	{
	case CRegionSettingDlg::Region_Point:
	case CRegionSettingDlg::Region_Polygon:
	case CRegionSettingDlg::Region_Line:
	{
		nRet = m_pcMyCamera->SetIntValue("TempRegionPointPositionX", m_nPointX);
		if (nRet != MV_OK)
		{
			ShowErrorMsg(TEXT("Set TempRegionPointPositionX Fail"), nRet);
		}

		nRet = m_pcMyCamera->SetIntValue("TempRegionPointPositionY", m_nPointY);
		if (nRet != MV_OK)
		{
			ShowErrorMsg(TEXT("Set TempRegionPointPositionY Fail"), nRet);
		}

		break;
	}
	case CRegionSettingDlg::Region_Circle:
	{
		nRet = m_pcMyCamera->SetIntValue("TempRegionCenterPointPositionX", m_nCenterX);
		if (nRet != MV_OK)
		{
			ShowErrorMsg(TEXT("Set TempRegionCenterPointPositionX Fail"), nRet);
		}

		nRet = m_pcMyCamera->SetIntValue("TempRegionCenterPointPositionY", m_nCenterY);
		if (nRet != MV_OK)
		{
			ShowErrorMsg(TEXT("Set TempRegionCenterPointPositionY Fail"), nRet);
		}

		nRet = m_pcMyCamera->SetIntValue("TempRegionRadius", m_nRadius);
		if (nRet != MV_OK)
		{
			ShowErrorMsg(TEXT("Set TempRegionRadius Fail"), nRet);
		}

		break;
	}
	default:
		break;
	}

	nRet = m_pcMyCamera->CommandExecute("TempControlLoad");
	if (nRet != MV_OK)
	{
		ShowErrorMsg(TEXT("Exec TempControlLoad Fail"), nRet);
	}
}

void CRegionSettingDlg::OnPointIndexChanged()
{
	int nSel = m_ctrlPointIndexCombo.GetCurSel();
	m_ctrlPointIndexCombo.SetCurSel(nSel);
	CString strText;
	m_ctrlPointIndexCombo.GetWindowText(strText);
	int nRet = SetEnumIntoDevice("TempRegionPointSelector", strText);
	if (nRet != MV_OK)
	{
		ShowErrorMsg(TEXT("Set TempRegionPointSelector Fail"), nRet);
	}

	MVCC_INTVALUE_EX oIntValue = { 0 };
	m_pcMyCamera->GetIntValue("TempRegionPointPositionX", &oIntValue);
	m_nPointX = oIntValue.nCurValue;

	m_pcMyCamera->GetIntValue("TempRegionPointPositionY", &oIntValue);
	m_nPointY = oIntValue.nCurValue;
	UpdateData(FALSE);
}

void CRegionSettingDlg::ShowErrorMsg(CString csMessage, int nErrorNum)
{
	CString errorMsg;
	if (nErrorNum == 0)
	{
		errorMsg.Format(_T("%s"), csMessage);
	}
	else
	{
		errorMsg.Format(_T("%s: Error = %x: "), csMessage, nErrorNum);
	}

	switch (nErrorNum)
	{
	case MV_E_HANDLE:           errorMsg += "Error or invalid handle ";                                         break;
	case MV_E_SUPPORT:          errorMsg += "Not supported function ";                                          break;
	case MV_E_BUFOVER:          errorMsg += "Cache is full ";                                                   break;
	case MV_E_CALLORDER:        errorMsg += "Function calling order error ";                                    break;
	case MV_E_PARAMETER:        errorMsg += "Incorrect parameter ";                                             break;
	case MV_E_RESOURCE:         errorMsg += "Applying resource failed ";                                        break;
	case MV_E_NODATA:           errorMsg += "No data ";                                                         break;
	case MV_E_PRECONDITION:     errorMsg += "Precondition error, or running environment changed ";              break;
	case MV_E_VERSION:          errorMsg += "Version mismatches ";                                              break;
	case MV_E_NOENOUGH_BUF:     errorMsg += "Insufficient memory ";                                             break;
	case MV_E_ABNORMAL_IMAGE:   errorMsg += "Abnormal image, maybe incomplete image because of lost packet ";   break;
	case MV_E_UNKNOW:           errorMsg += "Unknown error ";                                                   break;
	case MV_E_GC_GENERIC:       errorMsg += "General error ";                                                   break;
	case MV_E_GC_ACCESS:        errorMsg += "Node accessing condition error ";                                  break;
	case MV_E_ACCESS_DENIED:	errorMsg += "No permission ";                                                   break;
	case MV_E_BUSY:             errorMsg += "Device is busy, or network disconnected ";                         break;
	case MV_E_NETER:            errorMsg += "Network error ";                                                   break;
	}

	MessageBox(errorMsg, TEXT("PROMPT"), MB_OK | MB_ICONWARNING);
}

