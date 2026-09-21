#include "stdafx.h"
#include "ScreenDisplay.h"

BOOL		g_bUserInputData = FALSE;
CString		g_FSName, g_FSData;

int			g_TimeoutMode = SCR_TMODE_STOP;

///////////////////////////////////////////////////////////////////
// Local Function prototype define.
void	SendFSCommand(CString &strCommand, CString &strData);

void	PrepareScreen(CString strScreenNumber, BOOL PrevSetScreen=FALSE);
void	SetValueState(CNScreen *pScreen, CString &strCommand, CString &strData, BOOL bValueType);
void	SetApMaxTime(CNScreen *pScreen, CString &strCommand, CString &strData);
void	SetApTimeoutMode(CNScreen *pScreen, CString &strCommand, CString &strData);					// [#236] NZ AIREAT 2008.05.27
void	SetApUpdate(CNScreen *pScreen, CString &strCommand, CString &strData);
void	ChangeBlinking(CNScreen *pScreen, CString &strCommand, CString &strData);
void	DisplayKeyboardEvent(LPCTSTR pKeyCode, bool bFormatABC, CNScreen *pCurScreen);
void	SetValueStateEx(CNScreen *pScreen, CString &strCommand, CString &strData);

void	UpdateScreen(RECT *prcRect, BOOL bErase);

// Cross Compile
#ifdef UNDER_CE
void	ADV_Initialize();
void	ADV_Deinitialize();
#endif // UNDER_CE


/*-------------------------------------------------------------------
 CLASS    NAME: 
 FUNCTION NAME: DisplayInteractThread()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : AP로부터 오는 CMD와 AP로 가는 FSCmd를 처리 한다.
-------------------------------------------------------------------*/
DWORD	CROSS_WINAPI DisplayInteractThread(LPVOID lpParameter)
{
	CScrMsg		APMsg;

	g_bInteractDoing = TRUE;

	while(g_bInteractDoing)
	{
		// CHECK AP CMD
		if (g_VariableQueue.IsHasData(300))
		{
			// Get AP CMD
			if (g_VariableQueue.Dequeue(APMsg, 5))
			{
				if (APMsg.m_strCmd == _T("APKey"))
				{
					NHDEBUG(1, (L"[CScrCtrl-DisplayInteractThread] Recv Key Event (%s)\n", APMsg.m_strData));
					DisplayKeyboardEvent(APMsg.m_strData, NULL);
				}
				else if (APMsg.m_strCmd == _T("MouseKey"))
				{
					NHDEBUG(1, (L"[CScrCtrl-DisplayInteractThread] Recv Key Event (%s)\n", APMsg.m_strData));
					DisplayMouseEvent(APMsg.m_Point, NULL);
				}
				else
				{
					DisplaySetVariable(APMsg.m_strCmd, APMsg.m_strData);
				}
			}
		}
	}

	return 1;
}

// SCR -> AP
/*-------------------------------------------------------------------
 CLASS    NAME: 
 FUNCTION NAME: DisplayWndProc()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 화면 Window의 Msg Proc.
-------------------------------------------------------------------*/
void	SendFSCommand(CString &strCommand, CString &strData)
{
	g_FSCmdQueue.Enqueue(CScrMsg(strCommand, strData));
}

