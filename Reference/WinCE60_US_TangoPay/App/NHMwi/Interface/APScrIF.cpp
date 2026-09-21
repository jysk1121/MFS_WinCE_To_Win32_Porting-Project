// APSCRIF.cpp

#include "stdafx.h"
#include "..\NHMWI.h"
#include "..\NHMWICtl.h"
#include "..\NHMWIPpg.h"

#include ".\Common\NHDbgApi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// Method : Related SCREEN
///////////////////////////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// PROTOTYPE : long ScrInitialize() 
// DESCRIPT  : 화면출력데이타 처리-화면큐 비움
// PARAMETER : 없음
// RETURN    : TRUE
// ----------------------------------------------------------------------------
long CNHMWICtrl::ScrInitialize() 
{
NHDEBUG(1, (_T("CNHMWICtrl::ScrInitialize()(%s) \n"), "  "));

	m_pDevCtrl->EvtQReset(DEV_SCR);
	return TRUE;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long ScrClearKeyData() 
// DESCRIPT  : 화면출력데이타 처리-화면큐 비움
// PARAMETER : 없음
// RETURN    : TRUE
// REMARK    : ScrInitialize과 똑같음
// ----------------------------------------------------------------------------
long CNHMWICtrl::ScrClearKeyData() 
{
NHDEBUG(1, (_T("CNHMWICtrl::ScrClearKeyData()(%s) \n"), "  "));

	m_pDevCtrl->EvtQReset(DEV_SCR);
	return TRUE;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : BSTR ScrGetKeyString(long nWaitSec) 
// DESCRIPT  : 화면에서 입력받은 데이타를 조회한다.
// PARAMETER : nWaitSec : 대기시간
// RETURN    : 키값
// ----------------------------------------------------------------------------
BSTR CNHMWICtrl::ScrGetKeyString(long nWaitSec) 
{
NHDEBUG(1, (_T("CNHMWICtrl::ScrGetKeyString()(%s) \n"), "  "));

	CString strResult("");
	strResult = m_pDevCtrl->GetScrKeyString(nWaitSec);

NHDEBUG(1, (_T("CNHMWICtrl::ScrGetKeyString() strResult(%s) \n"), strResult));
	return strResult.AllocSysString();
}

// ----------------------------------------------------------------------------
// PROTOTYPE : BSTR CNHMWICtrl::ScrGetKeyData(long nWaitSec) 
// DESCRIPT  : 화면데이타를 조회한다
// PARAMETER : nWaitSec - 대기시간
// RETURN    : 키값
// REMARK    : ScrGetKeyString과 똑같은 함수
// ----------------------------------------------------------------------------
BSTR CNHMWICtrl::ScrGetKeyData(long nWaitSec) 
{
NHDEBUG(1, (_T("CNHMWICtrl::ScrGetKeyData()(%s) \n"), "  "));

	CString strResult("");
	strResult = m_pDevCtrl->GetScrKeyString(nWaitSec);

NHDEBUG(1, (_T("CNHMWICtrl::ScrGetKeyData() strResult(%s) \n"), strResult));
	return strResult.AllocSysString();
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long ScrSetData(LPCTSTR szDataName, LPCTSTR szDataValue) 
// DESCRIPT  : 화면에서 입력받은 값을 데이타큐에 저장한다.
// PARAMETER : 데이타이름, 데이타값
// RETURN    : TRUE
// ----------------------------------------------------------------------------
long CNHMWICtrl::ScrSetData(LPCTSTR szDataName, LPCTSTR szDataValue) 
{
NHDEBUG(1, (_T("CNHMWICtrl::ScrSetData() szDataName(%s) szDataValue(%s) \n"), szDataName, szDataValue));

	CString strDataName(szDataName);							// 2004.08.19

	char szTemp[128];
	memset(szTemp, NULL, sizeof(szTemp));
	WideToMulti(szTemp, szDataName, sizeof(szTemp));

	char szTemp2[1024];
	memset(szTemp2, NULL, sizeof(szTemp2));
	WideToMulti(szTemp2, szDataValue, sizeof(szTemp2));

	m_pDevCtrl->EvtQPutData("EVENT_USERDATA", DEVNM_SCR, szTemp, szTemp2);

	return 0;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : BSTR CNHMWICtrl::ScrGetScreenData(long nWaitSec) 
// DESCRIPT  : 화면에서 얻은 데이타를 조회하여 리턴한다.
// PARAMETER : nWaitSec - 조회중 데이타를 대기하는 시간(초단위)
// RETURN    : 조회된 데이타(문자열)
// REMARK    : 20030907_1.0_1071 : 정리작업(화면데이타 조회 인터페이스 수정-한정현팀장요청)
// ----------------------------------------------------------------------------
BSTR CNHMWICtrl::ScrGetScreenData(long nWaitSec) 
{
NHDEBUG(1, (_T("CNHMWICtrl::ScrGetScreenData() nWaitSec(%d)  \n"), nWaitSec));

	CString strResult("");
	strResult = m_pDevCtrl->GetScrKeyString(nWaitSec);

NHDEBUG(1, (_T("CNHMWICtrl::ScrGetScreenData() nWaitSec(%d) strResult(%s) \n"), nWaitSec, strResult));
	return strResult.AllocSysString();
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long CNHMWICtrl::ScrSetScreenData(LPCTSTR szDataName, LPCTSTR szDataValue) 
// DESCRIPT  : 화면으로부터 넘어온 데이타를 MWI에 전달한다.
// PARAMETER : 데이타명/데이타값
// RETURN    : 0(무의미함)
// REMARK    : 20030907_1.0_1071 : 정리작업(화면데이타 조회 인터페이스 수정-한정현팀장요청)
// ----------------------------------------------------------------------------
long CNHMWICtrl::ScrSetScreenData(LPCTSTR szDataName, LPCTSTR szDataValue) 
{
NHDEBUG(1, (_T("CNHMWICtrl::ScrSetScreenData() szDataName(%s) szDataValue(%s)  \n"), szDataName, szDataValue));

	CString strDataName(szDataName);							// 2004.08.19

	char szTemp[64];
	memset(szTemp, NULL, sizeof(szTemp));
	WideToMulti(szTemp, szDataName, sizeof(szTemp));

	char szTemp2[256];
	memset(szTemp2, NULL, sizeof(szTemp2));
	WideToMulti(szTemp2, szDataValue, sizeof(szTemp2));

	m_pDevCtrl->EvtQPutData("EVENT_USERDATA", DEVNM_SCR, szTemp, szTemp2);

	return 0;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long CNHMWICtrl::ScrSetDisplayData(LPCTSTR szDataName, LPCTSTR szDataValue) 
// DESCRIPT  : MWI로 화면출력을 요청한다.
// PARAMETER : 데이타이름/데이타 값
// RETURN    : 
// REMARK    : 20030907_1.0_1071 : 정리작업(화면데이타 조회 인터페이스 수정-한정현팀장요청)
// ----------------------------------------------------------------------------
long CNHMWICtrl::ScrSetDisplayData(LPCTSTR szDataName, LPCTSTR szDataValue) 
{
NHDEBUG(1, (_T("CNHMWICtrl::ScrSetDisplayData() szDataName(%s) szDataValue(%s)  \n"), szDataName, szDataValue));

	FireOnDisplayData(szDataName, szDataValue);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Method : Related SCREEN
///////////////////////////////////////////////////////////////////////////////////////////////////


// ----------------------------------------------------------------------------
// PROTOTYPE : long ScrDisplayScreen(LPCTSTR szTag, long nIndex, LPCTSTR szData) 
// DESCRIPT  : 화면출력 요청처리
// PARAMETER : 테그. 인덱스. 값
// RETURN    : 0
// REMARK    : 다시 처리를 이벤트로 요청한다.
// ----------------------------------------------------------------------------
long CNHMWICtrl::ScrDisplayScreen(LPCTSTR szTag, long nIndex, LPCTSTR szData) 
{
NHDEBUG(1, (_T("CNHMWICtrl::ScrDisplayScreen() szTag(%s) nIndex(%d) szData(%s)  \n"), szTag, nIndex, szData));

	FireOnDisplayScreen(szTag, nIndex, szData);
	return 0;
}


