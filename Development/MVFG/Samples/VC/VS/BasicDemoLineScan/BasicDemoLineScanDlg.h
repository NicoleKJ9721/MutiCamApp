
// BasicDemoLineScanDlg.h : header file
#pragma once
#include "afxwin.h" 
#include <string.h>
#include "MVFGControl.h"

#define WM_DISPLAY_ERROR            (WM_USER + 1)   // ch:显示错误消息ID | en:Display error message ID

#define FILE_NAME_LEN           256             // ch:文件名长度 | en:Length of file name

enum SAVE_IAMGE_TYPE
{
    Image_Undefined                  = 0,                        ///< \~chinese 未定义的图像格式             \~english Undefined Image Type
    Image_Bmp                        = 1,                        ///< \~chinese BMP图像格式                  \~english BMP Image Type
    Image_Jpeg                       = 2,                        ///< \~chinese JPEG图像格式                 \~english Jpeg Image Type
	Image_Tiff                       = 3,                        ///< \~chinese Tiff图像格式                 \~english Tiff Image Type
	Image_Png                        = 4,                        ///< \~chinese Png图像格式                  \~english Png Image Type

};

// CBasicDemoLineScanDlg dialog
class CBasicDemoLineScanDlg : public CDialog
{
// Construction
public:
	CBasicDemoLineScanDlg(CWnd* pParent = NULL);	// Standard constructor

    // Dialog Data
	enum { IDD = IDD_BasicDemoLineScan_DIALOG };

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

private:
    /*ch:最开始时的窗口初始化 | en:Window initialization*/
    void DisplayWindowInitial();

	void EnableControls(BOOL bIsCameraReady);
	void EnableIFParamsControls();
	void EnableDevParamsControls();
    void ShowErrorMsg(CString csMessage, int nErrorNum);

	int SetImageCompressionMode();                              // ch:设置图像压缩模式 | en:Set Image Compression Mode
	int GetInterfaceParams();									// ch:获取采集卡参数 | en:Get interface parameters
	int GetDeviceParams();										// ch:获取相机参数 | en:Get device parameters

    int CloseDevice();											// ch:关闭设备 | en:Close Device

	bool RemoveCustomPixelFormats(MV_FG_PIXEL_TYPE enPixelFormat);
	int SaveImage(SAVE_IAMGE_TYPE enSaveImageType);         // ch:保存图片 | en:Save image

    // ch:去除自定义的像素格式 | en:Remove custom pixel formats


private:
    BOOL                    m_bHBMode;                          // ch:是否获取图像压缩模式 | en:Whether to get Image Compress Mode

    BOOL                    m_bOpenIF;                            // ch:是否打开采集卡 | en:Whether to open interface
    BOOL                    m_bOpenDevice;                        // ch:是否打开设备 | en:Whether to open device
    BOOL                    m_bStartGrabbing;                     // ch:是否开始抓图 | en:Whether to start grabbing
    BOOL                    m_bLineInputPolarity;                 // ch:是否拥有线路输入极性节点 | en:Whether to have LineInputPolarity node
    BOOL                    m_bCCSelector;                        // ch:是否拥有相机控制选择节点 | en:Whether to have CCSelector node
    BOOL                    m_bCCSource;                          // ch:是否拥有相机控制源节点 | en:Whether to have CCSource node
    BOOL                    m_bScanMode;                          // ch:是否拥有扫描模式节点 | en:Whether to have ScanMode node
    BOOL                    m_bTriggerActivation;                 // ch:是否拥有触发极性节点 | en:Whether to have TriggerActivation node

    IFHANDLE                m_hInterface;               // ch:采集卡句柄 | en:Interface handle
    DEVHANDLE               m_hDevice;                  // ch:设备句柄 | en:Device handle
    STREAMHANDLE            m_hStream;                  // ch:流通道句柄 | en:Stream handle
    HWND                    m_hwndDisplay;              // ch:显示句柄 | en:Display Handle

    void*                   m_hGrabThread;              // ch:取流线程句柄 | en:Grab thread handle
    BOOL                    m_bThreadState;             // ch:线程状态 | en:Thread status

    unsigned int            m_nInterfaceNum;            // ch:采集卡数量 | en:Interface number

    CRITICAL_SECTION        m_hSaveImageMux;            // ch:存图锁 | en:Mutex for saving image
    unsigned char*          m_pDataBuf;                 // ch:数据缓存 | en:Data buffer
    unsigned int            m_nDataBufSize;             // ch:数据缓存大小 | en:Length of data buffer
    unsigned char*          m_pSaveImageBuf;            // ch:图像缓存 | en:Image buffer
    unsigned int            m_nSaveImageBufSize;        // ch:图像缓存大小 | en:Length of image buffer
    MV_FG_INPUT_IMAGE_INFO  m_stImageInfo;              // ch:图像信息 | en:Image info

