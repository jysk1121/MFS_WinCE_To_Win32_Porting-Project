// ScrCtrl.cpp : Defines the entry point for the DLL application.
//

/** ***********************************************************
*	@file		SCR_Manager.cpp
*	@brief		Screen <-> AP Interface 함수 제공
************************************************************/

#include "stdafx.h"
#include "SCR_Manager.h"
#include "ScreenDisplay.h"
#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"
#include "../../EagleCE_Framework/EagleCE_Framework/Util.h"
#include "CXImage/Include/ximage.h"


/////////////////////////////////////////////////////////////
//
//	CSCR_Manager Implementation
//
/////////////////////////////////////////////////////////////

// 인스턴스
CSCR_Manager* CSCR_Manager::m_pInstance = NULL;

/** ***********************************************************
*	@brief		인스턴스 생성
************************************************************/
void CSCR_Manager::CreateInstance()
{
	m_pInstance = new CSCR_Manager();

}


/** ***********************************************************
*	@brief		인스턴스 취득
************************************************************/
CSCR_Manager* CSCR_Manager::GetInstance()
{
	return m_pInstance;
}


/** **********************************************************
*	@brief		인스턴스 해제
************************************************************/
void CSCR_Manager::ReleaseInstance()
{
	if (m_pInstance)
	{
		delete m_pInstance;
	}
}

