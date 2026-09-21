// update.h : main header file for the update application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"

// CUpdateApp:
// See update.cpp for the implementation of this class
//

class CUpdateApp : public CWinApp
{
public:
	CUpdateApp();

// Overrides
public:
	virtual BOOL InitInstance();
};

extern CUpdateApp theApp;
