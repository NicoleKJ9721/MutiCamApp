
// SerialDemoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "string"
#include "SerialDemo.h"
#include "SerialDemoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

double MyMilliseconds()
{
#ifdef WIN32
    LARGE_INTEGER ticks;
    QueryPerformanceCounter(&ticks);
    LARGE_INTEGER resolution;
    QueryPerformanceFrequency(&resolution);
    double dticks = (double)ticks.QuadPart;
    double dresolution = (double)resolution.QuadPart;
    return dticks / dresolution * 1000.0;
#else
    return 0.0;
#endif
}
static void __stdcall ExceptionCallback(unsigned int nMsgType, void* pUser)
{
    if (pUser)
    {
        CSerialDemoDlg* pDlg = (CSerialDemoDlg*)pUser;
        if (pDlg)
        {
            pDlg->DisconnectProcess();
        }
    }
}

// CSerialDemoDlg dialog
CSerialDemoDlg::CSerialDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSerialDemoDlg::IDD, pParent)
    , m_nWidth(0)
    , m_nHeight(0)
    , m_nNodeType(0)
    , m_pSerialImpl(0)
    , m_bIsOpen(FALSE)
    , m_fFrameRate(0)
    , m_pRecvBuffer(0)
    , m_nBufferSize(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSerialDemoDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);

    DDX_Text(pDX, IDC_EDIT_WIDTH, m_nWidth);
    DDX_Text(pDX, IDC_EDIT_HEIGHT, m_nHeight);
    DDX_CBIndex(pDX, IDC_COMBO_NODE_TYPE, m_nNodeType);
    DDX_Text(pDX, IDC_EDIT_FRAME_RATE, m_fFrameRate);
}

BEGIN_MESSAGE_MAP(CSerialDemoDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_BUTTON_Open, &CSerialDemoDlg::OnBnClickedButtonOpen)
    ON_BN_CLICKED(IDC_BUTTON_Close, &CSerialDemoDlg::OnBnClickedButtonClose)
    ON_BN_CLICKED(IDC_BUTTON_Write, &CSerialDemoDlg::OnBnClickedButtonWrite)
    ON_BN_CLICKED(IDC_BUTTON_Read, &CSerialDemoDlg::OnBnClickedButtonRead)
    ON_BN_CLICKED(IDC_BUTTON_ClearInfo, &CSerialDemoDlg::OnBnClickedButtonClearinfo)
    ON_WM_TIMER()
    ON_WM_CLOSE()
    ON_BN_CLICKED(IDC_BUTTON_ENUM, &CSerialDemoDlg::OnBnClickedButtonEnum)
    ON_BN_CLICKED(IDC_BUTTON_SET_PARAM, &CSerialDemoDlg::OnBnClickedButtonSetParam)
    ON_BN_CLICKED(IDC_BUTTON_GET_PARAM, &CSerialDemoDlg::OnBnClickedButtonGetParam)
    ON_BN_CLICKED(IDC_BUTTON_SETUP, &CSerialDemoDlg::OnBnClickedButtonSetup)
    ON_BN_CLICKED(IDC_BUTTON_Send, &CSerialDemoDlg::OnBnClickedButtonSend)
    ON_BN_CLICKED(IDC_BUTTON_Send2, &CSerialDemoDlg::OnBnClickedButtonSend2)
END_MESSAGE_MAP()


// CSerialDemoDlg message handlers

