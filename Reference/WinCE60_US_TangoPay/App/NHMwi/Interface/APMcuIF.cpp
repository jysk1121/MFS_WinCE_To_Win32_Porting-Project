// MCUIF.cpp
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
// PROTOTYPE : long McuInitialize() 
// DESCRIPT  : AP I/F - 카드부 초기화
// PARAMETER : 없음
// RETURN    : 함수수행결과(0-정상수행)
// REMARK	 : 20030814_1.0_1066 : Device Open Proc.
// ----------------------------------------------------------------------------
long CNHMWICtrl::McuInitialize() 
{
	NHDEBUG(1, (_T("CNHMWICtrl::McuInitialize()(%s) \n"), L"  "));

	DevMcuPreCancelWaitTaken();									// 20030814_1.0_1066 : Pre-CancelWaitTaken Proc.

	m_nMcuExist = ST_NOT_DETECT;

	int nResult = 0;
	
	if (m_bMcuOpened)
	{
		m_pDevCtrl->EvtQReset(DEV_MCU);							// 2005.04.19
		nResult = DevMcuCloseConnection();
		if (nResult != R_NORMAL)
		{
			return nResult;
		}
	}
	m_bMcuOpened = TRUE;

	SetEventMatrix(DEV_MCU, "OpenSessionSync");
	nResult = DevMcuOpenConnection();

	return nResult;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long McuDeinitialize() 
// DESCRIPT  : 카드부 연결 끊기
// PARAMETER : 없음
// RETURN    : 함수수행결과(0-정상수행)
// REMARK	 : 20030814_1.0_1066 : Device Open Proc.
// ----------------------------------------------------------------------------
long CNHMWICtrl::McuDeinitialize() 
{
NHDEBUG(1, (_T("CNHMWICtrl::McuDeinitialize()(%s) \n"), "  "));

	DevMcuPreCancelWaitTaken();									// 20030814_1.0_1066 : Pre-CancelWaitTaken Proc.

	int nResult = 0;
	m_nMcuExist = ST_NOT_DETECT;

	m_bMcuOpened = FALSE;
	nResult = DevMcuCloseConnection();

	return nResult;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : 
// DESCRIPT  : 
// PARAMETER : 
// RETURN    : 
// ----------------------------------------------------------------------------
long CNHMWICtrl::McuICChipInitialize() 
{
NHDEBUG(1, (_T("CNHMWICtrl::McuICChipInitialize()(%s) \n"), "  "));

	SetEventMatrix(DEV_MCU, "ChipIO");

	return DevMcuICChipInitialize();
}

// ----------------------------------------------------------------------------
// PROTOTYPE : BSTR McuGetErrorCode() 
// DESCRIPT  : 장애코드를 조회한다.
// PARAMETER : 없음
// RETURN    : 장애코드
// ----------------------------------------------------------------------------
BSTR CNHMWICtrl::McuGetErrorCode() 
{
NHDEBUG(1, (_T("CNHMWICtrl::McuGetErrorCode() \n")));

	CString strResult("");
	strResult = DevMcuGetErrorCode();
NHDEBUG(1, (_T("CNHMWICtrl::McuGetErrorCode() strResult[%s]\n"), strResult));
	return strResult.AllocSysString();
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long McuClearErrorCode() 
// DESCRIPT  : 카드부의 장애코드를 클리어한다.
// PARAMETER : 없음
// RETURN    : NORMAL(0)
// ----------------------------------------------------------------------------
long CNHMWICtrl::McuClearErrorCode() 
{
NHDEBUG(1, (_T("CNHMWICtrl::McuClearErrorCode()(%s) \n"), L"  "));

	m_pDevCtrl->EvtQReset(DEV_MCU);

	return DevMcuClearErrorCode();
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long McuClearRetractCnt() 
// DESCRIPT  : 명세표 회수매수를 클리어한다.
// PARAMETER : 없음
// RETURN    : 함수수행결과(0:NORMAL)
// ----------------------------------------------------------------------------
long CNHMWICtrl::McuClearRetractCnt() 
{
NHDEBUG(1, (_T("CNHMWICtrl::McuClearRetractCnt()(%s) \n"), "  "));

	return DevMcuResetBinCount();
}

// ----------------------------------------------------------------------------
// PROTOTYPE : BSTR McuGetSensorInfo() 
// DESCRIPT  : 카드부의 센서정보를 조회한다
// PARAMETER : 없음
// RETURN    : 조회한 센서정보값(String type)
// ----------------------------------------------------------------------------
BSTR CNHMWICtrl::McuGetSensorInfo() 
{
NHDEBUG(1, (_T("CNHMWICtrl::McuGetSensorInfo()(%s) \n"), "  "));

	CString strResult("");
	strResult = DevMcuGetSensorInfo();							// 20030719_1.0_1053 : GetSensorInfo
	return strResult.AllocSysString();
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long McuGetDeviceStatus() 
// DESCRIPT  : 디바이스의 상태정보를 리턴한다.
// PARAMETER : nStatus : 1 Device Status, 2 Sensor Info.
// RETURN    : NORMAL(0), DOWN(2)
// ----------------------------------------------------------------------------
long CNHMWICtrl::McuGetDeviceStatus() 
{
	return DevMcuGetStatus(1);
}
// ----------------------------------------------------------------------------
// PROTOTYPE : long McuGetMaterialInfo()  
// DESCRIPT  : 디바이스의 매체정보를 리턴한다.
// PARAMETER : 없음
// RETURN    : ST_NOT_DETECT(0), ST_SENSOR1(1), ST_SENSOR2(2)
// ----------------------------------------------------------------------------
long CNHMWICtrl::McuGetMaterialInfo() 
{
	int nRet;
	nRet = DevMcuGetStatus(ST_MEDIASTATUS);
	return nRet;

/*	return (DevMcuGetStatus(2) | m_nMcuExist);*/
}

// ----------------------------------------------------------------------------
// PROTOTYPE : BSTR McuGetCardData() 
// DESCRIPT  : 카드부의 마그네틱 트랙정보를 리턴한다.
// PARAMETER : 없음
// RETURN    : 트랙데이타
// REMARK    : 트랙데이타를 HEXA STRING으로 변환한뒤, 1,2,3트랙데이타를 COMMA(,)로
//             연결하여 트랙데이타를 리턴한다.
// REMARK    : 20030917_1.0_1073 : MakeUnpack
// ----------------------------------------------------------------------------
BSTR CNHMWICtrl::McuGetCardData() 
{
NHDEBUG(1, (_T("CNHMWICtrl::McuGetCardData()(%s) \n"), "  "));

	CString strResult("");

	// [#2250] US Justin 2014.01.27 Read Track1 and Track2
	// KSK_2007-06-04오후 5:54:08
	// 속도개선을 위해 Card Data를 2 Track만 읽도록 수정
	// 각 데이타를 Hexa String으로 변환한다.
//	strResult.Format(_T("%s,%s,%s"), DevMcuGetTrackData(1), DevMcuGetTrackData(2), DevMcuGetTrackData(3));
//	strResult.Format(_T("%s"), DevMcuGetTrackData(2));
	strResult.Format(_T("%s,%s"), DevMcuGetTrackData(1), DevMcuGetTrackData(2));
	// end of KSK_2007-06-04오후 5:54:08
	// End of [#2250]

	NHDEBUG(1, (_T("Return CNHMWICtrl::McuGetCardData()(%s) \n"), strResult));

	return strResult.AllocSysString();
}

// ----------------------------------------------------------------------------
// PROTOTYPE : BSTR McuGetICData() 
// DESCRIPT  : 
// PARAMETER : 
// RETURN    : 
// ----------------------------------------------------------------------------
BSTR CNHMWICtrl::McuGetICData() 
{
NHDEBUG(1, (_T("CNHMWICtrl::McuGetICData() \n")));

	CString strResult("");
	strResult.Format(_T("%s"), m_strICData);
	m_strICData.ReleaseBuffer();
	m_strICData.Empty();

NHDEBUG(1, (_T("CNHMWICtrl::McuGetICData() return(%s) \n"), strResult));
	return strResult.AllocSysString();
}

// ----------------------------------------------------------------------------
// PROTOTYPE : BSTR McuGetRetractCnt() 
// DESCRIPT  : 카드의 회수매수 정보를 조회한다.
// PARAMETER : 없음
// RETURN    : 회수매수 문자열
// ----------------------------------------------------------------------------
BSTR CNHMWICtrl::McuGetRetractCnt() 
{
NHDEBUG(1, (_T("CNHMWICtrl::McuGetRetractCnt()(%s) \n"), "  "));

	CString strResult("");

	int		nRetractCnt = 0;									// 20030719_1.0_1053 : 회수정보조회
	if (!RegQueryValueExt(_REGKEY_MCUSTATUS, _T("Retain_Count"), REG_INT, 
						  sizeof(nRetractCnt), &nRetractCnt))
		nRetractCnt = 0;

	strResult.Format(_T("%d"), nRetractCnt);

	return strResult.AllocSysString();
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long McuEntryEnable(long nWaitSec) 
// DESCRIPT  : 카드부 엔트리 발행및 카드투입시 읽기시작 처리
// PARAMETER : nWaitSec-투입대기시간(초단위)
// RETURN    : 함수수행결과(0-정상수행)
// ----------------------------------------------------------------------------
long CNHMWICtrl::McuEntryEnable(long nWaitSec) 
{
NHDEBUG(1, (_T("CNHMWICtrl::McuEntryEnable()(%s) \n"), "  "));

	DevMcuPreCancelWaitTaken();									// 20030814_1.0_1066 : Pre-CancelWaitTaken Proc.

	SetEventMatrix(DEV_MCU, "EntryEnable");						// 20030814_1.0_1066 : EntryEnable
	return DevMcuAcceptAndReadAvailableTracks(0);				// 20030814_1.0_1066 : Waiting insert time Changing.
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long McuEntryDisable() 
// DESCRIPT  : 카드부 엔트리 발행 췻처리
// PARAMETER : 없음
// RETURN    : 함수수행결과(0-정상수행)
// ----------------------------------------------------------------------------
long CNHMWICtrl::McuEntryDisable() 
{
NHDEBUG(1, (_T("CNHMWICtrl::McuEntryDisable()(%s) \n"), "  "));

	DevMcuPreCancelWaitTaken();									// 20030814_1.0_1066 : Pre-CancelWaitTaken Proc.

	SetEventMatrix(DEV_MCU, "CancelAccept");
	return DevMcuCancelAccept();
}

// ----------------------------------------------------------------------------
// PROTOTYPE : 
// DESCRIPT  : 
// PARAMETER : 
// RETURN    : 
// ----------------------------------------------------------------------------
long CNHMWICtrl::McuICEntryEnable(long nWaitSec) 
{
NHDEBUG(1, (_T("CNHMWICtrl::McuICEntryEnable()\n")));

	DevMcuPreCancelWaitTaken();									// 20030814_1.0_1066 : Pre-CancelWaitTaken Proc.

	SetEventMatrix(DEV_MCU, "EntryEnable");
	return DevMcuICAcceptAndReadAvailableTracks(0);				// 20030814_1.0_1066 : Waiting insert time Changing.
}

// ----------------------------------------------------------------------------
// PROTOTYPE : 
// DESCRIPT  : 
// PARAMETER : 
// RETURN    : 
// ----------------------------------------------------------------------------
long CNHMWICtrl::McuICEntryDisable() 
{
NHDEBUG(1, (_T("CNHMWICtrl::McuICEntryDisable()\n")));

	DevMcuPreCancelWaitTaken();									// 20030814_1.0_1066 : Pre-CancelWaitTaken Proc.

	SetEventMatrix(DEV_MCU, "CancelAccept");					// [#GLDV-2740] AU HJAHN 2019.12.13 9791309 장애대책 
	return DevMcuCancelAccept();
}

// ----------------------------------------------------------------------------
// PROTOTYPE : 
// DESCRIPT  : 
// PARAMETER : 
// RETURN    : 
// ----------------------------------------------------------------------------
long CNHMWICtrl::McuICChipPower(long nActType) 
{
NHDEBUG(1, (_T("CNHMWICtrl::McuICChipPower() nActType(%d) \n"), nActType));

	DevMcuPreCancelWaitTaken();	

	SetEventMatrix(DEV_MCU, "ChipPower");
	//return DevMcuICChipPower(nActType);

	int ret = DevMcuICChipPower(nActType);
NHDEBUG(1, (_T("CNHMWICtrl::McuICChipPower() DevMcuICChipPower():[%d] \n"), ret));
	return ret;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long McuRead() 
// DESCRIPT  : 내부처리함수 - 실제로 SP에는 전달되지 않음
// PARAMETER : 없음
// RETURN    : R_NORMAL(0)
// ----------------------------------------------------------------------------
long CNHMWICtrl::McuRead() 
{
NHDEBUG(1, (_T("CNHMWICtrl::McuRead()(%s) \n"), "  "));

	SetEventMatrix(DEV_MCU, "CardRead");
	return R_NORMAL;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : 
// DESCRIPT  : 
// PARAMETER : 
// RETURN    : 
// ----------------------------------------------------------------------------
long CNHMWICtrl::McuICSendData(long ProtocolID, LPCTSTR szSendData, long nWaitSec) 
{
NHDEBUG(1, (_T("CNHMWICtrl::McuICSendData()ProtocolID(%d) szSendData(%s) \n"), ProtocolID, szSendData));

	SetEventMatrix(DEV_MCU, "ChipIO");

	long nRet = DevMcuICSendData(ProtocolID, szSendData, nWaitSec);

	return nRet;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long McuEject(long nWaitSec) 
// DESCRIPT  : 카드방출 및수취대기
// PARAMETER : nWaitSec-수취대기시간(초단위)
// RETURN    : 함수수행결과(0-정상수행)
// ----------------------------------------------------------------------------
long CNHMWICtrl::McuEject(long nWaitSec) 
{
NHDEBUG(1, (_T("CNHMWICtrl::McuEject()(%s) \n"), "  "));

	DevMcuPreCancelWaitTaken();									// 20030814_1.0_1066 : Pre-CancelWaitTaken Proc.

	m_nMcuExist = ST_SENSOR1;

	SetEventMatrix(DEV_MCU, "EjectMedia");
	return DevMcuEject(nWaitSec);
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long McuWaitTaken() 
// DESCRIPT  : 카드수취 확인:SP미전달
// PARAMETER : 없음
// RETURN    : 함수수행결과(0-정상수행)
// ----------------------------------------------------------------------------
long CNHMWICtrl::McuWaitTaken() 
{
NHDEBUG(1, (_T("CNHMWICtrl::McuWaitTaken()(%s) \n"), "  "));

	SetEventMatrix(DEV_MCU, "WaitTaken");
	return R_NORMAL;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long McuRetract() 
// DESCRIPT  : 카드회수처리
// PARAMETER : 없음
// RETURN    : 함수수행결과(0-정상수행)
// ----------------------------------------------------------------------------
long CNHMWICtrl::McuRetract()
{
NHDEBUG(1, (_T("CNHMWICtrl::McuRetract()(%s) \n"), "  "));

	DevMcuPreCancelWaitTaken();									// 20030814_1.0_1066 : Pre-CancelWaitTaken Proc.

	m_nMcuExist = ST_NOT_DETECT;

	SetEventMatrix(DEV_MCU, "RetainMedia");						// 20030807_1.0_1060 : Synchronize Bug Debugging
	return DevMcuRetract();
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long McuWrite(LPCTSTR szTrackData) 
// DESCRIPT  : 카드회수처리
// PARAMETER : 기록하고자 하는 카드데이타
// RETURN    : 함수수행결과(0-정상수행)
// ----------------------------------------------------------------------------
long CNHMWICtrl::McuWrite(LPCTSTR szTrackData) 
{
NHDEBUG(1, (_T("CNHMWICtrl::McuWrite()(%s) \n"), "  "));

	DevMcuPreCancelWaitTaken();									// 20030814_1.0_1066 : Pre-CancelWaitTaken Proc.

	SetEventMatrix(DEV_MCU, "WriteRawData");					// 20030807_1.0_1060 : Synchronize Bug Debugging
	long lret = DevMcuWriteTrack(szTrackData);

	SysFreeString((BSTR)szTrackData);

	return lret;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long McuCancelWaitTaken() 
// DESCRIPT  : 카드수취대기 취소처리
// PARAMETER : 없음
// RETURN    : 수행결과
// ----------------------------------------------------------------------------
long CNHMWICtrl::McuCancelWaitTaken() 
{
NHDEBUG(1, (_T("CNHMWICtrl::McuCancelWaitTaken()(%s) \n"), "  "));

	return DevMcuCancelWaitTaken();
}

