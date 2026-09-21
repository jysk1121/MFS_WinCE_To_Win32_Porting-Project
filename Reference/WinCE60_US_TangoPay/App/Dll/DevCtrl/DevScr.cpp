/////////////////////////////////////////////////////////////////////////////
//	DevScr.cpp : implementation file
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
//#define NH_DEBUG
#include ".\Common\CmnLib.h"
#include ".\Common\NHDbgApi.h"
#include ".\Common\ConstDef.h"
#include ".\Common\ScreenDef.h"

#include ".\Dll\nhmwi.h"

#include ".\Dev\DevDefine.h"
#include ".\Dev\DevCmn.h"

#include ".\Scr\ScrCtrl.h"

#include ".\Tran\TranCmnDefine.h"	// [#463] [NH] KSK 2008.12.10

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	DBG_CALL		1
#define DBG_INFO		1

/////////////////////////////////////////////////////////////////////////////
//	SCR FUNCTION(SCR) : SCR SET MODE
/////////////////////////////////////////////////////////////////////////////

// [#573] NH AIREAT 2009.10.06 iTM 소스 작업
CScrCtrl* CDevCmn::GetCurrentScreen()
{
	return m_pCurScrCtrl;
}
// end of [#573]

// [#573] NH AIREAT 2009.10.06 iTM 소스 작업
int CDevCmn::fnSCR_ClearUserInputData()
{
	m_pCurScrCtrl->ClearUserInputData();

	return 0;
}
// end of [#573]

// [#573] NH AIREAT 2009.10.06 iTM 소스 작업
int CDevCmn::fnSCR_SetVariable(CString Command, CString Data)
{
	return (int)m_pCurScrCtrl->SetVariable(Command, Data);
}
// end of [#573]

CScrCtrl* CDevCmn::GetScreenByType(NH_SCR_TYPE type)
{
	if (type < 0 || type >= SCR_LOCATION_MAX)
		return NULL;

	return m_arScrCtrl[type];
}


int CDevCmn::fnSCR_SetKeyEvent(CString Key)
{
	return (int)m_pCurScrCtrl->SetKeyEvent(Key);
}

int CDevCmn::fnSCR_SetByPassData(CString Key)
{
	return (int)m_pCurScrCtrl->SetByPassData(Key);
}

int CDevCmn::fnSCR_SetActiveMode(NH_SCR_TYPE type)
{
	if (type < 0 || type >= SCR_LOCATION_MAX)
		return 0;

	if (m_arScrCtrl[type] == NULL)
		return 0;

	m_pCurScrCtrl = m_arScrCtrl[type];
	m_pCurScrCtrl->SetActiveScreen();

	return 1;
}
// end of [#573]

// [#573] NH AIREAT 2009.10.06 iTM 소스 작업
CString	CDevCmn::fstrSCR_GetStringFromTextID(CString strTextID)
{
	//return m_pCurScrCtrl->GetAPTextIDString(strTextID);
	return m_MultiString.GetAPTextIDString(strTextID);
}
// end of [#573]

CString	CDevCmn::fstrSCR_GetStringFromTextID(CString strTextID, int dccCustomer, BOOL isVisa)
{
	CString customTextId;
	CString value;
	
	CString sOwner = MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_OPSURCHARGEOWNER);
	sOwner.TrimLeft();
	sOwner.TrimRight();

	customTextId.Format(L"%s.%d.%s", strTextID, dccCustomer, isVisa ? L"visa" : L"mc");
	if (m_MultiString.HasAPTextID(customTextId))
	{
		value = m_MultiString.GetAPTextIDString(customTextId);
		goto format;
	}

	customTextId.Format(L"%s.%d", strTextID, dccCustomer);
	if (m_MultiString.HasAPTextID(customTextId))
	{
		value = m_MultiString.GetAPTextIDString(customTextId);
		goto format;
	}

	value = m_MultiString.GetAPTextIDString(strTextID);

