// APBNAIF.cpp

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
long CNHMWICtrl::BnaInitialize()
{
	NHTRACE((_T("(%s) \n"), L"  "));

	int nResult = R_ERROR;

	if (m_bBnaOpened)
	{
		m_pDevCtrl->EvtQReset(DEV_BNA);
		DevBnaCloseConnection();
		m_bBnaOpened = FALSE;
	}

	SetEventMatrix(DEV_BNA, "OpenSessionSync");
	nResult = DevBnaOpenConnection();
	NHTRACE((_T("DevBnaOpenConnection returns: (%d) \n"), nResult));

	m_bBnaOpened = (nResult == R_NORMAL);
	DevBnaUpdateStatus();

	return nResult;
}

// ----------------------------------------------------------------------------
long CNHMWICtrl::BnaDeinitialize() 
{
	NHTRACE((_T("CNHMWICtrl::BnaDeinitialize()(%s) \n"), L"  "));
		
	int nResult = R_ERROR;
	if (m_bBnaOpened)
		nResult = DevBnaCloseConnection();						// 20030703_10_1047 : 함수호출수정

	m_bBnaOpened = FALSE;										
	return nResult;
}


//// ----------------------------------------------------------------------------
//long CNHMWICtrl::BnaOpenShutter() 
//{
//	NHTRACE((_T("CNHMWICtrl::BnaOpenShutter()(%s) \n"), L"  "));
//
//	SetEventMatrix(DEV_BNA, "OpenShutter");
//	return DevBnaOpenShutter();
//}
//
//// ----------------------------------------------------------------------------
//long CNHMWICtrl::BnaCloseShutter() 
//{
//	NHTRACE((_T("CNHMWICtrl::BnaCloseShutter()(%s) \n"), L"  "));
//	
//	SetEventMatrix(DEV_BNA, "CloseShutter");
//	return DevBnaCloseShutter();
//}
//
//// ----------------------------------------------------------------------------
//long CNHMWICtrl::BnaWaitTaken()
//{
//	NHTRACE((_T("CNHMWICtrl::BnaWaitTaken()(%s) \n"), L"  "));
//
//	SetEventMatrix(DEV_BNA, "WaitTaken");
//	return R_NORMAL;
//}
//
//
//// ----------------------------------------------------------------------------
//long CNHMWICtrl::BnaRetract() 
//{
//	NHTRACE((_T("CNHMWICtrl::BnaRetract()(%s) \n"), L"  "));
//
//	SetEventMatrix(DEV_BNA, "Retract");
//	return DevBnaRetract();
//}


// ----------------------------------------------------------------------------
BSTR CNHMWICtrl::BnaGetErrorCode() 
{
	NHTRACE((_T("(%s) \n"), L"  "));

	BSTR bstrResult = DevBnaGetErrorCode().AllocSysString();
	NHTRACE((_T("CNHMWICtrl::BnaGetErrorCode strResult(%s) \n"), bstrResult));
	return bstrResult;
}

// ----------------------------------------------------------------------------
long CNHMWICtrl::BnaClearErrorCode() 
{
	NHTRACE((_T("(%s) \n"), L"  "));
	return DevBnaClearErrorCode();
}

// ----------------------------------------------------------------------------
long CNHMWICtrl::BnaGetDeviceStatus() 
{
	int nResult = DevBnaGetDeviceStatus();

	return nResult;
}

BSTR CNHMWICtrl::BnaGetExtraStatus(LPCTSTR KeyName)
{
	BSTR bstrResult = DevBnaGetExtraStatus(KeyName).AllocSysString();
	NHTRACE((_T("(%s) => [%s]\n"), KeyName, bstrResult));
	return bstrResult;
}

long CNHMWICtrl::BnaStartCashIn()
{
	NHTRACE((_T("(%s) \n"), L"  "));

	SetEventMatrix(DEV_BNA, "StartCashIn");
	return DevBnaStartCashIn();
}

long CNHMWICtrl::BnaAcceptCash(long InsertionTimeOut, long TakenTimeOut)
{
	NHTRACE((_T("(%d, %d) \n"), InsertionTimeOut, TakenTimeOut));

	SetEventMatrix(DEV_BNA, "AcceptCash");
	return DevBnaAcceptCash(InsertionTimeOut, TakenTimeOut);
}

