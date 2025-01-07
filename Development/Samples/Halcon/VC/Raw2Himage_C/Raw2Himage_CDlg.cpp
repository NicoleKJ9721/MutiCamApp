
//

#include "stdafx.h"
#include "Raw2Himage_C.h"
#include "Raw2Himage_CDlg.h"

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

CRaw2Himage_CDlg::CRaw2Himage_CDlg(CWnd* pParent /*=NULL*/)
: CDialog(CRaw2Himage_CDlg::IDD, pParent)
, m_pcMyCamera(NULL)
, m_nDeviceCombo(0)                     
, m_bOpenDevice(FALSE)
, m_bStartGrabbing(FALSE)
, m_bThreadState(FALSE)
, m_hGrabThread(NULL)
, m_nTriggerMode(MV_TRIGGER_MODE_OFF)
, m_bSoftWareTriggerCheck(FALSE)
, m_nTriggerSource(MV_TRIGGER_SOURCE_SOFTWARE)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRaw2Himage_CDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_OPEN_BUTTON, m_ctrlOpenButton);
    DDX_Control(pDX, IDC_CLOSE_BUTTON, m_ctrlCloseButton);
    DDX_Control(pDX, IDC_START_GRABBING_BUTTON, m_ctrlStartGrabbingButton);
    DDX_Control(pDX, IDC_STOP_GRABBING_BUTTON, m_ctrlStopGrabbingButton);
    DDX_Control(pDX, IDC_SOFTWARE_TRIGGER_CHECK, m_ctrlSoftwareTriggerCheck);
    DDX_Control(pDX, IDC_SOFTWARE_ONCE_BUTTON, m_ctrlSoftwareOnceButton);
    DDX_Control(pDX, IDC_CONTINUS_MODE_RADIO, m_ctrlContinusModeRadio);
    DDX_Control(pDX, IDC_TRIGGER_MODE_RADIO, m_ctrlTriggerModeRadio);
    DDX_Control(pDX, IDC_DEVICE_COMBO, m_ctrlDeviceCombo);
    DDX_CBIndex(pDX, IDC_DEVICE_COMBO, m_nDeviceCombo);
    DDX_Check(pDX, IDC_SOFTWARE_TRIGGER_CHECK, m_bSoftWareTriggerCheck);
}

BEGIN_MESSAGE_MAP(CRaw2Himage_CDlg, CDialog)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    //}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_ENUM_BUTTON, &CRaw2Himage_CDlg::OnBnClickedEnumButton)
    ON_BN_CLICKED(IDC_OPEN_BUTTON, &CRaw2Himage_CDlg::OnBnClickedOpenButton)
    ON_BN_CLICKED(IDC_CLOSE_BUTTON, &CRaw2Himage_CDlg::OnBnClickedCloseButton)
    ON_BN_CLICKED(IDC_CONTINUS_MODE_RADIO, &CRaw2Himage_CDlg::OnBnClickedContinusModeRadio)
    ON_BN_CLICKED(IDC_TRIGGER_MODE_RADIO, &CRaw2Himage_CDlg::OnBnClickedTriggerModeRadio)
    ON_BN_CLICKED(IDC_START_GRABBING_BUTTON, &CRaw2Himage_CDlg::OnBnClickedStartGrabbingButton)
    ON_BN_CLICKED(IDC_STOP_GRABBING_BUTTON, &CRaw2Himage_CDlg::OnBnClickedStopGrabbingButton)
    ON_BN_CLICKED(IDC_SOFTWARE_TRIGGER_CHECK, &CRaw2Himage_CDlg::OnBnClickedSoftwareTriggerCheck)
    ON_BN_CLICKED(IDC_SOFTWARE_ONCE_BUTTON, &CRaw2Himage_CDlg::OnBnClickedSoftwareOnceButton)
    ON_WM_CLOSE()
END_MESSAGE_MAP()


// CRaw2Himage_CDlg message handlers

BOOL CRaw2Himage_CDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // Add "About..." menu item to system menu.

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

