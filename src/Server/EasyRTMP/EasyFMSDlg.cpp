// EasyFMSDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EasyFMS.h"
#include "EasyFMSDlg.h"
#include "../../../RsCommon/libEasyCapture/RsCameraDS.h"

#include <limits.h>				// ULONG_MAX

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About
CEasyRTMPDlg* g_pDlg = NULL;

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEasyRTMPDlg dialog

CEasyRTMPDlg::CEasyRTMPDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEasyRTMPDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEasyRTMPDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_hMixer = NULL;
}

void CEasyRTMPDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEasyRTMPDlg)
	// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_LIST1, m_lstLog);
	DDX_Control(pDX, IDC_EDIT_SERVERIP, m_edtServerIP);
	DDX_Control(pDX, IDC_EDIT_SERVERPORT, m_edtServerPort);
	DDX_Control(pDX, IDC_COMBO_CODEC, m_cboCodec);
	DDX_Control(pDX, IDC_COMBO_RESOLUTION, m_cboResolution);
	DDX_Control(pDX, IDC_COMBO_FRAMERATE, m_cboFrameRate);
	DDX_Control(pDX, IDC_COMBO_BITRATE, m_cboBitRate);
	DDX_Control(pDX, IDC_BTN_RELEASENOTES, m_btnReleaseNotes);
	DDX_Control(pDX, IDC_BTN_ABOUT, m_btnAbout);
	DDX_Control(pDX, IDC_BTN_EXIT, m_btnExit);
	DDX_Control(pDX, IDOK, m_btnStart);
	DDX_Control(pDX, IDCANCEL, m_btnStop);
	DDX_Control(pDX, IDC_COMBO_CAMERA, m_cboCamera);
	DDX_Control(pDX, IDC_STATIC_PARAM1, m_fraServer);
	DDX_Control(pDX, IDC_STATIC_PARAM2, m_fraEncode);
	DDX_Control(pDX, IDC_STATIC_CAMERA, m_lblCamera);
	DDX_Control(pDX, IDC_CHK_ENABLE_AUDIO, m_chkEnableAudio);
	DDX_Control(pDX, IDC_CBO_AUDIO, m_cboAudioDevices);
	DDX_Control(pDX, IDC_CBO_MICLINE, m_cboMixLine);
	DDX_Control(pDX, IDC_STATIC_RED5_IP, m_lblRed5IP);
	DDX_Control(pDX, IDC_STATIC_RED5_PORT, m_lblRed5Port);
	DDX_Control(pDX, IDC_STATIC_SOUNDCARD, m_lblMicrophone);
	DDX_Control(pDX, IDC_STATIC_VIDEO_BITRATE, m_lblVideoBitRate);
	DDX_Control(pDX, IDC_STATIC_VIDEO_FRAMERATE, m_lblVideoFrameRate);
	DDX_Control(pDX, IDC_STATIC_VIDEO_RESOLUTION, m_lblVideoResolution);
	DDX_Control(pDX, IDC_STATIC_VIDEO_CODEC, m_lblVideoCodec);
}

BEGIN_MESSAGE_MAP(CEasyRTMPDlg, CDialog)
	//{{AFX_MSG_MAP(CEasyRTMPDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BTN_EXIT, &CEasyRTMPDlg::OnBnClickedBtnExit)
	ON_BN_CLICKED(IDC_BTN_RELEASENOTES, &CEasyRTMPDlg::OnBnClickedBtnReleasenotes)
	ON_BN_CLICKED(IDC_BTN_ABOUT, &CEasyRTMPDlg::OnBnClickedBtnAbout)
	ON_BN_CLICKED(IDOK, &CEasyRTMPDlg::OnBnClickedOk)
	ON_WM_TIMER()
	ON_WM_CTLCOLOR()
	ON_WM_NCLBUTTONDOWN()
	ON_WM_NCMOUSEMOVE()
	ON_WM_CREATE()
	ON_BN_CLICKED(IDC_BTN_NEWVER, &CEasyRTMPDlg::OnBnClickedBtnNewver)
	ON_CBN_SELCHANGE(IDC_CBO_AUDIO, &CEasyRTMPDlg::OnCbnSelchangeCboAudio)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEasyRTMPDlg message handlers

BOOL CEasyRTMPDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	g_pDlg = this;

#ifdef _USING_BCGP_
	BOOL m_bNCArea = TRUE;
	EnableVisualManagerStyle (TRUE, m_bNCArea);
	CBCGPVisualManager2007::SetStyle (CBCGPVisualManager2007::VS2007_ObsidianBlack);
	CBCGPVisualManager::SetDefaultManager (RUNTIME_CLASS (CBCGPVisualManager2007));

	CHOOSEFONT cf;
	DWORD rgbCurrent = 0;   // current text color

	//LOGFONT lf;
	//ZeroMemory(&cf, sizeof(cf));
	//cf.lStructSize = sizeof (cf);
	//cf.hwndOwner = NULL;
	//cf.lpLogFont = &lf;
	//cf.rgbColors = rgbCurrent;
	//cf.Flags = CF_SCREENFONTS | CF_EFFECTS;
	//ChooseFont(&cf);

	LOGFONT lf; //lf定义字体属性 
	lf.lfOutPrecision	= OUT_STROKE_PRECIS; 
	lf.lfClipPrecision	= CLIP_STROKE_PRECIS; 
	lf.lfQuality		= DRAFT_QUALITY; 
	lf.lfPitchAndFamily	= VARIABLE_PITCH|FF_MODERN; 

	lf.lfHeight			= 15; 
	lf.lfWidth			= 0; 
	lf.lfEscapement		= 0; 
	lf.lfOrientation	= 0; 
	lf.lfWeight			= FW_NORMAL;//FW_HEAVY; 
	lf.lfItalic			= FALSE; 
	lf.lfUnderline		= FALSE; 
	lf.lfStrikeOut		= FALSE; 
	lf.lfCharSet		= DEFAULT_CHARSET;//GB2312_CHARSET;
	//strcpy(lf.lfFaceName,"宋体"); 
	strcpy(lf.lfFaceName,"Times New Roman"); 

	globalData.SetMenuFont(&lf,TRUE);
	this->Invalidate(FALSE);

