/*
This example shows the user the device's disconnected reconnection function.
This example shows the user how to try to reconnect the device and restore the state before the device went offline.
*/
#include "stdafx.h"
#include "ReconnectDemo.h"
#include "ReconnectDemoDlg.h"
#include "conio.h"

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

// CReconnectDemoDlg dialog
CReconnectDemoDlg::CReconnectDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CReconnectDemoDlg::IDD, pParent)
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
    , m_nDeviceIndex(0)
    , m_pDataBuf(NULL)
    , m_nDataBufSize(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    memset(m_chDeviceID, 0, MV_FG_MAX_DEV_INFO_SIZE);
}

void CReconnectDemoDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_IF_COMBO, m_ctrlInterfaceCombo);
    DDX_Control(pDX, IDC_DEVICE_COMBO, m_ctrlDeviceCombo);
    DDX_CBIndex(pDX, IDC_DEVICE_COMBO, m_nDeviceCombo);
}

BEGIN_MESSAGE_MAP(CReconnectDemoDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	// }}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_ENUM_IF_BUTTON, &CReconnectDemoDlg::OnBnClickedEnumIfButton)
    ON_BN_CLICKED(IDC_ENUM_DEV_BUTTON, &CReconnectDemoDlg::OnBnClickedEnumDevButton)
    ON_BN_CLICKED(IDC_OPEN_BUTTON, &CReconnectDemoDlg::OnBnClickedOpenButton)
    ON_BN_CLICKED(IDC_CLOSE_BUTTON, &CReconnectDemoDlg::OnBnClickedCloseButton)
    ON_BN_CLICKED(IDC_START_GRABBING_BUTTON, &CReconnectDemoDlg::OnBnClickedStartGrabbingButton)
    ON_BN_CLICKED(IDC_STOP_GRABBING_BUTTON, &CReconnectDemoDlg::OnBnClickedStopGrabbingButton)
    ON_WM_CLOSE()
    ON_BN_CLICKED(IDC_OPEN_IF_BUTTON, &CReconnectDemoDlg::OnBnClickedOpenIfButton)
    ON_BN_CLICKED(IDC_CLOSE_IF_BUTTON, &CReconnectDemoDlg::OnBnClickedCloseIfButton)
    ON_MESSAGE(WM_DISPLAY_ERROR, &CReconnectDemoDlg::OnDisplayError)
END_MESSAGE_MAP()

// ch:取流线程 | en:Grabbing thread
unsigned int __stdcall GrabThread(void* pUser)
{
    if (pUser)
    {
        CReconnectDemoDlg* pCam = (CReconnectDemoDlg*)pUser;

        pCam->GrabThreadProcess();

        return 0;
    }

    return -1;
}

// CReconnectDemoDlg message handlers
BOOL CReconnectDemoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

    if (!AllocConsole())
    {
        AfxMessageBox(_T("Faild to create the console!"), MB_ICONEXCLAMATION);
    }
    freopen("CON", "r", stdin);
    freopen("CON", "w", stdout);
    freopen("CON", "w", stderr);

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

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CReconnectDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
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
void CReconnectDemoDlg::OnPaint()
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
HCURSOR CReconnectDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// ch:初始化时的窗口显示 | en:Window display when Initial
void CReconnectDemoDlg::EnableWindowInitial()
{
    GetDlgItem(IDC_OPEN_BUTTON)->EnableWindow(FALSE);
    GetDlgItem(IDC_CLOSE_BUTTON)->EnableWindow(FALSE);
    GetDlgItem(IDC_START_GRABBING_BUTTON)->EnableWindow(FALSE);
    GetDlgItem(IDC_STOP_GRABBING_BUTTON)->EnableWindow(FALSE);
}

// ch:打开设备但不开始抓图 | en:Open device but does not start grabbing*/
void CReconnectDemoDlg::EnableWindowWhenOpenNotStart()
{
    GetDlgItem(IDC_OPEN_BUTTON)->EnableWindow(FALSE);
    GetDlgItem(IDC_CLOSE_BUTTON)->EnableWindow(TRUE);
    GetDlgItem(IDC_START_GRABBING_BUTTON)->EnableWindow(TRUE);
}

// ch:按下开始采集按钮时的按钮颜色 | en:Button color when the start grabbing button is pressed
void CReconnectDemoDlg::EnableWindowWhenStart()
{
    GetDlgItem(IDC_STOP_GRABBING_BUTTON)->EnableWindow(TRUE);
    GetDlgItem(IDC_START_GRABBING_BUTTON)->EnableWindow(FALSE);
}

// ch:按下结束采集时的按钮颜色 | en:Button color when the stop grabbing button is pressed
void CReconnectDemoDlg::EnableWindowWhenStop()
{
    GetDlgItem(IDC_STOP_GRABBING_BUTTON)->EnableWindow(FALSE);
    GetDlgItem(IDC_START_GRABBING_BUTTON)->EnableWindow(TRUE);
}

// ch:按钮使能 | en:Enable control
void CReconnectDemoDlg::EnableControls(BOOL bIsCameraReady)
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
}