BOOL CSerialDemoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
    ResetMember();


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

	// TODO: Add extra initialization here

    EnableControl(FALSE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSerialDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	CDialog::OnSysCommand(nID, lParam);
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSerialDemoDlg::OnPaint()
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
HCURSOR CSerialDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CSerialDemoDlg::ResetMember()
{
    m_nWidth = 0;
    m_nHeight = 0;

    memset(m_pstDeviceInfo, 0, sizeof(MV_SERIAL_DEVICE_INFO*) * MV_SERIAL_MAX_DEVICE);
    m_nDeviceNum = 0;

    ((CComboBox*)GetDlgItem(IDC_COMBO_ENUM_COM_NAME))->SetCurSel(0);
    ((CComboBox*)GetDlgItem(IDC_COMBO_ENUM_BAUDRATE))->SetCurSel(1);
    ((CComboBox*)GetDlgItem(IDC_COMBO_NODE_TYPE))->SetCurSel(0);
    ((CComboBox*)GetDlgItem(IDC_COMBO_SETUP_BAUDRATE))->SetCurSel(1);
}


void CSerialDemoDlg::OnBnClickedButtonOpen()
{
    CListBox* pListBox = (CListBox*)GetDlgItem(IDC_LIST_InfoBox);
    CString       strMsg;
    int           nComboIndex    =  ((CComboBox*)GetDlgItem(IDC_COMBO_Port))->GetCurSel();
    if (nComboIndex >= m_nDeviceNum)
    {
        return;
    }

    int nRet = MV_SERIAL_OK;
    do 
    {
        if (m_pSerialImpl == NULL)
        {
            int nRet = MV_SERIAL_CreateHandle(&m_pSerialImpl);
            if (MV_SERIAL_OK != nRet)
            {
                return;
            }
        }

        nRet = MV_SERIAL_Open(m_pSerialImpl, m_pstDeviceInfo[nComboIndex]);
        if (MV_SERIAL_OK != nRet)
        {
            break;
        }

        m_bIsOpen = true;
        EnableControl(TRUE);
        OnBnClickedButtonGetParam();

        MV_SERIAL_RegisterExceptionCallBack(m_pSerialImpl, ExceptionCallback, this);
    } while (0);

    strMsg.Format(_T("[Open] nRet = %x"),nRet);
    pListBox->InsertString(0,strMsg);

}

void CSerialDemoDlg::OnBnClickedButtonClose()
{
    CString strMsg;
    CListBox* pListBox = (CListBox*)GetDlgItem(IDC_LIST_InfoBox);
    if(NULL == m_pSerialImpl)
    {
        strMsg.Format(_T("Close Fail m_pSerialImpl = NULL"));
        pListBox->InsertString(0,strMsg);
        return;
    }
    int nRet = MV_SERIAL_Close(m_pSerialImpl);
    m_bIsOpen = false;
    EnableControl(TRUE);

    if (m_pSerialImpl)
    {
        MV_SERIAL_DestroyHandle(m_pSerialImpl);
        m_pSerialImpl = NULL;
    }

    strMsg.Format(_T("[Close] nRet = %x"),nRet);
    pListBox->InsertString(0,strMsg);

    EnableControl(TRUE);
}

void CSerialDemoDlg::OnBnClickedButtonWrite()
{
    CString strMsg;
    CListBox* pListBox = (CListBox*)GetDlgItem(IDC_LIST_InfoBox);
    if(NULL == m_pSerialImpl)
    {
        strMsg.Format(_T("Write Fail m_pSerialImpl = NULL"));
        pListBox->InsertString(0,strMsg);
        return;
    }

    UpdateData(TRUE);

    char chKey[256] = {0};
    char chValue[256] = {0};
    GetDlgItemText(IDC_EDIT_KEY_NAME,(LPTSTR)chKey,256);
    GetDlgItemText(IDC_EDIT_KEY_VALUE,(LPTSTR)chValue,256);

    int nRet = MV_SERIAL_OK;
    MV_SERIAL_NODE_TYPE enNodeType = (MV_SERIAL_NODE_TYPE)m_nNodeType;
    if (enNodeType == MV_NODE_TYPE_INT
        || enNodeType == MV_NODE_TYPE_BOOL)
    {
        int nValue = atoi(chValue);
        nRet = MV_SERIAL_SetValue(m_pSerialImpl, chKey, enNodeType, &nValue, sizeof(int));
    }
    else if(enNodeType == MV_NODE_TYPE_STRING
		|| enNodeType == MV_NODE_TYPE_FLOAT)
    {
        nRet = MV_SERIAL_SetValue(m_pSerialImpl, chKey, enNodeType, chValue, 256);
    }
    strMsg.Format(_T("[Write] Key[%s] nRet = 0x%x"), chKey, nRet);
    pListBox->InsertString(0,strMsg);
}

void CSerialDemoDlg::OnBnClickedButtonRead()
{
    unsigned int nReadNum = 0;
    CString strMsg;
    CString str;
    CListBox* pListBox = (CListBox*)GetDlgItem(IDC_LIST_InfoBox);
    if(NULL == m_pSerialImpl)
    {
        strMsg.Format(_T("Read Fail m_pSerialImpl = NULL"));
        pListBox->InsertString(0,strMsg);
        return;
    }

    UpdateData(TRUE);

    char chKey[256] = {0};
    char chValue[256] = {0};
    GetDlgItemText(IDC_EDIT_KEY_NAME,(LPTSTR)chKey,256);


    int nRet = MV_SERIAL_OK;

    MV_SERIAL_NODE_TYPE enNodeType = (MV_SERIAL_NODE_TYPE)m_nNodeType;
    if (enNodeType == MV_NODE_TYPE_INT
        || enNodeType == MV_NODE_TYPE_BOOL)
    {
        int nValue = 0;
        nRet = MV_SERIAL_GetValue(m_pSerialImpl, chKey, enNodeType, &nValue, sizeof(int));
        if (MV_SERIAL_OK == nRet)
        {
            _itoa(nValue, chValue, 10);
            SetDlgItemText(IDC_EDIT_KEY_VALUE,(LPTSTR)chValue);
        }
    }
    else if(enNodeType == MV_NODE_TYPE_FLOAT)
    {
        float fValue = 0;
        nRet = MV_SERIAL_GetValue(m_pSerialImpl, chKey, enNodeType, &fValue, sizeof(float));
        if (MV_SERIAL_OK == nRet)
        {
            gcvt(fValue, 2, chValue);
            SetDlgItemText(IDC_EDIT_KEY_VALUE,(LPTSTR)chValue);
        }
    }
    else if(enNodeType == MV_NODE_TYPE_STRING)
    {
        nRet = MV_SERIAL_GetValue(m_pSerialImpl, chKey, enNodeType, chValue, 256);
        if (MV_SERIAL_OK == nRet)
        {
            SetDlgItemText(IDC_EDIT_KEY_VALUE,(LPTSTR)chValue);
        }
    }


    strMsg.Format(_T("[Read] Key[%s], Value[%s], nRet = %x"), chKey, chValue, nRet);
    pListBox->InsertString(0,strMsg);
}

void CSerialDemoDlg::OnBnClickedButtonClearinfo()
{
    ((CListBox*)GetDlgItem(IDC_LIST_InfoBox))->ResetContent();
}

void CSerialDemoDlg::OnTimer(UINT_PTR nIDEvent)
{
    // TODO: Add your message handler code here and/or call default
    CDialog::OnTimer(nIDEvent);
}

void CSerialDemoDlg::OnClose()
{
    Deinit();

    CDialog::OnClose();
}

void CSerialDemoDlg::OnBnClickedButtonEnum()
{
    for (unsigned int i = 0; i < m_nDeviceNum && i< MV_SERIAL_MAX_DEVICE; i++)
    {
        if (m_pstDeviceInfo[i])
        {
            delete m_pstDeviceInfo[i];
            m_pstDeviceInfo[i] = NULL;
        }
    }
    m_nDeviceNum = 0;

    //串口
    CString strComName;
    CComboBox* pComboPort = (CComboBox*)GetDlgItem(IDC_COMBO_Port);
    CComboBox* pComboBaudrate = (CComboBox*)GetDlgItem(IDC_COMBO_ENUM_BAUDRATE);
    CComboBox* pComboCOMName = (CComboBox*)GetDlgItem(IDC_COMBO_ENUM_COM_NAME);

    CString ctrBaudrate;
    pComboBaudrate->GetLBText(pComboBaudrate->GetCurSel(), ctrBaudrate);
    pComboPort->ResetContent();

    MV_SERIAL_DEVICE_INFO stDeviceInfo;
    int nComIndex = pComboCOMName->GetCurSel();

	// the current demo support com port(0,20), if u need COM20+, u should add by yourself.
    if (nComIndex == 0) 
    {
        for(int i = 0; i < 21; i++)
        {
			strComName.Format(_T("COM%d"),i);

            double ts = MyMilliseconds();
            int nRet = MV_SERIAL_EnumDevice(strComName.GetBuffer(), (MV_SERIAL_BAUDRATE)atoi(ctrBaudrate.GetBuffer()), &stDeviceInfo);
            if(nRet == MV_SERIAL_OK && stDeviceInfo.enStatus == MV_STATUS_AVAILABLE)
            {
                strComName.AppendFormat(_T(" Cost %f"), MyMilliseconds() - ts);
                ((CListBox*)GetDlgItem(IDC_LIST_InfoBox))->InsertString(0, strComName);
                MV_SERIAL_DEVICE_INFO* pstDeviceInfo = new MV_SERIAL_DEVICE_INFO();
                memcpy(pstDeviceInfo, &stDeviceInfo, sizeof(MV_SERIAL_DEVICE_INFO));
                m_pstDeviceInfo[m_nDeviceNum] = pstDeviceInfo;
                m_nDeviceNum++;

                strComName.Format(_T("COM%d %s(%s)"),i, stDeviceInfo.chDeviceModelName, stDeviceInfo.chSerialNumber);
                pComboPort->AddString(strComName);
            }
            strComName.ReleaseBuffer();
            ctrBaudrate.ReleaseBuffer();
        }
    }
    else
    {
        strComName.Format(_T("COM%d"), nComIndex);
        double ts = MyMilliseconds();
        int nRet = MV_SERIAL_EnumDevice(strComName.GetBuffer(), (MV_SERIAL_BAUDRATE)atoi(ctrBaudrate.GetBuffer()), &stDeviceInfo);
        if(nRet == MV_SERIAL_OK && stDeviceInfo.enStatus == MV_STATUS_AVAILABLE)
        {
            strComName.AppendFormat(_T(" Cost %f"), MyMilliseconds() - ts);
            ((CListBox*)GetDlgItem(IDC_LIST_InfoBox))->InsertString(0, strComName);
            MV_SERIAL_DEVICE_INFO* pstDeviceInfo = new MV_SERIAL_DEVICE_INFO();
            memcpy(pstDeviceInfo, &stDeviceInfo, sizeof(MV_SERIAL_DEVICE_INFO));
            m_pstDeviceInfo[m_nDeviceNum] = pstDeviceInfo;
            m_nDeviceNum++;

            strComName.Format(_T("COM%d %s(%s)"), nComIndex, stDeviceInfo.chDeviceModelName, stDeviceInfo.chSerialNumber);
            pComboPort->AddString(strComName);
        }
        strComName.ReleaseBuffer();
        ctrBaudrate.ReleaseBuffer();
    }
    
    if (pComboPort->GetCount() > 0)
    {
        pComboPort->SetCurSel(0);
        EnableControl(TRUE);
    }
    else
    {
        EnableControl(FALSE);
        MessageBox(_T("None Device"));
    }

}

void CSerialDemoDlg::Deinit()
{
    if (m_pSerialImpl)
    {
        MV_SERIAL_DestroyHandle(m_pSerialImpl);
        m_pSerialImpl = NULL;
    }

    for (unsigned int i = 0; i < m_nDeviceNum && i< MV_SERIAL_MAX_DEVICE; i++)
    {
        if (m_pstDeviceInfo[i])
        {
            delete m_pstDeviceInfo[i];
            m_pstDeviceInfo[i] = NULL;
        }
    }
    m_nDeviceNum = 0;
}

void CSerialDemoDlg::EnableControl(bool bHasEnum)
{
    //枚举
    GetDlgItem(IDC_COMBO_ENUM_COM_NAME)->EnableWindow(m_bIsOpen ? FALSE : TRUE);
    GetDlgItem(IDC_COMBO_ENUM_BAUDRATE)->EnableWindow(m_bIsOpen ? FALSE : TRUE);
    GetDlgItem(IDC_COMBO_Port)->EnableWindow(m_bIsOpen ? FALSE : TRUE);
    GetDlgItem(IDC_BUTTON_ENUM)->EnableWindow(m_bIsOpen ? FALSE : TRUE);

    //相机操作
    GetDlgItem(IDC_BUTTON_Open)->EnableWindow(bHasEnum ? (!m_bIsOpen) : FALSE);
    GetDlgItem(IDC_BUTTON_Close)->EnableWindow(m_bIsOpen);
    GetDlgItem(IDC_BUTTON_GET_PARAM)->EnableWindow(m_bIsOpen);
    GetDlgItem(IDC_BUTTON_SET_PARAM)->EnableWindow(m_bIsOpen);

    //参数
    GetDlgItem(IDC_EDIT_WIDTH)->EnableWindow(m_bIsOpen);
    GetDlgItem(IDC_EDIT_HEIGHT)->EnableWindow(m_bIsOpen);
    GetDlgItem(IDC_COMBO_SETUP_BAUDRATE)->EnableWindow(m_bIsOpen);
    GetDlgItem(IDC_BUTTON_SETUP)->EnableWindow(m_bIsOpen);
    //读写
    GetDlgItem(IDC_COMBO_NODE_TYPE)->EnableWindow(m_bIsOpen);
    GetDlgItem(IDC_EDIT_KEY_NAME)->EnableWindow(m_bIsOpen);
    GetDlgItem(IDC_EDIT_KEY_VALUE)->EnableWindow(m_bIsOpen);
    GetDlgItem(IDC_BUTTON_Read)->EnableWindow(m_bIsOpen);
    GetDlgItem(IDC_BUTTON_Write)->EnableWindow(m_bIsOpen);

}

void CSerialDemoDlg::OnBnClickedButtonSetParam()
{
    UpdateData(TRUE);

    int nRet = MV_SERIAL_SetValue(m_pSerialImpl, "Width", MV_NODE_TYPE_INT, (void*)&m_nWidth, sizeof(m_nWidth));
    if(MV_SERIAL_OK != nRet)
    {
        ((CListBox*)GetDlgItem(IDC_LIST_InfoBox))->InsertString(0, _T("Set Width failed"));
    }
    else
    {
        ((CListBox*)GetDlgItem(IDC_LIST_InfoBox))->InsertString(0, _T("Set Width success"));
    }

    nRet = MV_SERIAL_SetValue(m_pSerialImpl, "Height", MV_NODE_TYPE_INT, &m_nHeight, sizeof(m_nHeight));
    if(MV_SERIAL_OK != nRet)
    {
        ((CListBox*)GetDlgItem(IDC_LIST_InfoBox))->InsertString(0, _T("Set Height failed"));
    }
    else
    {
        ((CListBox*)GetDlgItem(IDC_LIST_InfoBox))->InsertString(0, _T("Set Height success"));
    }

    UpdateData(FALSE);
}

void CSerialDemoDlg::OnBnClickedButtonGetParam()
{
    int nRet = MV_SERIAL_OK;

    CString str;
    CListBox* pInfoList = (CListBox*)GetDlgItem(IDC_LIST_InfoBox);

    nRet = MV_SERIAL_GetValue(m_pSerialImpl, "Width", MV_NODE_TYPE_INT, &m_nWidth, sizeof(m_nWidth));
    str.Format(_T("GetValue INT Width[%d], nRet[%x]"),m_nWidth, nRet);
    pInfoList->InsertString(0, str);

    MV_SERIAL_GetValue(m_pSerialImpl, "Height", MV_NODE_TYPE_INT, &m_nHeight, sizeof(m_nHeight));
    str.Format(_T("GetValue INT Height[%d], nRet[%x]"),m_nHeight, nRet);
    pInfoList->InsertString(0, str);

    MV_SERIAL_GetValue(m_pSerialImpl, "ResultingFrameRate", MV_NODE_TYPE_FLOAT, &m_fFrameRate, sizeof(float));
    str.Format(_T("GetValue FLOAT ResultingFrameRate[%f], nRet[%x]"),m_fFrameRate, nRet);
    pInfoList->InsertString(0, str);

    UpdateData(FALSE);
}

BOOL CSerialDemoDlg::PreTranslateMessage(MSG* pMsg)
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

void CSerialDemoDlg::DisconnectProcess()
{
    MessageBox(_T("Device Disconnect!"));
}

void CSerialDemoDlg::OnBnClickedButtonSetup()
{
    if (!m_bIsOpen)
    {
        return;
    }

    UpdateData(TRUE);

    CComboBox* pComboBaud = (CComboBox*)GetDlgItem(IDC_COMBO_SETUP_BAUDRATE);
    if(pComboBaud)
    {
        CString ctr;
        pComboBaud->GetLBText(pComboBaud->GetCurSel(), ctr);

        MV_SERIAL_SetBaudrate(m_pSerialImpl, (MV_SERIAL_BAUDRATE)atoi(ctr.GetBuffer()));
        ctr.ReleaseBuffer();
    }
}

void CSerialDemoDlg::OnBnClickedButtonSend()
{
    CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_SEND);
    if(pEdit)
    {
        CString ctr;
        pEdit->GetWindowText(ctr);

        unsigned int nSize= 1024*1024;
        char* pData = ctr.GetBuffer(0);
        if (NULL == m_pRecvBuffer)
        {
            m_pRecvBuffer = (unsigned char*)malloc(1024*1024);
            if (NULL == m_pRecvBuffer)
            {
                return;
            }
            m_nBufferSize = 1024*1024;
        }
        
        char chData[128] = {0};
        sprintf((char*)chData, "%s\r", pData);
        int nRet = MV_SERIAL_ReadMem(m_pSerialImpl, chData, strlen((char*)chData), (char*)m_pRecvBuffer, m_nBufferSize);

        {
            CListBox* pInfoList = (CListBox*)GetDlgItem(IDC_LIST_InfoBox);
            CString strInfo;
            strInfo.Format(_T("Send:%s, Result:[%s]"), pData, m_pRecvBuffer);
            pInfoList->InsertString(0, strInfo);
        }
        ctr.ReleaseBuffer();
    }
}

void CSerialDemoDlg::OnBnClickedButtonSend2()
{
    CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_SEND);
    if(pEdit)
    {
        CString ctr;
        pEdit->GetWindowText(ctr);

        unsigned int nSize= 1024*1024;
        char* pData = ctr.GetBuffer(0);
        if (NULL == m_pRecvBuffer)
        {
            m_pRecvBuffer = (unsigned char*)malloc(1024*1024);
            if (NULL == m_pRecvBuffer)
            {
                return;
            }
            m_nBufferSize = 1024*1024;
        }

        char chData[128] = {0};
        sprintf((char*)chData, "%s\r", pData);
        int nRet = MV_SERIAL_WriteMem(m_pSerialImpl, chData, strlen((char*)chData), (char*)m_pRecvBuffer, m_nBufferSize);

        {
            CListBox* pInfoList = (CListBox*)GetDlgItem(IDC_LIST_InfoBox);
            CString strInfo;
            strInfo.Format(_T("Send:%s, Result:[%s]"), pData, m_pRecvBuffer);
            pInfoList->InsertString(0, strInfo);
        }
        ctr.ReleaseBuffer();
    }
}    