    CComboBox m_ctrlImageCompressionModeCombo;
	CComboBox m_ctrlCameraTypeCombo;
	int m_nImageHeightEdit;
	CComboBox m_ctrlLineSelectorCombo;
	CComboBox m_ctrlLineModeCombo;
	CComboBox m_ctrlLineInputPolarityCombo;
	CComboBox m_ctrlEncoderSelectorCombo;
	CComboBox m_ctrlEncoderSourceACombo;
	CComboBox m_ctrlEncoderSourceBCombo;
	CComboBox m_ctrlEncoderTriggerModeCombo;
	CComboBox m_ctrlCCSelectorCombo;
	CComboBox m_ctrlCCSourceCombo;

	// ch:相机参数 | en:device params
	int m_nCameraWidth;
	int m_nCameraHeight;
	CComboBox m_ctrlPixelFormatCombo;
	CComboBox m_ctrlScanModeCombo;
	CComboBox m_ctrlTriggerSelectorCombo;
	CComboBox m_ctrlTriggerModeCombo;
	CComboBox m_ctrlTriggerSourceCombo;
	CComboBox m_ctrlTriggerActivationCombo;

    MV_FG_ENUMVALUE         m_stEnumImageCompressionModeValue;

public:
    /*ch:采集卡 | en:Interface*/
    afx_msg void OnBnClickedEnumIfButton();             // ch:枚举Interface | en:Enum Interface
    afx_msg void OnBnClickedOpenIfButton();             // ch:打开Interface | en:Open Interface
    afx_msg void OnBnClickedCloseIfButton();            // ch:关闭Interface | en:Close Interface

    /*ch:设备 | en:Device*/
    afx_msg void OnBnClickedEnumDevButton();            // ch:枚举设备 | en:Enum Device
    afx_msg void OnBnClickedOpenDevButton();               // ch:打开设备 | en:Open Devices
    afx_msg void OnBnClickedCloseDevButton();              // ch:关闭设备 | en:Close Devices
   
    /*ch:图像采集 | en:Image Acquisition*/
    afx_msg void OnBnClickedStartGrabbingButton();      // ch:开始采集 | en:Start Grabbing
    afx_msg void OnBnClickedStopGrabbingButton();       // ch:结束采集 | en:Stop Grabbing
    afx_msg void OnBnClickedSoftwareOnceButton();       // ch:软触发一次 | en:Software Trigger Execute Once

    /*ch:图像保存 | en:Image Save*/
    afx_msg void OnBnClickedBmpSaveButton();            // ch:保存为BMP | en:Save bmp
    afx_msg void OnBnClickedJpegSaveButton();           // ch:保存为JPEG | en:Save jpeg
	afx_msg void OnBnClickedTiffSaveButton();           // ch:保存为TIFF | en:Save tiff
    afx_msg void OnBnClickedPngSaveButton();            // ch:保存为PNG | en:Save png

    afx_msg void OnClose();                             // ch:退出 | en:Exit

    // ch:显示错误消息函数 | en:Display error message function
    afx_msg LRESULT OnDisplayError(WPARAM wParam, LPARAM lParam);

    virtual BOOL PreTranslateMessage(MSG* pMsg);
    int GrabThreadProcess();                            // ch:取图线程 | en:Grab thread

	afx_msg void OnCbnSelchangeCameraTypeCombo();
	afx_msg void OnEnKillfocusImageHeightEdit();
	afx_msg void OnCbnSelchangeLineSelectorCombo();
	afx_msg void OnCbnSelchangeLineModeCombo();
	afx_msg void OnCbnSelchangeLineInputPolarityCombo();
	afx_msg void OnCbnSelchangeEncoderSelectorCombo();
	afx_msg void OnCbnSelchangeEncoderSourceACombo();
	afx_msg void OnCbnSelchangeEncoderSourceBCombo();
	afx_msg void OnCbnSelchangeEncoderTriggerModeCombo();
	afx_msg void OnCbnSelchangeCcSelectorCombo();
	afx_msg void OnCbnSelchangeCcSourceCombo();
	afx_msg void OnEnKillfocusWidthEdit();
	afx_msg void OnEnKillfocusHeightEdit();
	afx_msg void OnCbnSelchangePixelformatCombo();
	afx_msg void OnCbnSelchangeTriggerSelectorCombo();
	afx_msg void OnCbnSelchangeTriggerModeComb();
	afx_msg void OnCbnSelchangeTriggerSourceCombo();
	afx_msg void OnCbnSelchangeTriggerActivationCombo();
	afx_msg void OnCbnSelchangeScanModeCombo();
};
