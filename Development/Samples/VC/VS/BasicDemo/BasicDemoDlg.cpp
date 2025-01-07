
// BasicDemoDlg.cpp : implementation file
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
    , m_pcMyCamera(NULL)
    , m_nDeviceCombo(0)
    , m_bOpenDevice(FALSE)
    , m_bStartGrabbing(FALSE)
    , m_hGrabThread(NULL)
    , m_bThreadState(FALSE)
    , m_nTriggerMode(MV_TRIGGER_MODE_OFF)
    , m_dExposureEdit(0)
    , m_dGainEdit(0)
    , m_dFrameRateEdit(0)
    , m_bSoftWareTriggerCheck(FALSE)
    , m_nTriggerSource(MV_TRIGGER_SOURCE_SOFTWARE)
    , m_pSaveImageBuf(NULL)
    , m_nSaveImageBufSize(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    memset(m_chPixelFormat, 0, MV_MAX_SYMBOLIC_LEN);
    memset(&m_stImageInfo, 0, sizeof(MV_FRAME_OUT_INFO_EX));
}

void CBasicDemoDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     DDX_Control(pDX, IDC_DEVICE_COMBO, m_ctrlDeviceCombo);
     DDX_CBIndex(pDX, IDC_DEVICE_COMBO, m_nDeviceCombo);
     DDX_Text(pDX, IDC_EXPOSURE_EDIT, m_dExposureEdit);
     DDX_Text(pDX, IDC_GAIN_EDIT, m_dGainEdit);
     DDX_Text(pDX, IDC_FRAME_RATE_EDIT, m_dFrameRateEdit);
     DDX_Text(pDX, IDC_PIXEL_FORMAT_EDIT, (CString)(m_chPixelFormat));
     DDX_Check(pDX, IDC_SOFTWARE_TRIGGER_CHECK, m_bSoftWareTriggerCheck);
}

BEGIN_MESSAGE_MAP(CBasicDemoDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	// }}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_ENUM_BUTTON, &CBasicDemoDlg::OnBnClickedEnumButton)
    ON_BN_CLICKED(IDC_OPEN_BUTTON, &CBasicDemoDlg::OnBnClickedOpenButton)
    ON_BN_CLICKED(IDC_CLOSE_BUTTON, &CBasicDemoDlg::OnBnClickedCloseButton)
    ON_BN_CLICKED(IDC_CONTINUS_MODE_RADIO, &CBasicDemoDlg::OnBnClickedContinusModeRadio)
    ON_BN_CLICKED(IDC_TRIGGER_MODE_RADIO, &CBasicDemoDlg::OnBnClickedTriggerModeRadio)
    ON_BN_CLICKED(IDC_START_GRABBING_BUTTON, &CBasicDemoDlg::OnBnClickedStartGrabbingButton)
    ON_BN_CLICKED(IDC_STOP_GRABBING_BUTTON, &CBasicDemoDlg::OnBnClickedStopGrabbingButton)
    ON_BN_CLICKED(IDC_GET_PARAMETER_BUTTON, &CBasicDemoDlg::OnBnClickedGetParameterButton)
    ON_BN_CLICKED(IDC_SET_PARAMETER_BUTTON, &CBasicDemoDlg::OnBnClickedSetParameterButton)
    ON_BN_CLICKED(IDC_SOFTWARE_TRIGGER_CHECK, &CBasicDemoDlg::OnBnClickedSoftwareTriggerCheck)
    ON_BN_CLICKED(IDC_SOFTWARE_ONCE_BUTTON, &CBasicDemoDlg::OnBnClickedSoftwareOnceButton)
    ON_BN_CLICKED(IDC_SAVE_BMP_BUTTON, &CBasicDemoDlg::OnBnClickedSaveBmpButton)
    ON_BN_CLICKED(IDC_SAVE_JPG_BUTTON, &CBasicDemoDlg::OnBnClickedSaveJpgButton)
    ON_BN_CLICKED(IDC_SAVE_TIFF_BUTTON, &CBasicDemoDlg::OnBnClickedSaveTiffButton)
    ON_BN_CLICKED(IDC_SAVE_PNG_BUTTON, &CBasicDemoDlg::OnBnClickedSavePngButton)
    ON_BN_CLICKED(IDC_CIRCLE_AUXILIARY_BUTTON, &CBasicDemoDlg::OnBnClickedCircleAuxiliaryButton)
    ON_BN_CLICKED(IDC_LINES_AUXILIARY_BUTTON, &CBasicDemoDlg::OnBnClickedLinesAuxiliaryButton)
    ON_WM_CLOSE()
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
    GetDlgItem(IDC_SOFTWARE_TRIGGER_CHECK)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);
    GetDlgItem(IDC_SOFTWARE_ONCE_BUTTON)->EnableWindow((m_bStartGrabbing && m_bSoftWareTriggerCheck && ((CButton *)GetDlgItem(IDC_TRIGGER_MODE_RADIO))->GetCheck())? TRUE : FALSE);
    GetDlgItem(IDC_SAVE_BMP_BUTTON)->EnableWindow(m_bStartGrabbing ? TRUE : FALSE);
    GetDlgItem(IDC_SAVE_TIFF_BUTTON)->EnableWindow(m_bStartGrabbing ? TRUE : FALSE);
    GetDlgItem(IDC_SAVE_PNG_BUTTON)->EnableWindow(m_bStartGrabbing ? TRUE : FALSE);
    GetDlgItem(IDC_SAVE_JPG_BUTTON)->EnableWindow(m_bStartGrabbing ? TRUE : FALSE);
    GetDlgItem(IDC_EXPOSURE_EDIT)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);
    GetDlgItem(IDC_GAIN_EDIT)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);
    GetDlgItem(IDC_FRAME_RATE_EDIT)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);
    GetDlgItem(IDC_PIXEL_FORMAT_EDIT)->EnableWindow(FALSE);
    GetDlgItem(IDC_GET_PARAMETER_BUTTON)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);
    GetDlgItem(IDC_SET_PARAMETER_BUTTON)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);
    GetDlgItem(IDC_CONTINUS_MODE_RADIO)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);
    GetDlgItem(IDC_TRIGGER_MODE_RADIO)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);
    GetDlgItem(IDC_CIRCLE_AUXILIARY_BUTTON)->EnableWindow(m_bStartGrabbing ? TRUE : FALSE);
    GetDlgItem(IDC_LINES_AUXILIARY_BUTTON)->EnableWindow(m_bStartGrabbing ? TRUE : FALSE);
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

    return MV_OK;
}