format:

	value.Replace(_T("X_OWNER"), sOwner);
	value.Replace(_T("X_CURR"), CURRENCY_TYPE);
	return value;
}

// [#2220] AU KMK 2014.02.10 "APTextID:" 문자열 불필요한 GetString 함수 추가
CString	CDevCmn::fstrSCR_GetStringByTextID(CString strTextID)
{
#if (US_VERSION)	// AU인 경우에는 Text 문자열을 그대로 검색할 경우 AU_ENG 검색이 안되는 현상이 있어 국가 Define 처리함
	// in case of ENGLISH, use textID directly since it is same as its US_ENG value for this function.
	if (m_nCurrentLangMode == ENG_MODE)
		return strTextID;
#endif

	return m_MultiString.GetStringByTextID(strTextID);
}
// end of [#2220]

// [#573] NH AIREAT 2009.10.06 iTM 소스 작업
int CDevCmn::fnSCR_SetScrOwner(NH_SCR_TYPE type, CScrCtrl *pScr)
{
	if (type < 0 || type >= SCR_LOCATION_MAX)
		return 0;

	m_arScrCtrl[type] = pScr;

	return 0;
}
// end of [#573]

// Clear Error Code
int	CDevCmn::fnSCR_ClearErrorCode()
{
	NHDEBUG(1, (_T("***DevScr***CDevCmn::fnSCR_ClearErrorCode() \r\n")));

	int		nReturn = TRUE;

	return nReturn;
}

// Clear Key Data
int	CDevCmn::fnSCR_ClearKeyData()
{
	NHDEBUG(1, (_T("***DevScr***CDevCmn::fnSCR_ClearKeyData() \r\n")));

	int		nReturn = FALSE;

//	nReturn = m_pMwi->ScrClearKeyData();

	return nReturn;
}

// Set Display Data(Display Data Name, Display Data Value)
int	CDevCmn::fnSCR_SetDisplayData(CString szSetDisplayDataName, CString szSetDisplayDataValue)			// [#65] NH AIREAT 2008.4.1 파라미터 CString으로 변경.
{
	NHUIDBG(1, (_T("***DevScr***CDevCmn::fnSCR_SetDisplayData() szSetDisplayDataName(%s) szSetDisplayDataValue(%s) \r\n"), szSetDisplayDataName, szSetDisplayDataValue));

	m_pCurScrCtrl->SetVariable(szSetDisplayDataName, szSetDisplayDataValue);

	return T_OK;
}

// Set Current Language Mode
int	CDevCmn::fnSCR_SetCurrentLangMode(int nLangMode)
{
	NHDEBUG(1, (_T("***DevScr***CDevCmn::fnSCR_SetCurrentLangMode() nLangMode=[%d] \r\n"), nLangMode));
	NHDEBUG(1, (_T("  Language Mode Changed on Screen [%d]\r\n"),  Asc2Int(m_pCurScrCtrl->GetCurrentScreenNumber())   ));

	m_nCurrentLangMode = nLangMode;
	m_MultiString.SetLocale(m_nCurrentLangMode);
	
	switch (m_nCurrentLangMode)
	{
		case ENG_MODE:	fnSCR_SetDisplayData(L"kindLanguage", L"ENGLISH");	break;
		case SPN_MODE:	fnSCR_SetDisplayData(L"kindLanguage", L"SPANISH");	break;
		case FRN_MODE:	fnSCR_SetDisplayData(L"kindLanguage", L"FRENCH");	break;
		case CHN_MODE:	fnSCR_SetDisplayData(L"kindLanguage", L"CHINESE");	break;
		case KOR_MODE:	fnSCR_SetDisplayData(L"kindLanguage", L"KOREAN");	break;
		case JPN_MODE:	fnSCR_SetDisplayData(L"kindLanguage", L"JAPANESE");	break;
		default:		fnSCR_SetDisplayData(L"kindLanguage", L"ENGLISH");	break;
	}
	return T_OK;
}
	
