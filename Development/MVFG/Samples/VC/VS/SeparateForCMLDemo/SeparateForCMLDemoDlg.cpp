/*
This example shows the how to grab image without communicate with camera through serial port.
*/
#include "stdafx.h"
#include "SeparateForCMLDemo.h"
#include "SeparateForCMLDemoDlg.h"

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

// CSeparateForCMLDemoDlg dialog
CSeparateForCMLDemoDlg::CSeparateForCMLDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSeparateForCMLDemoDlg::IDD, pParent)
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
    , m_nWidth(0)
    , m_nHeight(0)
    , m_strPixelFormat(_T(""))
    , m_strPixelSize(_T(""))
    , m_strTapGeometry(_T(""))
    , m_strClConfiguration(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    memset(m_chDeviceID, 0, MV_FG_MAX_DEV_INFO_SIZE);
}

void CSeparateForCMLDemoDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_IF_COMBO, m_ctrlInterfaceCombo);
    DDX_Control(pDX, IDC_DEVICE_COMBO, m_ctrlDeviceCombo);
    DDX_CBIndex(pDX, IDC_DEVICE_COMBO, m_nDeviceCombo);
    DDX_Text(pDX, IDC_EDIT_WIDTH, m_nWidth);
    DDX_Text(pDX, IDC_EDIT_HEIGHT, m_nHeight);
    DDX_CBString(pDX, IDC_COMBO_PIXELFORMAT, m_strPixelFormat);
    DDX_CBString(pDX, IDC_COMBO_PIXELSIZE, m_strPixelSize);
    DDX_CBString(pDX, IDC_COMBO_TAPGEOMETRY, m_strTapGeometry);
    DDX_CBString(pDX, IDC_COMBO_CLCONFIGURATION, m_strClConfiguration);
}