// ch:获取触发模式 | en:Get Trigger Mode
int CBasicDemoDlg::GetTriggerMode()
{
    MVCC_ENUMVALUE stEnumValue = {0};

    int nRet = m_pcMyCamera->GetEnumValue("TriggerMode", &stEnumValue);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    m_nTriggerMode = stEnumValue.nCurValue;
    if (MV_TRIGGER_MODE_ON ==  m_nTriggerMode)
    {
        OnBnClickedTriggerModeRadio();
    }
    else
    {
        m_nTriggerMode = MV_TRIGGER_MODE_OFF;
        OnBnClickedContinusModeRadio();
    }

    return MV_OK;
}

// ch:设置触发模式 | en:Set Trigger Mode
int CBasicDemoDlg::SetTriggerMode()
{
    return m_pcMyCamera->SetEnumValue("TriggerMode", m_nTriggerMode);
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

// ch:获取增益 | en:Get Gain
int CBasicDemoDlg::GetGain()
{
    MVCC_FLOATVALUE stFloatValue = {0};

    int nRet = m_pcMyCamera->GetFloatValue("Gain", &stFloatValue);
    if (MV_OK != nRet)
    {
        return nRet;
    }
    m_dGainEdit = stFloatValue.fCurValue;

    return MV_OK;
}

// ch:设置增益 | en:Set Gain
int CBasicDemoDlg::SetGain()
{
    // ch:设置增益前先把自动增益关闭，失败无需返回
    //en:Set Gain after Auto Gain is turned off, this failure does not need to return
    m_pcMyCamera->SetEnumValue("GainAuto", 0);

    return m_pcMyCamera->SetFloatValue("Gain", (float)m_dGainEdit);
}

// ch:获取帧率 | en:Get Frame Rate
int CBasicDemoDlg::GetFrameRate()
{
    MVCC_FLOATVALUE stFloatValue = {0};

    int nRet = m_pcMyCamera->GetFloatValue("ResultingFrameRate", &stFloatValue);
    if (MV_OK != nRet)
    {
        return nRet;
    }
    m_dFrameRateEdit = stFloatValue.fCurValue;

    return MV_OK;
}

// ch:设置帧率 | en:Set Frame Rate
int CBasicDemoDlg::SetFrameRate()
{
    int nRet = m_pcMyCamera->SetBoolValue("AcquisitionFrameRateEnable", true);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    return m_pcMyCamera->SetFloatValue("AcquisitionFrameRate", (float)m_dFrameRateEdit);
}

// ch:获取触发源 | en:Get Trigger Source
int CBasicDemoDlg::GetTriggerSource()
{
    MVCC_ENUMVALUE stEnumValue = {0};

    int nRet = m_pcMyCamera->GetEnumValue("TriggerSource", &stEnumValue);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    if ((unsigned int)MV_TRIGGER_SOURCE_SOFTWARE == stEnumValue.nCurValue)
    {
        m_bSoftWareTriggerCheck = TRUE;
    }
    else
    {
        m_bSoftWareTriggerCheck = FALSE;
    }

    return MV_OK;
}

// ch:设置触发源 | en:Set Trigger Source
int CBasicDemoDlg::SetTriggerSource()
{
    int nRet = MV_OK;
    if (m_bSoftWareTriggerCheck)
    {
        m_nTriggerSource = MV_TRIGGER_SOURCE_SOFTWARE;
        nRet = m_pcMyCamera->SetEnumValue("TriggerSource", m_nTriggerSource);
        if (MV_OK != nRet)
        {
            ShowErrorMsg(TEXT("Set Software Trigger Fail"), nRet);
            return nRet;
        }
        GetDlgItem(IDC_SOFTWARE_ONCE_BUTTON )->EnableWindow(TRUE);
    }
    else
    {
        m_nTriggerSource = MV_TRIGGER_SOURCE_LINE0;
        nRet = m_pcMyCamera->SetEnumValue("TriggerSource", m_nTriggerSource);
        if (MV_OK != nRet)
        {
            ShowErrorMsg(TEXT("Set Hardware Trigger Fail"), nRet);
            return nRet;
        }
        GetDlgItem(IDC_SOFTWARE_ONCE_BUTTON )->EnableWindow(FALSE);
    }

    return nRet;
}

// 该接口只展示GetEnumEntrySymbolic接口的使用方法
int CBasicDemoDlg::GetPixelFormat()
{
    MVCC_ENUMVALUE stEnumValue = {0};
    MVCC_ENUMENTRY stPixelFormatInfo = {0};

    int nRet = m_pcMyCamera->GetEnumValue("PixelFormat", &stEnumValue);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    stPixelFormatInfo.nValue = stEnumValue.nCurValue;
    nRet = m_pcMyCamera->GetEnumEntrySymbolic("PixelFormat", &stPixelFormatInfo);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    strcpy_s(m_chPixelFormat, MV_MAX_SYMBOLIC_LEN, stPixelFormatInfo.chSymbolic);

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
    int nRet = MV_OK;

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
            //用于保存图片
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
	int nRet = CMvCamera::EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE | MV_GENTL_GIGE_DEVICE | MV_GENTL_CAMERALINK_DEVICE | 
										MV_GENTL_CXP_DEVICE | MV_GENTL_XOF_DEVICE ,&m_stDevList);
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

		char strUserName[256] = {0};
        wchar_t* pUserName = NULL;
        if (pDeviceInfo->nTLayerType == MV_GIGE_DEVICE)
        {
            int nIp1 = ((m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24);
            int nIp2 = ((m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16);
            int nIp3 = ((m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8);
            int nIp4 = (m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

            if (strcmp("", (LPCSTR)(pDeviceInfo->SpecialInfo.stGigEInfo.chUserDefinedName)) != 0)
            {
				memset(strUserName,0,256);
				sprintf_s(strUserName, 256, "%s (%s)", pDeviceInfo->SpecialInfo.stGigEInfo.chUserDefinedName,
					pDeviceInfo->SpecialInfo.stGigEInfo.chSerialNumber);
                DWORD dwLenUserName = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strUserName), -1, NULL, 0);
                pUserName = new wchar_t[dwLenUserName];
                MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strUserName), -1, pUserName, dwLenUserName);
            }
            else
            {
                memset(strUserName,0,256);
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
                memset(strUserName,0,256);
				sprintf_s(strUserName, 256, "%s (%s)", pDeviceInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName,
					pDeviceInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);
                DWORD dwLenUserName = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strUserName), -1, NULL, 0);
                pUserName = new wchar_t[dwLenUserName];
                MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strUserName), -1, pUserName, dwLenUserName);
            }
            else
            {
                memset(strUserName,0,256);
                sprintf_s(strUserName, 256, "%s (%s)", pDeviceInfo->SpecialInfo.stUsb3VInfo.chModelName,
                    pDeviceInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);
                DWORD dwLenUserName = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strUserName), -1, NULL, 0);
                pUserName = new wchar_t[dwLenUserName];
                MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strUserName), -1, pUserName, dwLenUserName);
            }
            strMsg.Format(_T("[%d]UsbV3:  %s"), i, pUserName);
        }
		else if (pDeviceInfo->nTLayerType == MV_GENTL_CAMERALINK_DEVICE)
		{
			if (strcmp("", (char*)pDeviceInfo->SpecialInfo.stCMLInfo.chUserDefinedName) != 0)
			{
				memset(strUserName,0,256);
				sprintf_s(strUserName, 256, "%s (%s)", pDeviceInfo->SpecialInfo.stCMLInfo.chUserDefinedName,
					pDeviceInfo->SpecialInfo.stCMLInfo.chSerialNumber);
				DWORD dwLenUserName = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strUserName), -1, NULL, 0);
				pUserName = new wchar_t[dwLenUserName];
				MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strUserName), -1, pUserName, dwLenUserName);
			}
			else
			{
				memset(strUserName,0,256);
				sprintf_s(strUserName, 256, "%s (%s)", pDeviceInfo->SpecialInfo.stCMLInfo.chModelName,
					pDeviceInfo->SpecialInfo.stCMLInfo.chSerialNumber);
				DWORD dwLenUserName = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strUserName), -1, NULL, 0);
				pUserName = new wchar_t[dwLenUserName];
				MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strUserName), -1, pUserName, dwLenUserName);
			}
			strMsg.Format(_T("[%d]CML:  %s"), i, pUserName);
		}
		else if (pDeviceInfo->nTLayerType == MV_GENTL_CXP_DEVICE)
		{
			if (strcmp("", (char*)pDeviceInfo->SpecialInfo.stCXPInfo.chUserDefinedName) != 0)
			{
				memset(strUserName,0,256);
				sprintf_s(strUserName, 256, "%s (%s)", pDeviceInfo->SpecialInfo.stCXPInfo.chUserDefinedName,
					pDeviceInfo->SpecialInfo.stCXPInfo.chSerialNumber);
				DWORD dwLenUserName = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strUserName), -1, NULL, 0);
				pUserName = new wchar_t[dwLenUserName];
				MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strUserName), -1, pUserName, dwLenUserName);
			}
			else
			{
				memset(strUserName,0,256);
				sprintf_s(strUserName, 256, "%s (%s)", pDeviceInfo->SpecialInfo.stCXPInfo.chModelName,
					pDeviceInfo->SpecialInfo.stCXPInfo.chSerialNumber);
				DWORD dwLenUserName = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strUserName), -1, NULL, 0);
				pUserName = new wchar_t[dwLenUserName];
				MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strUserName), -1, pUserName, dwLenUserName);
			}
			strMsg.Format(_T("[%d]CXP:  %s"), i, pUserName);
		}
		else if (pDeviceInfo->nTLayerType == MV_GENTL_XOF_DEVICE)
		{
			if (strcmp("", (char*)pDeviceInfo->SpecialInfo.stXoFInfo.chUserDefinedName) != 0)
			{
				memset(strUserName,0,256);
				sprintf_s(strUserName, 256, "%s (%s)", pDeviceInfo->SpecialInfo.stXoFInfo.chUserDefinedName,
					pDeviceInfo->SpecialInfo.stXoFInfo.chSerialNumber);
				DWORD dwLenUserName = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strUserName), -1, NULL, 0);
				pUserName = new wchar_t[dwLenUserName];
				MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strUserName), -1, pUserName, dwLenUserName);
			}
			else
			{
				memset(strUserName,0,256);
				sprintf_s(strUserName, 256, "%s (%s)", pDeviceInfo->SpecialInfo.stXoFInfo.chModelName,
					pDeviceInfo->SpecialInfo.stXoFInfo.chSerialNumber);
				DWORD dwLenUserName = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strUserName), -1, NULL, 0);
				pUserName = new wchar_t[dwLenUserName];
				MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strUserName), -1, pUserName, dwLenUserName);
			}
			strMsg.Format(_T("[%d]CXP:  %s"), i, pUserName);
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
    OnBnClickedGetParameterButton(); // ch:获取参数 | en:Get Parameter
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
    ((CButton *)GetDlgItem(IDC_SOFTWARE_TRIGGER_CHECK))->EnableWindow(FALSE);
    m_nTriggerMode = MV_TRIGGER_MODE_OFF;
    int nRet = SetTriggerMode();
    if (MV_OK != nRet)
    {
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
    ((CButton *)GetDlgItem(IDC_SOFTWARE_TRIGGER_CHECK))->EnableWindow(TRUE);
    m_nTriggerMode = MV_TRIGGER_MODE_ON;
    int nRet = SetTriggerMode();
    if (MV_OK != nRet)
    {
        ShowErrorMsg(TEXT("Set Trigger Mode Fail"), nRet);
        return;
    }

    if (m_bStartGrabbing == TRUE)
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
}