#endif

	InitControls();
	InitCameras();
	InitSoundCards();

	m_pMainService = new CMainService();
	if (!m_pMainService->InitService())
	{
		delete m_pMainService;

		PostMessage(WM_CLOSE, 0, 0);

		return FALSE;
	}

	SetTimer(TIMER_WRITE_LOG, 500, NULL);
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

bool CEasyRTMPDlg::InitControls()
{
	int i = 0, nIndex = 0;
	m_edtServerIP.SetWindowText("10.131.1.73");
	m_edtServerIP.SetWindowText("127.0.0.1");
	m_edtServerPort.SetWindowText("1935");
	//Codec
	nIndex = m_cboCodec.AddString("H.264 - x264");
	m_cboCodec.AddString("H.264 - FFMPEG");
	m_cboCodec.SetCurSel(nIndex);
	//Frame Rate
	nIndex = m_cboFrameRate.AddString("5");
	nIndex = m_cboFrameRate.AddString("10");
	nIndex = m_cboFrameRate.AddString("15");
	nIndex = m_cboFrameRate.AddString("20");
	nIndex = m_cboFrameRate.AddString("25");
	nIndex = m_cboFrameRate.AddString("30");
	nIndex = m_cboFrameRate.SetCurSel(nIndex);
	//Bit Rate
	nIndex = m_cboBitRate.AddString("128");
	nIndex = m_cboBitRate.AddString("192");
	nIndex = m_cboBitRate.AddString("256");
	m_cboBitRate.AddString("384");
	m_cboBitRate.SetCurSel(nIndex);
	//Resolution
	nIndex = m_cboResolution.AddString("QQVGA - 160x120");
	nIndex = m_cboResolution.AddString("QVGA  - 320x240");
	m_cboResolution.AddString("VGA   - 640x480");
	m_cboResolution.SetCurSel(nIndex);
	//Enable Audio
	//m_chkEnableAudio.SetCheck(1);
	//task bar
	_tcscpy(m_task.szTip, "EasyRTMP @ RG4.NET");
	m_task.cbSize	= sizeof(NOTIFYICONDATA);
	m_task.hWnd		= this->m_hWnd;
	m_task.uFlags	= NIF_MESSAGE | NIF_ICON | NIF_TIP;
	m_task.uCallbackMessage = WM_NOTIFYICON; 
	m_task.uID		= IDR_MAINFRAME;
	m_task.hIcon	= m_hIcon;
	::Shell_NotifyIcon(NIM_ADD, &m_task);
	m_bShowNormal	= true;

	return true;
}

void CEasyRTMPDlg::InitCameras()
{
	//Cameras
	int cam_count, i;
	cam_count = CMainService::DS_CameraCount();
	for(i=0; i < cam_count; i++)
	{
		char camera_name[1024];  
		int retval = CMainService::DS_CameraName(i, camera_name, sizeof(camera_name) );
		if (retval > 0)
			m_cboCamera.AddString(camera_name);
		else
			m_cboCamera.AddString("--UNKNOWN CAMERA--");
	}
	if (cam_count > 0)
		m_cboCamera.SetCurSel(0);

	//Sound cards
	//const vector<CWaveINSimple*>& wInDevices = CWaveINSimple::GetDevices();
	////wInDevices = CWaveINSimple::GetDevices();
	//m_cboAudioDevices.ResetContent();
	//for (i = 0; i < wInDevices.size(); i++)
	//{
	//	m_cboAudioDevices.AddString(wInDevices[i]->GetName());
	//	m_cboAudioDevices.SetItemData(i, (DWORD_PTR)wInDevices[i]);
	//	TRACE("%s\n", wInDevices[i]->GetName());
	//}
	//if (m_cboAudioDevices.GetCount() > 0)
	//{
	//	printLines(*wInDevices[0]);
	//	m_cboAudioDevices.SetCurSel(0);
	//}
}

void CEasyRTMPDlg::EnableControls(BOOL bEnabled)
{
	m_cboResolution.EnableWindow(bEnabled);
	m_cboFrameRate.EnableWindow(bEnabled);
	m_cboBitRate.EnableWindow(bEnabled);
	m_cboCodec.EnableWindow(bEnabled);
	m_edtServerIP.EnableWindow(bEnabled);
	m_edtServerPort.EnableWindow(bEnabled);
	m_cboCamera.EnableWindow(bEnabled);
	m_cboAudioDevices.EnableWindow(bEnabled);
	m_cboMixLine.EnableWindow(bEnabled);

	m_chkEnableAudio.EnableWindow(bEnabled);

	m_btnStart.EnableWindow(bEnabled);
	m_btnStop.EnableWindow(!bEnabled);
}

void CEasyRTMPDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CEasyRTMPDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CEasyRTMPDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

BOOL CEasyRTMPDlg::DestroyWindow() 
{
	// TODO: Add your specialized code here and/or call the base class
	Shell_NotifyIcon(NIM_DELETE, &m_task);

	if (m_pMainService)
	{
		delete m_pMainService;
		m_pMainService = NULL;
	}
	
	KillTimer(TIMER_CHECK_LICENSE);
	KillTimer(TIMER_WRITE_LOG);

	return CDialog::DestroyWindow();
}

void CEasyRTMPDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	if (m_pMainService)
	{
		m_pMainService->StopCapture(0);
		EnableControls(TRUE);
		GetDlgItem(IDC_STATIC_STATUS)->SetWindowText("Stream Stopped!");
		KillTimer(TIMER_CHECK_LICENSE);
	}
	//OnCancel();
}

void CEasyRTMPDlg::OnBnClickedBtnExit()
{
	// TODO: Add your control notification handler code here
	CDialog::OnOK();
}

void CEasyRTMPDlg::OnBnClickedBtnReleasenotes()
{
	// TODO: Add your control notification handler code here
	if (m_pMainService)
		m_pMainService->GotoURL("http://rg4.net/p/easyrtmp", SW_SHOW);
}

