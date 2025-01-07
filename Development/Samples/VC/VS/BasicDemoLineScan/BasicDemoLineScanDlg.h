
// BasicDemoDlg.h : header file
#pragma once
#include "afxwin.h" 
#include "MvCamera.h"
#include <map>
using namespace std;

#define MV_TRIGGER_SOURCE_EncoderModuleOut 6

#define STR_SOFTWARE "Software"
#define STR_FRAMEBURSTSTART "FrameBurstStart"

//模拟增益
typedef enum _MV_PREAMP_GAIN_
{
    GAIN_1000x = 1000,
    GAIN_1400x = 1400,
    GAIN_1600x = 1600,
    GAIN_2000x = 2000,
    GAIN_2400x = 2400,
    GAIN_4000x = 4000,
    GAIN_3200x = 3200,

}MV_PREAMP_GAIN;

//ImageCompressionMode模式
typedef enum _MV_IMAGE_COMPRESSION_MODE_
{
    IMAGE_COMPRESSION_MODE_OFF = 0,
    IMAGE_COMPRESSION_MODE_HB = 2,
}MV_IMAGE_COMPRESSION_MODE;

//触发选项
typedef enum _MV_CAM_TRIGGER_OPTION_
{
    FRAMEBURSTSTART = 6,
    LINESTART = 9,
}MV_CAM_TRIGGER_OPTION;

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

/*ch:控件对应变量 | en:Control corresponding variable*/
private:
    BOOL                    m_bTriggerModeCheck;
    BOOL                    m_bHBMode;
    BOOL                    m_bAcquisitionLineRate;
    BOOL                    m_bPreampGain;
    double                  m_dExposureEdit;
    double                  m_dDigitalShiftGainEdit;
    int                     m_nAcquisitionLineRateEdit;
    int                     m_nResultingLineRateEdit;

    CComboBox               m_ctrlDeviceCombo;                      // ch:枚举到的设备 | en:Enumerated device
    int                     m_nDeviceCombo;
    CComboBox               m_ctrlTriggerSelectorCombo;             // ch:触发选项 | en:Trigger Selector
    int                     m_nTriggerSelector;
    CComboBox               m_ctrlTriggerModeCombo;                 // ch:触发开关 | en:Trigger Mode

    CComboBox               m_ctrlTriggerSourceCombo;               // ch:触发源 | en:Trigger Source
    int                     m_nTriggerSource;                       
    CComboBox               m_ctrlPixelFormatCombo;                 // ch:像素格式 | en:Pixel Format
    int                     m_nPixelFormat;                        
    CComboBox               m_ctrlPreampGainCombo;                  // ch:模拟增益 | en:PreampGain
    int                     m_nPreampGain;
    CComboBox               m_ctrlAcquisitionLineRateEnableCombo;   // ch:模拟增益 | en:AcquisitionLineRate
    CComboBox               m_ctrlImageCompressionModeCombo;        // ch:无损压缩 | en:Image Compression Mode
    int                     m_nImageCompressionMode;                

    map<CString, int> m_mapPixelFormat;
    map<CString, int> m_mapPreampGain;
	map<CString, int> m_mapTriggerSource;

