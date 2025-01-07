
// BasicDemoDlg.h : header file
#pragma once
#include "afxwin.h" 
#include "MvCamera.h"

// CBasicDemoDlg dialog
class CBasicDemoDlg : public CDialog
{
// Construction
public:
	CBasicDemoDlg(CWnd* pParent = NULL);	// Standard constructor

// Dialog Data
	enum { IDD = IDD_BasicDemo_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

// ch:内部函数 | en:Built-in function
private:
    // ch:最开始时的窗口初始化 | en:Window initialization
    void DisplayWindowInitial();          // ch:显示框初始化,最开始的初始化 | en:Display window initialization
    void ShowErrorMsg(CString csMessage, int nErrorNum);
    // ch:按钮的亮暗使能 | en:Button bright dark enable
    void EnableWindowInitial();
    void EnableWindowWhenClose();         // ch:关闭时的按钮颜色，相当于最开始初始化 | en:Button color when Close, equivalent to the initialization
    void EnableWindowWhenOpenNotStart();  // ch:按下打开设备按钮时的按钮颜色 | en:Button color when click Open 
    void EnableWindowWhenStart();         // ch:按下开始采集时的按钮颜色 | en:Button color when click Start
    void EnableWindowWhenStop();          // ch:按下结束采集时的按钮颜色 | en:Button color when click Stop

    int CloseDevice(void);                   // ch:关闭设备 | en:Close Device
    static void __stdcall ReconnectDevice(unsigned int nMsgType, void* pUser);

private:
    CButton                 m_ctrlOpenButton;                   // ch:打开设备 | en:Open device
    CButton                 m_ctrlCloseButton;                  // ch:关闭设备 | en:Close device
    CButton                 m_ctrlStartGrabbingButton;          // ch:开始抓图 | en:Start grabbing
    CButton                 m_ctrlStopGrabbingButton;           // ch:结束抓图 | en:Stop grabbing
    CComboBox               m_ctrlDeviceCombo;                  // ch:枚举到的设备 | en:Enumerated device
    int                     m_nDeviceCombo;

private:
    BOOL                    m_bOpenDevice;                        // ch:是否打开设备 | en:Whether to open device
    BOOL                    m_bStartGrabbing;                     // ch:是否开始抓图 | en:Whether to start grabbing
    CMvCamera*              m_pcMyCamera;                       // ch:CMyCamera封装了常用接口 | en:CMyCamera packed commonly used interface
    HWND                    m_hwndDisplay;                        // ch:显示句柄 | en:Display Handle
    MV_CC_DEVICE_INFO_LIST  m_stDevList;
    MV_CC_DEVICE_INFO       m_stDevInfo;

    void*                   m_hGrabThread;              // ch:取流线程句柄 | en:Grab thread handle
    BOOL                    m_bThreadState;

public:
    afx_msg void OnBnClickedEnumButton();               // ch:查找设备 | en:Find Devices
    afx_msg void OnBnClickedOpenButton();               // ch:打开设备 | en:Open Devices
    afx_msg void OnBnClickedCloseButton();              // ch:关闭设备 | en:Close Devices
    afx_msg void OnBnClickedStartGrabbingButton();      // ch:开始采集 | en:Start Grabbing
    afx_msg void OnBnClickedStopGrabbingButton();       // ch:结束采集 | en:Stop Grabbing
    afx_msg void OnClose();                             // ch:右上角退出 | en:Exit from upper right corner
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    int GrabThreadProcess();
};
