// DEVLIGHTIF.cpp

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
// Method : Call to GUIDELIGHT/INDICATOR
///////////////////////////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// 함 수 명 : int	CNHMWICtrl::DevLightOpenConnection()
// DESCRIPT : 세션연결
// ----------------------------------------------------------------------------
int	CNHMWICtrl::DevLightOpenConnection()
{
	m_bLightOpenedEvent = FALSE;								// 2005.03.30
	m_bIndOpenedEvent = FALSE;									// 2005.03.30

	NHDEBUG(1, (_T("OpenSessionSync() ... \n")));
	int nReturn = m_pLight.OpenSessionSync(K_30_WAIT*1000);
	NHDEBUG(1, (L"\n"));
	NHDEBUG(1, (_T("OpenSessionSync() ... DONE (%d)\n"), nReturn));

//	m_pInd.OpenSessionSync(K_30_WAIT*1000);						// 2004.06.19
	return R_NORMAL;
}

// ----------------------------------------------------------------------------
// 함 수 명 : int	CNHMWICtrl::DevLightCloseConnection()
// DESCRIPT : 세션종료
// ----------------------------------------------------------------------------
int	CNHMWICtrl::DevLightCloseConnection()
{
	// 모든 라이트를 끈다.
	DevLightSetIndicator(0);

	m_bLightClosedEvent = FALSE;								// 2005.03.30
	m_bIndClosedEvent = FALSE;									// 2005.03.30
	m_pLight.CloseSessionSync();
	//m_pInd.CloseSessionSync();									// 2004.06.19
	return R_NORMAL;
}

// ----------------------------------------------------------------------------
// 함 수 명 : void CNHMWICtrl::DevLightFlickerOnOff(int nFlickerIndex, int nOnOffFlag)
// DESCRIPT : 플리커를조절한다.
// ----------------------------------------------------------------------------
void CNHMWICtrl::DevLightFlickerOnOff(int nFlickerIndex, int nOnOffFlag)
{
	CString strValue("");
	if (nOnOffFlag == FLICKER_ON)					strValue = FLICKER_LIGHT_ON;
	else if (nOnOffFlag == FLICKER_CONTINUE)		strValue = FLICKER_LIGHT_CONTINUE;	// [#2327] NH KSK 2015.01.24
	else											strValue = FLICKER_LIGHT_OFF;

	if (m_pLight.GetDeviceStatus() != ST_DEVONLINE)	return;

	if (nFlickerIndex & SPR_FLICKER)
	{
		if (m_pLight.GetGuidLightStatus(_T("RECEIPTPRINTER")).CompareNoCase(_T("NOTAVAILABLE")) != 0)
			m_pLight.SetGuidLight(_T("RECEIPTPRINTER"), (LPCTSTR)strValue);
	}

	// 20040309 JUNXI MCU FLICKER
	if (nFlickerIndex & MCU_FLICKER)
	{
		if (m_pLight.GetGuidLightStatus(_T("CARDUNIT")).CompareNoCase(_T("NOTAVAILABLE")) != 0)
			m_pLight.SetGuidLight(_T("CARDUNIT"), (LPCTSTR)strValue);
	}

	// 20040309 JUNXI CDU FLICKER
	if (nFlickerIndex & CDU_FLICKER)
	{
		if (m_pLight.GetGuidLightStatus(_T("NOTESDISPENSER")).CompareNoCase(_T("NOTAVAILABLE")) != 0)
			m_pLight.SetGuidLight(_T("NOTESDISPENSER"), (LPCTSTR)strValue);
	}

	// 20040309 JUNXI PIN FLICKER 추가(NC3000)
	if (nFlickerIndex & PIN_FLICKER)
	{
		if (nOnOffFlag == FLICKER_ON)	strValue = FLICKER_LIGHT_CONTINUE;
		else strValue = FLICKER_LIGHT_OFF;						// 플리커는 CONTINUE : 2004.04.09

		if (m_pLight.GetGuidLightStatus(_T("PINPAD")).CompareNoCase(_T("NOTAVAILABLE")) != 0)
			m_pLight.SetGuidLight(_T("PINPAD"), (LPCTSTR)strValue);
	}

	// [#GLDV-3005] US Kook 2022.01.17 Support Side Car
	if (nFlickerIndex & SIDECAR_COD_FLICKER)
	{
		if (m_pLight.GetGuidLightStatus(_T("COINDISPENSER")).CompareNoCase(_T("NOTAVAILABLE")) != 0)
			m_pLight.SetGuidLight(_T("COINDISPENSER"), (LPCTSTR)strValue);
	}

	if (nFlickerIndex & SIDECAR_BNA_FLICKER)
	{
		if (m_pLight.GetGuidLightStatus(_T("BILLACCEPTOR")).CompareNoCase(_T("NOTAVAILABLE")) != 0)
			m_pLight.SetGuidLight(_T("BILLACCEPTOR"), (LPCTSTR)strValue);
	}
	// end of [#GLDV-3005]
}

