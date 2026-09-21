// FlashViewer.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#ifdef STANDARDSHELL_UI_MODEL
#include "resource.h"
#endif

// CFlashViewerApp:
// See FlashViewer.cpp for the implementation of this class
//

class CFlashViewerApp : public CWinApp
{
public:
	CFlashViewerApp();
	
// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CFlashViewerApp theApp;