/*-------------------------------------------------------------------
 CLASS    NAME: 
 FUNCTION NAME: DisplayWndProc()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 화면 Window의 Msg Proc.

 #201, #203, #204 화면출력은 inserted 되는 text가 늦게 뿌려지므로, 
 APValue를 먼저 받아놓고 CenterMovie시 전체 화면을 Draw한다.
 화면 출력 기본 Flow : APCenterMovie -> APMaxTime -> APValue -> APUpdate
 #201 화면 출력 Flow : PrevSetScreenNo -> APMaxTime -> APValue -> APCenterMovie
 AP -> SCR

-------------------------------------------------------------------*/
void	DisplaySetVariable(CString strCommand, CString strData)
{
	CString		strScreenNumber;
	CNScreen	*pScreen = NULL;

	CNHAutoLock	AutoSync(g_csNHInput);

	// _T("PrevSetScrrenNo")
	if (strCommand == DES_PREV_SET_SCREEN_NO)
	{
		if (_ttoi(strData) != 0)
		{
			strScreenNumber.Format(_T("%03d"), _ttoi(strData));
			PrepareScreen(strScreenNumber, TRUE);
		}
		else
		{
			PrepareScreen(strData, TRUE);
		}
	}
	// _T("APCenterMovie")
	else if (strCommand == DES_AP_CENTER_MOVIE)
	{
		if (_ttoi(strData) != 0)
		{
			strScreenNumber.Format(_T("%03d"), _ttoi(strData));
			PrepareScreen(strScreenNumber, FALSE);
		}
		else
		{
			PrepareScreen(strData, FALSE);
		}
	}
	else
	{
		if (g_pNextScreen)
			pScreen = g_pNextScreen;
		else
			pScreen = g_pCurrentScreen;
		
		if (!pScreen)
			return;

		// BLINKING
		if (_tcscmp(strCommand, DES_BLINKING) == 0)
		{
			ChangeBlinking(pScreen, strCommand, strData);
			return;
		}
		// APValueXX
		else if (_tcsncmp(strCommand, DES_AP_VALUE, 7) == 0)
		{
			SetValueState(pScreen, strCommand, strData, TRUE);
		}
		// APStatusXX
		else if (_tcsncmp(strCommand, DES_AP_STATE, 7) == 0)
		{
			SetValueState(pScreen, strCommand, strData, FALSE);
		}
		// APMaxTime
		else if (_tcscmp(strCommand, DES_AP_MAX_TIME) == 0)
		{
			SetApMaxTime(pScreen, strCommand, strData);
		}
		// [#236] NZ AIREAT 2008.05.27
		else if (_tcscmp(strCommand, DES_AP_TIMEOUT_MODE) == 0)
		{
			SetApTimeoutMode(pScreen, strCommand, strData);
		}
		// end of [#236]
		// APUpdate
		else if (_tcscmp(strCommand, DES_AP_UPDATE) == 0)
		{
			SetApUpdate(pScreen, strCommand, strData);
		}
		// [#434] AU AIREAT 2008.10.14
		else if (_tcscmp(strCommand, DES_AP_SETFOCUS) == 0)
		{
			CRequest		Req;
			Req.Set(REQ_SETFOCUS, strData, _T(""));
			pScreen->AddRequest(Req);
		}
		// end of [#434]
		else if (_tcscmp(strCommand, DES_AP_SET_INPUTMODE) == 0)
		{
			CRequest Req;
			Req.Set(REQ_SETINPUTMODE, _T(""), strData);
			pScreen->AddRequest(Req);
		}
		else if (_tcscmp(strCommand, DES_AP_KEY_DISABLE) == 0)
		{
			CRequest Req;
			Req.Set(REQ_KEYDISABLE, strData, _T(""));
			pScreen->AddRequest(Req);
		}
		else if (_tcscmp(strCommand, DES_AP_SET_INPUTTYPE) == 0)
		{
			CRequest Req;
			Req.Set(REQ_SETINPUTTYPE, _T(""), strData);
			pScreen->AddRequest(Req);
		}
		else if (_tcscmp(strCommand, DES_AP_SET_BLINKMODE) == 0)
		{
			CRequest Req;
			Req.Set(REQ_SETBLINKMODE, _T(""), strData);
			pScreen->AddRequest(Req);
		}
		else if (_tcsncmp(strCommand, DES_AP_EDIT_STATE, 11) == 0)
		{
			SetValueStateEx(pScreen, strCommand, strData);
		}

		pScreen->ProcessRequest();	
	}
}

