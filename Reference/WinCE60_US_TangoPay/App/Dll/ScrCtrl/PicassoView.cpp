#include "stdafx.h"
#include "PicassoView.h"
//#include ".\Dll\ScreenControlType.h"

//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------

//#define NH_DEBUG		// Debug Message On/Off
#include ".\Common\NHDbgApi.h"

#define DBG_CALL		1
#define DBG_INFO		1

//------------------------------------------------------------------
//	Define of Picasso
//------------------------------------------------------------------

#define WND_NAME			_T("NH Picasso")
#define IDC_PICASSO         2028

CAssetArchive* g_pTarScreenAsset = NULL;			// global variable used in ScrCtrl
//------------------------------------------------------------------
//	Implementation
//------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CPicassoView, CWnd)
	ON_WM_CREATE()
	ON_WM_SETCURSOR()
	ON_WM_SETFOCUS()
	ON_MESSAGE(WM_FS_COMMAND_MSG, OnFSCommand)
	ON_WM_DESTROY()
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoView
 FUNCTION NAME: CPicassoView()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : constructor
-------------------------------------------------------------------*/
CPicassoView::CPicassoView(int x, int y, int width, int height) : CCanvasView(SCR_TYPE_PICASSO)
{
	NHUIDBG(DBG_CALL, (_T("x(%d), y(%d), width(%d), height(%d)\n"), x, y, width, height));

	m_x = x;
	m_y = y;
	m_nWidth = width;
	m_nHeight = height;

	m_strAppName = WND_NAME;
	g_pTarScreenAsset = NULL;

	//CCanvasApp *m_pNewApp = new CCanvasApp();
	m_pNewApp = new CCanvasApp();

#ifdef UNDER_CE
	m_pNewApp->SetWindowInfo(WND_NAME, this, m_x, m_y, m_nWidth, m_nHeight);
#else
	CString strWndName;
	strWndName.Format(_T("%d_%d"), width, height);
	m_pNewApp->SetWindowInfo(strWndName, this, m_x, m_y, m_nWidth, m_nHeight);
#endif
	m_pNewApp->CreateThread();

	m_eEndOfCreate.Wait(INFINITE);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoView
 FUNCTION NAME: ~CPicassoView()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : destructor
-------------------------------------------------------------------*/
CPicassoView::~CPicassoView()
{
	NHUIDBG(DBG_CALL, (_T("~CPicassoView()\n")));
	Deinitialize();

}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoView
 FUNCTION NAME: OnCreate()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
int CPicassoView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	NHUIDBG(DBG_CALL, (_T("TRY TO CREATE\n")));

	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	NHUIDBG(DBG_CALL, (_T("SUCCESSFUL\n")));

	m_PicassoCtrl.Create(NULL, NULL, WS_VISIBLE | WS_CHILD, CRect(m_x, m_y, m_nWidth, m_nHeight), this, IDC_PICASSO);

	//m_hFrameWnd = GetParent()->m_hWnd;
	m_hFrameWnd = GetParent()->GetSafeHwnd();
	//m_hViewWnd = this->m_hWnd;
	m_hViewWnd = GetSafeHwnd();

	m_eEndOfCreate.Set();

	return 0;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoView
 FUNCTION NAME: Initialize()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoView::Initialize()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	CCanvasView::Initialize();

	if (::IsWindow(m_PicassoCtrl.m_hWnd))
	{
		::MoveWindow(m_PicassoCtrl.m_hWnd, m_x, m_y, m_nWidth, m_nHeight, FALSE);

		NHUIDBG(DBG_CALL, (_T("Start\n")));
		m_PicassoCtrl.Initialize(m_nWidth, m_nHeight);
		NHUIDBG(DBG_CALL, (_T("End\n")));
	}

	m_FSCmdQueue.Enqueue(CQueueData(_T("ENGINE"), _T("INIT_OK")));

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoView
 FUNCTION NAME: CPicassoView()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoView::Deinitialize()
{
	HideWindow();		// I wanna destroy but not working... hide it as workaround
	
	if (m_pNewApp) {
		m_pNewApp->SuspendThread();		// to suppress random exceptions while unloading
		m_pNewApp->ExitInstance();

		delete m_pNewApp;
		m_pNewApp = NULL;
	}

	if (g_pTarScreenAsset)
	{
		g_pTarScreenAsset->CloseArchive();
		g_pTarScreenAsset = NULL;
	}

	NHUIDBG(DBG_CALL, (_T("\n")));
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoView
 FUNCTION NAME: LoadScreens()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoView::LoadScreens(CString strPath, CString strSection, int nLoadWidth, int nLoadHeight)
{
	NHDBG((L"Load Screen %s\r\n", strPath));
	// we can't make a new interface to retrieve a TAR asset from mainframe for compatibility, (KeyMgr will be failed to open, which uses old ScrCtrl.h/lib.)
	// so open TAR itself.
	if (strSection != _T("UPDATE"))		// UPDATE have to skip this to avoid sharing violation with SCREEN.TAR.
	{
		g_pTarScreenAsset = CAssetArchive::GetInstance();
	}

	CScrMsg ApMsg;
	ApMsg.m_strCmd = _T("LOAD_SCREEN");
	ApMsg.m_strData = strPath;
	ApMsg.m_strData2 = strSection;

	m_PicassoCtrl.SetLoadScreenSize(nLoadWidth, nLoadHeight);

	return m_PicassoCtrl.SetVariable(ApMsg);
	
	//return m_PicassoCtrl.LoadScreens(strPath, strSection, nLoadWidth, nLoadHeight);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoView
 FUNCTION NAME: SendVariable()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoView::SendVariable(CString strCommand, CString strData)
{
	NHUIDBG(DBG_CALL, (_T("strCommand(%s), strData(%s)\n"), strCommand, strData));

	BOOL bRes = m_PicassoCtrl.SetVariable(strCommand, strData);

	if (bRes == TRUE && strCommand == AP_CENTER_MOVIE)
		MakeTopMostWindow();

	return bRes;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoView
 FUNCTION NAME: OnFSCommand()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
LRESULT CPicassoView::OnFSCommand(WPARAM wParam, LPARAM lParam)
{
	CString strCmd((LPCTSTR)wParam);
	CString strData((LPCTSTR)lParam);

	NHUIDBG(DBG_CALL, (_T("strCmd(%s), strData(%s)\n"), strCmd, strData));

	if (_tcscmp(strCmd, _T("DING")) == 0)
	{
#if defined(_WIN32_WCE)
		sndPlaySound(ATM_PATH L"\\DING.WAV", SND_ASYNC);
#endif
	}
	else if (_tcscmp(strCmd, _T("HIDEWND")) == 0)
	{
		if (m_bEnableHideWnd == TRUE)
			m_FSCmdQueue.Enqueue(CQueueData(strCmd, strCmd));

		//TEST
		MakeBottomWindow();
	}
	else if (m_FSCmdQueue.Enqueue(CQueueData(strCmd, strData)) == FALSE)
	{
		NHERROR((_T("failed in Enquene() for Key. ScreenType(%d), command(%s) args(%s)\n"), (int)m_eScreenType, strCmd, strData));
	}

	return 0;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoView
 FUNCTION NAME: SendKeyEvent()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoView::SendKeyEvent(CString strKey)
{
	NHUIDBG(DBG_CALL, (_T("strKey(%s)\n"), strKey));

	if (strKey.GetLength() != 1 || strKey == _T("."))
	{
		if (strKey == _T("."))
			strKey = DES_STAR;
		else if (strKey == _T("00"))
			strKey = DES_SHARP;
		else if (strKey == _T("000"))
			strKey = DES_CAPS;
	}

	return m_PicassoCtrl.SetVariable(DES_EPP_KEY, strKey);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoView
 FUNCTION NAME: OnSetFocus()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoView::OnSetFocus(CWnd* pOldWnd)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	__super::OnSetFocus(pOldWnd);

	if (::IsWindow(m_PicassoCtrl.m_hWnd))
	{
		m_PicassoCtrl.SetFocus();
		m_PicassoCtrl.SetActiveWindow();
	}
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoView
 FUNCTION NAME: OnSetCursor()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoView::OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT message)
{
#ifndef APP_LOCAL_MODE
	return __super::OnSetCursor(pWnd, nHitTest, message);
#else
	#ifndef UNDER_CE
		return __super::OnSetCursor(pWnd, nHitTest, message);
	#else
		SetCursor(NULL);
		return TRUE;
	#endif
#endif
}

void CPicassoView::PostNcDestroy()
{
	delete this;
}

void CPicassoView::OnDestroy()
{
	NHUIDBG(DBG_CALL, (_T("\n")));
	__super::OnDestroy();
}

void CPicassoView::OnKillFocus(CWnd* pNewWnd)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	__super::OnKillFocus(pNewWnd);

	// TODO: Add your message handler code here
}

int	CPicassoView::GetScreenCount()
{
	//return m_PicassoCtrl.m_PicassoScreens.GetDataCount();
	return m_PicassoCtrl.m_PicassoScreensCMap.GetCount();
}

CString CPicassoView::GetFirstScreenNumber()
{
	CPicassoScreen	*pScreen = NULL;

	//m_PicassoCtrl.m_PicassoScreens.MoveFirst(&pScreen);
	//if (pScreen != NULL)
	//	return pScreen->GetID();

	//return _T("");
	pos = m_PicassoCtrl.m_PicassoScreensCMap.GetStartPosition();

	CString key;
	m_PicassoCtrl.m_PicassoScreensCMap.GetNextAssoc(pos, key, pScreen);

	return key;


}

CString CPicassoView::GetNextScreenNumber()
{
	CPicassoScreen	*pScreen = NULL;

	//m_PicassoCtrl.m_PicassoScreens.MoveNext(&pScreen, 1);
	//if (pScreen != NULL)
	//	return pScreen->GetID();

	//return _T("");

	CString key;
	m_PicassoCtrl.m_PicassoScreensCMap.GetNextAssoc(pos, key, pScreen);

	return key;

}