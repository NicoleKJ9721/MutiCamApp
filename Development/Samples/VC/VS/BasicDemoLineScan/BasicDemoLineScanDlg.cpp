
// BasicDemoDlg.cpp : implementation file
#include "stdafx.h"
#include "BasicDemoLineScan.h"
#include "BasicDemoLineScanDlg.h"


#include <conio.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CAboutDlg dialog used for App About
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// CBasicDemoDlg dialog
CBasicDemoDlg::CBasicDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBasicDemoDlg::IDD, pParent)
    , m_pcMyCamera(NULL)
    , m_nDeviceCombo(0)
    , m_nTriggerSelector(0)
    , m_bOpenDevice(FALSE)
    , m_bStartGrabbing(FALSE)
    , m_hGrabThread(NULL)
    , m_bThreadState(FALSE)
    , m_bHBMode(FALSE)
    , m_bPreampGain(FALSE)
    , m_bAcquisitionLineRate(FALSE)
    , m_nTriggerMode(MV_TRIGGER_MODE_OFF)
    , m_dExposureEdit(0)
    , m_dDigitalShiftGainEdit(0)
    , m_nAcquisitionLineRateEdit(0)
    , m_nResultingLineRateEdit(0)
    , m_nPreampGain(0)
    , m_nImageCompressionMode(0)
    , m_bTriggerModeCheck(FALSE)
    , m_nTriggerSource(MV_TRIGGER_SOURCE_SOFTWARE)
    , m_nPixelFormat(PixelType_Gvsp_Mono8)
    , m_pSaveImageBuf(NULL)
    , m_nHBDecodeBufSize(0)
    , m_pHBDecodeBuf(NULL)
    , m_nSaveImageBufSize(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    memset(&m_stImageInfo, 0, sizeof(MV_FRAME_OUT_INFO_EX));

    m_mapPixelFormat.clear();
    m_mapPreampGain.clear();
	m_mapTriggerSource.clear();
}

void CBasicDemoDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     DDX_Control(pDX, IDC_DEVICE_COMBO, m_ctrlDeviceCombo);
     DDX_CBIndex(pDX, IDC_DEVICE_COMBO, m_nDeviceCombo);
     DDX_Control(pDX, IDC_TRIGGERSEL_COMBO, m_ctrlTriggerSelectorCombo);
     DDX_CBIndex(pDX, IDC_TRIGGERSEL_COMBO, m_nTriggerSelector);
     DDX_Control(pDX, IDC_TRIGGERSWITCH_COMBO, m_ctrlTriggerModeCombo);
     DDX_CBIndex(pDX, IDC_TRIGGERSWITCH_COMBO, m_nTriggerMode);
     DDX_Control(pDX, IDC_TRIGGERSOURCE_COMBO, m_ctrlTriggerSourceCombo);
     DDX_CBIndex(pDX, IDC_TRIGGERSOURCE_COMBO, m_nTriggerSource);
     DDX_Control(pDX, IDC_PIXELFORMAT_COMBO, m_ctrlPixelFormatCombo);
     DDX_CBIndex(pDX, IDC_PIXELFORMAT_COMBO, m_nPixelFormat); 
     DDX_Control(pDX, IDC_PREAMPGAIN_COMBO, m_ctrlPreampGainCombo); 
     DDX_CBIndex(pDX, IDC_PREAMPGAIN_COMBO, m_nPreampGain);
     DDX_Control(pDX, IDC_IMAGE_COMPRESSION_MODE_COMBO, m_ctrlImageCompressionModeCombo);
     DDX_CBIndex(pDX, IDC_IMAGE_COMPRESSION_MODE_COMBO, m_nImageCompressionMode);
     DDX_Text(pDX, IDC_EXPOSUACRE_EDIT, m_dExposureEdit);
     DDX_Text(pDX, IDC_DIGITAL_SHIFT_EDIT, m_dDigitalShiftGainEdit);
     DDX_Text(pDX, IDC_ACQUISITION_LINE_RATE_EDIT, m_nAcquisitionLineRateEdit); 
     DDX_Text(pDX, IDC_RESULTING_LINE_RATE_EDIT, m_nResultingLineRateEdit);
}

BEGIN_MESSAGE_MAP(CBasicDemoDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	// }}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_ENUM_BUTTON, &CBasicDemoDlg::OnBnClickedEnumButton)
    ON_BN_CLICKED(IDC_OPEN_BUTTON, &CBasicDemoDlg::OnBnClickedOpenButton)
    ON_BN_CLICKED(IDC_CLOSE_BUTTON, &CBasicDemoDlg::OnBnClickedCloseButton)
    ON_BN_CLICKED(IDC_START_GRABBING_BUTTON, &CBasicDemoDlg::OnBnClickedStartGrabbingButton)
    ON_BN_CLICKED(IDC_STOP_GRABBING_BUTTON, &CBasicDemoDlg::OnBnClickedStopGrabbingButton)
    ON_BN_CLICKED(IDC_GET_PARAMETER_BUTTON, &CBasicDemoDlg::OnBnClickedGetParameterButton)
    ON_BN_CLICKED(IDC_SET_PARAMETER_BUTTON, &CBasicDemoDlg::OnBnClickedSetParameterButton)
    ON_BN_CLICKED(IDC_SOFTWARE_ONCE_BUTTON, &CBasicDemoDlg::OnBnClickedSoftwareOnceButton)
    ON_BN_CLICKED(IDC_SAVE_BMP_BUTTON, &CBasicDemoDlg::OnBnClickedSaveBmpButton)
    ON_BN_CLICKED(IDC_SAVE_JPG_BUTTON, &CBasicDemoDlg::OnBnClickedSaveJpgButton)
    ON_BN_CLICKED(IDC_SAVE_TIFF_BUTTON, &CBasicDemoDlg::OnBnClickedSaveTiffButton)
    ON_BN_CLICKED(IDC_SAVE_PNG_BUTTON, &CBasicDemoDlg::OnBnClickedSavePngButton)
    ON_WM_CLOSE()
    ON_CBN_SELCHANGE(IDC_TRIGGERSEL_COMBO, &CBasicDemoDlg::OnCbnSelchangeTriggerselCombo)
    ON_CBN_SELCHANGE(IDC_TRIGGERSWITCH_COMBO, &CBasicDemoDlg::OnCbnSelchangeTriggerswitchCombo)
    ON_CBN_SELCHANGE(IDC_TRIGGERSOURCE_COMBO, &CBasicDemoDlg::OnCbnSelchangeTriggersourceCombo)
    ON_CBN_SELCHANGE(IDC_PIXELFORMAT_COMBO, &CBasicDemoDlg::OnCbnSelchangePixelformatCombo)
    ON_CBN_SELCHANGE(IDC_PREAMPGAIN_COMBO, &CBasicDemoDlg::OnCbnSelchangePreampgainCombo)
    ON_BN_CLICKED(IDC_ACQUISITION_LINE_RATE_ENABLE_CHECK, &CBasicDemoDlg::OnBnClickedAcquisitionLineRateEnableCheck)
    ON_CBN_SELCHANGE(IDC_IMAGE_COMPRESSION_MODE_COMBO, &CBasicDemoDlg::OnCbnSelchangeImageCompressionModeCombo)
END_MESSAGE_MAP()

// ch:取流线程 | en:Grabbing thread
unsigned int __stdcall GrabThread(void* pUser)
{
    if (pUser)
    {
        CBasicDemoDlg* pCam = (CBasicDemoDlg*)pUser;

        pCam->GrabThreadProcess();
        
        return 0;
    }

    return -1;
}

// CBasicDemoDlg message handlers
BOOL CBasicDemoDlg::OnInitDialog()
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

	CMvCamera::InitSDK();
	DisplayWindowInitial();             // ch:显示框初始化 | en:Display Window Initialization

    InitializeCriticalSection(&m_hSaveImageMux);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CBasicDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
void CBasicDemoDlg::OnPaint()
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
HCURSOR CBasicDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// ch:按钮使能 | en:Enable control
void CBasicDemoDlg::EnableControls(BOOL bIsCameraReady)
{
    GetDlgItem(IDC_OPEN_BUTTON)->EnableWindow(m_bOpenDevice ? FALSE : (bIsCameraReady ? TRUE : FALSE));
    GetDlgItem(IDC_CLOSE_BUTTON)->EnableWindow((m_bOpenDevice && bIsCameraReady) ? TRUE : FALSE);
    GetDlgItem(IDC_START_GRABBING_BUTTON)->EnableWindow((m_bStartGrabbing && bIsCameraReady) ? FALSE : (m_bOpenDevice ? TRUE : FALSE));
    GetDlgItem(IDC_STOP_GRABBING_BUTTON)->EnableWindow(m_bStartGrabbing ? TRUE : FALSE);
    GetDlgItem(IDC_SOFTWARE_ONCE_BUTTON)->EnableWindow((m_bStartGrabbing && m_bTriggerModeCheck) ? TRUE : FALSE);
    GetDlgItem(IDC_SAVE_BMP_BUTTON)->EnableWindow(m_bStartGrabbing ? TRUE : FALSE);
    GetDlgItem(IDC_SAVE_TIFF_BUTTON)->EnableWindow(m_bStartGrabbing ? TRUE : FALSE);
    GetDlgItem(IDC_SAVE_PNG_BUTTON)->EnableWindow(m_bStartGrabbing ? TRUE : FALSE);
    GetDlgItem(IDC_SAVE_JPG_BUTTON)->EnableWindow(m_bStartGrabbing ? TRUE : FALSE);
    GetDlgItem(IDC_EXPOSUACRE_EDIT)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);
    GetDlgItem(IDC_DIGITAL_SHIFT_EDIT)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);
    GetDlgItem(IDC_ACQUISITION_LINE_RATE_EDIT)->EnableWindow((m_bOpenDevice && m_bAcquisitionLineRate) ? TRUE : FALSE);
    GetDlgItem(IDC_RESULTING_LINE_RATE_EDIT)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);
    GetDlgItem(IDC_TRIGGERSEL_COMBO)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);
    GetDlgItem(IDC_TRIGGERSWITCH_COMBO)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);
    GetDlgItem(IDC_GET_PARAMETER_BUTTON)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);
    GetDlgItem(IDC_SET_PARAMETER_BUTTON)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);
    GetDlgItem(IDC_TRIGGERSOURCE_COMBO)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);
    GetDlgItem(IDC_PIXELFORMAT_COMBO)->EnableWindow((m_bOpenDevice &&!m_bStartGrabbing) ? TRUE : FALSE);
    GetDlgItem(IDC_IMAGE_COMPRESSION_MODE_COMBO)->EnableWindow((m_bOpenDevice && m_bHBMode &&!m_bStartGrabbing)? TRUE : FALSE);
    GetDlgItem(IDC_PREAMPGAIN_COMBO)->EnableWindow((m_bOpenDevice && m_bPreampGain) ? TRUE : FALSE);
    GetDlgItem(IDC_ACQUISITION_LINE_RATE_ENABLE_CHECK)->EnableWindow((m_bOpenDevice &&m_bAcquisitionLineRate)? TRUE : FALSE);
    GetDlgItem(IDC_RESULTING_LINE_RATE_EDIT)->EnableWindow(FALSE);

    if (!m_bOpenDevice)
    {
        ((CButton *)GetDlgItem(IDC_ACQUISITION_LINE_RATE_ENABLE_CHECK))->SetCheck(FALSE);
        m_ctrlTriggerSelectorCombo.ResetContent();
        m_ctrlTriggerModeCombo.ResetContent();
        m_ctrlTriggerSourceCombo.ResetContent();
        m_ctrlPixelFormatCombo.ResetContent();
        m_ctrlImageCompressionModeCombo.ResetContent(); 
        m_ctrlPreampGainCombo.ResetContent();

		char chIn[MV_MAX_SYMBOLIC_LEN] = "0";
		wchar_t wchOut[MV_MAX_SYMBOLIC_LEN] = { 0 };
		Char2Wchar(chIn, wchOut, MV_MAX_SYMBOLIC_LEN);

		CString strMsg = _T("");

		strMsg.Format(_T("%s"), wchOut);
        SetDlgItemText(IDC_EXPOSUACRE_EDIT, strMsg);
        SetDlgItemText(IDC_DIGITAL_SHIFT_EDIT, strMsg);
        SetDlgItemText(IDC_ACQUISITION_LINE_RATE_EDIT, strMsg);
        SetDlgItemText(IDC_RESULTING_LINE_RATE_EDIT, strMsg);
    }
}

