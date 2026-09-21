#include "stdafx.h"
#include "CanvasView.h"


//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------

//#define NH_DEBUG		// Debug Message On/Off
#include ".\Common\NHDbgApi.h"

#define DBG_CALL		1
#define DBG_INFO		1
#define DBG_TEST		1

//#define EVERY_TIME_TOPMOST

//------------------------------------------------------------------
//	Set Initial Value for static variable
//------------------------------------------------------------------
//BOOL	CCanvasView::m_bEnableHideWnd = FALSE;
BOOL	CCanvasView::m_bEnableHideWnd = TRUE;

//------------------------------------------------------------------
//	Implementation
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CCanvasView
 FUNCTION NAME: CCanvasView()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : constructor
-------------------------------------------------------------------*/
CCanvasView::CCanvasView(SCREEN_TYPE type) : m_eEndOfCreate(FALSE, FALSE), m_eWaitObject(FALSE, FALSE)
{	
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_eScreenType = type;
	m_bInit = FALSE;
	m_x = m_y = m_nWidth = m_nHeight = 0;
	m_hFrameWnd = NULL;
	m_hViewWnd = NULL;
	m_bSetTopMostWnd = FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CCanvasView
 FUNCTION NAME: ~CCanvasView()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : destructor
-------------------------------------------------------------------*/
CCanvasView::~CCanvasView()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (m_bInit == TRUE)
		Deinitialize();

	m_hFrameWnd = NULL;
	m_hViewWnd = NULL;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CCanvasView
 FUNCTION NAME: Initialize()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CCanvasView::Initialize()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	::SetWindowPos(m_hViewWnd, HWND_TOP, m_x, m_y, m_nWidth, m_nHeight, SWP_SHOWWINDOW);
	::SetWindowPos(m_hFrameWnd, HWND_TOP, m_x, m_y, m_nWidth, m_nHeight, SWP_SHOWWINDOW);
	m_bInit = TRUE;

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CCanvasView
 FUNCTION NAME: Deinitialize()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CCanvasView::Deinitialize()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_bInit = FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CCanvasView
 FUNCTION NAME: LoadScreens()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CCanvasView::LoadScreens(CString strPath, CString strSection, int nLoadWidth, int nLoadHeight)
{
	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CCanvasView
 FUNCTION NAME: MoveScreen()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CCanvasView::MoveScreen(int x, int y, int width, int height, BOOL bMakeTopMost)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_x = x;
	m_y = y;
	m_nWidth = width;
	m_nHeight = height;


	m_bSetTopMostWnd = TRUE;

	if (bMakeTopMost)
		MakeTopMostWindow();

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CCanvasView
 FUNCTION NAME: SetActiveScreen()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CCanvasView::SetActiveScreen()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

#ifndef UNDER_CE
	// move chosen window in front of others (screen viewer)
	::SetForegroundWindow(m_hFrameWnd);
	m_bSetTopMostWnd = TRUE;
	return;
#endif

	::SetFocus(m_hFrameWnd);

	m_bSetTopMostWnd = TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CCanvasView
 FUNCTION NAME: MakeTopMostWindow()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CCanvasView::MakeTopMostWindow()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (m_bSetTopMostWnd == TRUE)
	{
#ifdef UNDER_CE
		::SetWindowPos(m_hFrameWnd, HWND_TOPMOST, m_x, m_y, m_nWidth, m_nHeight, SWP_SHOWWINDOW);
//		::SetWindowPos(m_hFrameWnd, HWND_NOTOPMOST, m_x, m_y, m_nWidth, m_nHeight, SWP_SHOWWINDOW);		// TOP MOST 제거 TEST용
		m_eWaitObject.Wait(30);
#else
		int nBorderThickness = GetSystemMetrics(SM_CXSIZEFRAME) * 2;
		nBorderThickness += 4;	// required for accurate window size, but where it comes from?

		::SetWindowPos(m_hFrameWnd, HWND_NOTOPMOST, m_x, m_y, m_nWidth + nBorderThickness, m_nHeight + nBorderThickness + GetSystemMetrics(SM_CYCAPTION), SWP_SHOWWINDOW);
#endif

		m_bSetTopMostWnd = FALSE;
	}
}

/*-------------------------------------------------------------------
 CLASS    NAME: CCanvasView
 FUNCTION NAME: SetEnableHideWindow()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CCanvasView::SetEnableHideWindow(BOOL bEnable)
{
	m_bEnableHideWnd = bEnable;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CCanvasView
 FUNCTION NAME: HideWindow()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CCanvasView::HideWindow()
{
	MakeBottomWindow();

#ifndef UNDER_CE
	ShowWindow(m_hFrameWnd, SW_HIDE);		// hide from taskbar
#endif
}

/*-------------------------------------------------------------------
 CLASS    NAME: CCanvasView
 FUNCTION NAME: GetScreenType()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
SCREEN_TYPE CCanvasView::GetScreenType()
{
	return m_eScreenType;
}

#ifndef UNDER_CE
RECT CCanvasView::GetScreenRect()
{
	RECT rect;
	::GetWindowRect(m_hFrameWnd, &rect);

	return rect;
}
#endif

/*-------------------------------------------------------------------
 CLASS    NAME: CCanvasView
 FUNCTION NAME: MakeBottomWindow()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CCanvasView::MakeBottomWindow()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	::SetWindowPos(m_hFrameWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);

#ifdef UNDER_CE
	m_eWaitObject.Wait(30);
#endif
}

/*-------------------------------------------------------------------
 CLASS    NAME: CCanvasView
 FUNCTION NAME: SetVariable()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CCanvasView::SetVariable(CString strCommand, CString strData)
{
	NHUIDBG(DBG_CALL, (_T("[%s] Cmd(%s), Data(%s)\n"), m_strAppName, strCommand, strData));

	if (m_bInit == FALSE || m_hViewWnd == NULL)
	{
		NHERROR((_T("didn't initialize or ViewWnd is NULL. must be call Initialize(), ViewWnd(%d)\n"), m_hViewWnd));
		return FALSE;
	}

	if (strCommand == AP_CENTER_MOVIE)
	{
		NHUIDBG(DBG_INFO, (_T("FSCommand Queue is cleared.\n")));
		ClearUserInputData();

		m_strCurrentScreenNumber = strData;
	}

	BOOL bRes = SendVariable(strCommand, strData);
	return bRes;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CCanvasView
 FUNCTION NAME: SetKeyEvent()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CCanvasView::SetKeyEvent(CString strKey)
{
	NHUIDBG(DBG_CALL, (_T("strKey(%s)\n"), strKey));
	NHUIDBG(DBG_TEST, (_T("strKey(%s)\n"), strKey));

	if (m_bInit == FALSE || m_hViewWnd == NULL)
	{
		NHERROR((_T("didn't initialize or ViewWnd is NULL. must be call Initialize(), ViewWnd(%d)\n"), m_hViewWnd));
		return FALSE;
	}

	if (m_FSCmdQueue.IsHasData(0) == TRUE)
	{
		NHERROR((_T("FSCmdQueue has data. so Data is ignored. ScreenType(%d), strKey(%s)\n"), (int)m_eScreenType, strKey));
		return FALSE;
	}

	return SendKeyEvent(strKey);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CCanvasView
 FUNCTION NAME: SetByPassData()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CCanvasView::SetByPassData(CString strData)
{
	NHUIDBG(DBG_CALL, (_T("strData(%s)\n"), strData));

	if (m_bInit == FALSE || m_hViewWnd == NULL)
	{
		NHERROR((_T("didn't initialize or ViewWnd is NULL. must be call Initialize(), ViewWnd(%d)\n"), m_hViewWnd));
		return FALSE;
	}

#if !(AU_VERSION)	// [#2069] NH KSK 2011.06.13
	if (m_FSCmdQueue.IsHasData(0) == TRUE)
	{
		NHERROR((_T("FSCmdQueue has data. so Data is ignored. ScreenType(%d), strData(%s)\n"), (int)m_eScreenType, strData));
		return FALSE;
	}
#endif

	CString strByPassData;
	strByPassData.Format(_T("%s%s"), AP_BYPASS, strData);

	if (m_FSCmdQueue.Enqueue(CQueueData(AP_BYPASS, strByPassData)) == FALSE)
	{
		NHERROR((_T("failed in Enquene() for bypass. ScreenType(%d), strData(%s)\n"), (int)m_eScreenType, strData));
		return FALSE;
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CCanvasView
 FUNCTION NAME: GetUserInputCheck()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CCanvasView::GetUserInputCheck(DWORD dwTimeout /*= 20*/)
{
	if (m_bInit == FALSE)
	{
		NHERROR((_T("didn't initialize. must be call Initialize()\n")));
		return FALSE;
	}

	return m_FSCmdQueue.IsHasData(dwTimeout);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CCanvasView
 FUNCTION NAME: GetUserInputData()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CCanvasView::GetUserInputData(CString &strCommand, CString &strData)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (m_bInit == FALSE)
	{
		NHERROR((_T("didn't initialize. must be call Initialize()\n")));
		return FALSE;
	}

	CQueueData	Data;
	if (m_FSCmdQueue.Dequeue(Data, 0) == FALSE)
	{
		NHERROR((_T("failed in Dequeue() for user input. ScreenType(%d)\n"), (int)m_eScreenType));
		return FALSE;
	}

	strCommand = Data.m_strCmd;
	strData = Data.m_strData;

	NHUIDBG(DBG_INFO, (TEXT("%s >> : Return data is strCommand(%s), strData(%s)\n"), TEXT(__FUNCTION__), strCommand, strData));

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CCanvasView
 FUNCTION NAME: ClearUserInputData()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CCanvasView::ClearUserInputData()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_FSCmdQueue.Clear();
}

/*-------------------------------------------------------------------
 CLASS    NAME: CCanvasView
 FUNCTION NAME: GetCurrentScreenNumber()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
CString	CCanvasView::GetCurrentScreenNumber()
{
	return m_strCurrentScreenNumber;
}