#include "stdafx.h"
#include "ScreenDisplay.h"

#include "../../EagleCE_Framework/EagleCE_Framework/Util.h"
#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"

BOOL		g_bUserInputData = FALSE;
CString		g_FSName, g_FSData;
int			g_TimeoutMode = SCR_TMODE_STOP;

///////////////////////////////////////////////////////////////////
// Local Function prototype define.
void	SendFSCommand(CString &strCommand, CString &strData);

void	PrepareScreen(CString strScreenNumber, BOOL PrevSetScreen=FALSE);
void	SetValueState(CNScreen *pScreen, CString &strCommand, CString &strData, BOOL bValueType);
void	SetApMaxTime(CNScreen *pScreen, CString &strCommand, CString &strData);
void	SetApTimeoutMode(CNScreen *pScreen, CString &strCommand, CString &strData);
void	SetApUpdate(CNScreen *pScreen, CString &strCommand, CString &strData);
void	ChangeBlinking(CNScreen *pScreen, CString &strCommand, CString &strData);
void	DisplayKeyboardEvent(LPCTSTR pKeyCode, bool bFormatABC, CNScreen *pCurScreen);
void	SetValueStateEx(CNScreen *pScreen, CString &strCommand, CString &strData);

void	UpdateScreen(RECT *prcRect, BOOL bErase);

// ADV_Initialize() is defined further down in this file for both platforms.
void	ADV_Initialize();

// Cross Compile
#ifdef UNDER_CE
//void	ADV_Deinitialize();	// V1.0.2.5 2018.08.06 - ScreenDisplay.h로 이동
#endif // UNDER_CE


// [PCI-SSF Fix] 예외 처리 가드 - 메시지 디스패치 트램폴린 및 SEH wrapper
// __try/__except는 C++ 객체(소멸자)가 있는 함수 안에서 쓸 수 없으므로(C2712),
// 실제 디스패치(CString 등 사용)는 트램폴린 함수로 분리하고, SEH는 POD 인자만
// 받는 별도 wrapper에서만 감싼다.
struct DispatchCtx
{
	CScrMsg* pMsg;
};

static void TrampolineAPKey(LPVOID pArg)
{
	DispatchCtx* pCtx = (DispatchCtx*)pArg;
	DisplayKeyboardEvent(pCtx->pMsg->m_strData, NULL);
}

static void TrampolineMouseKey(LPVOID pArg)
{
	DispatchCtx* pCtx = (DispatchCtx*)pArg;
	DisplayMouseEvent(pCtx->pMsg->m_Point, NULL);
}

static void TrampolineSetVariable(LPVOID pArg)
{
	DispatchCtx* pCtx = (DispatchCtx*)pArg;
	DisplaySetVariable(pCtx->pMsg->m_strCmd, pCtx->pMsg->m_strData);
}

typedef void (*PFN_DISPATCH)(LPVOID);

