/*
This example shows the user how to control multiple interfaces and multiple devices at the same time.
This example covers enumeration module, control module and stream module.
*/
#include "stdafx.h"
#include "MultipleCamera.h"
#include "MultipleCameraDlg.h"
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

// CMultipleCameraDlg dialog
CMultipleCameraDlg::CMultipleCameraDlg(CWnd* pParent /*=NULL*/)
: CDialog(CMultipleCameraDlg::IDD, pParent)
, m_bStartGrabbing(FALSE)
, m_nCurCameraIndex(-1)
, m_nCurListIndex(-1)
, m_nInterfaceNum(0)
, m_bOpenIF(FALSE)
, m_bOpenDevice(FALSE)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    for (int i = 0; i < MAX_INTERFACE_NUM; i++)
    {
        for (int j = 0; j < MAX_DEVICE_NUM; j++)
        {
            m_bCamCheck[i][j]   = FALSE;
            m_bDeviceGrabbingFlag[i][j] = FALSE;
        } 

    }
    memset(m_stInterface, 0, sizeof(IF_INFO) * MAX_INTERFACE_NUM);

}

void CMultipleCameraDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    for (int i = 0; i < MAX_DEVICE_NUM; i++)
    {
        DDX_Check(pDX, IDC_CAM1_CHECK+i, m_bCamCheck[FIRST_INTERFACE][i]);
        DDX_Check(pDX, IDC_CAM1_CHECK2+i, m_bCamCheck[1][i]);
    }
    DDX_Control(pDX, IDC_OUTPUT_INFO_LIST, m_ctrlListBoxInfo);
    for (int i = 0;i < MAX_INTERFACE_NUM; i++)
    {
        DDX_Control(pDX, IDC_IF_COMBO + i, m_ctrlInterfaceCombo[i]);

    }
}

BEGIN_MESSAGE_MAP(CMultipleCameraDlg, CDialog)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_WM_TIMER()
    ON_BN_CLICKED(IDC_ENUM_DEVICES_BUTTON, &CMultipleCameraDlg::OnBnClickedEnumDevicesButton)
    ON_BN_CLICKED(IDC_OPEN_DEVICES_BUTTON, &CMultipleCameraDlg::OnBnClickedOpenDevicesButton)
    ON_BN_CLICKED(IDC_CLOSE_DEVICES_BUTTON, &CMultipleCameraDlg::OnBnClickedCloseDevicesButton)
    ON_BN_CLICKED(IDC_START_GRABBING_BUTTON, &CMultipleCameraDlg::OnBnClickedStartGrabbingButton)
    ON_BN_CLICKED(IDC_STOP_GRABBING_BUTTON, &CMultipleCameraDlg::OnBnClickedStopGrabbingButton)
    ON_WM_CLOSE()
    ON_LBN_DBLCLK(IDC_OUTPUT_INFO_LIST, &CMultipleCameraDlg::OnLbnDblclkOutputInfoList)
    ON_WM_LBUTTONDBLCLK()
    ON_BN_CLICKED(IDC_ENUM_INTERFACE_BUTTON, &CMultipleCameraDlg::OnBnClickedEnumInterfaceButton)
    ON_BN_CLICKED(IDC_OPEN_INTERFACE_BUTTON, &CMultipleCameraDlg::OnBnClickedOpenInterfaceButton)
    ON_BN_CLICKED(IDC_CLOSE_INTERFACE_BUTTON, &CMultipleCameraDlg::OnBnClickedCloseInterfaceButton)
END_MESSAGE_MAP()

// ch:工作线程 | en:Working thread
unsigned int __stdcall   WorkThread(void* pUser)
{
    if (pUser)
    {
        CMultipleCameraDlg* pCam = (CMultipleCameraDlg*)pUser;
        if (NULL == pCam)
        {
            return -1;
        }
        int nCurCameraIndex = pCam->m_nCurCameraIndex;
        int nCurListInde    = pCam->m_nCurListIndex;
        pCam->m_nCurCameraIndex = -1;
        pCam->m_nCurListIndex   = -1;
        pCam->ThreadFun(nCurListInde, nCurCameraIndex);
    }
    return 0;
}

