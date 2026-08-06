// ScrCtrl.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include ".\Dll\ScrCtrl.h"

#include "ScreenDisplay.h"

static AFX_EXTENSION_MODULE ScrCtrlDLL = { NULL, NULL };

BOOL APIENTRY DllMain(HANDLE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule(ScrCtrlDLL, (HINSTANCE)hInstance))
			return 0;

		// Insert this DLL into the resource chain
		// NOTE: If this Extension DLL is being implicitly linked to by
		//  an MFC Regular DLL (such as an ActiveX Control)
		//  instead of an MFC application, then you will want to
		//  remove this line from DllMain and put it in a separate
		//  function exported from this Extension DLL.  The Regular DLL
		//  that uses this Extension DLL should then explicitly call that
		//  function to initialize this Extension DLL.  Otherwise,
		//  the CDynLinkLibrary object will not be attached to the
		//  Regular DLL's resource chain, and serious problems will
		//  result.

		new CDynLinkLibrary(ScrCtrlDLL);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		// Terminate the library before destructors are called
		AfxTermExtensionModule(ScrCtrlDLL);
	}

    return TRUE;
}



/////////////////////////////////////////////////////////////
//
//	CScrCtrl Implementation
//
/////////////////////////////////////////////////////////////


/*-------------------------------------------------------------------
 CLASS    NAME: CScrCtrl
 FUNCTION NAME: CScrCtrl()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 생성자.
-------------------------------------------------------------------*/
CScrCtrl::CScrCtrl()
{
	m_bInit = FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CScrCtrl
 FUNCTION NAME: ~CScrCtrl()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 소멸자
-------------------------------------------------------------------*/
CScrCtrl::~CScrCtrl()
{
	DeInitalize();
}

/*-------------------------------------------------------------------
 CLASS    NAME: CScrCtrl
 FUNCTION NAME: GetState()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 초기화 상태를 가져온다.
-------------------------------------------------------------------*/
BOOL CScrCtrl::GetState()
{
	return m_bInit;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CScrCtrl
 FUNCTION NAME: Initalize()
 RETURN TYPE  : TRUE : Initialize success
				FLASE : Initialize failed
 PARAMETER    : hInstance : instance handle
				hWndParent : 부모가 있다면 부모 Window의 handle
 DESCRIPTION  : 화면 서비스를 가동 시킨다.
-------------------------------------------------------------------*/
BOOL CScrCtrl::Initialize(HINSTANCE hInstance, int nWidth, int nHeight, CString strLoadSection, CString strMasterPath)
{
	NHDEBUG(1, (L"[CScrCtrl::Initialize] --> Param Ins(0x%X) section(%s) Path(%s)\n", hInstance, strLoadSection, strMasterPath));

	g_sizeScreen.cx = nWidth;
	g_sizeScreen.cy = nHeight;

	g_BaseSizeScreen.cx = nWidth;
	g_BaseSizeScreen.cy = nHeight;
	
	if (CreateDisplayWnd(hInstance, strLoadSection, strMasterPath))
		m_bInit = TRUE;
	else
		m_bInit = FALSE;

	NHDEBUG(1, (L"[CScrCtrl::Initialize] <-- Result(%d)\n", m_bInit));

	return m_bInit;
}


/*-------------------------------------------------------------------
 CLASS    NAME: CScrCtrl
 FUNCTION NAME: DeInitalize()
 RETURN TYPE  : TRUE : Deinitialize success.
 PARAMETER    : 
 DESCRIPTION  : 화면 서비스를 종료 시킨다.
-------------------------------------------------------------------*/
BOOL CScrCtrl::DeInitalize()
{
	NHDEBUG(1, (L"[CScrCtrl] --> DeInitalize\n"));

	DestroyDisplayWnd();

	m_bInit = FALSE;

	NHDEBUG(1, (L"[CScrCtrl] <-- DeInitalize OK\n"));

	return TRUE;
}

void CScrCtrl::ClearUserInputData()
{
	g_FSCmdQueue.Clear();
}

/*-------------------------------------------------------------------
 CLASS    NAME: CScrCtrl
 FUNCTION NAME: SetVariable()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : AP에서 SCREEN에 화면 데이터 설정.	(AP -> SCR)
-------------------------------------------------------------------*/
BOOL CScrCtrl::SetVariable(CString Command, CString Data)
{
	NHDEBUG(1, (L"[CScrCtrl] --> SetVariable\n"));

	// Init이 되지 않으면 모든 함수는 실패이다.
	if (!m_bInit)
	{
		NHERROR(1, (_T("[CScrCtrl::SetVariable] don't or failed initialize ..\n")));
		return FALSE;
	}

	NHDEBUG(1, (_T("***SCR*** SetVariable(%s,%s)\n"), Command, Data));

	// Clear Queue.
	if (Command == DES_PREV_SET_SCREEN_NO || Command == DES_AP_CENTER_MOVIE)
		g_FSCmdQueue.Clear();
	
	if (!g_VariableQueue.Enqueue(CScrMsg(Command, Data)))
	{
		NHERROR(1, (_T("[CScrCtrl::SetVariable] failed Put Queue ..\n")));
		return FALSE;
	}

	NHDEBUG(1, (L"[CScrCtrl] <-- SetVariable OK\n"));

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CScrCtrl
 FUNCTION NAME: SetKeyEvent()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : AP에서 SCREEN에 Key Event 설정.		(AP -> SCR)
-------------------------------------------------------------------*/
BOOL CScrCtrl::SetKeyEvent(CString Key)
{
	NHDEBUG(1, (L"[CScrCtrl] --> SetKeyEvent\n"));

	// Init이 되지 않으면 모든 함수는 실패이다.
	if (!m_bInit)
	{
		NHERROR(1, (_T("[CScrCtrl::SetKeyEvent] don't or failed initialize ..\n")));
		return FALSE;
	}

	NHDEBUG(1, (_T("***SCR*** SetKeyEvent : (%s)\n"), Key));

	if (g_FSCmdQueue.IsHasData(0))
	{
		return FALSE;
	}

//	if (Key.GetLength() != 1 || Key == _T("."))
	{
		// Key Mapping
		if (Key == _T("A"))
			Key = DES_STAR;
		else if (Key == _T("B"))
			Key = DES_SHARP;
		else if (Key == _T("C"))
			Key = DES_CAPS;
		else if (Key == _T("D"))
			Key = DES_ENTER;
		else if (Key == _T("E"))
			Key = DES_CLEAR;
		else if (Key == _T("F"))
			Key = DES_CANCEL;
		
	}

	if (!g_VariableQueue.Enqueue(CScrMsg(_T("APKey"), Key)))
	{
		NHERROR(1, (_T("[CScrCtrl::SetKeyEvent] failed Put Queue ..\n")));
		return FALSE;
	}

	NHDEBUG(1, (L"[CScrCtrl] <-- SetKeyEvent OK\n"));

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CScrCtrl
 FUNCTION NAME: SetByPassData()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : SCR에서 해당 데이터 처리 하지 않고 AP로 BYPASS
-------------------------------------------------------------------*/
BOOL CScrCtrl::SetByPassData(CString Data)
{
NHDEBUG(1, (L"[CScrCtrl] --> SetByPassData\n"));

	// Init이 되지 않으면 모든 함수는 실패이다.
	if (!m_bInit)
	{
		NHERROR(1, (_T("[CScrCtrl::SetByPassData] don't or failed initialize ..\n")));
		return FALSE;
	}

	NHDEBUG(1, (_T("***SCR*** SetByPassData : (%s)\n"), Data));

#if !(AU_VERSION || NZ_VERSION)	// [#848] AU KSK 2017.01.06
	if (g_FSCmdQueue.IsHasData(0))
	{
		return FALSE;
	}
#endif

	CString strByPassData;
	strByPassData.Format(_T("BYPASS%s"), Data);
	g_FSCmdQueue.Enqueue(CScrMsg(_T("BYPASS"),strByPassData));

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CScrCtrl
 FUNCTION NAME: GetUserInputCheck()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : SCREEN에서 AP로 결과가 있는지 check.  (SCR -> AP)
-------------------------------------------------------------------*/
BOOL CScrCtrl::GetUserInputCheck(DWORD dwTimeout /* = 20 */)
{
	//NHDEBUG(1, (L"[CScrCtrl] --> GetUserInputCheck\n"));

	// Init이 되지 않으면 모든 함수는 실패이다.
	if (!m_bInit)
	{
		NHERROR(1, (_T("[CScrCtrl::GetUserInputCheck] don't or failed initialize ..\n")));
		return FALSE;
	}

	return g_FSCmdQueue.IsHasData(dwTimeout);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CScrCtrl
 FUNCTION NAME: GetUserInputData()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : SCREEN에서 AP로 주는 결과 값 가져 옴. (SCR -> AP)
-------------------------------------------------------------------*/
BOOL CScrCtrl::GetUserInputData(CString &Name, CString &Data)
{
	NHDEBUG(1, (L"[CScrCtrl] --> GetUserInputData\n"));

	// Init이 되지 않으면 모든 함수는 실패이다.
	if (!m_bInit)
	{
		NHERROR(1, (_T("[CScrCtrl::GetUserInputData] don't or failed initialize ..\n")));
		return FALSE;
	}

	CScrMsg	FSMsg;

	if (!g_FSCmdQueue.Dequeue(FSMsg, 10))
	{
		NHERROR(1, (_T("[CScrCtrl::GetUserInputData] failed Get ..\n")));
		return FALSE;
	}

	Name = FSMsg.m_strCmd;
	Data = FSMsg.m_strData;

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CScrCtrl
 FUNCTION NAME: GetCurrentScreenNumber()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 현재 표시되고 있는 화면 번호를 가져 옴.
-------------------------------------------------------------------*/
CString CScrCtrl::GetCurrentScreenNumber()
{
	NHDEBUG(1, (L"[CScrCtrl] --> GetCurrentScreenNumber\n"));

	// Init이 되지 않으면 모든 함수는 실패이다.
	if (!m_bInit)
	{
		NHERROR(1, (_T("[CScrCtrl::GetCurrentScreenNumber] don't or failed initialize ..\n")));
		return _T("");
	}

	return GetCurScreenNumber();
}

/*-------------------------------------------------------------------
 CLASS    NAME: CScrCtrl
 FUNCTION NAME: SetScreenLocale()
 RETURN TYPE  : 현재 설정된 LOCALE.
 PARAMETER    : 
 DESCRIPTION  : 화면의 LOCALE을 설정함.
-------------------------------------------------------------------*/
void CScrCtrl::SetScreenLocale(int Locale /* = 0 */)
{
	NHDEBUG(1, (L"[CScrCtrl] --> SetScreenLocale\n"));

	// Init이 되지 않으면 모든 함수는 실패이다.
	if (!m_bInit)
	{
		NHERROR(1, (_T("[CScrCtrl::SetScreenLocale] don't or failed initialize ..\n")));
		return;
	}

	g_nCurrentLocale = Locale;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CScrCtrl
 FUNCTION NAME: GetScreenLocale()
 RETURN TYPE  : 현재 설정된 LOCALE.
 PARAMETER    : 
 DESCRIPTION  : 화면의 LOCALE을 설정함.
-------------------------------------------------------------------*/
int CScrCtrl::GetScreenLocale()
{
	NHDEBUG(1, (L"[CScrCtrl] --> GetScreenLocale\n"));

	// Init이 되지 않으면 모든 함수는 실패이다.
	if (!m_bInit)
	{
		NHERROR(1, (_T("[CScrCtrl::GetScreenLocale] don't or failed initialize ..\n")));
		return -1;
	}

	return g_nCurrentLocale;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CScrCtrl
 FUNCTION NAME: SetDemoMode()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 데모모드 정보를 화면에 표시 하도록 설정
-------------------------------------------------------------------*/
BOOL CScrCtrl::SetDemoMode(BOOL bDemo /* = FALSE */)
{
	NHDEBUG(1, (L"[CScrCtrl] --> SetDemoMode\n"));

	// Init이 되지 않으면 모든 함수는 실패이다.
	if (!m_bInit)
	{
		NHERROR(1, (_T("[CScrCtrl::SetDemoMode] don't or failed initialize ..\n")));
		return FALSE;
	}

	g_bLocalMode = bDemo;

	NHDEBUG(1, (L"[CScrCtrl] <-- SetDemoMode OK\n"));

	return TRUE;
}

void CScrCtrl::DrawDebugMsg(int x, int y, CString strMsg)
{
	// Init이 되지 않으면 모든 함수는 실패이다.
	if (!m_bInit)
	{
		NHERROR(1, (_T("[CScrCtrl::DrawDebugMsg] don't or failed initialize ..\n")));
		return;
	}

	g_DebugPoint.x = x;
	g_DebugPoint.y = y;
	g_DebugMsg = strMsg;

	::InvalidateRect(g_hWndDisplay, NULL, FALSE);
}

// [#237] NH AIREAT 2008.05.27
/*-------------------------------------------------------------------
 CLASS    NAME: CScrCtrl
 FUNCTION NAME: GetAPTextIDString()
 RETURN TYPE  : 
 PARAMETER    : APTextID:XXXX (X:숫자)
 DESCRIPTION  : TextID에 해당하는 문자열을 조회한다.
-------------------------------------------------------------------*/
CString CScrCtrl::GetAPTextIDString(CString strTextID)
{
	CString			strTextString = _T("");
	CNLocaleText	*pTextRes = NULL;
	
	if (!_tcsncmp(strTextID, DES_AP_TEXT_ID, 9) && (strTextID.GetLength() > 9))
	{
		CString strKey = strTextID.Mid(9);
		
		if (g_NHResources.FindDataOfList(strKey, (CNResource**)&pTextRes))
			strTextString = pTextRes->GetLocaleText(g_nCurrentLocale);
	}
	
	return strTextString;
}
// end of [#237]

/*-------------------------------------------------------------------
 CLASS    NAME: CScrCtrl
 FUNCTION NAME: GetScreenCount()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 로딩된 화면의 갯수를 가져온다.
-------------------------------------------------------------------*/
int	 CScrCtrl::GetScreenCount()
{
	if (!m_bInit)
		return 0;

	return g_NHScreens.GetDataCount();
}

/*-------------------------------------------------------------------
 CLASS    NAME: CScrCtrl
 FUNCTION NAME: GetFirstScreenNumber()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 처음 화면 번호를 가져온다.
-------------------------------------------------------------------*/
CString	CScrCtrl::GetFirstScreenNumber()
{
	CNScreen *pScreen = NULL;

	if (!m_bInit)
		return _T("");

	g_NHScreens.MoveFirst(&pScreen);
	if (pScreen != NULL)
		return pScreen->GetID();

	return _T("");
}

/*-------------------------------------------------------------------
 CLASS    NAME: CScrCtrl
 FUNCTION NAME: GetNextScreenNumber()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 다음 화면 번호를 가져온다.
-------------------------------------------------------------------*/
CString	CScrCtrl::GetNextScreenNumber()
{
	CNScreen *pScreen = NULL;

	if (!m_bInit)
		return _T("");

	g_NHScreens.MoveNext(&pScreen, 1);
	if (pScreen != NULL)
		return pScreen->GetID();

	return _T("");
}

void CScrCtrl::SetActiveScreen()
{
	// iTM 용 임시 함수
}

RECT CScrCtrl::GetPosValue4Camera()
{
	return g_rect4Camera;
}
