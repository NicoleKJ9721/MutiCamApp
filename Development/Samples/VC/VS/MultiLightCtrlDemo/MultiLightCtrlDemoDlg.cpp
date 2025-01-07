
// BasicDemoDlg.cpp : implementation file
#include "stdafx.h"
#include "MultiLightCtrlDemo.h"
#include "MultiLightCtrlDemoDlg.h"

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

// CReconstructImageDemoDlg dialog
CReconstructImageDemoDlg::CReconstructImageDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CReconstructImageDemoDlg::IDD, pParent)
    , m_pcMyCamera(NULL)
    , m_nDeviceCombo(0)
	,m_nMultiLightCombo(0)
	,m_nMultiLightNum(0)
    , m_bOpenDevice(FALSE)
    , m_bStartGrabbing(FALSE)
    , m_hGrabThread(NULL)
    , m_bThreadState(FALSE)
    , m_nTriggerMode(MV_TRIGGER_MODE_OFF)
    , m_bSoftWareTriggerCheck(FALSE)
    , m_nTriggerSource(MV_TRIGGER_SOURCE_SOFTWARE)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_mapMultiLight.clear();
}

void CReconstructImageDemoDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_DEVICE_COMBO, m_ctrlDeviceCombo);
	DDX_Control(pDX, IDC_MULTILIGHT_COMBO, m_ctrlMultiLightCombo);
    DDX_CBIndex(pDX, IDC_DEVICE_COMBO, m_nDeviceCombo);
    DDX_Check(pDX, IDC_SOFTWARE_TRIGGER_CHECK, m_bSoftWareTriggerCheck);
}

BEGIN_MESSAGE_MAP(CReconstructImageDemoDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	// }}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_ENUM_DEV_BUTTON, &CReconstructImageDemoDlg::OnBnClickedEnumDevButton)
    ON_BN_CLICKED(IDC_OPEN_BUTTON, &CReconstructImageDemoDlg::OnBnClickedOpenButton)
    ON_BN_CLICKED(IDC_CLOSE_BUTTON, &CReconstructImageDemoDlg::OnBnClickedCloseButton)
    ON_BN_CLICKED(IDC_CONTINUS_MODE_RADIO, &CReconstructImageDemoDlg::OnBnClickedContinusModeRadio)
    ON_BN_CLICKED(IDC_TRIGGER_MODE_RADIO, &CReconstructImageDemoDlg::OnBnClickedTriggerModeRadio)
    ON_BN_CLICKED(IDC_START_GRABBING_BUTTON, &CReconstructImageDemoDlg::OnBnClickedStartGrabbingButton)
    ON_BN_CLICKED(IDC_STOP_GRABBING_BUTTON, &CReconstructImageDemoDlg::OnBnClickedStopGrabbingButton)
    ON_BN_CLICKED(IDC_SOFTWARE_TRIGGER_CHECK, &CReconstructImageDemoDlg::OnBnClickedSoftwareTriggerCheck)
    ON_BN_CLICKED(IDC_SOFTWARE_ONCE_BUTTON, &CReconstructImageDemoDlg::OnBnClickedSoftwareOnceButton)
    ON_WM_CLOSE()
	ON_CBN_SELCHANGE(IDC_MULTILIGHT_COMBO, &CReconstructImageDemoDlg::OnCbnSelchangeMultilightCombo)
END_MESSAGE_MAP()

// ch:取流线程 | en:Grabbing thread
unsigned int __stdcall GrabThread(void* pUser)
{
    if (pUser)
    {
        CReconstructImageDemoDlg* pCam = (CReconstructImageDemoDlg*)pUser;

        pCam->GrabThreadProcess();
        
        return 0;
    }

    return -1;
}

// CReconstructImageDemoDlg message handlers
BOOL CReconstructImageDemoDlg::OnInitDialog()
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

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CReconstructImageDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
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
void CReconstructImageDemoDlg::OnPaint()
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
HCURSOR CReconstructImageDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// ch:按钮使能 | en:Enable control
void CReconstructImageDemoDlg::EnableControls(BOOL bIsCameraReady)
{
    GetDlgItem(IDC_OPEN_BUTTON)->EnableWindow(m_bOpenDevice ? FALSE : (bIsCameraReady ? TRUE : FALSE));
    GetDlgItem(IDC_CLOSE_BUTTON)->EnableWindow((m_bOpenDevice && bIsCameraReady) ? TRUE : FALSE);
    GetDlgItem(IDC_START_GRABBING_BUTTON)->EnableWindow((m_bStartGrabbing && bIsCameraReady) ? FALSE : (m_bOpenDevice ? TRUE : FALSE));
    GetDlgItem(IDC_STOP_GRABBING_BUTTON)->EnableWindow(m_bStartGrabbing ? TRUE : FALSE);
    GetDlgItem(IDC_SOFTWARE_TRIGGER_CHECK)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);
    GetDlgItem(IDC_SOFTWARE_ONCE_BUTTON)->EnableWindow((m_bStartGrabbing && m_bSoftWareTriggerCheck && ((CButton *)GetDlgItem(IDC_TRIGGER_MODE_RADIO))->GetCheck())? TRUE : FALSE);
    GetDlgItem(IDC_CONTINUS_MODE_RADIO)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);
    GetDlgItem(IDC_TRIGGER_MODE_RADIO)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);
	GetDlgItem(IDC_MULTILIGHT_COMBO)->EnableWindow(m_bOpenDevice && !m_bStartGrabbing ? TRUE : FALSE);
}