void CMultipleCameraDlg::ThreadFun(int nCurListIndex, int nCurCameraIndex)
{
    MV_FG_BUFFER_INFO     stFrameInfo = { 0 };    // 图像信息
    MV_FG_DISPLAY_FRAME_INFO stDisplayFrameInfo = {0};   // 显示信息
    int nRet = MV_FG_SUCCESS;

    while(m_bStartGrabbing && m_bDeviceGrabbingFlag[nCurListIndex][nCurCameraIndex]) //统一一个 m_bStartGrabbing 表示所有相机都取流， 但当相机依次停流时需要对应相机的标识
    {
        nRet = MV_FG_GetFrameBuffer(m_stInterface[nCurListIndex].hStream[nCurCameraIndex], &stFrameInfo, TIMEOUT);
        if (MV_FG_SUCCESS != nRet)
        {
            printf("Get Frame Buffer fail! DevIndex[%d], nRet[%#x]\r\n", nCurCameraIndex, nRet);
            continue;
        }

        if (NULL != stFrameInfo.pBuffer)
        {
            printf("FrameNumber:%2I64d%, Width:%d, Height:%d\n", stFrameInfo.nFrameID, stFrameInfo.nWidth, stFrameInfo.nHeight);

            stDisplayFrameInfo.pImageBuf = (unsigned char*)stFrameInfo.pBuffer;
            stDisplayFrameInfo.nImageBufLen = stFrameInfo.nFilledSize;
            stDisplayFrameInfo.nWidth = stFrameInfo.nWidth;
            stDisplayFrameInfo.nHeight = stFrameInfo.nHeight;
            stDisplayFrameInfo.enPixelType = stFrameInfo.enPixelType;
 
            nRet =  MV_FG_DisplayOneFrame(m_stInterface[nCurListIndex].hStream[nCurCameraIndex], m_stInterface[nCurListIndex].hwndDisplay[nCurCameraIndex], &stDisplayFrameInfo);
            if (MV_FG_SUCCESS != nRet)
            {
                printf("Display OneFrame fail! DevIndex[%d], nRet[%#x]\r\n", nCurCameraIndex, nRet);
            }

            // ch:将缓存放回输入队列 | en:Put the buffer back into the input queue
            nRet = MV_FG_ReleaseFrameBuffer(m_stInterface[nCurListIndex].hStream[nCurCameraIndex], &stFrameInfo);
            if (MV_FG_SUCCESS != nRet)
            {
                printf("Release frame buffer failed! %#x\n", nRet);
            }
        }
    }
    printf("thread [%d][%d] end ...\n", nCurListIndex, nCurCameraIndex);
}

void CMultipleCameraDlg::OnSysCommand(UINT nID, LPARAM lParam)
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
void CMultipleCameraDlg::OnPaint()
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
HCURSOR CMultipleCameraDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

// ch:按钮使能 | en:Enable control
void CMultipleCameraDlg::EnableControls(BOOL bIsCameraReady)
{
    GetDlgItem(IDC_ENUM_INTERFACE_BUTTON)->EnableWindow(m_bOpenIF ? FALSE : TRUE);
    GetDlgItem(IDC_OPEN_INTERFACE_BUTTON)->EnableWindow(m_bOpenIF ? FALSE : (m_nInterfaceNum > 0 ? TRUE : FALSE));
    GetDlgItem(IDC_CLOSE_INTERFACE_BUTTON)->EnableWindow(m_bOpenDevice ? FALSE : m_bOpenIF ? TRUE : FALSE);
    GetDlgItem(IDC_IF_COMBO)->EnableWindow(m_bOpenIF ? FALSE : (m_nInterfaceNum > 0 ? TRUE : FALSE));
    GetDlgItem(IDC_IF_COMBO2)->EnableWindow(m_bOpenIF ? FALSE : (m_nInterfaceNum > 0 ? TRUE : FALSE));

    GetDlgItem(IDC_ENUM_DEVICES_BUTTON)->EnableWindow(m_bOpenDevice ? FALSE : m_bOpenIF ? TRUE : FALSE);
    GetDlgItem(IDC_OPEN_DEVICES_BUTTON)->EnableWindow(m_bOpenDevice ? FALSE : (bIsCameraReady ? TRUE : FALSE));
    GetDlgItem(IDC_CLOSE_DEVICES_BUTTON)->EnableWindow((m_bOpenDevice && bIsCameraReady) ? TRUE : FALSE);

    GetDlgItem(IDC_START_GRABBING_BUTTON)->EnableWindow(m_bStartGrabbing ? FALSE : (m_bOpenDevice && bIsCameraReady) ? TRUE : FALSE);
    GetDlgItem(IDC_STOP_GRABBING_BUTTON)->EnableWindow(m_bStartGrabbing ? TRUE : FALSE);
}

