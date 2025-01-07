// ForceIpDemoDlg.cpp : implementation file
#include "stdafx.h"
#include "ForceIpDemo.h"
#include "ForceIpDemoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CAboutDlg dialog used for App About
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// dialog data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV supported

// implementation
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

// CForceIpDemoDlg dialog
CForceIpDemoDlg::CForceIpDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CForceIpDemoDlg::IDD, pParent)
	, m_nDeviceCombo(0)
	, m_dwIpaddress(0)
	, m_dwNetworkMask(0)
    , m_dwDefaultGateway(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CForceIpDemoDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_DEVICE_COMBO, m_ctrlDeviceCombo);
    DDX_CBIndex(pDX, IDC_DEVICE_COMBO, m_nDeviceCombo);
}

BEGIN_MESSAGE_MAP(CForceIpDemoDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_ENUM_DEVICE_BUTTON, CForceIpDemoDlg::OnBnClickedEnumDeviceButton)
    ON_BN_CLICKED(IDC_SET_IP_BUTTON, CForceIpDemoDlg::OnBnClickedSetIpButton)
    ON_CBN_SELCHANGE(IDC_DEVICE_COMBO, CForceIpDemoDlg::OnCbnSelchangeDeviceCombo)
END_MESSAGE_MAP()

// CForceIpDemoDlg message handler
BOOL CForceIpDemoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

    // IDM_ABOUTBOX has to be in range of system's command
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

    // set the icon of this dialog. When main window of application is not dialog, the frame will
    // automatically perform this execution 
	SetIcon(m_hIcon, TRUE);			// set big icon
	SetIcon(m_hIcon, FALSE);		// set small icon 
	
	return TRUE;  // return TRUE unless the focus is on the widget
}

void CForceIpDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// The code below is needed if the you want to add minimzing button to the dialog
//  in order to draw image of the button. For documentation/visual model of the MFC application,
//  this will be finished by frame automatically 
void CForceIpDemoDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // used to draw context of the device

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// make the icon in the middle of the metircs of working area
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//System call this function to get the diaplay of cursor, when user drag the minimized window
HCURSOR CForceIpDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// ch:显示错误信息 | en:Show error message
void CForceIpDemoDlg::ShowErrorMsg(CString csMessage, int nErrorNum)
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
    case MV_E_ACCESS_DENIED:    errorMsg += "No permission ";                                                   break;
    case MV_E_BUSY:             errorMsg += "Device is busy, or network disconnected ";                         break;
    case MV_E_NETER:            errorMsg += "Network error ";                                                   break;
    }
	
    MessageBox(errorMsg, TEXT("PROMPT"), MB_OK | MB_ICONWARNING);
}

