/*
This is an example for line scan device.
This example covers enumeration module, control module, parameter configuration module and stream module.
This is also an example of how to save images.

[注] 需要保存文件的示例程序在部分环境下需以管理员权限执行，否则会有异常
[PS] Sample programs that need to save files need to be executed with administrator privileges \
     in some environments, otherwise there will be exceptions
     Sample programs currently support cameralink, cxp, xof interfaces and devices.
*/
#include "stdafx.h"
#include "BasicDemoLineScan.h"
#include "BasicDemoLineScanDlg.h"

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

// CBasicDemoLineScanDlg dialog
CBasicDemoLineScanDlg::CBasicDemoLineScanDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBasicDemoLineScanDlg::IDD, pParent)
    , m_hInterface(NULL)
    , m_hDevice(NULL)
    , m_hStream(NULL)
    , m_nDeviceCombo(0)
    , m_bOpenIF(FALSE)
    , m_bOpenDevice(FALSE)
    , m_bStartGrabbing(FALSE)
    , m_hGrabThread(NULL)
    , m_bThreadState(FALSE)
    , m_nInterfaceNum(0)
    , m_pDataBuf(NULL)
    , m_nDataBufSize(0)
    , m_pSaveImageBuf(NULL)
    , m_nSaveImageBufSize(0)
    , m_bHBMode(FALSE)
	, m_nImageHeightEdit(0)
	, m_nCameraWidth(0)
	, m_nCameraHeight(0)
    , m_bLineInputPolarity(FALSE)
    , m_bCCSelector(FALSE)
    , m_bCCSource(FALSE)
    , m_bScanMode(FALSE)
    , m_bTriggerActivation(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    memset(&m_stImageInfo, 0, sizeof(MV_FG_INPUT_IMAGE_INFO));
    memset(&m_stEnumImageCompressionModeValue, 0, sizeof(MV_FG_ENUMVALUE));
}

void CBasicDemoLineScanDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IF_COMBO, m_ctrlInterfaceCombo);
	DDX_Control(pDX, IDC_DEVICE_COMBO, m_ctrlDeviceCombo);
	DDX_CBIndex(pDX, IDC_DEVICE_COMBO, m_nDeviceCombo);
	DDX_Control(pDX, IDC_CAMERA_TYPE_COMBO, m_ctrlCameraTypeCombo);
	DDX_Text(pDX, IDC_IMAGE_HEIGHT_EDIT, m_nImageHeightEdit);
	DDX_Control(pDX, IDC_LINE_SELECTOR_COMBO, m_ctrlLineSelectorCombo);
	DDX_Control(pDX, IDC_LINE_MODE_COMBO, m_ctrlLineModeCombo);
	DDX_Control(pDX, IDC_LINE_INPUT_POLARITY_COMBO, m_ctrlLineInputPolarityCombo);
	DDX_Control(pDX, IDC_ENCODER_SELECTOR_COMBO, m_ctrlEncoderSelectorCombo);
	DDX_Control(pDX, IDC_ENCODER_SOURCE_A_COMBO, m_ctrlEncoderSourceACombo);
	DDX_Control(pDX, IDC_ENCODER_SOURCE_B_COMBO, m_ctrlEncoderSourceBCombo);
	DDX_Control(pDX, IDC_ENCODER_TRIGGER_MODE_COMBO, m_ctrlEncoderTriggerModeCombo);
	DDX_Control(pDX, IDC_CC_SELECTOR_COMBO, m_ctrlCCSelectorCombo);
	DDX_Control(pDX, IDC_CC_SOURCE_COMBO, m_ctrlCCSourceCombo);

	// ch: 相机参数 | en: Camera params
	DDX_Text(pDX, IDC_WIDTH_EDIT, m_nCameraWidth);
	DDX_Text(pDX, IDC_HEIGHT_EDIT, m_nCameraHeight);
	DDX_Control(pDX, IDC_PIXELFORMAT_COMBO, m_ctrlPixelFormatCombo);
	DDX_Control(pDX, IDC_SCAN_MODE_COMBO, m_ctrlScanModeCombo);
	DDX_Control(pDX, IDC_TRIGGER_SELECTOR_COMBO, m_ctrlTriggerSelectorCombo);
	DDX_Control(pDX, IDC_TRIGGER_MODE_COMB, m_ctrlTriggerModeCombo);
	DDX_Control(pDX, IDC_TRIGGER_SOURCE_COMBO, m_ctrlTriggerSourceCombo);
	DDX_Control(pDX, IDC_TRIGGER_ACTIVATION_COMBO, m_ctrlTriggerActivationCombo);
	
}