// ch:最开始时的窗口初始化 | en:Initial window initialization
void CBasicDemoDlg::DisplayWindowInitial()
{
    CWnd *pWnd = GetDlgItem(IDC_DISPLAY_STATIC);
    if (pWnd)
    {
        m_hwndDisplay = pWnd->GetSafeHwnd();
        if (m_hwndDisplay)
        {
            EnableControls(FALSE);
        }
    }
}

// ch:显示错误信息 | en:Show error message
void CBasicDemoDlg::ShowErrorMsg(CString csMessage, int nErrorNum)
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

    switch(nErrorNum)
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

// ch:关闭设备 | en:Close Device
int CBasicDemoDlg::CloseDevice()
{
	if(TRUE == m_bStartGrabbing)
	{
		int nRet = m_pcMyCamera->StopGrabbing();
		if (MV_OK != nRet)
		{
			ShowErrorMsg(TEXT("Stop grabbing fail"), nRet);
			return nRet;
		}
		 m_bStartGrabbing = FALSE;
	}

    m_bThreadState = FALSE;
    if (m_hGrabThread)
    {
        WaitForSingleObject(m_hGrabThread, INFINITE);
        CloseHandle(m_hGrabThread);
        m_hGrabThread = NULL;
    }

    if (m_pcMyCamera)
    {
        m_pcMyCamera->Close();
        delete m_pcMyCamera;
        m_pcMyCamera = NULL;
    }

    m_bStartGrabbing = FALSE;
    m_bOpenDevice = FALSE;

    if (m_pSaveImageBuf)
    {
        free(m_pSaveImageBuf);
        m_pSaveImageBuf = NULL;
    }
    m_nSaveImageBufSize = 0; 

    if (m_pHBDecodeBuf)
    {
        free(m_pHBDecodeBuf);
        m_pHBDecodeBuf = NULL;
    }
    m_nHBDecodeBufSize = 0;

    return MV_OK;
}

// ch:获取触发模式 | en:Get Trigger Mode
int CBasicDemoDlg::GetTriggerMode()
{
    MVCC_ENUMVALUE stEnumTriggerModeValue = { 0 };
    MVCC_ENUMENTRY stEnumTriggerModeEntry = { 0 };

    int nRet = m_pcMyCamera->GetEnumValue("TriggerMode", &stEnumTriggerModeValue);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    m_ctrlTriggerModeCombo.ResetContent();
    for (int i = 0; i < stEnumTriggerModeValue.nSupportedNum; i++)
    {
        memset(&stEnumTriggerModeEntry, 0, sizeof(stEnumTriggerModeEntry));
        stEnumTriggerModeEntry.nValue = stEnumTriggerModeValue.nSupportValue[i];
        m_pcMyCamera->GetEnumEntrySymbolic("TriggerMode", &stEnumTriggerModeEntry);

		char chIn[MV_MAX_SYMBOLIC_LEN] = "";
		memcpy(chIn, stEnumTriggerModeEntry.chSymbolic, MV_MAX_SYMBOLIC_LEN);
		wchar_t wchOut[MV_MAX_SYMBOLIC_LEN] = { 0 };
		Char2Wchar(chIn, wchOut, MV_MAX_SYMBOLIC_LEN);

		CString strMsg = _T("");

		strMsg.Format(_T("%s"), wchOut);

        m_ctrlTriggerModeCombo.AddString(strMsg);
    }

    for (int i = 0; i < stEnumTriggerModeValue.nSupportedNum; i++)
    {
        if (stEnumTriggerModeValue.nCurValue == stEnumTriggerModeValue.nSupportValue[i])
        {
            m_nTriggerMode = i;
            m_ctrlTriggerModeCombo.SetCurSel(m_nTriggerMode);
        }
    }

    int nIndex = m_ctrlTriggerModeCombo.GetCurSel();

    CString strCBText;

    m_ctrlTriggerModeCombo.GetLBText(nIndex, strCBText);

    return MV_OK;
}

//ch:设置无损压缩  | en:set Image Compression Mode
int CBasicDemoDlg::GetImageCompressionMode()
{
    MVCC_ENUMVALUE stEnumImageCompressionModeValue = { 0 };
    MVCC_ENUMENTRY stEnumImageCompressionModeEntry = { 0 };

    int nRet = m_pcMyCamera->GetEnumValue("ImageCompressionMode", &stEnumImageCompressionModeValue);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    m_ctrlImageCompressionModeCombo.ResetContent();
    for (int i = 0; i < stEnumImageCompressionModeValue.nSupportedNum; i++)
    {
        memset(&stEnumImageCompressionModeEntry, 0, sizeof(stEnumImageCompressionModeEntry));
        stEnumImageCompressionModeEntry.nValue = stEnumImageCompressionModeValue.nSupportValue[i];
        m_pcMyCamera->GetEnumEntrySymbolic("ImageCompressionMode", &stEnumImageCompressionModeEntry);

		char chIn[MV_MAX_SYMBOLIC_LEN] = "";
		memcpy(chIn, stEnumImageCompressionModeEntry.chSymbolic, MV_MAX_SYMBOLIC_LEN);
		wchar_t wchOut[MV_MAX_SYMBOLIC_LEN] = { 0 };
		Char2Wchar(chIn, wchOut, MV_MAX_SYMBOLIC_LEN);

		CString strMsg = _T("");

		strMsg.Format(_T("%s"), wchOut);

        m_ctrlImageCompressionModeCombo.AddString(strMsg);
    }

    for (int i = 0; i < stEnumImageCompressionModeValue.nSupportedNum; i++)
    {
        if (stEnumImageCompressionModeValue.nCurValue == stEnumImageCompressionModeValue.nSupportValue[i])
        {
            m_nImageCompressionMode = i;
            m_ctrlImageCompressionModeCombo.SetCurSel(m_nImageCompressionMode);
        }
    }

    m_bHBMode = TRUE;

    return MV_OK;
}