/*-------------------------------------------------------------------
 CLASS    NAME: 
 FUNCTION NAME: DisplayWndProc()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 화면 Window의 Msg Proc.
-------------------------------------------------------------------*/
void	DisplayKeyboardEvent(CString strKeyCode, CNScreen *pCurScreen)
{
	CRequest	KeyEvent;
	RECT	UpdateRect = {0,};

	CNHAutoLock	AutoSync(g_csNHInput);

	// Is NULL ?
	if (strKeyCode.GetLength() <= 0 || !g_pCurrentScreen)
		return;

	// When timeout occur in Screen Timer,pCurScreen parameter is enabled
	// Screen Number Check. 
	if (pCurScreen)
	{
		// ignore
		if (pCurScreen != g_pCurrentScreen)
			return;
	}

	KeyEvent.Set(REQ_KEYEVENT, _T(""), strKeyCode);

	g_pCurrentScreen->ClearUpdateRect();
	g_pCurrentScreen->AddRequest(KeyEvent);
	g_pCurrentScreen->ProcessRequest();

	if (g_pCurrentScreen->GetFSCmd(KeyEvent))
	{
		SendFSCommand(KeyEvent.GetName(), KeyEvent.m_Data);
	}

	if (g_pCurrentScreen->GetUpdateRect(&UpdateRect))
	{
		NHDEBUG(1, (_T("***SCR*** DisplayKeyboardEvent-InvalidateRect\n")));
		UpdateScreen(&UpdateRect, FALSE);
	}

	// [#236] NZ AIREAT 2008.05.27
	if (g_TimeoutMode == SCR_TMODE_LAST && strKeyCode != DES_TIMEOVER)
	{
		CString	strCommand, strTimeout;

		strCommand = DES_AP_MAX_TIME;
		strTimeout.Format(_T("%d"), g_pCurrentScreen->GetScreenTimeout());

		// update timeout value
		SetApMaxTime(g_pCurrentScreen, strCommand, strTimeout);
	}
	// end of [#236]

//	InputText(strKeyCode);	//*LEH	
}

/*-------------------------------------------------------------------
 CLASS    NAME: 
 FUNCTION NAME: DisplayMouseEvent()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 화면 Window의 Msg Proc.
-------------------------------------------------------------------*/
void	DisplayMouseEvent(POINT &pos, CNScreen *pCurScreen)
{
	CRequest	MouseEvent;
	RECT	UpdateRect;

	CNHAutoLock	AutoSync(g_csNHInput);

	// Is NULL ?
	if (!g_pCurrentScreen)
		return;

	// When timeout occur in Screen Timer,pCurScreen parameter is enabled
	// Screen Number Check. 
	if (pCurScreen)
	{
		// ignore
		if (pCurScreen != g_pCurrentScreen)
			return;
	}

	MouseEvent.Set(REQ_MOUSEEVENT, _T(""), _T(""));
	MouseEvent.m_Point = pos;

	g_pCurrentScreen->ClearUpdateRect();
	g_pCurrentScreen->AddRequest(MouseEvent);
	g_pCurrentScreen->ProcessRequest();

	if (g_pCurrentScreen->GetFSCmd(MouseEvent))
	{
		SendFSCommand(MouseEvent.GetName(), MouseEvent.m_Data);
	}

	if (g_pCurrentScreen->GetUpdateRect(&UpdateRect))
	{
		NHDEBUG(1, (_T("***SCR*** DisplayKeyboardEvent-InvalidateRect\n")));
		UpdateScreen(&UpdateRect, FALSE);
	}

	// [#236] NZ AIREAT 2008.05.27
	if (g_TimeoutMode == SCR_TMODE_LAST)
	{
		CString	strCommand, strTimeout;

		strCommand = DES_AP_MAX_TIME;
		strTimeout.Format(_T("%d"), g_pCurrentScreen->GetScreenTimeout());

		// update timeout value
		SetApMaxTime(g_pCurrentScreen, strCommand, strTimeout);
	}
	// end of [#236]
	
}