void CMultipleCameraDlg::PrintMessage(const char* pszFormat, ... )
{
    va_list args;
    va_start(args, pszFormat);
    char   szInfo[MAX_LOGINFO_LEN] = {0};
    vsprintf_s(szInfo, MAX_LOGINFO_LEN, pszFormat, args);
    va_end(args);
    m_ctrlListBoxInfo.AddString(CA2T(szInfo));
    m_ctrlListBoxInfo.SetTopIndex(m_ctrlListBoxInfo.GetCount() - 1);
}

// ch:显示错误信息 | en:Show error message
void CMultipleCameraDlg::ShowErrorMsg(CString csMessage, int nErrorNum)
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
    case MV_FG_ERR_IMG_ENCRYPT:         errorMsg += "Image encryption failed ";                             break;
    case MV_FG_ERR_IMG_FORMAT:          errorMsg += "Invalid or unsupport image format ";                   break;
    case MV_FG_ERR_IMG_SIZE:            errorMsg += "Invalid or out of range with image size ";             break;
    case MV_FG_ERR_IMG_STEP:            errorMsg += "Image size doesn't match the step param ";             break;
    case MV_FG_ERR_IMG_DATA_NULL:       errorMsg += "Image data storage address is empty ";                 break;
    default:                            errorMsg += "Undefined Error ";                                     break;
    }

    MessageBox(errorMsg, TEXT("PROMPT"), MB_OK | MB_ICONWARNING);
}


// CMultipleCameraDlg message handlers
BOOL CMultipleCameraDlg::OnInitDialog()
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
    EnableControls(FALSE);

    return TRUE;  // return TRUE  unless you set the focus to a control
}

// 枚举采集卡
void CMultipleCameraDlg::OnBnClickedEnumInterfaceButton()
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
        for (int i = 0; i < MAX_INTERFACE_NUM; i++)
        {
             m_ctrlInterfaceCombo[i].ResetContent();
        }

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
                m_ctrlInterfaceCombo[0].ResetContent();
                m_ctrlInterfaceCombo[1].ResetContent();
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
           
            for (int i = 0; i < MAX_INTERFACE_NUM; i++)
            {
                m_ctrlInterfaceCombo[i].AddString((CString)strIFInfo);
            }
        }
    }

    if (m_nInterfaceNum > 0)
    {
        m_ctrlInterfaceCombo[FIRST_INTERFACE].SetCurSel(0);
        m_ctrlInterfaceCombo[SECOND_INTERFACE].SetCurSel(m_nInterfaceNum > 1 ? 1 : 0);
    }

    EnableControls(FALSE);
}

// 打开采集卡
void CMultipleCameraDlg::OnBnClickedOpenInterfaceButton()
{
    int nRet = MV_FG_SUCCESS;
    int nInterfaceIndex[MAX_INTERFACE_NUM] = {0};

    for (int i = 0; i < MAX_INTERFACE_NUM; i++)
    {
        nInterfaceIndex[i] = m_ctrlInterfaceCombo[i].GetCurSel();
        if (nInterfaceIndex[i] < 0 || nInterfaceIndex[i] >= m_ctrlInterfaceCombo[i].GetCount()) 
        {
            ShowErrorMsg(TEXT("Please select valid interface"), 0);
            return;
        }
    }

    // ch:打开采集卡，获得采集卡句柄 | en:Open interface, get handle
    nRet = MV_FG_OpenInterface((unsigned int)nInterfaceIndex[0], &m_stInterface[FIRST_INTERFACE].hInterface);
    if (MV_FG_SUCCESS != nRet)
    {
        ShowErrorMsg(_TEXT("List 1:Open interface failed"), nRet);
        return;
    }
    
    if (nInterfaceIndex[FIRST_INTERFACE] != nInterfaceIndex[SECOND_INTERFACE])
    {
        nRet = MV_FG_OpenInterface((unsigned int)nInterfaceIndex[1], &m_stInterface[SECOND_INTERFACE].hInterface);
        if (MV_FG_SUCCESS != nRet)
        {
            ShowErrorMsg(_TEXT("List 2:Open interface failed"), nRet);
            return;
        }
        
    }

    m_bOpenIF = TRUE;
   
    for (int j = 0; j < MAX_DEVICE_NUM; j++)
    {
        m_stInterface[FIRST_INTERFACE].hwndDisplay[j] = GetDlgItem(IDC_DISPLAY1_STATIC+j)->GetSafeHwnd();
        GetDlgItem(IDC_DISPLAY1_STATIC+j)->GetWindowRect(&m_stInterface[FIRST_INTERFACE].hwndRect[j]);
        ScreenToClient(&m_stInterface[FIRST_INTERFACE].hwndRect[j]);

        m_stInterface[SECOND_INTERFACE].hwndDisplay[j] = GetDlgItem(IDC_DISPLAY1_STATIC2+j)->GetSafeHwnd();
        GetDlgItem(IDC_DISPLAY1_STATIC2+j)->GetWindowRect(&m_stInterface[SECOND_INTERFACE].hwndRect[j]);
        ScreenToClient(&m_stInterface[SECOND_INTERFACE].hwndRect[j]);
    }

    EnableControls(FALSE);
}