long CNHMWICtrl::BnaCancelAccept()
{
	NHTRACE((_T("(%s) \n"), L"  "));

	SetEventMatrix(DEV_BNA, "CancelAcceptCash");
	return DevBnaCancelAccept();
}

long CNHMWICtrl::BnaStoreCash()
{
	NHTRACE((_T("CNHMWICtrl::BnaStoreCash(%s) \n"), L"  "));

	SetEventMatrix(DEV_BNA, "StoreCash");
	return DevBnaStoreCash();
}

long CNHMWICtrl::BnaRollbackCash(long TakenTimeOut)
{
	NHTRACE((_T("CNHMWICtrl::BnaRollbackCash(%d) \n"), TakenTimeOut));

	SetEventMatrix(DEV_BNA, "RollbackCash");
	return DevBnaRollbackCash(TakenTimeOut);
}

long CNHMWICtrl::BnaReset(short UnitNumber)
{
	NHTRACE((_T("unitNumber: (%d) \n"), UnitNumber));

	SetEventMatrix(DEV_BNA, "Reset");
	return DevBnaReset(UnitNumber);
}

long CNHMWICtrl::BnaConfigureNoteType()
{
	NHTRACE((_T("(%s) \n"), L"  "));
	return DevBnaConfigureNoteTypeSync();
}

long CNHMWICtrl::BnaSetCashUnitInfo()
{
	NHTRACE((_T("(%s) \n"), L"  "));
	return DevBnaSetCashUnitInfo();
}



BSTR CNHMWICtrl::BnaGetLastCashInStatus()
{
	BSTR bstrResult = DevBnaGetLastCashInStatus().AllocSysString();
	NHTRACE((_T("CNHMWICtrl::BnaGetErrorCode strResult(%s) \n"), bstrResult));
	return bstrResult;
}

long CNHMWICtrl::BnaGetNumberOfCashInStatus()
{
	NHTRACE((_T("(%s) \n"), L"  "));
	return DevBnaGetNumberOfCashInStatus();
}

long CNHMWICtrl::BnaGetNumberOfLogicalUnit()
{
	NHTRACE((_T("(%s) \n"), L"  "));
	return DevBnaGetNumberOfLogicalUnit();
}

LPDISPATCH CNHMWICtrl::BnaGetCashInStatus(long Index)
{
	NHTRACE((_T("Index: (%d) \n"), Index));
	return DevBnaGetCashInStatus(Index);
}

LPDISPATCH CNHMWICtrl::BnaGetLogicalUnit(long Index)
{
	NHTRACE((_T("Index: (%d) \n"), Index));
	return DevBnaGetLogicalUnit(Index);
}

BSTR CNHMWICtrl::BnaGetAcceptorStatus()
{
	BSTR bstrResult = DevBnaGetAcceptorStatus().AllocSysString();
	NHTRACE((_T("CNHMWICtrl::BnaGetAcceptorStatus strResult(%s) \n"), bstrResult));
	return bstrResult;
}

BSTR CNHMWICtrl::BnaGetStackerStatus()
{
	BSTR bstrResult = DevBnaGetStackerStatus().AllocSysString();
	NHTRACE((_T("CNHMWICtrl::BnaGetStackerStatus strResult(%s) \n"), bstrResult));
	return bstrResult;
}

long CNHMWICtrl::BnaGetMaxCashInItem()
{
	return DevBnaGetMaxCashInItem();
}

long CNHMWICtrl::BnaGetMaxStackerItem()
{
	return DevBnaGetMaxStackerItem();
}

long CNHMWICtrl::BnaGetLastRefusedCount()
{
	return DevBnaGetLastRefusedCount();
}

BSTR CNHMWICtrl::BnaGetPositionStatus()
{
	// [RWC6-676] Start SKKim 2024.05.27 현재 SP에서 PositionStatus를 주지 않아 임의로 Refuse 처리를 위해 Refuse여부를 임의로 설정하여 상위국에 전달함.
	BSTR bstrResult = DevBnaGetPositionStatus().AllocSysString();
	NHTRACE((_T("CNHMWICtrl::GetPositionStatus strResult(%s) \n"), bstrResult));
	return bstrResult;
}

long CNHMWICtrl::BnaGetRefusedStatus()
{
	return DevBnaGetRefusedStatus();
}