BEGIN_MESSAGE_MAP(CSeparateForCMLDemoDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	// }}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_ENUM_IF_BUTTON, &CSeparateForCMLDemoDlg::OnBnClickedEnumIfButton)
    ON_BN_CLICKED(IDC_ENUM_DEV_BUTTON, &CSeparateForCMLDemoDlg::OnBnClickedEnumDevButton)
    ON_BN_CLICKED(IDC_OPEN_BUTTON, &CSeparateForCMLDemoDlg::OnBnClickedOpenButton)
    ON_BN_CLICKED(IDC_CLOSE_BUTTON, &CSeparateForCMLDemoDlg::OnBnClickedCloseButton)
    ON_BN_CLICKED(IDC_START_GRABBING_BUTTON, &CSeparateForCMLDemoDlg::OnBnClickedStartGrabbingButton)
    ON_BN_CLICKED(IDC_STOP_GRABBING_BUTTON, &CSeparateForCMLDemoDlg::OnBnClickedStopGrabbingButton)
    ON_WM_CLOSE()
    ON_BN_CLICKED(IDC_OPEN_IF_BUTTON, &CSeparateForCMLDemoDlg::OnBnClickedOpenIfButton)
    ON_BN_CLICKED(IDC_CLOSE_IF_BUTTON, &CSeparateForCMLDemoDlg::OnBnClickedCloseIfButton)
    ON_MESSAGE(WM_DISPLAY_ERROR, &CSeparateForCMLDemoDlg::OnDisplayError)
END_MESSAGE_MAP()

// ch:取流线程 | en:Grabbing thread
unsigned int __stdcall GrabThread(void* pUser)
{
    if (pUser)
    {
        CSeparateForCMLDemoDlg* pCam = (CSeparateForCMLDemoDlg*)pUser;

        pCam->GrabThreadProcess();

        return 0;
    }

    return -1;
}

// CSeparateForCMLDemoDlg message handlers
BOOL CSeparateForCMLDemoDlg::OnInitDialog()
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

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSeparateForCMLDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
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
void CSeparateForCMLDemoDlg::OnPaint()
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
HCURSOR CSeparateForCMLDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// ch:初始化时的窗口显示 | en:Window display when Initial
void CSeparateForCMLDemoDlg::EnableWindowInitial()
{
    GetDlgItem(IDC_OPEN_BUTTON)->EnableWindow(FALSE);
    GetDlgItem(IDC_CLOSE_BUTTON)->EnableWindow(FALSE);
    GetDlgItem(IDC_START_GRABBING_BUTTON)->EnableWindow(FALSE);
    GetDlgItem(IDC_STOP_GRABBING_BUTTON)->EnableWindow(FALSE);
}

// ch:打开设备但不开始抓图 | en:Open device but does not start grabbing*/
void CSeparateForCMLDemoDlg::EnableWindowWhenOpenNotStart()
{
    GetDlgItem(IDC_OPEN_BUTTON)->EnableWindow(FALSE);
    GetDlgItem(IDC_CLOSE_BUTTON)->EnableWindow(TRUE);
    GetDlgItem(IDC_START_GRABBING_BUTTON)->EnableWindow(TRUE);
}

// ch:按下开始采集按钮时的按钮颜色 | en:Button color when the start grabbing button is pressed
void CSeparateForCMLDemoDlg::EnableWindowWhenStart()
{
    GetDlgItem(IDC_STOP_GRABBING_BUTTON)->EnableWindow(TRUE);
    GetDlgItem(IDC_START_GRABBING_BUTTON)->EnableWindow(FALSE);
}

// ch:按下结束采集时的按钮颜色 | en:Button color when the stop grabbing button is pressed
void CSeparateForCMLDemoDlg::EnableWindowWhenStop()
{
    GetDlgItem(IDC_STOP_GRABBING_BUTTON)->EnableWindow(FALSE);
    GetDlgItem(IDC_START_GRABBING_BUTTON)->EnableWindow(TRUE);
}

// ch:按钮使能 | en:Enable control
void CSeparateForCMLDemoDlg::EnableControls(BOOL bIsCameraReady)
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
void CSeparateForCMLDemoDlg::DisplayWindowInitial()
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
void CSeparateForCMLDemoDlg::ShowErrorMsg(CString csMessage, int nErrorNum)
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
    case MV_FG_ERR_IMG_FORMAT:          errorMsg += "Invalid or unsupported image format ";                 break;
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
void CSeparateForCMLDemoDlg::OnBnClickedEnumIfButton()
{
    int nRet = 0;

    // ch:枚举采集卡 | en:Enum interface
    bool bChanged = false;
    nRet = MV_FG_UpdateInterfaceList(MV_FG_CAMERALINK_INTERFACE, &bChanged);
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

            case MV_FG_CAMERALINK_INTERFACE:
                {
                    sprintf_s(strIFInfo, nIFInfoLen, "CML[%d]: %s | %s | %s", i, 
                        stInterfaceInfo.IfaceInfo.stCMLIfaceInfo.chDisplayName, 
                        stInterfaceInfo.IfaceInfo.stCMLIfaceInfo.chInterfaceID, 
                        stInterfaceInfo.IfaceInfo.stCMLIfaceInfo.chSerialNumber);
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
void CSeparateForCMLDemoDlg::OnBnClickedEnumDevButton()
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
            case MV_FG_CAMERALINK_DEVICE:
                {
                    sprintf_s(strDevInfo, nDevInfoLen, "CML[%d]: %s | %s | %s", i, 
                        stDeviceInfo.DevInfo.stCMLDevInfo.chUserDefinedName,
                        stDeviceInfo.DevInfo.stCMLDevInfo.chModelName,
                        stDeviceInfo.DevInfo.stCMLDevInfo.chSerialNumber);
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
int CSeparateForCMLDemoDlg::CloseDevice()
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

int CSeparateForCMLDemoDlg::GrabThreadProcess()
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
int CSeparateForCMLDemoDlg::GetDeviceID(unsigned int nIndex)
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
    case MV_FG_CAMERALINK_DEVICE:
        {
            memcpy(m_chDeviceID, stDevInfo.DevInfo.stCMLDevInfo.chDeviceID, MV_FG_MAX_DEV_INFO_SIZE);
            break;
        }
    default:
        {
            return MV_FG_ERR_INVALID_ID;
        }
    }

    return nRet;
}

int CSeparateForCMLDemoDlg::InitInterfaceParam()
{
    int nRet = MV_FG_SUCCESS;

    MV_FG_INTVALUE stInvalue = {0};
    MV_FG_ENUMVALUE stEnumValue = {0};
    nRet = MV_FG_GetIntValue(m_hInterface, "CameraWidth", &stInvalue);
    if (MV_FG_SUCCESS != nRet)
    {
        return nRet;
    }
    m_nWidth = stInvalue.nCurValue;

    nRet = MV_FG_GetIntValue(m_hInterface, "CameraHeight", &stInvalue);
    if (MV_FG_SUCCESS != nRet)
    {
        return nRet;
    }
    m_nHeight = stInvalue.nCurValue;

    nRet = MV_FG_GetEnumValue(m_hInterface, "CameraPixelFormat", &stEnumValue);
    if (MV_FG_SUCCESS != nRet)
    {
        return nRet;
    }
    CComboBox* pComBox = (CComboBox*)GetDlgItem(IDC_COMBO_PIXELFORMAT);
    pComBox->ResetContent();
    for (unsigned int i = 0; i < stEnumValue.nSupportedNum; i++)
    {
        pComBox->AddString((CString)stEnumValue.strSymbolic[i]);
    }
    m_strPixelFormat = (CString)(stEnumValue.strCurSymbolic);

    nRet = MV_FG_GetEnumValue(m_hInterface, "CameraPixelSize", &stEnumValue);
    if (MV_FG_SUCCESS != nRet)
    {
        return nRet;
    }
    pComBox = (CComboBox*)GetDlgItem(IDC_COMBO_PIXELSIZE);
    pComBox->ResetContent();
    for (unsigned int i = 0; i < stEnumValue.nSupportedNum; i++)
    {
        pComBox->AddString((CString)stEnumValue.strSymbolic[i]);
    }
    m_strPixelSize = (CString)(stEnumValue.strCurSymbolic);

    nRet = MV_FG_GetEnumValue(m_hInterface, "CameraTapGeometry", &stEnumValue);
    if (MV_FG_SUCCESS != nRet)
    {
        return nRet;
    }
    pComBox = (CComboBox*)GetDlgItem(IDC_COMBO_TAPGEOMETRY);
    pComBox->ResetContent();
    for (unsigned int i = 0; i < stEnumValue.nSupportedNum; i++)
    {
        pComBox->AddString((CString)stEnumValue.strSymbolic[i]);
    }
    m_strTapGeometry = (CString)(stEnumValue.strCurSymbolic);

    nRet = MV_FG_GetEnumValue(m_hInterface, "CameraClConfiguration", &stEnumValue);
    if (MV_FG_SUCCESS != nRet)
    {
        return nRet;
    }
    pComBox = (CComboBox*)GetDlgItem(IDC_COMBO_CLCONFIGURATION);
    pComBox->ResetContent();
    for (unsigned int i = 0; i < stEnumValue.nSupportedNum; i++)
    {
        pComBox->AddString((CString)stEnumValue.strSymbolic[i]);
    }
    m_strClConfiguration = (CString)(stEnumValue.strCurSymbolic);

    UpdateData(FALSE);

    return MV_FG_SUCCESS;
}

// ch:按下打开设备按钮：打开设备 | en:Click Open button: Open Device
void CSeparateForCMLDemoDlg::OnBnClickedOpenButton()
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

    m_bOpenDevice = TRUE;
    m_nDeviceIndex = m_nDeviceCombo;

    UpdateData(FALSE);
    EnableControls(TRUE);
}

// ch:按下关闭设备按钮：关闭设备 | en:Click Close button: Close Device
void CSeparateForCMLDemoDlg::OnBnClickedCloseButton()
{
    CloseDevice();
    EnableControls(TRUE);
}

// ch:按下开始采集按钮 | en:Click Start button
void CSeparateForCMLDemoDlg::OnBnClickedStartGrabbingButton()
{
    if (FALSE == m_bOpenDevice || TRUE == m_bStartGrabbing)
    {
        return;
    }

    UpdateData(TRUE);

    int nRet = 0;

    char chValue[64] = {0};

    // ch:根据相机参数配置采集卡端的输出图像参数 | en:Configure output image parameters.
    nRet = MV_FG_SetIntValue(m_hInterface, "CameraWidth", m_nWidth);
    if (MV_FG_SUCCESS != nRet)
    {
        ShowErrorMsg(TEXT("Set CameraWidth failed"), nRet);
        return;
    }

    nRet = MV_FG_SetIntValue(m_hInterface, "CameraHeight", m_nHeight);
    if (MV_FG_SUCCESS != nRet)
    {
        ShowErrorMsg(TEXT("Set CameraHeight failed"), nRet);
        return;
    }

    int nLen = WideCharToMultiByte(CP_ACP, 0, m_strPixelFormat, m_strPixelFormat.GetLength(), NULL, 0, NULL,NULL);
    WideCharToMultiByte(CP_ACP,0, m_strPixelFormat, m_strPixelFormat.GetLength(), chValue, nLen,NULL,NULL);
    chValue[nLen] = '\0';
    nRet = MV_FG_SetEnumValueByString(m_hInterface, "CameraPixelFormat", chValue);
    if (MV_FG_SUCCESS != nRet)
    {
        ShowErrorMsg(TEXT("Set CameraPixelFormat failed"), nRet);
        return;
    }

    nLen = WideCharToMultiByte(CP_ACP, 0, m_strPixelSize, m_strPixelSize.GetLength(), NULL, 0, NULL,NULL);
    WideCharToMultiByte(CP_ACP,0, m_strPixelSize, m_strPixelSize.GetLength(), chValue, nLen,NULL,NULL);
    chValue[nLen] = '\0';
    nRet = MV_FG_SetEnumValueByString(m_hInterface, "CameraPixelSize", chValue);
    if (MV_FG_SUCCESS != nRet)
    {
        ShowErrorMsg(TEXT("Set CameraPixelSize failed"), nRet);
        return;
    }

    nLen = WideCharToMultiByte(CP_ACP, 0, m_strTapGeometry, m_strTapGeometry.GetLength(), NULL, 0, NULL,NULL);
    WideCharToMultiByte(CP_ACP,0, m_strTapGeometry, m_strTapGeometry.GetLength(), chValue, nLen,NULL,NULL);
    chValue[nLen] = '\0';
    nRet = MV_FG_SetEnumValueByString(m_hInterface, "CameraTapGeometry", chValue);
    if (MV_FG_SUCCESS != nRet)
    {
        ShowErrorMsg(TEXT("Set CameraTapGeometry failed"), nRet);
        return;
    }

    nLen = WideCharToMultiByte(CP_ACP, 0, m_strClConfiguration, m_strClConfiguration.GetLength(), NULL, 0, NULL,NULL);
    WideCharToMultiByte(CP_ACP,0, m_strClConfiguration, m_strClConfiguration.GetLength(), chValue, nLen,NULL,NULL);
    chValue[nLen] = '\0';
    nRet = MV_FG_SetEnumValueByString(m_hInterface, "CameraClConfiguration", chValue);
    if (MV_FG_SUCCESS != nRet)
    {
        ShowErrorMsg(TEXT("Set CameraClConfiguration failed"), nRet);
        return;
    }

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
void CSeparateForCMLDemoDlg::OnBnClickedStopGrabbingButton()
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
void CSeparateForCMLDemoDlg::OnClose()
{
    PostQuitMessage(0);
    CloseDevice();

    if (m_hInterface)
    {
        MV_FG_CloseInterface(m_hInterface);
        m_hInterface = NULL;
    }

    CDialog::OnClose();
}

// ch:显示错误消息函数 | en:Display error message function
LRESULT CSeparateForCMLDemoDlg::OnDisplayError(WPARAM wParam, LPARAM lParam)
{
    ShowErrorMsg(TEXT("Display failed"), (int)wParam);

    OnBnClickedStopGrabbingButton();

    return 0;
}

BOOL CSeparateForCMLDemoDlg::PreTranslateMessage(MSG* pMsg)
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

bool CSeparateForCMLDemoDlg::RemoveCustomPixelFormats(MV_FG_PIXEL_TYPE enPixelFormat)
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

void CSeparateForCMLDemoDlg::OnBnClickedOpenIfButton()
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

    // ch:开启采集卡上的分离方案使能 | en:Enable seperate scheme on interface.
    nRet = MV_FG_SetBoolValue(m_hInterface, "CameraDeviceEnable", true);
    if (MV_FG_SUCCESS != nRet)
    {
        MV_FG_CloseInterface(m_hInterface);
        m_hInterface = NULL;

        ShowErrorMsg(_TEXT("Set CameraDeviceEnable failed"), nRet);
        return;
    }

    // ch:初始化界面参数 | en:Init interface params.
    InitInterfaceParam();

    m_bOpenIF = TRUE;

    EnableControls(FALSE);
}

void CSeparateForCMLDemoDlg::OnBnClickedCloseIfButton()
{
    int nRet = 0;

    m_bOpenIF = FALSE;
    CloseDevice();

    // ch:关闭采集卡 | en:Close interface
    if (m_hInterface)
    {
        // ch:关闭采集卡上的分离方案使能 | en:Disable seperate scheme on interface.
        MV_FG_SetBoolValue(m_hInterface, "CameraDeviceEnable", false);

        nRet = MV_FG_CloseInterface(m_hInterface);
        if (MV_FG_SUCCESS != nRet)
        {
            ShowErrorMsg(_TEXT("Close interface failed"), nRet);
        }
        m_hInterface = NULL;
    }

    EnableControls(FALSE);
}