void CEasyRTMPDlg::OnBnClickedBtnAbout()
{
	// TODO: Add your control notification handler code here
	//CAboutDlg dlg;
	//dlg.DoModal();
	if (m_pMainService)
		m_pMainService->GotoURL("http://rg4.net/p/easyrtmp", SW_SHOW);
}

void CEasyRTMPDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	bool bResult = false;
	int nIndexVideo, nIndexAudio = 0;
	//1. Video related
	int nChannelID = 0;
	nChannelID = m_cboCamera.GetCurSel();
	if (nChannelID < 0)
	{
		int cam_count;
		cam_count = CMainService::DS_CameraCount();
		if (cam_count > 0)
		{
			m_cboCamera.ResetContent();
			for(int i=0; i < cam_count; i++)
			{
				char camera_name[1024];  
				int retval = CMainService::DS_CameraName(i, camera_name, sizeof(camera_name) );
				if (retval > 0)
					m_cboCamera.AddString(camera_name);
				else
					m_cboCamera.AddString("UNKNOWN CAMERA");
			}
			AfxMessageBox("Select Camera...");
		}
		else
		{
			AfxMessageBox("You don't have a camera.");
		}
		return;
	}

	//2. Audio related
	int n = m_chkEnableAudio.GetCheck();
	bool bAudioEnabled = m_chkEnableAudio.GetCheck();
//	bool bAudioEnabled = (n == 0);

	CString strDeviceName, strMixerLine;
	////if (bAudioEnabled)
	//{
	//	UINT i;
	//	int nChannelAudio = m_cboAudioDevices.GetCurSel();
	//	if (nChannelAudio < 0)
	//	{
	//		const vector<CWaveINSimple*>& wInDevices = CWaveINSimple::GetDevices();
	//		m_cboAudioDevices.ResetContent();
	//		for (i = 0; i < wInDevices.size(); i++)
	//		{
	//			nIndexAudio = m_cboAudioDevices.AddString(wInDevices[i]->GetName());
	//			TRACE("%s\n", wInDevices[i]->GetName());
	//			m_cboAudioDevices.SetItemData(i, (DWORD_PTR)wInDevices[i]);
	//		}
	//		if (m_cboAudioDevices.GetCount() > 0)
	//		{
	//			m_cboAudioDevices.SetCurSel(0);
	//			nChannelAudio = 0;
	//		}
	//	}
	//	if (nChannelAudio < 0) return;
	//	m_cboAudioDevices.GetLBText(nChannelAudio, strDeviceName);
	//	m_cboMixLine.GetLBText(0, strMixerLine);
	//}

	//3. Start Capture
	if (m_pMainService)
	{
		CString strTemp, str;
		int nCodecID, nFrameRate, nBitRate,nWidth, nHeight, nServerPort;

		nCodecID	= 28;//CODEC_ID_H264;
		nIndexVideo	= m_cboFrameRate.GetCurSel();
		m_cboFrameRate.GetLBText(nIndexVideo, strTemp);
		nFrameRate	= atoi(strTemp);
		nIndexVideo	= m_cboBitRate.GetCurSel();
		m_cboBitRate.GetLBText(nIndexVideo, strTemp);
		nBitRate	= atoi(strTemp);
		//Resolution
		nWidth		= 320;
		nHeight		= 240;
		nIndexVideo	= m_cboResolution.GetCurSel();
		m_cboResolution.GetLBText(nIndexVideo, strTemp);
		if (strTemp.Find(" - ") > 0)
		{
			strTemp = strTemp.Mid(strTemp.Find(" - ") + 3);
			if (strTemp.Find("x") == 3)
			{
				str = strTemp.Left(strTemp.Find("x"));
				nWidth = atoi(str);
				str = strTemp.Mid(strTemp.Find("x") + 1);
				nHeight = atoi(str);
			}
		}
		//ServerIP
		m_edtServerIP.GetWindowText(str);
		m_edtServerPort.GetWindowText(strTemp);
		nServerPort = atoi(strTemp);

		STREAMPROFILE vProfile;
		vProfile.wImageWidth	= nWidth;
		vProfile.wImageHeight	= nHeight;
		vProfile.iCodecIDV		= nCodecID;
		vProfile.dFrameRateV	= nFrameRate;
		vProfile.iBitRateV		= nBitRate;
		vProfile.iBitsPerSample	= 16;
		vProfile.iSamplePerSec	= 44100;
		vProfile.iChannels		= 2;
		vProfile.iCodecIDA		= 0x15000 + 1;	//CODEC_ID_MP3;
		vProfile.iCodecIDA		= 0x15000 + 2;	//CODEC_ID_AAC;
		vProfile.iBitRateA		= 128000;
		vProfile.iBytesPerCapture = 1024;

		strcpy(m_pMainService->m_vRTMPSvrInfo.szServerIP, str);
		m_pMainService->m_vRTMPSvrInfo.nServerPort	= nServerPort;

		bResult = m_pMainService->StartCapture(nChannelID, bAudioEnabled, strDeviceName, strMixerLine, &vProfile);

		if (bResult)
		{
			EnableControls(FALSE);

			CString strServerIP, strServerPort;
			m_edtServerIP.GetWindowText(strServerIP);
			m_edtServerPort.GetWindowText(strServerPort);

			strTemp.Format("Streaming to Red5. URL=rtmp://%s:%s/live/%d_%dx%d", strServerIP, strServerPort, nChannelID, nWidth, nHeight);
			GetDlgItem(IDC_STATIC_STATUS)->SetWindowText(strTemp);

			SetTimer(TIMER_CHECK_LICENSE, 1000, NULL);
		}
		else
		{
			AfxMessageBox("Start Capture Failed!");
		}
	}
	//CDialog::OnOK();
}

void CEasyRTMPDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	switch(nIDEvent)
	{
	case TIMER_WRITE_LOG:
		if (m_pMainService)
		{
			if (!m_pMainService->WriteLogToFile())
				CMainService::stcLogReport(LOGLEVEL0, "E", "Log", "Write LogFile Error");
		}
		break;
	case TIMER_CHECK_LICENSE:
		if (m_pMainService)
		{
			m_pMainService->Timer_Main();
			if (m_pMainService->m_nTimeCount >= EASYRTMP_MAX_DEMO_TIME)
			{
				CMainService::stcLogReport(LOGLEVEL0, "W", "License", "Demo timeout, system stopped!!!");
				OnBnClickedCancel();
				KillTimer(TIMER_CHECK_LICENSE);
			}
		}
	default:
		break;
	}

	CDialog::OnTimer(nIDEvent);
}
/*
void CEasyRTMPDlg::DrawTitleBar(CDC *pDC) 
{ 
	if (m_hWnd)
	{
		CBrush Brush(RGB(187,200,143));
		CBrush* pOldBrush = pDC->SelectObject(&Brush);

		CRect rtWnd, rtTitle, rtButtons;
		GetWindowRect(&rtWnd);

		//取得标题栏的位置
		//SM_CXFRAME 窗口边框的边缘宽度
		//SM_CYFRAME 窗口边框的边缘高度
		//SM_CXSIZE  窗口标题栏宽度
		//SM_CYSIZE  窗口标题栏高度
		rtTitle.left = GetSystemMetrics(SM_CXFRAME); 
		rtTitle.top = GetSystemMetrics(SM_CYFRAME);  
		rtTitle.right = rtWnd.right - rtWnd.left - GetSystemMetrics(SM_CXFRAME);
		rtTitle.bottom = rtTitle.top + GetSystemMetrics(SM_CYSIZE);

		CPoint point;
		//填充顶部框架
		point.x = rtWnd.Width();						
		point.y = GetSystemMetrics(SM_CYSIZE) + GetSystemMetrics(SM_CYFRAME) + 0;
		pDC->PatBlt(0, 0, point.x, point.y, PATCOPY);
		//填充左侧框架
		point.x = GetSystemMetrics(SM_CXFRAME) -1;
		point.y = rtWnd.Height()- 1;
		pDC->PatBlt(0, 0, point.x, point.y, PATCOPY);
		//填充底部框架
		point.x = rtWnd.Width(); 
		point.y = GetSystemMetrics(SM_CYFRAME);
		pDC->PatBlt(0, rtWnd.Height()-point.y, point.x, point.y, PATCOPY);
		//填充右侧框架
		point.x = GetSystemMetrics(SM_CXFRAME);
		point.y = rtWnd.Height();
		pDC->PatBlt(rtWnd.Width()-point.x, 0, point.x, point.y, PATCOPY);


		//重画标题栏图标
		m_rtIcon.left = rtTitle.left ;
		m_rtIcon.top = rtTitle.top;
		m_rtIcon.right = m_rtIcon.left + 16;
		m_rtIcon.bottom = m_rtIcon.top + 15;
		::DrawIconEx(pDC->m_hDC, m_rtIcon.left, m_rtIcon.top,  AfxGetApp()->LoadIcon(IDR_MAINFRAME), 
			m_rtIcon.Width(), m_rtIcon.Height(), 0, NULL, DI_NORMAL);
		m_rtIcon.OffsetRect(rtWnd.TopLeft());

		CBitmap* pBitmap = new CBitmap;
		CBitmap* pOldBitmap;
		CDC* pDisplayMemDC=new CDC;
		pDisplayMemDC->CreateCompatibleDC(pDC);

		//重画关闭button
		rtButtons.left = rtTitle.right - 16;
		rtButtons.top = rtTitle.top - 1;
		rtButtons.right = rtButtons.left + 16;
		rtButtons.bottom = rtButtons.top + 15;
		pBitmap->LoadBitmap(IDB_EXIT_FOCUS);
		pOldBitmap=(CBitmap*)pDisplayMemDC->SelectObject(pBitmap);
		pDC->BitBlt(rtButtons.left, rtButtons.top, rtButtons.Width(), rtButtons.Height(), pDisplayMemDC, 0, 0, SRCCOPY);
		pDisplayMemDC->SelectObject(pOldBitmap);
		m_rtButtExit = rtButtons;
		m_rtButtExit.OffsetRect(rtWnd.TopLeft()); 
		pBitmap->DeleteObject();

		//重画最大化/恢复button
		rtButtons.right = rtButtons.left - 3;
		rtButtons.left = rtButtons.right - 16;
		if (IsZoomed())
			pBitmap->LoadBitmap(IDB_RESTORE);
		else
			pBitmap->LoadBitmap(IDB_MAX);
		pOldBitmap=(CBitmap*)pDisplayMemDC->SelectObject(pBitmap);
		pDC->BitBlt(rtButtons.left, rtButtons.top, rtButtons.Width(), rtButtons.Height(), pDisplayMemDC, 0, 0, SRCCOPY);
		pDisplayMemDC->SelectObject(pOldBitmap);
		m_rtButtMax = rtButtons;
		m_rtButtMax.OffsetRect(rtWnd.TopLeft());
		pBitmap->DeleteObject();

		//重画最小化button
		rtButtons.right = rtButtons.left - 3;
		rtButtons.left = rtButtons.right - 16;
		pBitmap->LoadBitmap(IDB_MIN);
		pOldBitmap=(CBitmap*)pDisplayMemDC->SelectObject(pBitmap);
		pDC->BitBlt(rtButtons.left, rtButtons.top, rtButtons.Width(), rtButtons.Height(), pDisplayMemDC, 0, 0, SRCCOPY);
		pDisplayMemDC->SelectObject(pOldBitmap);
		m_rtButtMin = rtButtons;
		m_rtButtMin.OffsetRect(rtWnd.TopLeft());
		pBitmap->DeleteObject();

		//重画caption
		int nOldMode = pDC->SetBkMode(TRANSPARENT);
		COLORREF clOldText=pDC->SetTextColor(RGB(255, 255, 255));

		CFont m_captionFont;
		m_captionFont.CreateFont(
			18,                        // 字体的高度			
			0,                         // 字体的宽度
			0,                         // 字体显示的角度
			0,                         // 字体的角度
			FW_BOLD,                   // 字体的磅数
			FALSE,                     // 斜体字体
			FALSE,                     // 带下划线的字体
			0,                         // 带删除线的字体
			ANSI_CHARSET,              // 所需的字符集
			OUT_DEFAULT_PRECIS,        // 输出的精度
			CLIP_DEFAULT_PRECIS,       // 裁减的精度
			DEFAULT_QUALITY,           // 逻辑字体与输出设备的实际字体之间的精度
			DEFAULT_PITCH | FF_SWISS,  // 字体间距和字体集
			_T("Arial"));              // 字体名称

		CFont* pOldFont = NULL;			
		pOldFont = pDC->SelectObject(&m_captionFont);

		rtTitle.left += m_rtIcon.Width ()+3;
		rtTitle.top = rtTitle.top;
		rtTitle.bottom = rtTitle.top + 30;
		CString m_strTitle;
		GetWindowText(m_strTitle);
		pDC->DrawText(m_strTitle, &rtTitle, DT_LEFT);
		pDC->SetBkMode(nOldMode);
		pDC->SetTextColor(clOldText);

		ReleaseDC(pDisplayMemDC);
		delete pDisplayMemDC;
		delete pBitmap;
	}
} 

//实现标题栏按钮的鼠标移动效果
void CEasyRTMPDlg::OnNcMouseMove(UINT nHitTest, CPoint point) 
{
	CWindowDC dc(this);
	CWindowDC* pDC = &dc;
	CDC* pDisplayMemDC=new CDC;
	pDisplayMemDC->CreateCompatibleDC(pDC);
	CBitmap* pBitmap = new CBitmap;
	CBitmap* pOldBitmap;
	CRect rtWnd, rtButton;

	if (pDC)
	{
		GetWindowRect(&rtWnd);

		//关闭button
		if (m_rtButtExit.PtInRect(point))
			pBitmap->LoadBitmap(IDB_EXIT);
		else
			pBitmap->LoadBitmap(IDB_EXIT_FOCUS);
		rtButton = m_rtButtExit;
		rtButton.OffsetRect(-rtWnd.left, -rtWnd.top);
		pOldBitmap=(CBitmap*)pDisplayMemDC->SelectObject(pBitmap);
		pDC->BitBlt(rtButton.left, rtButton.top, rtButton.Width(), rtButton.Height(), pDisplayMemDC, 0, 0, SRCCOPY);
		pDisplayMemDC->SelectObject(pOldBitmap);
		pBitmap->DeleteObject();

		////最大化/恢复button
		//if (m_rtButtMax.PtInRect(point))
		//{
		//if (IsZoomed())
		//pBitmap->LoadBitmap(IDB_RESTORE_FOCUS);
		//else
		//pBitmap->LoadBitmap(IDB_MAX_FOCUS);
		//}
		//else
		//{
		//if (IsZoomed())
		//pBitmap->LoadBitmap(IDB_RESTORE);
		//else
		//pBitmap->LoadBitmap(IDB_MAX);
		//}
		//rtButton = m_rtButtMax;
		//rtButton.OffsetRect(-rtWnd.left, -rtWnd.top);
		//pOldBitmap=(CBitmap*)pDisplayMemDC->SelectObject(pBitmap);
		//pDC->BitBlt(rtButton.left, rtButton.top, rtButton.Width(), rtButton.Height(), pDisplayMemDC, 0, 0, SRCCOPY);
		//pDisplayMemDC->SelectObject(pOldBitmap);
		//pBitmap->DeleteObject();

		//最小化button
		if (m_rtButtMin.PtInRect(point))
			pBitmap->LoadBitmap(IDB_MIN_FOCUS);
		else
			pBitmap->LoadBitmap(IDB_MIN);
		rtButton = m_rtButtMin;
		rtButton.OffsetRect(-rtWnd.left, -rtWnd.top);
		pOldBitmap=(CBitmap*)pDisplayMemDC->SelectObject(pBitmap);
		pDC->BitBlt(rtButton.left, rtButton.top, rtButton.Width(), rtButton.Height(), pDisplayMemDC, 0, 0, SRCCOPY);
		pDisplayMemDC->SelectObject(pOldBitmap);
		pBitmap->DeleteObject();

	}

	pDisplayMemDC->DeleteDC();

	delete pDisplayMemDC;
	delete pBitmap;

	CDialog::OnNcMouseMove(nHitTest, point); 
}

//实现非客户区，用户鼠标点击操作
void CEasyRTMPDlg::OnNcLButtonDown(UINT nHitTest, CPoint point) 
{
	//检测各按钮是否按到
	if (m_rtIcon.PtInRect(point))
		AfxMessageBox("EasyCvt @ RG4.NET!");
	//	else if (m_rtButtHelp.PtInRect(point))
	//		SendMessage(WM_HELP);
	else if (m_rtButtExit.PtInRect(point))
	{
		//SendMessage(WM_CLOSE);
		SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, MAKELPARAM(point.x, point.y));
	}
	else if (m_rtButtMin.PtInRect(point))
		SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, MAKELPARAM(point.x, point.y));
	//	else if (m_rtButtMax.PtInRect(point))
	//	{
	//		if (IsZoomed())
	//			SendMessage(WM_SYSCOMMAND, SC_RESTORE, MAKELPARAM(point.x, point.y));
	//		else
	//		{
	//			SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, MAKELPARAM(point.x, point.y));
	//			Invalidate();
	//		}
	//	}
	else if (!IsZoomed())
		Default();
	else
	{
	}
}

//屏蔽回车和ESC退出
BOOL CEasyRTMPDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if(pMsg->message == WM_KEYDOWN){         
		switch(pMsg->wParam){
		case VK_RETURN://截获回车 
			return TRUE;
		case VK_ESCAPE://截获ESC 
			return TRUE;
			break;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

HBRUSH CEasyRTMPDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  Change any attributes of the DC here

	//static颜色
	if (pWnd->GetDlgCtrlID()==IDC_STATIC) 
	{
		pDC->SetTextColor(RGB(0,100,0));
		pDC->SetBkMode(TRANSPARENT);
	}

	////标题
	//if (pWnd->GetDlgCtrlID()==IDC_STATIC_TITLE) 
	//{
	//CFont font;
	//font.CreatePointFont(200,"隶书");
	//pDC->SelectObject(&font);
	//pDC->SetTextColor(RGB(0,0,255));
	//pDC->SetBkMode(TRANSPARENT);
	//}
	switch(nCtlColor)
	{
	case CTLCOLOR_STATIC:
		pDC->SetBkMode(TRANSPARENT); 
		HBRUSH B=CreateSolidBrush(RGB(236,233,216));
		return B;	
	}

	// TODO:  Return a different brush if the default is not desired
	return hbr;
}

int CEasyRTMPDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	CRect rtWnd;
	GetWindowRect(&rtWnd);

	CRgn rgn;
	rgn.CreateRoundRectRgn(0,0,rtWnd.Width(),rtWnd.Height(),5,5);
	SetWindowRgn((HRGN)rgn,true); 

	return 0;
}

LRESULT CEasyRTMPDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	LRESULT lrst=CDialog::DefWindowProc(message, wParam, lParam);

	//IsWindow()函数用法：
	//IsWindow()函数判断hwndDlg是否已经指向一个窗口，如果返回TRUE，表示对话框已经显示，
	//不需要再调用对话框，如果返回FALSE，调用CreateDialog()显示对话框。要注意一点，
	//hwndDlg必须声明为全局变量，而不是对话框函数的私有变量，否则每次对话框函数被调用都将重新初始化
	//该变量IsWindow(hwndDlg)总方会FALSE，上面的判断语句就形同虚设了。同时应在调用EndDialog()前设hwndDlg=NULL，
	//否则对话框关闭后IsWindow(hwndDlg)总返回TRUE，再也打不开了

	if (!::IsWindow(m_hWnd))
		return lrst;

	if (message==WM_MOVE||message==WM_PAINT||message==WM_NCPAINT||message==WM_NCACTIVATE ||message == WM_NOTIFY)
	{
		try
		{
			CDC* pWinDC = GetWindowDC();
			if (pWinDC)
				DrawTitleBar(pWinDC);
			ReleaseDC(pWinDC);
		}
		catch( CException* e )
		{
			e->Delete();
		}
	}
	return lrst;
}

LRESULT CEasyRTMPDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: Add your specialized code here and/or call the base class
	switch(message)
	{
	case WM_NOTIFYICON: 
		// right click
		//if(lParam==WM_RBUTTONDOWN)
		if ((lParam == WM_RBUTTONUP))
		{ 
			CMenu menu;
			menu.LoadMenu(IDR_MENU1); 
			CPoint pos;
			GetCursorPos(&pos);
			SetForegroundWindow();
			if (::GetActiveWindow() != m_hWnd)
				break;
			menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pos.x, pos.y, AfxGetMainWnd());
		}
		else if (lParam == WM_LBUTTONUP)
		{
			if (m_bShowNormal)
				this->ShowWindow(SW_HIDE);
			else
				this->ShowWindow(SW_SHOW);

			m_bShowNormal = !m_bShowNormal;
		}
		break;
	case WM_SYSCOMMAND: 
		if(wParam==SC_MINIMIZE || wParam==SC_CLOSE)
		{
			AfxGetApp()->m_pMainWnd->ShowWindow(SW_HIDE);
			m_bShowNormal = false;
			return 0;
		}
		break;
	}

	return CDialog::WindowProc(message, wParam, lParam);
}
*/

