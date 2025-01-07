
// InfraredDemoDlg.cpp : implementation file
#include "stdafx.h"
#include "InfraredDemo.h"
#include "InfraredDemoDlg.h"
#include "RegionSettingDlg.h"
#include "AlarmSettingDlg.h"
#include <list>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
//USES_CONVERSION;
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

// CInfraredDemoDlg dialog
CInfraredDemoDlg::CInfraredDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CInfraredDemoDlg::IDD, pParent)
    , m_pcMyCamera(NULL)
    , m_nDeviceCombo(0)
    , m_bOpenDevice(FALSE)
    , m_bStartGrabbing(FALSE)
    , m_hGrabThread(NULL)
    , m_bThreadState(FALSE)
	, m_dEmissivity(0)
	, m_dTargetDistance(0)
	, m_nTransmissivity(0)
    , m_pSaveImageBuf(NULL)
    , m_nSaveImageBufSize(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    memset(&m_stImageInfo, 0, sizeof(MV_FRAME_OUT_INFO_EX));
}

void CInfraredDemoDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DEVICE_COMBO, m_ctrlDeviceCombo);
	DDX_Control(pDX, IDC_PIXELFORMAT_COMBO, m_ctrlPixelCombo);
	DDX_Control(pDX, IDC_DISPLAYSOURCE_COMBO, m_ctrlDisplaySourceCombo);
	DDX_Control(pDX, IDC_REGIONSELECT_COMBO, m_ctrlRegionSelectCombo);
	DDX_Control(pDX, IDC_MEASURERANGE_COMBO, m_ctrlMeasureRangeCombo);
	DDX_Control(pDX, IDC_PALETTEMODE_COMBO, m_ctrlPaletteModeCombo);
	DDX_Control(pDX, IDC_LEGEND_CHECK, m_ctrlLegendCheckBox);
	DDX_Control(pDX, IDC_EXPORTMODE_CHECK, m_ctrlExportModeCheckBox);
	DDX_CBIndex(pDX, IDC_DEVICE_COMBO, m_nDeviceCombo);
	DDX_Text(pDX, IDC_TRANSMISSIVITY_EDIT, m_nTransmissivity);
	DDX_Text(pDX, IDC_EMISSIVITY_EDIT, m_dEmissivity);
	DDX_Text(pDX, IDC_TARGET_DIS_EDIT, m_dTargetDistance);
}

