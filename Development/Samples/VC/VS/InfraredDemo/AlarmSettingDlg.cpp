
// InfraredDemoDlg.cpp : implementation file
#include "stdafx.h"
#include "AlarmSettingDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CInfraredDemoDlg dialog
CAlarmSettingDlg::CAlarmSettingDlg(CMvCamera *pcMyCamera, const CString &strRegion, int nRegionIndex, CWnd* pParent /*=NULL*/)
	: CDialog(CAlarmSettingDlg::IDD, pParent)
	, m_pcMyCamera(pcMyCamera)
	, m_strRegion(strRegion)
	, m_nRegionIndex(nRegionIndex)
	, m_fReferenceValue(0)
	, m_fABSValue(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAlarmSettingDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ALARM_ENBLE_CHECK, m_ctrlAlarmEnableCheck);
	DDX_Control(pDX, IDC_ALARM_SOURCE_COMBO, m_ctrlSourceCombo);
	DDX_Control(pDX, IDC_ALARM_CONDITION_COMBO, m_ctrlConditionCombo);
	DDX_Text(pDX, IDC_ALRAM_REFERENCE_EDIT, m_fReferenceValue);
	DDX_Text(pDX, IDC_ALARM_ABS_EDIT, m_fABSValue);
}

BEGIN_MESSAGE_MAP(CAlarmSettingDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()


    ON_WM_CLOSE()
END_MESSAGE_MAP()


// CInfraredDemoDlg message handlers
BOOL CAlarmSettingDlg::OnInitDialog()
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


	return TRUE;  // return TRUE  unless you set the focus to a control
}


// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
void CAlarmSettingDlg::OnPaint()
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
HCURSOR CAlarmSettingDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// ch:按钮使能 | en:Enable control


BOOL CAlarmSettingDlg::PreTranslateMessage(MSG* pMsg)
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

void CAlarmSettingDlg::InitParameter()
{
	if (!m_pcMyCamera)
	{
		return;
	}

	int nRet = m_pcMyCamera->SetEnumValue("TempRegionAlarmRuleSelector", m_nRegionIndex);
	if (MV_OK != nRet)
	{
		ShowErrorMsg(TEXT("Read TempRegionAlarmRuleSelector Fail"), nRet);
		return;
	}

	bool bValue = false;
	m_pcMyCamera->GetBoolValue("TempRegionAlarmRuleEnable", &bValue);
	m_ctrlAlarmEnableCheck.SetCheck(bValue);

	MV_XML_AccessMode oAccessMode = AM_NA;
	m_pcMyCamera->GetNodeAccessMode("TempRegionAlarmRuleEnable", &oAccessMode);
	if (oAccessMode != AM_RW)
	{
		m_ctrlAlarmEnableCheck.EnableWindow(FALSE);
	}

	MVCC_FLOATVALUE oFloatValue = { 0 };
	m_pcMyCamera->GetFloatValue("TempRegionAlarmReferenceValue", &oFloatValue);
	m_fReferenceValue = oFloatValue.fCurValue;

	m_pcMyCamera->GetFloatValue("TempRegionAlarmRecoveryABSValue", &oFloatValue);
	m_fABSValue = oFloatValue.fCurValue;

	ReadEnumIntoCombo("TempRegionAlarmRuleSource", m_ctrlSourceCombo);
	ReadEnumIntoCombo("TempRegionAlarmRuleCondition", m_ctrlConditionCombo);

	UpdateData(FALSE);
}

int CAlarmSettingDlg::ReadEnumIntoCombo(const char* strKey, CComboBox &ctrlComboBox)
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

int CAlarmSettingDlg::SetEnumIntoDevice(const char* strKey, CComboBox &ctrlComboBox)
{
	CString strText;
	ctrlComboBox.GetWindowText(strText);
	LPCWSTR _lpw = NULL;
	int _convert = 0;
	UINT _acp = ATL::_AtlGetConversionACP();
	return m_pcMyCamera->SetEnumValueByString(strKey, W2A(strText));
}

void CAlarmSettingDlg::ShowErrorMsg(CString csMessage, int nErrorNum)
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

void CAlarmSettingDlg::OnOK()
{
	UpdateData(TRUE);

	int nRet = MV_OK;
	if (m_ctrlAlarmEnableCheck.IsWindowEnabled())
	{
		nRet = m_pcMyCamera->SetBoolValue("TempRegionAlarmRuleEnable", m_ctrlAlarmEnableCheck.GetCheck() > 0);
		if (MV_OK != nRet)
		{
			ShowErrorMsg(TEXT("Set TempRegionAlarmRuleEnable Fail"), nRet);
		}

		if (m_ctrlAlarmEnableCheck.GetCheck())
		{
			m_pcMyCamera->SetBoolValue("RegionDisplayAlarmEnable", TRUE);
		}
	}

	nRet = SetEnumIntoDevice("TempRegionAlarmRuleSource", m_ctrlSourceCombo);
	if (MV_OK != nRet)
	{
		ShowErrorMsg(TEXT("Set TempRegionAlarmRuleSource Fail"), nRet);
	}

	nRet = SetEnumIntoDevice("TempRegionAlarmRuleCondition", m_ctrlConditionCombo);
	if (MV_OK != nRet)
	{
		ShowErrorMsg(TEXT("Set TempRegionAlarmRuleCondition Fail"), nRet);
	}

	nRet = m_pcMyCamera->SetFloatValue("TempRegionAlarmReferenceValue", m_fReferenceValue);
	if (MV_OK != nRet)
	{
		ShowErrorMsg(TEXT("Set TempRegionAlarmReferenceValue Fail"), nRet);
	}

	nRet = m_pcMyCamera->SetFloatValue("TempRegionAlarmRecoveryABSValue", m_fABSValue);
	if (MV_OK != nRet)
	{
		ShowErrorMsg(TEXT("Set TempRegionAlarmRecoveryABSValue Fail"), nRet);
	}

	nRet = m_pcMyCamera->CommandExecute("TempControlLoad");
	if (nRet != MV_OK)
	{
		ShowErrorMsg(TEXT("Exec TempControlLoad Fail"), nRet);
	}
	
	CDialog::OnOK();
}

