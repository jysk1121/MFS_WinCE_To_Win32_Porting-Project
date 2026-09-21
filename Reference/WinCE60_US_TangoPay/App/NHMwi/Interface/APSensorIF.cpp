// APSENSORIF.cpp

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
// Method : Called From AP
///////////////////////////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// DESCRIPT  : 센서부 초기화
// PARAMETER : X
// RETURN    : NORMAL(0)
// ----------------------------------------------------------------------------
long CNHMWICtrl::SnsInitialize() 
{
	// TODO: Add your dispatch handler code here
	int nResult = 0;
	if (m_bSensorOpened)
	{
		nResult = DevSnsCloseConnection();						// 2005.03.30
		if (nResult != R_NORMAL)
			return nResult;										// 2005.03.30
	}
	m_bSensorOpened = TRUE;
	nResult = DevSnsOpenConnection();

	return nResult;
}

// ----------------------------------------------------------------------------
// DESCRIPT  : 센서부 세션종료
// PARAMETER : X
// RETURN    : NORMAL(0)
// ----------------------------------------------------------------------------
long CNHMWICtrl::SnsDeinitialize() 
{
	int nResult = 0;
	if (m_bSensorOpened)
	{
		m_bSensorOpened = FALSE;										
		nResult = DevSnsCloseConnection();
//		m_pDevCtrl->CheckDeviceClosed(DEV_SNS, K_30_WAIT);		// 2005.03.30
	}
	m_bSensorOpened = FALSE;										

	return nResult;
}

// ----------------------------------------------------------------------------
// DESCRIPT  : 센서부 장애코드초기화
// PARAMETER : X
// RETURN    : NORMAL
// ----------------------------------------------------------------------------
long CNHMWICtrl::SnsClearErrorCode() 
{
	return DevSnsClearErrorCode();
}

// ----------------------------------------------------------------------------
// DESCRIPT  : 센서부 장애코드조회
// PARAMETER : X
// RETURN    : 조회된 장애코드
// ----------------------------------------------------------------------------
BSTR CNHMWICtrl::SnsGetErrorCode() 
{
	CString strResult("");
	strResult = DevSnsGetErrorCode();
	return strResult.AllocSysString();
}

// ----------------------------------------------------------------------------
// DESCRIPT  : 센서부 장치상태조회
// PARAMETER : X
// RETURN    : Default NORMAL처리
// ----------------------------------------------------------------------------
long CNHMWICtrl::SnsGetDeviceStatus() 
{
	return DevSnsGetDeviceStatus();
}

// ----------------------------------------------------------------------------
// DESCRIPT  : Operator Switch 상태조회
// PARAMETER : X
// RETURN    : TRUE If Swich Status Changed
// ----------------------------------------------------------------------------
long CNHMWICtrl::SnsGetOperatorSwitch() 
{
	return DevSnsGetOperatorSwitch();
}

// ----------------------------------------------------------------------------
// DESCRIPT  : Operator Switch Clear
// PARAMETER : X
// RETURN    : NORMAL
// ----------------------------------------------------------------------------
long CNHMWICtrl::SnsClearOperatorSwitch() 
{
	return DevSnsClearOperatorSwitch();
}

// ----------------------------------------------------------------------------
// DESCRIPT  : Enhanced Audio (ADA) 상태조회
// PARAMETER : X
// RETURN    : TRUE If PRESENT Status Changed
// ----------------------------------------------------------------------------
long CNHMWICtrl::SnsGetEnhancedAudio() 
{
	// TODO: Add your dispatch handler code here
	return DevSnsGetEnhancedAudio();
}

// ----------------------------------------------------------------------------
// DESCRIPT  : Enhanced Audio (ADA) 상태조회
// PARAMETER : X
// RETURN    : TRUE If PRESENT Status Changed
// ----------------------------------------------------------------------------
long CNHMWICtrl::SnsGetProximity() 
{
	// TODO: Add your dispatch handler code here
	return DevSnsGetProximityStatus();
}
