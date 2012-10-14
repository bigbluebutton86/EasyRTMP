// EasyFMSDlg.h : header file
//

#if !defined(AFX_EASYFMSDLG_H__FF3DC9FF_1CD4_417C_B933_016840262F93__INCLUDED_)
#define AFX_EASYFMSDLG_H__FF3DC9FF_1CD4_417C_B933_016840262F93__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CEasyRTMPDlg dialog
#include "MainService.h"
#include "afxwin.h"

//#include <GuiLib2008/GuiStaticStandar.h>
//#include "RsControl/RsStatic.h"

#define TIMER_WRITE_LOG		0x2000
#define TIMER_CHECK_LICENSE 0x2001
#define WM_NOTIFYICON		(WM_APP+1)

//#define _USING_BCGP_
#ifdef _USING_BCGP_
#define CDialog CBCGPDialog
#define CListBox CBCGPListBox
#define CButton CBCGPButton
#define CEdit CBCGPEdit
#define CComboBox CBCGPComboBox
#define CStatic CBCGPStatic
#define CStatic2 CBCGPGroup
#else
#define CStatic2 CStatic
#endif

class CEasyRTMPDlg : public CDialog
{
	//////////////////////////////////////////////////////////////////////////
	void InitSoundCards();
	UINT m_nNumMixers;
	HMIXER m_hMixer;
	MIXERCAPS m_mxcaps;
	BOOL amdUninitialize();
	BOOL amdInitialize();
	BOOL amdGetMicSelectControl();
	BOOL amdGetMicSelectValue(LONG &lVal) const;
	BOOL amdSetMicSelectValue(LONG lVal) const;
	CString m_strDstLineName, m_strSelectControlName, m_strMicName;
	DWORD m_dwControlType, m_dwSelectControlID, m_dwMultipleItems, m_dwIndex;

public:
	CStatic2 m_fraServer;
	CStatic2 m_fraEncode;

	CStatic m_lblRed5IP;
	CStatic m_lblRed5Port;
	CStatic m_lblMicrophone;
	CStatic m_lblVideoBitRate;
	CStatic m_lblVideoFrameRate;
	CStatic m_lblVideoResolution;
	CStatic m_lblVideoCodec;
	CStatic	m_lblCamera;

	CListBox m_lstLog;
	CEdit m_edtServerIP;
	CEdit m_edtServerPort;
	CComboBox m_cboCamera;
	CComboBox m_cboCodec;
	CComboBox m_cboResolution;
	CComboBox m_cboFrameRate;
	CComboBox m_cboBitRate;
	CButton m_btnReleaseNotes;
	CButton m_btnAbout;
	CButton m_btnExit;
	CButton m_btnStart;
	CButton m_btnStop;
	//CGuiStaticStandar m_lblCamera;
	//CBCGPCheckBox m_chkEnableAudio;
	CButton m_chkEnableAudio;
	CComboBox m_cboAudioDevices;
	CComboBox m_cboMixLine;
	//////////////////////////////////////////////////////////////////////////
	void				ShowLog(CString strLog);

	bool				InitControls();
	void				EnableControls(BOOL bEnabled);
	CMainService*		m_pMainService;
	void				InitCameras();
//	void				printLines(CWaveINSimple& WaveInDevice);

	//////////////////////////////////////////////////////////////////////////
protected:
	/*
	virtual LRESULT		WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	afx_msg HBRUSH		OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg int			OnCreate(LPCREATESTRUCT lpCreateStruct);
	void				DrawTitleBar(CDC *pDC);
	virtual LRESULT		DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL		PreTranslateMessage(MSG* pMsg);
	afx_msg void		OnNcMouseMove(UINT nHitTest, CPoint point);
	afx_msg void		OnNcLButtonDown(UINT nHitTest, CPoint point);
	CRect				m_rtIcon;				//程序图标位置
	CRect				m_rtButtMax;			//最大化按钮位置
	CRect				m_rtButtMin;			//最小化按钮位置
	CRect				m_rtButtExit;			//关闭按钮位置
	*/
	//////////////////////////////////////////////////////////////////////////
	NOTIFYICONDATA		m_task;
	bool				m_bShowNormal;
// Construction
public:
	CEasyRTMPDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CEasyRTMPDlg)
	enum { IDD = IDD_EASYFMS_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEasyRTMPDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CEasyRTMPDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedBtnExit();
	afx_msg void OnBnClickedBtnReleasenotes();
	afx_msg void OnBnClickedBtnAbout();
	afx_msg void OnBnClickedOk();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedBtnNewver();
	afx_msg void OnCbnSelchangeCboAudio();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EASYFMSDLG_H__FF3DC9FF_1CD4_417C_B933_016840262F93__INCLUDED_)