// [#2186] US KMK 2013.04.10 Asian Language Support
// 아시아 언어 선택 시 명세표는 영문으로 출력해야 함 (SPR 아시아폰트 미지원)
// fnSCR_SetCurrentLangMode() 사용하면 화면 문구까지 영문으로 바뀌므로
// MultiString의 Locale만 변경할 수 있는 별도 함수 생성
//int	CDevCmn::fnSCR_SetTextLocale(int nLangMode)
//{
//	NHDEBUG(1, (_T("***DevScr***CDevCmn::fnSCR_SetTextLocale() nLang(%d) \r\n"), nLangMode));
//
//	m_MultiString.SetLocale(nLangMode);
//
//	return T_OK;
//}
// end of [#2186]

/////////////////////////////////////////////////////////////////////////////
//	SCR FUNCTION(SCR) : SCR GET MODE
/////////////////////////////////////////////////////////////////////////////
// Get Error Code
CString CDevCmn::fstrSCR_GetErrorCode()
{
	NHDEBUG(1, (_T("***DevScr***CDevCmn::fstrSCR_GetErrorCode() \r\n")));

	CString strReturn("");

	strReturn += "0000000";										// Error Code Adjust
	strReturn = strReturn.Left(7);

	return strReturn;
}

// Get Device Status
int	CDevCmn::fnSCR_GetDeviceStatus()
{
	return NORMAL;
}

void CDevCmn::fstrSCR_WaitTime(DWORD dwWaitSec)
{
	//m_pCurScrCtrl->GetUserInputCheck((dwWaitSec * 1000));
	m_eWaitObject.Wait((dwWaitSec * 1000));
}

void CDevCmn::fstrSCR_WaitMiliTime(DWORD dwWaitMili)
{
	m_eWaitObject.Wait(dwWaitMili);
}

// Get Key String(Init Flag)
CString	CDevCmn::fstrSCR_GetKeyString(DWORD dwWaitSec)
{
	NHDEBUG(1, (_T("***DevScr***CDevCmn::fstrSCR_GetKeyString() \r\n")));

	CString strCommand, strData;
	DWORD	dwProcTime = 10;
	DWORD	dwElapsed = 0;

	if (dwWaitSec == INFINITE)
		dwWaitSec = MAX_DEVSCR_TIME * 1000;
	else
		dwWaitSec *= 1000;

	while (1)
	{
		if (m_pCurScrCtrl->GetUserInputCheck(5))
		{
			m_pCurScrCtrl->GetUserInputData(strCommand, strData);

			NHDEBUG(1, (_T("***DevScr***CDevCmn::fstrSCR_GetKeyData() m_Data(%s) \r\n"), strData));

			break;
		}

		Delay_Msg(dwProcTime);

		dwElapsed += dwProcTime + 5;
		if (dwElapsed >= dwWaitSec)
		{
			strData = S_TIMEOVER;
			break;
		}
	}

	return strData;
}

// Get Check Screen(Screen No, Language Mode)
CString	CDevCmn::fstrSCR_GetCheckScreen(int nScreenNo, int nLangMode)
{
	NHDEBUG(1, (_T("***DevScr***CDevCmn::fstrSCR_GetCheckScreen() \r\n")));

	CString	strReturn("");

	if (nScreenNo > 0)
		strReturn.Format(L"%03d", nScreenNo);

	return strReturn;
}

// Get Current Screen No
int	CDevCmn::fnSCR_GetCurrentScreenNo()
{
	return Asc2Int(m_pCurScrCtrl->GetCurrentScreenNumber());	// KSK 2008.9.18 Screen 에서 읽어오도록 수정
}

// Get Current Language Mode
int	CDevCmn::fnSCR_GetCurrentLangMode()
{
	return m_nCurrentLangMode;
}

