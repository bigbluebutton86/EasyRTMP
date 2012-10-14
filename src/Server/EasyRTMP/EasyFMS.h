// EasyFMS.h : main header file for the EASYFMS application
//

#if !defined(AFX_EASYFMS_H__66ADB3FB_68D5_4B39_8540_276D170356DA__INCLUDED_)
#define AFX_EASYFMS_H__66ADB3FB_68D5_4B39_8540_276D170356DA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CEasyRTMPApp:
// See EasyFMS.cpp for the implementation of this class
//

class CEasyRTMPApp : public CWinApp
#ifdef _USING_BCGP_
	,
	public CBCGPWorkspace
#endif
{
public:
	CEasyRTMPApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEasyRTMPApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CEasyRTMPApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EASYFMS_H__66ADB3FB_68D5_4B39_8540_276D170356DA__INCLUDED_)
