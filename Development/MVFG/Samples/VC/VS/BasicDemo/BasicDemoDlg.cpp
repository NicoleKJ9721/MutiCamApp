/*
This example shows the user the basic functionality of the MVFGControl method.
This example covers enumeration module, control module and stream module.
This is also an example of how to save images.

[注] 需要保存文件的示例程序在部分环境下需以管理员权限执行，否则会有异常
[PS] Sample programs that need to save files need to be executed with administrator privileges \
     in some environments, otherwise there will be exceptions

*/
#include "stdafx.h"
#include "BasicDemo.h"
#include "BasicDemoDlg.h"

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
    , m_hInterface(NULL)
    , m_hDevice(NULL)
    , m_hStream(NULL)
    , m_nDeviceCombo(0)
    , m_bOpenIF(FALSE)
    , m_bOpenDevice(FALSE)
    , m_bStartGrabbing(FALSE)
    , m_hGrabThread(NULL)
    , m_bThreadState(FALSE)
    , m_nTriggerMode(TRIGGER_MODE_OFF)
    , m_bSoftWareTriggerCheck(FALSE)
    , m_nTriggerSource(TRIGGER_SOURCE_SOFTWARE)
    , m_nInterfaceNum(0)
    , m_pDataBuf(NULL)
    , m_nDataBufSize(0)
    , m_pSaveImageBuf(NULL)
    , m_nSaveImageBufSize(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    memset(&m_stImageInfo, 0, sizeof(MV_FG_INPUT_IMAGE_INFO));
}

void CBasicDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IF_COMBO, m_ctrlInterfaceCombo);
	DDX_Control(pDX, IDC_DEVICE_COMBO, m_ctrlDeviceCombo);
	DDX_CBIndex(pDX, IDC_DEVICE_COMBO, m_nDeviceCombo);
	DDX_Check(pDX, IDC_SOFTWARE_TRIGGER_CHECK, m_bSoftWareTriggerCheck);
}