//ch:设置触发选项  | en:set Trigger Selector
int CBasicDemoDlg::GetTriggerSelector()
{
    MVCC_ENUMVALUE stEnumTriggerSelectorValue = { 0 };
    MVCC_ENUMENTRY stEnumTriggerSelectorEntry = { 0 };

    int nRet = m_pcMyCamera->GetEnumValue("TriggerSelector", &stEnumTriggerSelectorValue);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    m_ctrlTriggerSelectorCombo.ResetContent();
    for (int i = 0; i < stEnumTriggerSelectorValue.nSupportedNum; i++)
    {
        memset(&stEnumTriggerSelectorEntry, 0, sizeof(stEnumTriggerSelectorEntry));
        stEnumTriggerSelectorEntry.nValue = stEnumTriggerSelectorValue.nSupportValue[i];
        m_pcMyCamera->GetEnumEntrySymbolic("TriggerSelector", &stEnumTriggerSelectorEntry);

		char chIn[MV_MAX_SYMBOLIC_LEN] = "";
		memcpy(chIn, stEnumTriggerSelectorEntry.chSymbolic, MV_MAX_SYMBOLIC_LEN);
		wchar_t wchOut[MV_MAX_SYMBOLIC_LEN] = { 0 };
		Char2Wchar(chIn, wchOut, MV_MAX_SYMBOLIC_LEN);

		CString strMsg = _T("");

		strMsg.Format(_T("%s"), wchOut);

        m_ctrlTriggerSelectorCombo.AddString(strMsg);

    }

    for (int i = 0; i < stEnumTriggerSelectorValue.nSupportedNum; i++)
    {
        if (stEnumTriggerSelectorValue.nCurValue == stEnumTriggerSelectorValue.nSupportValue[i])
        {
            m_nTriggerSelector = i;
            m_ctrlTriggerSelectorCombo.SetCurSel(m_nTriggerSelector);
        }
    }

    return MV_OK;
}

int CBasicDemoDlg::GetTriggerSource()
{
    MVCC_ENUMVALUE stEnumTriggerSourceValue = { 0 };
    MVCC_ENUMENTRY stEnumTriggerSourceEntry = { 0 };

    int nRet = m_pcMyCamera->GetEnumValue("TriggerSource", &stEnumTriggerSourceValue);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    m_ctrlTriggerSourceCombo.ResetContent();
    for (int i = 0; i < stEnumTriggerSourceValue.nSupportedNum; i++)
    {
        memset(&stEnumTriggerSourceEntry, 0, sizeof(stEnumTriggerSourceEntry));
        stEnumTriggerSourceEntry.nValue = stEnumTriggerSourceValue.nSupportValue[i];
        m_pcMyCamera->GetEnumEntrySymbolic("TriggerSource", &stEnumTriggerSourceEntry);

		char chIn[MV_MAX_SYMBOLIC_LEN] = "";
		memcpy(chIn, stEnumTriggerSourceEntry.chSymbolic, MV_MAX_SYMBOLIC_LEN);
		wchar_t wchOut[MV_MAX_SYMBOLIC_LEN] = { 0 };
		Char2Wchar(chIn, wchOut, MV_MAX_SYMBOLIC_LEN);

		CString strMsg = _T("");

		strMsg.Format(_T("%s"), wchOut);

        m_ctrlTriggerSourceCombo.AddString(strMsg);

		m_mapTriggerSource.insert(pair<CString, int>(strMsg, stEnumTriggerSourceEntry.nValue));
    }

    for (int i = 0; i < stEnumTriggerSourceValue.nSupportedNum; i++)
    {
        if (stEnumTriggerSourceValue.nCurValue == stEnumTriggerSourceValue.nSupportValue[i])
        {
            m_nTriggerSource = i;
            m_ctrlTriggerSourceCombo.SetCurSel(m_nTriggerSource);
        }
    }

    CString strCBText;
    int nIndex = m_ctrlTriggerSourceCombo.GetCurSel();
    m_ctrlTriggerSourceCombo.GetLBText(nIndex, strCBText);
    CString strTriggerSource = strCBText;

    nIndex = m_ctrlTriggerSelectorCombo.GetCurSel();
    m_ctrlTriggerSelectorCombo.GetLBText(nIndex, strCBText);
    CString cStrTriggerSelector = strCBText;

    nIndex = m_ctrlTriggerModeCombo.GetCurSel();
    m_ctrlTriggerModeCombo.GetLBText(nIndex, strCBText);
    CString cStrTriggerMode = strCBText;
    if (STR_FRAMEBURSTSTART == cStrTriggerSelector &&cStrTriggerMode == "On" && STR_SOFTWARE == strTriggerSource)
    {
        m_bTriggerModeCheck = TRUE;
    }

    EnableControls(TRUE);
    return MV_OK;
}

// ch:获取曝光时间 | en:Get Exposure Time
int CBasicDemoDlg::GetExposureTime()
{
    MVCC_FLOATVALUE stFloatValue = {0};

    int nRet = m_pcMyCamera->GetFloatValue("ExposureTime", &stFloatValue);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    m_dExposureEdit = stFloatValue.fCurValue;

    return MV_OK;
}

// ch:设置曝光时间 | en:Set Exposure Time
int CBasicDemoDlg::SetExposureTime()
{
    m_pcMyCamera->SetEnumValue("ExposureAuto", MV_EXPOSURE_AUTO_MODE_OFF);

    return m_pcMyCamera->SetFloatValue("ExposureTime", (float)m_dExposureEdit);
}

// ch:设置行频   | en:set Acquisition LineRate
int CBasicDemoDlg::SetAcquisitionLineRate()
{
    return m_pcMyCamera->SetIntValue("AcquisitionLineRate", (int)m_nAcquisitionLineRateEdit);
}

// ch:获取增益 | en:Get Gain
int CBasicDemoDlg::GetDigitalShiftGain()
{
    MVCC_FLOATVALUE stFloatValue = {0};

    int nRet = m_pcMyCamera->GetFloatValue("DigitalShift", &stFloatValue);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    CString str; 
    m_dDigitalShiftGainEdit = stFloatValue.fCurValue;
    str.Format(_T("%.4lf"), m_dDigitalShiftGainEdit);
    m_dDigitalShiftGainEdit =_tstof(str);

    return MV_OK;
}

// ch:获取行频  | en:Get Acquisition LineRate
int CBasicDemoDlg::GetAcquisitionLineRate()
{
    MVCC_INTVALUE_EX stIntValue = { 0 };

    int nRet = m_pcMyCamera->GetIntValue("AcquisitionLineRate", &stIntValue);
    if (MV_OK != nRet)
    {
        return nRet;
    }
    m_nAcquisitionLineRateEdit = stIntValue.nCurValue;
    m_bAcquisitionLineRate = TRUE;

    return MV_OK;
}

int CBasicDemoDlg::GetResultingLineRate()
{
    MVCC_INTVALUE_EX stIntValue = { 0 };

    int nRet = m_pcMyCamera->GetIntValue("ResultingLineRate", &stIntValue);
    if (MV_OK != nRet)
    {
        return nRet;
    }
    m_nResultingLineRateEdit = stIntValue.nCurValue;

    return MV_OK;
}

int CBasicDemoDlg::GetAcquisitionLineRateEnable()
{
    bool bAcquisitionLineRateEnable = FALSE;
    int nRet = m_pcMyCamera->GetBoolValue("AcquisitionLineRateEnable", &bAcquisitionLineRateEnable);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    if (TRUE == bAcquisitionLineRateEnable)
    {
        ((CButton *)GetDlgItem(IDC_ACQUISITION_LINE_RATE_ENABLE_CHECK))->SetCheck(TRUE);
    }
    else
    {
        ((CButton *)GetDlgItem(IDC_ACQUISITION_LINE_RATE_ENABLE_CHECK))->SetCheck(FALSE);
    }

    return MV_OK;
}

// ch:数字增益 | en:Digital Shift
int CBasicDemoDlg::SetDigitalShiftGain()
{
    // ch:设置增益前先把增益使能开关打开，失败无需返回
    //en:Set Gain after Auto Gain is turned off, this failure does not need to return
    m_pcMyCamera->SetBoolValue("DigitalShiftEnable", TRUE);

    return m_pcMyCamera->SetFloatValue("DigitalShift", (float)m_dDigitalShiftGainEdit);
}

