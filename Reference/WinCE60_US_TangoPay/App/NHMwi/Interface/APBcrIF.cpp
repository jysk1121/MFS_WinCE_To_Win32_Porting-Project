// APBCRIF.cpp

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
// PROTOTYPE : long BcrInitialize() 
// DESCRIPT  : Power-On 초기화처리
// PARAMETER : 없음
// RETURN    : 함수수행결과(0-정상수행)
// REMARK	 : 20030814_1.0_1066 : Device Open Proc.
// ----------------------------------------------------------------------------
long CNHMWICtrl::BcrInitialize() 
{
	NHTRACE((_T("(%s) \n"), L"  "));

	int nResult = R_ERROR;
	if (m_bBcrOpened)
	{
		m_pDevCtrl->EvtQReset(DEV_BCR);
		nResult = DevBcrCloseConnection();
		m_bBcrOpened = FALSE;
	}
	
	SetEventMatrix(DEV_BCR, "OpenSessionSync");
	nResult = DevBcrOpenConnection();
	NHTRACE((_T("DevBcrOpenConnection() returns: (%d) \n"), nResult));

	m_bBcrOpened = (nResult == R_NORMAL);
	DevBcrUpdateStatus();

	return nResult;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long BcrDeinitialize() 
// DESCRIPT  : 세션종료
// PARAMETER : 없음
// RETURN    : 함수수행결과(0-정상수행)
// REMARK	 : 
// ----------------------------------------------------------------------------
long CNHMWICtrl::BcrDeinitialize() 
{
	NHTRACE((_T("(%s) \n"), L"  "));

	int nResult = R_ERROR;
	if (m_bBcrOpened)
		nResult = DevBcrCloseConnection();

	m_bBcrOpened = FALSE;										
	return nResult;
}

long CNHMWICtrl::BcrSendRawData()
{
	NHDEBUG(1, (_T("(%s) \n"), L"  "));

	SetEventMatrix(DEV_BCR, "SendRawData");
	return DevBcrSendRawData();
}

long CNHMWICtrl::BcrCancelAccept()
{
	NHDEBUG(1, (_T("(%s) \n"), L"  "));

	SetEventMatrix(DEV_BCR, "CancelAccept");
	return DevBcrCancelAccept();
}

long CNHMWICtrl::BcrReset()
{
	NHDEBUG(1, (_T("(%s) \n"), L"  "));

	SetEventMatrix(DEV_BCR, "Reset");
	return DevBcrReset();
}

BSTR CNHMWICtrl::BcrGetBarcodeData()
{
	NHDEBUG(1, (_T("(%s) \n"), L"  "));

	BSTR bstrBarcode = DevBcrGetBarcodeData().AllocSysString();
	NHDEBUG(1, (_T("bstrBarcode: [%s]\n"), bstrBarcode));

	return bstrBarcode;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : BSTR BcrGetErrorCode() 
// DESCRIPT  : 장애코드를 조회한다.
// PARAMETER : 없음
// RETURN    : 장애코드
// ----------------------------------------------------------------------------
BSTR CNHMWICtrl::BcrGetErrorCode() 
{
	NHDEBUG(1, (_T("BcrGetErrorCode(%s) \n"), L"  "));

	BSTR bstrResult = DevBcrGetErrorCode().AllocSysString();
	NHTRACE((_T("CNHMWICtrl::BcrGetErrorCode strResult(%s) \n"), bstrResult));
	return bstrResult;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long BcrClearErrorCode() 
// DESCRIPT  : 저널부의 장애코드를 클리어한다.
// PARAMETER : 없음
// RETURN    : NORMAL(0)
// ----------------------------------------------------------------------------
long CNHMWICtrl::BcrClearErrorCode() 
{
	return DevBcrClearErrorCode();
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long BcrGetDeviceStatus() 
// DESCRIPT  : 저널의 장치상태를 조회한다.
// PARAMETER : 없음
// RETURN    : NORMAL(0), DOWN(2)
// ----------------------------------------------------------------------------
long CNHMWICtrl::BcrGetDeviceStatus() 
{
	return DevBcrGetDeviceStatus();
}