BEGIN_MESSAGE_MAP(CInfraredDemoDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	// }}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_ENUM_BUTTON, &CInfraredDemoDlg::OnBnClickedEnumButton)
    ON_BN_CLICKED(IDC_OPEN_BUTTON, &CInfraredDemoDlg::OnBnClickedOpenButton)
    ON_BN_CLICKED(IDC_CLOSE_BUTTON, &CInfraredDemoDlg::OnBnClickedCloseButton)
    ON_BN_CLICKED(IDC_START_GRABBING_BUTTON, &CInfraredDemoDlg::OnBnClickedStartGrabbingButton)
    ON_BN_CLICKED(IDC_STOP_GRABBING_BUTTON, &CInfraredDemoDlg::OnBnClickedStopGrabbingButton)
	ON_BN_CLICKED(IDC_GETPARAMETER_BUTTON, &CInfraredDemoDlg::OnBnClickedGetParameterButton)
	ON_BN_CLICKED(IDC_SETPARAMETER_BUTTON, &CInfraredDemoDlg::OnBnClickedSetParameterButton)
	ON_BN_CLICKED(IDC_REGIONSETTING_BUTTON, &CInfraredDemoDlg::OnBnClickedRegionSetting)
	ON_BN_CLICKED(IDC_WARNING_BUTTON, &CInfraredDemoDlg::OnBnClickedAlarmSetting)
	ON_CBN_SELCHANGE(IDC_PIXELFORMAT_COMBO, &CInfraredDemoDlg::OnPixelFormatChanged)
	ON_CBN_SELCHANGE(IDC_DISPLAYSOURCE_COMBO, &CInfraredDemoDlg::OnDisplaySourceChanged)
	ON_CBN_SELCHANGE(IDC_REGIONSELECT_COMBO, &CInfraredDemoDlg::OnRegionSelectChanged)
	ON_CBN_SELCHANGE(IDC_PALETTEMODE_COMBO, &CInfraredDemoDlg::OnPaletteModeChanged)
	ON_BN_CLICKED(IDC_LEGEND_CHECK, &CInfraredDemoDlg::OnLegendVisibleChanged)
	ON_BN_CLICKED(IDC_EXPORTMODE_CHECK, &CInfraredDemoDlg::OnExportModeChanged)

    ON_WM_CLOSE()
END_MESSAGE_MAP()

// ch:取流线程 | en:Grabbing thread
unsigned int __stdcall GrabThread(void* pUser)
{
    if (pUser)
    {
        CInfraredDemoDlg* pCam = (CInfraredDemoDlg*)pUser;

        pCam->GrabThreadProcess();
        
        return 0;
    }

    return -1;
}

// CInfraredDemoDlg message handlers
BOOL CInfraredDemoDlg::OnInitDialog()
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

void CInfraredDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
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
void CInfraredDemoDlg::OnPaint()
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
HCURSOR CInfraredDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// ch:按钮使能 | en:Enable control
void CInfraredDemoDlg::EnableControls(BOOL bIsCameraReady)
{
    GetDlgItem(IDC_OPEN_BUTTON)->EnableWindow(m_bOpenDevice ? FALSE : (bIsCameraReady ? TRUE : FALSE));
    GetDlgItem(IDC_CLOSE_BUTTON)->EnableWindow((m_bOpenDevice && bIsCameraReady) ? TRUE : FALSE);
    GetDlgItem(IDC_START_GRABBING_BUTTON)->EnableWindow((m_bStartGrabbing && bIsCameraReady) ? FALSE : (m_bOpenDevice ? TRUE : FALSE));
    GetDlgItem(IDC_STOP_GRABBING_BUTTON)->EnableWindow(m_bStartGrabbing ? TRUE : FALSE);
	GetDlgItem(IDC_PIXELFORMAT_COMBO)->EnableWindow((m_bStartGrabbing && bIsCameraReady) ? FALSE : (m_bOpenDevice ? TRUE : FALSE));
	GetDlgItem(IDC_DISPLAYSOURCE_COMBO)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);
	GetDlgItem(IDC_LEGEND_CHECK)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);
	GetDlgItem(IDC_REGIONSELECT_COMBO)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);
	GetDlgItem(IDC_REGIONSETTING_BUTTON)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);
	GetDlgItem(IDC_WARNING_BUTTON)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);
	GetDlgItem(IDC_TRANSMISSIVITY_EDIT)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);
	GetDlgItem(IDC_TARGET_DIS_EDIT)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);
	GetDlgItem(IDC_EMISSIVITY_EDIT)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);
	GetDlgItem(IDC_MEASURERANGE_COMBO)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);
	GetDlgItem(IDC_GETPARAMETER_BUTTON)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);
	GetDlgItem(IDC_SETPARAMETER_BUTTON)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);
	GetDlgItem(IDC_PALETTEMODE_COMBO)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);
	GetDlgItem(IDC_EXPORTMODE_CHECK)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);
}

// ch:最开始时的窗口初始化 | en:Initial window initialization
void CInfraredDemoDlg::DisplayWindowInitial()
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
void CInfraredDemoDlg::ShowErrorMsg(CString csMessage, int nErrorNum)
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
int CInfraredDemoDlg::CloseDevice()
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


// ch:设置触发模式 | en:Set Trigger Mode
int CInfraredDemoDlg::SetTriggerMode()
{
	return m_pcMyCamera->SetEnumValue("TriggerMode", MV_TRIGGER_MODE_OFF);
}

// ch:保存图片 | en:Save Image
int CInfraredDemoDlg::SaveImage(MV_SAVE_IAMGE_TYPE enSaveImageType)
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

