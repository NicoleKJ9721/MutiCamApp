
// BasicDemoDlg.h : header file
#pragma once
#include "afxwin.h" 
#include "MvCamera.h"
#include <map>
using namespace std;

#define EXPOSURE_NUM 4  // 分时频闪的灯数(用户可根据相机具体的节点等方式自定义曝光个数)

// CReconstructImageDemoDlg dialog
class CReconstructImageDemoDlg : public CDialog
{
// Construction
public:
	CReconstructImageDemoDlg(CWnd* pParent = NULL);	// Standard constructor

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

/*ch:控件对应变量 | en:Control corresponding variable*/
private:
    CComboBox               m_ctrlDeviceCombo;                // ch:枚举到的设备 | en:Enumerated device
    int                     m_nDeviceCombo;

	CComboBox               m_ctrlMultiLightCombo;            // ch:多灯控制 | en:MultiLight
	int                     m_nMultiLightCombo;

    BOOL                    m_bSoftWareTriggerCheck;

	int						m_nMultiLightNum;

	map<CString, int>		m_mapMultiLight;

private:
    /*ch:最开始时的窗口初始化 | en:Window initialization*/
    void DisplayWindowInitial();

    void EnableControls(BOOL bIsCameraReady);
    void ShowErrorMsg(CString csMessage, int nErrorNum);

    int SetTriggerMode();                // ch:设置触发模式 | en:Set Trigger Mode
    int GetTriggerMode();

    int GetTriggerSource();              // ch:设置触发源 | en:Set Trigger Source
    int SetTriggerSource();

	int GetMultiLight();				 //ch: 获取多灯参数 | en:Get MultiLight

    int CloseDevice();                   // ch:关闭设备 | en:Close Device

	//字符转换函数
	bool IsStrUTF8(const char* pBuffer, int size);
	bool Char2Wchar(const char *pStr, wchar_t *pOutWStr, int nOutStrSize);
	bool Wchar2char(wchar_t *pOutWStr, char *pStr);

private:
    BOOL                    m_bOpenDevice;                        // ch:是否打开设备 | en:Whether to open device
    BOOL                    m_bStartGrabbing;                     // ch:是否开始抓图 | en:Whether to start grabbing
    int                     m_nTriggerMode;                       // ch:触发模式 | en:Trigger Mode
    int                     m_nTriggerSource;                     // ch:触发源 | en:Trigger Source

    CMvCamera*              m_pcMyCamera;						  // ch:CMyCamera封装了常用接口 | en:CMyCamera packed commonly used interface
    HWND                    m_hwndDisplay[EXPOSURE_NUM];          // ch:显示句柄 | en:Display Handle
	MV_CC_DEVICE_INFO_LIST  m_stDevList;						  // ch:设备列表 | en: Device List
    void*                   m_hGrabThread;						  // ch:取流线程句柄 | en:Grab thread handle
    BOOL                    m_bThreadState;

public:
    /*ch:初始化 | en:Initialization*/
    afx_msg void OnBnClickedEnumDevButton();               // ch:查找设备 | en:Find Devices
    afx_msg void OnBnClickedOpenButton();               // ch:打开设备 | en:Open Devices
    afx_msg void OnBnClickedCloseButton();              // ch:关闭设备 | en:Close Devices
   
    /*ch:图像采集 | en:Image Acquisition*/
    afx_msg void OnBnClickedContinusModeRadio();        // ch:连续模式 | en:Continus Mode
    afx_msg void OnBnClickedTriggerModeRadio();         // ch:触发模式 | en:Trigger Mode
    afx_msg void OnBnClickedStartGrabbingButton();      // ch:开始采集 | en:Start Grabbing
    afx_msg void OnBnClickedStopGrabbingButton();       // ch:结束采集 | en:Stop Grabbing
    afx_msg void OnBnClickedSoftwareTriggerCheck();     // ch:软触发 | en:Software Trigger
    afx_msg void OnBnClickedSoftwareOnceButton();       // ch:软触发一次 | en:Software Trigger Execute Once
  
    afx_msg void OnClose();

    virtual BOOL PreTranslateMessage(MSG* pMsg);
    int GrabThreadProcess();
	afx_msg void OnCbnSelchangeMultilightCombo();
};