void CRaw2Himage_CDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CRaw2Himage_CDlg::OnPaint()
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
HCURSOR CRaw2Himage_CDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

void CRaw2Himage_CDlg::EnableWindowWhenClose(void)
{
    m_ctrlOpenButton.EnableWindow(FALSE);
    m_ctrlCloseButton.EnableWindow(FALSE);
    m_ctrlStartGrabbingButton.EnableWindow(FALSE);
    m_ctrlStopGrabbingButton.EnableWindow(FALSE);
    m_ctrlSoftwareTriggerCheck.EnableWindow(FALSE);
    m_ctrlSoftwareOnceButton.EnableWindow(FALSE);
    m_ctrlContinusModeRadio.EnableWindow(FALSE);
    m_ctrlTriggerModeRadio.EnableWindow(FALSE);
}

// ch: 打开设备但不开始抓图 || en:The device is opened but it does not grab image
void CRaw2Himage_CDlg::EnableWindowWhenOpenNotStart(void)
{
    m_ctrlOpenButton.EnableWindow(FALSE);
    m_ctrlCloseButton.EnableWindow(TRUE);
    m_ctrlStartGrabbingButton.EnableWindow(TRUE);
    m_ctrlSoftwareTriggerCheck.EnableWindow(TRUE);
    m_ctrlContinusModeRadio.EnableWindow(TRUE);
    m_ctrlTriggerModeRadio.EnableWindow(TRUE);
}

// ch: 按下开始采集按钮时的按钮颜色 || en:color of button when it starts grabbing image
void CRaw2Himage_CDlg::EnableWindowWhenStart(void)
{
    m_ctrlStopGrabbingButton.EnableWindow(TRUE);
    m_ctrlStartGrabbingButton.EnableWindow(FALSE);
    if (TRUE == m_bSoftWareTriggerCheck && MV_TRIGGER_MODE_ON == m_nTriggerMode)
    {
        m_ctrlSoftwareOnceButton.EnableWindow(TRUE);
    }
}

// ch: 按下结束采集时的按钮颜色 || en: color of button when it stops grabbing image
void CRaw2Himage_CDlg::EnableWindowWhenStop(void)
{
    m_ctrlStopGrabbingButton.EnableWindow(FALSE);
    m_ctrlStartGrabbingButton.EnableWindow(TRUE);
    m_ctrlSoftwareOnceButton.EnableWindow(FALSE);
}

// ch: 最开始时的窗口初始化 || en: initialization of window in the beginning
void CRaw2Himage_CDlg::DisplayWindowInitial(void)
{
    // ch: 定义显示的起点和长宽高 || en:defined display starting point, height, width, and length
    HTuple hWindowRow, hWindowColumn, hWindowWidth, hWindowHeight;  
    // ch: 定义窗口ID || en:Define window ID 
    HTuple hWindowID;

    // ch; 定义一个CRect类的对象，用于获取图像框的尺寸和位置 || en:define an object of CRect class to measure the size and position of image window
    CRect mRect;  
    CStatic* pWnd = (CStatic*)GetDlgItem(IDC_DISPLAY_STATIC);  

    CDC *pDC = pWnd->GetDC();
    pWnd->GetWindowRect(&mRect);

    // ch: 设置显示窗口的起点和宽高 || en:set the starting point, width, and height of display window 
    hWindowRow = 0;  
    hWindowColumn = 0;  
    hWindowWidth = mRect.Width();  
    hWindowHeight = mRect.Height();
    try  
    {   
        hWindowID = (Hlong)pWnd->m_hWnd;
        open_window(hWindowRow, hWindowColumn, hWindowWidth, hWindowHeight, hWindowID, "visible", "", &m_hWindowID);  
    }  
    catch (HException& except)  
    {  
        TRACE(except.message);  
    }

    EnableWindowWhenClose();
}

// ch: 关闭设备 || en: close device
int CRaw2Himage_CDlg::CloseDevice(void)
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

    return MV_OK;
}

