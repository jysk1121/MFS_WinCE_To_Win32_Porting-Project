// update.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "update.h"
#include "UpdateStateManager.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CupdateApp
CUpdateStateManager	g_StateMgr;

// CupdateApp construction
CUpdateApp::CUpdateApp() : CWinApp()
{
}


// The one and only CupdateApp object
CUpdateApp theApp;

// CupdateApp initialization

BOOL CUpdateApp::InitInstance()
{
#ifdef _DEBUG
	_CrtSetReportMode(_CRT_ASSERT,0);
#endif

	g_StateMgr.Initialize(::AfxGetInstanceHandle());
	g_StateMgr.Run();

	return FALSE;
}