// [#2186] US KSK 2013.04.30
int	CDevCmn::fnSCR_IsFontLoaded(int nLanguage)
{
	// KMK 2013.05.06 IsFontLoaded()와 관련있는 ExtraFont 값은 현재 AP.dat(SCR_FRONT)에만 존재함.
	// [#2339] NH Justin 2015.04.09 Check FONT and necessary files to enable Asian Language....
	//return m_arScrCtrl[SCR_FRONT]->IsFontLoaded(nLanguage);

	if(m_arScrCtrl[SCR_FRONT]->IsFontLoaded(nLanguage))
	{
		// 1500SE Master file includes Asian Language graphics by default
		if( m_pConfig->GetOSVersion() == NH_OS_NH1500SE )
			return TRUE;

		// [#2518] US Kook 2018.01.27 Support MX-2800SE		// [#GLDV-2505] Support MX-2800T
		if( m_pConfig->GetOSVersion() == NH_OS_MX2800SE || m_pConfig->GetOSVersion() == NH_OS_MX2800_F_T )
			return TRUE;

		// [#2533] US Justin 2018.02.27 Language Detection Logic Change <= Removing Button Image
		/*
		CString strLangFile = L"";
		if( nLanguage == (CHN_MODE-1) )			strLangFile = L"AP_Button_Chinese_US_ENG.jpg";
		else if( nLanguage == (KOR_MODE-1) )	strLangFile = L"AP_Button_Korean_US_ENG.jpg";
		else if( nLanguage == (JPN_MODE-1) )	strLangFile = L"AP_Button_Japanese_US_ENG.jpg";
		if(strLangFile.GetLength() < 2)
			return FALSE;

		CString strFolderName = L"1024_600";
		if( (m_pConfig->GetOSVersion() == NH_OS_NH1800SE_B) || (m_pConfig->GetOSVersion() == NH_OS_NH1800_POS) )
			strFolderName = L"800_600";

		CString strLanguageButton = _T("\\ATM\\SCREEN\\") + strFolderName + _T("\\") + strLangFile;
		return IsExistFile(strLanguageButton);
		*/

		CString strFontFile = L"";
		if( nLanguage == (CHN_MODE-1) )			strFontFile = ASIAN_FONT_DEST_PATH L"\\Simsun.ttc";
		else if( nLanguage == (KOR_MODE-1) )	strFontFile = ASIAN_FONT_DEST_PATH L"\\gulim_1_30.ttc";
		else if( nLanguage == (JPN_MODE-1) )	strFontFile = ASIAN_FONT_DEST_PATH L"\\Simsun.ttc";
		return IsExistFile(strFontFile);
		// End of [#2533]
	}
	return FALSE;
	// End of [#2339]
}
// end of [#2186]

/////////////////////////////////////////////////////////////////////////////
//	SCR FUNCTION(SCR) : SCR OPERATION MODE
/////////////////////////////////////////////////////////////////////////////
// Initialize
int	CDevCmn::fnSCR_Initialize()
{
	NHDEBUG(1, (_T("***DevScr***CDevCmn::fnSCR_Initialize() \r\n")));

	return NORMAL;
}

// De-initialize
int	CDevCmn::fnSCR_Deinitialize()
{
NHDEBUG(1, (_T("***DevScr***CDevCmn::fnSCR_Deinitialize() \r\n")));

	return NORMAL;
}

void CDevCmn::fnSCR_ShowScreen(int nScreenNo)
{
	fnSCR_DisplayPrevSet(nScreenNo);
	fnSCR_DisplayScreen(nScreenNo);
}

// Display Pre Set(Screen No)
int	CDevCmn::fnSCR_DisplayPrevSet(int nScreenNo)
{
	NHDEBUG(1, (_T("***DevScr***CDevCmn::fnSCR_DisplayPrevSet() nScreenNo(%d) \r\n"), nScreenNo));

	int		nReturn = FALSE;

	CString	strTempCheckName = fstrSCR_GetCheckScreen(nScreenNo, m_nCurrentLangMode);

	nReturn = fnSCR_SetDisplayData(L"PrevSetScreenNo", strTempCheckName);

	return nReturn;
}

