#include "stdafx.h"
#include ".\FlashLiteView.h"

#ifndef UNDER_CE
	#include "NHCmnFuncForScrCtrl.h"
	#define Delay_Msg	Sleep
	#define Asc2Int				NH_Asc2Int
	#define GetCurrencySymbol	NH_GetCurrencySymbol
	#define SplitString			NH_SplitString
#endif

#define WM_FLASH_CREATE		(WM_APP+100)

//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------

//#define NH_DEBUG		// Debug Message On/Off
#include ".\Common\NHDbgApi.h"
#include ".\Common\ConstDef.h"	// [#2388] US Justin include Constant

#define DBG_CALL		0
#define DBG_INFO		1
#define DBG_AGING		1

//------------------------------------------------------------------
//	Define of Flash Lite 3.0
//------------------------------------------------------------------

#define WND_NAME			_T("Adobe Flash")
#define IDC_FLASH           1028


//------------------------------------------------------------------
//	Implementation
//------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CFlashLiteView, CWnd)
	ON_WM_CREATE()
	ON_WM_SETCURSOR()
	ON_WM_SETFOCUS()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_MESSAGE(WM_FLASH_CREATE, OnFlashCreate)
END_MESSAGE_MAP()


BEGIN_EVENTSINK_MAP(CFlashLiteView, CWnd)
	ON_EVENT(CFlashLiteView, IDC_FLASH, 102 /* FSCommand */, CFlashLiteView::OnFSCommandFlash, VTS_BSTR VTS_BSTR)
END_EVENTSINK_MAP()

/*-------------------------------------------------------------------
 CLASS    NAME: CFlashLiteView
 FUNCTION NAME: CFlashLiteView()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : constructor
-------------------------------------------------------------------*/
CFlashLiteView::CFlashLiteView(int x, int y, int width, int height) : CCanvasView(SCR_TYPE_FLASH_LITE)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	m_x = x;
	m_y = y;
	m_nWidth = width;
	m_nHeight = height;
	m_bInputEnable = FALSE;

	m_strAppName = WND_NAME;

	m_pNewApp = new CCanvasApp;

	m_pNewApp->SetWindowInfo(WND_NAME, this, m_x, m_y, m_nWidth, m_nHeight);
	m_pNewApp->CreateThread();

	m_dwLastShowTime = GetTickCount();
	m_eEndOfCreate.Wait(INFINITE);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CFlashLiteView
 FUNCTION NAME: ~CFlashLiteView()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : destructor
-------------------------------------------------------------------*/
CFlashLiteView::~CFlashLiteView()
{
	NHUIDBG(DBG_CALL, (_T("\n")));
}