BEGIN_MESSAGE_MAP(CBasicDemoDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	// }}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_ENUM_IF_BUTTON, &CBasicDemoDlg::OnBnClickedEnumIfButton)
    ON_BN_CLICKED(IDC_ENUM_DEV_BUTTON, &CBasicDemoDlg::OnBnClickedEnumDevButton)
    ON_BN_CLICKED(IDC_OPEN_BUTTON, &CBasicDemoDlg::OnBnClickedOpenButton)
    ON_BN_CLICKED(IDC_CLOSE_BUTTON, &CBasicDemoDlg::OnBnClickedCloseButton)
    ON_BN_CLICKED(IDC_CONTINUS_MODE_RADIO, &CBasicDemoDlg::OnBnClickedContinusModeRadio)
    ON_BN_CLICKED(IDC_TRIGGER_MODE_RADIO, &CBasicDemoDlg::OnBnClickedTriggerModeRadio)
    ON_BN_CLICKED(IDC_START_GRABBING_BUTTON, &CBasicDemoDlg::OnBnClickedStartGrabbingButton)
    ON_BN_CLICKED(IDC_STOP_GRABBING_BUTTON, &CBasicDemoDlg::OnBnClickedStopGrabbingButton)
    ON_BN_CLICKED(IDC_SOFTWARE_TRIGGER_CHECK, &CBasicDemoDlg::OnBnClickedSoftwareTriggerCheck)
    ON_BN_CLICKED(IDC_SOFTWARE_ONCE_BUTTON, &CBasicDemoDlg::OnBnClickedSoftwareOnceButton)
    ON_WM_CLOSE()
    ON_BN_CLICKED(IDC_OPEN_IF_BUTTON, &CBasicDemoDlg::OnBnClickedOpenIfButton)
    ON_BN_CLICKED(IDC_CLOSE_IF_BUTTON, &CBasicDemoDlg::OnBnClickedCloseIfButton)
    ON_BN_CLICKED(IDC_BMP_SAVE_BUTTON, &CBasicDemoDlg::OnBnClickedBmpSaveButton)
    ON_BN_CLICKED(IDC_JPEG_SAVE_BUTTON, &CBasicDemoDlg::OnBnClickedJpegSaveButton)
    ON_MESSAGE(WM_DISPLAY_ERROR, &CBasicDemoDlg::OnDisplayError)
	ON_BN_CLICKED(IDC_TIFF_SAVE_BUTTON, &CBasicDemoDlg::OnBnClickedTiffSaveButton)
	ON_BN_CLICKED(IDC_PNG_SAVE_BUTTON, &CBasicDemoDlg::OnBnClickedPngSaveButton)
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
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	DisplayWindowInitial();     // ch:显示框初始化 | en:Display Window Initialization

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

    GetDlgItem(IDC_SOFTWARE_TRIGGER_CHECK)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);
    GetDlgItem(IDC_SOFTWARE_ONCE_BUTTON)->EnableWindow((m_bStartGrabbing && m_bSoftWareTriggerCheck && ((CButton *)GetDlgItem(IDC_TRIGGER_MODE_RADIO))->GetCheck()) ? TRUE : FALSE);
    GetDlgItem(IDC_CONTINUS_MODE_RADIO)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);
    GetDlgItem(IDC_TRIGGER_MODE_RADIO)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);

    GetDlgItem(IDC_BMP_SAVE_BUTTON)->EnableWindow(m_bStartGrabbing ? TRUE : FALSE);
    GetDlgItem(IDC_JPEG_SAVE_BUTTON)->EnableWindow(m_bStartGrabbing ? TRUE : FALSE);
	GetDlgItem(IDC_TIFF_SAVE_BUTTON)->EnableWindow(m_bStartGrabbing ? TRUE : FALSE);
	GetDlgItem(IDC_PNG_SAVE_BUTTON)->EnableWindow(m_bStartGrabbing ? TRUE : FALSE);

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
void CBasicDemoDlg::OnBnClickedEnumIfButton()
{
    int nRet = 0;

    // ch:枚举采集卡 | en:Enum interface
    bool bChanged = false;
    nRet = MV_FG_UpdateInterfaceList(MV_FG_CXP_INTERFACE | MV_FG_GEV_INTERFACE | MV_FG_CAMERALINK_INTERFACE | MV_FG_XoF_INTERFACE, &bChanged);
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
void CBasicDemoDlg::OnBnClickedEnumDevButton()
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
int CBasicDemoDlg::CloseDevice()
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

// ch:获取触发模式 | en:Get Trigger Mode
int CBasicDemoDlg::GetTriggerMode()
{
    MV_FG_ENUMVALUE stEnumValue = { 0 };

    int nRet = MV_FG_GetEnumValue(m_hDevice, "TriggerMode", &stEnumValue);
    if (MV_FG_SUCCESS != nRet)
    {
        return nRet;
    }

    m_nTriggerMode = stEnumValue.nCurValue;
    if (TRIGGER_MODE_ON ==  m_nTriggerMode)
    {
        OnBnClickedTriggerModeRadio();
    }
    else
    {
        m_nTriggerMode = TRIGGER_MODE_OFF;
        OnBnClickedContinusModeRadio();
    }

    return MV_FG_SUCCESS;
}

// ch:设置触发模式 | en:Set Trigger Mode
int CBasicDemoDlg::SetTriggerMode()
{
    return MV_FG_SetEnumValue(m_hDevice, "TriggerMode", (unsigned int)m_nTriggerMode);
}

// ch:获取触发源 | en:Get Trigger Source
int CBasicDemoDlg::GetTriggerSource()
{
    MV_FG_ENUMVALUE stEnumValue = { 0 };

    int nRet = MV_FG_GetEnumValue(m_hDevice, "TriggerSource", &stEnumValue);
    if (MV_FG_SUCCESS != nRet)
    {
        return nRet;
    }

    if (TRIGGER_SOURCE_SOFTWARE == stEnumValue.nCurValue)
    {
        m_bSoftWareTriggerCheck = TRUE;
    }
    else
    {
        m_bSoftWareTriggerCheck = FALSE;
    }

    return MV_FG_SUCCESS;
}

// ch:设置触发源 | en:Set Trigger Source
int CBasicDemoDlg::SetTriggerSource()
{
    if (m_bSoftWareTriggerCheck)
    {
        m_nTriggerSource = TRIGGER_SOURCE_SOFTWARE;
        int nRet = MV_FG_SetEnumValue(m_hDevice, "TriggerSource", (unsigned int)m_nTriggerSource);
        if (MV_FG_SUCCESS != nRet)
        {
            ShowErrorMsg(TEXT("Set Software Trigger Failed"), nRet);
            return nRet;
        }
        GetDlgItem(IDC_SOFTWARE_ONCE_BUTTON )->EnableWindow(TRUE);
    }
    else
    {
        m_nTriggerSource = TRIGGER_SOURCE_LINE0;
        int nRet = MV_FG_SetEnumValue(m_hDevice, "TriggerSource", (unsigned int)m_nTriggerSource);
        if (MV_FG_SUCCESS != nRet)
        {
            ShowErrorMsg(TEXT("Set Hardware Trigger Failed"), nRet);
            return nRet;
        }
        GetDlgItem(IDC_SOFTWARE_ONCE_BUTTON )->EnableWindow(FALSE);
    }

    return MV_FG_SUCCESS;
}

int CBasicDemoDlg::GrabThreadProcess()
{
    MV_FG_BUFFER_INFO           stFrameInfo = { 0 };    // ch:图像信息 | en:Frame info
    MV_FG_DISPLAY_FRAME_INFO    stDisplayInfo = { 0 };  // ch:显示的图像信息 | en:Display frame info
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
            else
            {
                if (TRIGGER_MODE_ON ==  m_nTriggerMode)
                {
                    Sleep(5);       // ch:触发模式超时正常，调用Sleep() | en:Trigger mode timeout is normal, call Sleep()
                }
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
void CBasicDemoDlg::OnBnClickedOpenButton()
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

    nRet = GetTriggerMode();
    if (MV_FG_SUCCESS != nRet)
    {
        ShowErrorMsg(TEXT("Get Trigger Mode Failed"), nRet);
    }

    nRet = GetTriggerSource();
    if (MV_FG_SUCCESS != nRet)
    {
        ShowErrorMsg(TEXT("Get Trigger Source Failed"), nRet);
    }

    UpdateData(FALSE);
    EnableControls(TRUE);
}

// ch:按下关闭设备按钮：关闭设备 | en:Click Close button: Close Device
void CBasicDemoDlg::OnBnClickedCloseButton()
{
    CloseDevice();

    EnableControls(TRUE);
}

// ch:按下连续模式按钮 | en:Click Continues button
void CBasicDemoDlg::OnBnClickedContinusModeRadio()
{
    ((CButton *)GetDlgItem(IDC_CONTINUS_MODE_RADIO))->SetCheck(TRUE);
    ((CButton *)GetDlgItem(IDC_TRIGGER_MODE_RADIO))->SetCheck(FALSE);
    m_nTriggerMode = TRIGGER_MODE_OFF;

    int nRet = SetTriggerMode();
    if (MV_FG_SUCCESS != nRet)
    {
        ShowErrorMsg(TEXT("Set Continus Mode Failed"), nRet);
        return;
    }

    GetDlgItem(IDC_SOFTWARE_ONCE_BUTTON)->EnableWindow(FALSE);
}

// ch:按下触发模式按钮 | en:Click Trigger Mode button
void CBasicDemoDlg::OnBnClickedTriggerModeRadio()
{
    UpdateData(TRUE);

    ((CButton *)GetDlgItem(IDC_CONTINUS_MODE_RADIO))->SetCheck(FALSE);
    ((CButton *)GetDlgItem(IDC_TRIGGER_MODE_RADIO))->SetCheck(TRUE);
    m_nTriggerMode = TRIGGER_MODE_ON;

    int nRet = SetTriggerMode();
    if (MV_FG_SUCCESS != nRet)
    {
        ShowErrorMsg(TEXT("Set Trigger Mode Failed"), nRet);
        return;
    }

    if (TRUE == m_bStartGrabbing)
    {
        if (TRUE == m_bSoftWareTriggerCheck)
        {
            GetDlgItem(IDC_SOFTWARE_ONCE_BUTTON )->EnableWindow(TRUE);
        }
    }
}

// ch:按下开始采集按钮 | en:Click Start button
void CBasicDemoDlg::OnBnClickedStartGrabbingButton()
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
void CBasicDemoDlg::OnBnClickedStopGrabbingButton()
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

// ch:按下软触发按钮 | en:Click Software button
void CBasicDemoDlg::OnBnClickedSoftwareTriggerCheck()
{
    UpdateData(TRUE);

    int nRet = SetTriggerSource();
    if (MV_FG_SUCCESS != nRet)
    {
        return;
    }

    EnableControls(TRUE);
}

// ch:按下软触发一次按钮 | en:Click Execute button
void CBasicDemoDlg::OnBnClickedSoftwareOnceButton()
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
void CBasicDemoDlg::OnClose()
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
LRESULT CBasicDemoDlg::OnDisplayError(WPARAM wParam, LPARAM lParam)
{
    ShowErrorMsg(TEXT("Display failed"), (int)wParam);

    OnBnClickedStopGrabbingButton();

    return 0;
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

bool CBasicDemoDlg::RemoveCustomPixelFormats(MV_FG_PIXEL_TYPE enPixelFormat)
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

void CBasicDemoDlg::OnBnClickedOpenIfButton()
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

    EnableControls(FALSE);
}

void CBasicDemoDlg::OnBnClickedCloseIfButton()
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

int CBasicDemoDlg::SaveImage(SAVE_IAMGE_TYPE enSaveImageType)
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

void CBasicDemoDlg::OnBnClickedBmpSaveButton()
{
    int nRet = SaveImage(Image_Bmp);
    if (MV_FG_SUCCESS != nRet)
    {
        ShowErrorMsg(TEXT("Save bmp failed"), nRet);
        return;
    }
    ShowErrorMsg(TEXT("Save bmp succeed"), 0);
}

void CBasicDemoDlg::OnBnClickedJpegSaveButton()
{
    int nRet = SaveImage(Image_Jpeg);
    if (MV_FG_SUCCESS != nRet)
    {
        ShowErrorMsg(TEXT("Save jpg failed"), nRet);
        return;
    }
    ShowErrorMsg(TEXT("Save jpg succeed"), 0);
}



void CBasicDemoDlg::OnBnClickedTiffSaveButton()
{
	int nRet = SaveImage(Image_Tiff);
	if (MV_FG_SUCCESS != nRet)
	{
		ShowErrorMsg(TEXT("Save tiff failed"), nRet);
		return;
	}
	ShowErrorMsg(TEXT("Save tiff succeed"), 0);
}

void CBasicDemoDlg::OnBnClickedPngSaveButton()
{
	int nRet = SaveImage(Image_Png);
	if (MV_FG_SUCCESS != nRet)
	{
		ShowErrorMsg(TEXT("Save png failed"), nRet);
		return;
	}
	ShowErrorMsg(TEXT("Save png succeed"), 0);
}
