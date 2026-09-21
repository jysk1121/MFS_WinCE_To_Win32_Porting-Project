#include "stdafx.h"
#include "CanvasApp.h"

#include "resource1.h"

//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------

//#define NH_DEBUG		// Debug Message On/Off
#include ".\Common\NHDbgApi.h"

#define	DBG_CALL		1
#define DBG_INFO		1


//------------------------------------------------------------------
//	Implementation
//------------------------------------------------------------------

IMPLEMENT_DYNCREATE(CCanvasApp, CWinThread)

/*-------------------------------------------------------------------
 CLASS    NAME: CCanvasApp
 FUNCTION NAME: CCanvasApp()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : constructor
-------------------------------------------------------------------*/
CCanvasApp::CCanvasApp()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_pWnd = NULL;
	m_x = 0;
	m_y = 0;
	m_nWidth = 0;
	m_nHeight = 0;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CCanvasApp
 FUNCTION NAME: CCanvasApp()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : destructor
-------------------------------------------------------------------*/
CCanvasApp::~CCanvasApp()
{
	NHUIDBG(DBG_CALL, (_T("\n")));
}

/*-------------------------------------------------------------------
 CLASS    NAME: CCanvasApp
 FUNCTION NAME: SetWindowInfo()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : set information for window size
-------------------------------------------------------------------*/
void CCanvasApp::SetWindowInfo(CString strName, CWnd *pWnd, int x, int y, int width, int height)
{
	NHUIDBG(DBG_CALL, (_T("Name(%s), pWnd(0x%08X), x(%d), y(%d), width(%d), height(%d)\n"), strName, pWnd, x, y, width, height));

	m_strAppName = strName;
	m_pWnd = pWnd;
	m_x = x;
	m_y = y;
	m_nWidth = width;
	m_nHeight = height;
}
/*
BOOL CCanvasApp::OnIdle(LONG lCount)
{
	if (lCount <= 0)
	{
		CWinThread::OnIdle(lCount);

		// call doc-template idle hook
		POSITION pos = NULL;
		if (m_pDocManager != NULL)
			pos = m_pDocManager->GetFirstDocTemplatePosition();

		while (pos != NULL)
		{
			CDocTemplate* pTemplate = m_pDocManager->GetNextDocTemplate(pos);
			ASSERT_KINDOF(CDocTemplate, pTemplate);
			pTemplate->OnIdle();
		}
	}
	else if (lCount == 1)
	{
		VERIFY(!CWinThread::OnIdle(lCount));
	}
	return lCount < 1;  // more to do if lCount < 1
}
*/

/*-------------------------------------------------------------------
 CLASS    NAME: CCanvasApp
 FUNCTION NAME: InitInstance()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : create frame wnd
-------------------------------------------------------------------*/
BOOL CCanvasApp::InitInstance()
{
	NHUIDBG(DBG_CALL, (TEXT("NAME(%s) WND(0x%08X)\n"), m_strAppName, m_pWnd));
	
	m_bAutoDelete = TRUE;
	AfxEnableControlContainer();

	pFrame = new CCanvasFrame(m_strAppName, m_pWnd, m_x, m_y, m_nWidth, m_nHeight);

	if (!pFrame)
		return FALSE;

	NHUIDBG(DBG_INFO, (_T("NAME(%s) WND(0x%08X) FRAME(0x%08X) THARED(%08X)\n"), m_strAppName, m_pWnd, pFrame, GetCurrentThreadId()));

#ifdef UNDER_CE
	if (pFrame->LoadFrame(IDR_MAINFRAME, WS_OVERLAPPEDWINDOW, NULL, NULL) == FALSE)
		::MessageBox(NULL, _T("FAILED TO CREATE FRAME WND"), _T(""), MB_OK);
#else
	if (pFrame->Create(NULL, NULL) == FALSE)
		::MessageBox(NULL, _T("FAILED TO CREATE FRAME WND"), _T(""), MB_OK);
#endif
	
	NHUIDBG(DBG_INFO, (_T("Frame Wnd Created.\n")));

#ifndef UNDER_CE
	// Screen Viewer, remove title bar
//	SetWindowLong(pFrame->GetSafeHwnd(), GWL_STYLE, WS_BORDER);
#endif

	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();

	return TRUE;
}

int CCanvasApp::ExitInstance()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (pFrame) {
		delete pFrame;
		pFrame = NULL;
	}

	return __super::ExitInstance();
}