void CEasyRTMPDlg::OnBnClickedBtnNewver()
{
	// TODO: Add your control notification handler code here
}

void CEasyRTMPDlg::OnCbnSelchangeCboAudio()
{
	// TODO: Add your control notification handler code here
	/*
	CString strDeviceName;
	int nIndex = m_cboAudioDevices.GetCurSel();

	if (nIndex >= 0)
	{
		CWaveINSimple* p = (CWaveINSimple*)m_cboAudioDevices.GetItemData(nIndex);

		CHAR szName[MIXER_LONG_NAME_CHARS];
		UINT j;

		try
		{
			CMixer& mixer = p->OpenMixer();
			const vector<CMixerLine*>& mLines = mixer.GetLines();
			for (j = 0; j < mLines.size(); j++)
			{
				::CharToOem(mLines[j]->GetName(), szName);
				printf("%s\n", szName);
				m_cboMixLine.AddString(szName);
			}

			mixer.Close();
		}
		catch (const char *err)
		{
			printf("%s\n",err);
		}
	}
	*/
}

void CEasyRTMPDlg::ShowLog(CString strLog)
{
	CString szLogPath = m_pMainService->m_vProductInfo.m_szLogPath.c_str();

	BOOL hr = ::SHCreateDirectoryEx(NULL, szLogPath, NULL);
	if (hr != ERROR_ALREADY_EXISTS && hr != ERROR_SUCCESS)
	{
		return;
	}

	// Open Log File
	CTime ctDateTime = CTime::GetCurrentTime();
	CString szLogName;
	if (m_pMainService->m_vProductInfo.m_nLogLevel <= 2)	// Log file Per day
		szLogName.Format("%s\\EasyRTMP_%s.txt", szLogPath, ctDateTime.Format("%Y-%m-%d"));
	else if (m_pMainService->m_vProductInfo.m_nLogLevel >= 3 && m_pMainService->m_vProductInfo.m_nLogLevel <= 4)	// Log file Per hour
		szLogName.Format("%s\\EasyRTMP_%s.txt", szLogPath, ctDateTime.Format("%Y-%m-%d_%H"));
	else	// Log file Per 10 minutes
		szLogName.Format("%s\\EasyRTMP_%s_%.2d.txt", szLogPath, ctDateTime.Format("%Y-%m-%d_%H"), ctDateTime.GetMinute() / 10);

	BOOL bFileException = FALSE;

	CFile vLogFile;
	try
	{
		if (vLogFile.Open(szLogName, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite))
		{
			vLogFile.SeekToEnd();
			vLogFile.Write(strLog, strLog.GetLength());
			vLogFile.Close();
		}
	}
	catch (...)
	{

	}
	if (m_lstLog.AddString(strLog) == LB_ERRSPACE)
	{
		m_lstLog.ResetContent();
		m_lstLog.AddString(strLog);
	}

	int nCount = m_lstLog.GetCount();
	if (nCount > 500) m_lstLog.DeleteString(0);
	nCount = m_lstLog.GetCount();
	if (nCount > 0) m_lstLog.SetCurSel(nCount-1);
}