/*-------------------------------------------------------------------
 CLASS    NAME: 
 FUNCTION NAME: DisplayWndProc()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 화면 Window의 Msg Proc.
-------------------------------------------------------------------*/
void	PrepareScreen(CString strScreenNumber, BOOL PrevSetScreen)
{
	CNScreen	*pScreen = NULL;

	//
	// _T("PrevSetScrrenNo") -> find scrren -> save screen -> Data setting.
	if (PrevSetScreen)
	{
		// Search Screen by screen number
		if (g_NHScreens.FindDataOfList(strScreenNumber, &pScreen))
		{
			g_pNextScreen = pScreen;

			// Clear Timer
			KillTimer(g_hWndDisplay, (UM_TIMER_ID_BASE + _ttoi(g_pNextScreen->GetID())));
			
			// Initialize.
			g_pNextScreen->Initialize();
		}

	}
	//
	// _T("APCenterMovie") -> find screen. or saved screen -> screen show.
	else
	{
		// find screen.
		{
			// is it saved screen?
			if (g_pNextScreen)
			{
				// if right screen, use saved screen.
				if (g_pNextScreen->GetID() == strScreenNumber)
					pScreen = g_pNextScreen;
				
				g_pNextScreen = NULL;
			}
			
			// new search screen.
			if (!pScreen)
			{
				if (g_NHScreens.FindDataOfList(strScreenNumber, &pScreen))
				{
					// Clear Timer
					KillTimer(g_hWndDisplay, (UM_TIMER_ID_BASE + _ttoi(pScreen->GetID())));

					// Initialize.
					pScreen->Initialize();
					g_TimeoutMode = SCR_TMODE_STOP;
				}
			}
		}

		// update..
		if (pScreen)
		{
			//[#577]2009.11.09 광고 PreView 시 AP Holding 현상 
//			CString strWndTitle;
//			strWndTitle.Format((_T("%s"), pScreen->GetID()));
//			::SetWindowText(g_hWndDisplay, strWndTitle);
			//end of [577]
			
			// [#650] NH KJW 2010.09.06 OP 광고화면 Preview가 실시간 갱신이 안되는 문제 대응
			if( g_pCurrentScreen )
				g_pCurrentScreen->Deinitialize();
			// end of [#650]

			g_pCurrentScreen = pScreen;
			SetForegroundWindow(g_hWndDisplay);
			UpdateScreen(NULL, TRUE);

#ifdef UNDER_CE
			ADV_Deinitialize();						// 광고가 진행중이더라고 새로운 화면이면 광고는 종료한다.

			// Lanch Advertisement
			if (_ttoi(g_pCurrentScreen->GetID()) == 101 && g_AdvertiseInfo.bEnable == FALSE)
			{
				ADV_Initialize();
			}
#endif // UNDER_CE

		}
		else
		{
			g_pCurrentScreen = NULL;
		}

		if (g_pCurrentScreen == NULL)				// [CODESONAR] /* Null Pointer Dereference (ID: 161) */
			return;

	}

	return;
}