// 关闭采集卡
void CMultipleCameraDlg::OnBnClickedCloseInterfaceButton()
{
    OnClose();
    for (int i = 0; i < MAX_INTERFACE_NUM * MAX_DEVICE_NUM; i++)
    {
        GetDlgItem(IDC_CAM1_CHECK + i)->SetWindowText(0);
    }
    EnableControls(FALSE);
}

// 枚举设备
void CMultipleCameraDlg::OnBnClickedEnumDevicesButton()
{
    int nRet = MV_FG_SUCCESS;
    unsigned int nDeviceNum[MAX_INTERFACE_NUM] = {0};           // ch:每张采集卡下设备数量 | en:Number of devices under each acquisition card
    bool bChanged[MAX_INTERFACE_NUM]  = {false};

    // ch:枚举设备 | en:Enum device
    nRet = MV_FG_UpdateDeviceList(m_stInterface[FIRST_INTERFACE].hInterface, &bChanged[0]);
    if (MV_FG_SUCCESS != nRet)
    {
        ShowErrorMsg(_TEXT("List 1:Enum devices failed"), nRet);
        return;
    }
    
    // ch:获取设备数量 | en:Get device number
    nRet = MV_FG_GetNumDevices(m_stInterface[FIRST_INTERFACE].hInterface, &nDeviceNum[0]);
    if (MV_FG_SUCCESS != nRet)
    {
        ShowErrorMsg(_TEXT("List 1:Get device number failed"), nRet);
        return;
    }

    if (NULL != m_stInterface[SECOND_INTERFACE].hInterface)
    {
        nRet = MV_FG_UpdateDeviceList(m_stInterface[SECOND_INTERFACE].hInterface, &bChanged[1]);
        if (MV_FG_SUCCESS != nRet)
        {
            ShowErrorMsg(_TEXT("List 2:Enum devices failed"), nRet);
            return;
        }

        nRet = MV_FG_GetNumDevices(m_stInterface[SECOND_INTERFACE].hInterface, &nDeviceNum[1]);
        if (MV_FG_SUCCESS != nRet)
        {
            ShowErrorMsg(_TEXT("List 2:Get device number failed"), nRet);
            return;
        }
    }
    

    if (0 == nDeviceNum[FIRST_INTERFACE] && 0 == nDeviceNum[SECOND_INTERFACE])
    {
        ShowErrorMsg(_TEXT("No Device"), 0);
        return;
    }


    if (bChanged[FIRST_INTERFACE] || bChanged[SECOND_INTERFACE])
    {
        // ch:向相机复选框添加设备信息 | en:Add device info in Check box
        unsigned int i = 0;
        for (i = 0; i < (nDeviceNum[FIRST_INTERFACE] + nDeviceNum[SECOND_INTERFACE]) && i < MAX_DEVICE_NUM * MAX_INTERFACE_NUM; i++)
        {
            char                strDevInfo[DEVICE_INFO_LEN] = { 0 };
            MV_FG_DEVICE_INFO   stDeviceInfo = { 0 };

            if (i < nDeviceNum[FIRST_INTERFACE])
            {
                nRet = MV_FG_GetDeviceInfo(m_stInterface[FIRST_INTERFACE].hInterface, i, &stDeviceInfo);
                if (MV_FG_SUCCESS != nRet)
                {
                    ShowErrorMsg(_TEXT("List 1:Get Devices info failed"), nRet);
                    return;
                }

            }
            else
            {
                nRet = MV_FG_GetDeviceInfo(m_stInterface[SECOND_INTERFACE].hInterface, i - nDeviceNum[0], &stDeviceInfo);
                if (MV_FG_SUCCESS != nRet)
                {
                    ShowErrorMsg(_TEXT("List 2:Get Devices info failed"), nRet);
                    return;
                }
            }
            

            switch (stDeviceInfo.nDevType)
            {
            case MV_FG_CXP_DEVICE:
                {
                    if (strcmp("", (LPCSTR)(stDeviceInfo.DevInfo.stCXPDevInfo.chUserDefinedName)) != 0)
                    {
                        sprintf_s(strDevInfo, DEVICE_INFO_LEN, "[CXP]%s [%s]", stDeviceInfo.DevInfo.stCXPDevInfo.chModelName, stDeviceInfo.DevInfo.stCXPDevInfo.chUserDefinedName);
                    }
                    else
                    {
                        sprintf_s(strDevInfo, DEVICE_INFO_LEN, "[CXP]%s (%s)", stDeviceInfo.DevInfo.stCXPDevInfo.chModelName, stDeviceInfo.DevInfo.stCXPDevInfo.chSerialNumber);
                    }
                    break;
                }
            case MV_FG_GEV_DEVICE:
                {
                    if (strcmp("", (LPCSTR)(stDeviceInfo.DevInfo.stGEVDevInfo.chUserDefinedName)) != 0)
                    {
                        sprintf_s(strDevInfo, DEVICE_INFO_LEN, "[GEV]%s [%s]", stDeviceInfo.DevInfo.stGEVDevInfo.chModelName, stDeviceInfo.DevInfo.stGEVDevInfo.chUserDefinedName);
                    }
                    else
                    {
                        sprintf_s(strDevInfo, DEVICE_INFO_LEN, "[GEV]%s (%s)", stDeviceInfo.DevInfo.stGEVDevInfo.chModelName, stDeviceInfo.DevInfo.stGEVDevInfo.chSerialNumber);
                    }
                    break;
                }
            case MV_FG_CAMERALINK_DEVICE:
                {
                    if (strcmp("", (LPCSTR)(stDeviceInfo.DevInfo.stCMLDevInfo.chUserDefinedName)) != 0)
                    {
                        sprintf_s(strDevInfo, DEVICE_INFO_LEN, "[CML]%s [%s]", stDeviceInfo.DevInfo.stCMLDevInfo.chModelName, stDeviceInfo.DevInfo.stCMLDevInfo.chUserDefinedName);
                    }
                    else
                    {
                        sprintf_s(strDevInfo, DEVICE_INFO_LEN, "[CML]%s (%s)", stDeviceInfo.DevInfo.stCMLDevInfo.chModelName, stDeviceInfo.DevInfo.stCMLDevInfo.chSerialNumber);
                    }
                    break;
                }
            case MV_FG_XoF_DEVICE:
                {
                    if (strcmp("", (LPCSTR)(stDeviceInfo.DevInfo.stXoFDevInfo.chUserDefinedName)) != 0)
                    {
                        sprintf_s(strDevInfo, DEVICE_INFO_LEN, "[XoF]%s [%s]", stDeviceInfo.DevInfo.stXoFDevInfo.chModelName, stDeviceInfo.DevInfo.stXoFDevInfo.chUserDefinedName);
                    }
                    else
                    {
                        sprintf_s(strDevInfo, DEVICE_INFO_LEN, "[XoF]%s (%s)", stDeviceInfo.DevInfo.stXoFDevInfo.chModelName, stDeviceInfo.DevInfo.stXoFDevInfo.chSerialNumber);
                    }
                    break;
                }
            default:
                {
                    sprintf_s(strDevInfo, DEVICE_INFO_LEN, "Unknown device type[%d]", i);
                    break;
                }
            }

            if (i < nDeviceNum[0])
            {
                GetDlgItem(IDC_CAM1_CHECK + i)->EnableWindow(TRUE);
                wchar_t strUserName[128] = {0};
                MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strDevInfo), -1, strUserName, 128);
                GetDlgItem(IDC_CAM1_CHECK + i)->SetWindowText(strUserName);
                m_bCamCheck[FIRST_INTERFACE][i] = FALSE;
                m_stInterface[FIRST_INTERFACE].nCurIndex[i] = i;
            }
            else
            {
                GetDlgItem(IDC_CAM1_CHECK2 + (i - nDeviceNum[FIRST_INTERFACE]))->EnableWindow(TRUE);
                wchar_t strUserName[128] = {0};
                MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strDevInfo), -1, strUserName, 128);
                GetDlgItem(IDC_CAM1_CHECK2 + (i - nDeviceNum[FIRST_INTERFACE]))->SetWindowText(strUserName);
                m_bCamCheck[SECOND_INTERFACE][i - nDeviceNum[FIRST_INTERFACE]] = FALSE;
                m_stInterface[SECOND_INTERFACE].nCurIndex[i - nDeviceNum[FIRST_INTERFACE]] = i - nDeviceNum[FIRST_INTERFACE];
            }
            
        }

        // 不可选中无相机信息的复选框，重置相机名
        for (int j= nDeviceNum[0]; j < MAX_DEVICE_NUM; j++)
        {
            char strDevInfo[DEVICE_INFO_LEN] = { 0 };
            wchar_t strUserName[128] = { 0 };
            sprintf_s(strDevInfo, DEVICE_INFO_LEN, "Cam%d", j + 1);
            MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strDevInfo), -1, strUserName, 128);
            GetDlgItem(IDC_CAM1_CHECK + j)->SetWindowText(strUserName);
            GetDlgItem(IDC_CAM1_CHECK + j)->EnableWindow(FALSE);
            m_bCamCheck[FIRST_INTERFACE][j] = FALSE;
        }

        for (int k = nDeviceNum[1]; k < MAX_DEVICE_NUM; k++)
        {
            char strDevInfo[DEVICE_INFO_LEN] = { 0 };
            wchar_t strUserName[128] = { 0 };
            sprintf_s(strDevInfo, DEVICE_INFO_LEN, "Cam%d", k + 1);
            MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strDevInfo), -1, strUserName, 128);
            GetDlgItem(IDC_CAM1_CHECK2 + k)->SetWindowText(strUserName);
            GetDlgItem(IDC_CAM1_CHECK2 + k)->EnableWindow(FALSE);
            m_bCamCheck[SECOND_INTERFACE][k] = FALSE;
        }


        
    }
    for (int i = 0; i< MAX_INTERFACE_NUM; i++)
    {
        PrintMessage("List %d:Total Find %d devices!\r\n", i + 1, nDeviceNum[i]);
    }
    
    EnableControls(TRUE);
    UpdateData(FALSE);
}