BOOL CEasyRTMPDlg::amdInitialize()
{
	ASSERT(m_hMixer == NULL);

	// get the number of mixer devices present in the system
	m_nNumMixers = ::mixerGetNumDevs();

	m_hMixer = NULL;
	::ZeroMemory(&m_mxcaps, sizeof(MIXERCAPS));

	m_strDstLineName.Empty();
	m_strSelectControlName.Empty();
	m_strMicName.Empty();
	m_dwControlType = 0;
	m_dwSelectControlID = 0;
	m_dwMultipleItems = 0;
	m_dwIndex = ULONG_MAX;

	// open the first mixer
	// A "mapper" for audio mixer devices does not currently exist.
	if (m_nNumMixers != 0)
	{
		if (::mixerOpen(&m_hMixer,
			0,
			reinterpret_cast<DWORD>(this->GetSafeHwnd()),
			NULL,
			MIXER_OBJECTF_MIXER | CALLBACK_WINDOW)
			!= MMSYSERR_NOERROR)
		{
			return FALSE;
		}

		if (::mixerGetDevCaps(reinterpret_cast<UINT>(m_hMixer),
			&m_mxcaps, sizeof(MIXERCAPS))
			!= MMSYSERR_NOERROR)
		{
			return FALSE;
		}
	}

	return TRUE;
}

