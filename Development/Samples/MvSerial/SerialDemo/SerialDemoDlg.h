
// SerialDemoDlg.h : header file
//
#include "MvSerialCtrl.h"
#include <crtdbg.h>
#include <list>

using namespace std;
#pragma once

#define MV_SERIAL_MAX_DEVICE 8

// CSerialDemoDlg dialog
class CSerialDemoDlg : public CDialog
{
// Construction
public:
	CSerialDemoDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_SERIALDEMO_DIALOG };

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
public:
    afx_msg void OnBnClickedButtonOpen();
    afx_msg void OnBnClickedButtonClose();
    afx_msg void OnBnClickedButtonWrite();
    afx_msg void OnBnClickedButtonRead();
    afx_msg void OnBnClickedButtonClearinfo();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnClose();
    afx_msg void OnBnClickedButtonEnum();
    afx_msg void OnBnClickedButtonSetParam();
    afx_msg void OnBnClickedButtonGetParam();
    afx_msg void OnBnClickedButtonSetup();

    void DisconnectProcess();

private:
    void ResetMember();
    void Deinit();
    void EnableControl(bool bHasEnum);
    BOOL PreTranslateMessage(MSG* pMsg);

private:
    void*   m_pSerialImpl;

    MV_SERIAL_DEVICE_INFO*  m_pstDeviceInfo[MV_SERIAL_MAX_DEVICE];
    unsigned int            m_nDeviceNum;

    //界面参数
    unsigned int m_nWidth;
    unsigned int m_nHeight;
    int          m_nNodeType;

    bool        m_bIsOpen;
    float       m_fFrameRate;

    unsigned char* m_pRecvBuffer;
    unsigned int    m_nBufferSize;
public:
    afx_msg void OnBnClickedButtonSend();
    afx_msg void OnBnClickedButtonSend2();
};