// Display Screen(Screen No, Input Time, Pin Input Mode, Card Data)
int	CDevCmn::fnSCR_DisplayScreen(int nScreenNo, int nInputSec, int nPinInputMode, LPCTSTR szCardData, int nPinPassWordMin, int nPinPassWordMax, int nPinPassWordAuto, LPCTSTR szPinPassWordTerm, LPCTSTR szEnabledKey)
{
	NHDEBUG(1, (_T("***DevScr***CDevCmn::fnSCR_DisplayScreen() nScreenNo(%d) \r\n"), nScreenNo));

	int		nReturn = FALSE;
	CString	strTempCheckName = fstrSCR_GetCheckScreen(nScreenNo, m_nCurrentLangMode);

	//////////////////////////////
	// 1. Display Screen

	if (strTempCheckName.GetLength() > 0)
	{
		NHDEBUG(DBG_INFO, (_T("SCREEN DISPLAY [%s]\n"), strTempCheckName));
		if (nInputSec > 0)
		{
			nReturn = fnSCR_SetDisplayData(L"APTimeoutMode", L"TimeoutLast");
			nReturn = fnSCR_SetDisplayData(L"APMaxTime", Int2Asc(nInputSec));
		}

		switch (m_nCurrentLangMode)
		{
			case ENG_MODE:	fnSCR_SetDisplayData(L"kindLanguage", L"ENGLISH");	break;
			case SPN_MODE:	fnSCR_SetDisplayData(L"kindLanguage", L"SPANISH");	break;
			case FRN_MODE:	fnSCR_SetDisplayData(L"kindLanguage", L"FRENCH");	break;
			case CHN_MODE:	fnSCR_SetDisplayData(L"kindLanguage", L"CHINESE");	break;
			case KOR_MODE:	fnSCR_SetDisplayData(L"kindLanguage", L"KOREAN");	break;
			case JPN_MODE:	fnSCR_SetDisplayData(L"kindLanguage", L"JAPANESE");	break;
			default:		fnSCR_SetDisplayData(L"kindLanguage", L"ENGLISH");	break;
		}

		nReturn = fnSCR_SetDisplayData(L"APCenterMovie", strTempCheckName);

#if 0
		// wait for "LOAD_OK" string
		CString	strCommand, strData;
		while(1)
		{
			if (m_pCurScrCtrl->GetUserInputCheck(5))
			{
				m_pCurScrCtrl->GetUserInputData(strCommand, strData);
				if (strData == L"LOAD_OK")
					break;
			}

			m_eWaitObject.Wait(10);
		}

		//m_eWaitObject.Wait(200);
#endif
	}

	//////////////////////////////
	// 2. Pin Enable

	if (nPinInputMode)
	{
		NHDEBUG(DBG_INFO, (_T("PIN ENABLE_MODE [%d]\n"), nPinInputMode));
		fnAPL_DeviceEnDisable(DEV_PIN, 
							  ENABLE, 
							  FALSE, 
							  nPinInputMode, 
							  szCardData, 
							  nPinPassWordMin, 
							  nPinPassWordMax, 
							  nPinPassWordAuto, 
							  szPinPassWordTerm, 
							  szEnabledKey);
		
		if(nPinInputMode != PIN_DISABLE_MODE)
			Delay_Msg(50);
	}

	return nReturn;
}

int CDevCmn::fnSCR_DisplayAdaPrevSet(int nScreenNo)
{
	CString	strTempCheckName = fstrSCR_GetCheckScreen(nScreenNo, m_nCurrentLangMode);

	return fnSCR_SetDisplayData(L"APUpdateStart", strTempCheckName);
}