private:
    /*ch:最开始时的窗口初始化 | en:Window initialization*/
    void DisplayWindowInitial();

    void EnableControls(BOOL bIsCameraReady);
    void ShowErrorMsg(CString csMessage, int nErrorNum);

    uint64_t GetPixelSize(MvGvspPixelType enType, UINT16 nWidth, UINT16 nHeight);

    int GetImageCompressionMode();
    int GetTriggerSelector();
    int GetTriggerSwitch();
    int GetTriggerMode();
    int GetExposureTime();                                      // ch:获取曝光时间 | en:Get Exposure Time
    int SetExposureTime();                                      // ch:设置曝光时间 | en:Set Exposure Time
    int GetDigitalShiftGain();                                  // ch:获取数字增益 | en:Get Gain
    int SetDigitalShiftGain();                                  // ch:设置数字增益 | en:Set Gain
    int GetPreampGain();                                        // ch:获取模拟增益 | en:Get PreampGain
    int GetTriggerSource();                                     // ch:获取触发源 | en:Get Trigger Source
    int GetPixelFormat();                                       // ch:获取像素格式 | en:Get Pixel Format
    int GetAcquisitionLineRate();                               // ch:获取实际行频值 | en:Get Acquisition LineRate
    int SetAcquisitionLineRate();                               // ch:设置行频   | en:set Acquisition LineRate
    int GetAcquisitionLineRateEnable();                         // ch:获取行频使能开关 | en:Get Acquisition LineRate Enable
    int GetResultingLineRate();                                 // ch:获取实际行频 | en:Get Resulting LineRate

    int CloseDevice();                                          // ch:关闭设备 | en:Close Device
    int SaveImage(MV_SAVE_IAMGE_TYPE enSaveImageType);          // ch:保存图片 | en:Save Image

	//字符转换函数
	bool IsStrUTF8(const char* pBuffer, int size);
	bool Char2Wchar(const char *pStr, wchar_t *pOutWStr, int nOutStrSize);
	bool Wchar2char(wchar_t *pOutWStr, char *pStr);

private:
    BOOL                    m_bOpenDevice;                      // ch:是否打开设备 | en:Whether to open device
    BOOL                    m_bStartGrabbing;                   // ch:是否开始抓图 | en:Whether to start grabbing
    int                     m_nTriggerMode;                     // ch:触发模式 | en:Trigger Mode

    CMvCamera*              m_pcMyCamera;                       // ch:CMyCamera封装了常用接口 | en:CMyCamera packed commonly used interface
    HWND                    m_hwndDisplay;                      // ch:显示句柄 | en:Display Handle
    MV_CC_DEVICE_INFO_LIST  m_stDevList;         

    CRITICAL_SECTION        m_hSaveImageMux;
    unsigned char*          m_pSaveImageBuf;
    char*                   m_pHBDecodeBuf;
    uint64_t                m_nHBDecodeBufSize;
    unsigned int            m_nSaveImageBufSize;
    MV_FRAME_OUT_INFO_EX    m_stImageInfo;

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
    afx_msg void OnBnClickedSoftwareOnceButton();       // ch:软触发一次 | en:Software Trigger Execute Once
  
    /*ch:图像保存 | en:Image Save*/
    afx_msg void OnBnClickedSaveBmpButton();            // ch:保存bmp | en:Save bmp
    afx_msg void OnBnClickedSaveJpgButton();            // ch:保存jpg | en:Save jpg
    afx_msg void OnBnClickedSaveTiffButton();
    afx_msg void OnBnClickedSavePngButton();
  
    /*ch:参数设置获取 | en:Parameters Get and Set*/
    afx_msg void OnBnClickedGetParameterButton();       // ch:获取参数 | en:Get Parameter
    afx_msg void OnBnClickedSetParameterButton();       // ch:设置参数 | en:Exit from upper right corner

    afx_msg void OnClose();

    virtual BOOL PreTranslateMessage(MSG* pMsg);
    int GrabThreadProcess();
    afx_msg void OnCbnSelchangeTriggerselCombo();
    afx_msg void OnCbnSelchangeTriggerselCombo2();
    afx_msg void OnCbnSelchangeTriggerswitchCombo();
    afx_msg void OnCbnSelchangeTriggersourceCombo();
    afx_msg void OnCbnSelchangePixelformatCombo();
    afx_msg void OnCbnSelchangePreampgainCombo();
    afx_msg void OnCbnSelchangeImageCompressionModeCombo();
    afx_msg void OnBnClickedAcquisitionLineRateEnableCheck();
    afx_msg void OnEnChangeAcquisitionLineRateEdit();
};
