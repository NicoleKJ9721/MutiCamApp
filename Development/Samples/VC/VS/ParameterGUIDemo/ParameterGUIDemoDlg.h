
// ParameterGUIDemoDlg.h : header file
#pragma once
#include "afxwin.h" 
#include "MvCamera.h"

// CParameterGUIDemoDlg dialog
class CParameterGUIDemoDlg : public CDialog
{
// Construction
public:
	CParameterGUIDemoDlg(CWnd* pParent = NULL);	// Standard constructor

    // Dialog Data
	enum { IDD = IDD_ParameterGUIDemo_DIALOG };

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

/*ch:控件对应变量 | en:Control corresponding variable*/
private:
    CComboBox               m_ctrlDeviceCombo;                // ch:枚举到的设备 | en:Enumerated device
    int                     m_nDeviceCombo;

private:
    void DisplayWindowInitial();                                        // 最开始时的窗口初始化
    void EnableControls(BOOL bIsCameraReady);                           // 控件状态
    void ShowErrorMsg(CString csMessage, int nErrorNum);                // 显示错误信息
    int CloseDevice();                                                  // 关闭设备

private:
    BOOL                    m_bOpenDevice;              // ch:是否打开设备 | en:Whether to open device
    BOOL                    m_bStartGrabbing;           // ch:是否开始抓图 | en:Whether to start grabbing

    CMvCamera*              m_pcMyCamera;               // ch:CMyCamera封装了常用接口 | en:CMyCamera packed commonly used interface
    HWND                    m_hwndDisplay;              // ch:显示句柄 | en:Display Handle
    MV_CC_DEVICE_INFO_LIST  m_stDevList;

    void*                   m_hGrabThread;              // ch:取流线程句柄 | en:Grab thread handle
    BOOL                    m_bThreadState;

public:
    /*ch:初始化 | en:Initialization*/
    afx_msg void OnBnClickedEnumButton();               // ch:查找设备 | en:Find Devices
    afx_msg void OnBnClickedOpenButton();               // ch:打开设备 | en:Open Devices
    afx_msg void OnBnClickedCloseButton();              // ch:关闭设备 | en:Close Devices

    /*ch:图像采集 | en:Image Acquisition*/
    afx_msg void OnBnClickedStartGrabbingButton();      // ch:开始采集 | en:Start Grabbing
    afx_msg void OnBnClickedStopGrabbingButton();       // ch:结束采集 | en:Stop Grabbing

    /*ch:参数配置 | en:Parameter Configuration*/
    afx_msg void OnBnClickedOpenParameterGuiButton();
    afx_msg void OnClose();

    virtual BOOL PreTranslateMessage(MSG* pMsg);
    int GrabThreadProcess();
};