static void SafeDispatch(PFN_DISPATCH pfn, LPVOID pArg)
{
	__try
	{
		pfn(pArg);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		// LOG는 CString ????? ?????? ??????? ?????? ???Ŀ? ????? ???? ???.
		LOG(Error, _T("DisplayInteractThread: unhandled exception (0x%08X) caught during dispatch - screen thread continues"), GetExceptionCode());
	}
}

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
	DispatchCtx	ctx;

	ctx.pMsg = &APMsg;

	g_bInteractDoing = TRUE;

	while(g_bInteractDoing)
	{
		// CHECK AP CMD
		if (g_VariableQueue.IsHasData(300))
		{
			// Get AP CMD
			if (g_VariableQueue.Dequeue(APMsg, 5))
			{
				// [PCI-SSF Fix] 스크립트/화면 상태 처리 중 예외가 발생해도 프로세스가
				// 죽지 않고 다음 메시지 처리를 계속하도록 SEH로 감싼다.
				if (APMsg.m_strCmd == _T("APKey"))
				{
					SafeDispatch(TrampolineAPKey, &ctx);
				}
				else if (APMsg.m_strCmd == _T("MouseKey"))
				{
					SafeDispatch(TrampolineMouseKey, &ctx);
				}
				else
				{
					SafeDispatch(TrampolineSetVariable, &ctx);
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
		else if (_tcscmp(strCommand, DES_AP_TIMEOUT_MODE) == 0)
		{
			SetApTimeoutMode(pScreen, strCommand, strData);
		}
		// APUpdate
		else if (_tcscmp(strCommand, DES_AP_UPDATE) == 0)
		{
			SetApUpdate(pScreen, strCommand, strData);
		}
		else if (_tcscmp(strCommand, DES_AP_SETFOCUS) == 0)
		{
			CRequest		Req;
			Req.Set(REQ_SETFOCUS, strData, _T(""));
			pScreen->AddRequest(Req);
		}
		else if (_tcscmp(strCommand, DES_AP_SET_INPUTMODE) == 0)
		{
			CRequest Req;
			Req.Set(REQ_SETINPUTMODE, _T(""), strData);
			pScreen->AddRequest(Req);
		}
		//>>> @ADD: 2017.11.21 LEH
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
		//<<<
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
		UpdateScreen(&UpdateRect, FALSE);
	}

	if (g_TimeoutMode == SCR_TMODE_LAST && strKeyCode != DES_TIMEOVER)
	{
		CString	strCommand, strTimeout;

		strCommand = DES_AP_MAX_TIME;
		strTimeout.Format(_T("%d"), g_pCurrentScreen->GetScreenTimeout());

		// update timeout value
		SetApMaxTime(g_pCurrentScreen, strCommand, strTimeout);
	}

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
		UpdateScreen(&UpdateRect, FALSE);
	}

	if (g_TimeoutMode == SCR_TMODE_LAST)
	{
		CString	strCommand, strTimeout;

		strCommand = DES_AP_MAX_TIME;
		strTimeout.Format(_T("%d"), g_pCurrentScreen->GetScreenTimeout());

		// update timeout value
		SetApMaxTime(g_pCurrentScreen, strCommand, strTimeout);
	}
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
			if( g_pCurrentScreen )
				g_pCurrentScreen->Deinitialize();

			g_pCurrentScreen = pScreen;
			SetForegroundWindow(g_hWndDisplay);
			UpdateScreen(NULL, TRUE);

			// V1.0.2.5 2018.08.06 - 매번 ADV 파일을 Decode/Release 하는 로직 제거
//			ADV_Deinitialize();						// 광고가 진행중이더라고 새로운 화면이면 광고는 종료한다.

			// Launch Advertisement
//			if (_ttoi(g_pCurrentScreen->GetID()) == 101 && g_AdvertiseInfo.bEnable == FALSE)
			if (g_AdvertiseInfo.bActiveFlag == TRUE)	// 101번 화면과 상관없이 상위국에서 Setting한 값을 Display하도록 수정
			{
				ADV_Initialize();
			}
		}
		else
		{
			g_pCurrentScreen = NULL;
		}

		if (g_pCurrentScreen == NULL)
			return;

	}

	return;
}

/** **********************************************
*	@brief		control 제어 및 value 입력 
*	@details	버튼 on/off 및 edit 입력
*	@param	CNScreen *pScreen	: screen 정보 
*	@param	CString &strCommand	: Command
*	@param	CString &strData	: Data
*	@param	BOOL bValueType		: APState/APValue 구분
***************************************************/ 
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
	nSetIndex = CUtil::StringToInt(strCommand.Mid(nTypeLen));

	if (nSetIndex <= 0)
		nSetIndex = 1;

	// Split String.
	CUtil::ParsingStringToStringArray(strData, SCR_CMD_DELIMITER, strTempArray);

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

