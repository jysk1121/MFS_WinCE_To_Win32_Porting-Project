// DEVSENSORIF.cpp

#include "stdafx.h"

//#define NH_DEBUG
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
// Method : Call to SENSOR
///////////////////////////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// DESCRIPT  : 세션연결처리
// PARAMETER : X
// RETURN    : R_NORMAL
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevSnsOpenConnection()
{
	NHDEBUG(1, (_T("OpenSessionSync() ... \n")));
	int nReturn = m_pSensor.OpenSessionSync(K_30_WAIT*1000);
	NHDEBUG(1, (L"\n"));
	NHDEBUG(1, (_T("OpenSessionSync() ... DONE (%d)\n"), nReturn));

	m_bSensorOpenedEvent = TRUE;

	// V05.00.03 SIU 센서갱신처리
	//           ASYNC처리시에는 이벤트핸들러에서 처리하면 되나
	//			 SYNC처리시에는 해당 처리가 완료되는 시점에 처리해 주어야 한다.
	/*
	if (m_pSensor.GetOperatorSwitchStatus().CompareNoCase(L"SUPERVISOR") == 0)
		ProcSetDeviceStatus(L"SENSOR", L"OperatorSwitch", L"SUPERVISOR");
	else
	if (m_pSensor.GetOperatorSwitchStatus().CompareNoCase(L"RUN") == 0)
		ProcSetDeviceStatus(L"SENSOR", L"OperatorSwitch", L"RUN");
	else
		ProcSetDeviceStatus(L"SENSOR", L"OperatorSwitch", m_pSensor.GetOperatorSwitchStatus());
	*/
	ProcSetDeviceStatus(L"SENSOR", L"OperatorSwitch", m_pSensor.GetOperatorSwitchStatus());

	// V05.00.04 ENHANCED AUDIO 센서갱신처리
	/*
	if (m_pSensor.GetEnhancedAudioStatus().CompareNoCase(L"NOTAVAILABLE") == 0)
		ProcSetDeviceStatus(L"SENSOR", L"EnhancedAudio", L"NOTAVAILABLE");
	else
	if (m_pSensor.GetEnhancedAudioStatus().CompareNoCase(L"PRESENT") == 0)
		ProcSetDeviceStatus(L"SENSOR", L"EnhancedAudio", L"PRESENT");
	else
	if (m_pSensor.GetEnhancedAudioStatus().CompareNoCase(L"NOTPRESENT") == 0)
		ProcSetDeviceStatus(L"SENSOR", L"EnhancedAudio", L"NOTPRESENT");
	else
		ProcSetDeviceStatus(L"SENSOR", L"EnhancedAudio", m_pSensor.GetEnhancedAudioStatus());
	*/

	ProcSetDeviceStatus(L"SENSOR", L"EnhancedAudio", m_pSensor.GetEnhancedAudioStatus());

	// PROXIMITY SENSOR 
	/*
	if (m_pSensor.GetProximityStatus().CompareNoCase(L"NOTAVAILABLE") == 0)
		ProcSetDeviceStatus(L"SENSOR", L"Proximity", L"NOTAVAILABLE");
	else
	if (m_pSensor.GetProximityStatus().CompareNoCase(L"PRESENT") == 0)
		ProcSetDeviceStatus(L"SENSOR", L"Proximity", L"PRESENT");
	else
	if (m_pSensor.GetProximityStatus().CompareNoCase(L"NOTPRESENT") == 0)
		ProcSetDeviceStatus(L"SENSOR", L"Proximity", L"NOTPRESENT");
	else
		ProcSetDeviceStatus(L"SENSOR", L"Proximity", m_pSensor.GetProximityStatus());
	*/

	ProcSetDeviceStatus(L"SENSOR", L"Proximity", m_pSensor.GetProximityStatus());

	return R_NORMAL;
}

// ----------------------------------------------------------------------------
// DESCRIPT  : 세션종료처리
// PARAMETER : X
// RETURN    : R_NORMAL
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevSnsCloseConnection()
{
//	m_pSensor.DisableStatusEvents();
//	m_pSensor.DisableOperatorSwitch();
	m_pSensor.CloseSessionSync();
	m_bSensorClosedEvent = TRUE;
	return R_NORMAL;
}