// ch: 获取触发模式 || en: get trigger mode
int CRaw2Himage_CDlg::GetTriggerMode(void)
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
    else if (MV_TRIGGER_MODE_OFF == m_nTriggerMode)
    {
        OnBnClickedContinusModeRadio();
    }
    else
    {
        return nRet;
    }
    UpdateData(FALSE);

    return MV_OK;
}

// ch: 设置触发模式 || en: set trigger mode
int CRaw2Himage_CDlg::SetTriggerMode(void)
{
    return m_pcMyCamera->SetEnumValue("TriggerMode", m_nTriggerMode);
}

// ch: 获取触发源 || en: get trigger source
int CRaw2Himage_CDlg::GetTriggerSource(void)
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
    UpdateData(FALSE);

    return MV_OK;
}

// ch: 设置触发源 || en: set trigger source
int CRaw2Himage_CDlg::SetTriggerSource(void)
{
    UpdateData(TRUE);

    if (m_bSoftWareTriggerCheck)
    {
        m_nTriggerSource = MV_TRIGGER_SOURCE_SOFTWARE;
        int nRet = m_pcMyCamera->SetEnumValue("TriggerSource", m_nTriggerSource);
        if (MV_OK != nRet)
        {
            MessageBox(TEXT("Set Software Trigger Fail"), TEXT("warning"), MB_OK | MB_ICONWARNING);
            return nRet;
        }
        m_ctrlSoftwareOnceButton.EnableWindow(TRUE);
    }
    else
    {
        m_nTriggerSource = MV_TRIGGER_SOURCE_LINE0;
        int nRet = m_pcMyCamera->SetEnumValue("TriggerSource", m_nTriggerSource);
        if (MV_OK != nRet)
        {
            MessageBox(TEXT("Set Hardware Trigger Fail"), TEXT("warning"), MB_OK | MB_ICONWARNING);
            return nRet;
        }
        m_ctrlSoftwareOnceButton.EnableWindow(FALSE);
    }

    return MV_OK;
}

// ch: 按下查找设备按钮:枚举 || en:click the look for device button: enumeration
void CRaw2Himage_CDlg::OnBnClickedEnumButton()
{
    CString strMsg;

    // ch: 1. 清除设备列表框中的信息 || en: 1. clear the information in the device list
    m_ctrlDeviceCombo.ResetContent();

    // ch: 2. 初始化设备信息列表 || en: 2. initialize information list of device
    memset(&m_stDevList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));

    // ch:3. 枚举子网内所有设备 || en: 3. enumerate all of devices in the subnetwork
    int nRet = CMvCamera::EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &m_stDevList);
    if (MV_OK != nRet)
    {
        // ch: 枚举设备失败 || en: fail to enumerate devices
        return;
    }

    // ch: 将值加入到信息列表框中并显示出来 || en: add value to the information list and display it
    unsigned int i;
    int nIp1, nIp2, nIp3, nIp4;
    for (i = 0; i < m_stDevList.nDeviceNum; i++)
    {
        MV_CC_DEVICE_INFO* pDeviceInfo = m_stDevList.pDeviceInfo[i];
        if (NULL == pDeviceInfo)
        {
            continue;
        }

        wchar_t* pUserName = NULL;
        if (pDeviceInfo->nTLayerType == MV_GIGE_DEVICE)
        {
            nIp1 = ((m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24);
            nIp2 = ((m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16);
            nIp3 = ((m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8);
            nIp4 = (m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

            if (strcmp("", (LPCSTR)(pDeviceInfo->SpecialInfo.stGigEInfo.chUserDefinedName)) != 0)
            {
                DWORD dwLenUserName = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(pDeviceInfo->SpecialInfo.stGigEInfo.chUserDefinedName), -1, NULL, 0);
                pUserName = new wchar_t[dwLenUserName];
                MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(pDeviceInfo->SpecialInfo.stGigEInfo.chUserDefinedName), -1, pUserName, dwLenUserName);
            }
            else
            {

                char strUserName[256];
                sprintf_s(strUserName, "%s %s (%s)", pDeviceInfo->SpecialInfo.stGigEInfo.chManufacturerName,
                    pDeviceInfo->SpecialInfo.stGigEInfo.chModelName,
                    pDeviceInfo->SpecialInfo.stGigEInfo.chSerialNumber);
                DWORD dwLenUserName = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strUserName), -1, NULL, 0);
                pUserName = new wchar_t[dwLenUserName];
                MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strUserName), -1, pUserName, dwLenUserName);
            }
            strMsg.Format(_T("[%d]GigE:    %s  (%d.%d.%d.%d)"), i, 
                pUserName, nIp1, nIp2, nIp3, nIp4);
            
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
                sprintf_s(strUserName, "%s %s (%s)", pDeviceInfo->SpecialInfo.stUsb3VInfo.chManufacturerName,
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
        }
        m_ctrlDeviceCombo.AddString(strMsg);

        if (NULL != pUserName)
        {
            delete(pUserName);
            pUserName = NULL;
        }
    }
    m_ctrlDeviceCombo.SetCurSel(CB_ERR);

    // ch: 枚举到设备之后要显示出来 || en: display the device after enumeration
    UpdateData(FALSE);

    if (0 == m_stDevList.nDeviceNum)
    {
        MessageBox(TEXT("No device"));
        return;
    }
    // ch: 将打开设备的按钮显现出来 || en: show the button for opening device
    m_ctrlOpenButton.EnableWindow(TRUE);
}

