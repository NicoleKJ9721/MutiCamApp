
// InfraredDemoDlg.h : header file
#pragma once
#include "afxwin.h" 
#include "resource.h"
#include "MvCamera.h"

// CInfraredDemoDlg dialog
class CRegionSettingDlg : public CDialog
{
// Construction
public:
	CRegionSettingDlg(CMvCamera *pcMyCamera, const CString &strRegion, bool bExportMode,CWnd* pParent = NULL);	// Standard constructor

    // Dialog Data
	enum { IDD = IDD_REGIONSET_DIALOG };

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
	RegionType              m_RegionType;
	CString                 m_strRegion;
	float                   m_fReflectance;
	float                   m_fEmissivity;
	float                   m_fTargetDistance;
	int64_t                 m_nPointNum;
	int64_t                 m_nPointX;
	int64_t                 m_nPointY;
	int64_t                 m_nCenterX;
	int64_t                 m_nCenterY;
	int64_t                 m_nRadius;
	bool                    m_bExportMode;

private:
	CComboBox m_ctrlPointIndexCombo;
	CButton   m_ctrlRegionEnableCheck;
	CButton   m_ctrlMaxCheck;
	CButton   m_ctrlMinCheck;
	CButton   m_ctrlAvgCheck;
	CButton   m_ctrlReflectEnableCheck;


private:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void InitParameter();
	int ReadEnumIntoCombo(const char* strKey, CComboBox &ctrlComboBox);
	int SetEnumIntoDevice(const char* strKey, const CString &strValue);
	void ShowErrorMsg(CString csMessage, int nErrorNum);

private:
	afx_msg void OnBnClickedSaveParameter();            
	afx_msg void OnBnClickedPointInfo();               
	afx_msg void OnPointIndexChanged();


public:

};