/** **********************************************************
*	@brief		생성자
************************************************************/
CSCR_Manager::CSCR_Manager()
{
	m_bInit = FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CSCR_Manager
 FUNCTION NAME: ~CSCR_Manager()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 소멸자
-------------------------------------------------------------------*/
CSCR_Manager::~CSCR_Manager()
{
	DeInitalize();
}

/*-------------------------------------------------------------------
 CLASS    NAME: CSCR_Manager
 FUNCTION NAME: GetState()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 초기화 상태를 가져온다.
-------------------------------------------------------------------*/
BOOL CSCR_Manager::GetState()
{
	return m_bInit;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CSCR_Manager
 FUNCTION NAME: Initalize()
 RETURN TYPE  : TRUE : Initialize success
				FLASE : Initialize failed
 PARAMETER    : hInstance : instance handle
				hWndParent : 부모가 있다면 부모 Window의 handle
 DESCRIPTION  : 화면 서비스를 가동 시킨다.
-------------------------------------------------------------------*/
BOOL CSCR_Manager::Initialize(HINSTANCE hInstance, int nWidth, int nHeight, CString strLoadSection, CString strMasterPath)
{
	LOG(Info, _T("[CSCR_Manager::Initialize] --> Param Ins(0x%X) section(%s) Path(%s)"), hInstance, strLoadSection, strMasterPath);

	g_sizeScreen.cx = nWidth;
	g_sizeScreen.cy = nHeight;

	if (CreateDisplayWnd(hInstance, strLoadSection, strMasterPath))
		m_bInit = TRUE;
	else
		m_bInit = FALSE;

	LOG(Info, _T("[CSCR_Manager::Initialize] <-- Result(%d)"), m_bInit);

	return m_bInit;
}


/*-------------------------------------------------------------------
 CLASS    NAME: CSCR_Manager
 FUNCTION NAME: DeInitalize()
 RETURN TYPE  : TRUE : Deinitialize success.
 PARAMETER    : 
 DESCRIPTION  : 화면 서비스를 종료 시킨다.
-------------------------------------------------------------------*/
BOOL CSCR_Manager::DeInitalize()
{
	DestroyDisplayWnd();

	m_bInit = FALSE;

	return TRUE;
}

void CSCR_Manager::ClearUserInputData()
{
	g_FSCmdQueue.Clear();
}

/*-------------------------------------------------------------------
 CLASS    NAME: CSCR_Manager
 FUNCTION NAME: SetVariable()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : AP에서 SCREEN에 화면 데이터 설정.	(AP -> SCR)
-------------------------------------------------------------------*/
BOOL CSCR_Manager::SetVariable(CString Command, CString Data)
{
	CString strLogData = _T("");

	// '%'문자열이 있는경우 log write시 program 죽는 현상으로 인해 replace하도록 함
	strLogData = Data;
	strLogData.Replace(_T("%"), _T("."));

	// Init이 되지 않으면 모든 함수는 실패이다.
	if (!m_bInit)
	{
		LOG(Error, _T("[CSCR_Manager::SetVariable] don't or failed initialize"));
		return FALSE;
	}

	// Clear Queue.
	if (Command == DES_PREV_SET_SCREEN_NO || Command == DES_AP_CENTER_MOVIE)
		g_FSCmdQueue.Clear();
	
	if (!g_VariableQueue.Enqueue(CScrMsg(Command, Data)))
	{
		LOG(Error, _T("[CSCR_Manager::SetVariable] failed Put Queue"));
		return FALSE;
	}

	//LOG(Info, _T("[CSCR_Manager] <-- SetVariable OK"));

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CSCR_Manager
 FUNCTION NAME: SetKeyEvent()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : AP에서 SCREEN에 Key Event 설정.		(AP -> SCR)
-------------------------------------------------------------------*/
BOOL CSCR_Manager::SetKeyEvent(CString Key, BOOL bADAMode)
{
	// Init이 되지 않으면 모든 함수는 실패이다.
	if (!m_bInit)
	{
		LOG(Error, _T("[CSCR_Manager::SetKeyEvent] don't or failed initialize"));
		return FALSE;
	}

	if (g_FSCmdQueue.IsHasData(0))
	{
		return FALSE;
	}

//	if (Key.GetLength() != 1 || Key == _T("."))
	{
		// AP <-> SCREEN Key Mapping
		if (bADAMode == FALSE)
		{
			if (Key == CString(KEYMAP_LEFT))
				Key = DES_STAR;
			else if (Key == CString(KEYMAP_RIGHT))
				Key = DES_SHARP;
			else if (Key == CString(KEYMAP_BLANK))
				Key = DES_CAPS;
			else if (Key == CString(KEYMAP_ENTER))
				Key = DES_ENTER;
			else if (Key == CString(KEYMAP_CLEAR))
				Key = DES_CLEAR;
			else if (Key == CString(KEYMAP_CANCEL))
				Key = DES_CANCEL;
			else if (Key == CString(KEYMAP_FDK_L1))
				Key = EMUL_FUNCKEY_L1;
			else if (Key == CString(KEYMAP_FDK_L2))
				Key = EMUL_FUNCKEY_L2;
			else if (Key == CString(KEYMAP_FDK_L3))
				Key = EMUL_FUNCKEY_L3;
			else if (Key == CString(KEYMAP_FDK_L4))
				Key = EMUL_FUNCKEY_L4;
			else if (Key == CString(KEYMAP_FDK_R1))
				Key = EMUL_FUNCKEY_R1;
			else if (Key == CString(KEYMAP_FDK_R2))
				Key = EMUL_FUNCKEY_R2;
			else if (Key == CString(KEYMAP_FDK_R3))
				Key = EMUL_FUNCKEY_R3;
			else if (Key == CString(KEYMAP_FDK_R4))
				Key = EMUL_FUNCKEY_R4;
		}
		else
		{
			// ADA MODE Emulation key
			if (Key == CString(KEYMAP_1))
				Key = KEYMAP_ADA_1;
			else if (Key == CString(KEYMAP_2))
				Key = KEYMAP_ADA_2;
			else if (Key == CString(KEYMAP_3))
				Key = KEYMAP_ADA_3;
			else if (Key == CString(KEYMAP_4))
				Key = KEYMAP_ADA_4;
			else if (Key == CString(KEYMAP_5))
				Key = KEYMAP_ADA_5;
			else if (Key == CString(KEYMAP_6))
				Key = KEYMAP_ADA_6;
			else if (Key == CString(KEYMAP_7))
				Key = KEYMAP_ADA_7;
			else if (Key == CString(KEYMAP_8))
				Key = KEYMAP_ADA_8;
			else if (Key == CString(KEYMAP_9))
				Key = KEYMAP_ADA_9;
			else if (Key == CString(KEYMAP_0))
				Key = KEYMAP_ADA_0;
			else if (Key == CString(KEYMAP_LEFT))
				Key = KEYMAP_ADA_LEFT;
			else if (Key == CString(KEYMAP_RIGHT))
				Key = KEYMAP_ADA_RIGHT;
			else if (Key == CString(KEYMAP_BLANK))
				Key = KEYMAP_ADA_BLANK;
			else if (Key == CString(KEYMAP_ENTER))
				Key = DES_ENTER;
			else if (Key == CString(KEYMAP_CLEAR))
				Key = DES_CLEAR;
			else if (Key == CString(KEYMAP_CANCEL))
				Key = DES_CANCEL;
		}

	}

	//LOG(Info, (_T("SetKeyEvent Emulated Key: (%s)"), Key));

	if (!g_VariableQueue.Enqueue(CScrMsg(_T("APKey"), Key)))
	{
		LOG(Error, _T("[CSCR_Manager::SetKeyEvent] failed Put Queue"));
		return FALSE;
	}

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CSCR_Manager
 FUNCTION NAME: SetByPassData()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : SCR에서 해당 데이터 처리 하지 않고 AP로 BYPASS
-------------------------------------------------------------------*/
BOOL CSCR_Manager::SetByPassData(CString Data)
{
	// Init이 되지 않으면 모든 함수는 실패이다.
	if (!m_bInit)
	{
		LOG(Error, (_T("[CSCR_Manager::SetByPassData] don't or failed initialize")));
		return FALSE;
	}

	if (g_FSCmdQueue.IsHasData(0))
	{
		return FALSE;
	}

	CString strByPassData;
	strByPassData.Format(_T("BYPASS%s"), Data);
	g_FSCmdQueue.Enqueue(CScrMsg(_T("BYPASS"),strByPassData));

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CSCR_Manager
 FUNCTION NAME: GetUserInputCheck()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : SCREEN에서 AP로 결과가 있는지 check.  (SCR -> AP)
-------------------------------------------------------------------*/
BOOL CSCR_Manager::GetUserInputCheck(DWORD dwTimeout /* = 20 */)
{
	// Init이 되지 않으면 모든 함수는 실패이다.
	if (!m_bInit)
	{
		LOG(Error, (_T("[CSCR_Manager::GetUserInputCheck] don't or failed initialize")));
		return FALSE;
	}

	return g_FSCmdQueue.IsHasData(dwTimeout);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CSCR_Manager
 FUNCTION NAME: GetUserInputData()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : SCREEN에서 AP로 주는 결과 값 가져 옴. (SCR -> AP)
-------------------------------------------------------------------*/
BOOL CSCR_Manager::GetUserInputData(CString &Name, CString &Data)
{
	// Init이 되지 않으면 모든 함수는 실패이다.
	if (!m_bInit)
	{
		LOG(Error, (_T("[CSCR_Manager::GetUserInputData] don't or failed initialize")));
		return FALSE;
	}

	CScrMsg	FSMsg;

	if (!g_FSCmdQueue.Dequeue(FSMsg, 10))
	{
		LOG(Error, (_T("[CSCR_Manager::GetUserInputData] failed Get")));
		return FALSE;
	}

	Name = FSMsg.m_strCmd;
	Data = FSMsg.m_strData;

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CSCR_Manager
 FUNCTION NAME: GetCurrentScreenNumber()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 현재 표시되고 있는 화면 번호를 가져 옴.
-------------------------------------------------------------------*/
CString CSCR_Manager::GetCurrentScreenNumber()
{
	//LOG(Info, _T("[CSCR_Manager] --> GetCurrentScreenNumber"));

	// Init이 되지 않으면 모든 함수는 실패이다.
	if (!m_bInit)
	{
		LOG(Error, (_T("[CSCR_Manager::GetCurrentScreenNumber] don't or failed initialize")));
		return _T("");
	}

	return GetCurScreenNumber();
}

/*-------------------------------------------------------------------
 CLASS    NAME: CSCR_Manager
 FUNCTION NAME: SetScreenLocale()
 RETURN TYPE  : 현재 설정된 LOCALE.
 PARAMETER    : 
 DESCRIPTION  : 화면의 LOCALE을 설정함.
-------------------------------------------------------------------*/
void CSCR_Manager::SetScreenLocale(int Locale /* = 0 */)
{
	LOG(Info, _T("[CSCR_Manager] --> SetScreenLocale (%d)"), Locale);

	// Init이 되지 않으면 모든 함수는 실패이다.
	if (!m_bInit)
	{
		LOG(Error, (_T("[CSCR_Manager::SetScreenLocale] don't or failed initialize")));
		return;
	}

	g_nCurrentLocale = Locale;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CSCR_Manager
 FUNCTION NAME: GetScreenLocale()
 RETURN TYPE  : 현재 설정된 LOCALE.
 PARAMETER    : 
 DESCRIPTION  : 화면의 LOCALE을 설정함.
-------------------------------------------------------------------*/
int CSCR_Manager::GetScreenLocale()
{
	// Init이 되지 않으면 모든 함수는 실패이다.
	if (!m_bInit)
	{
		LOG(Error, (_T("[CSCR_Manager::GetScreenLocale] don't or failed initialize")));
		return -1;
	}

	return g_nCurrentLocale;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CSCR_Manager
 FUNCTION NAME: SetDemoMode()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 데모모드 정보를 화면에 표시 하도록 설정
-------------------------------------------------------------------*/
BOOL CSCR_Manager::SetDemoMode(BOOL bDemo /* = FALSE */)
{
	LOG(Info, _T("[CSCR_Manager] --> SetDemoMode"));

	// Init이 되지 않으면 모든 함수는 실패이다.
	if (!m_bInit)
	{
		LOG(Error, (_T("[CSCR_Manager::SetDemoMode] don't or failed initialize ")));
		return FALSE;
	}

	g_bLocalMode = bDemo;

	LOG(Info, _T("[CSCR_Manager] <-- SetDemoMode OK"));

	return TRUE;
}

void CSCR_Manager::DrawDebugMsg(int x, int y, CString strMsg)
{
	LOG(Info, _T("[CSCR_Manager] --> DrawDebugMsg (%s)"), strMsg);

	// Init이 되지 않으면 모든 함수는 실패이다.
	if (!m_bInit)
	{
		LOG(Error, (_T("[CSCR_Manager::DrawDebugMsg] don't or failed initialize ..")));
		return;
	}

	g_DebugPoint.x = x;
	g_DebugPoint.y = y;
	g_DebugMsg = strMsg;

	::InvalidateRect(g_hWndDisplay, NULL, FALSE);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CSCR_Manager
 FUNCTION NAME: GetAPTextIDString()
 RETURN TYPE  : 
 PARAMETER    : APTextID:XXXX (X:숫자)
 DESCRIPTION  : TextID에 해당하는 문자열을 조회한다.
-------------------------------------------------------------------*/
CString CSCR_Manager::GetAPTextIDString(CString strTextID)
{
	CString			strTextString;
	CNLocaleText	*pTextRes = NULL;
	
	if (!_tcsncmp(strTextID, DES_AP_TEXT_ID, 9) && (strTextID.GetLength() > 9))
	{
		CString strKey = strTextID.Mid(9);
		
		if (g_NHResources.FindDataOfList(strKey, (CNResource**)&pTextRes))
			strTextString = pTextRes->GetLocaleText(g_nCurrentLocale);
	}
	else if (strTextID.GetLength() > 0)
	{
		// TEXT 문자열을 그대로 Fine하는 경우도 지원하도록 로직 추가
		CString strKey = strTextID;

		if (g_NHResources.FindDataOfList(strKey, (CNResource**)&pTextRes))
		{
			strTextString = pTextRes->GetLocaleText(g_nCurrentLocale);

			// 해당 ID는 있으나 Locale에 맞는 Text가 없을 경우 ENG로 return하도록 보완처리
			if (strTextString.IsEmpty())
				strTextString = pTextRes->GetLocaleText(MFS_ENGLISH);
		}
	}

	// 해당 문자열을 찾지 못할 경우 parameter값(strTextID)을 그대로 return한다.
	if (strTextString.IsEmpty())
		strTextString = strTextID;
	
	return strTextString;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CSCR_Manager
 FUNCTION NAME: GetScreenCount()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 로딩된 화면의 갯수를 가져온다.
-------------------------------------------------------------------*/
int	 CSCR_Manager::GetScreenCount()
{
	if (!m_bInit)
		return 0;

	return g_NHScreens.GetDataCount();
}


/*-------------------------------------------------------------------
 CLASS    NAME: CSCR_Manager
 FUNCTION NAME: GetFirstScreenNumber()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 처음 화면 번호를 가져온다.
-------------------------------------------------------------------*/
CString	CSCR_Manager::GetFirstScreenNumber()
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
 CLASS    NAME: CSCR_Manager
 FUNCTION NAME: GetNextScreenNumber()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 다음 화면 번호를 가져온다.
-------------------------------------------------------------------*/
CString	CSCR_Manager::GetNextScreenNumber()
{
	CNScreen *pScreen = NULL;

	if (!m_bInit)
		return _T("");

	g_NHScreens.MoveNext(&pScreen, 1);
	if (pScreen != NULL)
		return pScreen->GetID();

	return _T("");
}


void CSCR_Manager::SetActiveScreen()
{
	// it is not used
}


/*-------------------------------------------------------------------
 CLASS    NAME: CSCR_Manager
 FUNCTION NAME: SetAdvertisement()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 광고 Display 설정.
-------------------------------------------------------------------*/
void CSCR_Manager::SetAdvertisement(BOOL bActiveAdverEnable, int nRefreshTime, CString strEnable)
{
	LOG(Info, _T("[CSCR_Manager] --> SetAdvertisement : bActiveAdverEnable(%d), (%d), (%s)"), bActiveAdverEnable, nRefreshTime, strEnable);

	g_AdvertiseInfo.bActiveFlag = bActiveAdverEnable;
	g_AdvertiseInfo.bEnable = bActiveAdverEnable;		// 추가
	g_AdvertiseInfo.nRefreshTime = nRefreshTime;

	for(int i=0; i<SUPPORT_ADVERTISE; i++)
	{
		if (strEnable.GetLength() > 0)
		{
			if (strEnable.GetLength() > i)
			{
				if (strEnable.GetAt(i) == '1')
					g_AdvertiseInfo.bEnableAdv[i] = TRUE;
				else
					g_AdvertiseInfo.bEnableAdv[i] = FALSE;
			}
			else
			{
				g_AdvertiseInfo.bEnableAdv[i] = FALSE;
			}
		}
		else
		{
			g_AdvertiseInfo.bEnableAdv[i] = FALSE;
		}
	}
}


/*-------------------------------------------------------------------
 CLASS    NAME: CSCR_Manager
 FUNCTION NAME: GetPosValue4Camera()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Camera Viewer Pos 취득
-------------------------------------------------------------------*/
RECT CSCR_Manager::GetPosValue4Camera()
{
	return g_rect4Camera;
}


/*-------------------------------------------------------------------
 CLASS    NAME: CSCR_Manager
 FUNCTION NAME: LoadAdvertisement()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : ADV 파일을 원하는 시점에 Decode하기 위함.
-------------------------------------------------------------------*/
void CSCR_Manager::LoadAdvertisement()
{
	ADV_Deinitialize();
	ADV_Decode();
}


/*-------------------------------------------------------------------
CLASS    NAME: CSCR_Manager
FUNCTION NAME: FlipImage()
RETURN TYPE  :
PARAMETER    :
DESCRIPTION  : 이미지 파일 상하 반전
-------------------------------------------------------------------*/
BOOL CSCR_Manager::FlipImage(CString strInputFilePath, CString strOutputFilePath)
{
	CxImage image;
	BOOL bRet = FALSE;
	//CString strFileDir;
	//CString strFileName;
	int nIndex = 0;

	//strFileDir = strInputFilePath.Left(strInputFilePath.ReverseFind('\\'));
	//nIndex = strInputFilePath.ReverseFind('\\');
	//strFileName = strInputFilePath.Right(strInputFilePath.GetLength() - nIndex - 1);

#ifdef _WIN32_WCE
	image.Load(strInputFilePath, CXIMAGE_FORMAT_JPG);
#else
	// cximage.lib is the ANSI build, so convert the paths for its API.
	CT2A szInputFile(strInputFilePath);
	image.Load(szInputFile.m_psz, CXIMAGE_FORMAT_JPG);
#endif	// _WIN32_WCE

	if(image.IsValid())
	{
		image.Flip();

#ifdef _WIN32_WCE
		bRet = image.Save(strOutputFilePath, CXIMAGE_FORMAT_JPG);
#else
		CT2A szOutputFile(strOutputFilePath);
		bRet = image.Save(szOutputFile.m_psz, CXIMAGE_FORMAT_JPG);
#endif	// _WIN32_WCE

		//if(TRUE == bRet)
		//	CUtil::DeleteFileInDirectory(strFileDir, strFileName);
		::DeleteFile(strInputFilePath);

	}
	else
	{
		bRet = FALSE;
	}

	return bRet;
}
