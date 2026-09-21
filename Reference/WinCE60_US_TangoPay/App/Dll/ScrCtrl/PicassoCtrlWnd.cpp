#include "stdafx.h"
//#include "ScreenEngine.h"
#include ".\PicassoCtrl.h"

//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------

//#define NH_DEBUG		// Debug Message On/Off
//#define NH_DEBUG_SHOW_SCREEN_DRAW_TIME

#include "..\..\H\Common\NHDbgApi.h"

#define DBG_CALL		1
#define DBG_INFO		1
#define DBG_TEST		1


//------------------------------------------------------------------
//	Implementation
//------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CPicassoCtrl, CWnd)
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_DESTROY()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()

#ifdef NH_DEBUG_SHOW_SCREEN_DRAW_TIME
long long milliseconds_now() {
	static LARGE_INTEGER s_frequency;
	static BOOL s_use_qpc = QueryPerformanceFrequency(&s_frequency);
	if (s_use_qpc) {
		LARGE_INTEGER now;
		QueryPerformanceCounter(&now);
		return (1000LL * now.QuadPart) / s_frequency.QuadPart;
	} else {
		return GetTickCount();
	}
}
#endif

HBRUSH	gRedBrush = NULL;
/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: OnPaint()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoCtrl::OnPaint()
{
#ifdef NH_DEBUG_SHOW_SCREEN_DRAW_TIME
	long long startTime = milliseconds_now();
#endif

	NHUIDBG(DBG_TEST, (_T("\n")));

	CPaintDC dc(this); // device context for painting

	if (m_bInit == FALSE)
		return;

	if (m_bmBackBuffer == NULL)
	{
		NHUIDBG(DBG_TEST, (_T("m_bmBackBuffer is NULL!! (%d).\n"), GetLastError()));
		return;
	}

	if (m_bmScreenBuffer == NULL)
	{
		NHUIDBG(DBG_TEST, (_T("m_bmScreenBuffer is NULL!! (%d).\n"), GetLastError()));
		return;
	}

	HDC hBufDC = ::CreateCompatibleDC(dc.GetSafeHdc());
	if (hBufDC != NULL)
	{
		CNHAutoLock	AutoSync(m_csNHProcess);

		RECT rcScreenRect = {0, 0, m_nScreenWidth, m_nScreenHeight};
		RECT rcUpdate = dc.m_ps.rcPaint;

		BOOL	bDrawed = FALSE;
		HGDIOBJ hOldScr = ::SelectObject(hBufDC, m_bmScreenBuffer);

		// 화면을 그린다
		if (m_pDisplayScreen != NULL)
		{
			NHUIDBG(DBG_TEST, (_T("Refresh(%d), BackDrew(%d).\n"), m_bBackRefresh, m_bBackDrawed));

			////////////////////////////////////
			// Draw Background Image on screen buffer.
			HDC hBgrDC = ::CreateCompatibleDC(dc.GetSafeHdc());
			if (hBgrDC != NULL)
			{
				HGDIOBJ hOldBgr = ::SelectObject(hBgrDC, m_bmBackBuffer);

				//RETAILMSG(1, (_T("OnPaint() EVENT : Refresh(%d), left(%d), top(%d), right(%d), bottom(%d) \n"),
				//	m_bBackRefresh, rcUpdate.left, rcUpdate.top, rcUpdate.right, rcUpdate.bottom));

				// 배경 Refresh and 이전에 그려놓은 것과 다른 배경일때
				if ((m_pDisplayScreen->GetBackScreenID().GetLength() > 0) &&
					((m_strBackID != m_pDisplayScreen->GetBackScreenID()) ||(m_pDisplayScreen != m_pCurrentScreen)) &&
					(m_bDecodeCash == FALSE))
				{
					int	i;
					CString strNewBackID = m_pDisplayScreen->GetBackScreenID();

					// 1. Search
					for (i = 0; i < MAX_CACHE_SCREEN; i++)
					{
						if ((m_eCacheScreens[i].hBitmap != NULL) &&
							(m_eCacheScreens[i].strID == strNewBackID))
						{
							HDC	hCacheDC = ::CreateCompatibleDC(dc.GetSafeHdc());
							HGDIOBJ hOldCacheBgr = ::SelectObject(hCacheDC, m_eCacheScreens[i].hBitmap);

							::BitBlt(hBgrDC, 0, 0, m_nScreenWidth, m_nScreenHeight, hCacheDC, 0, 0, SRCCOPY);

							::SelectObject(hCacheDC, hOldCacheBgr);
							::DeleteDC(hCacheDC);

							NHUIDBG(DBG_TEST, (_T("Cache Background Drew. index(%d)\n"), i));
							m_bBackDrawed = TRUE;
							break;
						}
					}

					// 2. Draw New.
					if (i >= MAX_CACHE_SCREEN)
					{
						FillRect(hBgrDC, &rcScreenRect, (HBRUSH)::GetStockObject(WHITE_BRUSH));
						m_bBackDrawed = m_pDisplayScreen->DrawBackground(hBgrDC, m_nCurrentLocale, &rcScreenRect);
						NHUIDBG(DBG_TEST, (_T("OnPaint() : New Background Drew. \n")));
					}

					m_strBackID = m_pDisplayScreen->GetBackScreenID();
					m_strScreenID = m_pDisplayScreen->GetID();
				}

				// Draw on Screen.
				if ((m_bBackRefresh == TRUE && m_pDisplayScreen->GetBackScreenID().GetLength() > 0) || (m_bBackDrawed == TRUE))
				{
					::BitBlt(hBufDC, rcUpdate.left, rcUpdate.top, (rcUpdate.right - rcUpdate.left), (rcUpdate.bottom - rcUpdate.top),	
							 hBgrDC, rcUpdate.left, rcUpdate.top, 
							 SRCCOPY);
					bDrawed |= TRUE;

					NHUIDBG(DBG_TEST, (_T("OnPaint() : Background copied.. Refresh(%d), BackDrew(%d).\n"), m_bBackRefresh, m_bBackDrawed));
					m_bBackDrawed = FALSE;
					m_bBackRefresh = FALSE;
				}

				::SelectObject(hBgrDC, hOldBgr);
				::DeleteDC(hBgrDC);
			}

			////////////////////////////////////
			// Draw Screen Image on screen buffer
			if (m_pDisplayScreen->Draw(hBufDC, m_nCurrentLocale, &rcUpdate) == TRUE)
				bDrawed |= TRUE;

			////////////////////////////////////
			// Mode Message on screen buffer
			if (m_strModeMsg.GetLength() != 0)
			{
				int			OldBkMode;
				HFONT		OldFont = NULL;
				COLORREF	OldTextColor;
				RECT		rcDemo = {5, 0, m_nScreenWidth, 72};

				// Select to Transparent Mode in Background Mode
				OldBkMode = ::SetBkMode(hBufDC, TRANSPARENT);

				// Select Font
				if (m_hLocalFont)
					OldFont = (HFONT)::SelectObject(hBufDC, m_hLocalFont);

				// Set Text Color
				OldTextColor = ::SetTextColor(hBufDC, RGB(255,0,0));

				// Draw..
				::DrawText(hBufDC, (LPCTSTR)m_strModeMsg, -1, &rcDemo, DT_CENTER | DT_SINGLELINE | DT_VCENTER);	

				// Restore Text Color
				::SetTextColor(hBufDC, OldTextColor);

				// Restore Font
				if (m_hLocalFont)
					::SelectObject(hBufDC, OldFont);

				// Restore Background Mode
				::SetBkMode(hBufDC, OldBkMode);
			}

#ifdef NH_DEBUG_SHOW_SCREEN_DRAW_TIME
			long long drawTime = milliseconds_now();
			m_strDebugMsg.Format(_T("%lli ms"), drawTime - startTime);
#endif

			////////////////////////////////////
			// Debug Message on screen buffer
			if (m_strDebugMsg.GetLength() != 0)
			{
				int			OldBkMode;
				COLORREF	OldTextColor;

				// Select to Transparent Mode in Background Mode
				OldBkMode = ::SetBkMode(hBufDC, TRANSPARENT);
				
				// Set Text Color
				OldTextColor = ::SetTextColor(hBufDC, RGB(255,0,0));
				
				// Draw..
				::ExtTextOut(hBufDC, 0, 0, 0, NULL, m_strDebugMsg, m_strDebugMsg.GetLength(), NULL);
				
				// Restore Text Color
				::SetTextColor(hBufDC, OldTextColor);
				
				// Restore Background Mode
				::SetBkMode(hBufDC, OldBkMode);
			}
		}

		////////////////////////////////////
		// Draw screen from buffer to Device
		if (bDrawed == TRUE)
		{
			// Draw Screen
			::BitBlt(dc.m_hDC, rcUpdate.left, rcUpdate.top, (rcUpdate.right - rcUpdate.left), (rcUpdate.bottom - rcUpdate.top),	
					 hBufDC,   rcUpdate.left, rcUpdate.top, 
					 SRCCOPY);

			if (0)
			{
				int jj;
				int	left, top, right, bottom;

				if (gRedBrush == NULL)
					gRedBrush = CreateSolidBrush(RGB(255,0,0));

				HBRUSH	OldBrush = (HBRUSH)::SelectObject(dc.m_hDC, gRedBrush);

				for (jj = 0; jj < m_nPointCount; jj++)
				{
					left = m_arPoint[jj].x - 2;
					right = m_arPoint[jj].x + 2;
					top = m_arPoint[jj].y - 2;
					bottom = m_arPoint[jj].y + 2;
	
					Ellipse(dc.m_hDC, left, top, right, bottom);
				}

				::SelectObject(dc.m_hDC, OldBrush);
			}

			NHUIDBG(DBG_TEST, (_T("***SCR*** Current Screen Locale(%3d) Update(%3d, %3d, %3d, %3d)\n"),
								m_nCurrentLocale, rcUpdate.left, rcUpdate.top, rcUpdate.right, rcUpdate.bottom));
		}

		::SelectObject(hBufDC, hOldScr);
		::DeleteDC(hBufDC);
	}

}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: OnTimer()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoCtrl::OnTimer(UINT_PTR nIDEvent)
{
	int	nTimerID = nIDEvent;
			
	if (nTimerID != UM_BLINKING_TIME)
		::KillTimer(this->m_hWnd, nTimerID);

	// 완전히 로딩된 후에.
	if (m_pCurrentScreen == NULL || m_pNextScreen != NULL)
		return;

#ifndef UNDER_CE		// for screen viewer reloading
	if (!m_isWorkingThread)
		return;
#endif

	// Blinking 
	if (nTimerID == UM_BLINKING_TIME)
	{
		if (m_pDisplayScreen == m_pCurrentScreen)
			m_VariableQueue.Enqueue(CScrMsg(DES_BLINKING, _T("")));
	}
#ifdef UNDER_CE
	// Advertisement.
	else if (nTimerID == UM_ADV_SHOW_TIME)
	{
		if (m_AdvNoticeWelcomeInfo.bEnable == FALSE)
			return;

		CPicassoScreen *pCurScreen = m_AdvNoticeWelcomeInfo.pScreen[m_AdvNoticeWelcomeInfo.nCurIndex];
		CPicassoScreen *pShowScreen;

		if (pCurScreen != NULL)
			pShowScreen = pCurScreen;
		else
			pShowScreen = m_pCurrentScreen;

		{
			CNHAutoLock	AutoSync(m_csNHProcess);
			m_pDisplayScreen = pShowScreen;
		}

		m_AdvNoticeWelcomeInfo.nCurIndex = AdvertiseGetNextScreen();

		// Draw
		::InvalidateRect(this->m_hWnd, NULL, TRUE);
		::SetTimer(this->m_hWnd, UM_ADV_SHOW_TIME, (m_AdvertiseInfo.nRefreshTime * 1000), NULL);
	}
#endif //UNDER_CE
	// occur time out.
	else if (nTimerID >= UM_TIMER_ID_BASE)
	{
		if ((nTimerID - UM_TIMER_ID_BASE) == (m_pCurrentScreen->GetUniqueNumber()))
		{
			//RETAILMSG(1, (L"TIME OVER, SCR(%s), UniqueID(%d)\n", m_pCurrentScreen->GetID(), (nTimerID - UM_TIMER_ID_BASE)));
			m_VariableQueue.Enqueue(CScrMsg(DES_EPP_KEY, DES_TIMEOVER));
		}
	}

	__super::OnTimer(nIDEvent);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: OnEraseBkgnd()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CPicassoCtrl::OnEraseBkgnd(CDC* pDC)
{
	NHUIDBG(DBG_TEST, (_T("[CPicassoCtrl::OnEraseBkgnd] m_pDisplayScreen(%08X), m_pCurrentScreen(%08X)\n"), m_pDisplayScreen, m_pCurrentScreen));

	if (m_pDisplayScreen == m_pCurrentScreen)
		m_bBackRefresh = TRUE;

	return __super::OnEraseBkgnd(pDC);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: OnLButtonDown()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	NHUIDBG(DBG_CALL, (_T("Point(%d, %d)\n"), point.x, point.y));

	if (0)
	{
		int jj;
		for (jj = 0; jj < m_nPointCount; jj++)
		{
			if (m_arPoint[jj] == point)
				break;
		}

		if (jj >= m_nPointCount && m_nPointCount < 1024)
		{
			m_arPoint[m_nPointCount++] = point;

			RECT	rcRect =  {point.x - 2, point.y - 2, point.x + 2, point.y + 2};

			if (gRedBrush == NULL)
				gRedBrush = CreateSolidBrush(RGB(255,0,0));

			CDC *pDC = GetDC();
			HBRUSH	OldBrush = (HBRUSH)::SelectObject(pDC->m_hDC, gRedBrush);

			::Ellipse(pDC->m_hDC, rcRect.left, rcRect.top, rcRect.right, rcRect.bottom);

			::SelectObject(pDC->m_hDC, OldBrush);
			ReleaseDC(pDC);
		}
	}

	m_VariableQueue.Enqueue(CScrMsg(_T("MouseKey"), point.x, point.y));

	//__super::OnLButtonDown(nFlags, point);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: OnLButtonDblClk()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	NHUIDBG(DBG_TEST, (_T("Point(%d, %d)\n"), point.x, point.y));

	if (0)
	{
		int jj;
		for (jj = 0; jj < m_nPointCount; jj++)
		{
			if (m_arPoint[jj] == point)
				break;
		}

		if (jj >= m_nPointCount && m_nPointCount < 1024)
		{
			m_arPoint[m_nPointCount++] = point;

			RECT	rcRect =  {point.x - 2, point.y - 2, point.x + 2, point.y + 2};

			if (gRedBrush == NULL)
				gRedBrush = CreateSolidBrush(RGB(255,0,0));

			CDC *pDC = GetDC();
			HBRUSH	OldBrush = (HBRUSH)::SelectObject(pDC->m_hDC, gRedBrush);

			::Ellipse(pDC->m_hDC, rcRect.left, rcRect.top, rcRect.right, rcRect.bottom);

			::SelectObject(pDC->m_hDC, OldBrush);
			ReleaseDC(pDC);
		}
	}

	m_VariableQueue.Enqueue(CScrMsg(_T("MouseKey"), point.x, point.y));
}


/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: OnRButtonDown()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoCtrl::OnRButtonDown(UINT nFlags, CPoint point)
{
	NHUIDBG(DBG_CALL, (_T("Point(%d, %d)\n"), point.x, point.y));

	SendFSCommand(_T("HIDEWND"), _T(""));

	//__super::OnLButtonDown(nFlags, point);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: OnKeyDown()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	NHUIDBG(DBG_CALL, (_T("Char(%c)\n"), (char)nChar));

	//__super::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CPicassoCtrl::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	NHUIDBG(DBG_CALL, (_T("Char(%c)\n"), (char)nChar));

#ifndef UNDER_CE
	
	CString strKey;

	int nVerKey = (int)nChar;

	if ((nVerKey == VK_RETURN || nVerKey == VK_BACK || nVerKey == VK_DELETE ||
		 nVerKey == VK_LEFT || nVerKey == VK_RIGHT || nVerKey == VK_CAPITAL) ||
		(nVerKey >= VK_F1 && nVerKey <= VK_F8) ||
		(nVerKey >= '0' && nVerKey <= '9') ||
		(nVerKey >= VK_NUMPAD0 && nVerKey <= VK_NUMPAD9))
	{
		CString strKey;
		
		if (nVerKey == VK_RETURN)
			strKey = _T("ENTER");
		else if (nVerKey == VK_BACK)
			strKey = _T("CANCEL");
		else if (nVerKey == VK_DELETE)
			strKey = _T("CLEAR");
		else if (nVerKey == VK_LEFT)
			strKey = DES_STAR;  //_T(".");
		else if (nVerKey == VK_RIGHT)
			strKey = DES_SHARP; //_T("00");
		else if (nVerKey == VK_CAPITAL)
			strKey = DES_CAPS; //_T("000");
		else if (nVerKey >= VK_F1 && nVerKey <= VK_F8)
		{
			nVerKey -= VK_F1;
			strKey.Format(_T("F%d"), nVerKey+1);
		}
		else if (nVerKey >= '0' && nVerKey <= '9')
			strKey.Format(_T("%c"), nVerKey);
		else
		{
			nVerKey -= VK_NUMPAD0;
			strKey.Format(_T("%d"), nVerKey);
		}

		m_VariableQueue.Enqueue(CScrMsg(DES_EPP_KEY, strKey));
	}


#endif
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: OnDestroy()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoCtrl::OnDestroy()
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	__super::OnDestroy();

	for (int i = 0; i < MAX_CACHE_SCREEN; i++)
	{
		m_eCacheScreens[i].strID = L"";
		if (m_eCacheScreens[i].hBitmap != NULL)
		{
			::DeleteObject(m_eCacheScreens[i].hBitmap);
			m_eCacheScreens[i].hBitmap = NULL;
		}
	}

	if (m_bmBackBuffer != NULL)
	{
		::DeleteObject(m_bmBackBuffer);
		m_bmBackBuffer = NULL;
	}

	if (m_bmScreenBuffer != NULL)
	{
		::DeleteObject(m_bmScreenBuffer);
		m_bmScreenBuffer = NULL;
	}
}

void CPicassoCtrl::OnKillFocus(CWnd* pNewWnd)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	__super::OnKillFocus(pNewWnd);

	// TODO: Add your message handler code here
}

void CPicassoCtrl::OnSetFocus(CWnd* pOldWnd)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	__super::OnSetFocus(pOldWnd);

	// TODO: Add your message handler code here
}