// ch:获取模拟增益  | en:Get PreampGain
int CBasicDemoDlg::GetPreampGain()
{
    MVCC_ENUMVALUE stEnumPreampGainValue = { 0 };
    MVCC_ENUMENTRY stEnumPreampGainEntry = { 0 };

    int nRet = m_pcMyCamera->GetEnumValue("PreampGain", &stEnumPreampGainValue);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    m_ctrlPreampGainCombo.ResetContent();
    for (int i = 0; i < stEnumPreampGainValue.nSupportedNum; i++)
    {
        memset(&stEnumPreampGainEntry, 0, sizeof(stEnumPreampGainEntry));
        stEnumPreampGainEntry.nValue = stEnumPreampGainValue.nSupportValue[i];
        m_pcMyCamera->GetEnumEntrySymbolic("PreampGain", &stEnumPreampGainEntry);

		char chIn[MV_MAX_SYMBOLIC_LEN] = "";
		memcpy(chIn, stEnumPreampGainEntry.chSymbolic, MV_MAX_SYMBOLIC_LEN);
		wchar_t wchOut[MV_MAX_SYMBOLIC_LEN] = { 0 };
		Char2Wchar(chIn, wchOut, MV_MAX_SYMBOLIC_LEN);

		CString strMsg = _T("");

		strMsg.Format(_T("%s"), wchOut);

        m_ctrlPreampGainCombo.AddString(strMsg);

        m_mapPreampGain.insert(pair<CString, int>(strMsg, stEnumPreampGainEntry.nValue));
    }

    for (int i = 0; i < stEnumPreampGainValue.nSupportedNum; i++)
    {
        if (stEnumPreampGainValue.nCurValue == stEnumPreampGainValue.nSupportValue[i])
        {
            m_nPreampGain = i;
            m_ctrlPreampGainCombo.SetCurSel(m_nPreampGain);
        }
    }

    m_bPreampGain = TRUE;

    return MV_OK;
}

int CBasicDemoDlg::GetPixelFormat()
{
    MVCC_ENUMVALUE stEnumPixelFormatValue = { 0 };
    MVCC_ENUMENTRY stEnumPixelFormatEntry = { 0 };

    int nRet = m_pcMyCamera->GetEnumValue("PixelFormat", &stEnumPixelFormatValue);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    m_ctrlPixelFormatCombo.ResetContent();
    for (int i = 0; i < stEnumPixelFormatValue.nSupportedNum; i++)
    {
        memset(&stEnumPixelFormatEntry, 0, sizeof(stEnumPixelFormatEntry));
        stEnumPixelFormatEntry.nValue = stEnumPixelFormatValue.nSupportValue[i];
        m_pcMyCamera->GetEnumEntrySymbolic("PixelFormat", &stEnumPixelFormatEntry);

		char chIn[MV_MAX_SYMBOLIC_LEN] = "";
		memcpy(chIn, stEnumPixelFormatEntry.chSymbolic, MV_MAX_SYMBOLIC_LEN);
		wchar_t wchOut[MV_MAX_SYMBOLIC_LEN] = { 0 };
		Char2Wchar(chIn, wchOut, MV_MAX_SYMBOLIC_LEN);

		CString strMsg = _T("");

		strMsg.Format(_T("%s"), wchOut);

        m_ctrlPixelFormatCombo.AddString(strMsg);

        m_mapPixelFormat.insert(pair<CString, int>(strMsg, stEnumPixelFormatEntry.nValue));
    }

    for (int i = 0; i < stEnumPixelFormatValue.nSupportedNum; i++)
    {
        if (stEnumPixelFormatValue.nCurValue == stEnumPixelFormatValue.nSupportValue[i])
        {
            m_nPixelFormat = i;
            m_ctrlPixelFormatCombo.SetCurSel(m_nPixelFormat);
        }
    }

    return MV_OK;
}

// ch:保存图片 | en:Save Image
int CBasicDemoDlg::SaveImage(MV_SAVE_IAMGE_TYPE enSaveImageType)
{
    MV_SAVE_IMAGE_TO_FILE_PARAM_EX stSaveFileParam;
    memset(&stSaveFileParam, 0, sizeof(MV_SAVE_IMAGE_TO_FILE_PARAM_EX));

    EnterCriticalSection(&m_hSaveImageMux);
    if (m_pSaveImageBuf == NULL || m_stImageInfo.enPixelType == 0)
    {
        LeaveCriticalSection(&m_hSaveImageMux);
        return MV_E_NODATA;
    }

    stSaveFileParam.enImageType = enSaveImageType; // ch:需要保存的图像类型 | en:Image format to save
    stSaveFileParam.enPixelType = m_stImageInfo.enPixelType;  // ch:相机对应的像素格式 | en:Camera pixel type
    stSaveFileParam.nWidth      = m_stImageInfo.nWidth;         // ch:相机对应的宽 | en:Width
    stSaveFileParam.nHeight     = m_stImageInfo.nHeight;          // ch:相机对应的高 | en:Height
    stSaveFileParam.nDataLen    = m_stImageInfo.nFrameLen;
    stSaveFileParam.pData       = m_pSaveImageBuf;
    stSaveFileParam.iMethodValue = 0;
	stSaveFileParam.pcImagePath=(char*)malloc(256);
	memset(stSaveFileParam.pcImagePath,0,256);

    // ch:jpg图像质量范围为(50-99], png图像质量范围为[0-9] | en:jpg image nQuality range is (50-99], png image nQuality range is [0-9]
    if (MV_Image_Bmp == stSaveFileParam.enImageType)
    {
        sprintf_s(stSaveFileParam.pcImagePath, 256, "Image_w%d_h%d_fn%03d.bmp", stSaveFileParam.nWidth, stSaveFileParam.nHeight, m_stImageInfo.nFrameNum);
    }
    else if (MV_Image_Jpeg == stSaveFileParam.enImageType)
    {
        stSaveFileParam.nQuality = 80;
        sprintf_s(stSaveFileParam.pcImagePath, 256, "Image_w%d_h%d_fn%03d.jpg", stSaveFileParam.nWidth, stSaveFileParam.nHeight, m_stImageInfo.nFrameNum);
    }
    else if (MV_Image_Tif == stSaveFileParam.enImageType)
    {
        sprintf_s(stSaveFileParam.pcImagePath, 256, "Image_w%d_h%d_fn%03d.tif", stSaveFileParam.nWidth, stSaveFileParam.nHeight, m_stImageInfo.nFrameNum);
    }
    else if (MV_Image_Png == stSaveFileParam.enImageType)
    {
        stSaveFileParam.nQuality = 8;
        sprintf_s(stSaveFileParam.pcImagePath, 256, "Image_w%d_h%d_fn%03d.png", stSaveFileParam.nWidth, stSaveFileParam.nHeight, m_stImageInfo.nFrameNum);
    }

    int nRet = m_pcMyCamera->SaveImageToFile(&stSaveFileParam);
    LeaveCriticalSection(&m_hSaveImageMux);
	free(stSaveFileParam.pcImagePath);

    return nRet;
}