BOOL CEasyRTMPDlg::amdUninitialize()
{
	BOOL bSucc = TRUE;

	if (m_hMixer != NULL)
	{
		bSucc = (::mixerClose(m_hMixer) == MMSYSERR_NOERROR);
		m_hMixer = NULL;
	}

	return bSucc;
}

void CEasyRTMPDlg::InitSoundCards()
{
	int nIndex , nValue = 0;
	if (this->amdInitialize())
	{
		// display the number of mixer devices present in the system
		//m_strStat1.Format(IDS_FMTNUMMIXERS, m_nNumMixers);

		// display the information about the first mixer
		//m_strStat2.Format(IDS_FMTCAPS,m_mxcaps.szPname, m_mxcaps.cDestinations);
		nIndex = m_cboAudioDevices.AddString(m_mxcaps.szPname);
		m_cboAudioDevices.SetCurSel(nIndex);

		nIndex = m_cboMixLine.AddString(m_mxcaps.szPname);
		m_cboMixLine.SetCurSel(nIndex);

		// get the Control ID, index and the names
		if (this->amdGetMicSelectControl())
		{
			// display the line and control name
			//m_strStat3.Format(IDS_FMTNAMES,m_strDstLineName, m_strSelectControlName);

			// display the Microphone Select item name
			//this->SetDlgItemText(IDC_CHECK, m_strMicName);

			// get the initial value of the Microphone Select control
			LONG lVal = 0;
			if (this->amdGetMicSelectValue(lVal))
			{
				// lVal may be zero or nonzero.
				// m_bSelect needs to be 0 or 1 in order for DDX_Check to work.
//				m_bSelect = (lVal != 0);
			}
			else
			{
				ASSERT(FALSE);
			}
		}
		else
		{
			// disable the checkbox
			m_cboAudioDevices.EnableWindow(FALSE);
		}

		VERIFY(this->UpdateData(FALSE));
	}
	else
	{
		ASSERT(FALSE);
	}
}

BOOL CEasyRTMPDlg::amdGetMicSelectControl()
{
	if (m_hMixer == NULL)
	{
		return FALSE;
	}

	// get dwLineID
	MIXERLINE mxl;
	mxl.cbStruct = sizeof(MIXERLINE);
	mxl.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_WAVEIN;
	if (::mixerGetLineInfo(reinterpret_cast<HMIXEROBJ>(m_hMixer),
		&mxl,
		MIXER_OBJECTF_HMIXER |
		MIXER_GETLINEINFOF_COMPONENTTYPE)
		!= MMSYSERR_NOERROR)
	{
		return FALSE;
	}

	// get dwControlID
	MIXERCONTROL mxc;
	MIXERLINECONTROLS mxlc;
	m_dwControlType = MIXERCONTROL_CONTROLTYPE_MIXER;
	mxlc.cbStruct = sizeof(MIXERLINECONTROLS);
	mxlc.dwLineID = mxl.dwLineID;
	mxlc.dwControlType = m_dwControlType;
	mxlc.cControls = 1;
	mxlc.cbmxctrl = sizeof(MIXERCONTROL);
	mxlc.pamxctrl = &mxc;
	if (::mixerGetLineControls(reinterpret_cast<HMIXEROBJ>(m_hMixer),
		&mxlc,
		MIXER_OBJECTF_HMIXER |
		MIXER_GETLINECONTROLSF_ONEBYTYPE)
		!= MMSYSERR_NOERROR)
	{
		// no mixer, try MUX
		m_dwControlType = MIXERCONTROL_CONTROLTYPE_MUX;
		mxlc.cbStruct = sizeof(MIXERLINECONTROLS);
		mxlc.dwLineID = mxl.dwLineID;
		mxlc.dwControlType = m_dwControlType;
		mxlc.cControls = 1;
		mxlc.cbmxctrl = sizeof(MIXERCONTROL);
		mxlc.pamxctrl = &mxc;
		if (::mixerGetLineControls(reinterpret_cast<HMIXEROBJ>(m_hMixer),
			&mxlc,
			MIXER_OBJECTF_HMIXER |
			MIXER_GETLINECONTROLSF_ONEBYTYPE)
			!= MMSYSERR_NOERROR)
		{
			return FALSE;
		}
	}

	// store dwControlID, cMultipleItems
	m_strDstLineName = mxl.szName;
	m_strSelectControlName = mxc.szName;
	m_dwSelectControlID = mxc.dwControlID;
	m_dwMultipleItems = mxc.cMultipleItems;

	if (m_dwMultipleItems == 0)
	{
		return FALSE;
	}

	// get the index of the Microphone Select control
	MIXERCONTROLDETAILS_LISTTEXT *pmxcdSelectText = new MIXERCONTROLDETAILS_LISTTEXT[m_dwMultipleItems];
	if (pmxcdSelectText != NULL)
	{
		MIXERCONTROLDETAILS mxcd;
		mxcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
		mxcd.dwControlID = m_dwSelectControlID;
		mxcd.cChannels = 1;
		mxcd.cMultipleItems = m_dwMultipleItems;
		mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_LISTTEXT);
		mxcd.paDetails = pmxcdSelectText;
		if (::mixerGetControlDetails(reinterpret_cast<HMIXEROBJ>(m_hMixer),
			&mxcd,
			MIXER_OBJECTF_HMIXER |
			MIXER_GETCONTROLDETAILSF_LISTTEXT)
			== MMSYSERR_NOERROR)
		{
			DWORD dwi ;
			// determine which controls the Microphone source line
			for (dwi = 0; dwi < m_dwMultipleItems; dwi++)
			{
				// get the line information
				MIXERLINE mxl;
				mxl.cbStruct = sizeof(MIXERLINE);
				mxl.dwLineID = pmxcdSelectText[dwi].dwParam1;
				if (::mixerGetLineInfo(reinterpret_cast<HMIXEROBJ>(m_hMixer),
					&mxl,
					MIXER_OBJECTF_HMIXER |
					MIXER_GETLINEINFOF_LINEID)
					== MMSYSERR_NOERROR &&
					mxl.dwComponentType == MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE)
				{
					// found, dwi is the index.
					m_dwIndex = dwi;
					m_strMicName = pmxcdSelectText[dwi].szName;
					break;
				}
			}

			if (dwi >= m_dwMultipleItems)
			{
				// could not find it using line IDs, some mixer drivers have
				// different meaning for MIXERCONTROLDETAILS_LISTTEXT.dwParam1.
				// let's try comparing the item names.
				for (dwi = 0; dwi < m_dwMultipleItems; dwi++)
				{
					if (::lstrcmp(pmxcdSelectText[dwi].szName,
						_T("Microphone")) == 0)
					{
						// found, dwi is the index.
						m_dwIndex = dwi;
						m_strMicName = pmxcdSelectText[dwi].szName;
						break;
					}
				}
			}
		}

		delete []pmxcdSelectText;
	}

	return m_dwIndex < m_dwMultipleItems;
}