/*-------------------------------------------------------------------
 CLASS    NAME: CFlashLiteView
 FUNCTION NAME: OnCreate()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
int CFlashLiteView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	NHUIDBG(DBG_CALL, (_T("TRY TO CREATE\n")));

	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	NHUIDBG(DBG_CALL, (_T("SUCCESSFUL\n")));

	m_hFrameWnd = GetParent()->GetSafeHwnd();
	m_hViewWnd = GetSafeHwnd();

	m_eEndOfCreate.Set();

	return 0;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CFlashLiteView
 FUNCTION NAME: Initialize()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CFlashLiteView::Initialize()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	CCanvasView::Initialize();

	m_eEndOfCreate.Reset();

	::PostMessage(m_hViewWnd, WM_FLASH_CREATE, 0, 0);

	m_eEndOfCreate.Wait(INFINITE);

	if (::IsWindow(m_FlashCtrl.m_hWnd))
		NHUIDBG(DBG_INFO, (_T("Flash Create Success..\n")));
	else
		NHUIDBG(DBG_INFO, (_T("Flash Create Failed..\n")));

	OnFSCommandFlash(_T("ENGINE"), _T("INIT_OK"));

	return TRUE;
}

LRESULT CFlashLiteView::OnFlashCreate(WPARAM wParam, LPARAM lParam)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (::IsWindow(m_FlashCtrl.m_hWnd) == FALSE)
	{
		if (m_FlashCtrl.Create(NULL, WS_VISIBLE | WS_CHILD, CRect(0, 0, m_nWidth+1, m_nHeight), this, IDC_FLASH) == FALSE)
		{
			NHERROR((_T("FLASH CONTROL CREATE FAILED..(%d)\n"), GetLastError()));
		}

		NHUIDBG(DBG_INFO, (_T("CREATE NEW~~\n")));
		m_eEndOfCreate.Set();
	}

	return 0;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CFlashLiteView
 FUNCTION NAME: Deinitialize()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CFlashLiteView::Deinitialize()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	if (::IsWindow(m_FlashCtrl.m_hWnd))
	{
		m_FlashCtrl.Stop();
		m_FlashCtrl.DestroyWindow();
		//m_FlashCtrl.PostMessage(WM_CLOSE);
		Delay_Msg(10);
	}

	if (m_hViewWnd != NULL)
	{
		m_eEndOfCreate.Reset();
		::PostMessage(m_hFrameWnd, WM_CLOSE, 0, 0);
		m_eEndOfCreate.Wait(INFINITE);
	}
}

/*-------------------------------------------------------------------
 CLASS    NAME: CFlashLiteView
 FUNCTION NAME: LoadScreens()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CFlashLiteView::LoadScreens(CString strPath, CString strSection, int nLoadWidth, int nLoadHeight)
{
	if (::IsWindow(m_FlashCtrl.m_hWnd))
	{
		NHUIDBG(DBG_INFO, (_T("Main.swf Load..\n")));

		if (strSection.GetLength() > 0)
			m_strFlashMainPath.Format(_T("%s\\%s"), strPath, strSection);
		else
			m_strFlashMainPath.Format(_T("%s"), strPath);

		NHUIDBG(DBG_INFO, (_T("LOAD SECTION : [%s]\n"), m_strFlashMainPath));

		//::MoveWindow(m_FlashCtrl.m_hWnd, m_x, m_y, m_nWidth, m_nHeight, FALSE);

		m_FlashCtrl.LoadMovie(0, m_strFlashMainPath);
		m_FlashCtrl.SetVariable(_T("APFlashPath"), m_strFlashMainPath);

		::MoveWindow(m_FlashCtrl.m_hWnd, m_x, m_y, m_nWidth, m_nHeight, TRUE);

		NHUIDBG(DBG_INFO, (_T("WAIT TO END OF LOAD\n")));

		m_eEndOfCreate.Wait(INFINITE);
	}
	else
	{
		NHUIDBG(DBG_INFO, (_T("FLASH PLAYER NOT CREATED.\n")));
		OnFSCommandFlash(_T("ENGINE"), _T("LOAD_FAIL"));
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CFlashLiteView
 FUNCTION NAME: SendVariable()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CFlashLiteView::SendVariable(CString strCommand, CString strData)
{
	NHUIDBG(DBG_CALL, (_T("Command(%s), Data(%s)\n"), strCommand, strData));

	if (strCommand == AP_CENTER_MOVIE || strCommand == PREV_SET_SCREEN_NO)
	{
		if (strCommand == PREV_SET_SCREEN_NO)
		{
			DWORD	m_dwDiffShowTime = GetTickCount() - m_dwLastShowTime;
			m_bInputEnable = FALSE;

			if (m_dwDiffShowTime < 1000)
				m_eWaitObject.Wait(1000-m_dwDiffShowTime);
		}
		else if (strCommand == AP_CENTER_MOVIE)
		{
			m_eEndOfCreate.Reset();
		}

		m_FlashCtrl.SetVariable(strCommand, strData);
		m_eWaitObject.Wait(1);

		if (strCommand == PREV_SET_SCREEN_NO && strData == _T("101"))
		{
			NHUIDBG(DBG_INFO, (_T("Send APFlashPath : [%s]\n"), m_strFlashMainPath));
			m_FlashCtrl.SetVariable(_T("APFlashPath"), m_strFlashMainPath);
			m_eWaitObject.Wait(1);
		}
		else if (strCommand == AP_CENTER_MOVIE)
		{
			//NHUIDBG(DBG_AGING, (_T("WAIT TO LOAD_OK FOR (%s)\n"), strData));
			
			m_eEndOfCreate.Wait(INFINITE);

			MakeTopMostWindow();
			m_dwLastShowTime = GetTickCount();

			m_eWaitObject.Wait(50);
			m_bInputEnable = TRUE;
		}
	}
	else if (strCommand.Left(7) == AP_VALUE)
	{
		int				i, nSetIndex;
		CStringArray	strTempArray;
		CString			strValueName;

		nSetIndex = Asc2Int(strCommand.Mid(7));
		if (nSetIndex <= 0)
			nSetIndex = 1;

		// Split String.
		SplitString(strData, SCR_CMD_DELIMITER, strTempArray);

		for (i = 0; i < strTempArray.GetSize(); i++, nSetIndex++)
		{
			strValueName.Format(_T("%s%d"), AP_VALUE, nSetIndex);
			m_FlashCtrl.SetVariable(strValueName, strTempArray[i]);
			m_eWaitObject.Wait(1);
		}
	}
	else if (strCommand.Left(7) == AP_STATE)
	{
		int				i, nSetIndex;
		CStringArray	strTempArray;
		CString			strValueName;

		nSetIndex = Asc2Int(strCommand.Mid(7));
		if (nSetIndex <= 0)
			nSetIndex = 1;

		// Split String.
		SplitString(strData, SCR_CMD_DELIMITER, strTempArray);

		for (i = 0; i < strTempArray.GetSize(); i++, nSetIndex++)
		{
			strValueName.Format(_T("%s%d"), AP_STATE, nSetIndex);
			m_FlashCtrl.SetVariable(strValueName, strTempArray[i]);
			m_eWaitObject.Wait(1);
		}
	}
	else
	{
		m_FlashCtrl.SetVariable(strCommand, strData);
		m_eWaitObject.Wait(1);
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CFlashLiteView
 FUNCTION NAME: SendKeyEvent()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CFlashLiteView::SendKeyEvent(CString strKey)
{
	NHUIDBG(DBG_CALL, (_T("strKey(%s)\n"), strKey));

	BYTE	VirtualKeyCode = 0x0;
	int		nKeyLen = strKey.GetLength();

	if (nKeyLen == 1)
	{
		BYTE	nHex = (BYTE)strKey[0];
		
		// 0 ~ 9
		if (nHex >= 48 && nHex <= 57)
			VirtualKeyCode = nHex;
		// <- --> I '.' is 46
		else if (nHex == 46)
			VirtualKeyCode = 73;
	}
	else if (nKeyLen == 2)
	{
		BYTE	nHex = (BYTE)strKey[1];

		// -> --> J
		if (nHex == 48)
			VirtualKeyCode = 74;
		// F1-F8 --> A-H
		else if (nHex >= 49 && nHex <=56)
			VirtualKeyCode = nHex + 16;
	}
	else
	{
		// ENTER
		if (strKey == _T("ENTER"))
			VirtualKeyCode = VK_RETURN;
		// CANCEL
		else if (strKey == _T("CANCEL"))
			VirtualKeyCode = VK_ESCAPE;
		// CLEAR
		else if (strKey == _T("CLEAR"))
			VirtualKeyCode = VK_DELETE;
		// CAPS
		else if (strKey == _T("000"))
			VirtualKeyCode = 76;
	}

	NHUIDBG(DBG_INFO, (_T("Key Convert for Flash Lite : [%s] -> [%c][%d]\n"), strKey, (char)VirtualKeyCode, (int)VirtualKeyCode));

	if (VirtualKeyCode != 0 && m_bInputEnable == TRUE)
	{
		mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN, m_x+500, m_y, 0, 0);
		mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTUP, m_x+500, m_y, 0, 0);
		keybd_event(VirtualKeyCode, 0, KEYEVENTF_KEYUP, 0);
		m_eWaitObject.Wait(100);
		return TRUE;
	}

	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CFlashLiteView
 FUNCTION NAME: OnFSCommandFlash()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CFlashLiteView::OnFSCommandFlash(LPCTSTR pCmd, LPCTSTR pData)
{
	NHUIDBG(DBG_CALL, (_T("Cmd(%s), Data(%s)\n"), pCmd, pData));

	if (_tcscmp(pData, _T("LOAD_OK")) == 0)
	{
		//NHUIDBG(DBG_AGING, (_T("RECEIVE LOAD_OK(%s)\n"), pCmd));
		m_eEndOfCreate.Set();
	}
	else if (_tcscmp(pCmd, _T("DING")) == 0)
	{
#if defined(_WIN32_WCE)
		sndPlaySound(L"\\ATM\\DING.WAV", SND_ASYNC);
#endif
	}
	else if (_tcscmp(pCmd, _T("HIDEWND")) == 0)
	{
		//::SetWindowPos(m_FlashCtrl.m_hWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
		//MakeBottomWindow();
	}
	else if (m_FSCmdQueue.Enqueue(CQueueData(pCmd, pData)) == FALSE)
	{
		NHERROR((_T("failed in Enquene() for Key. ScreenType(%d), command(%s) args(%s)\n"), (int)m_eScreenType, pCmd, pData));
	}
}

/*-------------------------------------------------------------------
 CLASS    NAME: CFlashLiteView
 FUNCTION NAME: OnSetCursor()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CFlashLiteView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
#ifndef APP_LOCAL_MODE
	return __super::OnSetCursor(pWnd, nHitTest, message);
#else
	SetCursor(NULL);
	return TRUE;
#endif
}

/*-------------------------------------------------------------------
 CLASS    NAME: CFlashLiteView
 FUNCTION NAME: OnSetFocus()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CFlashLiteView::OnSetFocus(CWnd* pOldWnd)
{
	NHUIDBG(DBG_CALL, (_T("pOldWnd(0x%08X)\n"), pOldWnd));

	__super::OnSetFocus(pOldWnd);

	if (::IsWindow(m_FlashCtrl.m_hWnd))
	{
		m_FlashCtrl.SetFocus();
		m_FlashCtrl.SetActiveWindow();
	}
}

BOOL CFlashLiteView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Add your specialized code here and/or call the base class

	cs.style &= ~WS_BORDER;

	return __super::PreCreateWindow(cs);
}

int	CFlashLiteView::GetScreenCount()
{
	return 0;
}

CString CFlashLiteView::GetFirstScreenNumber()
{
	return (_T(""));
}

CString CFlashLiteView::GetNextScreenNumber()
{
	return (_T(""));
}

void CFlashLiteView::OnClose()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	// TODO: Add your message handler code here and/or call default
	__super::OnClose();
}

void CFlashLiteView::OnDestroy()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	__super::OnDestroy();

	// TODO: Add your message handler code here
	PostQuitMessage(0);
	m_eEndOfCreate.Set();
}