// ch: 按下打开设备按钮：打开设备 || en: press open device button: open device
void CRaw2Himage_CDlg::OnBnClickedOpenButton()
{
    if (TRUE == m_bOpenDevice || NULL != m_pcMyCamera)
    {
        return;
    }
    UpdateData(TRUE);

    int nIndex = m_nDeviceCombo;
    if ((nIndex < 0) | (nIndex >= MV_MAX_DEVICE_NUM))
    {
        MessageBox(TEXT("Please select device"), TEXT("warning"), MB_OK | MB_ICONWARNING);
        return;
    }

    // ch: 由设备信息创建设备实例 || en:get and create example from the device information list
    if (NULL == m_stDevList.pDeviceInfo[nIndex])
    {
        MessageBox(TEXT("Device does not exist"), TEXT("warning"), MB_OK | MB_ICONWARNING);
        return;
    }

    m_pcMyCamera = new CMvCamera;
    // ch: 创建设备失败 || en:fail to create device
    if (NULL == m_pcMyCamera)
    {
        return;
    }

    int nRet = m_pcMyCamera->Open(m_stDevList.pDeviceInfo[nIndex]);
    // ch: 创建设备失败 || en:fail to create device
    if (MV_OK != nRet)
    {
        delete m_pcMyCamera;
        m_pcMyCamera = NULL;
        MessageBox(TEXT("Open Fail"), TEXT("warning"), MB_OK | MB_ICONWARNING);
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
                MessageBox(TEXT("Warning: Set Packet Size fail!"), TEXT("warning"), MB_OK | MB_ICONWARNING);
            }
        }
        else
        {
            MessageBox(TEXT("Warning: Get Packet Size fail!"), TEXT("warning"), MB_OK | MB_ICONWARNING);
        }
    }

    m_bOpenDevice = TRUE;

    OnBnClickedGetParameterButton(); // ch: 获取参数 || en: get parameter
    EnableWindowWhenOpenNotStart();
}

// ch: 按下关闭设备按钮：关闭设备 || en: press button for closing devices: close device
void CRaw2Himage_CDlg::OnBnClickedCloseButton()
{
    CloseDevice();
    EnableWindowWhenClose();
    m_ctrlOpenButton.EnableWindow(TRUE);
}