// ch:按下获取参数按钮 | en:Click Get Parameter button
void CBasicDemoDlg::OnBnClickedGetParameterButton()
{
    int nRet = GetTriggerMode();
    if (nRet != MV_OK)
    {
        ShowErrorMsg(TEXT("Get Trigger Mode Fail"), nRet);
    }

    nRet = GetExposureTime();
    if (nRet != MV_OK)
    {
        ShowErrorMsg(TEXT("Get Exposure Time Fail"), nRet);
    }

    nRet = GetGain();
    if (nRet != MV_OK)
    {
        ShowErrorMsg(TEXT("Get Gain Fail"), nRet);
    }

    nRet = GetFrameRate();
    if (nRet != MV_OK)
    {
        ShowErrorMsg(TEXT("Get Frame Rate Fail"), nRet);
    }

    nRet = GetTriggerSource();
    if (nRet != MV_OK)
    {
        ShowErrorMsg(TEXT("Get Trigger Source Fail"), nRet);
    }

    nRet = GetPixelFormat();
    if (nRet != MV_OK)
    {
        ShowErrorMsg(TEXT("Get Pixel Format Fail"), nRet);
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
    nRet = SetGain();
    if (nRet != MV_OK)
    {
        bIsSetSucceed = false;
        ShowErrorMsg(TEXT("Set Gain Fail"), nRet);
    }
    nRet = SetFrameRate();
    if (nRet != MV_OK)
    {
        bIsSetSucceed = false;
        ShowErrorMsg(TEXT("Set Frame Rate Fail"), nRet);
    }
    
    if (true == bIsSetSucceed)
    {
        ShowErrorMsg(TEXT("Set Parameter Succeed"), nRet);
    }
}

// ch:按下软触发按钮 | en:Click Software button
void CBasicDemoDlg::OnBnClickedSoftwareTriggerCheck()
{
    UpdateData(TRUE);

    int nRet = SetTriggerSource();
    if (nRet != MV_OK)
    {
        return;
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

// ch:辅助线绘制 | en:Auxiliary Line Drawing
void CBasicDemoDlg::OnBnClickedCircleAuxiliaryButton()
{
    MVCC_CIRCLE_INFO stCircleInfo = {0};    // 圆形辅助线信息

    stCircleInfo.fR1 = 0.5;                 // 横向半径
    stCircleInfo.fR2 = 0.5;                 // 纵向半径
    stCircleInfo.nLineWidth = 2;            // 辅助线线宽
    stCircleInfo.stCenterPoint.fX = 0.5;    // 圆心横坐标
    stCircleInfo.stCenterPoint.fY = 0.5;    // 圆心纵坐标
    stCircleInfo.stColor.fR = 1.0;
    stCircleInfo.stColor.fG = 0;
    stCircleInfo.stColor.fB = 0;
    stCircleInfo.stColor.fAlpha = 0;

    int nRet = m_pcMyCamera->DrawCircle(&stCircleInfo);
    if (MV_OK != nRet)
    {
        ShowErrorMsg(TEXT("Draw circle auxiliary line fail"), nRet);
    }
}

void CBasicDemoDlg::OnBnClickedLinesAuxiliaryButton()
{
    MVCC_LINES_INFO stLinesInfo = {0};

    stLinesInfo.nLineWidth = 2;
    stLinesInfo.stStartPoint.fX = 0;
    stLinesInfo.stStartPoint.fY = 0;
    stLinesInfo.stEndPoint.fX = 1.0;
    stLinesInfo.stEndPoint.fY = 1.0;
    stLinesInfo.stColor.fR = 0;
    stLinesInfo.stColor.fG = 1;
    stLinesInfo.stColor.fB = 0;
    stLinesInfo.stColor.fAlpha = 0.5;

    int nRet = m_pcMyCamera->DrawLines(&stLinesInfo);
    if (MV_OK != nRet)
    {
        ShowErrorMsg(TEXT("Draw lines auxiliary line fail"), nRet);
    }
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