// ch:初始化相机，有打开相机操作 | en:Initialzation, include opening device
void CMultipleCameraDlg::OnBnClickedOpenDevicesButton()
{
    if (TRUE == m_bOpenDevice)
    {
        return;
    }

    UpdateData(TRUE);
    BOOL bHaveCheck[MAX_INTERFACE_NUM]  = {FALSE};

    for (int i = 0; i < MAX_INTERFACE_NUM; i++)
    {
        for (unsigned int nDeviceIndex = 0; nDeviceIndex < MAX_DEVICE_NUM; nDeviceIndex++)
        {
            if (!m_bCamCheck[i][nDeviceIndex] || !m_stInterface[i].hInterface)  //已勾选的相机
            {
                continue;
            }
        
            bHaveCheck[i] = TRUE;
            int nRet = MV_FG_OpenDevice(m_stInterface[i].hInterface, m_stInterface[i].nCurIndex[nDeviceIndex], &(m_stInterface[i].hDevice[nDeviceIndex]));
            if (MV_FG_SUCCESS != nRet)
            {
                PrintMessage("List %d:Open device failed! DevIndex[%d], nRet[%#x]\r\n", i + 1, nDeviceIndex+1, nRet);
                m_stInterface[i].hDevice[nDeviceIndex] = NULL;
                continue;
            }
            else
            {
                PrintMessage("List %d:Open device success! DevIndex[%d]", i + 1, nDeviceIndex + 1);
                m_bOpenDevice = TRUE;
            }

            // ch:设置连续采集模式 | en:Set Continuous Aquisition Mode
            MV_FG_SetEnumValue(m_stInterface[i].hDevice[nDeviceIndex], "AcquisitionMode", 2);  // 0 - SingleFrame, 2 - Continuous
            MV_FG_SetEnumValue(m_stInterface[i].hDevice[nDeviceIndex], "TriggerMode", (unsigned int)0);  // 0 - Trigger off, 1 - Trigger on
            
        }
    }

    if (TRUE == m_bOpenDevice)
    {
        EnableControls(TRUE);
        for (int i = 0; i < MAX_INTERFACE_NUM * MAX_DEVICE_NUM; i++)
        {
            GetDlgItem(IDC_CAM1_CHECK + i)->EnableWindow(FALSE);
        }
    }
    else
    {
        for (int i = 0; i < MAX_INTERFACE_NUM; i++)
        {
            if (FALSE == bHaveCheck[i])
            {
                PrintMessage("List %d:Unchecked device!\r\n", i + 1);
            }
            else
            {
                PrintMessage("List %d:No device opened successfully!\r\n", i + 1);
            }
        }
        

    }
    
    
    
}