BOOL CEasyRTMPDlg::amdGetMicSelectValue(LONG &lVal) const
{
	if (m_hMixer == NULL ||
		m_dwMultipleItems == 0 ||
		m_dwIndex >= m_dwMultipleItems)
	{
		return FALSE;
	}

	BOOL bRetVal = FALSE;

	MIXERCONTROLDETAILS_BOOLEAN *pmxcdSelectValue = new MIXERCONTROLDETAILS_BOOLEAN[m_dwMultipleItems];

	if (pmxcdSelectValue != NULL)
	{
		MIXERCONTROLDETAILS mxcd;
		mxcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
		mxcd.dwControlID = m_dwSelectControlID;
		mxcd.cChannels = 1;
		mxcd.cMultipleItems = m_dwMultipleItems;
		mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
		mxcd.paDetails = pmxcdSelectValue;
		if (::mixerGetControlDetails(reinterpret_cast<HMIXEROBJ>(m_hMixer),
			&mxcd,
			MIXER_OBJECTF_HMIXER |
			MIXER_GETCONTROLDETAILSF_VALUE)
			== MMSYSERR_NOERROR)
		{
			lVal = pmxcdSelectValue[m_dwIndex].fValue;
			bRetVal = TRUE;
		}

		delete []pmxcdSelectValue;
	}

	return bRetVal;
}

BOOL CEasyRTMPDlg::amdSetMicSelectValue(LONG lVal) const
{
	if (m_hMixer == NULL ||
		m_dwMultipleItems == 0 ||
		m_dwIndex >= m_dwMultipleItems)
	{
		return FALSE;
	}

	BOOL bRetVal = FALSE;

	// get all the values first
	MIXERCONTROLDETAILS_BOOLEAN *pmxcdSelectValue =
		new MIXERCONTROLDETAILS_BOOLEAN[m_dwMultipleItems];

	if (pmxcdSelectValue != NULL)
	{
		MIXERCONTROLDETAILS mxcd;
		mxcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
		mxcd.dwControlID = m_dwSelectControlID;
		mxcd.cChannels = 1;
		mxcd.cMultipleItems = m_dwMultipleItems;
		mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
		mxcd.paDetails = pmxcdSelectValue;
		if (::mixerGetControlDetails(reinterpret_cast<HMIXEROBJ>(m_hMixer),
			&mxcd,
			MIXER_OBJECTF_HMIXER |
			MIXER_GETCONTROLDETAILSF_VALUE)
			== MMSYSERR_NOERROR)
		{
			ASSERT(m_dwControlType == MIXERCONTROL_CONTROLTYPE_MIXER ||
				m_dwControlType == MIXERCONTROL_CONTROLTYPE_MUX);

			// MUX restricts the line selection to one source line at a time.
			if (lVal != 0 && m_dwControlType == MIXERCONTROL_CONTROLTYPE_MUX)
			{
				::ZeroMemory(pmxcdSelectValue,
					m_dwMultipleItems * sizeof(MIXERCONTROLDETAILS_BOOLEAN));
			}

			// set the Microphone value
			pmxcdSelectValue[m_dwIndex].fValue = lVal;

			mxcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
			mxcd.dwControlID = m_dwSelectControlID;
			mxcd.cChannels = 1;
			mxcd.cMultipleItems = m_dwMultipleItems;
			mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
			mxcd.paDetails = pmxcdSelectValue;
			if (::mixerSetControlDetails(reinterpret_cast<HMIXEROBJ>(m_hMixer),
				&mxcd,
				MIXER_OBJECTF_HMIXER |
				MIXER_SETCONTROLDETAILSF_VALUE)
				== MMSYSERR_NOERROR)
			{
				bRetVal = TRUE;
			}
		}

		delete []pmxcdSelectValue;
	}

	return bRetVal;
}