int	CDevCmn::fnSCR_DisplayAdaScreen(int nScreenNo, int nInputSec, int nPinInputMode, LPCTSTR szCardData, int nPinPassWordMin, int nPinPassWordMax, int nPinPassWordAuto, LPCTSTR szPinPassWordTerm, LPCTSTR szEnabledKey)
{
	NHDEBUG(1, (_T("[CDevCmn::fnSCR_DisplayAdaScreen]\n")));

	int		nReturn = FALSE;

	//////////////////////////////
	// 1. Display Screen

	CString	strTempCheckName = fstrSCR_GetCheckScreen(nScreenNo, m_nCurrentLangMode);

	if (strTempCheckName.GetLength() > 0)
	{
		if (nInputSec == KEYIN_TIME_OUT)
			nInputSec = ADA_SCREEN_TIMEOUT;

		if (nInputSec > 0)
		{
			nReturn = fnSCR_SetDisplayData(L"APTimeoutMode", L"TimeoutLast");
			nReturn = fnSCR_SetDisplayData(L"APMaxTime", Int2Asc(nInputSec));
		}

		nReturn = fnSCR_SetDisplayData(L"APUpdate", strTempCheckName);
		//Delay_Msg(100);
		m_pCurScrCtrl->GetUserInputCheck(100);
	}	

	//////////////////////////////
	// 2. Pin Enable
	if (nPinInputMode)
	{
		fnAPL_DeviceEnDisable(DEV_PIN, 
							  ENABLE, 
							  FALSE, 
							  nPinInputMode, 
							  szCardData, 
							  nPinPassWordMin, 
							  nPinPassWordMax, 
							  nPinPassWordAuto, 
							  szPinPassWordTerm, 
							  szEnabledKey);
		
		if(nPinInputMode != PIN_DISABLE_MODE)
			Delay_Msg(50);
	}

	return nReturn;
}

// Display Update(Screen No, Input Time, Pin Input Mode, Card Data)
int	CDevCmn::fnSCR_DisplayUpdate(int nScreenNo)
{
	NHDEBUG(1, (_T("***DevScr***CDevCmn::fnSCR_DisplayUpdate() nScreenNo(%d) \r\n"), nScreenNo));

	int		nReturn = FALSE;

	nReturn = fnSCR_SetDisplayData(L"APUpdate", L"SCREEN");
																// Screen Update
	return nReturn;
}

// Display Message()
int	CDevCmn::fnSCR_DisplayMessage()
{
	NHDEBUG(1, (_T("***DevScr***CDevCmn::fnSCR_DisplayMessage() \r\n")));

	int		nReturn = FALSE;

	nReturn = fnSCR_SetDisplayData(L"APUpdate", L"MESSAGE");

	return nReturn;
}


// Display Image(Image No, OnOff Flag)
int	CDevCmn::fnSCR_DisplayImage(int nImageNo, int nOnOffFlag)
{
	NHUIDBG(1, (_T("***DevScr***CDevCmn::fnSCR_DisplayImage() nImageNo(%d) nOnOffFlag(%d) \r\n"), nImageNo, nOnOffFlag));

	int		nReturn = FALSE;
	CString strTemp;

	if (nImageNo > 0)
	{
		strTemp.Format(L"%s%d", SCR_BTNONOFF, nImageNo);
		nReturn = fnSCR_SetDisplayData(strTemp, ((nOnOffFlag) ? L"on" : L"off"));
	}

	return nReturn;
}

// Display Image(Image No, OnOff Flag)
int	CDevCmn::fnSCR_DisplayImage(int nImageNo, CString szString)
{
	NHUIDBG(1, (_T("***DevScr***CDevCmn::fnSCR_DisplayImage() nImageNo(%d) nOnOffFlag(%s) \r\n"), nImageNo, szString));

	int		nReturn = FALSE;
	CString strTemp;

	if (nImageNo > 0)
	{
		strTemp.Format(L"%s%d", SCR_BTNONOFF, nImageNo);
		nReturn = fnSCR_SetDisplayData(strTemp, szString);
	}

	return nReturn;
}

