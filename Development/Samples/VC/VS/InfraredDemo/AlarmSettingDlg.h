
// InfraredDemoDlg.h : header file
#pragma once
#include "afxwin.h" 
#include "resource.h"
#include "MvCamera.h"

// CInfraredDemoDlg dialog
class CAlarmSettingDlg : public CDialog
{
// Construction
public:
	CAlarmSettingDlg(CMvCamera *pcMyCamera, const CString &strRegion, int nRegionIndex, CWnd* pParent = NULL);	// Standard constructor

    // Dialog Data
	enum { IDD = IDD_ALARM_DIALOG };

	enum RegionType
	{
		Region_Point = 0,
		Region_Polygon,
		Region_Line,
		Region_Circle,
	};

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

/*ch:控件对应变量 | en:Control corresponding variable*/
private:
	CMvCamera*              m_pcMyCamera;               // ch:CMyCamera封装了常用接口 | en:CMyCamera packed commonly used interface
	CString                 m_strRegion;
	int                     m_nRegionIndex;
	float                   m_fReferenceValue;
	float                   m_fABSValue;

private:
	CButton					m_ctrlAlarmEnableCheck;
	CComboBox				m_ctrlSourceCombo;
	CComboBox				m_ctrlConditionCombo;


private:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void InitParameter();
	int ReadEnumIntoCombo(const char* strKey, CComboBox &ctrlComboBox);
	int SetEnumIntoDevice(const char* strKey, CComboBox &ctrlComboBox);
	void ShowErrorMsg(CString csMessage, int nErrorNum);

protected:
	virtual void OnOK();

};
