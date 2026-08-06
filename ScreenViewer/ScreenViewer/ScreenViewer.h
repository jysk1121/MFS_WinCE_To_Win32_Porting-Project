// ScreenViewer.h : main header file for the SCREENVIEWER application
//

#if !defined(AFX_SCREENVIEWER_H__0A33BF71_AA81_46EB_AA52_D5FA97B4B1D6__INCLUDED_)
#define AFX_SCREENVIEWER_H__0A33BF71_AA81_46EB_AA52_D5FA97B4B1D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CScreenViewerApp:
// See ScreenViewer.cpp for the implementation of this class
//

class CScreenViewerApp : public CWinApp
{
public:
	CScreenViewerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CScreenViewerApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CScreenViewerApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCREENVIEWER_H__0A33BF71_AA81_46EB_AA52_D5FA97B4B1D6__INCLUDED_)
