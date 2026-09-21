// SPRIF.cpp

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
// PROTOTYPE : long SprInitialize() 
// DESCRIPT  : Power-On 초기화수행
// PARAMETER : 없음
// RETURN    : 함수실행결과(0-정상수행)
// REMARK	 : 20030814_1.0_1066 : Device Open Proc.
// ----------------------------------------------------------------------------
long CNHMWICtrl::SprInitialize() 
{
NHDEBUG(1, (_T("CNHMWICtrl::SprInitialize()(%s) \n"), "  "));

	DevSprPreCancelWaitTaken();									// 20030814_1.0_1066 : Pre-CancelWaitTaken Proc.

	m_nSprExist = ST_NOT_DETECT;

	int nResult = 0;
	if (m_bSprOpened)
	{
		m_pDevCtrl->EvtQReset(DEV_SPR);							// 2005.04.19
		nResult = DevSprCloseConnection();						// 2005.03.30
		if (nResult != R_NORMAL)
		{
			return nResult;										// 2005.03.30
		}
	}
	
	m_bSprOpened = TRUE;

	SetEventMatrix(DEV_SPR, "OpenSessionSync");
	nResult = DevSprOpenConnection();

	return nResult;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long SprDeinitialize() 
// DESCRIPT  : 세션종료처리
// PARAMETER : 없음
// RETURN    : 함수실행결과(0-정상수행)
// REMARK	 : 20030814_1.0_1066 : Device Open Proc.
// ----------------------------------------------------------------------------
long CNHMWICtrl::SprDeinitialize() 
{
NHDEBUG(1, (_T("CNHMWICtrl::SprDeinitialize()(%s) \n"), "  "));

	DevSprPreCancelWaitTaken();									// 20030814_1.0_1066 : Pre-CancelWaitTaken Proc.

	m_nSprExist = ST_NOT_DETECT;

	int nResult = 0;
	if (m_bSprOpened)
	{
		m_bSprOpened = FALSE;										
		nResult = DevSprCloseConnection();
	}
	else
	{
		m_bSprOpened = FALSE;										
	}
	return nResult;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : BSTR SprGetErrorCode() 
// DESCRIPT  : 장애코드를 조회한다.
// PARAMETER : 없음
// RETURN    : 장애코드
// ----------------------------------------------------------------------------
BSTR CNHMWICtrl::SprGetErrorCode() 
{
NHDEBUG(1, (_T("CNHMWICtrl::SprGetErrorCode()(%s) \n"), "  "));

	CString strResult("");
	strResult = DevSprGetErrorCode();
	return strResult.AllocSysString();
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long SprClearErrorCode() 
// DESCRIPT  : 명세표부의 장애코드를 클리어한다.
// PARAMETER : 없음
// RETURN    : NORMAL(0)
// ----------------------------------------------------------------------------
long CNHMWICtrl::SprClearErrorCode() 
{
NHDEBUG(1, (_T("CNHMWICtrl::SprClearErrorCode()(%s) \n"), "  "));

	return DevSprClearErrorCode();
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long SprClearRetractCnt() 
// DESCRIPT  : 명세표 회수매수를 클리어한다.
// PARAMETER : 없음
// RETURN    : 함수수행결과(0:NORMAL)
// ----------------------------------------------------------------------------
long CNHMWICtrl::SprClearRetractCnt() 
{
NHDEBUG(1, (_T("CNHMWICtrl::SprClearRetractCnt()(%s) \n"), "  "));

	return DevSprResetBinCount();
}

// ----------------------------------------------------------------------------
// PROTOTYPE : BSTR SprGetSensorInfo() 
// DESCRIPT  : 명세표부의 센서정보를 조회한다
// PARAMETER : 없음
// RETURN    : 조회한 센서정보값(String type)
// ----------------------------------------------------------------------------
BSTR CNHMWICtrl::SprGetSensorInfo() 
{
NHDEBUG(1, (_T("CNHMWICtrl::SprGetSensorInfo()(%s) \n"), "  "));

	CString strResult("");
	strResult = DevSprGetSensorInfo();							// 20030719_1.0_1053 : GetSensorInfo
	return strResult.AllocSysString();
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long SprGetDeviceStatus() 
// DESCRIPT  : 명세표부 장치의 상태를 조회한다.
// PARAMETER : 없음
// RETURN    : NORMAL(0), DOWN(2)
// ----------------------------------------------------------------------------
long CNHMWICtrl::SprGetDeviceStatus() 
{
//NHDEBUG(1, (_T("CNHMWICtrl::SprGetDeviceStatus()(%s) \n"), "  "));

	return DevSprGetStatus(1);
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long SprGetPaperStatus() 
// DESCRIPT  : 명세표부 용지의 상태를 조회한다.
// PARAMETER : 없음
// RETURN    : SLIP_NORMAL(0), SLIP_EMPTY_PAPER(1), SLIP_LOW_END(2)
// ----------------------------------------------------------------------------
long CNHMWICtrl::SprGetPaperStatus() 
{
NHDEBUG(1, (_T("CNHMWICtrl::SprGetPaperStatus()(%s) \n"), "  "));

	return DevSprGetStatus(3);
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long SprGetMaterialInfo() 
// DESCRIPT  : 매체정보 조회(잔류등)
// PARAMETER : 없음
// RETURN    : ST_NOT_DETECT(0), ST_SENSOR1(1), ST_SENSOR2(2)의 조합값
// ----------------------------------------------------------------------------
long CNHMWICtrl::SprGetMaterialInfo() 
{
NHDEBUG(1, (_T("CNHMWICtrl::SprGetMaterialInfo()(%s) \n"), "  "));

	int nResult = ST_NOT_DETECT;
	int nMediaStatus = DevSprGetStatus(2);

	nMediaStatus &= ~ST_SENSOR2;								// 명세표부에 롤만있어도 SENSOR2값이 온다
	nResult = (nMediaStatus | m_nSprExist);

	return nResult;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : BSTR SprGetRetractCnt() 
// DESCRIPT  : 명세표의 회수매수 정보를 조회한다.
// PARAMETER : 없음
// RETURN    : 회수매수 문자열
// ----------------------------------------------------------------------------
BSTR CNHMWICtrl::SprGetRetractCnt() 
{
NHDEBUG(1, (_T("CNHMWICtrl::SprGetRetractCnt()(%s) \n"), "  "));

	CString strResult("");

	int		nRetractCnt = 0;									// 20030719_1.0_1053 : 회수정보조회
	if (!RegQueryValueExt(_REGKEY_SPRSTATUS, _T("Retain_Count"), REG_INT, 
						  sizeof(nRetractCnt), &nRetractCnt))
		nRetractCnt = 0;
	strResult.Format(_T("%d"), nRetractCnt);

	return strResult.AllocSysString();
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long SprPrint(LPCTSTR szPrintData) 
// DESCRIPT  : 명세표를 인자한다.
// PARAMETER : 인자하고자 하는 데이타
// RETURN    : 함수수행결과
// REMARK    : 파라미터로 전달되는 출력데이타는 hexaString이다. 또한 Comma(,)로 구분되어 
//             있으므로 이를 나누고 변형하여 출력데이타 폼에 맞추어야 한다.
// ----------------------------------------------------------------------------
long CNHMWICtrl::SprPrint(LPCTSTR szPrintData) 
{
NHTRACE((_T("CNHMWICtrl::SprPrint() szPrintData(%s) \n"), szPrintData));

	long nRet;

	CString strTemp("");

	DevSprPreCancelWaitTaken();									// 20030814_1.0_1066 : Pre-CancelWaitTaken Proc.

	m_nSprExist = ST_SENSOR2;
	SetEventMatrix(DEV_SPR, "PrintForm");

	strTemp = CString(szPrintData);

	// [#2373] US Justin 2015.09.22 Support Barcode Printing
	// 명세표 인자 데이타 보정처리
	// 인자시 데이타의 길이를 확인한다.
	// 만일 데이타가 NULL이면 인자시 Device(혹은 Fatal)Error가 발생한다.
	// 저널의 경우도 마찬가지이다.
	// 데이타가 없는 경우 "20"(Space)을 삽입한다.
	// 20030913_1.0_1072 : Print NULL Data 보정처리
	if (wcslen(szPrintData) == 0)
		nRet = DevSprPrint(_T("20"));
	else if (!strTemp.CompareNoCase(CMD_PRINT_HEADER_IMAGE))
		nRet = DevSprImagePrint(IMAGETYPE_HEADER);
	else if (!strTemp.CompareNoCase(CMD_PRINT_COUPON_IMAGE_BCD))		
		nRet = DevSprImagePrint(IMAGETYPE_COUPON_BCD);
	else if (!strTemp.CompareNoCase(CMD_PRINT_COUPON_IMAGE_QR))	
		nRet = DevSprImagePrint(IMAGETYPE_COUPON_QR);
	else if (!strTemp.CompareNoCase(CMD_PRINT_COUPON_IMAGE_LARGEQR))	
		nRet = DevSprImagePrint(IMAGETYPE_COUPON_LARGEQR);					// [#2503] US Justin 2017.09.15 Add Ethereum.. Large QR Image
	// End of [#2373]	
	else
		nRet = DevSprPrint(szPrintData);

	SysFreeString((BSTR)szPrintData);

	return nRet;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long SprEject(long nWaitSec) 
// DESCRIPT  : 명세표방출
// PARAMETER : nWaitSec-방출후 수취대기시간
// RETURN    : 함수수행결과(0-정상수행)
// ----------------------------------------------------------------------------
long CNHMWICtrl::SprEject(long nWaitSec) 
{
NHDEBUG(1, (_T("CNHMWICtrl::SprEject() (%s) \n"), "  "));

	DevSprPreCancelWaitTaken();									// 20030814_1.0_1066 : Pre-CancelWaitTaken Proc.

	m_nSprExist = ST_SENSOR1;
	SetEventMatrix(DEV_SPR, "ControlMedia");
	return DevSprEject(nWaitSec);
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long SprWaitTaken()
// DESCRIPT  : 명세표 수취대기
// PARAMETER : 없음
// RETURN    : 함수수행결과(0-정상수행)
// ----------------------------------------------------------------------------
long CNHMWICtrl::SprWaitTaken() 
{
NHDEBUG(1, (_T("CNHMWICtrl::SprWaitTaken() (%s) \n"), "  "));

	SetEventMatrix(DEV_SPR, "WaitTaken");
	return R_NORMAL;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long SprRetract() 
// DESCRIPT  : 명세표회수
// PARAMETER : 없음
// RETURN    : 함수수행결과(0-정상수행)
// ----------------------------------------------------------------------------
long CNHMWICtrl::SprRetract() 
{
NHDEBUG(1, (_T("CNHMWICtrl::SprRetract() (%s) \n"), "  "));

	DevSprPreCancelWaitTaken();									// 20030814_1.0_1066 : Pre-CancelWaitTaken Proc.

	m_nSprExist = ST_NOT_DETECT;
	SetEventMatrix(DEV_SPR, "RetractMedia");
	return DevSprRetract();
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long SprCancelWaitTaken() 
// DESCRIPT  : 명세표수취대기를 취소한다
// PARAMETER : 없음
// RETURN    : 함수수행결과(0-정상수행)
// REMARK    : 20030729_1.0_1055 : Add CancelWaitTaken
// ----------------------------------------------------------------------------
long CNHMWICtrl::SprCancelWaitTaken() 
{
NHDEBUG(1, (_T("CNHMWICtrl::SprCancelWaitTaken() (%s) \n"), "  "));

	SetEventMatrix(DEV_SPR, "CancelWaitForMediaTaken");
	return DevSprCancelWaitTaken();
}