int CBasicDemoDlg::GrabThreadProcess()
{
    MV_FRAME_OUT stImageInfo = {0};
    MV_DISPLAY_FRAME_INFO stDisplayInfo = {0};
    MV_CC_HB_DECODE_PARAM stDecodeParam = { 0 };
    int nRet = MV_OK;

    CString strCBText;
    if (TRUE == m_bHBMode)
    {
        int nIndex = m_ctrlImageCompressionModeCombo.GetCurSel();
        m_ctrlImageCompressionModeCombo.GetLBText(nIndex, strCBText);
    }
  
    while(m_bThreadState)
    {
		if (!m_bStartGrabbing)
		{
			Sleep(10);
			continue;
		}

        nRet = m_pcMyCamera->GetImageBuffer(&stImageInfo, 1000);
        if (nRet == MV_OK)
        {
            //用于保存图片  | en:save image
            EnterCriticalSection(&m_hSaveImageMux);
            if (NULL == m_pSaveImageBuf || stImageInfo.stFrameInfo.nFrameLen > m_nSaveImageBufSize)
            {
                if (m_pSaveImageBuf)
                {
                    free(m_pSaveImageBuf);
                    m_pSaveImageBuf = NULL;
                }

                m_pSaveImageBuf = (unsigned char *)malloc(sizeof(unsigned char) * stImageInfo.stFrameInfo.nFrameLen);
                if (m_pSaveImageBuf == NULL)
                {
                    LeaveCriticalSection(&m_hSaveImageMux);
					m_pcMyCamera->FreeImageBuffer(&stImageInfo);
                    return 0;
                }
                m_nSaveImageBufSize = stImageInfo.stFrameInfo.nFrameLen;
            }
           
            memcpy(m_pSaveImageBuf, stImageInfo.pBufAddr, stImageInfo.stFrameInfo.nFrameLen);
            
            memcpy(&m_stImageInfo, &(stImageInfo.stFrameInfo), sizeof(MV_FRAME_OUT_INFO_EX));
            LeaveCriticalSection(&m_hSaveImageMux);

            stDisplayInfo.hWnd = m_hwndDisplay;
            stDisplayInfo.pData = stImageInfo.pBufAddr;
            stDisplayInfo.nDataLen = stImageInfo.stFrameInfo.nFrameLen;
            stDisplayInfo.nWidth = stImageInfo.stFrameInfo.nWidth;
            stDisplayInfo.nHeight = stImageInfo.stFrameInfo.nHeight;
            stDisplayInfo.enPixelType = stImageInfo.stFrameInfo.enPixelType;
            m_pcMyCamera->DisplayOneFrame(&stDisplayInfo);

            m_pcMyCamera->FreeImageBuffer(&stImageInfo);

        }
        else
        {
            if (MV_TRIGGER_MODE_ON ==  m_nTriggerMode)
            {
                Sleep(5);
            }
        }
    }

    return MV_OK;
}
// ch:按下查找设备按钮:枚举 | en:Click Find Device button:Enumeration 
void CBasicDemoDlg::OnBnClickedEnumButton()
{
    CString strMsg;

    m_ctrlDeviceCombo.ResetContent();
    memset(&m_stDevList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));

    // ch:枚举子网内所有设备 | en:Enumerate all devices within subnet
    int nRet = CMvCamera::EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &m_stDevList);
    if (MV_OK != nRet)
    {
        return;
    }

    // ch:将值加入到信息列表框中并显示出来 | en:Add value to the information list box and display
    for (unsigned int i = 0; i < m_stDevList.nDeviceNum; i++)
    {
        MV_CC_DEVICE_INFO* pDeviceInfo = m_stDevList.pDeviceInfo[i];
        if (NULL == pDeviceInfo)
        {
            continue;
        }

        wchar_t* pUserName = NULL;
        if (pDeviceInfo->nTLayerType == MV_GIGE_DEVICE)
        {
            int nIp1 = ((m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24);
            int nIp2 = ((m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16);
            int nIp3 = ((m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8);
            int nIp4 = (m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

            if (strcmp("", (LPCSTR)(pDeviceInfo->SpecialInfo.stGigEInfo.chUserDefinedName)) != 0)
            {
                char strUserName[256] = {0};
				sprintf_s(strUserName, 256, "%s (%s)", pDeviceInfo->SpecialInfo.stGigEInfo.chUserDefinedName,
					pDeviceInfo->SpecialInfo.stGigEInfo.chSerialNumber);
                DWORD dwLenUserName = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strUserName), -1, NULL, 0);
                pUserName = new wchar_t[dwLenUserName];
                MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strUserName), -1, pUserName, dwLenUserName);
            }
            else
            {
                char strUserName[256] = {0};
                sprintf_s(strUserName, 256, "%s (%s)", pDeviceInfo->SpecialInfo.stGigEInfo.chModelName,
                    pDeviceInfo->SpecialInfo.stGigEInfo.chSerialNumber);
                DWORD dwLenUserName = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strUserName), -1, NULL, 0);
                pUserName = new wchar_t[dwLenUserName];
                MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strUserName), -1, pUserName, dwLenUserName);
            }
            strMsg.Format(_T("[%d]GigE:    %s  (%d.%d.%d.%d)"), i, pUserName, nIp1, nIp2, nIp3, nIp4);
        }
        else if (pDeviceInfo->nTLayerType == MV_USB_DEVICE)
        {
            if (strcmp("", (char*)pDeviceInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName) != 0)
            {
               char strUserName[256] = {0};
				sprintf_s(strUserName, 256, "%s (%s)", pDeviceInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName,
					pDeviceInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);
                DWORD dwLenUserName = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strUserName), -1, NULL, 0);
                pUserName = new wchar_t[dwLenUserName];
                MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strUserName), -1, pUserName, dwLenUserName);
            }
            else
            {
                char strUserName[256] = {0};
               sprintf_s(strUserName, 256, "%s (%s)", pDeviceInfo->SpecialInfo.stUsb3VInfo.chModelName,
                    pDeviceInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);
                DWORD dwLenUserName = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strUserName), -1, NULL, 0);
                pUserName = new wchar_t[dwLenUserName];
                MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strUserName), -1, pUserName, dwLenUserName);
            }
            strMsg.Format(_T("[%d]UsbV3:  %s"), i, pUserName);
        }
        else
        {
            ShowErrorMsg(TEXT("Unknown device enumerated"), 0);
        }
        m_ctrlDeviceCombo.AddString(strMsg);

        if (pUserName)
        {
            delete[] pUserName;
            pUserName = NULL;
        }
    }

    if (0 == m_stDevList.nDeviceNum)
    {
        ShowErrorMsg(TEXT("No device"), 0);
        return;
    }
    m_ctrlDeviceCombo.SetCurSel(0);

    EnableControls(TRUE);
}

// ch:按下打开设备按钮：打开设备 | en:Click Open button: Open Device
void CBasicDemoDlg::OnBnClickedOpenButton()
{
    if (TRUE == m_bOpenDevice || NULL != m_pcMyCamera)
    {
		ShowErrorMsg(TEXT("The device has been opened or Handle is not empty"), 0);
        return;
    }
    UpdateData(TRUE);

    int nIndex = m_nDeviceCombo;
    if ((nIndex < 0) | (nIndex >= MV_MAX_DEVICE_NUM))
    {
        ShowErrorMsg(TEXT("Please select device"), 0);
        return;
    }

    // ch:由设备信息创建设备实例 | en:Device instance created by device information
    if (NULL == m_stDevList.pDeviceInfo[nIndex])
    {
        ShowErrorMsg(TEXT("Device does not exist"), 0);
        return;
    }

    m_pcMyCamera = new CMvCamera;
    if (NULL == m_pcMyCamera)
    {
        return;
    }

    int nRet = m_pcMyCamera->Open(m_stDevList.pDeviceInfo[nIndex]);
    if (MV_OK != nRet)
    {
        delete m_pcMyCamera;
        m_pcMyCamera = NULL;
        ShowErrorMsg(TEXT("Open Fail"), nRet);
        return;
    }

    // ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera)
    if (m_stDevList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE)
    {
        unsigned int nPacketSize = 0;
        nRet = m_pcMyCamera->GetOptimalPacketSize(&nPacketSize);
        if (nRet == MV_OK)
        {
            nRet = m_pcMyCamera->SetIntValue("GevSCPSPacketSize",nPacketSize);
            if(nRet != MV_OK)
            {
                ShowErrorMsg(TEXT("Warning: Set Packet Size fail!"), nRet);
            }
        }
        else
        {
            ShowErrorMsg(TEXT("Warning: Get Packet Size fail!"), nRet);
        }
    }

    m_bOpenDevice = TRUE;
    EnableControls(TRUE);
    GetDlgItem(IDC_EXPOSUACRE_EDIT)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);
    OnBnClickedGetParameterButton(); // ch:获取参数 | en:Get Parameter
    EnableControls(TRUE);
}

// ch:按下关闭设备按钮：关闭设备 | en:Click Close button: Close Device
void CBasicDemoDlg::OnBnClickedCloseButton()
{
    CloseDevice();
    m_mapPixelFormat.clear();
    m_mapPreampGain.clear();
	m_mapTriggerSource.clear();
    m_bTriggerModeCheck = FALSE;
    m_bAcquisitionLineRate = FALSE;
    m_bPreampGain = FALSE;
    m_bHBMode = FALSE;
    EnableControls(TRUE);

    //关闭相机后清空显示图像
    GetDlgItem(IDC_DISPLAY_STATIC)->ShowWindow(FALSE);
    GetDlgItem(IDC_DISPLAY_STATIC)->ShowWindow(TRUE);
}

// ch:按下开始采集按钮 | en:Click Start button
void CBasicDemoDlg::OnBnClickedStartGrabbingButton()
{
    if (FALSE == m_bOpenDevice || TRUE == m_bStartGrabbing || NULL == m_pcMyCamera)
    {
        return;
    }  

    memset(&m_stImageInfo, 0, sizeof(MV_FRAME_OUT_INFO_EX));
    m_bThreadState = TRUE;
    unsigned int nThreadID = 0;
    m_hGrabThread = (void*)_beginthreadex( NULL , 0 , GrabThread , this, 0 , &nThreadID );
    if (NULL == m_hGrabThread)
    {
        m_bThreadState = FALSE;
        ShowErrorMsg(TEXT("Create thread fail"), 0);
        return;
    }

	int nRet = m_pcMyCamera->StartGrabbing();
	if (MV_OK != nRet)
	{
		m_bThreadState = FALSE;
		ShowErrorMsg(TEXT("Start grabbing fail"), nRet);
		return;
	}
    m_bStartGrabbing = TRUE;
    EnableControls(TRUE);

    int nIndex = m_ctrlTriggerSourceCombo.GetCurSel();

    CString strCBText;

    m_ctrlTriggerSourceCombo.GetLBText(nIndex, strCBText);

    if (STR_SOFTWARE == strCBText && m_bTriggerModeCheck == TRUE)
    {
        GetDlgItem(IDC_SOFTWARE_ONCE_BUTTON)->EnableWindow(TRUE);
    }
    else
    {
        GetDlgItem(IDC_SOFTWARE_ONCE_BUTTON)->EnableWindow(FALSE);
    }

    OnBnClickedGetParameterButton(); // ch:获取参数 | en:Get Parameter
}