// ch:最开始时的窗口初始化 | en:Initial window initialization
void CReconstructImageDemoDlg::DisplayWindowInitial()
{
    CWnd *pWnd = GetDlgItem(IDC_DISPLAY1_STATIC);
    if (pWnd)
    {
        m_hwndDisplay[0] = pWnd->GetSafeHwnd();
        if (m_hwndDisplay[0])
        {
            EnableControls(FALSE);
        }
    }

    pWnd = GetDlgItem(IDC_DISPLAY2_STATIC);
    if (pWnd)
    {
        m_hwndDisplay[1] = pWnd->GetSafeHwnd();
        if (m_hwndDisplay[1])
        {
            EnableControls(FALSE);
        }
    }

	pWnd = GetDlgItem(IDC_DISPLAY3_STATIC);
	if (pWnd)
	{
		m_hwndDisplay[2] = pWnd->GetSafeHwnd();
		if (m_hwndDisplay[2])
		{
			EnableControls(FALSE);
		}
	}

	pWnd = GetDlgItem(IDC_DISPLAY4_STATIC);
	if (pWnd)
	{
		m_hwndDisplay[3] = pWnd->GetSafeHwnd();
		if (m_hwndDisplay[3])
		{
			EnableControls(FALSE);
		}
	}
}

// ch:显示错误信息 | en:Show error message
void CReconstructImageDemoDlg::ShowErrorMsg(CString csMessage, int nErrorNum)
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
int CReconstructImageDemoDlg::CloseDevice()
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
	m_mapMultiLight.clear();

    return MV_OK;
}