int CInfraredDemoDlg::GrabThreadProcess()
{
    MV_FRAME_OUT stImageInfo = {0};
    MV_DISPLAY_FRAME_INFO stDisplayInfo = {0};
    int nRet = MV_OK;

	CFile logFile;
	logFile.Open(L".\\InfraredLog.txt", CFile::modeCreate | CFile::modeReadWrite | CFile::shareDenyNone);

    while(m_bThreadState)
    {
		if (!m_bStartGrabbing)
		{
			Sleep(10);
			continue;
		}

        nRet = m_pcMyCamera->GetImageBuffer(&stImageInfo, 1000);
        if (nRet != MV_OK)
        {
			continue;
        }


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

		IFR_OUTCOME_LIST stOutComeList = {0};
		IFR_ALARM_UPLOAD_INFO stAlarmInfoList = { 0 };
		IFR_FULL_SCREEN_MAX_MIN_INFO stFullScreenMaxMin = { 0 };
		IRF_OSD_INFO stOsdInfo;
		float *fFullScreenData = NULL;
		int nWidth = stImageInfo.stFrameInfo.nWidth;
		int nHeight = stImageInfo.stFrameInfo.nHeight;

		if (stImageInfo.stFrameInfo.enPixelType == PXIEL_FORMAT_FLOAT32 &&
			stImageInfo.stFrameInfo.nFrameLen == nWidth * nHeight * 4)
		{
			fFullScreenData = (float *)stImageInfo.pBufAddr;
		}

		for (unsigned int i = 0; i < stImageInfo.stFrameInfo.nUnparsedChunkNum; ++i)
		{
			MV_CHUNK_DATA_CONTENT stChunkData = stImageInfo.stFrameInfo.UnparsedChunkList.pUnparsedChunkContent[i];

			if (TEMP_CHUNK_ID_TEST == stChunkData.nChunkID &&
				sizeof(IFR_OUTCOME_LIST) == stChunkData.nChunkLen)
			{
				memcpy(&stOutComeList, stChunkData.pChunkData, sizeof(IFR_OUTCOME_LIST));
			}
			else if (TEMP_CHUNK_ID_ALARM == stChunkData.nChunkID &&
				     sizeof(IFR_ALARM_UPLOAD_INFO) == stChunkData.nChunkLen)
			{
				memcpy(&stAlarmInfoList, stChunkData.pChunkData, sizeof(IFR_ALARM_UPLOAD_INFO));
			}
			else if (TEMP_CHUNK_ID_MIN_MAX_TEMP == stChunkData.nChunkID &&
				sizeof(IFR_FULL_SCREEN_MAX_MIN_INFO) == stChunkData.nChunkLen)
			{
				memcpy(&stFullScreenMaxMin, stChunkData.pChunkData, sizeof(IFR_FULL_SCREEN_MAX_MIN_INFO));
			}
			else if (TEMP_CHUNK_ID_OSD_INFO == stChunkData.nChunkID &&
				sizeof(IRF_OSD_INFO) == stChunkData.nChunkLen)
			{
				memcpy(&stOsdInfo, stChunkData.pChunkData, sizeof(IRF_OSD_INFO));
			}
		}

		CString strLog;
		strLog.Format(L"************Beginning Output Test Temperature Info, Frame Number:%d************\r\n", stImageInfo.stFrameInfo.nFrameNum);
		logFile.Write(strLog.GetBuffer(), strLog.GetLength() * sizeof(wchar_t));
		
		std::list<unsigned char> oAlramList;
		for (int i = 0; i < TEMP_REGION_COUNT; ++i)
		{
			if (!stOsdInfo.regionDispRules[i].regionDispEnable)
			{
				continue;
			}


			IFR_ALARM_INFO stAlarmInfo = stAlarmInfoList.alarmOutcome[i];

			//deal with the alarm region.
			if (stAlarmInfo.alarmkey > 0 && TEMP_ALARM_LEVER_WARN == stAlarmInfo.alarmLevel &&
				stOsdInfo.regionDispRules[i].regionAlarmDispEnable)
			{

				if (TEMP_ALARM_TYPE_MAX == stAlarmInfo.alarmType)
				{
					strLog.Format(L"RegionID: %d; Alarm Max Temp : %f; PointX : %d; PointY : %d\r\n",
						stAlarmInfo.regionId, 0.1 * stAlarmInfo.measureTmpData, stAlarmInfo.points[0].x, stAlarmInfo.points[0].y);
					logFile.Write(strLog.GetBuffer(), strLog.GetLength() * sizeof(wchar_t));

				}
				else if (TEMP_ALARM_TYPE_MIN == stAlarmInfo.alarmType)
				{
					strLog.Format(L"RegionID: %d; Alarm Min Temp : %d; PointX : %f; PointY : %d\r\n",
						stAlarmInfo.regionId, 0.1 * stAlarmInfo.measureTmpData, stAlarmInfo.points[1].x, stAlarmInfo.points[1].y);
					logFile.Write(strLog.GetBuffer(), strLog.GetLength() * sizeof(wchar_t));

				}
				else if (TEMP_ALARM_TYPE_AVG == stAlarmInfo.alarmType)
				{
					strLog.Format(L"RegionID: %d; Alarm Avg Temp : %d; PointX : %d; PointY : %d\r\n",
						stAlarmInfo.regionId, 0.1 * stAlarmInfo.measureTmpData, stAlarmInfo.points[0].x, stAlarmInfo.points[0].y);
					logFile.Write(strLog.GetBuffer(), strLog.GetLength() * sizeof(wchar_t));

				}
				else
				{
					strLog.Format(L"RegionID: %d; Alarm Differ Temp : %d; PointX : %d; PointY : %d\r\n",
						stAlarmInfo.regionId, 0.1 * stAlarmInfo.measureTmpData, stAlarmInfo.points[0].x, stAlarmInfo.points[0].y);
					logFile.Write(strLog.GetBuffer(), strLog.GetLength() * sizeof(wchar_t));
				}

				oAlramList.push_back(stAlarmInfo.regionId);
				if (stOsdInfo.osdProcessor == OSD_BY_CLIENT)
				{
					DrawShapeInImg(stAlarmInfo.polygon, stAlarmInfo.regiontype, true, nWidth, nHeight);
				}

				continue;
			}

			//deal with the enable region.
			IFR_OUTCOME_INFO stOutCome = stOutComeList.ifrOutcome[i];
			if (!stOutCome.enable)
			{
				continue;
			}

			if (TEMP_ROI_TYPE_POINT == stOutCome.regiontype)
			{
				if (stOsdInfo.regionDispRules[i].regionAvgTempDispEnable)
				{
					strLog.Format(L"RegionID: %d; Avg Temp : %f; PointX : %d; PointY : %d\r\n",
						stOutCome.regionId, 0.1 * stOutCome.avrTmp, stOutCome.points[0].x, stOutCome.points[0].y);
					logFile.Write(strLog.GetBuffer(), strLog.GetLength() * sizeof(wchar_t));

				}
			}
			else
			{
				//均值显示在最大值的后面
				if (stOsdInfo.regionDispRules[i].regionAvgTempDispEnable)
				{
					strLog.Format(L"RegionID: %d; Avg Temp : %f; PointX : %d; PointY : %d\r\n",
						stOutCome.regionId, 0.1 * stOutCome.avrTmp, stOutCome.points[0].x, stOutCome.points[0].y);
					logFile.Write(strLog.GetBuffer(), strLog.GetLength() * sizeof(wchar_t));

				}

				if (stOsdInfo.regionDispRules[i].regionMaxTempDispEnable)
				{

					strLog.Format(L"RegionID: %d; Max Temp : %f; PointX : %d; PointY : %d\r\n",
						stOutCome.regionId, 0.1 * stOutCome.maxTmp, stOutCome.points[0].x, stOutCome.points[0].y);
					logFile.Write(strLog.GetBuffer(), strLog.GetLength() * sizeof(wchar_t));

				}


				if (stOsdInfo.regionDispRules[i].regionMinTempDispEnable)
				{
					strLog.Format(L"RegionID: %d; Min Temp : %f; PointX : %d; PointY : %d\r\n",
						stOutCome.regionId, 0.1 * stOutCome.minTmp, stOutCome.points[1].x, stOutCome.points[1].y);
					logFile.Write(strLog.GetBuffer(), strLog.GetLength() * sizeof(wchar_t));

				}

			}

			if (stOsdInfo.osdProcessor == OSD_BY_CLIENT)
			{
				DrawShapeInImg(stOutCome.polygon, stOutCome.regiontype, false, nWidth, nHeight);
			}
		}

		if (fFullScreenData)
		{
			for (int i = 0; i < nHeight; ++i)
			{
				for (int j = 0; j < nWidth; ++j)
				{
					float fTemp = fFullScreenData[j + i * nWidth];
					strLog.Format(L"%f  ", fTemp);
					logFile.Write(strLog.GetBuffer(), strLog.GetLength() * sizeof(wchar_t));
				}

				strLog.Format(L"\r\n");
				logFile.Write(strLog.GetBuffer(), strLog.GetLength() * sizeof(wchar_t));
			}
		}

		strLog.Format(L"************Ending Output, Frame Number:%d************\r\n\r\n\r\n", stImageInfo.stFrameInfo.nFrameNum);
		logFile.Write(strLog.GetBuffer(), strLog.GetLength() * sizeof(wchar_t));
		logFile.Flush();

		m_pcMyCamera->FreeImageBuffer(&stImageInfo);
    }

	logFile.Flush();
	logFile.Close();

    return MV_OK;
}
// ch:按下查找设备按钮:枚举 | en:Click Find Device button:Enumeration 
void CInfraredDemoDlg::OnBnClickedEnumButton()
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
                DWORD dwLenUserName = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(pDeviceInfo->SpecialInfo.stGigEInfo.chUserDefinedName), -1, NULL, 0);
                pUserName = new wchar_t[dwLenUserName];
                MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(pDeviceInfo->SpecialInfo.stGigEInfo.chUserDefinedName), -1, pUserName, dwLenUserName);
            }
            else
            {
                char strUserName[256] = {0};
                sprintf_s(strUserName, 256, "%s %s (%s)", pDeviceInfo->SpecialInfo.stGigEInfo.chManufacturerName,
                    pDeviceInfo->SpecialInfo.stGigEInfo.chModelName,
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
                DWORD dwLenUserName = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(pDeviceInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName), -1, NULL, 0);
                pUserName = new wchar_t[dwLenUserName];
                MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(pDeviceInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName), -1, pUserName, dwLenUserName);
            }
            else
            {
                char strUserName[256] = {0};
                sprintf_s(strUserName, 256, "%s %s (%s)", pDeviceInfo->SpecialInfo.stUsb3VInfo.chManufacturerName,
                    pDeviceInfo->SpecialInfo.stUsb3VInfo.chModelName,
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
void CInfraredDemoDlg::OnBnClickedOpenButton()
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
void CInfraredDemoDlg::OnBnClickedCloseButton()
{
    CloseDevice();
    EnableControls(TRUE);
}

// ch:按下开始采集按钮 | en:Click Start button
void CInfraredDemoDlg::OnBnClickedStartGrabbingButton()
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
void CInfraredDemoDlg::OnBnClickedStopGrabbingButton()
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
void CInfraredDemoDlg::OnBnClickedGetParameterButton()
{
	int nRet = SetTriggerMode();

	nRet = ReadEnumIntoCombo("PixelFormat", m_ctrlPixelCombo);
	if (nRet != MV_OK)
	{
		ShowErrorMsg(TEXT("Get PixelFormat Fail"), nRet);
		return;
	}

	nRet = ReadEnumIntoCombo("OverScreenDisplayProcessor", m_ctrlDisplaySourceCombo);
	if (nRet != MV_OK)
	{
		ShowErrorMsg(TEXT("Get OverScreenDisplayProcessor Fail"), nRet);
		return;
	}

	nRet = ReadEnumIntoCombo("PalettesMode", m_ctrlPaletteModeCombo);
	if (nRet != MV_OK)
	{
		ShowErrorMsg(TEXT("Get PalettesMode Fail"), nRet);
		return;
	}

	bool bValue = false;
	nRet = m_pcMyCamera->GetBoolValue("LegendDisplayEnable", &bValue);
	if (nRet != MV_OK)
	{
		ShowErrorMsg(TEXT("Get LegendDisplayEnable Fail"), nRet);
		return;
	}
	else
	{
		m_ctrlLegendCheckBox.SetCheck(bValue);
	}

	nRet = m_pcMyCamera->GetBoolValue("MtExpertMode", &bValue);
	if (nRet != MV_OK)
	{
		ShowErrorMsg(TEXT("Get MtExpertMode Fail"), nRet);
		return;
	}
	else
	{
		m_ctrlExportModeCheckBox.SetCheck(bValue);
	}

	nRet = ReadEnumIntoCombo("TempRegionSelector", m_ctrlRegionSelectCombo);
	if (nRet != MV_OK)
	{
		ShowErrorMsg(TEXT("Get TempRegionSelector Fail"), nRet);
		return;
	}

	nRet = ReadEnumIntoCombo("TempMeasurementRange", m_ctrlMeasureRangeCombo);
	if (nRet != MV_OK)
	{
		ShowErrorMsg(TEXT("Get TempMeasurementRange Fail"), nRet);
		return;
	}

	MVCC_INTVALUE_EX oIntValue = { 0 };
	nRet = m_pcMyCamera->GetIntValue("AtmosphericTransmissivity", &oIntValue);
	if (nRet != MV_OK)
	{
		ShowErrorMsg(TEXT("Get AtmosphericTransmissivity Fail"), nRet);
		return;
	}
	else
	{
		m_nTransmissivity = oIntValue.nCurValue;
	}

	MVCC_FLOATVALUE oFloatValue = { 0 };
	nRet = m_pcMyCamera->GetFloatValue("TargetDistance", &oFloatValue);
	if (nRet != MV_OK)
	{
		ShowErrorMsg(TEXT("Get TargetDistance Fail"), nRet);
		return;
	}
	else
	{
		m_dTargetDistance = oFloatValue.fCurValue;
	}

	nRet = m_pcMyCamera->GetFloatValue("FullScreenEmissivity", &oFloatValue);
	if (nRet != MV_OK)
	{
		ShowErrorMsg(TEXT("Get FullScreenEmissivity Fail"), nRet);
		return;
	}
	else
	{
		m_dEmissivity = oFloatValue.fCurValue;
	}

    UpdateData(FALSE);
}

// ch:按下设置参数按钮 | en:Click Set Parameter button
void CInfraredDemoDlg::OnBnClickedSetParameterButton()
{
    UpdateData(TRUE);

    bool bIsSetSucceed = true;
	int nRet = MV_OK;
	nRet = m_pcMyCamera->SetIntValue("AtmosphericTransmissivity", m_nTransmissivity);
	if (nRet != MV_OK)
	{
		ShowErrorMsg(TEXT("Set AtmosphericTransmissivity Fail"), nRet);
	}

	nRet = m_pcMyCamera->SetFloatValue("TargetDistance", m_dTargetDistance);
	if (nRet != MV_OK)
	{
		ShowErrorMsg(TEXT("Set TargetDistance Fail"), nRet);
	}

	nRet = m_pcMyCamera->SetFloatValue("FullScreenEmissivity", m_dEmissivity + 0.000001);
	if (nRet != MV_OK)
	{
		ShowErrorMsg(TEXT("Set FullScreenEmissivity Fail"), nRet);
	}
 
	nRet = SetEnumIntoDevice("TempMeasurementRange", m_ctrlMeasureRangeCombo);
	if (nRet != MV_OK)
	{
		ShowErrorMsg(TEXT("Set TempMeasurementRange Fail"), nRet);
	}
}


// ch:右上角退出 | en:Exit from upper right corner
void CInfraredDemoDlg::OnClose()
{
    PostQuitMessage(0);
    CloseDevice();
	CMvCamera::FinalizeSDK();
    DeleteCriticalSection(&m_hSaveImageMux);
    CDialog::OnClose();
}

BOOL CInfraredDemoDlg::PreTranslateMessage(MSG* pMsg)
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

int CInfraredDemoDlg::ReadEnumIntoCombo(const char* strKey, CComboBox &ctrlComboBox)
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

int CInfraredDemoDlg::SetEnumIntoDevice(const char* strKey, CComboBox &ctrlComboBox)
{
	CString strText;
	ctrlComboBox.GetWindowText(strText);
	LPCWSTR _lpw = NULL;
	int _convert = 0;
	UINT _acp = ATL::_AtlGetConversionACP();
	return m_pcMyCamera->SetEnumValueByString(strKey, W2A(strText));
}

void CInfraredDemoDlg::OnPixelFormatChanged()
{
	int nSel = m_ctrlPixelCombo.GetCurSel();
	m_ctrlPixelCombo.SetCurSel(nSel);
	int nRet = SetEnumIntoDevice("PixelFormat", m_ctrlPixelCombo);
	if (nRet != MV_OK)
	{
		ShowErrorMsg(TEXT("Set PixelFormat Fail"), nRet);
	}
}

void CInfraredDemoDlg::OnDisplaySourceChanged()
{
	int nSel = m_ctrlDisplaySourceCombo.GetCurSel();
	m_ctrlDisplaySourceCombo.SetCurSel(nSel);
	int nRet = SetEnumIntoDevice("OverScreenDisplayProcessor", m_ctrlDisplaySourceCombo);
	if (nRet != MV_OK)
	{
		ShowErrorMsg(TEXT("Set OverScreenDisplayProcessor Fail"), nRet);
		return;
	}


	nRet = m_pcMyCamera->CommandExecute("TempControlLoad");
	if (nRet != MV_OK)
	{
		ShowErrorMsg(TEXT("Exec TempControlLoad Fail"), nRet);
	}
}

void CInfraredDemoDlg::OnRegionSelectChanged()
{
	int nSel = m_ctrlRegionSelectCombo.GetCurSel();
	m_ctrlRegionSelectCombo.SetCurSel(nSel);
	int nRet = SetEnumIntoDevice("TempRegionSelector", m_ctrlRegionSelectCombo);
	if (nRet != MV_OK)
	{
		ShowErrorMsg(TEXT("Set TempRegionSelector Fail"), nRet);
	}
}

void CInfraredDemoDlg::OnPaletteModeChanged()
{
	int nSel = m_ctrlPaletteModeCombo.GetCurSel();
	m_ctrlPaletteModeCombo.SetCurSel(nSel);
	int nRet = SetEnumIntoDevice("PalettesMode", m_ctrlPaletteModeCombo);
	if (nRet != MV_OK)
	{
		ShowErrorMsg(TEXT("Set PaletteMode Fail"), nRet);
	}
}

void CInfraredDemoDlg::OnLegendVisibleChanged()
{
	bool bChecked = m_ctrlLegendCheckBox.GetCheck() > 0;
	int nRet = m_pcMyCamera->SetBoolValue("LegendDisplayEnable", bChecked);
	if (nRet != MV_OK)
	{
		ShowErrorMsg(TEXT("Set LegendDisplayEnable Fail"), nRet);
		return;
	}

	nRet = m_pcMyCamera->CommandExecute("TempControlLoad");
	if (nRet != MV_OK)
	{
		ShowErrorMsg(TEXT("Exec TempControlLoad Fail"), nRet);
	}
}

void CInfraredDemoDlg::OnExportModeChanged()
{
	bool bChecked = m_ctrlExportModeCheckBox.GetCheck() > 0;
	int nRet = m_pcMyCamera->SetBoolValue("MtExpertMode", bChecked);
	if (nRet != MV_OK)
	{
		ShowErrorMsg(TEXT("Set ExpertMode Fail"), nRet);
		return;
	}

	nRet = m_pcMyCamera->CommandExecute("TempControlLoad");
	if (nRet != MV_OK)
	{
		ShowErrorMsg(TEXT("Exec TempControlLoad Fail"), nRet);
	}
}

void CInfraredDemoDlg::OnBnClickedRegionSetting()
{
	CString strRegion;
	m_ctrlRegionSelectCombo.GetWindowText(strRegion);
	CRegionSettingDlg dlgRegionSetting(m_pcMyCamera, strRegion, m_ctrlExportModeCheckBox.GetCheck() > 0, this);
	dlgRegionSetting.DoModal();
}

void CInfraredDemoDlg::OnBnClickedAlarmSetting()
{
	CString strRegion;
	m_ctrlRegionSelectCombo.GetWindowText(strRegion);
	CAlarmSettingDlg dlgAlarmSetting(m_pcMyCamera, strRegion, m_ctrlRegionSelectCombo.GetCurSel(), this);
	dlgAlarmSetting.DoModal();
}

void CInfraredDemoDlg::DrawShapeInImg(const IFR_POLYGON & oPolygon, int nRegionType, bool bAlarm, int nWidth, int nHeight)
{
	MVCC_LINES_INFO stLinesInfo = { 0 };
	stLinesInfo.nLineWidth = 2;
	stLinesInfo.stStartPoint.fX = 0;
	stLinesInfo.stStartPoint.fY = 0;
	stLinesInfo.stEndPoint.fX = 1.0;
	stLinesInfo.stEndPoint.fY = 1.0;
	stLinesInfo.stColor.fR = 0;
	stLinesInfo.stColor.fG = 1;
	stLinesInfo.stColor.fB = 0;
	stLinesInfo.stColor.fAlpha = 1;
	if (bAlarm)
	{
		stLinesInfo.stColor.fR = 1;
		stLinesInfo.stColor.fG = 0;
	}

	if (TEMP_ROI_TYPE_POINT == nRegionType && oPolygon.pointNum > 0)    //点
	{
		IFR_POINT tPoint = oPolygon.pointList[0];
		float fX = (float)tPoint.x / nWidth;
		float fY = (float)tPoint.y / nHeight;
		stLinesInfo.stStartPoint.fX = fX - 0.01;
		stLinesInfo.stStartPoint.fY = fY;
		stLinesInfo.stEndPoint.fX = fX + 0.01;
		stLinesInfo.stEndPoint.fY = fY;
		m_pcMyCamera->DrawLines(&stLinesInfo);

		stLinesInfo.stStartPoint.fX = fX;
		stLinesInfo.stStartPoint.fY = fY - 0.01;
		stLinesInfo.stEndPoint.fX = fX;
		stLinesInfo.stEndPoint.fY = fY + 0.01;
		m_pcMyCamera->DrawLines(&stLinesInfo);
	}
	else if (TEMP_ROI_TYPE_LINE == nRegionType && oPolygon.pointNum >= 2)    //线
	{
		IFR_POINT tPoint_0 = oPolygon.pointList[0];
		IFR_POINT tPoint_1 = oPolygon.pointList[1];
		stLinesInfo.stStartPoint.fX = 1.0 * tPoint_0.x / nWidth;
		stLinesInfo.stStartPoint.fY = 1.0 * tPoint_0.y / nHeight;
		stLinesInfo.stEndPoint.fX = 1.0 * tPoint_1.x / nWidth;
		stLinesInfo.stEndPoint.fY = 1.0 * tPoint_1.y / nHeight;
		m_pcMyCamera->DrawLines(&stLinesInfo);
	}
	else if ((TEMP_ROI_TYPE_POLYGON == nRegionType || TEMP_ROI_TYPE_CIRCLE == nRegionType) && oPolygon.pointNum > 2)    //多边形
	{
		for (int i = 1; i <= oPolygon.pointNum; ++i)
		{
			IFR_POINT tPoint_0 = oPolygon.pointList[i - 1];
			IFR_POINT tPoint_1 = oPolygon.pointList[i % oPolygon.pointNum];
			stLinesInfo.stStartPoint.fX = 1.0 * tPoint_0.x / nWidth;
			stLinesInfo.stStartPoint.fY = 1.0 * tPoint_0.y / nHeight;
			stLinesInfo.stEndPoint.fX = 1.0 * tPoint_1.x / nWidth;
			stLinesInfo.stEndPoint.fY = 1.0 * tPoint_1.y / nHeight;
			m_pcMyCamera->DrawLines(&stLinesInfo);
		}
	}
}