void CForceIpDemoDlg::OnBnClickedEnumDeviceButton()
{
	    CString strMsg;

    // ch:清除设备列表框中的信息 | en:clear the information in the device list
    m_ctrlDeviceCombo.ResetContent();

    // ch:初始化设备信息列表 | en:initialize device list
    memset(&m_stDevList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));

    // ch:枚举子网内所有设备 | en:enumerate all the devices in the subnetworks
    int nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE, &m_stDevList);

    if (MV_OK != nRet)
    {
        ShowErrorMsg(TEXT("No device"), nRet);
        return;
    }

    // ch:将值加入到信息列表框中并显示出来 | en:add value to the information list and display it
    unsigned int i;
    int nIp1, nIp2, nIp3, nIp4;
    for (i = 0; i < m_stDevList.nDeviceNum; i++)
    {
        MV_CC_DEVICE_INFO* pDeviceInfo = m_stDevList.pDeviceInfo[i];
        if (NULL == pDeviceInfo)
        {
            continue;
        }
        if (pDeviceInfo->nTLayerType == MV_GIGE_DEVICE)
        {
            nIp1 = ((m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24);
            nIp2 = ((m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16);
            nIp3 = ((m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8);
            nIp4 = (m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

            if (strcmp("", (LPCSTR)(pDeviceInfo->SpecialInfo.stGigEInfo.chUserDefinedName)) != 0)
            {
                strMsg.Format(_T("[%d]GigE: %s  (%d.%d.%d.%d)"), i, 
                    (LPCSTR)(pDeviceInfo->SpecialInfo.stGigEInfo.chUserDefinedName), nIp1, nIp2, nIp3, nIp4);
            }
            else
            {
                strMsg.Format(_T("[%d]GigE: %s  (%d.%d.%d.%d)"), i, (LPCSTR)(pDeviceInfo->SpecialInfo.stGigEInfo.chModelName), nIp1, nIp2, nIp3, nIp4);
            }
            m_ctrlDeviceCombo.AddString(strMsg);
        }
    }
    m_ctrlDeviceCombo.SetCurSel(CB_ERR);

    DisplayDeviceIp();

    UpdateData(FALSE);

    if (0 == m_stDevList.nDeviceNum)
    {
        ShowErrorMsg(TEXT("No device"), 0);
        return;
    }
    
    GetDlgItem(IDC_SET_IP_BUTTON)->EnableWindow(TRUE);
}

void CForceIpDemoDlg::OnBnClickedSetIpButton()
{
    // ch:先读取ip框数据 | en:Read ip data first
    UpdateData(TRUE);
	
    int nIndex = m_nDeviceCombo;
    if ((nIndex < 0) | (nIndex >= MV_MAX_DEVICE_NUM))
    {
        ShowErrorMsg(TEXT("Please select device"), 0);
        return;
    }
	
    // ch:由设备信息创建设备实例 | en:create example of device from the device list
    if (NULL == m_stDevList.pDeviceInfo[nIndex])
    {
        ShowErrorMsg(TEXT("Device does not exist"), 0);
        return;
    }
    
    int nRet = MV_CC_CreateHandle(&m_handle, m_stDevList.pDeviceInfo[nIndex]);
    if (MV_OK != nRet)
    {
        ShowErrorMsg(TEXT("Create handle fail"), nRet);
        return;
    }

	BYTE f1, f2, f3, f4;
	CString str;
	((CIPAddressCtrl*)GetDlgItem(IDC_IPADDRESS))->GetAddress(f1, f2, f3, f4);
	str.Format(TEXT("%d.%d.%d.%d"), f1, f2, f3, f4);
	m_dwIpaddress = (DWORD)ntohl(inet_addr(str));

	((CIPAddressCtrl*)GetDlgItem(IDC_IPADDRESS2))->GetAddress(f1, f2, f3, f4);
	str.Format(TEXT("%d.%d.%d.%d"), f1, f2, f3, f4);
	m_dwDefaultGateway = (DWORD)ntohl(inet_addr(str));

	((CIPAddressCtrl*)GetDlgItem(IDC_IPADDRESS3))->GetAddress(f1, f2, f3, f4);
	str.Format(TEXT("%d.%d.%d.%d"), f1, f2, f3, f4);
	m_dwNetworkMask = (DWORD)ntohl(inet_addr(str));

    // ch:判断设备IP是否可达
    bool bAccessible = MV_CC_IsDeviceAccessible(m_stDevList.pDeviceInfo[nIndex], MV_ACCESS_Exclusive);
    if(bAccessible)
    {
        nRet = MV_GIGE_SetIpConfig(m_handle, MV_IP_CFG_STATIC);
        if (MV_OK != nRet)
        {
            ShowErrorMsg(TEXT("Set Ip config fail"), nRet);
            MV_CC_DestroyHandle(m_handle);
            m_handle = NULL;
            return;
        }
        nRet = MV_GIGE_ForceIpEx(m_handle, m_dwIpaddress, m_dwNetworkMask, m_dwDefaultGateway);
        if (MV_OK != nRet)
        {
            ShowErrorMsg(TEXT("ForceIp fail"), nRet);
            MV_CC_DestroyHandle(m_handle);
            m_handle = NULL;
            return;
        }
    }
    else
    {
        nRet = MV_GIGE_ForceIpEx(m_handle, m_dwIpaddress, m_dwNetworkMask, m_dwDefaultGateway);
        if (MV_OK != nRet)
        {
            ShowErrorMsg(TEXT("ForceIp fail"), nRet);
            MV_CC_DestroyHandle(m_handle);
            m_handle = NULL;
            return;
        }
        MV_CC_DestroyHandle(m_handle);
        m_handle = NULL;
		
        //ch:需要重新创建句柄，设置为静态IP方式进行保存
        m_stDevList.pDeviceInfo[nIndex]->SpecialInfo.stGigEInfo.nCurrentIp = m_dwIpaddress;
        m_stDevList.pDeviceInfo[nIndex]->SpecialInfo.stGigEInfo.nCurrentSubNetMask = m_dwNetworkMask;
        m_stDevList.pDeviceInfo[nIndex]->SpecialInfo.stGigEInfo.nDefultGateWay = m_dwDefaultGateway;
        int nRet = MV_CC_CreateHandle(&m_handle, m_stDevList.pDeviceInfo[nIndex]);
        if (MV_OK != nRet)
        {
            ShowErrorMsg(TEXT("Create handle fail"), nRet);
            return;
        }
		
        nRet = MV_GIGE_SetIpConfig(m_handle, MV_IP_CFG_STATIC);
        if (MV_OK != nRet)
        {
            ShowErrorMsg(TEXT("Set Ip config fail"), nRet);
            MV_CC_DestroyHandle(m_handle);
            m_handle = NULL;
            return;
        }
    }
	
    ShowErrorMsg(TEXT("ForceIp succeed"), 0);
	
    MV_CC_DestroyHandle(m_handle);
    m_handle = NULL;
}

void CForceIpDemoDlg::OnCbnSelchangeDeviceCombo()
{
    UpdateData(TRUE);
    DisplayDeviceIp();
}
	
void CForceIpDemoDlg::DisplayDeviceIp()
{
    int nIndex = m_nDeviceCombo;
    if ((nIndex < 0) | (nIndex >= MV_MAX_DEVICE_NUM))
    {
        ShowErrorMsg(TEXT("Please select device"), 0);
        return;
    }

	char * ipaddr=NULL;
	char addr[20];
	in_addr inaddr;
	
	inaddr.s_addr = m_stDevList.pDeviceInfo[nIndex]->SpecialInfo.stGigEInfo.nCurrentIp;
	ipaddr= inet_ntoa(inaddr);
	strcpy(addr,ipaddr);  
	m_dwIpaddress = inet_addr(ipaddr);
	CIPAddressCtrl* pIP = (CIPAddressCtrl *) GetDlgItem(IDC_IPADDRESS);
    pIP->SetAddress(m_dwIpaddress);
	
	char * defaultGateWay = NULL;
	char chDeftGatWay[20];
	in_addr DefultGateWay;
	DefultGateWay.s_addr = m_stDevList.pDeviceInfo[nIndex]->SpecialInfo.stGigEInfo.nDefultGateWay;
	defaultGateWay = inet_ntoa(DefultGateWay);
	strcpy(chDeftGatWay,defaultGateWay);  
	m_dwDefaultGateway = inet_addr(defaultGateWay);
	CIPAddressCtrl* pDefultGateWay = (CIPAddressCtrl *) GetDlgItem(IDC_IPADDRESS2);
    pDefultGateWay->SetAddress(m_dwDefaultGateway);
	
	char * subMask = NULL;
	char chSubMask[20];
	in_addr SubNetMask;
	SubNetMask.s_addr = m_stDevList.pDeviceInfo[nIndex]->SpecialInfo.stGigEInfo.nCurrentSubNetMask;
	subMask= inet_ntoa(SubNetMask);
	strcpy(chSubMask,subMask);  
	m_dwNetworkMask = inet_addr(subMask);
	CIPAddressCtrl* pSubNetMask = (CIPAddressCtrl *) GetDlgItem(IDC_IPADDRESS3);
    pSubNetMask->SetAddress(m_dwNetworkMask);
	
    UpdateData(FALSE);
}

BOOL CForceIpDemoDlg::PreTranslateMessage(MSG* pMsg)
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