// ch: 按下连续模式按钮 || en: press button for trigger mode
void CRaw2Himage_CDlg::OnBnClickedContinusModeRadio()
{
    ((CButton *)GetDlgItem(IDC_CONTINUS_MODE_RADIO))->SetCheck(TRUE);
    ((CButton *)GetDlgItem(IDC_TRIGGER_MODE_RADIO))->SetCheck(FALSE);
    m_nTriggerMode = MV_TRIGGER_MODE_OFF;
    int nRet = SetTriggerMode();
    if (MV_OK != nRet)
    {
        return;
    }
    m_ctrlSoftwareOnceButton.EnableWindow(FALSE);

    return;
}

// ch: 按下触发模式按钮 || en: press button to start grabbing
void CRaw2Himage_CDlg::OnBnClickedTriggerModeRadio()
{
    ((CButton *)GetDlgItem(IDC_CONTINUS_MODE_RADIO))->SetCheck(FALSE);
    ((CButton *)GetDlgItem(IDC_TRIGGER_MODE_RADIO))->SetCheck(TRUE);
    m_nTriggerMode = MV_TRIGGER_MODE_ON;
    int nRet = SetTriggerMode();
    if (MV_OK != nRet)
    {
        MessageBox(TEXT("Set Trigger Mode Fail"), TEXT("warning"), MB_OK | MB_ICONWARNING);
        return;
    }

    if (m_bStartGrabbing == TRUE)
    {
        UpdateData(TRUE);
        if (TRUE == m_bSoftWareTriggerCheck)
        {
            m_ctrlSoftwareOnceButton.EnableWindow(TRUE);
        }
    }

    return;
}