// ----------------------------------------------------------------------------
// DESCRIPT  : 장애코드 초기화
// PARAMETER : X
// RETURN    : NORMAL
// REMARK    : Get로직과 같이 주의하여야 할 사항은, 레지스트리의 경로/Key가 
//			   정확한지 확인하여야 한다.
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevSnsClearErrorCode()
{
	RegSetValueExt(_REGKEY_DEVERROR, _T("SENSOR"), REG_STR, 0, NULL);	// 센서
	return R_NORMAL;
}

// ----------------------------------------------------------------------------
// DESCRIPT  : 장애코드 조회
// PARAMETER : X
// RETURN    : 조회된 장애코드
// REMARK    : Clear로직과 같이 주의하여야 할 사항은, 레지스트리의 경로/Key가 
//			   정확한지 확인하여야 한다.
// ----------------------------------------------------------------------------
CString CNHMWICtrl::DevSnsGetErrorCode()
{
	CString strResult("");
//	TCHAR szCode[256];		memset(szCode, 0x00, sizeof(szCode));
//	int nResult = 0;
//
//	// 레지스트리에서 장애코드를 확보한다.
//	nResult = RegQueryValueExt(_REGKEY_DEVERROR, _T("SENSOR"), REG_STR, sizeof(szCode), (LPVOID)szCode);
//	if (nResult)
//	{
//		strResult.Format(_T("%S0000000"), szCode);					// 2004.06.19
//		strResult = strResult.Left(7);							// 에러코드보정	: 2004.06.19
//	}

	return strResult;
}

// ----------------------------------------------------------------------------
// DESCRIPT  : Device Status Check
// PARAMETER : X
// RETURN    : Always NORMAL - No Property exist So, It treated as "Dummy device"
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevSnsGetDeviceStatus()
{
	return R_NORMAL;
}

// ----------------------------------------------------------------------------
// DESCRIPT  : operator Switch상태조회
// PARAMETER : X
// RETURN    : "RUN"상태가 아니면 TRUE Return
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevSnsGetOperatorSwitch()
{
	if (!m_bSensorOpened)
		return FALSE;
																// 2005.04.19
	CString strStatus = m_pDevCtrl->GetDeviceStatus(_T("SENSOROperatorSwitch"));
	if	(strStatus.CompareNoCase(_T("SUPERVISOR")) == 0)
		return OPER_SUPERVISOR;
	else
	if	(strStatus.CompareNoCase(_T("RUN")) == 0)
		return OPER_RUN;
	else
	if	(strStatus.CompareNoCase(_T("SUPERVISORCHANGED")) == 0)
		return OPER_SUPERVISORCHANGED;
	else
	if	(strStatus.CompareNoCase(_T("RUNCHANGED")) == 0)
		return OPER_RUNCHANGED;
	else
	if	(strStatus.CompareNoCase(_T("MAINTENANCE")) == 0)
		return OPER_RUNCHANGED;
	
	return OPER_NONE;
}

// ----------------------------------------------------------------------------
// DESCRIPT  : operator Switch Clear
// PARAMETER : X
// RETURN    : Always NORMAL
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevSnsClearOperatorSwitch()
{
	ProcSetDeviceStatus(L"SENSOR", L"OperatorSwitch", NULL);		// 2004.11.05
	return R_NORMAL;
}

// ----------------------------------------------------------------------------
// DESCRIPT  : Enhanced Audio 상태조회
// PARAMETER : X
// RETURN    : "PRESENT"상태가 아니면 FALSE Return
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevSnsGetEnhancedAudio()
{
	if (!m_bSensorOpened)
		return FALSE;
																// 2006.02.23
	CString strStatus = m_pDevCtrl->GetDeviceStatus(L"SENSOREnhancedAudio");
	if	(strStatus.CompareNoCase(L"NOTAVAILABLE") == 0)
		return FALSE;
	else
	if	(strStatus.CompareNoCase(L"PRESENT") == 0)
		return TRUE;
	else
	if	(strStatus.CompareNoCase(L"NOTPRESENT") == 0)
		return FALSE;
	
	return FALSE;
}