// ch:开始取图 | en:Start grabbing
void CMultipleCameraDlg::OnBnClickedStartGrabbingButton()
{
    if (FALSE == m_bOpenDevice || TRUE == m_bStartGrabbing)
    {        
        return;
    }

    int nRet = MV_FG_SUCCESS;

    for (int i = 0; i < MAX_INTERFACE_NUM; i++)
    {
        for (int j = 0; j < MAX_DEVICE_NUM; j++)
        {
            if (!m_stInterface[i].hDevice[j])
            {
                continue;
            }
        
            // ch:获取流通道个数 | en:Get number of stream
            unsigned int nStreamNum = 0;
            nRet = MV_FG_GetNumStreams(m_stInterface[i].hDevice[j], &nStreamNum);
            if (MV_FG_SUCCESS != nRet || 0 == nStreamNum)
            {
                PrintMessage("List %d:No stream available! number = %d, DevIndex[%d], nRet[%#x]\r\n", i + 1, nStreamNum, i+1, nRet);
                continue;
            }

            // ch:打开流通道(目前只支持单个通道) | en:Open stream(Only a single stream is supported now)
            nRet = MV_FG_OpenStream(m_stInterface[i].hDevice[j], 0, &m_stInterface[i].hStream[j]);
            if (MV_FG_SUCCESS != nRet)
            {
                PrintMessage("List %d:Open Stream fail! DevIndex[%d], nRet[%#x]\r\n", i + 1, j+1, nRet);
                continue;
            }

            // ch:设置SDK内部缓存数量 | en:Set internal buffer number
            nRet = MV_FG_SetBufferNum(m_stInterface[i].hStream[j], BUFFER_NUMBER);
            if (MV_FG_SUCCESS != nRet)
            {
                PrintMessage("List %d:Set buffer number failed! DevIndex[%d], nRet[%#x]\r\n", i + 1, j+1, nRet);
                nRet = MV_FG_CloseStream(m_stInterface[i].hStream[j]);
                if (MV_FG_SUCCESS != nRet)
                {
                    printf("List %d:Close stream failed! %#x\n", i + 1, nRet);
                }
                m_stInterface[i].hStream[j] = NULL;
                continue;
            }


            // ch:开始取流 | en:Start Acquisition
            nRet = MV_FG_StartAcquisition(m_stInterface[i].hStream[j]);
            if (MV_FG_SUCCESS != nRet)
            {
                PrintMessage("List %d:Start Acquisition failed! DevIndex[%d], nRet[%#x]\r\n", i + 1, j+1, nRet);
                nRet = MV_FG_CloseStream(m_stInterface[i].hStream[j]);
                if (MV_FG_SUCCESS != nRet)
                {
                    printf("List %d:Close stream failed! %#x\n", i + 1, nRet);
                }
                m_stInterface[i].hStream[j] = NULL;
                continue;
            }

            m_bStartGrabbing = TRUE;

            // ch:开始采集之后才创建workthread线程
            unsigned int nThreadID = 0;
            m_nCurListIndex   = i;
            m_nCurCameraIndex = j;
            m_stInterface[i].hGrabThread[j] = (void*) _beginthreadex( NULL , 0 , WorkThread , this, 0 , &nThreadID );
            if (NULL == m_stInterface[i].hGrabThread[j])
            {
                PrintMessage("List %d:Create grab thread fail! DevIndex[%d]\r\n", i + 1, j+1);
            }
            m_bDeviceGrabbingFlag[i][j] = TRUE;
        
        }
    }
    
    EnableControls(TRUE);
}