/*-------------------------------------------------------------------
 CLASS    NAME: 
 FUNCTION NAME: DisplayWndProc()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 화면 Window의 Msg Proc.
-------------------------------------------------------------------*/
void	SetValueState(CNScreen *pScreen, CString &strCommand, CString &strData, BOOL bValueType)
{
	int				i, nTypeLen = 0, nSetIndex = 0;
	CString			strValueName;
	CStringArray	strTempArray;
	CRequest		Req;

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

void	SetValueStateEx(CNScreen *pScreen, CString &strCommand, CString &strData)
{
	int				i, nTypeLen = 0, nSetIndex = 0;
	CString			strValueName;
	CStringArray	strTempArray;
	CRequest		Req;
	
	// Get Type Length.
	nTypeLen = _tcslen(DES_AP_EDIT_STATE);
	
	// Get Start Index
	nSetIndex = Asc2Int(strCommand.Mid(nTypeLen));
	
	if (nSetIndex <= 0)
		nSetIndex = 1;
	
	// Split String.
	SplitString(strData, SCR_CMD_DELIMITER, strTempArray);
	
	for (i = 0; i < strTempArray.GetSize(); i++, nSetIndex++)
	{
		strValueName.Format(_T("%s%d"), DES_AP_EDIT_STATE, nSetIndex);
		
		Req.Set(REQ_SETVALUE, strValueName, strTempArray[i]);
		
		// Add Request.
		pScreen->AddRequest(Req);
	}
}

/*-------------------------------------------------------------------
 CLASS    NAME: 
 FUNCTION NAME: DisplayWndProc()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 화면 Window의 Msg Proc.
-------------------------------------------------------------------*/
void	SetApMaxTime(CNScreen *pScreen, CString &strCommand, CString &strData)
{
	UINT	nTimerID;
	UINT	uElapse;
	
	uElapse = _ttoi(strData);

	if (uElapse > 0 && g_TimeoutMode != SCR_TMODE_STOP)
	{
		pScreen->SetScreenTimeout(uElapse);
		
		uElapse *= 1000;
		nTimerID = UM_TIMER_ID_BASE + _ttoi(pScreen->GetID());

		KillTimer(g_hWndDisplay, nTimerID);
		SetTimer(g_hWndDisplay, nTimerID, uElapse, NULL);
	}
}

// [#236] NZ AIREAT 2008.05.27
void	SetApTimeoutMode(CNScreen *pScreen, CString &strCommand, CString &strData)
{
	if (strData == DES_AP_TIMEOUT_FIRST)
		g_TimeoutMode = SCR_TMODE_FIRST;
	else if (strData == DES_AP_TIMEOUT_LAST)
		g_TimeoutMode = SCR_TMODE_LAST;
	else
	{
		UINT	nTimerID;
		
		nTimerID = UM_TIMER_ID_BASE + _ttoi(pScreen->GetID());
		KillTimer(g_hWndDisplay, nTimerID);

		g_TimeoutMode = SCR_TMODE_STOP;
	}
}
// end of [#236]

/*-------------------------------------------------------------------
 CLASS    NAME: 
 FUNCTION NAME: DisplayWndProc()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 화면 Window의 Msg Proc.
-------------------------------------------------------------------*/
void	SetApUpdate(CNScreen *pScreen, CString &strCommand, CString &strData)
{
	NHDEBUG(1, (_T("***SCR*** SetApUpdate-InvalidateRect\n")));
	UpdateScreen(NULL, TRUE);
}

/*-------------------------------------------------------------------
 CLASS    NAME: 
 FUNCTION NAME: ChangeBlinking()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 현재 화면의 Blink 속성을 가진 컨트롤을 위하여 텍스트를 변경한다.
-------------------------------------------------------------------*/
void	ChangeBlinking(CNScreen *pScreen, CString &strCommand, CString &strData)
{
	RECT		rcChange;
	CRequest	Req;

	Req.Set(REQ_BLINKING, NULL, NULL);

	// Add Request.
	pScreen->ClearUpdateRect();
	pScreen->AddRequest(Req);
	pScreen->ProcessRequest();

	if (pScreen->GetUpdateRect(&rcChange))
	{
		NHDEBUG(1, (_T("***SCR*** ChangeBlinking-InvalidateRect\n")));
		UpdateScreen(&rcChange, FALSE);
	}
}

void UpdateScreen(RECT *prcRect, BOOL bErase)
{
	CNHAutoLock	AutoSync(g_csNHScreen);

	if (g_pCurrentScreen != NULL)
	{
		g_pDisplayScreen = g_pCurrentScreen;
		g_pDisplayScreen->ClearUpdateRect();
		::InvalidateRect(g_hWndDisplay, prcRect, bErase);
	}
}


// Cross Compile
#ifdef UNDER_CE

void ADV_Initialize()
{
	NHDEBUG(1, (_T("[ADV] ADV_Initialize\n")));

	CString strKey;
	g_AdvertiseInfo.bEnable = FALSE;
	g_AdvertiseInfo.nEnableCount = 0;
	
	for (int i = 0; i < SUPPORT_ADVERTISE; i++)
	{
		// Get Advertisement Screen.
		if (g_AdvertiseInfo.pAdvScreen[i] == NULL)
		{
			strKey.Format(_T("%d"), 30001+i);
			g_NHScreens.FindDataOfList(strKey, &g_AdvertiseInfo.pAdvScreen[i]);
		}

		// Get user setting.
		g_AdvertiseInfo.bEnableAdv[i] = MemGetInt(_MEM_FLD_ADVINFO, _MEM_VAR_ADV_ENDISFLAG1+i);

		// Advertisement enable set
		g_AdvertiseInfo.bEnable |= g_AdvertiseInfo.bEnableAdv[i];

		if (g_AdvertiseInfo.bEnableAdv[i] == TRUE)
			g_AdvertiseInfo.nEnableCount++;
	}

	g_AdvertiseInfo.nCurAdvIndex = -1;
	g_AdvertiseInfo.nShowCount = 0;
	
	// [#14] NH AIREAT 2008.04.28 --> _MEM_VAR_ADV_GUIDEDISP_TIME로 대체
	//g_AdvertiseInfo.nRefreshTime = MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ADVERTISEMENT_TIMER);
	g_AdvertiseInfo.nRefreshTime = MemGetInt(_MEM_FLD_ADVINFO, _MEM_VAR_ADV_GUIDEDISP_TIME);
	// end of [#14]

	if (g_AdvertiseInfo.nRefreshTime < 5 || g_AdvertiseInfo.nRefreshTime > 30)
	{
		g_AdvertiseInfo.nRefreshTime = 5;

		//MemSetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_ADVERTISEMENT_TIMER, 5);		// [#14] NH AIREAT 2008.04.28 --> _MEM_VAR_ADV_GUIDEDISP_TIME로 대체
		MemSetInt(_MEM_FLD_ADVINFO, _MEM_VAR_ADV_GUIDEDISP_TIME, 5);
	}

	if (g_AdvertiseInfo.bEnable)
		SetTimer(g_hWndDisplay, UM_ADV_DECODE_TIME, DECODE_INTERVAL, NULL);
}


void ADV_Deinitialize()
{
	if (g_pCurrentScreen == NULL)
		return;

	if (g_AdvertiseInfo.bEnable == TRUE)
	{
		g_AdvertiseInfo.bEnable = FALSE;
		g_AdvertiseInfo.pCurAdvScreen = NULL;
		g_AdvertiseInfo.nShowCount = 0;		// [#433] NH AIREAT 2008.10.13
	}
	else if (_ttoi(g_pCurrentScreen->GetID()) < 30000)
	{
		for (int i = 0; i < SUPPORT_ADVERTISE; i++)
		{
			// Release Memroy.
			if (g_AdvertiseInfo.pAdvScreen[i] != NULL)
				g_AdvertiseInfo.pAdvScreen[i]->Deinitialize();
		}
	}

}

void ADV_SetNextScreen()
{
	int i, nStart;

	// [#642] 2010.06.30 Next Adv 설정 전에 이전에 Display하는 Adv 메모리를 해제한다.
	if (g_AdvertiseInfo.pCurAdvScreen != NULL)
		g_AdvertiseInfo.pCurAdvScreen->Deinitialize();
	// end of [#642]

	g_AdvertiseInfo.pCurAdvScreen = NULL;
	nStart = g_AdvertiseInfo.nCurAdvIndex; 

	for (i = 0, nStart++; i < SUPPORT_ADVERTISE; i++, nStart++)
	{
		if (nStart >= SUPPORT_ADVERTISE)
			nStart = 0;

		// 현재 화면을 빼고 다음 화면을 찾아야 함.
		//if (g_AdvertiseInfo.nCurAdvIndex == nStart)
		//	break;

		if (g_AdvertiseInfo.bEnableAdv[nStart] == TRUE)
		{
			g_AdvertiseInfo.nCurAdvIndex = nStart;
			g_AdvertiseInfo.pCurAdvScreen = g_AdvertiseInfo.pAdvScreen[nStart];
			break;
		}
	}

	NHDEBUG(1, (_T("[ADV] Current Index(%d)\n"),g_AdvertiseInfo.nCurAdvIndex));
}

int GetNextAdvScreen()
{
	int	nCurIndex;

	if (g_AdvertiseInfo.nShowCount >= 1001)
		g_AdvertiseInfo.nShowCount = 1;

	for (int i = 1; i <= SUPPORT_ADVERTISE; i++)
	{
		nCurIndex = g_AdvertiseInfo.nCurAdvIndex + i;
		if (nCurIndex > SUPPORT_ADVERTISE)
			nCurIndex = nCurIndex - SUPPORT_ADVERTISE;

		if (g_AdvertiseInfo.bEnableAdv[nCurIndex])
		{
			if ((g_AdvertiseInfo.nShowCount % 2) == 0)
				return 0;

			return nCurIndex;
		}
	}

	return 0;
}
#endif // UNDER_CE