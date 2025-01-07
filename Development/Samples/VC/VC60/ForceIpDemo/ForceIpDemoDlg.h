// ForceIpDemoDlg.h : header file
#if !defined(AFX_FORCEIPDEMODLG_H__5ACA97CB_4641_4529_89C8_D9F8EF149751__INCLUDED_)
#define AFX_FORCEIPDEMODLG_H__5ACA97CB_4641_4529_89C8_D9F8EF149751__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma comment(lib, "MvSdkExport.lib")

#include <stdio.h>
#include <WINSOCK2.H>
#include "MvSdkExport.h"

// CForceIpDemoDlg dialog
class CForceIpDemoDlg : public CDialog
{
// construction
public:
	CForceIpDemoDlg(CWnd* pParent = NULL);	// standard construction function

// dialog data
	enum { IDD = IDD_FORCEIPDEMO_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV supported

// implementation
protected:
	HICON m_hIcon;

	// generated message mapping function
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
	CComboBox m_ctrlDeviceCombo;
    int m_nDeviceCombo;
	DWORD m_dwIpaddress;
    DWORD                   m_dwNetworkMask;
    DWORD                   m_dwDefaultGateway;
	
    MV_CC_DEVICE_INFO_LIST m_stDevList;
    void *                 m_handle;

public:
    afx_msg void OnBnClickedEnumDeviceButton();
    afx_msg void OnBnClickedSetIpButton();
    afx_msg void OnCbnSelchangeDeviceCombo();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

    void ShowErrorMsg(CString csMessage, int nErrorNum);
    void DisplayDeviceIp();
};
#endif // !defined(AFX_FORCEIPDEMODLG_H__5ACA97CB_4641_4529_89C8_D9F8EF149751__INCLUDED_)