BEGIN_MESSAGE_MAP(CBasicDemoLineScanDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	// }}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_ENUM_IF_BUTTON, &CBasicDemoLineScanDlg::OnBnClickedEnumIfButton)
    ON_BN_CLICKED(IDC_ENUM_DEV_BUTTON, &CBasicDemoLineScanDlg::OnBnClickedEnumDevButton)
    ON_BN_CLICKED(IDC_OPEN_BUTTON, &CBasicDemoLineScanDlg::OnBnClickedOpenDevButton)
    ON_BN_CLICKED(IDC_CLOSE_BUTTON, &CBasicDemoLineScanDlg::OnBnClickedCloseDevButton)
    ON_BN_CLICKED(IDC_START_GRABBING_BUTTON, &CBasicDemoLineScanDlg::OnBnClickedStartGrabbingButton)
    ON_BN_CLICKED(IDC_STOP_GRABBING_BUTTON, &CBasicDemoLineScanDlg::OnBnClickedStopGrabbingButton)
    ON_BN_CLICKED(IDC_SOFTWARE_ONCE_BUTTON, &CBasicDemoLineScanDlg::OnBnClickedSoftwareOnceButton)
    ON_WM_CLOSE()
    ON_BN_CLICKED(IDC_OPEN_IF_BUTTON, &CBasicDemoLineScanDlg::OnBnClickedOpenIfButton)
    ON_BN_CLICKED(IDC_CLOSE_IF_BUTTON, &CBasicDemoLineScanDlg::OnBnClickedCloseIfButton)
    ON_MESSAGE(WM_DISPLAY_ERROR, &CBasicDemoLineScanDlg::OnDisplayError)
	ON_CBN_SELCHANGE(IDC_CAMERA_TYPE_COMBO, &CBasicDemoLineScanDlg::OnCbnSelchangeCameraTypeCombo)
	ON_EN_KILLFOCUS(IDC_IMAGE_HEIGHT_EDIT, &CBasicDemoLineScanDlg::OnEnKillfocusImageHeightEdit)
	ON_CBN_SELCHANGE(IDC_LINE_SELECTOR_COMBO, &CBasicDemoLineScanDlg::OnCbnSelchangeLineSelectorCombo)
	ON_CBN_SELCHANGE(IDC_LINE_MODE_COMBO, &CBasicDemoLineScanDlg::OnCbnSelchangeLineModeCombo)
	ON_CBN_SELCHANGE(IDC_LINE_INPUT_POLARITY_COMBO, &CBasicDemoLineScanDlg::OnCbnSelchangeLineInputPolarityCombo)
	ON_CBN_SELCHANGE(IDC_ENCODER_SELECTOR_COMBO, &CBasicDemoLineScanDlg::OnCbnSelchangeEncoderSelectorCombo)
	ON_CBN_SELCHANGE(IDC_ENCODER_SOURCE_A_COMBO, &CBasicDemoLineScanDlg::OnCbnSelchangeEncoderSourceACombo)
	ON_CBN_SELCHANGE(IDC_ENCODER_SOURCE_B_COMBO, &CBasicDemoLineScanDlg::OnCbnSelchangeEncoderSourceBCombo)
	ON_CBN_SELCHANGE(IDC_ENCODER_TRIGGER_MODE_COMBO, &CBasicDemoLineScanDlg::OnCbnSelchangeEncoderTriggerModeCombo)
	ON_CBN_SELCHANGE(IDC_CC_SELECTOR_COMBO, &CBasicDemoLineScanDlg::OnCbnSelchangeCcSelectorCombo)
	ON_CBN_SELCHANGE(IDC_CC_SOURCE_COMBO, &CBasicDemoLineScanDlg::OnCbnSelchangeCcSourceCombo)
	ON_EN_KILLFOCUS(IDC_WIDTH_EDIT, &CBasicDemoLineScanDlg::OnEnKillfocusWidthEdit)
	ON_EN_KILLFOCUS(IDC_HEIGHT_EDIT, &CBasicDemoLineScanDlg::OnEnKillfocusHeightEdit)
	ON_CBN_SELCHANGE(IDC_PIXELFORMAT_COMBO, &CBasicDemoLineScanDlg::OnCbnSelchangePixelformatCombo)
	ON_CBN_SELCHANGE(IDC_TRIGGER_SELECTOR_COMBO, &CBasicDemoLineScanDlg::OnCbnSelchangeTriggerSelectorCombo)
	ON_CBN_SELCHANGE(IDC_TRIGGER_MODE_COMB, &CBasicDemoLineScanDlg::OnCbnSelchangeTriggerModeComb)
	ON_CBN_SELCHANGE(IDC_TRIGGER_SOURCE_COMBO, &CBasicDemoLineScanDlg::OnCbnSelchangeTriggerSourceCombo)
	ON_CBN_SELCHANGE(IDC_TRIGGER_ACTIVATION_COMBO, &CBasicDemoLineScanDlg::OnCbnSelchangeTriggerActivationCombo)
	ON_CBN_SELCHANGE(IDC_SCAN_MODE_COMBO, &CBasicDemoLineScanDlg::OnCbnSelchangeScanModeCombo)
END_MESSAGE_MAP()

// ch:取流线程 | en:Grabbing thread
unsigned int __stdcall GrabThread(void* pUser)
{
    if (pUser)
    {
        CBasicDemoLineScanDlg* pCam = (CBasicDemoLineScanDlg*)pUser;

        pCam->GrabThreadProcess();

        return 0;
    }

    return -1;
}

// CBasicDemoLineScanDlg message handlers
BOOL CBasicDemoLineScanDlg::OnInitDialog()
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
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	DisplayWindowInitial();     // ch:显示框初始化 | en:Display Window Initialization

    InitializeCriticalSection(&m_hSaveImageMux);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CBasicDemoLineScanDlg::OnSysCommand(UINT nID, LPARAM lParam)
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
void CBasicDemoLineScanDlg::OnPaint()
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
HCURSOR CBasicDemoLineScanDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// ch:按钮使能 | en:Enable control
void CBasicDemoLineScanDlg::EnableControls(BOOL bIsCameraReady)
{
    GetDlgItem(IDC_ENUM_IF_BUTTON)->EnableWindow(m_bOpenIF ? FALSE : TRUE);
    GetDlgItem(IDC_OPEN_IF_BUTTON)->EnableWindow(m_bOpenIF ? FALSE : m_nInterfaceNum > 0 ? TRUE : FALSE);
    GetDlgItem(IDC_CLOSE_IF_BUTTON)->EnableWindow(m_bOpenDevice ? FALSE : m_bOpenIF ? TRUE : FALSE);
    GetDlgItem(IDC_IF_COMBO)->EnableWindow(m_bOpenIF ? FALSE : m_nInterfaceNum > 0 ? TRUE : FALSE);

    GetDlgItem(IDC_ENUM_DEV_BUTTON)->EnableWindow(m_bOpenDevice ? FALSE : m_bOpenIF ? TRUE : FALSE);
    GetDlgItem(IDC_OPEN_BUTTON)->EnableWindow(m_bOpenDevice ? FALSE : (bIsCameraReady ? TRUE : FALSE));
    GetDlgItem(IDC_CLOSE_BUTTON)->EnableWindow((m_bOpenDevice && bIsCameraReady) ? TRUE : FALSE);
    GetDlgItem(IDC_DEVICE_COMBO)->EnableWindow(m_bOpenDevice ? FALSE : (bIsCameraReady ? TRUE : FALSE));

    GetDlgItem(IDC_START_GRABBING_BUTTON)->EnableWindow(m_bStartGrabbing ? FALSE : (m_bOpenDevice && bIsCameraReady) ? TRUE : FALSE);
    GetDlgItem(IDC_STOP_GRABBING_BUTTON)->EnableWindow(m_bStartGrabbing ? TRUE : FALSE);

	EnableIFParamsControls();
	EnableDevParamsControls();
}

void CBasicDemoLineScanDlg::EnableIFParamsControls()
{
	GetDlgItem(IDC_CAMERA_TYPE_COMBO)->EnableWindow(m_bOpenIF);

	if (m_bOpenIF)
	{
		CString cstrCusValue;
		m_ctrlCameraTypeCombo.GetLBText(m_ctrlCameraTypeCombo.GetCurSel(), cstrCusValue);
		GetDlgItem(IDC_IMAGE_HEIGHT_EDIT)->EnableWindow(cstrCusValue == "LineScan" && !m_bStartGrabbing);
	}
	else
	{
		GetDlgItem(IDC_IMAGE_HEIGHT_EDIT)->EnableWindow(FALSE);
	}


	GetDlgItem(IDC_LINE_SELECTOR_COMBO)->EnableWindow(m_bOpenIF);
	GetDlgItem(IDC_LINE_MODE_COMBO)->EnableWindow(m_bOpenIF);

	if (m_bOpenIF && m_bLineInputPolarity)
	{
		CString cstrCusValue;
		m_ctrlLineModeCombo.GetLBText(m_ctrlLineModeCombo.GetCurSel(), cstrCusValue);
		GetDlgItem(IDC_LINE_INPUT_POLARITY_COMBO)->EnableWindow(cstrCusValue == "Input");
	}
	else
	{
		GetDlgItem(IDC_LINE_INPUT_POLARITY_COMBO)->EnableWindow(FALSE);
	}


	GetDlgItem(IDC_ENCODER_SELECTOR_COMBO)->EnableWindow(m_bOpenIF);
	GetDlgItem(IDC_ENCODER_SOURCE_A_COMBO)->EnableWindow(m_bOpenIF);
	GetDlgItem(IDC_ENCODER_SOURCE_B_COMBO)->EnableWindow(m_bOpenIF);
	GetDlgItem(IDC_ENCODER_TRIGGER_MODE_COMBO)->EnableWindow(m_bOpenIF);
	GetDlgItem(IDC_CC_SELECTOR_COMBO)->EnableWindow(m_bOpenIF && m_bCCSelector);
	GetDlgItem(IDC_CC_SOURCE_COMBO)->EnableWindow(m_bOpenIF && m_bCCSource);
}

void CBasicDemoLineScanDlg::EnableDevParamsControls()
{
	GetDlgItem(IDC_WIDTH_EDIT)->EnableWindow(m_bOpenDevice && !m_bStartGrabbing);
	if (m_bOpenDevice)
	{
		if (!m_bScanMode)
		{
			GetDlgItem(IDC_HEIGHT_EDIT)->EnableWindow(!m_bStartGrabbing);
		}
		else
		{
			CString cstrCusValue;
			m_ctrlScanModeCombo.GetLBText(m_ctrlScanModeCombo.GetCurSel(), cstrCusValue);
			GetDlgItem(IDC_HEIGHT_EDIT)->EnableWindow(cstrCusValue == "FrameScan" && !m_bStartGrabbing);
		}
	}
	else
	{
		GetDlgItem(IDC_HEIGHT_EDIT)->EnableWindow(FALSE);
	}
	
	GetDlgItem(IDC_PIXELFORMAT_COMBO)->EnableWindow(m_bOpenDevice && !m_bStartGrabbing);
	GetDlgItem(IDC_SCAN_MODE_COMBO)->EnableWindow(m_bOpenDevice && !m_bStartGrabbing && m_bScanMode);
	GetDlgItem(IDC_TRIGGER_SELECTOR_COMBO)->EnableWindow(m_bOpenDevice);
	GetDlgItem(IDC_TRIGGER_MODE_COMB)->EnableWindow(m_bOpenDevice);
	GetDlgItem(IDC_TRIGGER_SOURCE_COMBO)->EnableWindow(m_bOpenDevice);
	GetDlgItem(IDC_TRIGGER_ACTIVATION_COMBO)->EnableWindow(m_bOpenDevice && m_bTriggerActivation);

	if (m_bOpenDevice)
	{
		CString cstrTriggerModeValue;
		CString cstrTriggerSourceValue;
		m_ctrlTriggerModeCombo.GetLBText(m_ctrlTriggerModeCombo.GetCurSel(), cstrTriggerModeValue);
		m_ctrlTriggerSourceCombo.GetLBText(m_ctrlTriggerSourceCombo.GetCurSel(), cstrTriggerSourceValue);
		GetDlgItem(IDC_SOFTWARE_ONCE_BUTTON)->EnableWindow(cstrTriggerModeValue == "On" && cstrTriggerSourceValue == "Software");
	}
	else
	{
		GetDlgItem(IDC_SOFTWARE_ONCE_BUTTON)->EnableWindow(FALSE);
	}
}

// ch:最开始时的窗口初始化 | en:Initial window initialization
void CBasicDemoLineScanDlg::DisplayWindowInitial()
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
void CBasicDemoLineScanDlg::ShowErrorMsg(CString csMessage, int nErrorNum)
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
    case MV_FG_SUCCESS:                 errorMsg += "";                                                     break;
    case MV_FG_ERR_ERROR:               errorMsg += "Unknown error ";                                       break;
    case MV_FG_ERR_NOT_INITIALIZED:     errorMsg += "Not initialized ";                                     break;
    case MV_FG_ERR_NOT_IMPLEMENTED:     errorMsg += "Not implemented ";                                     break;
    case MV_FG_ERR_RESOURCE_IN_USE:     errorMsg += "Resource in use ";                                     break;
    case MV_FG_ERR_ACCESS_DENIED:       errorMsg += "No permission ";                                       break;
    case MV_FG_ERR_INVALID_HANDLE:      errorMsg += "Error or invalid handle ";                             break;
    case MV_FG_ERR_INVALID_ID:          errorMsg += "Error or invalid ID ";                                 break;
    case MV_FG_ERR_NO_DATA:             errorMsg += "No data ";                                             break;
    case MV_FG_ERR_INVALID_PARAMETER:   errorMsg += "Invalid parameter ";                                   break;
    case MV_FG_ERR_IO:                  errorMsg += "Input or output error ";                               break;
    case MV_FG_ERR_TIMEOUT:             errorMsg += "Timeout ";                                             break;
    case MV_FG_ERR_ABORT:               errorMsg += "Operation was interrupted ";                           break;
    case MV_FG_ERR_INVALID_BUFFER:      errorMsg += "Invalid buffer ";                                      break;
    case MV_FG_ERR_NOT_AVAILABLE:       errorMsg += "Unreachable ";                                         break;
    case MV_FG_ERR_INVALID_ADDRESS:     errorMsg += "Invalid address ";                                     break;
    case MV_FG_ERR_BUFFER_TOO_SMALL:    errorMsg += "Insufficient memory ";                                 break;
    case MV_FG_ERR_INVALID_INDEX:       errorMsg += "Invalid Index ";                                       break;
    case MV_FG_ERR_PARSING_CHUNK_DATA:  errorMsg += "Parsing chunk data failed ";                           break;
    case MV_FG_ERR_INVALID_VALUE:       errorMsg += "Invalid value ";                                       break;
    case MV_FG_ERR_RESOURCE_EXHAUSTED:  errorMsg += "Resource exhaustion ";                                 break;
    case MV_FG_ERR_OUT_OF_MEMORY:       errorMsg += "Out of memory ";                                       break;
    case MV_FG_ERR_BUSY:                errorMsg += "Device is busy, or network disconnected ";             break;
    case MV_FG_ERR_LOADLIBRARY:         errorMsg += "Load library failed ";                                 break;
    case MV_FG_ERR_GC_GENERIC:          errorMsg += "General error ";                                       break;
    case MV_FG_ERR_GC_ARGUMENT:         errorMsg += "Illegal parameters ";                                  break;
    case MV_FG_ERR_GC_RANGE:            errorMsg += "Value is out of range ";                               break;
    case MV_FG_ERR_GC_PROPERTY:         errorMsg += "Property error ";                                      break;
    case MV_FG_ERR_GC_RUNTIME:          errorMsg += "Running environment error ";                           break;
    case MV_FG_ERR_GC_LOGICAL:          errorMsg += "Logical error ";                                       break;
    case MV_FG_ERR_GC_ACCESS:           errorMsg += "Accessing condition error ";                           break;
    case MV_FG_ERR_GC_TIMEOUT:          errorMsg += "Timeout ";                                             break;
    case MV_FG_ERR_GC_DYNAMICCAST:      errorMsg += "Transformation exception ";                            break;
    case MV_FG_ERR_GC_UNKNOW:           errorMsg += "Unknown error ";                                       break;
    case MV_FG_ERR_IMG_HANDLE:          errorMsg += "Handle error ";                                        break;
    case MV_FG_ERR_IMG_SUPPORT:         errorMsg += "Not supported ";                                       break;
    case MV_FG_ERR_IMG_PARAMETER:       errorMsg += "Parameter error ";                                     break;
    case MV_FG_ERR_IMG_OVERFLOW:        errorMsg += "Out of memory ";                                       break;
    case MV_FG_ERR_IMG_INITIALIZED:     errorMsg += "Uninitialized ";                                       break;
    case MV_FG_ERR_IMG_RESOURCE:        errorMsg += "Resource release failed ";                             break;
    case MV_FG_ERR_IMG_ENCRYPT:         errorMsg += "Image encryption failed ";                             break;
    case MV_FG_ERR_IMG_FORMAT:          errorMsg += "Invalid or unsupport image format ";                   break;
    case MV_FG_ERR_IMG_SIZE:            errorMsg += "Invalid or out of range with image size ";             break;
    case MV_FG_ERR_IMG_STEP:            errorMsg += "Image size doesn't match the step param ";             break;
    case MV_FG_ERR_IMG_DATA_NULL:       errorMsg += "Image data storage address is empty ";                 break;
    case MV_FG_ERR_IMG_ABILITY_ARG:     errorMsg += "Invalid parameter for algorithm ability ";             break;
    case MV_FG_ERR_IMG_UNKNOW:          errorMsg += "Unknown error ";                                       break;
    default:                            errorMsg += "Undefined Error ";                                     break;
    }

    MessageBox(errorMsg, TEXT("PROMPT"), MB_OK | MB_ICONWARNING);
}