// ----------------------------------------------------------------------------
// 함 수 명 : void CNHMWICtrl::DevLightSetIndicator(int nValue)
// DESCRIPT : 거래표시등을 조절한다.
// ----------------------------------------------------------------------------
void CNHMWICtrl::DevLightSetIndicator(int nValue)
{
//	CString strValue("");
//	strValue.Format("%d", nValue);
//
//	// 거래표시등의 인덱스는 [10]으로 고정하도록 한다(SP강진현과장)
//	if (m_pInd.GetStOpenClosedStatus().CompareNoCase("HEALTHY") == 0)
//		m_pInd.SetIndicator("10", strValue);					// 2004.06.19

//	if (m_pInd.GetStOpenClosedStatus().CompareNoCase("HEALTHY") == 0)

//	if (m_pInd.GetDeviceStatus() == ST_DEVONLINE)
//	{
//		if (nValue)
//			m_pInd.SetOpenClose(_T("OPEN"));
//		else
//			m_pInd.SetOpenClose(_T("CLOSED"));
//	}
}

// ----------------------------------------------------------------------------
// PROTOTYPE : int DevLightGetStatus()
// DESCRIPT  : 장애코드를 조회하여 장치상태를 조회한다.
// PARAMETER : 없음
// RETURN    : NORMAL(0), DOWN(2)
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevLightGetStatus()
{
	CString strResult("");
	int nResult = 0;

	// 장애코드를 확보한다.
	strResult = DevLightGetErrorCode();
	char szTemp[64];
	memset(szTemp, NULL, sizeof(szTemp));
	WideToMulti(szTemp, strResult, sizeof(szTemp));

	if (strResult.IsEmpty())	nResult = NORMAL;
	else if (IsZero(szTemp, strResult.GetLength()))	nResult = NORMAL;
	else nResult = DOWN;

	return nResult;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : int LightClearErrorCode() 
// DESCRIPT  : LIGHT부의 장애코드를 클리어한다.
// PARAMETER : 없음
// RETURN    : NORMAL(0)
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevLightClearErrorCode() 
{
	RegSetValueExt(_REGKEY_DEVERROR, _T("SIU"), REG_STR, 0, NULL);	// 센서
	return NORMAL;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : BSTR DevLightGetErrorCode() 
// DESCRIPT  : 장애코드를 얻어온다.
// PARAMETER : 없음
// RETURN    : 장애코드
// ----------------------------------------------------------------------------
CString CNHMWICtrl::DevLightGetErrorCode() 
{
	CString strResult("");
//	TCHAR szCode[256];		memset(szCode, 0x00, sizeof(szCode));
//	int nResult = 0;
//
//	// 레지스트리에서 장애코드를 확보한다.
//	nResult = RegQueryValueExt(_REGKEY_DEVERROR, _T("SIU"), REG_STR, sizeof(szCode), (LPVOID)szCode);
//	if (nResult)
//	{
//		strResult.Format(_T("%S0000000"), szCode);					// 2004.06.19
//		strResult = strResult.Left(7);							// 에러코드보정	: 2004.06.19
//	}

	return strResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Event : Accepted From GUIDELIGHT/INDICATOR
///////////////////////////////////////////////////////////////////////////////////////////////////
void CNHMWICtrl::OnFatalErrorNxguidlights(LPCTSTR Action, long Result) 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnDeviceErrorNxguidlights(LPCTSTR Action, long Result) 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnLockGrantedNxguidlights() 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnLockReleasedNxguidlights() 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnLockTimeoutNxguidlights() 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnDeviceStatusChangedNxguidlights(LPCTSTR Value) 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnGuidLightChangedNxguidlights(LPCTSTR GuidLightUnit, LPCTSTR State) 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnResetCompleteNxguidlights() 
{
	// TODO: Add your control notification handler code here
	
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Method : Call From GUIDELIGHT/INDICATOR to GUIDELIGHT/INDICATOR
///////////////////////////////////////////////////////////////////////////////////////////////////