/** **********************************************
*	@brief		EditBox control 기능 추가 
*	@details	EditBox control에도 상태 정보를 설정 가능하도록
*	@param	CNScreen *pScreen	: screen 정보 
*	@param	CString &strCommand	: Command
*	@param	CString &strData	: Data
***************************************************/  
void	SetValueStateEx(CNScreen *pScreen, CString &strCommand, CString &strData)
{
	int				i, nTypeLen = 0, nSetIndex = 0;
	CString			strValueName;
	CStringArray	strTempArray;
	CRequest		Req;

	// Get Type Length.
	nTypeLen = _tcslen(DES_AP_EDIT_STATE);

	// Get Start Index
	nSetIndex = CUtil::StringToInt(strCommand.Mid(nTypeLen));

	if (nSetIndex <= 0)
		nSetIndex = 1;

	// Split String.
	CUtil::ParsingStringToStringArray(strData, SCR_CMD_DELIMITER, strTempArray);

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

/*-------------------------------------------------------------------
 CLASS    NAME: 
 FUNCTION NAME: DisplayWndProc()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 화면 Window의 Msg Proc.
-------------------------------------------------------------------*/
void	SetApUpdate(CNScreen *pScreen, CString &strCommand, CString &strData)
{
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



void ADV_Initialize()
{
//	CString strKey;
	g_AdvertiseInfo.bEnable = FALSE;
	g_AdvertiseInfo.nEnableCount = 0;
	
	for (int i = 0; i < SUPPORT_ADVERTISE; i++)
	{
		// V1.0.2.5 2018.08.06 - ADV 파일을 원하는 시점에 Decode하기 위해 주석 처리함.
		// Get Advertisement Screen.
// 		if (g_AdvertiseInfo.pAdvScreen[i] == NULL)
// 		{
// 			strKey.Format(_T("%d"), CLIENT_ADV_SCREEN_1 + i);
// 			g_NHScreens.FindDataOfList(strKey, &g_AdvertiseInfo.pAdvScreen[i]);
// 		}

		// Get user setting.
		//g_AdvertiseInfo.bEnableAdv[i] 값은 상위국에서 Setting한 값을 사용

		// Advertisement enable set
		if (g_AdvertiseInfo.bEnable == FALSE)
			g_AdvertiseInfo.bEnable = g_AdvertiseInfo.bEnableAdv[i];

		if (g_AdvertiseInfo.bEnableAdv[i] == TRUE)
			g_AdvertiseInfo.nEnableCount++;
	}

	g_AdvertiseInfo.nCurAdvIndex = -1;
	g_AdvertiseInfo.nShowCount = 0;
	
	// GUIDE DISP TIME
	//g_AdvertiseInfo.nRefreshTime 값은 상위국에서 Setting한 값을 사용

	if (g_AdvertiseInfo.nRefreshTime < 5 || g_AdvertiseInfo.nRefreshTime > 60)	// 60 sec까지 가능하도록 수정
		g_AdvertiseInfo.nRefreshTime = 5;

	if (g_AdvertiseInfo.bEnable)
		SetTimer(g_hWndDisplay, UM_ADV_DECODE_TIME, DECODE_INTERVAL, NULL);
}


void ADV_Deinitialize()
{
	if (g_pCurrentScreen == NULL)
		return;

	if (g_AdvertiseInfo.bEnable)
	{
		g_AdvertiseInfo.bEnable = FALSE;
		g_AdvertiseInfo.pCurAdvScreen = NULL;
		g_AdvertiseInfo.nShowCount = 0;	
	}
//	else// if (_ttoi(g_pCurrentScreen->GetID()) < 30000)	// V1.0.2.5 2018.08.06 - ADV 파일 LOAD 하기전 Release 시키기 위해
//	{
		for (int i = 0; i < SUPPORT_ADVERTISE; i++)
		{
			// Release Memory
			if (g_AdvertiseInfo.pAdvScreen[i] != NULL)
			{
				g_AdvertiseInfo.pAdvScreen[i]->Deinitialize();
				g_AdvertiseInfo.pAdvScreen[i] = NULL;
			}
		}
//	}
}

void ADV_SetNextScreen()
{
	int i, nStart;

	// V1.0.2.5 2018.08.06 - 매번 ADV 파일을 Decode/Release 하는 로직 제거
	// Adv 설정 전에 이전에 Display하는 Adv 메모리를 해제한다.
// 	if (g_AdvertiseInfo.pCurAdvScreen != NULL)
// 		g_AdvertiseInfo.pCurAdvScreen->Deinitialize();

	g_AdvertiseInfo.pCurAdvScreen = NULL;
	nStart = g_AdvertiseInfo.nCurAdvIndex; 

	for (i = 0, nStart++; i < SUPPORT_ADVERTISE; i++, nStart++)
	{
		if (nStart >= SUPPORT_ADVERTISE)
			nStart = 0;

		// 현재 화면을 빼고 다음 화면을 찾아야 함.
		if (g_AdvertiseInfo.bEnableAdv[nStart] == TRUE)
		{
			g_AdvertiseInfo.nCurAdvIndex = nStart;
			g_AdvertiseInfo.pCurAdvScreen = g_AdvertiseInfo.pAdvScreen[nStart];
			break;
		}
	}
}

//int GetNextAdvScreen()
//{
//	int	nCurIndex;
//
//	if (g_AdvertiseInfo.nShowCount >= 1001)
//		g_AdvertiseInfo.nShowCount = 1;
//
//	for (int i = 1; i <= SUPPORT_ADVERTISE; i++)
//	{
//		nCurIndex = g_AdvertiseInfo.nCurAdvIndex + i;
//		if (nCurIndex > SUPPORT_ADVERTISE)
//			nCurIndex = nCurIndex - SUPPORT_ADVERTISE;
//
//		if (g_AdvertiseInfo.bEnableAdv[nCurIndex])
//		{
//			if ((g_AdvertiseInfo.nShowCount % 2) == 0)
//				return 0;
//
//			return nCurIndex;
//		}
//	}
//
//	return 0;
//}

void ADV_Decode()
{
	CString strKey;

	for (int i = 0; i < SUPPORT_ADVERTISE; i++)
	{
		// Get Advertisement Screen.
		if (g_AdvertiseInfo.pAdvScreen[i] == NULL)
		{
			strKey.Format(_T("%d"), CLIENT_ADV_SCREEN_1 + i);
			g_NHScreens.FindDataOfList(strKey, &g_AdvertiseInfo.pAdvScreen[i]);
		}

		if (g_AdvertiseInfo.pAdvScreen[i] != NULL)
		{
			g_AdvertiseInfo.pAdvScreen[i]->Initialize();
		}
	}
}