// ch:最开始时的窗口初始化 | en:Initial window initialization
void CReconnectDemoDlg::DisplayWindowInitial()
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
void CReconnectDemoDlg::ShowErrorMsg(CString csMessage, int nErrorNum)
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
    case MV_FG_ERR_IMG_FORMAT:          errorMsg += "Invalid or unsupported image format ";                   break;
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
void CReconnectDemoDlg::OnBnClickedEnumIfButton()
{
    int nRet = 0;

    // ch:枚举采集卡 | en:Enum interface
    bool bChanged = false;
    nRet = MV_FG_UpdateInterfaceList(MV_FG_CXP_INTERFACE | MV_FG_GEV_INTERFACE | MV_FG_CAMERALINK_INTERFACE | MV_FG_XoF_INTERFACE, &bChanged);
    if (MV_FG_SUCCESS != nRet)
    {
        ShowErrorMsg(_TEXT("Enum Interfaces failed"), nRet);
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
void CReconnectDemoDlg::OnBnClickedEnumDevButton()
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
int CReconnectDemoDlg::CloseDevice()
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

    return nRet;
}

int CReconnectDemoDlg::GrabThreadProcess()
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
                Sleep(5);
            }
        }
        else
        {
            Sleep(5);       // ch:还未开始取流，调用Sleep() | en:Acquisition not start, call Sleep()
        }
    }

    return MV_FG_SUCCESS;
}

// ch:获取设备ID | en:Get Device ID
int CReconnectDemoDlg::GetDeviceID(unsigned int nIndex)
{
    int nRet = MV_FG_SUCCESS;
    MV_FG_DEVICE_INFO stDevInfo = {0};

    nRet = MV_FG_GetDeviceInfo(m_hInterface, nIndex, &stDevInfo);
    if (MV_FG_SUCCESS != nRet)
    {
        ShowErrorMsg(TEXT("Get Device Info failed"), nRet);
        MV_FG_CloseDevice(m_hDevice);
        return nRet;
    }

    switch (stDevInfo.nDevType)
    {
    case MV_FG_CXP_DEVICE:
        {
            memcpy(m_chDeviceID, stDevInfo.DevInfo.stCXPDevInfo.chDeviceID, MV_FG_MAX_DEV_INFO_SIZE);
            break;
        }
    case MV_FG_GEV_DEVICE:
        {
            memcpy(m_chDeviceID, stDevInfo.DevInfo.stGEVDevInfo.chDeviceID, MV_FG_MAX_DEV_INFO_SIZE);
            break;
        }
    case MV_FG_CAMERALINK_DEVICE:
        {
            memcpy(m_chDeviceID, stDevInfo.DevInfo.stCMLDevInfo.chDeviceID, MV_FG_MAX_DEV_INFO_SIZE);
            break;
        }
    case MV_FG_XoF_DEVICE:
        {
            memcpy(m_chDeviceID, stDevInfo.DevInfo.stXoFDevInfo.chDeviceID, MV_FG_MAX_DEV_INFO_SIZE);
            break;
        }
    default:
        {
            return MV_FG_ERR_INVALID_ID;
        }
    }

    return nRet;
}

// ch:按下打开设备按钮：打开设备 | en:Click Open button: Open Device
void CReconnectDemoDlg::OnBnClickedOpenButton()
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

    nRet = GetDeviceID((unsigned int)nIndex);
    if (MV_FG_SUCCESS != nRet)
    {
        ShowErrorMsg(TEXT("Get device id failed"), nRet);
        return;
    }

    // ch:关闭触发模式 | en:Close Trigger Mode
    MV_FG_SetEnumValue(m_hDevice, "TriggerMode", 0);

    m_bOpenDevice = TRUE;
    m_nDeviceIndex = m_nDeviceCombo;
    MV_FG_RegisterExceptionCallBack(m_hDevice, ReconnectDevice, this);
    UpdateData(FALSE);
    EnableControls(TRUE);
}