// ch:结束取图 | en:Stop grabbing
void CMultipleCameraDlg::OnBnClickedStopGrabbingButton()
{
    if (FALSE == m_bOpenDevice || FALSE == m_bStartGrabbing)
    {        
        return;
    }

    int nRet = MV_FG_SUCCESS;

    for (int i = 0; i < MAX_INTERFACE_NUM; i++)
    {
        for (int j = 0; j < MAX_DEVICE_NUM; j++)
        {
            if (!m_stInterface[i].hStream[j])
            {
                continue;
            }
        
            m_bDeviceGrabbingFlag[i][j] = FALSE;

            if (m_stInterface[i].hGrabThread[j])
            {
                WaitForSingleObject(m_stInterface[i].hGrabThread[j], INFINITE);
                CloseHandle(m_stInterface[i].hGrabThread[j]);
                
                m_stInterface[i].hGrabThread[j] = NULL;
            }

            nRet = MV_FG_StopAcquisition(m_stInterface[i].hStream[j]);

            if (MV_FG_SUCCESS != nRet)
            {
                PrintMessage("List %d:Stop grabbing fail! DevIndex[%d], nRet[%#x]\r\n", i + 1, j+1, nRet);
            }

            nRet = MV_FG_CloseStream(m_stInterface[i].hStream[j]);
            if (MV_FG_SUCCESS != nRet)
            {
                printf("List %d:Close stream failed! %#x\n", i + 1, nRet);
            }
            m_stInterface[i].hStream[j] = NULL;

        }

    }
    
    m_bStartGrabbing = FALSE;

    EnableControls(TRUE);
}

