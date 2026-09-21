// APDOORIF.cpp

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
// PROTOTYPE : long DorInitialize() 
// DESCRIPT  : 뒷문 세션연결
// PARAMETER : 없음
// RETURN    : 함수 실행결과(0-정상실행)
// REMARK	 : 20030814_1.0_1066 : Device Open Proc.
// ----------------------------------------------------------------------------
long CNHMWICtrl::DorInitialize() 
{
	int nResult = 0;
	if (m_bDoorOpened)
	{
		nResult = DevDoorCloseConnection();						// 2005.03.30
		if (nResult != R_NORMAL)
			return nResult;										// 2005.03.30
	}
	m_bDoorOpened = TRUE;
	nResult = DevDoorOpenConnection();
//	m_pDevCtrl->CheckDeviceOpend(DEV_DOR, K_30_WAIT);			// 2005.03.30

	return nResult;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long DorDeinitialize() 
// DESCRIPT  : 뒷문 세션종료
// PARAMETER : 없음
// RETURN    : 함수 실행결과(0-정상실행)
// REMARK	 : 20030814_1.0_1066 : Device Open Proc.
// ----------------------------------------------------------------------------
long CNHMWICtrl::DorDeinitialize() 
{
	int nResult = 0;
	if (m_bDoorOpened)
	{
		m_bDoorOpened = FALSE;										
		nResult = DevDoorCloseConnection();
//		m_pDevCtrl->CheckDeviceClosed(DEV_DOR, K_30_WAIT);		// 2005.03.30
	}
	m_bDoorOpened = FALSE;										

	return nResult;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long DorClearErrorCode() 
// DESCRIPT  : 뒷문의 장애코드를 클리어한다.
// PARAMETER : 없음
// RETURN    : NORMAL(0)
// ----------------------------------------------------------------------------
long CNHMWICtrl::DorClearErrorCode() 
{
	return DevDoorClearErrorCode();
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long CNHMWICtrl::DorGetDeviceStatus() 
// DESCRIPT  : 장치 상태조회
// PARAMETER : 없음
// RETURN    : NORMAL(0)
// ----------------------------------------------------------------------------
long CNHMWICtrl::DorGetDeviceStatus() 
{
	return DevDoorGetStatus();
}

// ----------------------------------------------------------------------------
// PROTOTYPE : BOOL DorGetDoorStatus() 
// DESCRIPT  : 뒷문의 상태를 조회한다.
// PARAMETER : 없음
// RETURN    : TRUE(1)-문닫힘, FALSE(0)-문열림
// ----------------------------------------------------------------------------
long CNHMWICtrl::DorGetDoorStatus() 
{
	return DevDoorGetDoorSwitch();
}