// ch:按下关闭设备按钮：关闭设备 | en:Click Close button: Close Device
void CReconnectDemoDlg::OnBnClickedCloseButton()
{
    CloseDevice();
    EnableControls(TRUE);
}

// ch:按下开始采集按钮 | en:Click Start button
void CReconnectDemoDlg::OnBnClickedStartGrabbingButton()
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
void CReconnectDemoDlg::OnBnClickedStopGrabbingButton()
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

// ch:右上角退出 | en:Exit from upper right corner
void CReconnectDemoDlg::OnClose()
{
    PostQuitMessage(0);
    CloseDevice();

    if (m_hInterface)
    {
        MV_FG_CloseInterface(m_hInterface);
        m_hInterface = NULL;
    }

    if(!FreeConsole())
    {
        AfxMessageBox(_T("Could not free the console!"));
    }

    CDialog::OnClose();
}

// ch:重连设备 | en:Reconnect Device
void __stdcall CReconnectDemoDlg::ReconnectDevice(MV_FG_EXCEPTION_TYPE enExceptionType, void* pUser)
{
    if (EXCEPTION_TYPE_CAMERA_DISCONNECT_ERR == enExceptionType)
    {
        int nRet = MV_FG_SUCCESS;
        bool bChanged = false;
        CReconnectDemoDlg* pThis = (CReconnectDemoDlg*)pUser;

        // 设备掉线时初始化控件状态
        pThis->EnableWindowInitial();
        if (pThis->m_bOpenDevice)
        {
            MV_FG_CloseStream(pThis->m_hStream);
            pThis->m_hStream = NULL;

            MV_FG_CloseDevice(pThis->m_hDevice);
            pThis->m_hDevice = NULL;

            BOOL bConnected = FALSE;
            while(1)
            {
                MV_FG_UpdateDeviceList(pThis->m_hInterface, &bChanged);
                nRet = MV_FG_OpenDeviceByID(pThis->m_hInterface, (char *)pThis->m_chDeviceID, &pThis->m_hDevice);
                if (MV_FG_SUCCESS == nRet)
                {
                    MV_FG_RegisterExceptionCallBack(pThis->m_hDevice, ReconnectDevice, pUser);
                    bConnected = TRUE;
                    pThis->EnableWindowWhenOpenNotStart();
                    break;
                }
                else
                {
                    Sleep(100);
                }
            }

            if (bConnected && pThis->m_bStartGrabbing)
            {
                do
                {
                    // ch:获取流通道个数 | en:Get number of stream
                    unsigned int nStreamNum = 0;
                    nRet = MV_FG_GetNumStreams(pThis->m_hDevice, &nStreamNum);
                    if (MV_FG_SUCCESS != nRet || 0 == nStreamNum)
                    {
                        break;
                    }

                    // ch:打开流通道(目前只支持单个通道) | en:Open stream(Only a single stream is supported now)
                    nRet = MV_FG_OpenStream(pThis->m_hDevice, 0, &pThis->m_hStream);
                    if (MV_FG_SUCCESS != nRet)
                    {
                        break;
                    }

                    // ch:设置SDK内部缓存数量 | en:Set internal buffer number
                    const unsigned int nBufferNum = 3;
                    nRet = MV_FG_SetBufferNum(pThis->m_hStream, nBufferNum);
                    if (MV_FG_SUCCESS != nRet)
                    {
                        break;
                    }

                    nRet = MV_FG_StartAcquisition(pThis->m_hStream);
                    if (MV_FG_SUCCESS != nRet)
                    {
                        break;
                    }

                } while (FALSE);

                // 根据重连后重新开始取流是否成功更改控件状态
                if (MV_FG_SUCCESS == nRet)
                {
                    pThis->EnableWindowWhenStart();
                }
                else
                {
                    pThis->EnableWindowWhenStop();
                }
            }
        }
    }
}

// ch:显示错误消息函数 | en:Display error message function
LRESULT CReconnectDemoDlg::OnDisplayError(WPARAM wParam, LPARAM lParam)
{
    ShowErrorMsg(TEXT("Display failed"), (int)wParam);

    OnBnClickedStopGrabbingButton();

    return 0;
}

BOOL CReconnectDemoDlg::PreTranslateMessage(MSG* pMsg)
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

bool CReconnectDemoDlg::RemoveCustomPixelFormats(MV_FG_PIXEL_TYPE enPixelFormat)
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

void CReconnectDemoDlg::OnBnClickedOpenIfButton()
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

void CReconnectDemoDlg::OnBnClickedCloseIfButton()
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

