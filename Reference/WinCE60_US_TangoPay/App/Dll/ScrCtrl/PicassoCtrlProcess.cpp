#include "stdafx.h"
#include "PicassoCtrl.h"

//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------

//#define NH_DEBUG		// Debug Message On/Off
#include ".\Common\NHDbgApi.h"
#include ".\Common\ConstDef.h"	// [#2388] US Justin include Constant

#define DBG_CALL		1
#define DBG_INFO		1
#define DBG_TEST		1


//------------------------------------------------------------------
//	Implementation
//------------------------------------------------------------------

UINT ThPicassoCtrl(LPVOID param)
{
	CPicassoCtrl* pPicassoCtrl = (CPicassoCtrl*)param;

	NHDBG((_T("ThPicassoCtrl() begins, THREAD(%08X)\n"), GetCurrentThreadId()));
	CScrMsg	ApMsg;

	while (pPicassoCtrl->m_isWorkingThread)
	{
		if (pPicassoCtrl->m_VariableQueue.Dequeue(ApMsg, 300))
		{
			if (ApMsg.m_strCmd == DES_EPP_KEY)
				pPicassoCtrl->ProcEppEvent(ApMsg.m_strData);
			else if (ApMsg.m_strCmd == _T("MouseKey"))
				pPicassoCtrl->ProcMouseEvent(ApMsg.m_Point);
			else if (ApMsg.m_strCmd == _T("LOAD_SCREEN"))
			{
				NHUIDBG(DBG_INFO, (_T("SCREEN LOAD [%s] START\n"), ApMsg.m_strData2));
				BOOL bRes = pPicassoCtrl->LoadSection(ApMsg.m_strData, ApMsg.m_strData2);

				// For Back Screens.
				if (ApMsg.m_strData2 == _T("FRONT_FUNCTION")
					|| ApMsg.m_strData2 == _T("FRONT_TOUCH")
					|| ApMsg.m_strData2 == _T("1800SE_FRONT_FUNCTION")
					|| ApMsg.m_strData2 == _T("1500_FRONT_FUNCTION")
					|| ApMsg.m_strData2 == _T("5200_FRONT_FUNCTION")	// [#2300] US KSK 2014.11.03	// [#2315] US Justin 2014.12.11 Name Change MX3000 => MX5200SE
					|| ApMsg.m_strData2 == _T("2800_FRONT_FUNCTION"))	// [#2518] US Kook 2018.01.18 Support MX-2800SE
				{
					CString strFileName;

					// [#2327] NH KSK 2015.01.24 Image 중복 방지를 위해 Back Image는 Backs 폴더를 참조하도록 수정
					strFileName.Format(_T("%s\\%d_%d\\Function\\Back_1.jpg"), SCREEN_PATH_BACKS, pPicassoCtrl->m_nScreenWidth, pPicassoCtrl->m_nScreenHeight);
					// end of [#2327]

					pPicassoCtrl->PrepareCacheScreen(strFileName);
				}

				if (bRes == TRUE)
					pPicassoCtrl->SendFSCommand(ApMsg.m_strData2, _T("LOAD_OK"));
				else
					pPicassoCtrl->SendFSCommand(ApMsg.m_strData2, _T("LOAD_FAIL"));

				NHUIDBG(DBG_INFO, (_T("SCREEN LOAD [%s] END - [%d]\n"), ApMsg.m_strData2, (int)bRes));
			}
			else
				pPicassoCtrl->ProcVariableEvent(ApMsg.m_strCmd, ApMsg.m_strData);
		}
		else
		{
			// nothing dequeued
		}
	}

	NHDBG((_T("ThPicassoCtrl() ends, THREAD(%08X)\n"), GetCurrentThreadId()));

	return 0;
}