int CReconstructImageDemoDlg::GetMultiLight()
{
	MVCC_ENUMVALUE stEnumMultiLightValue = { 0 };
	MVCC_ENUMENTRY stEnumMultiLightEntry = { 0 };

	int nRet = m_pcMyCamera->GetEnumValue("MultiLightControl", &stEnumMultiLightValue);
	if (MV_OK != nRet)
	{
		return nRet;
	}

	m_ctrlMultiLightCombo.ResetContent();
	for (int i = 0; i < stEnumMultiLightValue.nSupportedNum; i++)
	{
		memset(&stEnumMultiLightEntry, 0, sizeof(stEnumMultiLightEntry));
		stEnumMultiLightEntry.nValue = stEnumMultiLightValue.nSupportValue[i];
		m_pcMyCamera->GetEnumEntrySymbolic("MultiLightControl", &stEnumMultiLightEntry);

		char chIn[MV_MAX_SYMBOLIC_LEN] = "";
		memcpy(chIn, stEnumMultiLightEntry.chSymbolic, MV_MAX_SYMBOLIC_LEN);
		wchar_t wchOut[MV_MAX_SYMBOLIC_LEN] = { 0 };
		Char2Wchar(chIn, wchOut, MV_MAX_SYMBOLIC_LEN);

		CString strMsg = _T("");

		strMsg.Format(_T("%s"), wchOut);

		m_ctrlMultiLightCombo.AddString(strMsg);

		m_mapMultiLight.insert(pair<CString, int>(strMsg, stEnumMultiLightEntry.nValue));
	}

	for (int i = 0; i < stEnumMultiLightValue.nSupportedNum; i++)
	{
		if (stEnumMultiLightValue.nCurValue == stEnumMultiLightValue.nSupportValue[i])
		{
			m_nMultiLightCombo = i;
			m_ctrlMultiLightCombo.SetCurSel(m_nMultiLightCombo);
		}
	}

	return MV_OK;
}

// ch:获取触发模式 | en:Get Trigger Mode
int CReconstructImageDemoDlg::GetTriggerMode()
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
int CReconstructImageDemoDlg::SetTriggerMode()
{
    return m_pcMyCamera->SetEnumValue("TriggerMode", m_nTriggerMode);
}

// ch:获取触发源 | en:Get Trigger Source
int CReconstructImageDemoDlg::GetTriggerSource()
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
int CReconstructImageDemoDlg::SetTriggerSource()
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