// ch:枚举Interface | en:Enum Interface
void CBasicDemoLineScanDlg::OnBnClickedEnumIfButton()
{
    int nRet = 0;

    // ch:枚举采集卡 | en:Enum interface
    bool bChanged = false;
    nRet = MV_FG_UpdateInterfaceList(MV_FG_CXP_INTERFACE | MV_FG_CAMERALINK_INTERFACE | MV_FG_XoF_INTERFACE, &bChanged);
    if (MV_FG_SUCCESS != nRet)
    {
        ShowErrorMsg(_TEXT("Enum Interfacs failed"), nRet);
        return;
    }
    m_nInterfaceNum = 0;

    // ch:获取采集卡数量 | en:Get interface num
    nRet = MV_FG_GetNumInterfaces(&m_nInterfaceNum);
    if (MV_FG_SUCCESS != nRet)
    {
        ShowErrorMsg(_TEXT("Get Interfacs number failed"), nRet);
        return;
    }
    if (0 == m_nInterfaceNum)
    {
        ShowErrorMsg(_TEXT("No Interface"), 0);
        return;
    }

    if (bChanged)
    {
        m_ctrlInterfaceCombo.ResetContent();

        // ch:向下拉框添加采集卡信息 | en:Add interface info in Combo
        const int nIFInfoLen = 256;
        for (unsigned int i = 0; i < m_nInterfaceNum; i++)
        {
            char                    strIFInfo[nIFInfoLen] = { 0 };
            MV_FG_INTERFACE_INFO    stInterfaceInfo = { 0 };

            nRet = MV_FG_GetInterfaceInfo(i, &stInterfaceInfo);
            if (MV_FG_SUCCESS != nRet)
            {
                ShowErrorMsg(_TEXT("Get Interfacs info failed"), nRet);
                m_ctrlInterfaceCombo.ResetContent();
                return;
            }

            switch (stInterfaceInfo.nTLayerType)
            {
            case MV_FG_CXP_INTERFACE:
                {
                    sprintf_s(strIFInfo, nIFInfoLen, "CXP[%d]: %s | %s | %s", i, 
                        stInterfaceInfo.IfaceInfo.stCXPIfaceInfo.chDisplayName, 
                        stInterfaceInfo.IfaceInfo.stCXPIfaceInfo.chInterfaceID, 
                        stInterfaceInfo.IfaceInfo.stCXPIfaceInfo.chSerialNumber);
                    break;
                }
            // Maybe support GEV interfaces and devices in the future, so reserve these codes
            case MV_FG_GEV_INTERFACE:
                {
                    sprintf_s(strIFInfo, nIFInfoLen, "GEV[%d]: %s | %s | %s", i, 
                        stInterfaceInfo.IfaceInfo.stGEVIfaceInfo.chDisplayName, 
                        stInterfaceInfo.IfaceInfo.stGEVIfaceInfo.chInterfaceID, 
                        stInterfaceInfo.IfaceInfo.stGEVIfaceInfo.chSerialNumber);
                    break;
                }
            case MV_FG_CAMERALINK_INTERFACE:
                {
                    sprintf_s(strIFInfo, nIFInfoLen, "CML[%d]: %s | %s | %s", i, 
                        stInterfaceInfo.IfaceInfo.stCMLIfaceInfo.chDisplayName, 
                        stInterfaceInfo.IfaceInfo.stCMLIfaceInfo.chInterfaceID, 
                        stInterfaceInfo.IfaceInfo.stCMLIfaceInfo.chSerialNumber);
                    break;
                }
            case MV_FG_XoF_INTERFACE:
                {
                    sprintf_s(strIFInfo, nIFInfoLen, "XoF[%d]: %s | %s | %s", i, 
                        stInterfaceInfo.IfaceInfo.stXoFIfaceInfo.chDisplayName, 
                        stInterfaceInfo.IfaceInfo.stXoFIfaceInfo.chInterfaceID, 
                        stInterfaceInfo.IfaceInfo.stXoFIfaceInfo.chSerialNumber);
                    break;
                }
            default:
                {
                    sprintf_s(strIFInfo, nIFInfoLen, "Unknown interface type[%d]", i);
                    break;
                }
            }
            m_ctrlInterfaceCombo.AddString((CString)strIFInfo);
        }
    }

    if (m_nInterfaceNum > 0)
    {
        m_ctrlInterfaceCombo.SetCurSel(0);
    }

    EnableControls(FALSE);
}

// ch:枚举设备 | en:Enum Device
void CBasicDemoLineScanDlg::OnBnClickedEnumDevButton()
{
    int          nRet = 0;
    unsigned int nDeviceNum = 0;

    // ch:枚举采集卡上的相机 | en:Enum camera of interface
    bool bChanged = false;
    nRet = MV_FG_UpdateDeviceList(m_hInterface, &bChanged);
    if (MV_FG_SUCCESS != nRet)
    {
        ShowErrorMsg(_TEXT("Enum devices failed"), nRet);
        return;
    }

    // ch:获取设备数量 | en:Get device number
    nRet = MV_FG_GetNumDevices(m_hInterface, &nDeviceNum);
    if (MV_FG_SUCCESS != nRet)
    {
        ShowErrorMsg(_TEXT("Get device number failed"), nRet);
        return;
    }
    if (0 == nDeviceNum)
    {
        ShowErrorMsg(_TEXT("No Device"), 0);
        return;
    }

    if (bChanged)
    {
        m_ctrlDeviceCombo.ResetContent();

        // ch:向下拉框添加设备信息 | en:Add device info in Combo
        const int nDevInfoLen = 256;
        for (unsigned int i = 0; i < nDeviceNum; i++)
        {
            char                strDevInfo[nDevInfoLen] = { 0 };
            MV_FG_DEVICE_INFO   stDeviceInfo = { 0 };

            nRet = MV_FG_GetDeviceInfo(m_hInterface, i, &stDeviceInfo);
            if (MV_FG_SUCCESS != nRet)
            {
                ShowErrorMsg(_TEXT("Get Devices info failed"), nRet);
                m_ctrlDeviceCombo.ResetContent();
                return;
            }

            switch (stDeviceInfo.nDevType)
            {
            case MV_FG_CXP_DEVICE:
                {
                    sprintf_s(strDevInfo, nDevInfoLen, "CXP[%d]: %s | %s | %s", i, 
                        stDeviceInfo.DevInfo.stCXPDevInfo.chUserDefinedName,
                        stDeviceInfo.DevInfo.stCXPDevInfo.chModelName,
                        stDeviceInfo.DevInfo.stCXPDevInfo.chSerialNumber);
                    break;
                }
            // Maybe support GEV interfaces and devices in the future, so reserve these codes
            case MV_FG_GEV_DEVICE:
                {
                    sprintf_s(strDevInfo, nDevInfoLen, "GEV[%d]: %s | %s | %s", i, 
                        stDeviceInfo.DevInfo.stGEVDevInfo.chUserDefinedName,
                        stDeviceInfo.DevInfo.stGEVDevInfo.chModelName,
                        stDeviceInfo.DevInfo.stGEVDevInfo.chSerialNumber);
                    break;
                }
            case MV_FG_CAMERALINK_DEVICE:
                {
                    sprintf_s(strDevInfo, nDevInfoLen, "CML[%d]: %s | %s | %s", i, 
                        stDeviceInfo.DevInfo.stCMLDevInfo.chUserDefinedName,
                        stDeviceInfo.DevInfo.stCMLDevInfo.chModelName,
                        stDeviceInfo.DevInfo.stCMLDevInfo.chSerialNumber);
                    break;
                }
            case MV_FG_XoF_DEVICE:
                {
                    sprintf_s(strDevInfo, nDevInfoLen, "XoF[%d]: %s | %s | %s", i, 
                        stDeviceInfo.DevInfo.stXoFDevInfo.chUserDefinedName,
                        stDeviceInfo.DevInfo.stXoFDevInfo.chModelName,
                        stDeviceInfo.DevInfo.stXoFDevInfo.chSerialNumber);
                    break;
                }
            default:
                {
                    sprintf_s(strDevInfo, nDevInfoLen, "Unknown device type[%d]", i);
                    break;
                }
            }
            m_ctrlDeviceCombo.AddString((CString)strDevInfo);
        }
    }

    if (nDeviceNum > 0)
    {
        m_ctrlDeviceCombo.SetCurSel(0);
    }

    EnableControls(TRUE);
}

// ch:关闭设备 | en:Close Device
int CBasicDemoLineScanDlg::CloseDevice()
{
    if (m_bThreadState)
    {
        m_bThreadState = FALSE;
        if (m_hGrabThread)
        {
            WaitForSingleObject(m_hGrabThread, INFINITE);
            CloseHandle(m_hGrabThread);
            m_hGrabThread = NULL;
        }
    }

    int nRet = 0;

    if (m_bStartGrabbing)
    {
        // ch:停止取流 | en:Stop Acquisition
        nRet = MV_FG_StopAcquisition(m_hStream);
        if (MV_FG_SUCCESS != nRet)
        {
            ShowErrorMsg(TEXT("Stop acquistion failed"), nRet);
        }
        m_bStartGrabbing = FALSE;
    }

    if (m_hStream)
    {
        nRet = MV_FG_CloseStream(m_hStream);
        if (MV_FG_SUCCESS != nRet)
        {
            ShowErrorMsg(TEXT("Close stream Failed"), nRet);
        }
        m_hStream = NULL;
    }

    if (m_hDevice)
    {
        nRet = MV_FG_CloseDevice(m_hDevice);
        if (MV_FG_SUCCESS != nRet)
        {
            ShowErrorMsg(TEXT("Close device Failed"), nRet);
        }
        m_hDevice = NULL;
    }

    m_bOpenDevice = FALSE;

    if (m_pDataBuf)
    {
        free(m_pDataBuf);
        m_pDataBuf = NULL;
    }
    m_nDataBufSize = 0;

    if (m_pSaveImageBuf)
    {
        free(m_pSaveImageBuf);
        m_pSaveImageBuf = NULL;
    }
    m_nSaveImageBufSize = 0;

    return nRet;
}

