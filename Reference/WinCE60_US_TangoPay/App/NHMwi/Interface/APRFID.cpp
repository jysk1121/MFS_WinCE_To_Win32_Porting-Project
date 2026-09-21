// RFIDIF.cpp
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
// PROTOTYPE : long RFIDInitialize() 
// DESCRIPT  : AP I/F - 카드부 초기화
// PARAMETER : 없음
// RETURN    : 함수수행결과(0-정상수행)
// REMARK	 : 20030814_1.0_1066 : Device Open Proc.
// ----------------------------------------------------------------------------
long CNHMWICtrl::RFIDInitialize() 
{
NHDEBUG(1, (_T("CNHMWICtrl::RFIDInitialize() \n")));

	int nResult = 0;
	
	if (m_bRFIDOpened)
	{
		m_pDevCtrl->EvtQReset(DEV_RFID);							// 2005.04.19
		nResult = DevRFIDCloseConnection();
		if (nResult != R_NORMAL)
		{
			return nResult;
		}
	}
	m_bRFIDOpened = TRUE;

	SetEventMatrix(DEV_RFID, "OpenSessionSync");
	nResult = DevRFIDOpenConnection();

	return nResult;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long McuDeinitialize() 
// DESCRIPT  : RFID 연결 끊기
// PARAMETER : 없음
// RETURN    : 함수수행결과(0-정상수행)
// REMARK	 : 20030814_1.0_1066 : Device Open Proc.
// ----------------------------------------------------------------------------
long CNHMWICtrl::RFIDDeinitialize() 
{
NHDEBUG(1, (_T("CNHMWICtrl::RFIDDeinitialize()(%s) \n"), "  "));

	int nResult = 0;

	m_bRFIDOpened = FALSE;
	nResult = DevRFIDCloseConnection();

	return nResult;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : BSTR RFIDGetErrorCode() 
// DESCRIPT  : 장애코드를 조회한다.
// PARAMETER : 없음
// RETURN    : 장애코드
// ----------------------------------------------------------------------------
BSTR CNHMWICtrl::RFIDGetErrorCode() 
{
NHDEBUG(1, (_T("CNHMWICtrl::RFIDGetErrorCode() \n")));

	CString strResult("");
	strResult = DevRFIDGetErrorCode();
NHDEBUG(1, (_T("CNHMWICtrl::RFIDGetErrorCode() strResult[%s]\n"), strResult));
	return strResult.AllocSysString();
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long RFIDClearErrorCode() 
// DESCRIPT  : RFID의 장애코드를 클리어한다.
// PARAMETER : 없음
// RETURN    : NORMAL(0)
// ----------------------------------------------------------------------------
long CNHMWICtrl::RFIDClearErrorCode() 
{
NHDEBUG(1, (_T("CNHMWICtrl::RFIDClearErrorCode()(%s) \n"), "  "));

	m_pDevCtrl->EvtQReset(DEV_RFID);

	return DevRFIDClearErrorCode();
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long RFIDGetDeviceStatus() 
// DESCRIPT  : 디바이스의 상태정보를 리턴한다.
// PARAMETER : nStatus : 1 Device Status, 2 Sensor Info.
// RETURN    : NORMAL(0), DOWN(2)
// ----------------------------------------------------------------------------
long CNHMWICtrl::RFIDGetDeviceStatus() 
{
	return DevRFIDGetStatus(1);
}

// ----------------------------------------------------------------------------
// PROTOTYPE : BSTR RFIDGetCardData() 
// DESCRIPT  : 카드부의 마그네틱 트랙정보를 리턴한다.
// PARAMETER : 없음
// RETURN    : 트랙데이타
// REMARK    : 트랙데이타를 HEXA STRING으로 변환한뒤, 1,2,3트랙데이타를 COMMA(,)로
//             연결하여 트랙데이타를 리턴한다.
// REMARK    : 20030917_1.0_1073 : MakeUnpack
// ----------------------------------------------------------------------------
BSTR CNHMWICtrl::RFIDGetCardData() 
{
NHDEBUG(1, (_T("CNHMWICtrl::RFIDGetCardData() \n")));

	CString strResult("");

	strResult.Format(_T("%s,%s"), DevRFIDGetTrackData(1), DevRFIDGetTrackData(2));

	NHDEBUG(1, (_T("Return CNHMWICtrl::RFIDGetCardData()(%s) \n"), strResult));

	return strResult.AllocSysString();
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long RFIDEntryEnable(long nWaitSec) 
// DESCRIPT  : 카드부 엔트리 발행및 카드투입시 읽기시작 처리
// PARAMETER : nWaitSec-투입대기시간(초단위)
// RETURN    : 함수수행결과(0-정상수행)
// ----------------------------------------------------------------------------
long CNHMWICtrl::RFIDEntryEnable(long nWaitSec) 
{
NHDEBUG(1, (_T("CNHMWICtrl::RFIDEntryEnable()(%s) \n"), "  "));

	SetEventMatrix(DEV_RFID, "EntryEnable");
	return DevRFIDAcceptAndReadAvailableTracks(0);
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long RFIDEntryDisable() 
// DESCRIPT  : 카드부 엔트리 발행 췻처리
// PARAMETER : 없음
// RETURN    : 함수수행결과(0-정상수행)
// ----------------------------------------------------------------------------
long CNHMWICtrl::RFIDEntryDisable() 
{
NHDEBUG(1, (_T("CNHMWICtrl::RFIDEntryDisable()(%s) \n"), "  "));

	SetEventMatrix(DEV_RFID, "CancelAccept");
	return DevRFIDCancelAccept();
}