// ch:按下结束采集按钮 | en:Click Stop button
void CBasicDemoDlg::OnBnClickedStopGrabbingButton()
{
    if (FALSE == m_bOpenDevice || FALSE == m_bStartGrabbing || NULL == m_pcMyCamera)
    {
        return;
    }

    m_bThreadState = FALSE;
    if (m_hGrabThread)
    {
        WaitForSingleObject(m_hGrabThread, INFINITE);
        CloseHandle(m_hGrabThread);
        m_hGrabThread = NULL;
    }

    int nRet = m_pcMyCamera->StopGrabbing();
    if (MV_OK != nRet)
    {
        ShowErrorMsg(TEXT("Stop grabbing fail"), nRet);
        return;
    }
    m_bStartGrabbing = FALSE;
    EnableControls(TRUE);
    OnBnClickedGetParameterButton(); // ch:获取参数 | en:Get Parameter
}

// ch:按下获取参数按钮 | en:Click Get Parameter button
void CBasicDemoDlg::OnBnClickedGetParameterButton()
{
    UpdateData(TRUE);

    int nRet = GetTriggerSelector();
    if (nRet != MV_OK)
    {
        GetDlgItem(IDC_TRIGGERSEL_COMBO)->EnableWindow(FALSE);
    }

    nRet = GetTriggerMode();
    if (nRet != MV_OK)
    {
        GetDlgItem(IDC_TRIGGERSWITCH_COMBO)->EnableWindow(FALSE);
    }

    nRet = GetTriggerSource();
    if (nRet != MV_OK)
    {
        GetDlgItem(IDC_TRIGGERSOURCE_COMBO)->EnableWindow(FALSE);
    }

    nRet = GetExposureTime();
    if (nRet != MV_OK)
    {
        GetDlgItem(IDC_EXPOSUACRE_EDIT)->EnableWindow(FALSE);
    }

    nRet = GetDigitalShiftGain();
    if (nRet != MV_OK)
    {
        GetDlgItem(IDC_DIGITAL_SHIFT_EDIT)->EnableWindow(FALSE);
    }

    nRet = GetPreampGain();
    if (nRet != MV_OK)
    {
        GetDlgItem(IDC_PREAMPGAIN_COMBO)->EnableWindow(FALSE);
    }

    nRet = GetAcquisitionLineRateEnable();
    if (nRet != MV_OK)
    {
        GetDlgItem(IDC_ACQUISITION_LINE_RATE_EDIT)->EnableWindow(FALSE);
    }

    nRet = GetAcquisitionLineRate();
    if (nRet != MV_OK)
    {
        GetDlgItem(IDC_ACQUISITION_LINE_RATE_ENABLE_CHECK)->EnableWindow(FALSE);
    }

    nRet = GetResultingLineRate();
    if (nRet != MV_OK)
    {
        GetDlgItem(IDC_RESULTING_LINE_RATE_EDIT)->EnableWindow(FALSE);
    }

    nRet = GetPixelFormat();
    if (nRet != MV_OK)
    {
        GetDlgItem(IDC_PIXELFORMAT_COMBO)->EnableWindow(FALSE);
    }

    nRet = GetImageCompressionMode();
    if (nRet != MV_OK)
    {
        GetDlgItem(IDC_IMAGE_COMPRESSION_MODE_COMBO)->EnableWindow(FALSE);
    }

    UpdateData(FALSE);
}

// ch:按下设置参数按钮 | en:Click Set Parameter button
void CBasicDemoDlg::OnBnClickedSetParameterButton()
{
    UpdateData(TRUE);

    bool bIsSetSucceed = true;
    int nRet = SetExposureTime();
    if (nRet != MV_OK)
    {
        bIsSetSucceed = false;
        ShowErrorMsg(TEXT("Set Exposure Time Fail"), nRet);
    }
    nRet = SetDigitalShiftGain();
    if (nRet != MV_OK)
    {
        bIsSetSucceed = false;
        ShowErrorMsg(TEXT("Set Digital Shift Fail"), nRet);
    }

    if (TRUE == m_bAcquisitionLineRate)
    {
        nRet = SetAcquisitionLineRate();
        if (nRet != MV_OK)
        {
            bIsSetSucceed = false;
            ShowErrorMsg(TEXT("Set Acquisition Line Rate Fail"), nRet);
        }
    }
    
    if (true == bIsSetSucceed)
    {
        ShowErrorMsg(TEXT("Set Parameter Succeed"), nRet);
    }
}

// ch:按下软触发一次按钮 | en:Click Execute button
void CBasicDemoDlg::OnBnClickedSoftwareOnceButton()
{
    if (TRUE != m_bStartGrabbing)
    {
        return;
    }

    m_pcMyCamera->CommandExecute("TriggerSoftware");
}

// ch:按下保存bmp图片按钮 | en:Click Save BMP button
void CBasicDemoDlg::OnBnClickedSaveBmpButton()
{
    int nRet = SaveImage(MV_Image_Bmp);
    if (MV_OK != nRet)
    {
        ShowErrorMsg(TEXT("Save bmp fail"), nRet);
        return;
    }
    ShowErrorMsg(TEXT("Save bmp succeed"), nRet);
}

// ch:按下保存jpg图片按钮 | en:Click Save JPG button
void CBasicDemoDlg::OnBnClickedSaveJpgButton()
{
    int nRet = SaveImage(MV_Image_Jpeg);
    if (MV_OK != nRet)
    {
        ShowErrorMsg(TEXT("Save jpg fail"), nRet);
        return;
    }
    ShowErrorMsg(TEXT("Save jpg succeed"), nRet);
}

void CBasicDemoDlg::OnBnClickedSaveTiffButton()
{
    int nRet = SaveImage(MV_Image_Tif);
    if (MV_OK != nRet)
    {
        ShowErrorMsg(TEXT("Save tiff fail"), nRet);
        return;
    }
    ShowErrorMsg(TEXT("Save tiff succeed"), nRet);
}

void CBasicDemoDlg::OnBnClickedSavePngButton()
{
    int nRet = SaveImage(MV_Image_Png);
    if (MV_OK != nRet)
    {
        ShowErrorMsg(TEXT("Save png fail"), nRet);
        return;
    }
    ShowErrorMsg(TEXT("Save png succeed"), nRet);
}

// ch:右上角退出 | en:Exit from upper right corner
void CBasicDemoDlg::OnClose()
{
    PostQuitMessage(0);
    CloseDevice();

    DeleteCriticalSection(&m_hSaveImageMux);
	CMvCamera::FinalizeSDK();
    CDialog::OnClose();
}

BOOL CBasicDemoDlg::PreTranslateMessage(MSG* pMsg)
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

void CBasicDemoDlg::OnCbnSelchangeTriggerselCombo()
{
    UpdateData(true);
 
    CString strCBText;
    int nIndex = m_ctrlTriggerSelectorCombo.GetCurSel();
    m_ctrlTriggerSelectorCombo.GetLBText(nIndex, strCBText);

    if (STR_FRAMEBURSTSTART == strCBText)
    {
        int nRet = m_pcMyCamera->SetEnumValue("TriggerSelector", FRAMEBURSTSTART);
        if (MV_OK != nRet)
        {
            ShowErrorMsg(TEXT("Set TriggerSelector FrameBurstStart fail"), nRet);
            return;
        }

        int nIndex = m_ctrlTriggerModeCombo.GetCurSel();
        m_ctrlTriggerModeCombo.GetLBText(nIndex, strCBText);
        CString cStrTriggerMode = strCBText;

        nIndex = m_ctrlTriggerSourceCombo.GetCurSel();
        m_ctrlTriggerSourceCombo.GetLBText(nIndex, strCBText);
        CString cStrTriggerSourceCombo = strCBText;
        if (cStrTriggerMode == "On" && STR_SOFTWARE == cStrTriggerSourceCombo)
        {
            m_bTriggerModeCheck = TRUE;
        }
    }
    else if (strCBText == "LineStart")
    {
        int nRet = m_pcMyCamera->SetEnumValue("TriggerSelector", LINESTART);
        if (MV_OK != nRet)
        {
            ShowErrorMsg(TEXT("Set TriggerSelector LineStart fail"), nRet);
            return;
        }    
        m_bTriggerModeCheck = FALSE;
    }

    int nRet = GetTriggerSource();
    if (nRet != MV_OK)
    {
        ShowErrorMsg(TEXT("Get Trigger Source Fail"), nRet);
        return;
    }
    EnableControls(TRUE);
}

