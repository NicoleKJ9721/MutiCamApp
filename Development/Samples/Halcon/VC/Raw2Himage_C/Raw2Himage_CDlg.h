
// Raw2Himage_CDlg.h : 头文件
//

#pragma once
#include "afxwin.h" 
#include <stdio.h>
#include "MvCamera.h"
#include "ConvertPixel.h"

// CRaw2Himage_CDlg 对话框
class CRaw2Himage_CDlg : public CDialog
{
// 构造
public:
	CRaw2Himage_CDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_RAW2HIMAGE_C_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

    /*控件对应变量*/
private:
    /*初始化*/
    CButton m_ctrlOpenButton;                   //打开设备
    CButton m_ctrlCloseButton;                  //关闭设备

    /*图像采集*/
    CButton m_ctrlContinusModeRadio;            //连续模式             
    CButton m_ctrlTriggerModeRadio;             //触发模式
    CButton m_ctrlStartGrabbingButton;          //开始抓图          
    CButton m_ctrlStopGrabbingButton;           //结束抓图
    CButton m_ctrlSoftwareTriggerCheck;         //软触发
    BOOL    m_bSoftWareTriggerCheck;
    CButton m_ctrlSoftwareOnceButton;           //软触发一次

    /*设备显示下拉框*/
    CComboBox m_ctrlDeviceCombo;                //枚举到的设备
    int     m_nDeviceCombo;

    /*内部函数*/
private:
    /*最开始时的窗口初始化*/
    void DisplayWindowInitial(void);          //显示框初始化,最开始的初始化 

    /*按钮的亮暗使能*/
    void EnableWindowWhenClose(void);         //关闭时的按钮颜色，相当于最开始初始化
    void EnableWindowWhenOpenNotStart(void);  //按下打开设备按钮时的按钮颜色  
    void EnableWindowWhenStart(void);         //按下开始采集时的按钮颜色 
    void EnableWindowWhenStop(void);          //按下结束采集时的按钮颜色

    /*初始化相机操作*/
    int OpenDevice(void);                    //打开相机

    /*设置、获取参数操作*/
    int SetTriggerMode(void);                //设置触发模式
    int GetTriggerMode(void);
    int GetTriggerSource(void);              //设置触发源
    int SetTriggerSource(void);

    /*关闭销毁操作*/
    int CloseDevice(void);                   //关闭设备
    int DestroyHandle(void);                 //销毁设备

    /*自己定义变量*/
private:
    BOOL  m_bOpenDevice;                        //是否打开设备
    BOOL  m_bStartGrabbing;                     //是否开始抓图

    BOOL  m_bThreadState;
    void* m_hGrabThread;                        //取流线程句柄

    int   m_nTriggerMode;                       //触发模式
    int   m_nTriggerSource;                     //触发源

    /*设备相关*/
    CMvCamera*      m_pcMyCamera;               //CMyCamera封装了常用接口
    HTuple  m_hWindowID;                        //Halcon显示句柄
    MV_CC_DEVICE_INFO_LIST m_stDevList;         //设备信息列表结构体变量，用来存储设备列表
public:
    /*初始化*/
    afx_msg void OnBnClickedEnumButton();               //查找设备
    afx_msg void OnBnClickedOpenButton();               //打开设备
    afx_msg void OnBnClickedCloseButton();              //关闭设备

    /*图像采集*/
    afx_msg void OnBnClickedContinusModeRadio();        //连续模式
    afx_msg void OnBnClickedTriggerModeRadio();         //触发模式
    afx_msg void OnBnClickedStartGrabbingButton();      //开始采集
    afx_msg void OnBnClickedStopGrabbingButton();       //结束采集
    afx_msg void OnBnClickedSoftwareTriggerCheck();     //软触发
    afx_msg void OnBnClickedSoftwareOnceButton();       //软触发一次

    /*参数设置获取*/
    afx_msg void OnBnClickedGetParameterButton();       //获取参数

public:    
    afx_msg void OnClose();                             //右上角退出
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    static unsigned int __stdcall WorkThread(void* pUser);
};