unsigned int CRaw2Himage_CDlg::WorkThread(void* pUser)
{
    int nRet = MV_OK;
    CRaw2Himage_CDlg *pBasicDemo = (CRaw2Himage_CDlg*)pUser;
    Halcon::Hobject Hobj;
    unsigned char* pTemp = NULL;
    HTuple * hWindow= (HTuple *)(&pBasicDemo->m_hWindowID);
    MV_FRAME_OUT stImageInfo = {0};
    unsigned char* pImageBuf = NULL;
    unsigned int nImageBufSize = 0;

    while(pBasicDemo->m_bThreadState)
    {
        nRet = pBasicDemo->m_pcMyCamera->GetImageBuffer(&stImageInfo, 1000);
        if(MV_OK == nRet)
        {
            if (IsBayer8PixelFormat(stImageInfo.stFrameInfo.enPixelType))
            {
                nRet = ConvertBayer8ToHalcon(&Hobj, stImageInfo.stFrameInfo.nHeight, stImageInfo.stFrameInfo.nWidth, stImageInfo.stFrameInfo.enPixelType, stImageInfo.pBufAddr);
                if (MV_OK != nRet)
                {
                    break;
                }
            }
            else if(IsColorPixelFormat(stImageInfo.stFrameInfo.enPixelType))//Color
            {
                if (PixelType_Gvsp_RGB8_Packed == stImageInfo.stFrameInfo.enPixelType)
                {
                    pTemp = stImageInfo.pBufAddr;
                }
                else
                {
                    if(NULL == pImageBuf || nImageBufSize < (stImageInfo.stFrameInfo.nWidth * stImageInfo.stFrameInfo.nHeight * 3))
                    {
                        if(pImageBuf)
                        {
                            free(pImageBuf);
                            pImageBuf = NULL;
                        }

                        pImageBuf = (unsigned char *)malloc(stImageInfo.stFrameInfo.nWidth * stImageInfo.stFrameInfo.nHeight * 3);
                        if (pImageBuf == NULL)
                        {
                            break;
                        }
                        nImageBufSize = stImageInfo.stFrameInfo.nWidth * stImageInfo.stFrameInfo.nHeight * 3;
                    }

                    MV_CC_PIXEL_CONVERT_PARAM stPixelConvertParam;
                    memset(&stPixelConvertParam, 0, sizeof(MV_CC_PIXEL_CONVERT_PARAM));
                    stPixelConvertParam.pSrcData = stImageInfo.pBufAddr;
                    stPixelConvertParam.nWidth = stImageInfo.stFrameInfo.nWidth;
                    stPixelConvertParam.nHeight = stImageInfo.stFrameInfo.nHeight;
                    stPixelConvertParam.enSrcPixelType = stImageInfo.stFrameInfo.enPixelType;
                    stPixelConvertParam.nSrcDataLen = stImageInfo.stFrameInfo.nFrameLen;
                    stPixelConvertParam.nDstBufferSize = nImageBufSize;
                    stPixelConvertParam.pDstBuffer = pImageBuf;
                    stPixelConvertParam.enDstPixelType = PixelType_Gvsp_RGB8_Packed;
                    nRet = pBasicDemo->m_pcMyCamera->ConvertPixelType(&stPixelConvertParam);
                    if (MV_OK != nRet)
                    {
                        break;
                    }
                    pTemp = pImageBuf;
                }

                nRet = ConvertRGBToHalcon(&Hobj, stImageInfo.stFrameInfo.nHeight, stImageInfo.stFrameInfo.nWidth, pTemp);
                if (MV_OK != nRet)
                {
                    break;
                }
            }
            else if(IsMonoPixelFormat(stImageInfo.stFrameInfo.enPixelType))//Mono
            {
                if(PixelType_Gvsp_Mono8 == stImageInfo.stFrameInfo.enPixelType)
                {
                    pTemp = stImageInfo.pBufAddr;
                }
                else
                {
                    if(NULL == pImageBuf || nImageBufSize < (stImageInfo.stFrameInfo.nWidth * stImageInfo.stFrameInfo.nHeight))
                    {
                        if(pImageBuf)
                        {
                            free(pImageBuf);
                            pImageBuf = NULL;
                        }

                        pImageBuf = (unsigned char *)malloc(stImageInfo.stFrameInfo.nWidth * stImageInfo.stFrameInfo.nHeight);
                        if (pImageBuf == NULL)
                        {
                            break;
                        }
                        nImageBufSize = stImageInfo.stFrameInfo.nWidth * stImageInfo.stFrameInfo.nHeight;
                    }

                    MV_CC_PIXEL_CONVERT_PARAM stPixelConvertParam;
                    memset(&stPixelConvertParam, 0, sizeof(MV_CC_PIXEL_CONVERT_PARAM));
                    stPixelConvertParam.pSrcData = stImageInfo.pBufAddr;
                    stPixelConvertParam.nWidth = stImageInfo.stFrameInfo.nWidth;
                    stPixelConvertParam.nHeight = stImageInfo.stFrameInfo.nHeight;
                    stPixelConvertParam.enSrcPixelType = stImageInfo.stFrameInfo.enPixelType;
                    stPixelConvertParam.nSrcDataLen = stImageInfo.stFrameInfo.nFrameLen;
                    stPixelConvertParam.nDstBufferSize = nImageBufSize;
                    stPixelConvertParam.pDstBuffer = pImageBuf;
                    stPixelConvertParam.enDstPixelType = PixelType_Gvsp_Mono8;
                    nRet = pBasicDemo->m_pcMyCamera->ConvertPixelType(&stPixelConvertParam);
                    if (MV_OK != nRet)
                    {
                        break;
                    }
                    pTemp = pImageBuf;
                }

                nRet = ConvertMono8ToHalcon(&Hobj, stImageInfo.stFrameInfo.nHeight, stImageInfo.stFrameInfo.nWidth, pTemp);
                if (MV_OK != nRet)
                {
                    break;
                }
            }
            else
            {
                pBasicDemo->m_pcMyCamera->FreeImageBuffer(&stImageInfo);
                continue;
            }
            HalconDisplay(hWindow, Hobj, stImageInfo.stFrameInfo.nHeight, stImageInfo.stFrameInfo.nWidth);

            pBasicDemo->m_pcMyCamera->FreeImageBuffer(&stImageInfo);
        }
        else
        {
            continue;
        }
    }

    if(pImageBuf)
    {
        free(pImageBuf);
        pImageBuf = NULL;
    }

    return 0;
}

