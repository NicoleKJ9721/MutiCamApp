
// MultipleCameraDlg.h : header file

#pragma once
#include "afxwin.h"
#include <stdio.h>
#include "MvCamera.h"

#define WM_TIMER_GRAB_INFO      1
#define MAX_DEVICE_NUM          9

// CMultipleCameraDlg dialog
class CMultipleCameraDlg : public CDialog
{
// Construction
public:
	CMultipleCameraDlg(CWnd* pParent = NULL);	        // Standard constructor

// Dialog Data
	enum { IDD = IDD_MULTIPLECAMERA_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	HICON           m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
	DECLARE_MESSAGE_MAP()

private:
    MV_CC_DEVICE_INFO_LIST m_stDevList;
    BOOL            m_bCamCheck[MAX_DEVICE_NUM];
    unsigned int    m_nValidCamNum;
    BOOL            m_bOpenDevice;
    BOOL            m_bStartGrabbing;
    int             m_nTriggerMode;
    int             m_nTriggerSource;
    void*           m_hGrabThread[MAX_DEVICE_NUM];

    CRITICAL_SECTION        m_hSaveImageMux[MAX_DEVICE_NUM];
    unsigned char*          m_pSaveImageBuf[MAX_DEVICE_NUM];
    unsigned int            m_nSaveImageBufSize[MAX_DEVICE_NUM];
    MV_FRAME_OUT_INFO_EX    m_stImageInfo[MAX_DEVICE_NUM];

    CListBox        m_ctrlListBoxInfo;
    int             m_nZoomInIndex;                         // 标记哪一个相机被放大

public:
    int             m_nCurCameraIndex;
    CMvCamera*      m_pcMyCamera[MAX_DEVICE_NUM];           // ch:CMyCamera封装了常用接口 | en:CMyCamera packed normal used interface
    HWND            m_hwndDisplay[MAX_DEVICE_NUM];          // ch:显示句柄 | en:Display window
    CRect           m_hwndRect[MAX_DEVICE_NUM];

public:
    afx_msg void OnBnClickedEnumDevicesButton();
    afx_msg void OnBnClickedOpenDevicesButton();
    afx_msg void OnBnClickedCloseDevicesButton();
    afx_msg void OnBnClickedStartGrabbingButton();
    afx_msg void OnBnClickedStopGrabbingButton();
    afx_msg void OnBnClickedContinusModeRadio();
    afx_msg void OnBnClickedTriggerModeRadio();
    afx_msg void OnBnClickedSoftwareModeButton();
    afx_msg void OnBnClickedHardwareModeButton();
    afx_msg void OnBnClickedSoftwareOnceButton();
    afx_msg void OnBnClickedSaveImageButton();
    afx_msg void OnClose();
    afx_msg void OnLbnDblclkOutputInfoList();
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);

    virtual BOOL PreTranslateMessage(MSG* pMsg);

    int ThreadFun(int nCurCameraIndex);

private:
    void EnableControls();
    void PrintMessage( const char* pszFormat, ... );

    void SetTriggerMode(void);
    void SetTriggerSource(void);
    void DoSoftwareOnce(void);

};