void CMultipleCameraDlg::OnClose()
{
    m_bStartGrabbing = FALSE;
    OnBnClickedCloseDevicesButton();
   // memset(m_bOpenDevice, FALSE, sizeof(m_bOpenDevice));

    int nRet = MV_FG_SUCCESS;
    nRet = MV_FG_CloseInterface(m_stInterface[FIRST_INTERFACE].hInterface);
    if (MV_FG_SUCCESS != nRet)
    {
        PrintMessage("List 1:Close interface fail! nRet[%#x]\r\n", nRet);
    }
    
    if (NULL != m_stInterface[SECOND_INTERFACE].hInterface)
    {
        nRet = MV_FG_CloseInterface(m_stInterface[SECOND_INTERFACE].hInterface);
        if (MV_FG_SUCCESS != nRet)
        {
            PrintMessage("List 2:Close interface fail! nRet[%#x]\r\n", nRet);
        }
    }

    memset(m_stInterface, 0, sizeof(IF_INFO) * MAX_INTERFACE_NUM);
    m_bOpenIF = FALSE;

    CDialog::OnClose();
}

// ch:关闭，包含销毁句柄 | en:Close, include destroy handle
void CMultipleCameraDlg::OnBnClickedCloseDevicesButton()
{
    OnBnClickedStopGrabbingButton();

    int nRet = MV_FG_SUCCESS;
    for (int i = 0; i < MAX_INTERFACE_NUM; i++)
    {
        for (int j = 0; j < MAX_DEVICE_NUM; j++)
        {
            if (!m_stInterface[i].hDevice[j])
            {
                continue;
            }
        
            nRet = MV_FG_CloseDevice(m_stInterface[i].hDevice[j]);
            if (MV_FG_SUCCESS != nRet)
            {
                PrintMessage("List %d:Close device fail! DevIndex[%d], nRet[%#x]\r\n", i+1, j+1, nRet);
            }
            m_stInterface[i].hDevice[j] = NULL;
        

        }
        
    }
    

    for (int i = 0; i < MAX_DEVICE_NUM; i++)
    {
        m_bCamCheck[FIRST_INTERFACE][i] = FALSE;
        m_bCamCheck[1][i] = FALSE;
        GetDlgItem(IDC_CAM1_CHECK + i)->EnableWindow(TRUE);
        GetDlgItem(IDC_CAM1_CHECK2 + i)->EnableWindow(TRUE);

    }

    m_bOpenDevice = FALSE;
    EnableControls(TRUE);
    UpdateData(FALSE);

    if(!FreeConsole())
    {
        AfxMessageBox(_T("Could not free the console!"));
    }
    Invalidate();//刷新主界面
}

BOOL CMultipleCameraDlg::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN &&pMsg->wParam == VK_ESCAPE)
    {
        return TRUE;
    }
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
    {
        return TRUE;
    }

    return CDialog::PreTranslateMessage(pMsg);
}

void CMultipleCameraDlg::OnLbnDblclkOutputInfoList()
{
    m_ctrlListBoxInfo.ResetContent();
}