// ch: 按下开始采集按钮 || press button stop grabbing
void CRaw2Himage_CDlg::OnBnClickedStartGrabbingButton()
{
    if (FALSE == m_bOpenDevice || TRUE == m_bStartGrabbing || NULL == m_pcMyCamera)
    {
        return;
    }

    m_bThreadState = TRUE;
    unsigned int nThreadID = 0;
    m_hGrabThread = (void*)_beginthreadex( NULL , 0 , WorkThread , this, 0 , &nThreadID );
    if (NULL == m_hGrabThread)
    {
        m_bThreadState = FALSE;
        MessageBox(TEXT("Start Thread Fail"), TEXT("warning"), MB_OK | MB_ICONWARNING);
        return;
    }

    int nRet = m_pcMyCamera->StartGrabbing();
    if (nRet != MV_OK)
    {
        m_bThreadState = FALSE;
        MessageBox(TEXT("Start Grabbing Fail"), TEXT("warning"), MB_OK | MB_ICONWARNING);
    }
    m_bStartGrabbing = TRUE;
    EnableWindowWhenStart();
    return;
}

// ch: 按下结束采集按钮 || en: click button to get parameters
void CRaw2Himage_CDlg::OnBnClickedStopGrabbingButton()
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

    int nRet = nRet = m_pcMyCamera->StopGrabbing();
    if (MV_OK != nRet)
    {
        MessageBox(TEXT("Stop Grabbing Fail"), TEXT("warning"), MB_OK | MB_ICONWARNING);
        return ;
    }
    m_bStartGrabbing = FALSE;
    EnableWindowWhenStop();
    return;
}

// ch: 按下获取参数按钮 || en: click button to get parameters
void CRaw2Himage_CDlg::OnBnClickedGetParameterButton()
{
    int nRet = GetTriggerMode();
    if (nRet != MV_OK)
    {
        MessageBox(TEXT("Get Trigger Mode Fail"), TEXT("warning"), MB_OK | MB_ICONWARNING);
        return;
    }

    nRet = GetTriggerSource();
    if (nRet != MV_OK)
    {
        MessageBox(TEXT("Get Trigger Source Fail"), TEXT("warning"), MB_OK | MB_ICONWARNING);
        return;
    }

    return;
}

// ch: 按下软触发按钮 || en: click software trigger button
void CRaw2Himage_CDlg::OnBnClickedSoftwareTriggerCheck()
{
    int nRet = SetTriggerSource();
    if (nRet != MV_OK)
    {
        return;
    }
    return;
}

// ch: 按下软触发一次按钮 || en: click button to software trigger once
void CRaw2Himage_CDlg::OnBnClickedSoftwareOnceButton()
{
    if (TRUE != m_bStartGrabbing)
    {
        return;
    }

    int nRet = m_pcMyCamera->CommandExecute("TriggerSoftware");
    if (MV_OK != nRet)
    {
        return;
    }

    return;
}

// ch: 右上角退出 || en: quit by clicking top right corner
void CRaw2Himage_CDlg::OnClose()
{
    PostQuitMessage(0);
    CloseDevice();

    CDialog::OnClose();
}

BOOL CRaw2Himage_CDlg::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN&&pMsg->wParam == VK_ESCAPE)
    {
        // ch:如果消息是键盘按下事件，且是Esc键，执行以下代码（什么都不做，你可以自己添加想要的代码）
        // en:If the message is a keyboard press event and a Esc key, execute the following code (nothing is done, you can add the code you want)
        return TRUE;
    }
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
    {
        // ch:如果消息是键盘按下事件，且是Entert键，执行以下代码（什么都不做，你可以自己添加想要的代码）
        // en:If the message is a keyboard press event and a Esc key, execute the following code (nothing is done, you can add the code you want)
        return TRUE;
    }

    return CDialog::PreTranslateMessage(pMsg);
}