// ----------------------------------------------------------------------------
// DESCRIPT  : Proximity Status 상태조회
// PARAMETER : X
// RETURN    : "PRESENT"상태가 아니면 FALSE Return
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevSnsGetProximityStatus()
{
	if (!m_bSensorOpened)
		return FALSE;
																// 2006.02.23
	CString strStatus = m_pDevCtrl->GetDeviceStatus(L"SENSORProximity");
	if	(strStatus.CompareNoCase(L"NOTAVAILABLE") == 0)
		return FALSE;
	else
	if	(strStatus.CompareNoCase(L"PRESENT") == 0)
		return TRUE;
	else
	if	(strStatus.CompareNoCase(L"NOTPRESENT") == 0)
		return FALSE;
	
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Event : Accepted From SENSOR
///////////////////////////////////////////////////////////////////////////////////////////////////
void CNHMWICtrl::OnFatalErrorNxsensors(LPCTSTR Action, long Result) 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnDeviceErrorNxsensors(LPCTSTR Action, long Result) 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnDeviceStatusChangedNxsensors(LPCTSTR Value) 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnOperatorSwitchChangedNxsensors(LPCTSTR Status) 
{
	/*
	char szTemp[128];
	memset(szTemp, NULL, sizeof(szTemp));
	WideToMulti(szTemp, Status, sizeof(szTemp));

	if (strcmp(szTemp, "SUPERVISOR") == 0)
		ProcSetDeviceStatus(L"SENSOR", L"OperatorSwitch", L"SUPERVISOR");
	if (strcmp(szTemp, "RUN") == 0)
		ProcSetDeviceStatus(L"SENSOR", L"OperatorSwitch", L"RUN");
		*/
	ProcSetDeviceStatus(L"SENSOR", L"OperatorSwitch", Status);
}

void CNHMWICtrl::OnTamperChangedNxsensors(LPCTSTR Status) 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnInternalTamperChangedNxsensors(LPCTSTR Status) 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnSeismicChangedNxsensors(LPCTSTR Status) 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnHeatChangedNxsensors(LPCTSTR Status) 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnResetCompleteNxsensors() 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnEnhancedAudioChangedNxsensors(LPCTSTR Status) 
{
	/*
	// TODO: Add your control notification handler code here
	char szTemp[128];
	memset(szTemp, NULL, sizeof(szTemp));
	WideToMulti(szTemp, Status, sizeof(szTemp));

	if (strcmp(szTemp, SIU_NOTAVAILABLE) == 0)
		ProcSetDeviceStatus(L"SENSOR", L"EnhancedAudio", L"NOTAVAILABLE");
	else
	if (strcmp(szTemp, SIU_PRESENT) == 0)
	{
		ProcSetDeviceStatus(L"SENSOR", L"EnhancedAudio", L"PRESENT");	
	}
	else
	if (strcmp(szTemp, SIU_NOTPRESENT) == 0)
	{
		ProcSetDeviceStatus(L"SENSOR", L"EnhancedAudio", L"NOTPRESENT");	
	}
	*/
	ProcSetDeviceStatus(L"SENSOR", L"EnhancedAudio", Status);
	
}

void CNHMWICtrl::OnProximityChangedNxsensors(LPCTSTR Status) 
{
	// TODO: Add your control notification handler code here
	ProcSetDeviceStatus(L"SENSOR", L"Proximity", Status);
	//RETAILMSG(1, (L"SENSOR VALUE : %s\n", Status));
}

void CNHMWICtrl::OnAmblightChangedNxsensors(LPCTSTR Status) 
{
	// TODO: Add your control notification handler code here
	
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Method : Call From SENSOR to SENSOR
///////////////////////////////////////////////////////////////////////////////////////////////////
int CNHMWICtrl::DevSnsUpdateStatus()
{
	if (!m_bSensorOpened)										// 장치가Open되지 않았으면
		return R_NORMAL;										// 상태갱신은 의미없음
	
	// 1.Sensor
	ProcSetDeviceStatus(L"SENSOR", L"OperatorSwitch", m_pSensor.GetOperatorSwitchStatus());
																// 2005.04.19
	// 2.Enhanced Audio (ADA), 2006.02.24
	ProcSetDeviceStatus(L"SENSOR", L"EnhancedAudio", m_pSensor.GetEnhancedAudioStatus());

	// 3.Proximity
	ProcSetDeviceStatus(L"SENSOR", L"Proximity", m_pSensor.GetProximityStatus());

	return R_NORMAL;
}