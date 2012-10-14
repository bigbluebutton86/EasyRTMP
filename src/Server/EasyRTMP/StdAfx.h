// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__FCE74057_E7F0_4C8D_B316_141703412742__INCLUDED_)
#define AFX_STDAFX_H__FCE74057_E7F0_4C8D_B316_141703412742__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <WINSOCK2.H>
#include <afxsock.h>		// MFC socket extensions

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
#pragma warning(disable:4996)
//GuiLib
//#include <GuiLib2008/GuiLib.h>
//#include <GuiLib2008/GuiListCtrl.h>//this can not work now.
//#include <GuiLib2008/GuiHeaderCtrl.h>

#ifdef _USING_BCGP_
#include <BCGCBProInc.h>			// BCGControlBar Pro
#endif

#include <mmsystem.h>

#endif // !defined(AFX_STDAFX_H__FCE74057_E7F0_4C8D_B316_141703412742__INCLUDED_)
