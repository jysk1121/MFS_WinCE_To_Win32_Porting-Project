// DEVRDOORIF.cpp

#include "stdafx.h"

//#define NH_DEBUG
#include ".\Common\NHDbgApi.h"

#include "..\NHMWI.h"
#include "..\NHMWICtl.h"
#include "..\NHMWIPpg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// Method : Call to REARDOOR
///////////////////////////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// 함 수 명 : BOOL CNHMWICtrl::DevDoorGetDoorSwitch()
// DESCRIPT : 뒷문의 열림상태를 조회한다.
// ----------------------------------------------------------------------------
BOOL CNHMWICtrl::DevDoorGetDoorSwitch()
{
	BOOL bResult = TRUE;
	CString strStatus = m_pDevCtrl->GetDeviceStatus(_T("RDOORStSafeStatus"));

	if (strStatus.CompareNoCase(_T("OPEN")) == 0)
		bResult = FALSE;
	
	return bResult;
}

// ----------------------------------------------------------------------------
// 함 수 명 : int CNHMWICtrl::DevDoorOpenConnection()
// DESCRIPT : 세션연결
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevDoorOpenConnection()
{
	m_bDoorOpenedEvent = FALSE;

	NHDEBUG(1, (_T("OpenSessionSync() ... \n")));
	int nReturn = m_pDoor.OpenSessionSync(K_30_WAIT*1000);
	NHDEBUG(1, (L"\n"));
	NHDEBUG(1, (_T("OpenSessionSync() ... DONE (%d)\n"), nReturn));

	if (nReturn == R_NORMAL)
	{
		DevDoorUpdateStatus();
		m_bDoorOpenedEvent = TRUE;
	}

	return nReturn;
}

// ----------------------------------------------------------------------------
// 함 수 명 : int CNHMWICtrl::DevDoorCloseConnection()
// DESCRIPT : 세샨종료
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevDoorCloseConnection()
{
	// 상태정보를 갱신한다.
	DevDoorUpdateStatus();

	m_bDoorClosedEvent = FALSE;									// 2005.03.30
	return m_pDoor.CloseSessionSync();
}

// ----------------------------------------------------------------------------
// PROTOTYPE : BSTR DorGetErrorCode() 
// DESCRIPT  : 장애코드 조회
// PARAMETER : 없음
// RETURN    : 장애코드
// ----------------------------------------------------------------------------
BSTR CNHMWICtrl::DorGetErrorCode() 
{
	return DevDoorGetErrorCode().AllocSysString();
}

// ----------------------------------------------------------------------------
// PROTOTYPE : int DevDoorGetStatus()
// DESCRIPT  : 장애코드를 조회하여 장치상태를 조회한다.
// PARAMETER : 없음
// RETURN    : NORMAL(0), DOWN(2)
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevDoorGetStatus()
{
	CString strResult("");
	int nResult = 0;

	// 장애코드를 확보한다.
	strResult = DevDoorGetErrorCode();
	char szTemp[64];
	memset(szTemp, NULL, sizeof(szTemp));
	WideToMulti(szTemp, strResult, sizeof(szTemp));

	if (strResult.IsEmpty())	
		nResult = NORMAL;
	else if (IsZero(szTemp, strlen(szTemp)))	
		nResult = NORMAL;
	else 
		nResult = DOWN;

	return nResult;
}

// ----------------------------------------------------------------------------
// 함 수 명 : int CNHMWICtrl::DevDoorEnableStatusEvents()
// DESCRIPT : 상태변경 이벤트 발생을 가능하게 한다.
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevDoorEnableStatusEvents()
{
	int nResult = 0;

	// 상태정보를 갱신한다.
	DevDoorUpdateStatus();

//	nResult = m_pDoor.EnableSafeEvents();
//	nResult = m_pDoor.EnableCabinetEvents();					// 2004.06.30
//	nResult = m_pDoor.EnableShieldEvents();						// 2005.05.26
	return nResult;
}