void CPicassoCtrl::PrepareCacheScreen(CString strFileName)
{
	NHUIDBG(DBG_CALL, (_T("\n")));

	HDC				hDC;
	HDC				hBufDC = NULL;
	HDC				hBackDC = NULL;
	HGDIOBJ			hOldBufScr;
	HGDIOBJ			hOldBackScr;
	CString			strKey;
	RECT			rcScreenRect = {2, 2, m_nScreenWidth-2, m_nScreenHeight-2};
	
	m_bDecodeCash = TRUE;

	hDC = ::GetDC(m_hDisplayWnd);
	if (hDC != NULL)
	{
		// Create Background Buffer
		for (int i=0; i < MAX_CACHE_SCREEN; i++)
		{
			if (m_eCacheScreens[i].hBitmap == NULL)
				m_eCacheScreens[i].hBitmap = MakeDIBSection(hDC, m_nScreenWidth, m_nScreenHeight);
		}

		for (int i=0; i < MAX_CACHE_SCREEN; i++)
		{
			// Search Screen
			strKey.Format(_T("%d"), ID_CACHE_SCREEN + i);

			CPicassoScreen	*pScreen = NULL;
			m_PicassoScreensCMap.Lookup(strKey, pScreen);

			if (pScreen != NULL)
			{
				m_eCacheScreens[i].strID = pScreen->GetID();
				
				hBufDC = ::CreateCompatibleDC(hDC);

				// Message Processing
				{
					CPicassoRequest		Req;
					CString			strName, strData;

					pScreen->Initialize();

					if (i == 0)
					{
						// SET FILE NAME to AdvBox of ID_CACHE_SCREEN (199901) 
						Req.Set(REQ_SETVALUE, _T("APValue1"), strFileName);
						pScreen->AddRequest(Req);
						pScreen->ProcessRequest();
					}
				}

				if (m_eCacheScreens[i].hBitmap == NULL)
				{
					NHUIDBG(DBG_CALL, (_T("hBitmap is not prepared yet! #[%d]\n"), i));
					continue;
				}

				// Draw Screen
				if (i == 0)
				{
					hOldBackScr = ::SelectObject(hBufDC, m_eCacheScreens[i].hBitmap);
					::FillRect(hBufDC, &rcScreenRect, (HBRUSH)::GetStockObject(WHITE_BRUSH));
				}
				else
				{
					hOldBufScr = ::SelectObject(hBufDC, m_eCacheScreens[i].hBitmap);
					::BitBlt(hBufDC, 0, 0, m_nScreenWidth, m_nScreenHeight, hBackDC, 0, 0, SRCCOPY);
				}

				NHUIDBG(DBG_CALL, (_T("Draw cache screen #[%d], [%d], (%d, %d, %d, %d)\n"), i, m_nCurrentLocale, rcScreenRect.top, rcScreenRect.bottom, rcScreenRect.left, rcScreenRect.right));
				pScreen->Draw(hBufDC, m_nCurrentLocale, &rcScreenRect, FALSE);

				if (i == 0)
				{
					NHUIDBG(DBG_CALL, (_T("hbackdc = hbufdc #[%d]\n"), i));
					hBackDC = hBufDC;
				}
				else
				{
					NHUIDBG(DBG_CALL, (_T("Delete dc#[%d]\n"), i));

					::SelectObject(hBufDC, hOldBufScr);
					::DeleteDC(hBufDC);
				}
				
				NHUIDBG(DBG_INFO, (_T("CACHE SCREEN WAS DECODED.. (%s)\n"), m_eCacheScreens[i].strID));
			}
		}

		if (hBackDC != NULL)
		{
			::SelectObject(hBufDC, hOldBackScr);
			::DeleteDC(hBackDC);
		}

		::ReleaseDC(m_hDisplayWnd, hDC);
	}

	m_strBackID = L"";
	m_bDecodeCash = FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: SendFSCommand()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoCtrl::SendFSCommand(CString strCommand, CString strData)
{
	NHUIDBG(DBG_CALL, (_T("Command(%s), Data(%s)\n"), strCommand, strData));

	if (m_hParentWnd != NULL)
	{
		NHUIDBG(DBG_TEST, (_T("m_bInputEnabled (%d) TickCount(%d)\n"), m_bInputEnabled, GetTickCount()));

		if ((strCommand == _T("DING")) ||
			(strCommand == _T("HIDEWND")) ||
			(strData.Left(5) == _T("LOAD_")))
		{
			NHUIDBG(DBG_TEST, (_T("SEND FS COMMAND (%s)\n"), strData));
			::SendMessage(m_hParentWnd, WM_FS_COMMAND_MSG, (WPARAM)(LPCTSTR)strCommand, (LPARAM)(LPCTSTR)strData);
		}
		else if (m_bInputEnabled == TRUE)
		{
			NHUIDBG(DBG_TEST, (_T("SEND FS COMMAND (%s)\n"), strData));
			::SendMessage(m_hParentWnd, WM_FS_COMMAND_MSG, (WPARAM)(LPCTSTR)strCommand, (LPARAM)(LPCTSTR)strData);

			m_bInputEnabled = FALSE;
			NHUIDBG(DBG_TEST, (_T("FS COMMAND-INPUT DISABLED\n")));
		}
		else
		{
			NHUIDBG(DBG_TEST, (_T("IGNORE FS COMMAND (%s)\n"), strData));
		}
	}
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: ProcEppEvent()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoCtrl::ProcEppEvent(CString strKeyCode)
{
	CPicassoRequest	KeyEvent;
	RECT	UpdateRect = {0,};
	CPicassoScreen *pCurScreen = NULL;

	NHUIDBG(DBG_CALL, (_T("KeyCode(%s)\n"), strKeyCode));

	if (m_bInputEnabled == FALSE)
		return;

	// Is NULL ?
	if (strKeyCode.GetLength() <= 0 || !m_pCurrentScreen)
		return;

	// When timeout occur in Screen Timer,pCurScreen parameter is enabled
	// Screen Number Check. 
	if (pCurScreen)
	{
		// ignore
		if (pCurScreen != m_pCurrentScreen)
			return;
	}

	NHUIDBG(DBG_TEST, (_T("KeyCode(%s)\n"), strKeyCode));

	KeyEvent.Set(REQ_KEYEVENT, _T(""), strKeyCode);

	m_pCurrentScreen->ClearUpdateRect();
	m_pCurrentScreen->AddRequest(KeyEvent);
	
	m_pCurrentScreen->ProcessRequest();

	while (m_pCurrentScreen->GetFSCmd(KeyEvent))
	{
		SendFSCommand(KeyEvent.GetName(), KeyEvent.m_Data);
	}

	if (m_pCurrentScreen->GetUpdateRect(&UpdateRect))
	{
		NHUIDBG(1, (_T("***SCR*** DisplayKeyboardEvent-InvalidateRect\n")));
		UpdateScreen(&UpdateRect, TRUE);
	}

	// [#236] NZ AIREAT 2008.05.27
	if (m_TimeoutMode == SCR_TMODE_LAST && strKeyCode != DES_TIMEOVER)
	{
		CString	strCommand, strTimeout;

		strCommand = DES_AP_MAX_TIME;
		strTimeout.Format(_T("%d"), m_pCurrentScreen->GetScreenTimeout());

		// update timeout value
		SetApMaxTime(m_pCurrentScreen, strCommand, strTimeout);
	}
	// end of [#236]
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: ProcMouseEvent()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoCtrl::ProcMouseEvent(POINT &pos)
{
	CPicassoRequest	MouseEvent;
	RECT	UpdateRect;
	CPicassoScreen *pCurScreen = NULL;

	NHUIDBG(DBG_CALL, (_T("Pos(%d, %d)\n"), pos.x, pos.y));

	if (m_bInputEnabled == FALSE)
		return;

	// Is NULL ?
	if (!m_pCurrentScreen)
		return;

	// When timeout occur in Screen Timer,pCurScreen parameter is enabled
	// Screen Number Check. 
	if (pCurScreen)
	{
		// ignore
		if (pCurScreen != m_pCurrentScreen)
			return;
	}

	NHUIDBG(DBG_TEST, (_T("Pos(%d, %d) TickCount(%d)\n"), pos.x, pos.y, GetTickCount()));

	MouseEvent.Set(REQ_MOUSEEVENT, _T(""), _T(""));
	MouseEvent.m_Point = pos;

	m_pCurrentScreen->ClearUpdateRect();
	m_pCurrentScreen->AddRequest(MouseEvent);
	
	m_pCurrentScreen->ProcessRequest();
	
	while (m_pCurrentScreen->GetFSCmd(MouseEvent))
	{
		SendFSCommand(MouseEvent.GetName(), MouseEvent.m_Data);
	}

	if (m_pCurrentScreen->GetUpdateRect(&UpdateRect))
	{
		NHUIDBG(1, (_T("***SCR*** DisplayKeyboardEvent-InvalidateRect\n")));
		UpdateScreen(&UpdateRect, TRUE);
	}

	if (m_TimeoutMode == SCR_TMODE_LAST)
	{
		CString	strCommand, strTimeout;

		strCommand = DES_AP_MAX_TIME;
		strTimeout.Format(_T("%d"), m_pCurrentScreen->GetScreenTimeout());

		// update timeout value
		SetApMaxTime(m_pCurrentScreen, strCommand, strTimeout);
	}
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: ProcVariableEvent()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoCtrl::ProcVariableEvent(CString strCmd, CString strData)
{
	if (strCmd != DES_BLINKING)
	{
		CString commaData = strData;
		commaData.Replace('\r', ',');
		NHUIDBG(DBG_CALL, (_T("Command(%s), Data(%s)\n"), strCmd, commaData));
	}

	CString		strScreenNumber;
	CPicassoScreen	*pScreen = NULL;

	// _T("PrevSetScrrenNo")
	if (strCmd == DES_PREV_SET_SCREEN_NO)
	{
		int	nScrNum = _ttoi(strData);
		if (nScrNum != 0)
		{
			strScreenNumber.Format(_T("%03d"), nScrNum);
			PrepareScreen(strScreenNumber, TRUE);
		}
		else
		{
			PrepareScreen(strData, TRUE);
		}

		NHUIDBG(DBG_TEST, (_T("PREPARE SCREEN(%s)\n"), strData));
		m_bInputEnabled = FALSE;
		NHUIDBG(DBG_TEST, (_T("SCREEN-INPUT DISABLED\n")));
	}
	// _T("APCenterMovie")
	else if (strCmd == DES_AP_CENTER_MOVIE)
	{
		int	nScrNum = _ttoi(strData);
		if (nScrNum != 0)
		{
			strScreenNumber.Format(_T("%03d"), nScrNum);
			PrepareScreen(strScreenNumber, FALSE);
		}
		else
		{
			PrepareScreen(strData, FALSE);
		}

		NHUIDBG(DBG_TEST, (_T("ThreadID(0x%08X) VIEW(0x%08X)--SHOW SCREEN(%s)\n"), GetCurrentThreadId(), m_hDisplayWnd, strData));
		m_bInputEnabled = TRUE;
		NHUIDBG(DBG_TEST, (_T("SCREEN-INPUT ENABLE\n")));

		// PIN 모드에서 화면 LOADING 후 FSCOMMAND 발행.
//		if (m_pCurrentScreen != NULL)
//		{
//			SendFSCommand(m_pCurrentScreen->GetID(), _T("LOAD_OK"));
//		}
	}
	else
	{
		if (m_pNextScreen)
			pScreen = m_pNextScreen;
		else
			pScreen = m_pCurrentScreen;
		
		if (!pScreen)
			return;

		// BLINKING
		if (_tcscmp(strCmd, DES_BLINKING) == 0)
		{
			if (m_bInputEnabled == TRUE)
				ChangeBlinking(pScreen, strCmd, strData);

			return;
		}
		// APValueXX
		else if (_tcsncmp(strCmd, DES_AP_VALUE, 7) == 0)
		{
			SetValueState(pScreen, strCmd, strData, TRUE);
		}
		// APStatusXX
		else if (_tcsncmp(strCmd, DES_AP_STATE, 7) == 0)
		{
			SetValueState(pScreen, strCmd, strData, FALSE);
		}
		// APMaxTime
		else if (_tcscmp(strCmd, DES_AP_MAX_TIME) == 0)
		{
			SetApMaxTime(pScreen, strCmd, strData);
		}
		// [#236] NZ AIREAT 2008.05.27
		else if (_tcscmp(strCmd, DES_AP_TIMEOUT_MODE) == 0)
		{
			SetApTimeoutMode(pScreen, strCmd, strData);
		}
		// end of [#236]
		// APUpdate
		else if (_tcscmp(strCmd, DES_AP_UPDATE) == 0)
		{
			SetApUpdate(pScreen, strCmd, strData);
		}
		// SetFocus
		else if (_tcscmp(strCmd, DES_AP_SETFOCUS) == 0)
		{
			CPicassoRequest		Req;
			Req.Set(REQ_SETFOCUS, strData, _T(""));
			pScreen->AddRequest(Req);
		}
		// APAdv
		else if (_tcsncmp(strCmd, DES_AP_ADV, 5) == 0)
		{
			if (strCmd == DES_AP_ADV)
			{
				m_AdvertiseInfo.nEnableCount = Asc2Int(strData);
				NHUIDBG(DBG_INFO, (_T("ADVER nEableCount(%d)\n"), m_AdvertiseInfo.nEnableCount));
			}
			else
			{
				int	nIndex = Asc2Int(strCmd.Right(1));
				if (nIndex > 0 && nIndex <= SUPPORT_ADVERTISE)
				{
					m_AdvertiseInfo.bEnableAdv[nIndex-1] = TRUE;
					m_AdvertiseInfo.strFileName[nIndex-1] = strData;

					NHUIDBG(DBG_INFO, (_T("ADVER [%d] FileName[%s]\n"), nIndex-1, m_AdvertiseInfo.strFileName[nIndex-1]));
				}
			}
		}
		// APNotice
		else if (_tcsncmp(strCmd, DES_AP_NOTICE, 8) == 0)
		{
			int nIndex = Asc2Int(strCmd.Right(1));

			if (nIndex > 0 && nIndex <= SUPPORT_NOTICE)
			{
				m_NoticeInfo.strMsg[nIndex-1] = strData;
				m_NoticeInfo.bEnable = TRUE;

				NHUIDBG(DBG_INFO, (_T("NOTICE [%d] - [%s]\n"), nIndex-1, m_NoticeInfo.strMsg[nIndex-1]));
			}
		}
		// APSwitchTime
		else if (_tcscmp(strCmd, DES_AP_SWITCH_TIME) == 0)
		{
			m_AdvertiseInfo.nRefreshTime = Asc2Int(strData);
		}
		// kinLanguage
		else if (_tcscmp(strCmd, DES_KIND_LANGUAGE) == 0)
		{
			if (strData == _T("ENGLISH"))
				m_nCurrentLocale = ENG_MODE;
			else if (strData == _T("SPANISH"))
				m_nCurrentLocale = SPN_MODE;
			else if (strData == _T("FRENCH"))
				m_nCurrentLocale = FRN_MODE;
			// [#2186] US KMK 2013.04.08 Asian Language Support
			else if (strData == _T("CHINESE"))		
				m_nCurrentLocale = CHN_MODE;
			else if (strData == _T("KOREAN"))
				m_nCurrentLocale = KOR_MODE;
			else if (strData == _T("JAPANESE"))
				m_nCurrentLocale = JPN_MODE;
			// end of [#2186]
		}
		else if (_tcsncmp(strCmd, DES_AP_WEATHER, 9) == 0 ||
				 _tcsncmp(strCmd, DES_AP_TRANADV, 9) == 0)
		{
			SetOtherSetting(pScreen, strCmd, strData);
		}
		else if (_tcscmp(strCmd, DES_BACK_IMAGE) == 0)
		{
			PrepareCacheScreen(strData);
		}
		// APLocalMovie
		else if (_tcscmp(strCmd, _T("APLocalMovie")) == 0)
		{
			m_strModeMsg = strData;
			return;
		}
		// APDebugMsg
		else if (_tcscmp(strCmd, _T("APDebugMsg")) == 0)
		{
			m_strDebugMsg = strData;
			return;
		}
		else
		{
			return;
		}
		
		if (pScreen)
		{
			pScreen->ProcessRequest();

			CPicassoRequest		Req;
			while (pScreen->GetFSCmd(Req))
			{
				SendFSCommand(Req.GetName(), Req.m_Data);
			}
		}
	}
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: PrepareScreen()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoCtrl::PrepareScreen(CString strScreenNumber, BOOL PrevSetScreen)
{
	CPicassoScreen	*pScreen = NULL;

	//
	// _T("PrevSetScrrenNo") -> find scrren -> save screen -> Data setting.
	if (PrevSetScreen)
	{
		// Search Screen by screen number
		//if (m_PicassoScreens.FindDataOfList(strScreenNumber, &pScreen))
		if (m_PicassoScreensCMap.Lookup(strScreenNumber, pScreen) == TRUE)
		{
			m_pNextScreen = pScreen;

			// Clear Timer
			//::KillTimer(m_hDisplayWnd, (UM_TIMER_ID_BASE + _ttoi(m_pNextScreen->GetID())));
			::KillTimer(m_hDisplayWnd, (UM_TIMER_ID_BASE + pScreen->GetUniqueNumber()));
			
			// Initialize.
			m_pNextScreen->Initialize();
			AdvertiseDeinitialize();
		}
	}
	//
	// _T("APCenterMovie") -> find screen. or saved screen -> screen show.
	else
	{
		// find screen.
		{
			// is it saved screen?
			if (m_pNextScreen)
			{
				// if right screen, use saved screen.
				if (m_pNextScreen->GetID() == strScreenNumber)
					pScreen = m_pNextScreen;
				
				m_pNextScreen = NULL;
			}
			
			// new search screen.
			if (!pScreen)
			{
				//if (m_PicassoScreens.FindDataOfList(strScreenNumber, &pScreen))
				if (m_PicassoScreensCMap.Lookup(strScreenNumber, pScreen) == TRUE)
				{
					// Clear Timer
					//::KillTimer(m_hDisplayWnd, (UM_TIMER_ID_BASE + _ttoi(pScreen->GetID())));
					::KillTimer(m_hDisplayWnd, (UM_TIMER_ID_BASE + pScreen->GetUniqueNumber()));

					// Initialize.
					pScreen->Initialize();
					m_TimeoutMode = SCR_TMODE_STOP;
				}
			}
		}

		// update..
		if (pScreen)
		{
			{
				CNHAutoLock	AutoSync(m_csNHProcess);

				CPicassoScreen	*pPrevScreen = NULL;

				pPrevScreen = m_pCurrentScreen;
				m_pCurrentScreen = pScreen;	

				if (pPrevScreen != NULL)
					pPrevScreen->Deinitialize();
			}

			UpdateScreen(NULL, TRUE);

#ifdef UNDER_CE

			// Lanch Advertisement
			// [#2351] US Justin 2015.06.30 AddOnAP Welcome Screen (ATM + AddOnAP)
			if (m_pCurrentScreen->GetID() == _T("100") && m_AdvertiseInfo.bEnable == FALSE)		// [#2351] US Justin 2015.06.30 AddOnAP Welcome Screen
			{
				AdvertiseInitialize();
			}	// End of [#2351]
			else if (m_pCurrentScreen->GetID() == _T("101") && m_AdvertiseInfo.bEnable == FALSE)
			{
				AdvertiseInitialize();
			}
			else if (m_pCurrentScreen->GetID() == _T("300") && m_AdvertiseInfo.bEnable == FALSE)
			{
				AdvertiseInitialize();
			}	// end of [#2011]
			else
			{
				AdvertiseDeinitialize();						// 광고가 진행중이더라고 새로운 화면이면 광고는 종료한다.
			}
#endif // UNDER_CE
		}
		else
		{
			m_pCurrentScreen = NULL;
		}
	}

	return;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: SetValueState()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoCtrl::SetValueState(CPicassoScreen *pScreen, CString &strCommand, CString &strData, BOOL bValueType)
{
	int				i, nTypeLen = 0, nSetIndex = 0;
	CString			strValueName;
	CStringArray	strTempArray;
	CPicassoRequest		Req;

	// Get Type Length.
	if (bValueType)
		nTypeLen = _tcslen(DES_AP_VALUE);
	else
		nTypeLen = _tcslen(DES_AP_STATE);

	// Get Start Index
	nSetIndex = Asc2Int(strCommand.Mid(nTypeLen));

	if (nSetIndex <= 0)
		nSetIndex = 1;

	// Split String.
	SplitString(strData, SCR_CMD_DELIMITER, strTempArray);

	for (i = 0; i < strTempArray.GetSize(); i++, nSetIndex++)
	{
		if (bValueType)
			strValueName.Format(_T("%s%d"), DES_AP_VALUE, nSetIndex);
		else
			strValueName.Format(_T("%s%d"), DES_AP_STATE, nSetIndex);

		Req.Set(REQ_SETVALUE, strValueName, strTempArray[i]);

		// Add Request.
		pScreen->AddRequest(Req);
	}
}

void CPicassoCtrl::SetOtherSetting(CPicassoScreen *pScreen, CString &strCommand, CString &strData)
{
	CPicassoRequest		Req;

	Req.Set(REQ_SETVALUE, strCommand, strData);

	// Add Request.
	pScreen->AddRequest(Req);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: SetApMaxTime()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoCtrl::SetApMaxTime(CPicassoScreen *pScreen, CString &strCommand, CString &strData)
{
	UINT	nTimerID;
	UINT	uElapse;
	
	uElapse = _ttoi(strData);

	if (uElapse > 0 && m_TimeoutMode != SCR_TMODE_STOP)
	{
		pScreen->SetScreenTimeout(uElapse);
		
		uElapse *= 1000;
		//nTimerID = UM_TIMER_ID_BASE + _ttoi(pScreen->GetID());
		nTimerID = UM_TIMER_ID_BASE + pScreen->GetUniqueNumber();

		::KillTimer(m_hDisplayWnd, nTimerID);
		::SetTimer(m_hDisplayWnd, nTimerID, uElapse, NULL);
		NHUIDBG(DBG_INFO, (_T(" SCR[%d], ID[%d], Time[%ld]\n"), pScreen->GetID(), nTimerID, uElapse));
	}
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: SetApTimeoutMode()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoCtrl::SetApTimeoutMode(CPicassoScreen *pScreen, CString &strCommand, CString &strData)
{
	if (strData == DES_AP_TIMEOUT_FIRST)
		m_TimeoutMode = SCR_TMODE_FIRST;
	else if (strData == DES_AP_TIMEOUT_LAST)
		m_TimeoutMode = SCR_TMODE_LAST;
	else
	{
		UINT	nTimerID;
		
		//nTimerID = UM_TIMER_ID_BASE + _ttoi(pScreen->GetID());
		nTimerID = UM_TIMER_ID_BASE + pScreen->GetUniqueNumber();
		::KillTimer(m_hDisplayWnd, nTimerID);

		m_TimeoutMode = SCR_TMODE_STOP;
	}
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: SetApUpdate()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoCtrl::SetApUpdate(CPicassoScreen *pScreen, CString &strCommand, CString &strData)
{
	NHUIDBG(1, (_T("***SCR*** SetApUpdate-InvalidateRect\n")));

	if (pScreen != NULL)
	{
		pScreen->SetInputEnable();

		NHUIDBG(DBG_TEST, (_T("UPDATE SCREEN(%s)\n"), strData));
		m_bInputEnabled = TRUE;
		NHUIDBG(DBG_TEST, (_T("UPDATE-INPUT ENABLE\n")));
	}

	UpdateScreen(NULL, TRUE);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: ChangeBlinking()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoCtrl::ChangeBlinking(CPicassoScreen *pScreen, CString &strCommand, CString &strData)
{
	RECT		rcChange;
	CPicassoRequest	Req;

	//NHUIDBG(DBG_CALL, (_T("\n")));

	Req.Set(REQ_BLINKING, NULL, NULL);

	// Add Request.
	if (pScreen)
	{
		pScreen->ClearUpdateRect();
		pScreen->AddRequest(Req);
		pScreen->ProcessRequest();

		while (pScreen->GetFSCmd(Req))
		{
			SendFSCommand(Req.GetName(), Req.m_Data);
		}

		if (pScreen->GetUpdateRect(&rcChange))
		{
			UpdateScreen(&rcChange, TRUE);
		}
	}
}

/*-------------------------------------------------------------------
 CLASS    NAME: CPicassoCtrl
 FUNCTION NAME: UpdateScreen()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CPicassoCtrl::UpdateScreen(RECT *prcRect, BOOL bErase)
{
	if (m_pCurrentScreen != NULL)
	{
		{
			CNHAutoLock	AutoSync(m_csNHProcess);
			m_pDisplayScreen = m_pCurrentScreen;
			m_pDisplayScreen->ClearUpdateRect();
		}

		{
			//CNHAutoLock	AutoSync(m_csReDraw);
			::InvalidateRect(m_hDisplayWnd, prcRect, bErase);
		}
	}
}