void CBasicDemoDlg::OnCbnSelchangeTriggerswitchCombo()
{
    UpdateData(true);

    int nIndex = m_ctrlTriggerModeCombo.GetCurSel();

    CString strCBText;

    m_ctrlTriggerModeCombo.GetLBText(nIndex, strCBText);

    if (strCBText == "On")
    {
        int nRet = m_pcMyCamera->SetEnumValue("TriggerMode", MV_TRIGGER_MODE_ON);
        if (MV_OK != nRet)
        {
            ShowErrorMsg(TEXT("Set Trigger Mode fail"), nRet);
            return;
        }

        int nIndex = m_ctrlTriggerSelectorCombo.GetCurSel();
        m_ctrlTriggerSelectorCombo.GetLBText(nIndex, strCBText);
        CString cStrTriggerSelector = strCBText;

        nIndex = m_ctrlTriggerSourceCombo.GetCurSel();
        m_ctrlTriggerSourceCombo.GetLBText(nIndex, strCBText);
        CString cStrTriggerSourceCombo = strCBText;
        if (STR_FRAMEBURSTSTART == cStrTriggerSelector && STR_SOFTWARE == cStrTriggerSourceCombo)
        {
            m_bTriggerModeCheck = TRUE;
        }
    }
    else if (strCBText == "Off")
    {
        int nRet = m_pcMyCamera->SetEnumValue("TriggerMode", MV_TRIGGER_MODE_OFF);
        if (MV_OK != nRet)
        {
            ShowErrorMsg(TEXT("Set Trigger Mode fail"), nRet);
            return;
        }
        m_bTriggerModeCheck = FALSE;
    }
    EnableControls(TRUE);
}


void CBasicDemoDlg::OnCbnSelchangeTriggersourceCombo()
{
    UpdateData(true);

    m_bTriggerModeCheck = FALSE;

    int nIndex = m_ctrlTriggerSourceCombo.GetCurSel();

    CString strCBText;

    m_ctrlTriggerSourceCombo.GetLBText(nIndex, strCBText);

    /*if (strCBText == "Line0")
    {
        int nRet = m_pcMyCamera->SetEnumValue("TriggerSource", MV_TRIGGER_SOURCE_LINE0);
        if (MV_OK != nRet)
        {
            ShowErrorMsg(TEXT("Set Trigger Source fail"), nRet);
            return;
        }
    }
    else if (strCBText == "Line2")
    {
        int nRet = m_pcMyCamera->SetEnumValue("TriggerSource", MV_TRIGGER_SOURCE_LINE2);
        if (MV_OK != nRet)
        {
            ShowErrorMsg(TEXT("Set Trigger Source fail"), nRet);
            return;
        }
    }
    else if (strCBText == "Line3")
    {
        int nRet = m_pcMyCamera->SetEnumValue("TriggerSource", MV_TRIGGER_SOURCE_LINE3);
        if (MV_OK != nRet)
        {
            ShowErrorMsg(TEXT("Set Trigger Source fail"), nRet);
            return;
        }
    }
    else if (strCBText == "FrequencyConverter")
    {
        int nRet = m_pcMyCamera->SetEnumValue("TriggerSource", MV_TRIGGER_SOURCE_FrequencyConverter);
        if (MV_OK != nRet)
        {
            ShowErrorMsg(TEXT("Set Trigger Source fail"), nRet);
            return;
        }
    }
    else if (strCBText == "EncoderModuleOut")
    {
        int nRet = m_pcMyCamera->SetEnumValue("TriggerSource", MV_TRIGGER_SOURCE_EncoderModuleOut);
        if (MV_OK != nRet)
        {
            ShowErrorMsg(TEXT("Set Trigger Source fail"), nRet);
            return;
        }
    }*/
    if (STR_SOFTWARE == strCBText)
    {
        int nRet = m_pcMyCamera->SetEnumValue("TriggerSource", MV_TRIGGER_SOURCE_SOFTWARE);
        if (MV_OK != nRet)
        {
            ShowErrorMsg(TEXT("Set Trigger Source fail"), nRet);
            return;
        }

        int nIndex = m_ctrlTriggerSelectorCombo.GetCurSel();
        m_ctrlTriggerSelectorCombo.GetLBText(nIndex, strCBText);
        CString cStrTriggerSelector = strCBText;

        nIndex = m_ctrlTriggerModeCombo.GetCurSel();
        m_ctrlTriggerModeCombo.GetLBText(nIndex, strCBText);
        CString cStrTriggerMode = strCBText;
        if (STR_FRAMEBURSTSTART == cStrTriggerSelector && cStrTriggerMode == "On")
        {
            m_bTriggerModeCheck = TRUE;
        }
    }


	for (map<CString, int>::iterator it = m_mapTriggerSource.begin(); it != m_mapTriggerSource.end(); it++)
	{
		if (it->first == strCBText)
		{
			int nRet = m_pcMyCamera->SetEnumValue("TriggerSource", it->second);
			if (MV_OK != nRet)
			{
				ShowErrorMsg(TEXT("Set TriggerSource fail"), nRet);
				return;
			}
			break;
		}
	}

    EnableControls(TRUE);
}

void CBasicDemoDlg::OnCbnSelchangePixelformatCombo()
{
    UpdateData(true);

    int nIndex = m_ctrlPixelFormatCombo.GetCurSel();

    CString strCBText;

    m_ctrlPixelFormatCombo.GetLBText(nIndex, strCBText);

    for (map<CString, int>::iterator it = m_mapPixelFormat.begin(); it != m_mapPixelFormat.end(); it++)
    {
        if (it->first == strCBText)
        {
            int nRet = m_pcMyCamera->SetEnumValue("PixelFormat", it->second);
            if (MV_OK != nRet)
            {
                ShowErrorMsg(TEXT("Set PixelFormat fail"), nRet);
                return;
            }
            break;
        }
    }
    
    if (TRUE == m_bHBMode)
    {
        int nRet = GetImageCompressionMode();
        if (nRet != MV_OK)
        {
            ShowErrorMsg(TEXT("Get Image Compression Mode Fail"), nRet);
            return;
        }
    }
}



void CBasicDemoDlg::OnCbnSelchangePreampgainCombo()
{
    UpdateData(true);

    int nIndex = m_ctrlPreampGainCombo.GetCurSel();

    CString strCBText;

    m_ctrlPreampGainCombo.GetLBText(nIndex, strCBText);

    for (map<CString, int>::iterator it = m_mapPreampGain.begin(); it != m_mapPreampGain.end(); it++)
    {
        if (it->first == strCBText)
        {
            int nRet = m_pcMyCamera->SetEnumValue("PreampGain", it->second);
            if (MV_OK != nRet)
            {
                ShowErrorMsg(TEXT("Set PreampGain fail"), nRet);
                return;
            }
            break;
        }
    }
}

void CBasicDemoDlg::OnCbnSelchangeImageCompressionModeCombo()
{
    UpdateData(true);

    int nIndex = m_ctrlImageCompressionModeCombo.GetCurSel();

    CString strCBText;

    m_ctrlImageCompressionModeCombo.GetLBText(nIndex, strCBText);

    if (strCBText == "Off")
    {
        int nRet = m_pcMyCamera->SetEnumValue("ImageCompressionMode",IMAGE_COMPRESSION_MODE_OFF);
        if (MV_OK != nRet)
        {
            ShowErrorMsg(TEXT("Set Image Compression Mode fail"), nRet);
            return;
        }
    }
    else if (strCBText == "HB")
    {
        int nRet = m_pcMyCamera->SetEnumValue("ImageCompressionMode", IMAGE_COMPRESSION_MODE_HB);
        if (MV_OK != nRet)
        {
            ShowErrorMsg(TEXT("Set Image Compression Mode fail"), nRet);
            return;
        }
    }
}

