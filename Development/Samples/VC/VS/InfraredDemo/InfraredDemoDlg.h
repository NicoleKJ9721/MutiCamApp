
// InfraredDemoDlg.h : header file
#pragma once
#include "afxwin.h" 
#include "MvCamera.h"
#include "InfraredDefines.h"

// CInfraredDemoDlg dialog
class CInfraredDemoDlg : public CDialog
{
// Construction
public:
	CInfraredDemoDlg(CWnd* pParent = NULL);	// Standard constructor

    // Dialog Data
	enum { IDD = IDD_InfraredDemo_DIALOG };

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
	float                   m_dEmissivity;
	float                   m_dTargetDistance;
	int64_t                 m_nTransmissivity;

    CComboBox               m_ctrlDeviceCombo;                // ch:枚举到的设备 | en:Enumerated device
	CComboBox               m_ctrlPixelCombo;                 // ch:像素格式 | en:Pixel Format
	CComboBox               m_ctrlDisplaySourceCombo;         // ch:显示来源| en:Display Source
	CComboBox               m_ctrlRegionSelectCombo;          // ch:区域选择 | en:Region Select
	CComboBox               m_ctrlMeasureRangeCombo;          // ch:测量范围 | en:Measure Range
	CComboBox               m_ctrlPaletteModeCombo;           // ch:伪彩模式 | en:Palettes Mode
	CButton                 m_ctrlLegendCheckBox;
	CButton                 m_ctrlExportModeCheckBox;

    int                     m_nDeviceCombo;

private:
    /*ch:最开始时的窗口初始化 | en:Window initialization*/
    void DisplayWindowInitial();

    void EnableControls(BOOL bIsCameraReady);
    void ShowErrorMsg(CString csMessage, int nErrorNum);

    int SetTriggerMode();                // ch:设置触发模式 | en:Set Trigger Mode

    int CloseDevice();                   // ch:关闭设备 | en:Close Device
    int SaveImage(MV_SAVE_IAMGE_TYPE enSaveImageType);                     // ch:保存图片 | en:Save Image

	int ReadEnumIntoCombo(const char* strKey, CComboBox &ctrlComboBox);
	int SetEnumIntoDevice(const char* strKey, CComboBox &ctrlComboBox);

	void DrawShapeInImg(const IFR_POLYGON & oPolygon, int nRegionType, bool bAlarm, int nWidth, int nHeight);

private:
    BOOL                    m_bOpenDevice;                        // ch:是否打开设备 | en:Whether to open device
    BOOL                    m_bStartGrabbing;                     // ch:是否开始抓图 | en:Whether to start grabbing

    CMvCamera*              m_pcMyCamera;               // ch:CMyCamera封装了常用接口 | en:CMyCamera packed commonly used interface
    HWND                    m_hwndDisplay;                        // ch:显示句柄 | en:Display Handle
    MV_CC_DEVICE_INFO_LIST  m_stDevList;         

    CRITICAL_SECTION        m_hSaveImageMux;
    unsigned char*          m_pSaveImageBuf;
    unsigned int            m_nSaveImageBufSize;
    MV_FRAME_OUT_INFO_EX    m_stImageInfo;

    void*                   m_hGrabThread;              // ch:取流线程句柄 | en:Grab thread handle
    BOOL                    m_bThreadState;

public:
    /*ch:初始化 | en:Initialization*/
    afx_msg void OnBnClickedEnumButton();               // ch:查找设备 | en:Find Devices
    afx_msg void OnBnClickedOpenButton();               // ch:打开设备 | en:Open Devices
    afx_msg void OnBnClickedCloseButton();              // ch:关闭设备 | en:Close Devices
    afx_msg void OnBnClickedStartGrabbingButton();      // ch:开始采集 | en:Start Grabbing
    afx_msg void OnBnClickedStopGrabbingButton();       // ch:结束采集 | en:Stop Grabbing
  

  
    /*ch:参数设置获取 | en:Parameters Get and Set*/
    afx_msg void OnBnClickedGetParameterButton();       // ch:获取参数 | en:Get Parameter
    afx_msg void OnBnClickedSetParameterButton();       // ch:设置参数 | en:Exit from upper right corner
	afx_msg void OnBnClickedRegionSetting();            // ch:区域设置 | en:Region Setting
	afx_msg void OnBnClickedAlarmSetting();           // ch:告警设置 | en:Warning Setting


	afx_msg void OnPixelFormatChanged();                 // ch:像素格式改变 | en:Pixel Format Changed
	afx_msg void OnDisplaySourceChanged();               // ch:显示来源改变 | en:Display Source Changed
	afx_msg void OnRegionSelectChanged();                // ch:区域选择改变 | en:Region Select Changed
	afx_msg void OnPaletteModeChanged();                 // ch:伪彩模式改变 | en:Palette Mode Changed
	afx_msg void OnLegendVisibleChanged();               // ch:温度条显隐性改变 | en:Legend Visible Changed
	afx_msg void OnExportModeChanged();                  // ch:测温专家模式改变 | en:Export Mode Changed


    afx_msg void OnClose();

    virtual BOOL PreTranslateMessage(MSG* pMsg);
    int GrabThreadProcess();
};
