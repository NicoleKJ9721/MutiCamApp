
// MultipleCameraDlg.h : header file

#pragma once
#include "afxwin.h"
#include <stdio.h>
#include "MVFGControl.h"
#include <list>

#define MAX_DEVICE_NUM          4        // ch:每张采集卡的最大设备个数 | en:Maximum number of devices on each card
#define MAX_STREAM_NUM          4        // ch:每个相机的最大流通道个数 | en:Maximum number of stream channels per camera
#define BUFFER_NUMBER           3        // ch:申请的缓存个数 | en:Number of requested buffer
#define TIMEOUT                 1000     // ch:超时时间，单位毫秒 | en:Timeout, unit ms
#define DEVICE_INFO_LEN         256      // ch:设备信息长度 | en:Device information length
#define MAX_LOGINFO_LEN         1024     // ch:日志信息长度 | en:Log information length
#define MAX_INTERFACE_NUM       2        // ch:采集卡列表个数 | en:Number of acquisition card lists

#define  FIRST_INTERFACE  0
#define  SECOND_INTERFACE 1

using namespace std;

typedef struct _INTERFACE_INFO_
{
    IFHANDLE          hInterface;                               // ch:采集卡句柄 | en:Interface handle
    DEVHANDLE         hDevice[MAX_DEVICE_NUM];                  // ch:设备句柄 | en:Device handle
    STREAMHANDLE      hStream[MAX_DEVICE_NUM];                  // ch:流通道句柄 | en:Stream handle
    HWND              hwndDisplay[MAX_DEVICE_NUM];              // ch:显示句柄 | en:Display handle
    CRect             hwndRect[MAX_DEVICE_NUM];                 // ch:显示窗口 | en:Display window
    void*             hGrabThread[MAX_DEVICE_NUM];              // ch:取流线程 | en: Grabbing thread
    unsigned int      nCurIndex[MAX_DEVICE_NUM];                // ch:设备的索引值 | en:Index value of the device
}IF_INFO;

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
	DECLARE_MESSAGE_MAP()

private:
    CComboBox               m_ctrlInterfaceCombo[MAX_INTERFACE_NUM];                       // ch:枚举到的Interface | en:Enumerated Interface

    BOOL                    m_bCamCheck[MAX_INTERFACE_NUM][MAX_DEVICE_NUM];
    BOOL                    m_bOpenIF;
    BOOL                    m_bOpenDevice;
    BOOL                    m_bStartGrabbing;
    BOOL                    m_bDeviceGrabbingFlag[MAX_INTERFACE_NUM][MAX_DEVICE_NUM];
    
    int                     m_nTriggerMode;

    unsigned int            m_nInterfaceNum;

    IF_INFO         m_stInterface[MAX_INTERFACE_NUM];                              // ch:采集卡信息 | en:Interface information

    CListBox                m_ctrlListBoxInfo;

public:
    int                     m_nCurCameraIndex;
    int                     m_nCurListIndex;

public:
    afx_msg void OnBnClickedEnumDevicesButton();
    afx_msg void OnBnClickedOpenDevicesButton();
    afx_msg void OnBnClickedCloseDevicesButton();
    afx_msg void OnBnClickedStartGrabbingButton();
    afx_msg void OnBnClickedStopGrabbingButton();
    afx_msg void OnClose();
    afx_msg void OnLbnDblclkOutputInfoList();

    virtual BOOL PreTranslateMessage(MSG* pMsg);

    void ThreadFun(int  nCurListIndex, int nCurCameraIndex);
    void PrintMessage( const char* pszFormat, ... );

    void ShowErrorMsg(CString csMessage, int nErrorNum);
private:
    bool PrintInterfaceInfo(unsigned int nInterfaceNum);
    bool PrintDeviceInfo(IFHANDLE hInterface, unsigned int nDeviceNum);
    void EnableControls(BOOL bIsCameraReady);

public:
    afx_msg void OnBnClickedEnumInterfaceButton();
    afx_msg void OnBnClickedOpenInterfaceButton();
    afx_msg void OnBnClickedCloseInterfaceButton();
    
};
