
// MultiLightCtrlDemoDlg.h : header file
#pragma once
#include "afxwin.h" 
#include <string.h>
#include "MVFGControl.h"

#define WM_DISPLAY_ERROR            (WM_USER + 1)   // ch:显示错误消息ID | en:Display error message ID

#define TRIGGER_MODE_ON             1           // ch:触发模式开 | en:Trigger mode on
#define TRIGGER_MODE_OFF            0           // ch:触发模式关 | en:Trigger mode off

#define TRIGGER_SOURCE_LINE0                0           // ch:Line0 | en:Line0
#define TRIGGER_SOURCE_LINE1                1           // ch:Line1 | en:Line1
#define TRIGGER_SOURCE_LINE2                2           // ch:Line2 | en:Line2
#define TRIGGER_SOURCE_LINE3                3           // ch:Line3 | en:Line3
#define TRIGGER_SOURCE_COUNTER0             4           // ch:Conuter0 | en:Conuter0
#define TRIGGER_SOURCE_SOFTWARE             7           // ch:软触发 | en:Software
#define TRIGGER_SOURCE_FrequencyConverter   8           // ch:变频器 | en:Frequency Converter

#define FILE_NAME_LEN           256             // ch:文件名长度 | en:Length of file name

#define MAX_EXPOSURE_NUM 4  // 分时频闪的最大灯数(用户可根据相机具体的节点等方式自定义曝光个数)

enum CAMERA_TYPE
{
    TYPE_FRAME_SCAN = 0,
    TYPE_LINE_SCAN  = 1,
};

// CMultiLightCtrlDemoDlg dialog
class CMultiLightCtrlDemoDlg : public CDialog
{
// Construction
public:
	CMultiLightCtrlDemoDlg(CWnd* pParent = NULL);	// Standard constructor

    // Dialog Data
	enum { IDD = IDD_MultiLightCtrlDemo_DIALOG };

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
    CComboBox           m_ctrlMultiLightControlCombo;         // ch:多灯控制枚举框 | en:Multi Light Control Enumerate Combo 

    BOOL                m_bSoftWareTriggerCheck;

private:
    /*ch:最开始时的窗口初始化 | en:Window initialization*/
    void DisplayWindowInitial();

    void EnableControls(BOOL bIsCameraReady);
    void ShowErrorMsg(CString csMessage, int nErrorNum);

    int SetTriggerMode();                // ch:设置触发模式 | en:Set Trigger Mode
    int GetTriggerMode();
    int SetTriggerSource();              // ch:设置触发源 | en:Set Trigger Source
    int GetTriggerSource();
    int GetMultiLightControl();			 // ch:获取多灯控制 | en:Get Multi Light Control

    int CloseDevice();                   // ch:关闭设备 | en:Close Device

    // ch:去除自定义的像素格式 | en:Remove custom pixel formats
    bool RemoveCustomPixelFormats(MV_FG_PIXEL_TYPE enPixelFormat);

private:
    BOOL                    m_bOpenIF;                            // ch:是否打开采集卡 | en:Whether to open interface
    BOOL                    m_bOpenDevice;                        // ch:是否打开设备 | en:Whether to open device
    BOOL                    m_bStartGrabbing;                     // ch:是否开始抓图 | en:Whether to start grabbing
    int                     m_nTriggerMode;                       // ch:触发模式 | en:Trigger Mode
    int                     m_nTriggerSource;                     // ch:触发源 | en:Trigger Source
    int                     m_nLineNum;                           // ch:线数 | en:Line Number

    IFHANDLE                m_hInterface;               // ch:采集卡句柄 | en:Interface handle
    DEVHANDLE               m_hDevice;                  // ch:设备句柄 | en:Device handle
    STREAMHANDLE            m_hStream;                  // ch:流通道句柄 | en:Stream handle
    HWND                    m_hwndDisplay[MAX_EXPOSURE_NUM]; // ch:显示句柄 | en:Display Handle

    void*                   m_hGrabThread;              // ch:取流线程句柄 | en:Grab thread handle
    BOOL                    m_bThreadState;             // ch:线程状态 | en:Thread status
    MV_FG_RECONSTRUCT_INFO  m_stReconstructInfo;        // ch:图像重组信息 | en:Image reconstruction info

    unsigned int            m_nInterfaceNum;            // ch:采集卡数量 | en:Interface number

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
    afx_msg void OnBnClickedContinusModeRadio();        // ch:连续模式 | en:Continus Mode
    afx_msg void OnBnClickedTriggerModeRadio();         // ch:触发模式 | en:Trigger Mode
    afx_msg void OnBnClickedStartGrabbingButton();      // ch:开始采集 | en:Start Grabbing
    afx_msg void OnBnClickedStopGrabbingButton();       // ch:结束采集 | en:Stop Grabbing
    afx_msg void OnBnClickedSoftwareTriggerCheck();     // ch:软触发 | en:Software Trigger
    afx_msg void OnBnClickedSoftwareOnceButton();       // ch:软触发一次 | en:Software Trigger Execute Once

    afx_msg void OnCbnSelchangeMultiLightControlCombo();// ch:设置多灯控制 | en:Set Multi Light Control

    afx_msg void OnClose();                             // ch:退出 | en:Exit

    // ch:显示错误消息函数 | en:Display error message function
    afx_msg LRESULT OnDisplayError(WPARAM wParam, LPARAM lParam);

    virtual BOOL PreTranslateMessage(MSG* pMsg);
    int GrabThreadProcess();                            // ch:取图线程 | en:Grab thread

	
	
	
    
};
