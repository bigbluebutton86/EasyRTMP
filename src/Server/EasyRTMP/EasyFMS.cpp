// EasyFMS.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "EasyFMS.h"
#include "EasyFMSDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEasyRTMPApp

BEGIN_MESSAGE_MAP(CEasyRTMPApp, CWinApp)
	//{{AFX_MSG_MAP(CEasyRTMPApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEasyRTMPApp construction

CEasyRTMPApp::CEasyRTMPApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
#ifdef _USING_BCGP_
	globalData.m_bUseVisualManagerInBuiltInDialogs = TRUE;
#endif
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CEasyRTMPApp object

CEasyRTMPApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CEasyRTMPApp initialization

BOOL CEasyRTMPApp::InitInstance()
{
#ifdef _USING_BCGP_
	globalData.SetDPIAware ();
#endif

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	CoInitialize(NULL);

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.
/*
	LOGFONT lf; //lf定义字体属性 
	lf.lfOutPrecision	= OUT_STROKE_PRECIS; 
	lf.lfClipPrecision	= CLIP_STROKE_PRECIS; 
	lf.lfQuality		= DRAFT_QUALITY; 
	lf.lfPitchAndFamily	= VARIABLE_PITCH|FF_MODERN; 

	lf.lfHeight			= 10; 
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
	//m_wndMenuBar.SetMenuFont(&lf,TRUE); 
	globalData.SetMenuFont(&lf, TRUE);
	
//	CFont vFont;
//	vFont.CreatePointFontIndirect(&lf);
	//globalData.fontRegular.Attach(vFont.GetSafeHandle());
	//globalData.fontRegular.CreateFontIndirect(&lf);
	//globalData.fontRegular.FromHandle(vFont.GetSafeHandle());
*/

#ifdef _USING_BCGP_
	InitContextMenuManager ();
#endif

	CEasyRTMPDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
#ifdef _USING_BCGP_
	BCGCBProCleanUp ();
#endif

	return FALSE;
}