int CReconstructImageDemoDlg::GrabThreadProcess()
{
    MV_FRAME_OUT stImageInfo = {0};
    MV_DISPLAY_FRAME_INFO stDisplayInfo = {0};
    MV_RECONSTRUCT_IMAGE_PARAM stImgReconstructionParam = {0};
    unsigned char* pImageBufferList[EXPOSURE_NUM] = {0};
    unsigned int   nImageBufferSize = 0;
    int nRet = MV_OK;

    while(m_bThreadState)
    {
        nRet = m_pcMyCamera->GetImageBuffer(&stImageInfo, 1000);
        if (nRet == MV_OK)
        {
            if (m_nMultiLightNum > 1)      // 多灯
            {
                stImgReconstructionParam.nWidth = stImageInfo.stFrameInfo.nWidth;
                stImgReconstructionParam.nHeight = stImageInfo.stFrameInfo.nHeight;
                stImgReconstructionParam.enPixelType = stImageInfo.stFrameInfo.enPixelType;
                stImgReconstructionParam.pSrcData = stImageInfo.pBufAddr;
                stImgReconstructionParam.nSrcDataLen = stImageInfo.stFrameInfo.nFrameLen;
                stImgReconstructionParam.nExposureNum = m_nMultiLightNum;
                stImgReconstructionParam.enReconstructMethod = MV_SPLIT_BY_LINE;

                if (stImageInfo.stFrameInfo.nFrameLen / m_nMultiLightNum > nImageBufferSize)
                {
                    for (unsigned int i = 0; i < EXPOSURE_NUM; i++)
                    {
                        if (pImageBufferList[i])
                        {
                            free(pImageBufferList[i]);
                            pImageBufferList[i] = NULL;
                        }

                        pImageBufferList[i] = (unsigned char*)malloc(sizeof(unsigned char) * stImageInfo.stFrameInfo.nFrameLen / m_nMultiLightNum);
                        if (NULL != pImageBufferList[i])
                        {
                            stImgReconstructionParam.stDstBufList[i].pBuf = pImageBufferList[i];
                            stImgReconstructionParam.stDstBufList[i].nBufSize = stImageInfo.stFrameInfo.nFrameLen / m_nMultiLightNum;
                        }
                        else
                        {
                            return MV_E_RESOURCE;
                        }
                    }

                    nImageBufferSize = stImageInfo.stFrameInfo.nFrameLen / m_nMultiLightNum;
                }

                // Split Image And Reconstruct Image
                nRet = m_pcMyCamera->ReconstructImage(&stImgReconstructionParam);
                if (MV_OK != nRet)
                {
					continue;
                }

                for (unsigned int i = 0; i < m_nMultiLightNum; i++)
                {
                    stDisplayInfo.hWnd = m_hwndDisplay[i];
                    stDisplayInfo.pData = stImgReconstructionParam.stDstBufList[i].pBuf;
                    stDisplayInfo.nDataLen = stImgReconstructionParam.stDstBufList[i].nBufLen;
                    stDisplayInfo.nWidth = stImgReconstructionParam.stDstBufList[i].nWidth;
                    stDisplayInfo.nHeight = stImgReconstructionParam.stDstBufList[i].nHeight;
                    stDisplayInfo.enPixelType = stImgReconstructionParam.stDstBufList[i].enPixelType;
                    m_pcMyCamera->DisplayOneFrame(&stDisplayInfo);
                }
            }
            else
            {
                stDisplayInfo.hWnd = m_hwndDisplay[0];
                stDisplayInfo.pData = stImageInfo.pBufAddr;
                stDisplayInfo.nDataLen = stImageInfo.stFrameInfo.nFrameLen;
                stDisplayInfo.nWidth = stImageInfo.stFrameInfo.nWidth;
                stDisplayInfo.nHeight = stImageInfo.stFrameInfo.nHeight;
                stDisplayInfo.enPixelType = stImageInfo.stFrameInfo.enPixelType;
                m_pcMyCamera->DisplayOneFrame(&stDisplayInfo);
            }

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

    for (unsigned int i = 0; i < EXPOSURE_NUM; i++)
    {
        if (pImageBufferList[i])
        {
            free(pImageBufferList[i]);
            pImageBufferList[i] = NULL;
        }
    }

    return MV_OK;
}

// ch:按下查找设备按钮:枚举 | en:Click Find Device button:Enumeration 
void CReconstructImageDemoDlg::OnBnClickedEnumDevButton()
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
			strMsg.Format(_T("[%d]XOF:  %s"), i, pUserName);
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
void CReconstructImageDemoDlg::OnBnClickedOpenButton()
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

    MVCC_ENUMVALUE stEnumValue = {0};
    nRet = m_pcMyCamera->GetEnumValue("MultiLightControl", &stEnumValue);
    if (MV_OK != nRet)
    {
        CloseDevice();
        delete m_pcMyCamera;
        m_pcMyCamera = NULL;
        ShowErrorMsg(TEXT("Get Multi Light Control failed, Not Support!"), nRet);
        return ;
    }

    nRet = m_pcMyCamera->SetEnumValue("MultiLightControl", stEnumValue.nCurValue);
    if (MV_OK != nRet)
    {
        ShowErrorMsg(TEXT("Set Multi Light Control failed"), nRet);
    }

	m_nMultiLightNum=stEnumValue.nCurValue;

    m_bOpenDevice = TRUE;

    nRet = GetTriggerMode();
    if (nRet != MV_OK)
    {
        ShowErrorMsg(TEXT("Get Trigger Mode Fail"), nRet);
    }

    nRet = GetTriggerSource();
    if (nRet != MV_OK)
    {
        ShowErrorMsg(TEXT("Get Trigger Source Fail"), nRet);
    }

	nRet = GetMultiLight();
	if (nRet != MV_OK)
	{
		ShowErrorMsg(TEXT("Get MultiLight Fail"), nRet);
	}

    UpdateData(FALSE);
    EnableControls(TRUE);
}

// ch:按下关闭设备按钮：关闭设备 | en:Click Close button: Close Device
void CReconstructImageDemoDlg::OnBnClickedCloseButton()
{
    CloseDevice();
    EnableControls(TRUE);
}

// ch:按下连续模式按钮 | en:Click Continues button
void CReconstructImageDemoDlg::OnBnClickedContinusModeRadio()
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
void CReconstructImageDemoDlg::OnBnClickedTriggerModeRadio()
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
void CReconstructImageDemoDlg::OnBnClickedStartGrabbingButton()
{
    if (FALSE == m_bOpenDevice || TRUE == m_bStartGrabbing || NULL == m_pcMyCamera)
    {
        return;
    }

	GetDlgItem(IDC_DISPLAY1_STATIC)->ShowWindow(FALSE);
	GetDlgItem(IDC_DISPLAY1_STATIC)->ShowWindow(TRUE);
	GetDlgItem(IDC_DISPLAY2_STATIC)->ShowWindow(FALSE);
	GetDlgItem(IDC_DISPLAY2_STATIC)->ShowWindow(TRUE);
	GetDlgItem(IDC_DISPLAY3_STATIC)->ShowWindow(FALSE);
	GetDlgItem(IDC_DISPLAY3_STATIC)->ShowWindow(TRUE);
	GetDlgItem(IDC_DISPLAY4_STATIC)->ShowWindow(FALSE);
	GetDlgItem(IDC_DISPLAY4_STATIC)->ShowWindow(TRUE);

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
void CReconstructImageDemoDlg::OnBnClickedStopGrabbingButton()
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

// ch:按下软触发按钮 | en:Click Software button
void CReconstructImageDemoDlg::OnBnClickedSoftwareTriggerCheck()
{
    UpdateData(TRUE);

    int nRet = SetTriggerSource();
    if (nRet != MV_OK)
    {
        return;
    }
}

// ch:按下软触发一次按钮 | en:Click Execute button
void CReconstructImageDemoDlg::OnBnClickedSoftwareOnceButton()
{
    if (TRUE != m_bStartGrabbing)
    {
        return;
    }

    m_pcMyCamera->CommandExecute("TriggerSoftware");
}
// ch:右上角退出 | en:Exit from upper right corner
void CReconstructImageDemoDlg::OnClose()
{
    PostQuitMessage(0);
    CloseDevice();
	CMvCamera::FinalizeSDK();
    CDialog::OnClose();
}

BOOL CReconstructImageDemoDlg::PreTranslateMessage(MSG* pMsg)
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

bool CReconstructImageDemoDlg::IsStrUTF8(const char* pBuffer, int size)
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

bool CReconstructImageDemoDlg::Char2Wchar(const char *pStr, wchar_t *pOutWStr, int nOutStrSize)
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

bool CReconstructImageDemoDlg::Wchar2char(wchar_t *pOutWStr, char *pStr)
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


void CReconstructImageDemoDlg::OnCbnSelchangeMultilightCombo()
{
	UpdateData(true);

	int nIndex = m_ctrlMultiLightCombo.GetCurSel();

	CString strCBText;

	m_ctrlMultiLightCombo.GetLBText(nIndex, strCBText);

	for (map<CString, int>::iterator it = m_mapMultiLight.begin(); it != m_mapMultiLight.end(); it++)
	{
		if (it->first == strCBText)
		{
			int nRet = m_pcMyCamera->SetEnumValue("MultiLightControl", it->second);
			if (MV_OK != nRet)
			{
				ShowErrorMsg(TEXT("Set MultiLightControl fail"), nRet);
				return;
			}
			break;
		}
	}

	MVCC_ENUMVALUE stEnumValue = {0};
	int nRet = m_pcMyCamera->GetEnumValue("MultiLightControl", &stEnumValue);
	if (MV_OK != nRet)
	{
		CloseDevice();
		delete m_pcMyCamera;
		m_pcMyCamera = NULL;
		ShowErrorMsg(TEXT("Get Multi Light Control failed, Not Support!"), nRet);
		return ;
	}

	m_nMultiLightNum=stEnumValue.nCurValue;
}