int CBasicDemoLineScanDlg::GrabThreadProcess()
{
    MV_FG_BUFFER_INFO           stFrameInfo = { 0 };    // ch:图像信息 | en:Frame info
    MV_FG_DISPLAY_FRAME_INFO    stDisplayInfo = { 0 };  // ch:显示的图像信息 | en:Display frame info
    MV_FG_HB_DECODE_PARAM       stHBDecodeInfo = { 0 }; // ch:HB解码图像信息 | en:HB decodes image info
    const unsigned int          nTimeout = 1000;
    int                         nRet = 0;

    while(m_bThreadState)
    {
        if (m_bStartGrabbing)
        {
            // ch:获取一帧图像缓存信息 | en:Get one frame buffer's info
            nRet = MV_FG_GetFrameBuffer(m_hStream, &stFrameInfo, nTimeout);
            if (MV_FG_SUCCESS == nRet)
            {
                // 用于保存图片
                EnterCriticalSection(&m_hSaveImageMux);
                memset(&m_stImageInfo, 0, sizeof(MV_FG_INPUT_IMAGE_INFO));
                if (NULL == m_pDataBuf || stFrameInfo.nFilledSize > m_nDataBufSize)
                {
                    if (m_pDataBuf)
                    {
                        free(m_pDataBuf);
                        m_pDataBuf = NULL;
                    }

                    m_pDataBuf = (unsigned char*)malloc(sizeof(unsigned char) * stFrameInfo.nFilledSize);
                    if (NULL == m_pDataBuf)
                    {
                        LeaveCriticalSection(&m_hSaveImageMux);
                        return MV_FG_ERR_OUT_OF_MEMORY;
                    }
                    m_nDataBufSize = stFrameInfo.nFilledSize;
                }
                memset(m_pDataBuf, 0, m_nDataBufSize);
                memcpy(m_pDataBuf, stFrameInfo.pBuffer, stFrameInfo.nFilledSize);
                m_stImageInfo.nWidth        = stFrameInfo.nWidth;
                m_stImageInfo.nHeight       = stFrameInfo.nHeight;
                m_stImageInfo.enPixelType   = stFrameInfo.enPixelType;
                m_stImageInfo.pImageBuf     = m_pDataBuf;
                m_stImageInfo.nImageBufLen  = stFrameInfo.nFilledSize;
                LeaveCriticalSection(&m_hSaveImageMux);

                // 自定义格式不支持显示
                if (RemoveCustomPixelFormats(stFrameInfo.enPixelType))
                {
                    MV_FG_ReleaseFrameBuffer(m_hStream, &stFrameInfo);
                    continue;
                }

                // 配置显示图像的参数
                memset(&stDisplayInfo, 0, sizeof(MV_FG_DISPLAY_FRAME_INFO));
                stDisplayInfo.nWidth        = stFrameInfo.nWidth;
                stDisplayInfo.nHeight       = stFrameInfo.nHeight;
                stDisplayInfo.enPixelType   = stFrameInfo.enPixelType;
                stDisplayInfo.pImageBuf     = (unsigned char*)stFrameInfo.pBuffer;
                stDisplayInfo.nImageBufLen  = stFrameInfo.nFilledSize;
                nRet = MV_FG_DisplayOneFrame(m_hStream, m_hwndDisplay, &stDisplayInfo);
                if (MV_FG_SUCCESS != nRet)
                {
                    MV_FG_ReleaseFrameBuffer(m_hStream, &stFrameInfo);
                    ::PostMessage(this->m_hWnd, WM_DISPLAY_ERROR, (WPARAM)nRet, 0);
                    break;
                }

                MV_FG_ReleaseFrameBuffer(m_hStream, &stFrameInfo);
                
            }
        }
        else
        {
            Sleep(5);       // ch:还未开始取流，调用Sleep() | en:Acquisition not start, call Sleep()
        }
    }

    return MV_FG_SUCCESS;
}

// ch:按下打开设备按钮：打开设备 | en:Click Open button: Open Device
void CBasicDemoLineScanDlg::OnBnClickedOpenDevButton()
{
    if (TRUE == m_bOpenDevice)
    {
        return;
    }
    UpdateData(TRUE);

    int nIndex = m_nDeviceCombo;
    if ((nIndex < 0) || nIndex >= m_ctrlDeviceCombo.GetCount())
    {
        ShowErrorMsg(TEXT("Please select device"), 0);
        return;
    }

    int nRet = 0;

    // ch:打开设备，获得设备句柄 | en:Open device, get handle
    nRet = MV_FG_OpenDevice(m_hInterface, (unsigned int)nIndex, &m_hDevice);
    if (MV_FG_SUCCESS != nRet)
    {
        ShowErrorMsg(TEXT("Open device failed"), nRet);
        return;
    }

    m_bOpenDevice = TRUE;

	UpdateData(FALSE);

	GetDeviceParams();

	EnableControls(TRUE);

}

// ch:按下关闭设备按钮：关闭设备 | en:Click Close button: Close Device
void CBasicDemoLineScanDlg::OnBnClickedCloseDevButton()
{
    CloseDevice();
    memset(&m_stEnumImageCompressionModeValue, 0, sizeof(MV_FG_ENUMVALUE));
    m_bHBMode = FALSE;

    EnableControls(TRUE);
}

// ch:按下开始采集按钮 | en:Click Start button
void CBasicDemoLineScanDlg::OnBnClickedStartGrabbingButton()
{
    if (FALSE == m_bOpenDevice || TRUE == m_bStartGrabbing)
    {
        return;
    }

    int nRet = 0;

    // ch:获取流通道个数 | en:Get number of stream
    unsigned int nStreamNum = 0;
    nRet = MV_FG_GetNumStreams(m_hDevice, &nStreamNum);
    if (MV_FG_SUCCESS != nRet || 0 == nStreamNum)
    {
        ShowErrorMsg(TEXT("Get stream number failed"), nRet);
        CloseDevice();
        return;
    }

    // ch:打开流通道(目前只支持单个通道) | en:Open stream(Only a single stream is supported now)
    nRet = MV_FG_OpenStream(m_hDevice, 0, &m_hStream);
    if (MV_FG_SUCCESS != nRet)
    {
        ShowErrorMsg(TEXT("Open stream failed"), nRet);
        CloseDevice();
        return;
    }

    // ch:设置SDK内部缓存数量 | en:Set internal buffer number
    const unsigned int nBufferNum = 3;
    nRet = MV_FG_SetBufferNum(m_hStream, nBufferNum);
    if (MV_FG_SUCCESS != nRet)
    {
        ShowErrorMsg(TEXT("Set buffer number failed"), nRet);
        return;
    }

    m_bThreadState = TRUE;
    unsigned int nThreadID = 0;
    m_hGrabThread = (void*)_beginthreadex( NULL , 0 , GrabThread , this, 0 , &nThreadID );
    if (NULL == m_hGrabThread)
    {
        m_bThreadState = FALSE;
        ShowErrorMsg(TEXT("Create thread failed"), 0);
        return;
    }

    // ch:开始取流 | en:Start Acquisition
    nRet = MV_FG_StartAcquisition(m_hStream);
    if (MV_FG_SUCCESS != nRet)
    {
        m_bThreadState = FALSE;
        ShowErrorMsg(TEXT("Start acquistion failed"), nRet);
        return;
    }
    m_bStartGrabbing = TRUE;

    EnableControls(TRUE);
}

// ch:按下结束采集按钮 | en:Click Stop button
void CBasicDemoLineScanDlg::OnBnClickedStopGrabbingButton()
{
    if (FALSE == m_bOpenDevice || FALSE == m_bStartGrabbing)
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

    // ch:停止取流 | en:Stop Acquisition
    int nRet = MV_FG_StopAcquisition(m_hStream);
    if (MV_FG_SUCCESS != nRet)
    {
        ShowErrorMsg(TEXT("Stop acquistion failed"), nRet);
        return;
    }
    m_bStartGrabbing = FALSE;

    nRet = MV_FG_CloseStream(m_hStream);
    if (MV_FG_SUCCESS != nRet)
    {
        ShowErrorMsg(TEXT("Close stream Failed"), nRet);
    }
    m_hStream = NULL;

    EnableControls(TRUE);
}

// ch:按下软触发一次按钮 | en:Click Execute button
void CBasicDemoLineScanDlg::OnBnClickedSoftwareOnceButton()
{
    if (TRUE != m_bStartGrabbing)
    {
        return;
    }

    int nRet = MV_FG_SetCommandValue(m_hDevice, "TriggerSoftware");
    if (MV_FG_SUCCESS != nRet)
    {
        ShowErrorMsg(TEXT("Trigger software failed"), nRet);
    }
}

// ch:右上角退出 | en:Exit from upper right corner
void CBasicDemoLineScanDlg::OnClose()
{
    PostQuitMessage(0);
    CloseDevice();

    if (m_hInterface)
    {
        MV_FG_CloseInterface(m_hInterface);
        m_hInterface = NULL;
    }

    DeleteCriticalSection(&m_hSaveImageMux);

    CDialog::OnClose();
}

// ch:显示错误消息函数 | en:Display error message function
LRESULT CBasicDemoLineScanDlg::OnDisplayError(WPARAM wParam, LPARAM lParam)
{
    ShowErrorMsg(TEXT("Display failed"), (int)wParam);

    OnBnClickedStopGrabbingButton();

    return 0;
}

BOOL CBasicDemoLineScanDlg::PreTranslateMessage(MSG* pMsg)
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

