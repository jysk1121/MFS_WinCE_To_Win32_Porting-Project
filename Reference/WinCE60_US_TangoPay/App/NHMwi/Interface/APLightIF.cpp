// APLIGHTIF.cpp

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
// Method : Called From AP
///////////////////////////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// PROTOTYPE : long LgtInitialize() 
// DESCRIPT  : 세션연결
// PARAMETER : 없음
// RETURN    : 함수수행결과(0-정상수행)
// REMARK	 : 20030814_1.0_1066 : Device Open Proc.
// ----------------------------------------------------------------------------
long CNHMWICtrl::LgtInitialize() 
{
	int nResult = 0;
	if (m_bLightOpened)
	{
		nResult = DevLightCloseConnection();					// 2005.03.30
		if (nResult != R_NORMAL)
			return nResult;										// 2005.03.30
	}
	m_bLightOpened = TRUE;
	nResult = DevLightOpenConnection();
//	m_pDevCtrl->CheckDeviceOpend(DEV_LGT, K_30_WAIT);			// 2005.03.30

	return nResult;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long LgtDeinitialize() 
// DESCRIPT  : 세션종료
// PARAMETER : 없음
// RETURN    : 함수수행결과(0-정상수행)
// REMARK	 : 20030814_1.0_1066 : Device Open Proc.
// ----------------------------------------------------------------------------
long CNHMWICtrl::LgtDeinitialize() 
{
	int nResult = 0;
	if (m_bLightOpened)
	{
		m_bLightOpened = FALSE;										
		nResult = DevLightCloseConnection();
//		m_pDevCtrl->CheckDeviceClosed(DEV_LGT, K_30_WAIT);		// 2005.03.30
	}
	m_bLightOpened = FALSE;										

	return nResult;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : BSTR LgtClearErrorCode() 
// DESCRIPT  : 장애정보 삭제
// PARAMETER : 없음
// RETURN    : 장애코드
// REMARK    : 20030731_1.0_1057 : Interface Compaction
// ----------------------------------------------------------------------------
long CNHMWICtrl::LgtClearErrorCode() 
{
	return DevLightClearErrorCode();
}

// ----------------------------------------------------------------------------
// PROTOTYPE : BSTR LgtGetErrorCode() 
// DESCRIPT  : 장애정보 삭제
// PARAMETER : 없음
// RETURN    : 장애코드
// REMARK    : 20030731_1.0_1057 : Interface Compaction
// ----------------------------------------------------------------------------
BSTR CNHMWICtrl::LgtGetErrorCode() 
{
	return DevLightGetErrorCode().AllocSysString();
}

// ----------------------------------------------------------------------------
// PROTOTYPE : void LgtSetFlicker(long nIndex, long nValue)
// DESCRIPT  : 플리커를 조작한다
// PARAMETER : nIndex-플리커인덱스, nValue-On/Off값
// RETURN    : NORMAL(0)
// ----------------------------------------------------------------------------
long CNHMWICtrl::LgtSetFlicker(long nIndex, long nValue) 
{
	DevLightFlickerOnOff(nIndex, nValue);
	return R_NORMAL;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : void LgtSetIndicator(long nValue) 
// DESCRIPT  : 거래표시등을 조작한다
// PARAMETER : nValue-켜고자 하는 거래표시등의 값(사양서 참조)
// RETURN    : NORMAL(0)
// ----------------------------------------------------------------------------
long CNHMWICtrl::LgtSetIndicator(long nValue) 
{
	DevLightSetIndicator(nValue);
	return R_NORMAL;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long CNHMWICtrl::LgtGetDeviceStatus() 
// DESCRIPT  : 장치상태 조회
// PARAMETER : 없음
// RETURN    : 상태값
// REMARK    : 20030731_1.0_1057 : Interface Compaction
// ----------------------------------------------------------------------------
long CNHMWICtrl::LgtGetDeviceStatus() 
{
	return DevLightGetStatus();
}

