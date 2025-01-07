
// SeparateForCMLDemoDlg.h : header file
#pragma once
#include "afxwin.h" 
#include <string.h>
#include "MVFGControl.h"

#define WM_DISPLAY_ERROR            (WM_USER + 1)   // ch:显示错误消息ID | en:Display error message ID
#define FILE_NAME_LEN               256             // ch:文件名长度 | en:Length of file name

// CSeparateForCMLDemoDlg dialog
class CSeparateForCMLDemoDlg : public CDialog
{
// Construction
public:
	CSeparateForCMLDemoDlg(CWnd* pParent = NULL);	// Standard constructor

    // Dialog Data
	enum { IDD = IDD_SeparateForCMLDemo_DIALOG };

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
    CComboBox           m_ctrlInterfaceCombo;                 // ch:枚举到的Interface | en:Enumerated Interface
    CComboBox           m_ctrlDeviceCombo;                    // ch:枚举到的设备 | en:Enumerated device
    int                 m_nDeviceCombo;                       // ch:设备框索引 | en:Index of Device Combo

    UINT m_nWidth;
    UINT m_nHeight;
    CString m_strPixelFormat;
    CString m_strPixelSize;
    CString m_strTapGeometry;
    CString m_strClConfiguration;

private:
    /*ch:最开始时的窗口初始化 | en:Window initialization*/
    void DisplayWindowInitial();
    void EnableControls(BOOL bIsCameraReady);
    void ShowErrorMsg(CString csMessage, int nErrorNum);

    // ch:按钮的亮暗使能 | en:Button bright dark enable
    void EnableWindowInitial();
    void EnableWindowWhenOpenNotStart();  // ch:按下打开设备按钮时的按钮颜色 | en:Button color when click Open 
    void EnableWindowWhenStart();         // ch:按下开始采集时的按钮颜色 | en:Button color when click Start
    void EnableWindowWhenStop();          // ch:按下结束采集时的按钮颜色 | en:Button color when click Stop

    int CloseDevice();                   // ch:关闭设备 | en:Close Device

    // ch:去除自定义的像素格式 | en:Remove custom pixel formats
    bool RemoveCustomPixelFormats(MV_FG_PIXEL_TYPE enPixelFormat);

    // ch:获取设备ID | en:Get Device ID
    int GetDeviceID(unsigned int nIndex);

    // ch:初始化界面上的采集卡参数 | en:Init interface param
    int InitInterfaceParam();

private:
    BOOL                    m_bOpenIF;                            // ch:是否打开采集卡 | en:Whether to open interface
    BOOL                    m_bOpenDevice;                        // ch:是否打开设备 | en:Whether to open device
    BOOL                    m_bStartGrabbing;                     // ch:是否开始抓图 | en:Whether to start grabbing

    IFHANDLE                m_hInterface;               // ch:采集卡句柄 | en:Interface handle
    DEVHANDLE               m_hDevice;                  // ch:设备句柄 | en:Device handle
    STREAMHANDLE            m_hStream;                  // ch:流通道句柄 | en:Stream handle
    HWND                    m_hwndDisplay;              // ch:显示句柄 | en:Display Handle

    void*                   m_hGrabThread;              // ch:取流线程句柄 | en:Grab thread handle
    BOOL                    m_bThreadState;             // ch:线程状态 | en:Thread status

    unsigned int            m_nInterfaceNum;            // ch:采集卡数量 | en:Interface number
    unsigned int            m_nDeviceIndex;             // ch:设备索引 | en:Device Index

    unsigned char*          m_pDataBuf;                 // ch:数据缓存 | en:Data buffer
    unsigned int            m_nDataBufSize;             // ch:数据缓存大小 | en:Length of data buffer

    unsigned char           m_chDeviceID[MV_FG_MAX_DEV_INFO_SIZE];  // 设备ID

public:
    /*ch:采集卡 | en:Interface*/
    afx_msg void OnBnClickedEnumIfButton();             // ch:枚举Interface | en:Enum Interface
    afx_msg void OnBnClickedOpenIfButton();             // ch:打开Interface | en:Open Interface
    afx_msg void OnBnClickedCloseIfButton();            // ch:关闭Interface | en:Close Interface

    /*ch:设备 | en:Device*/
    afx_msg void OnBnClickedEnumDevButton();            // ch:枚举设备 | en:Enum Device
    afx_msg void OnBnClickedOpenButton();               // ch:打开设备 | en:Open Devices
    afx_msg void OnBnClickedCloseButton();              // ch:关闭设备 | en:Close Devices
   
    /*ch:图像采集 | en:Image Acquisition*/
    afx_msg void OnBnClickedStartGrabbingButton();      // ch:开始采集 | en:Start Grabbing
    afx_msg void OnBnClickedStopGrabbingButton();       // ch:结束采集 | en:Stop Grabbing

    afx_msg void OnClose();                             // ch:退出 | en:Exit

    // ch:显示错误消息函数 | en:Display error message function
    afx_msg LRESULT OnDisplayError(WPARAM wParam, LPARAM lParam);

    virtual BOOL PreTranslateMessage(MSG* pMsg);
    int GrabThreadProcess();                            // ch:取图线程 | en:Grab thread
};