// Display String(String No, String)
int	CDevCmn::fnSCR_DisplayString(int nStringNo, CString szString)
{
	NHUIDBG(1, (_T("***DevScr***CDevCmn::fnSCR_DisplayString() nStringNo(%d) szString(%s) \r\n"), nStringNo, (CString)szString));

	int		nReturn = FALSE;
	CString strTemp("");

	if (nStringNo > 0)
	{
		strTemp.Format(L"%s%d", SCR_STRING, nStringNo);
		nReturn = fnSCR_SetDisplayData(strTemp, szString);
	}

	return nReturn;
}

// Display String(String No, String)
int	CDevCmn::fnSCR_DisplayFormat(int nStringNo, LPCWSTR szFmt, ...)
{
	NHDEBUG(1, (_T("***DevScr***CDevCmn::fnSCR_DisplayFormat() nStringNo(%d) szFmt(%s) \r\n"), nStringNo, (CString)szFmt));

	int		nReturn = FALSE;
	CString strTemp("");
	CString strValue("");

	if (nStringNo > 0)
	{
		va_list args;
		va_start(args, szFmt);
		strValue.FormatV(szFmt, args);
		va_end(args);

		strTemp.Format(L"%s%d", SCR_STRING, nStringNo);
		nReturn = fnSCR_SetDisplayData(strTemp, strValue);
	}

	return nReturn;
}

int CDevCmn::fnSCR_DisplayHistory(int nCount)
{
	CString strTemp;

	strTemp.Format(L"%d", nCount);
	fnSCR_SetDisplayData(SCR_HISTORY, strTemp);

	return TRUE;
}

int CDevCmn::fnSCR_DisplayHistory(int nStringNo, CString szString)
{
	NHDEBUG(1, (_T("***DevScr***CDevCmn::fnSCR_DisplayHistory() nStringNo(%d) szString(%s) \r\n"), nStringNo, (CString)szString));

	int		nReturn = FALSE;
	CString strTemp("");

	if (nStringNo > 0)
	{
		strTemp.Format(L"%s%d", SCR_HISTORY, nStringNo);
		nReturn = fnSCR_SetDisplayData(strTemp, szString);
	}

	return nReturn;
}

// Display String (String No, String)
int	CDevCmn::fnSCR_DisplayStringArray(int nStringNo, LPCTSTR szString)
{
	NHUIDBG(1, (_T("***DevScr***CDevCmn::fnSCR_DisplayStringArray() nStringNo(%d) szString(%s) \r\n"), nStringNo, szString));

	int		nReturn = FALSE;
	CString strTemp;
	CString strErrMsg;

	if (nStringNo > 0)
	{
		// [#65] NH AIREAT 2008.4.1 파라미터 CString으로 변경.
		
		strTemp.Format(L"%s%d", SCR_STRING, nStringNo);
		strErrMsg = szString;
		strErrMsg.Replace(L"\n", SCR_CMD_DELIMITER);
		
		nReturn = fnSCR_SetDisplayData(strTemp, strErrMsg);

		// end of [#65]
	}

	return nReturn;
}

// Scan Device Action(Device Id, Check Time, Event Kind)
int	CDevCmn::fnSCR_ScanDeviceAction(int nDevId, int nScanTime, int nEventKind)
{
	if (m_pCurScrCtrl->GetUserInputCheck(nScanTime) == TRUE)
	{
		//Delay_Msg(100);
		return nDevId;
	}

	return 0;
}

int	CDevCmn::fnSCR_SetDebugMessage(CString strDbgMsg)
{
	fnSCR_SetDisplayData(L"APDebugMsg", strDbgMsg);

	return 0;
}

void CDevCmn::fnSCR_SaveScreenshotToSD()
{
	if (!IsExistFile(SCREENSHOTS_PATH))
	{
		return;
	}

	SYSTEMTIME time;
	CString filename;

	GetSystemTime(&time);
	filename.Format(L"%s\\MoniPlusCE2-%04d%02d%02d-%02d%02d%02d.bmp",
		SCREENSHOTS_PATH,
		time.wYear, time.wMonth, time.wDay,
		time.wHour, time.wMinute, time.wSecond);
	
	SaveScreenToFile(filename);
}