bool CBasicDemoLineScanDlg::RemoveCustomPixelFormats(MV_FG_PIXEL_TYPE enPixelFormat)
{
    int nResult = enPixelFormat & MV_FG_PIXEL_CUSTOM;
    if(MV_FG_PIXEL_CUSTOM == nResult)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void CBasicDemoLineScanDlg::OnBnClickedOpenIfButton()
{
    int nInterfaceIndex =  m_ctrlInterfaceCombo.GetCurSel();
    if (nInterfaceIndex < 0 || nInterfaceIndex >= m_ctrlInterfaceCombo.GetCount())
    {
        ShowErrorMsg(TEXT("Please select valid interface"), 0);
        return;
    }

    // ch:打开采集卡，获得采集卡句柄 | en:Open interface, get handle
    int nRet = MV_FG_OpenInterface((unsigned int)nInterfaceIndex, &m_hInterface);
    if (MV_FG_SUCCESS != nRet)
    {
        ShowErrorMsg(_TEXT("Open interface failed"), nRet);
        return;
    }

    m_bOpenIF = TRUE;

	GetInterfaceParams();
	EnableControls(FALSE);

}

void CBasicDemoLineScanDlg::OnBnClickedCloseIfButton()
{
    int nRet = 0;

    m_bOpenIF = FALSE;
    CloseDevice();

    // ch:关闭采集卡 | en:Close interface
    if (m_hInterface)
    {
        nRet = MV_FG_CloseInterface(m_hInterface);
        if (MV_FG_SUCCESS != nRet)
        {
            ShowErrorMsg(_TEXT("Close interface failed"), nRet);
        }
        m_hInterface = NULL;
    }

    EnableControls(FALSE);
}

int CBasicDemoLineScanDlg::SaveImage(SAVE_IAMGE_TYPE enSaveImageType)
{
    EnterCriticalSection(&m_hSaveImageMux);

    if (NULL == m_pDataBuf)
    {
        LeaveCriticalSection(&m_hSaveImageMux);
        return MV_FG_ERR_NO_DATA;
    }

    if (RemoveCustomPixelFormats(m_stImageInfo.enPixelType))
    {
        LeaveCriticalSection(&m_hSaveImageMux);
        return MV_FG_ERR_INVALID_VALUE;
    }

    int             nRet = 0;
    unsigned int    nMaxImageLen = m_stImageInfo.nWidth * m_stImageInfo.nHeight * 4 + 2048; // 确保存图空间足够，包括图像头

    if (NULL == m_pSaveImageBuf || nMaxImageLen > m_nSaveImageBufSize)
    {
        if (m_pSaveImageBuf)
        {
            free(m_pSaveImageBuf);
            m_pSaveImageBuf = NULL;
        }

        m_pSaveImageBuf = (unsigned char*)malloc(sizeof(unsigned char) * nMaxImageLen);
        if (NULL == m_pSaveImageBuf)
        {
            LeaveCriticalSection(&m_hSaveImageMux);
            return MV_FG_ERR_OUT_OF_MEMORY;
        }
        m_nSaveImageBufSize = nMaxImageLen;
    }
    memset(m_pSaveImageBuf, 0, nMaxImageLen);

    char szFileName[FILE_NAME_LEN] = { 0 };
    SYSTEMTIME stSysTime = { 0 };
    GetLocalTime(&stSysTime);

    do
    {
        if (Image_Bmp == enSaveImageType)
        {
            MV_FG_SAVE_BITMAP_INFO stBmpInfo = { 0 };

            stBmpInfo.stInputImageInfo   = m_stImageInfo;
            stBmpInfo.pBmpBuf            = m_pSaveImageBuf;
            stBmpInfo.nBmpBufSize        = m_nSaveImageBufSize;
            stBmpInfo.enCfaMethod        = MV_FG_CFA_METHOD_OPTIMAL;

            // ch:保存BMP图像 | en:Save to BMP
            nRet = MV_FG_SaveBitmap(m_hStream, &stBmpInfo);
            if (MV_FG_SUCCESS != nRet)
            {
                break;
            }

            sprintf_s(szFileName, FILE_NAME_LEN, "Image_w%d_h%d_%02d%02d%04d.bmp", 
                m_stImageInfo.nWidth, m_stImageInfo.nHeight, stSysTime.wMinute, stSysTime.wSecond, stSysTime.wMilliseconds);

            FILE* pImgFile = NULL;
            if ((0 != fopen_s(&pImgFile, szFileName, "wb")))
            {
                nRet = MV_FG_ERR_INVALID_PARAMETER;
                break;
            }

            fwrite(stBmpInfo.pBmpBuf, 1, stBmpInfo.nBmpBufLen, pImgFile);
            fclose(pImgFile);
        }
        else if (Image_Jpeg == enSaveImageType)
        {
            MV_FG_SAVE_JPEG_INFO stJpgInfo = { 0 };

            stJpgInfo.stInputImageInfo  = m_stImageInfo;
            stJpgInfo.pJpgBuf           = m_pSaveImageBuf;
            stJpgInfo.nJpgBufSize       = m_nSaveImageBufSize;
            stJpgInfo.nJpgQuality       = 60;                   // JPG编码质量(0-100]
            stJpgInfo.enCfaMethod       = MV_FG_CFA_METHOD_OPTIMAL;

            // ch:保存JPEG图像 | en:Save to JPEG
            nRet = MV_FG_SaveJpeg(m_hStream, &stJpgInfo);
            if (MV_FG_SUCCESS != nRet)
            {
                break;
            }

            sprintf_s(szFileName, FILE_NAME_LEN, "Image_w%d_h%d_%02d%02d%04d.jpg", 
                m_stImageInfo.nWidth, m_stImageInfo.nHeight, stSysTime.wMinute, stSysTime.wSecond, stSysTime.wMilliseconds);

            FILE* pImgFile = NULL;
            if ((0 != fopen_s(&pImgFile, szFileName, "wb")))
            {
                nRet = MV_FG_ERR_INVALID_PARAMETER;
                break;
            }

            fwrite(stJpgInfo.pJpgBuf, 1, stJpgInfo.nJpgBufLen, pImgFile);
            fclose(pImgFile);
        }
        else if (Image_Tiff == enSaveImageType)
        {
            MV_FG_SAVE_TIFF_TO_FILE_INFO stTiffInfo ={0};

            stTiffInfo.stInputImageInfo = m_stImageInfo;
            stTiffInfo.fXResolution     = (float)m_stImageInfo.nWidth;
            stTiffInfo.fYResolution     = (float)m_stImageInfo.nHeight;
            stTiffInfo.enResolutionUnit = MV_FG_Resolution_Unit_Inch;
            stTiffInfo.enCfaMethod      = MV_FG_CFA_METHOD_OPTIMAL;

            if (NULL == stTiffInfo.pcImagePath)
            {
                stTiffInfo.pcImagePath = (char*)malloc(sizeof(char) * FILE_NAME_LEN);
                if (NULL == stTiffInfo.pcImagePath)
                {
                    nRet = MV_FG_ERR_OUT_OF_MEMORY;
                    break;
                }
            }
            sprintf_s(stTiffInfo.pcImagePath, FILE_NAME_LEN, "Image_w%d_h%d_%02d%02d%04d.tif", 
                m_stImageInfo.nWidth, m_stImageInfo.nHeight, stSysTime.wMinute, stSysTime.wSecond, stSysTime.wMilliseconds);

            // ch:保存Tiff图像 | en:Save to Tiff
            nRet = MV_FG_SaveTiffToFile(m_hStream,&stTiffInfo);
            if (NULL != stTiffInfo.pcImagePath)
            {
                free(stTiffInfo.pcImagePath);
                stTiffInfo.pcImagePath = NULL;
            }
            if (MV_FG_SUCCESS != nRet)
            {
                 break;
            }

        }
        else if (Image_Png == enSaveImageType)
        {
            MV_FG_SAVE_PNG_TO_FILE_INFO stPngInfo = {0};
            
            stPngInfo.stInputImageInfo = m_stImageInfo;
            stPngInfo.nPngCompression  = 6;
            stPngInfo.enCfaMethod      = MV_FG_CFA_METHOD_OPTIMAL;

            if (NULL == stPngInfo.pcImagePath)
            {
                stPngInfo.pcImagePath = (char*)malloc(sizeof(char) * FILE_NAME_LEN);
                if (NULL == stPngInfo.pcImagePath)
                {
                    nRet = MV_FG_ERR_OUT_OF_MEMORY;
                    break;
                }
            }
            sprintf_s(stPngInfo.pcImagePath, FILE_NAME_LEN, "Image_w%d_h%d_%02d%02d%04d.png", 
                m_stImageInfo.nWidth, m_stImageInfo.nHeight, stSysTime.wMinute, stSysTime.wSecond, stSysTime.wMilliseconds);

            // ch:保存Png图像 | en:Save to Png
            nRet = MV_FG_SavePngToFile(m_hStream,&stPngInfo);
            if (NULL != stPngInfo.pcImagePath)
            {
                free(stPngInfo.pcImagePath);
                stPngInfo.pcImagePath = NULL;
            }
            if (MV_FG_SUCCESS != nRet)
            {
                break;
            }


        }
        else
        {
            nRet =  MV_FG_ERR_INVALID_PARAMETER;
            break;
        }
    } while (0);

    LeaveCriticalSection(&m_hSaveImageMux);
    return nRet;
}

void CBasicDemoLineScanDlg::OnBnClickedBmpSaveButton()
{
    int nRet = SaveImage(Image_Bmp);
    if (MV_FG_SUCCESS != nRet)
    {
        ShowErrorMsg(TEXT("Save bmp failed"), nRet);
        return;
    }
    ShowErrorMsg(TEXT("Save bmp succeed"), 0);
}

void CBasicDemoLineScanDlg::OnBnClickedJpegSaveButton()
{
    int nRet = SaveImage(Image_Jpeg);
    if (MV_FG_SUCCESS != nRet)
    {
        ShowErrorMsg(TEXT("Save jpg failed"), nRet);
        return;
    }
    ShowErrorMsg(TEXT("Save jpg succeed"), 0);
}



void CBasicDemoLineScanDlg::OnBnClickedTiffSaveButton()
{
	int nRet = SaveImage(Image_Tiff);
	if (MV_FG_SUCCESS != nRet)
	{
		ShowErrorMsg(TEXT("Save tiff failed"), nRet);
		return;
	}
	ShowErrorMsg(TEXT("Save tiff succeed"), 0);
}

void CBasicDemoLineScanDlg::OnBnClickedPngSaveButton()
{
	int nRet = SaveImage(Image_Png);
	if (MV_FG_SUCCESS != nRet)
	{
		ShowErrorMsg(TEXT("Save png failed"), nRet);
		return;
	}
	ShowErrorMsg(TEXT("Save png succeed"), 0);
}

// ch:设置图像压缩模式 | en:Set Image Compression Mode
int CBasicDemoLineScanDlg::SetImageCompressionMode()
{
    int nRet = 0;

    if (FALSE == m_ctrlImageCompressionModeCombo.IsWindowEnabled())
    {
        return MV_FG_ERR_ACCESS_DENIED;
    }

    int nIndex = m_ctrlImageCompressionModeCombo.GetCurSel();
    CString strCBText;
    m_ctrlImageCompressionModeCombo.GetLBText(nIndex, strCBText);

    for (unsigned int i = 0; i < m_stEnumImageCompressionModeValue.nSupportedNum; i++)
    {
        if (strCBText == m_stEnumImageCompressionModeValue.strSymbolic[i])
        {
            nRet = MV_FG_SetEnumValue(m_hDevice, "ImageCompressionMode", m_stEnumImageCompressionModeValue.nSupportValue[i]);
            break;
        }
    }

    return nRet;

}

// ch:获取采集卡参数 | en:Get interfaces parameters
int CBasicDemoLineScanDlg::GetInterfaceParams()
{
	if (NULL == m_hInterface)
	{
		return MV_FG_ERR_NOT_INITIALIZED;
	}

	MV_FG_ENUMVALUE stEnumValue = {0};
	MV_FG_INTVALUE  stIntValue = {0};
	int nRet = MV_FG_SUCCESS;

	// ch:获取CameraType | en:Get CameraType
	m_ctrlCameraTypeCombo.ResetContent();
	memset(&stEnumValue, 0, sizeof(MV_FG_ENUMVALUE));
	nRet = MV_FG_GetEnumValue(m_hInterface, "CameraType", &stEnumValue);
	if (MV_FG_SUCCESS == nRet)
	{
		unsigned int nIndex = 0;
		for (unsigned int i = 0; i < stEnumValue.nSupportedNum; i++)
		{
			m_ctrlCameraTypeCombo.AddString((CString)stEnumValue.strSymbolic[i]);
			if (stEnumValue.nSupportValue[i] == stEnumValue.nCurValue)
			{
				nIndex = i;
			}
		}

		m_ctrlCameraTypeCombo.SetCurSel(nIndex);
	}

	// ch:获取Image Height | en:Get Image Height
	memset(&stIntValue, 0, sizeof(stIntValue));
	nRet = MV_FG_GetIntValue(m_hInterface, "ImageHeight", &stIntValue);
	if (MV_FG_SUCCESS == nRet)
	{
		m_nImageHeightEdit = stIntValue.nCurValue;
	}

	// ch:获取LineSelector | en:Get LineSelector
	m_ctrlLineSelectorCombo.ResetContent();
	memset(&stEnumValue, 0, sizeof(MV_FG_ENUMVALUE));
	nRet = MV_FG_GetEnumValue(m_hInterface, "LineSelector", &stEnumValue);
	if (MV_FG_SUCCESS == nRet)
	{
		unsigned int nIndex = 0;
		for (unsigned int i = 0; i < stEnumValue.nSupportedNum; i++)
		{
			m_ctrlLineSelectorCombo.AddString((CString)stEnumValue.strSymbolic[i]);
			if (stEnumValue.nSupportValue[i] == stEnumValue.nCurValue)
			{
				nIndex = i;
			}
		}

		m_ctrlLineSelectorCombo.SetCurSel(nIndex);
	}

	// ch:获取LineMode | en:Get LineMode
	m_ctrlLineModeCombo.ResetContent();
	memset(&stEnumValue, 0, sizeof(MV_FG_ENUMVALUE));
	nRet = MV_FG_GetEnumValue(m_hInterface, "LineMode", &stEnumValue);
	if (MV_FG_SUCCESS == nRet)
	{
		unsigned int nIndex = 0;
		for (unsigned int i = 0; i < stEnumValue.nSupportedNum; i++)
		{
			m_ctrlLineModeCombo.AddString((CString)stEnumValue.strSymbolic[i]);
			if (stEnumValue.nSupportValue[i] == stEnumValue.nCurValue)
			{
				nIndex = i;
			}
		}

		m_ctrlLineModeCombo.SetCurSel(nIndex);
	}

	// ch:获取LineInputPolarity | en:Get LineInputPolarity
	m_ctrlLineInputPolarityCombo.ResetContent();
	memset(&stEnumValue, 0, sizeof(MV_FG_ENUMVALUE));
	nRet = MV_FG_GetEnumValue(m_hInterface, "LineInputPolarity", &stEnumValue);
	if (MV_FG_SUCCESS == nRet)
	{
		unsigned int nIndex = 0;
		for (unsigned int i = 0; i < stEnumValue.nSupportedNum; i++)
		{
			m_ctrlLineInputPolarityCombo.AddString((CString)stEnumValue.strSymbolic[i]);
			if (stEnumValue.nSupportValue[i] == stEnumValue.nCurValue)
			{
				nIndex = i;
			}
		}

		m_ctrlLineInputPolarityCombo.SetCurSel(nIndex);
        m_bLineInputPolarity = TRUE;
	}
    else
    {
        m_bLineInputPolarity = FALSE;
    }

	// ch:获取EncoderSelector | en:Get EncoderSelector
	m_ctrlEncoderSelectorCombo.ResetContent();
	memset(&stEnumValue, 0, sizeof(MV_FG_ENUMVALUE));
	nRet = MV_FG_GetEnumValue(m_hInterface, "EncoderSelector", &stEnumValue);
	if (MV_FG_SUCCESS == nRet)
	{
		unsigned int nIndex = 0;
		for (unsigned int i = 0; i < stEnumValue.nSupportedNum; i++)
		{
			m_ctrlEncoderSelectorCombo.AddString((CString)stEnumValue.strSymbolic[i]);
			if (stEnumValue.nSupportValue[i] == stEnumValue.nCurValue)
			{
				nIndex = i;
			}
		}

		m_ctrlEncoderSelectorCombo.SetCurSel(nIndex);
	}

	// ch:获取EncoderSourceA | en:Get EncoderSourceA
	m_ctrlEncoderSourceACombo.ResetContent();
	memset(&stEnumValue, 0, sizeof(MV_FG_ENUMVALUE));
	nRet = MV_FG_GetEnumValue(m_hInterface, "EncoderSourceA", &stEnumValue);
	if (MV_FG_SUCCESS == nRet)
	{
		unsigned int nIndex = 0;
		for (unsigned int i = 0; i < stEnumValue.nSupportedNum; i++)
		{
			m_ctrlEncoderSourceACombo.AddString((CString)stEnumValue.strSymbolic[i]);
			if (stEnumValue.nSupportValue[i] == stEnumValue.nCurValue)
			{
				nIndex = i;
			}
		}

		m_ctrlEncoderSourceACombo.SetCurSel(nIndex);
	}


	// ch:获取EncoderSourceB | en:Get EncoderSourceB
	m_ctrlEncoderSourceBCombo.ResetContent();
	memset(&stEnumValue, 0, sizeof(MV_FG_ENUMVALUE));
	nRet = MV_FG_GetEnumValue(m_hInterface, "EncoderSourceB", &stEnumValue);
	if (MV_FG_SUCCESS == nRet)
	{
		unsigned int nIndex = 0;
		for (unsigned int i = 0; i < stEnumValue.nSupportedNum; i++)
		{
			m_ctrlEncoderSourceBCombo.AddString((CString)stEnumValue.strSymbolic[i]);
			if (stEnumValue.nSupportValue[i] == stEnumValue.nCurValue)
			{
				nIndex = i;
			}
		}

		m_ctrlEncoderSourceBCombo.SetCurSel(nIndex);
	}

	// ch:获取EncoderTriggerMode | en:Get EncoderTriggerMode
	m_ctrlEncoderTriggerModeCombo.ResetContent();
	memset(&stEnumValue, 0, sizeof(MV_FG_ENUMVALUE));
	nRet = MV_FG_GetEnumValue(m_hInterface, "EncoderTriggerMode", &stEnumValue);
	if (MV_FG_SUCCESS == nRet)
	{
		unsigned int nIndex = 0;
		for (unsigned int i = 0; i < stEnumValue.nSupportedNum; i++)
		{
			m_ctrlEncoderTriggerModeCombo.AddString((CString)stEnumValue.strSymbolic[i]);
			if (stEnumValue.nSupportValue[i] == stEnumValue.nCurValue)
			{
				nIndex = i;
			}
		}

		m_ctrlEncoderTriggerModeCombo.SetCurSel(nIndex);
	}

	// ch:获取CCSelector | en:Get CCSelector
	m_ctrlCCSelectorCombo.ResetContent();
	memset(&stEnumValue, 0, sizeof(MV_FG_ENUMVALUE));
	nRet = MV_FG_GetEnumValue(m_hInterface, "CCSelector", &stEnumValue);
	if (MV_FG_SUCCESS == nRet)
	{
		unsigned int nIndex = 0;
		for (unsigned int i = 0; i < stEnumValue.nSupportedNum; i++)
		{
			m_ctrlCCSelectorCombo.AddString((CString)stEnumValue.strSymbolic[i]);
			if (stEnumValue.nSupportValue[i] == stEnumValue.nCurValue)
			{
				nIndex = i;
			}
		}

		m_ctrlCCSelectorCombo.SetCurSel(nIndex);
        m_bCCSelector = TRUE;
	}
    else
    {
        m_bCCSelector = FALSE;
    }

	// ch:获取CCSource | en:Get CCSource
	m_ctrlCCSourceCombo.ResetContent();
	memset(&stEnumValue, 0, sizeof(MV_FG_ENUMVALUE));
	nRet = MV_FG_GetEnumValue(m_hInterface, "CCSource", &stEnumValue);
	if (MV_FG_SUCCESS == nRet)
	{
		unsigned int nIndex = 0;
		for (unsigned int i = 0; i < stEnumValue.nSupportedNum; i++)
		{
			m_ctrlCCSourceCombo.AddString((CString)stEnumValue.strSymbolic[i]);
			if (stEnumValue.nSupportValue[i] == stEnumValue.nCurValue)
			{
				nIndex = i;
			}
		}

		m_ctrlCCSourceCombo.SetCurSel(nIndex);
        m_bCCSource = TRUE;
	}
    else
    {
        m_bCCSource = FALSE;
    }

	UpdateData(FALSE);

	EnableIFParamsControls();

	return nRet;
}

int CBasicDemoLineScanDlg::GetDeviceParams()
{
	int nRet = 0;

	MV_FG_INTVALUE stIntValue = {0};
	MV_FG_ENUMVALUE stEnumValue = {0};

	// ch:获取Width | en:Get Width
	memset(&stIntValue, 0, sizeof(stIntValue));
	nRet = MV_FG_GetIntValue(m_hDevice, "Width", &stIntValue);
	if (MV_FG_SUCCESS == nRet)
	{
		m_nCameraWidth = stIntValue.nCurValue;
	}

	// ch:获取Height | en:Get Height
	memset(&stIntValue, 0, sizeof(stIntValue));
	nRet = MV_FG_GetIntValue(m_hDevice, "Height", &stIntValue);
	if (MV_FG_SUCCESS == nRet)
	{
		m_nCameraHeight = stIntValue.nCurValue;
	}

	// ch:获取PixelFormat | en:Get PixelFormat
	m_ctrlPixelFormatCombo.ResetContent();
	memset(&stEnumValue, 0, sizeof(stEnumValue));
	nRet = MV_FG_GetEnumValue(m_hDevice, "PixelFormat", &stEnumValue);
	if (MV_FG_SUCCESS == nRet)
	{
		unsigned int nIndex = 0;
		for (unsigned int i = 0; i < stEnumValue.nSupportedNum; i++)
		{
			m_ctrlPixelFormatCombo.AddString((CString)stEnumValue.strSymbolic[i]);
			if (stEnumValue.nSupportValue[i] == stEnumValue.nCurValue)
			{
				nIndex = i;
			}

		}
		m_ctrlPixelFormatCombo.SetCurSel(nIndex);
	}

	// ch:获取ScanMode | en:Get ScanMode
	m_ctrlScanModeCombo.ResetContent();
	memset(&stEnumValue, 0, sizeof(stEnumValue));
	nRet = MV_FG_GetEnumValue(m_hDevice, "ScanMode", &stEnumValue);
	if (MV_FG_SUCCESS == nRet)
	{
		unsigned int nIndex = 0;
		for (unsigned int i = 0; i < stEnumValue.nSupportedNum; i++)
		{
			m_ctrlScanModeCombo.AddString((CString)stEnumValue.strSymbolic[i]);
			if (stEnumValue.nSupportValue[i] == stEnumValue.nCurValue)
			{
				nIndex = i;
			}

		}
		m_ctrlScanModeCombo.SetCurSel(nIndex);
		m_bScanMode = TRUE;
	}
	else
	{
		m_bScanMode = FALSE;
	}

	// ch:获取TriggerSelector | en:Get TriggerSelector
	m_ctrlTriggerSelectorCombo.ResetContent();
	memset(&stEnumValue, 0, sizeof(stEnumValue));
	nRet = MV_FG_GetEnumValue(m_hDevice, "TriggerSelector", &stEnumValue);
	if (MV_FG_SUCCESS == nRet)
	{
		unsigned int nIndex = 0;
		for (unsigned int i = 0; i < stEnumValue.nSupportedNum; i++)
		{
			m_ctrlTriggerSelectorCombo.AddString((CString)stEnumValue.strSymbolic[i]);
			if (stEnumValue.nSupportValue[i] == stEnumValue.nCurValue)
			{
				nIndex = i;
			}

		}
		m_ctrlTriggerSelectorCombo.SetCurSel(nIndex);
	}

	// ch:获取TriggerMode | en:Get TriggerMode
	m_ctrlTriggerModeCombo.ResetContent();
	memset(&stEnumValue, 0, sizeof(stEnumValue));
	nRet = MV_FG_GetEnumValue(m_hDevice, "TriggerMode", &stEnumValue);
	if (MV_FG_SUCCESS == nRet)
	{
		unsigned int nIndex = 0;
		for (unsigned int i = 0; i < stEnumValue.nSupportedNum; i++)
		{
			m_ctrlTriggerModeCombo.AddString((CString)stEnumValue.strSymbolic[i]);
			if (stEnumValue.nSupportValue[i] == stEnumValue.nCurValue)
			{
				nIndex = i;
			}

		}
		m_ctrlTriggerModeCombo.SetCurSel(nIndex);
	}

	// ch:获取TriggerSource | en:Get TriggerSource
	m_ctrlTriggerSourceCombo.ResetContent();
	memset(&stEnumValue, 0, sizeof(stEnumValue));
	nRet = MV_FG_GetEnumValue(m_hDevice, "TriggerSource", &stEnumValue);
	if (MV_FG_SUCCESS == nRet)
	{
		unsigned int nIndex = 0;
		for (unsigned int i = 0; i < stEnumValue.nSupportedNum; i++)
		{
			m_ctrlTriggerSourceCombo.AddString((CString)stEnumValue.strSymbolic[i]);
			if (stEnumValue.nSupportValue[i] == stEnumValue.nCurValue)
			{
				nIndex = i;
			}

		}
		m_ctrlTriggerSourceCombo.SetCurSel(nIndex);
	}


	// ch:获取TriggerActivation | en:Get TriggerActivation
	m_ctrlTriggerActivationCombo.ResetContent();
	memset(&stEnumValue, 0, sizeof(stEnumValue));
	nRet = MV_FG_GetEnumValue(m_hDevice, "TriggerActivation", &stEnumValue);
	if (MV_FG_SUCCESS == nRet)
	{
		unsigned int nIndex = 0;
		for (unsigned int i = 0; i < stEnumValue.nSupportedNum; i++)
		{
			m_ctrlTriggerActivationCombo.AddString((CString)stEnumValue.strSymbolic[i]);
			if (stEnumValue.nSupportValue[i] == stEnumValue.nCurValue)
			{
				nIndex = i;
			}

		}
		m_ctrlTriggerActivationCombo.SetCurSel(nIndex);
		m_bTriggerActivation = TRUE;
	}
	else
	{
		m_bTriggerActivation = FALSE;
	}

	UpdateData(FALSE);

	EnableDevParamsControls();

	return nRet;
}

void CBasicDemoLineScanDlg::OnCbnSelchangeCameraTypeCombo()
{
	if (FALSE == m_ctrlCameraTypeCombo.IsWindowEnabled())
	{
		return;
	}

	int nIndex = m_ctrlCameraTypeCombo.GetCurSel();
	CString strCBText;
	m_ctrlCameraTypeCombo.GetLBText(nIndex, strCBText);
#if _MSC_VER == 1500//VS2008
	int nRet = MV_FG_SetEnumValueByString(m_hInterface, "CameraType", strCBText);
#else
	char chValue[64] = {0};
	int nLen = WideCharToMultiByte(CP_ACP, 0, strCBText, strCBText.GetLength(), NULL, 0, NULL,NULL);
    WideCharToMultiByte(CP_ACP,0, strCBText, strCBText.GetLength(), chValue, nLen,NULL,NULL);
    chValue[nLen] = '\0';
	int nRet = MV_FG_SetEnumValueByString(m_hInterface, "CameraType", chValue);
#endif
	if (MV_FG_SUCCESS != nRet)
	{
		ShowErrorMsg(TEXT("Set CameraType failed."), nRet);
	}

	GetInterfaceParams();
}

void CBasicDemoLineScanDlg::OnEnKillfocusImageHeightEdit()
{
	UpdateData(TRUE);

	int nRet = MV_FG_SetIntValue(m_hInterface, "ImageHeight", m_nImageHeightEdit);
	if (MV_FG_SUCCESS != nRet)
	{
		ShowErrorMsg(TEXT("Set ImageHeight failed."), nRet);
	}

    GetInterfaceParams();
}

void CBasicDemoLineScanDlg::OnCbnSelchangeLineSelectorCombo()
{
	if (FALSE == m_ctrlLineSelectorCombo.IsWindowEnabled())
	{
		return;
	}

	int nIndex = m_ctrlLineSelectorCombo.GetCurSel();
	CString strCBText;
	m_ctrlLineSelectorCombo.GetLBText(nIndex, strCBText);
#if _MSC_VER == 1500//VS2008
	int nRet = MV_FG_SetEnumValueByString(m_hInterface, "LineSelector", strCBText);
#else
	char chValue[64] = {0};
	int nLen = WideCharToMultiByte(CP_ACP, 0, strCBText, strCBText.GetLength(), NULL, 0, NULL,NULL);
    WideCharToMultiByte(CP_ACP,0, strCBText, strCBText.GetLength(), chValue, nLen,NULL,NULL);
    chValue[nLen] = '\0';
	int nRet = MV_FG_SetEnumValueByString(m_hInterface, "LineSelector", chValue);
#endif

	if (MV_FG_SUCCESS != nRet)
	{
		ShowErrorMsg(TEXT("Set LineSelector failed."), nRet);
	}

	GetInterfaceParams();
}

void CBasicDemoLineScanDlg::OnCbnSelchangeLineModeCombo()
{
	if (FALSE == m_ctrlLineModeCombo.IsWindowEnabled())
	{
		return;
	}

	int nIndex = m_ctrlLineModeCombo.GetCurSel();
	CString strCBText;
	m_ctrlLineModeCombo.GetLBText(nIndex, strCBText);
#if _MSC_VER == 1500//VS2008
	int nRet = MV_FG_SetEnumValueByString(m_hInterface, "LineMode", strCBText);
#else
	char chValue[64] = {0};
	int nLen = WideCharToMultiByte(CP_ACP, 0, strCBText, strCBText.GetLength(), NULL, 0, NULL,NULL);
    WideCharToMultiByte(CP_ACP,0, strCBText, strCBText.GetLength(), chValue, nLen,NULL,NULL);
    chValue[nLen] = '\0';
	int nRet = MV_FG_SetEnumValueByString(m_hInterface, "LineMode", chValue);
#endif
	if (MV_FG_SUCCESS != nRet)
	{
		ShowErrorMsg(TEXT("Set LineMode failed."), nRet);
	}

	GetInterfaceParams();
}

void CBasicDemoLineScanDlg::OnCbnSelchangeLineInputPolarityCombo()
{
	if (FALSE == m_ctrlLineInputPolarityCombo.IsWindowEnabled())
	{
		return;
	}

	int nIndex = m_ctrlLineInputPolarityCombo.GetCurSel();
	CString strCBText;
	m_ctrlLineInputPolarityCombo.GetLBText(nIndex, strCBText);
#if _MSC_VER == 1500//VS2008
	int nRet = MV_FG_SetEnumValueByString(m_hInterface, "LineInputPolarity", strCBText);
#else
	char chValue[64] = {0};
	int nLen = WideCharToMultiByte(CP_ACP, 0, strCBText, strCBText.GetLength(), NULL, 0, NULL,NULL);
    WideCharToMultiByte(CP_ACP,0, strCBText, strCBText.GetLength(), chValue, nLen,NULL,NULL);
    chValue[nLen] = '\0';
	int nRet = MV_FG_SetEnumValueByString(m_hInterface, "LineInputPolarity", chValue);
#endif
	if (MV_FG_SUCCESS != nRet)
	{
		ShowErrorMsg(TEXT("Set LineInputPolarity failed."), nRet);
	}

	GetInterfaceParams();
}

void CBasicDemoLineScanDlg::OnCbnSelchangeEncoderSelectorCombo()
{
	if (FALSE == m_ctrlEncoderSelectorCombo.IsWindowEnabled())
	{
		return;
	}

	int nIndex = m_ctrlEncoderSelectorCombo.GetCurSel();
	CString strCBText;
	m_ctrlEncoderSelectorCombo.GetLBText(nIndex, strCBText);
#if _MSC_VER == 1500//VS2008
	int nRet = MV_FG_SetEnumValueByString(m_hInterface, "EncoderSelector", strCBText);
#else
	char chValue[64] = {0};
	int nLen = WideCharToMultiByte(CP_ACP, 0, strCBText, strCBText.GetLength(), NULL, 0, NULL,NULL);
    WideCharToMultiByte(CP_ACP,0, strCBText, strCBText.GetLength(), chValue, nLen,NULL,NULL);
    chValue[nLen] = '\0';
	int nRet = MV_FG_SetEnumValueByString(m_hInterface, "EncoderSelector", chValue);
#endif
	if (MV_FG_SUCCESS != nRet)
	{
		ShowErrorMsg(TEXT("Set EncoderSelector failed."), nRet);
	}

	GetInterfaceParams();
}

void CBasicDemoLineScanDlg::OnCbnSelchangeEncoderSourceACombo()
{
	if (FALSE == m_ctrlEncoderSourceACombo.IsWindowEnabled())
	{
		return;
	}

	int nIndex = m_ctrlEncoderSourceACombo.GetCurSel();
	CString strCBText;
	m_ctrlEncoderSourceACombo.GetLBText(nIndex, strCBText);
#if _MSC_VER == 1500//VS2008
	int nRet = MV_FG_SetEnumValueByString(m_hInterface, "EncoderSourceA", strCBText);
#else
	char chValue[64] = {0};
	int nLen = WideCharToMultiByte(CP_ACP, 0, strCBText, strCBText.GetLength(), NULL, 0, NULL,NULL);
    WideCharToMultiByte(CP_ACP,0, strCBText, strCBText.GetLength(), chValue, nLen,NULL,NULL);
    chValue[nLen] = '\0';
	int nRet = MV_FG_SetEnumValueByString(m_hInterface, "EncoderSourceA", chValue);
#endif
	if (MV_FG_SUCCESS != nRet)
	{
		ShowErrorMsg(TEXT("Set EncoderSourceA failed."), nRet);
	}

	GetInterfaceParams();
}

void CBasicDemoLineScanDlg::OnCbnSelchangeEncoderSourceBCombo()
{
	if (FALSE == m_ctrlEncoderSourceBCombo.IsWindowEnabled())
	{
		return;
	}

	int nIndex = m_ctrlEncoderSourceBCombo.GetCurSel();
	CString strCBText;
	m_ctrlEncoderSourceBCombo.GetLBText(nIndex, strCBText);
#if _MSC_VER == 1500//VS2008
	int nRet = MV_FG_SetEnumValueByString(m_hInterface, "EncoderSourceB", strCBText);
#else
	char chValue[64] = {0};
	int nLen = WideCharToMultiByte(CP_ACP, 0, strCBText, strCBText.GetLength(), NULL, 0, NULL,NULL);
    WideCharToMultiByte(CP_ACP,0, strCBText, strCBText.GetLength(), chValue, nLen,NULL,NULL);
    chValue[nLen] = '\0';
	int nRet = MV_FG_SetEnumValueByString(m_hInterface, "EncoderSourceB", chValue);
#endif
	if (MV_FG_SUCCESS != nRet)
	{
		ShowErrorMsg(TEXT("Set EncoderSourceB failed."), nRet);
	}

	GetInterfaceParams();
}

void CBasicDemoLineScanDlg::OnCbnSelchangeEncoderTriggerModeCombo()
{
	if (FALSE == m_ctrlEncoderTriggerModeCombo.IsWindowEnabled())
	{
		return;
	}

	int nIndex = m_ctrlEncoderTriggerModeCombo.GetCurSel();
	CString strCBText;
	m_ctrlEncoderTriggerModeCombo.GetLBText(nIndex, strCBText);
#if _MSC_VER == 1500//VS2008
	int nRet = MV_FG_SetEnumValueByString(m_hInterface, "EncoderTriggerMode", strCBText);
#else
	char chValue[64] = {0};
	int nLen = WideCharToMultiByte(CP_ACP, 0, strCBText, strCBText.GetLength(), NULL, 0, NULL,NULL);
    WideCharToMultiByte(CP_ACP,0, strCBText, strCBText.GetLength(), chValue, nLen,NULL,NULL);
    chValue[nLen] = '\0';
	int nRet = MV_FG_SetEnumValueByString(m_hInterface, "EncoderTriggerMode", chValue);
#endif
	if (MV_FG_SUCCESS != nRet)
	{
		ShowErrorMsg(TEXT("Set EncoderTriggerMode failed."), nRet);
	}

	GetInterfaceParams();
}

void CBasicDemoLineScanDlg::OnCbnSelchangeCcSelectorCombo()
{
	if (FALSE == m_ctrlCCSelectorCombo.IsWindowEnabled())
	{
		return;
	}

	int nIndex = m_ctrlCCSelectorCombo.GetCurSel();
	CString strCBText;
	m_ctrlCCSelectorCombo.GetLBText(nIndex, strCBText);
#if _MSC_VER == 1500//VS2008
	int nRet = MV_FG_SetEnumValueByString(m_hInterface, "CCSelector", strCBText);
#else
	char chValue[64] = {0};
	int nLen = WideCharToMultiByte(CP_ACP, 0, strCBText, strCBText.GetLength(), NULL, 0, NULL,NULL);
    WideCharToMultiByte(CP_ACP,0, strCBText, strCBText.GetLength(), chValue, nLen,NULL,NULL);
    chValue[nLen] = '\0';
	int nRet = MV_FG_SetEnumValueByString(m_hInterface, "CCSelector", chValue);
#endif
	if (MV_FG_SUCCESS != nRet)
	{
		ShowErrorMsg(TEXT("Set CCSelector failed."), nRet);
	}

	GetInterfaceParams();
}

void CBasicDemoLineScanDlg::OnCbnSelchangeCcSourceCombo()
{
	if (FALSE == m_ctrlCCSourceCombo.IsWindowEnabled())
	{
		return;
	}

	int nIndex = m_ctrlCCSourceCombo.GetCurSel();
	CString strCBText;
	m_ctrlCCSourceCombo.GetLBText(nIndex, strCBText);
#if _MSC_VER == 1500//VS2008
	int nRet = MV_FG_SetEnumValueByString(m_hInterface, "CCSource", strCBText);
#else
	char chValue[64] = {0};
	int nLen = WideCharToMultiByte(CP_ACP, 0, strCBText, strCBText.GetLength(), NULL, 0, NULL,NULL);
    WideCharToMultiByte(CP_ACP,0, strCBText, strCBText.GetLength(), chValue, nLen,NULL,NULL);
    chValue[nLen] = '\0';
	int nRet = MV_FG_SetEnumValueByString(m_hInterface, "CCSource", chValue);
#endif
	if (MV_FG_SUCCESS != nRet)
	{
		ShowErrorMsg(TEXT("Set CCSource failed."), nRet);
	}

	GetInterfaceParams();
}

void CBasicDemoLineScanDlg::OnEnKillfocusWidthEdit()
{
	UpdateData(TRUE);

	int nRet = MV_FG_SetIntValue(m_hDevice, "Width", m_nCameraWidth);
	if (MV_FG_SUCCESS != nRet)
	{
		ShowErrorMsg(TEXT("Set Width failed."), nRet);
	}

	GetDeviceParams();
}

void CBasicDemoLineScanDlg::OnEnKillfocusHeightEdit()
{
	UpdateData(TRUE);

	int nRet = MV_FG_SetIntValue(m_hDevice, "Height", m_nCameraHeight);
	if (MV_FG_SUCCESS != nRet)
	{
		ShowErrorMsg(TEXT("Set Height failed."), nRet);
	}

	GetDeviceParams();
}

void CBasicDemoLineScanDlg::OnCbnSelchangePixelformatCombo()
{
	if (FALSE == m_ctrlPixelFormatCombo.IsWindowEnabled())
	{
		return;
	}

	int nIndex = m_ctrlPixelFormatCombo.GetCurSel();
	CString strCBText;
	m_ctrlPixelFormatCombo.GetLBText(nIndex, strCBText);
#if _MSC_VER == 1500//VS2008
	int nRet = MV_FG_SetEnumValueByString(m_hInterface, "PixelFormat", strCBText);
#else
	char chValue[64] = {0};
	int nLen = WideCharToMultiByte(CP_ACP, 0, strCBText, strCBText.GetLength(), NULL, 0, NULL,NULL);
    WideCharToMultiByte(CP_ACP,0, strCBText, strCBText.GetLength(), chValue, nLen,NULL,NULL);
    chValue[nLen] = '\0';
	int nRet = MV_FG_SetEnumValueByString(m_hDevice, "PixelFormat", chValue);
#endif
	if (MV_FG_SUCCESS != nRet)
	{
		ShowErrorMsg(TEXT("Set PixelFormat failed."), nRet);
	}

	GetDeviceParams();
}

void CBasicDemoLineScanDlg::OnCbnSelchangeTriggerSelectorCombo()
{
	if (FALSE == m_ctrlTriggerSelectorCombo.IsWindowEnabled())
	{
		return;
	}

	int nIndex = m_ctrlTriggerSelectorCombo.GetCurSel();
	CString strCBText;
	m_ctrlTriggerSelectorCombo.GetLBText(nIndex, strCBText);
#if _MSC_VER == 1500//VS2008
	int nRet = MV_FG_SetEnumValueByString(m_hInterface, "TriggerSelector", strCBText);
#else
	char chValue[64] = {0};
	int nLen = WideCharToMultiByte(CP_ACP, 0, strCBText, strCBText.GetLength(), NULL, 0, NULL,NULL);
    WideCharToMultiByte(CP_ACP,0, strCBText, strCBText.GetLength(), chValue, nLen,NULL,NULL);
    chValue[nLen] = '\0';
	int nRet = MV_FG_SetEnumValueByString(m_hDevice, "TriggerSelector", chValue);
#endif
	if (MV_FG_SUCCESS != nRet)
	{
		ShowErrorMsg(TEXT("Set TriggerSelector failed."), nRet);
	}

	GetDeviceParams();
}

void CBasicDemoLineScanDlg::OnCbnSelchangeTriggerModeComb()
{
	if (FALSE == m_ctrlTriggerModeCombo.IsWindowEnabled())
	{
		return;
	}

	int nIndex = m_ctrlTriggerModeCombo.GetCurSel();
	CString strCBText;
	m_ctrlTriggerModeCombo.GetLBText(nIndex, strCBText);
#if _MSC_VER == 1500//VS2008
	int nRet = MV_FG_SetEnumValueByString(m_hInterface, "TriggerMode", strCBText);
#else
	char chValue[64] = {0};
	int nLen = WideCharToMultiByte(CP_ACP, 0, strCBText, strCBText.GetLength(), NULL, 0, NULL,NULL);
    WideCharToMultiByte(CP_ACP,0, strCBText, strCBText.GetLength(), chValue, nLen,NULL,NULL);
    chValue[nLen] = '\0';
	int nRet = MV_FG_SetEnumValueByString(m_hDevice, "TriggerMode", chValue);
#endif
	if (MV_FG_SUCCESS != nRet)
	{
		ShowErrorMsg(TEXT("Set TriggerMode failed."), nRet);
	}

	GetDeviceParams();
}

void CBasicDemoLineScanDlg::OnCbnSelchangeTriggerSourceCombo()
{
	if (FALSE == m_ctrlTriggerSourceCombo.IsWindowEnabled())
	{
		return;
	}

	int nIndex = m_ctrlTriggerSourceCombo.GetCurSel();
	CString strCBText;
	m_ctrlTriggerSourceCombo.GetLBText(nIndex, strCBText);
#if _MSC_VER == 1500//VS2008
	int nRet = MV_FG_SetEnumValueByString(m_hInterface, "TriggerSource", strCBText);
#else
	char chValue[64] = {0};
	int nLen = WideCharToMultiByte(CP_ACP, 0, strCBText, strCBText.GetLength(), NULL, 0, NULL,NULL);
    WideCharToMultiByte(CP_ACP,0, strCBText, strCBText.GetLength(), chValue, nLen,NULL,NULL);
    chValue[nLen] = '\0';
	int nRet = MV_FG_SetEnumValueByString(m_hDevice, "TriggerSource", chValue);
#endif
	if (MV_FG_SUCCESS != nRet)
	{
		ShowErrorMsg(TEXT("Set TriggerSource failed."), nRet);
	}

	GetDeviceParams();
}

void CBasicDemoLineScanDlg::OnCbnSelchangeTriggerActivationCombo()
{
	if (FALSE == m_ctrlTriggerActivationCombo.IsWindowEnabled())
	{
		return;
	}

	int nIndex = m_ctrlTriggerActivationCombo.GetCurSel();
	CString strCBText;
	m_ctrlTriggerActivationCombo.GetLBText(nIndex, strCBText);
#if _MSC_VER == 1500//VS2008
	int nRet = MV_FG_SetEnumValueByString(m_hInterface, "TriggerActivation", strCBText);
#else
	char chValue[64] = {0};
	int nLen = WideCharToMultiByte(CP_ACP, 0, strCBText, strCBText.GetLength(), NULL, 0, NULL,NULL);
    WideCharToMultiByte(CP_ACP,0, strCBText, strCBText.GetLength(), chValue, nLen,NULL,NULL);
    chValue[nLen] = '\0';
	int nRet = MV_FG_SetEnumValueByString(m_hDevice, "TriggerActivation", chValue);
#endif
	if (MV_FG_SUCCESS != nRet)
	{
		ShowErrorMsg(TEXT("Set TriggerActivation failed."), nRet);
	}

	GetDeviceParams();
}

void CBasicDemoLineScanDlg::OnCbnSelchangeScanModeCombo()
{
	if (FALSE == m_ctrlScanModeCombo.IsWindowEnabled())
	{
		return;
	}

	int nIndex = m_ctrlScanModeCombo.GetCurSel();
	CString strCBText;
	m_ctrlScanModeCombo.GetLBText(nIndex, strCBText);
#if _MSC_VER == 1500//VS2008
	int nRet = MV_FG_SetEnumValueByString(m_hInterface, "ScanMode", strCBText);
#else
	char chValue[64] = {0};
	int nLen = WideCharToMultiByte(CP_ACP, 0, strCBText, strCBText.GetLength(), NULL, 0, NULL,NULL);
    WideCharToMultiByte(CP_ACP,0, strCBText, strCBText.GetLength(), chValue, nLen,NULL,NULL);
    chValue[nLen] = '\0';
	int nRet = MV_FG_SetEnumValueByString(m_hDevice, "ScanMode", chValue);
#endif
	if (MV_FG_SUCCESS != nRet)
	{
		ShowErrorMsg(TEXT("Set ScanMode failed."), nRet);
	}

	GetDeviceParams();
}