uint64_t CBasicDemoDlg::GetPixelSize(MvGvspPixelType enType, UINT16 nWidth, UINT16 nHeight)
{
    uint64_t nWidth64 = nWidth;
    uint64_t nHeight64 = nHeight;
    uint64_t nSize = nWidth64 * nHeight64;

    switch (enType)
    {
    case PixelType_Gvsp_Mono8:
    case PixelType_Gvsp_HB_Mono8:
    case PixelType_Gvsp_BayerGR8:
    case PixelType_Gvsp_HB_BayerGR8:
    case PixelType_Gvsp_BayerRG8:
    case PixelType_Gvsp_HB_BayerRG8:
    case PixelType_Gvsp_BayerGB8:
    case PixelType_Gvsp_HB_BayerGB8:
    case PixelType_Gvsp_BayerBG8:
    case PixelType_Gvsp_HB_BayerBG8:
    case PixelType_Gvsp_BayerRBGG8:
    case PixelType_Gvsp_HB_BayerRBGG8:  
         return nSize;
    case PixelType_Gvsp_Mono10:
    case PixelType_Gvsp_HB_Mono10:
    case PixelType_Gvsp_Mono12:
    case PixelType_Gvsp_HB_Mono12:
    case PixelType_Gvsp_Mono16:
    case PixelType_Gvsp_HB_Mono16:
    case PixelType_Gvsp_BayerBG10:
    case PixelType_Gvsp_HB_BayerBG10:
    case PixelType_Gvsp_BayerGB10:
    case PixelType_Gvsp_HB_BayerGB10:
    case PixelType_Gvsp_BayerRG10:
    case PixelType_Gvsp_HB_BayerRG10:
    case PixelType_Gvsp_BayerGR10:
    case PixelType_Gvsp_HB_BayerGR10:
    case PixelType_Gvsp_BayerGR12:
    case PixelType_Gvsp_HB_BayerGR12:
    case PixelType_Gvsp_BayerRG12:
    case PixelType_Gvsp_HB_BayerRG12:
    case PixelType_Gvsp_BayerGB12:
    case PixelType_Gvsp_HB_BayerGB12:
    case PixelType_Gvsp_BayerBG12:
    case PixelType_Gvsp_HB_BayerBG12:
    case PixelType_Gvsp_BayerGR16:
    case PixelType_Gvsp_BayerRG16:
    case PixelType_Gvsp_BayerGB16:
    case PixelType_Gvsp_BayerBG16:
    case PixelType_Gvsp_YUV422_Packed:
    case PixelType_Gvsp_HB_YUV422_Packed:
    case PixelType_Gvsp_YUV422_YUYV_Packed:
    case PixelType_Gvsp_HB_YUV422_YUYV_Packed:  
         return (nSize * 2);
    case PixelType_Gvsp_Mono10_Packed:
    case PixelType_Gvsp_HB_Mono10_Packed:
    case PixelType_Gvsp_Mono12_Packed:
    case PixelType_Gvsp_HB_Mono12_Packed:
    case PixelType_Gvsp_BayerBG10_Packed:
    case PixelType_Gvsp_HB_BayerBG10_Packed:
    case PixelType_Gvsp_BayerGB10_Packed:
    case PixelType_Gvsp_HB_BayerGB10_Packed:
    case PixelType_Gvsp_BayerRG10_Packed:
    case PixelType_Gvsp_HB_BayerRG10_Packed:
    case PixelType_Gvsp_BayerGR10_Packed:
    case PixelType_Gvsp_HB_BayerGR10_Packed:
    case PixelType_Gvsp_BayerBG12_Packed:
    case PixelType_Gvsp_HB_BayerBG12_Packed:
    case PixelType_Gvsp_BayerGB12_Packed:
    case PixelType_Gvsp_HB_BayerGB12_Packed:
    case PixelType_Gvsp_BayerRG12_Packed:
    case PixelType_Gvsp_HB_BayerRG12_Packed:
    case PixelType_Gvsp_BayerGR12_Packed:
    case PixelType_Gvsp_HB_BayerGR12_Packed:  
         return (nSize * 3 / 2);
    case PixelType_Gvsp_RGB8_Planar:
    case PixelType_Gvsp_RGB8_Packed:
    case PixelType_Gvsp_HB_RGB8_Packed:
    case PixelType_Gvsp_BGR8_Packed:
    case PixelType_Gvsp_HB_BGR8_Packed:  
         return nSize * 3;
    case PixelType_Gvsp_RGBA8_Packed:
    case PixelType_Gvsp_HB_RGBA8_Packed:
    case PixelType_Gvsp_BGRA8_Packed:
    case PixelType_Gvsp_HB_BGRA8_Packed:  
         return (nSize * 4);
    case PixelType_Gvsp_Coord3D_A32f:
    case PixelType_Gvsp_Coord3D_A32:
    case PixelType_Gvsp_Coord3D_C32f:
    case PixelType_Gvsp_Coord3D_C32:  
         return (nSize * 1 * 4);
    case PixelType_Gvsp_Coord3D_AC32f:
    case PixelType_Gvsp_Coord3D_AC32:
    case PixelType_Gvsp_Coord3D_AB32f:
    case PixelType_Gvsp_Coord3D_AB32:
    case PixelType_Gvsp_RGBA16_Packed:
    case PixelType_Gvsp_HB_RGBA16_Packed:
    case PixelType_Gvsp_BGRA16_Packed:
    case PixelType_Gvsp_HB_BGRA16_Packed:  
         return (nSize * 2 * 4);
    case PixelType_Gvsp_Coord3D_ABC32f:
    case PixelType_Gvsp_Coord3D_ABC32:  
         return (nSize * 3 * 4);
    case PixelType_Gvsp_Coord3D_ABC16:  
         return (nSize * 3 * 2);
    default:
        return 0;
    }
}

void CBasicDemoDlg::OnBnClickedAcquisitionLineRateEnableCheck()
{
    CButton* pBtn = (CButton*)GetDlgItem(IDC_ACQUISITION_LINE_RATE_ENABLE_CHECK);
    int state = pBtn->GetCheck();
    if (TRUE ==  state)
    {
        int nRet = m_pcMyCamera->SetBoolValue("AcquisitionLineRateEnable", TRUE);
        if (MV_OK != nRet)
        {
            ShowErrorMsg(TEXT("Set Acquisition LineRate Enable fail"), nRet);
            return;
        }
    }
    else
    {
        int nRet = m_pcMyCamera->SetBoolValue("AcquisitionLineRateEnable", FALSE);
        if (MV_OK != nRet)
        {
            ShowErrorMsg(TEXT("Set Acquisition LineRate Enable fail"), nRet);
            return;
        }
    }
}

bool CBasicDemoDlg::IsStrUTF8(const char* pBuffer, int size)
{
	if (size < 0)
	{
		return false;
	}

	bool IsUTF8 = true;
	unsigned char* start = (unsigned char*)pBuffer;
	unsigned char* end = (unsigned char*)pBuffer + size;
	if (NULL == start ||
		NULL == end)
	{
		return false;
	}
	while (start < end)
	{
		if (*start < 0x80) // ch:(10000000): 值小于0x80的为ASCII字符 | en:(10000000): if the value is smaller than 0x80, it is the ASCII character
		{
			start++;
		}
		else if (*start < (0xC0)) // ch:(11000000): 值介于0x80与0xC0之间的为无效UTF-8字符 | en:(11000000): if the value is between 0x80 and 0xC0, it is the invalid UTF-8 character
		{
			IsUTF8 = false;
			break;
		}
		else if (*start < (0xE0)) // ch:(11100000): 此范围内为2字节UTF-8字符  | en: (11100000): if the value is between 0xc0 and 0xE0, it is the 2-byte UTF-8 character
		{
			if (start >= end - 1)
			{
				break;
			}

			if ((start[1] & (0xC0)) != 0x80)
			{
				IsUTF8 = false;
				break;
			}

			start += 2;
		}
		else if (*start < (0xF0)) // ch:(11110000): 此范围内为3字节UTF-8字符 | en: (11110000): if the value is between 0xE0 and 0xF0, it is the 3-byte UTF-8 character 
		{
			if (start >= end - 2)
			{
				break;
			}

			if ((start[1] & (0xC0)) != 0x80 || (start[2] & (0xC0)) != 0x80)
			{
				IsUTF8 = false;
				break;
			}

			start += 3;
		}
		else
		{
			IsUTF8 = false;
			break;
		}
	}

	return IsUTF8;
}

bool CBasicDemoDlg::Char2Wchar(const char *pStr, wchar_t *pOutWStr, int nOutStrSize)
{
	if (!pStr || !pOutWStr)
	{
		return false;
	}

	bool bIsUTF = IsStrUTF8(pStr, strlen(pStr));
	UINT nChgType = bIsUTF ? CP_UTF8 : CP_ACP;

	int iLen = MultiByteToWideChar(nChgType, 0, (LPCSTR)pStr, -1, NULL, 0);

	memset(pOutWStr, 0, sizeof(wchar_t) * nOutStrSize);

	if (iLen >= nOutStrSize)
	{
		iLen = nOutStrSize - 1;
	}

	MultiByteToWideChar(nChgType, 0, (LPCSTR)pStr, -1, pOutWStr, iLen);

	pOutWStr[iLen] = 0;

	return true;
}

bool CBasicDemoDlg::Wchar2char(wchar_t *pOutWStr, char *pStr)
{
	if (!pStr || !pOutWStr)
	{
		return false;
	}

	int nLen = WideCharToMultiByte(CP_ACP, 0, pOutWStr, wcslen(pOutWStr), NULL, 0, NULL, NULL);

	WideCharToMultiByte(CP_ACP, 0, pOutWStr, wcslen(pOutWStr), pStr, nLen, NULL, NULL);

	pStr[nLen] = '\0';

	return true;
}