// ----------------------------------------------------------------------------
// 함 수 명 : int CNHMWICtrl::DevDoorDisableStatusEvnets()
// DESCRIPT : 상태변경 이벤트 발생을 불가하게 한다.
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevDoorDisableStatusEvnets()
{
	int nResult = 0;

	// 상태정보를 갱신한다.
	DevDoorUpdateStatus();

//	nResult = m_pDoor.DisableShieldEvents();					// 2005.05.26
//	nResult = m_pDoor.DisableCabinetEvents();					// 2004.06.30
//	nResult = m_pDoor.DisableSafeEvents();

	return nResult;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : int DevDoorClearErrorCode() 
// DESCRIPT  : 뒷문의 장애코드를 클리어한다.
// PARAMETER : 없음
// RETURN    : NORMAL(0)
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevDoorClearErrorCode() 
{																// 센서
	RegSetValueExt(_REGKEY_DEVERROR, _T("RDOOR"), REG_STR, 0, NULL);	
	return NORMAL;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : BSTR DevDoorGetErrorCode() 
// DESCRIPT  : 장애코드를 얻어온다.
// PARAMETER : 없음
// RETURN    : 장애코드
// ----------------------------------------------------------------------------
CString CNHMWICtrl::DevDoorGetErrorCode() 
{
	CString strResult("");
//	TCHAR szCode[256];		memset(szCode, 0x00, sizeof(szCode));
//	int nResult = 0;
//
//	// 레지스트리에서 장애코드를 확보한다.
//	nResult = RegQueryValueExt(_REGKEY_DEVERROR, _T("RDOOR"), REG_STR, sizeof(szCode), (LPVOID)szCode);
//	if (nResult)
//	{
//		strResult.Format(_T("%S0000000"), szCode);					// 2004.06.19
//		strResult = strResult.Left(7);							// 에러코드보정	: 2004.06.19
//	}

	return strResult;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Event : Accepted From REARDOOR
///////////////////////////////////////////////////////////////////////////////////////////////////

void CNHMWICtrl::OnFatalErrorNxdoors(LPCTSTR Action, long Result) 
{
	DevDoorUpdateStatus();
}

void CNHMWICtrl::OnDeviceErrorNxdoors(LPCTSTR Action, long Result) 
{
	DevDoorUpdateStatus();
}

void CNHMWICtrl::OnLockGrantedNxdoors() 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnLockReleasedNxdoors() 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnLockTimeoutNxdoors() 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnDeviceStatusChangedNxdoors(LPCTSTR Value) 
{
	DevDoorUpdateStatus();
}

void CNHMWICtrl::OnCabinetChangedNxdoors(LPCTSTR Status) 
{
	DevDoorUpdateStatus();
}

void CNHMWICtrl::OnSafeChangedNxdoors(LPCTSTR Status) 
{
	FireOnSafeDoorChanged(Status);		// [#RWC6-14] US William 2019.09.18 Add Safe Door Status To Journal
	DevDoorUpdateStatus();
}

void CNHMWICtrl::OnShieldChangedNxdoors(LPCTSTR Status) 
{
	DevDoorUpdateStatus();
}

void CNHMWICtrl::OnTimeoutNxdoors() 
{
	DevDoorUpdateStatus();
}

void CNHMWICtrl::OnResetCompleteNxdoors() 
{
	DevDoorUpdateStatus();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Method : Call From REARDOOR to REARDOOR
///////////////////////////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// 함 수 명 : int CNHMWICtrl::DevDoorUpdateStatus()
// DESCRIPT : 상태정보를 갱신한다
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevDoorUpdateStatus()
{
																// AP DOWN 장애현상 디버그 : 2003.12.20
	if (!m_bDoorOpened)											// 장치가Open되지 않았으면
		return R_NORMAL;										// 상태갱신은 의미없음

	// 디바이스 상태와 매체 상태를 업데이트 한다.
	CString strStatusSafe = m_pDoor.GetSafeStatus();
	CString strStatusCabinet = m_pDoor.GetCabinetStatus();

	if ((strStatusSafe.CompareNoCase(_T("OPEN")) == 0)	||		// 2004.06.30
		(strStatusCabinet.CompareNoCase(_T("OPEN")) == 0))
		ProcSetDeviceStatus(L"RDOOR", L"StSafeStatus", _T("OPEN"));
	else
		ProcSetDeviceStatus(L"RDOOR", L"StSafeStatus", strStatusSafe);
	
	if ((m_pCdu) && (m_bCduOpened))								// CDU장치를 사용할 경우. : 2004.04.01
		ProcSetDeviceStatus(L"RDOOR", L"Position", DevCduGetStPosition());

	return R_NORMAL;
}
