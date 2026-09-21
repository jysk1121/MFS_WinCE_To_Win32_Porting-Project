// DEVBNAIF.cpp

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
// BNA ActiveX 
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// Method : Call to BNA
///////////////////////////////////////////////////////////////////////////////////////////////////
int CNHMWICtrl::DevBnaOpenConnection()
{
	int		nResult = R_ERROR;

	NHDEBUG(1, (_T("OpenSessionSync() ... \n")));
	DWORD curTickCount = GetTickCount();
	nResult = m_pBna.OpenSessionSync(K_30_WAIT*1000);			// TODO
	NHDEBUG(1, (L"\n"));
	NHDEBUG(1, (_T("OpenSessionSync() ... DONE (%d) (%lu ms)\n"), nResult, GetTickCount() - curTickCount));

	if (nResult != R_NORMAL)
	{
		char chBnaOpenErrorCode[] = "9792B00";
		m_pDevCtrl->WriteMwiErrorCode(DEV_BNA, DEVNM_BNA, _countof(chBnaOpenErrorCode), chBnaOpenErrorCode);
		ProcSetDeviceEvent(L"BNA", L"FatalError", NULL);
	}
	else
	{
		// This is to remove a fatal error generated after successful OpenSessionSync(), which was executed to open SP which was killed by force by GOMA.
		m_pDevCtrl->EvtQReset(DEV_BNA);

		ProcSetDeviceEvent(L"BNA", L"OpenComplete", NULL);
	}

	return nResult;
}

int CNHMWICtrl::DevBnaCloseConnection()
{
	NHDEBUG(1, (_T(" \n")));
	int nResult = m_pBna.CloseSessionSync();
	NHDEBUG(1, (_T(" DONE (%d)\n"), nResult));

	return nResult;
}

//int CNHMWICtrl::DevBnaRequestLock()
//{
//	NHDEBUG(1, (_T("DevBnaRequestLock() ... \n")));
//	int nResult = m_pBna.RequestLock(K_30_WAIT * 1000);
//	NHDEBUG(1, (_T("DevBnaRequestLock() ... DONE (%d)\n"), nResult));
//
//	return nResult;
//}
//
//int CNHMWICtrl::DevBnaReleaseLock()
//{
//	NHDEBUG(1, (_T("DevBnaReleaseLock() ... \n")));
//	int nResult = m_pBna.ReleaseLock();
//	NHDEBUG(1, (_T("DevBnaReleaseLock() ... DONE (%d)\n"), nResult));
//
//	return nResult;
//}

CString CNHMWICtrl::DevBnaGetExtraStatus(LPCTSTR KeyName)
{
	NHDEBUG(1, (_T(" \n")));
	CString strResult = m_pBna.GetExtraStatus(KeyName);
	NHDEBUG(1, (_T("strResult: [%s]\n"), strResult));

	return strResult;
}

//CString CNHMWICtrl::DevBnaGetExtraCaps(CString KeyName)
//{
//	NHDEBUG(1, (_T("DevBnaGetExtraCaps() ... \n")));
//	CString strResult = m_pBna.GetExtraCaps(KeyName);
//	NHDEBUG(1, (_T("DevBnaGetExtraCaps() ... DONE (%s)\n"), strResult));
//
//	return strResult;
//}

int CNHMWICtrl::DevBnaStartCashIn()
{
	NHDEBUG(1, (_T(" \n")));
	int nResult = m_pBna.StartCashIn();
	NHDEBUG(1, (_T(" DONE (%d)\n"), nResult));

	return nResult;
}

int CNHMWICtrl::DevBnaAcceptCash(long InsertionTimeOut, long TakenTimeOut)
{
	NHDEBUG(1, (_T("Insert: (%d), Taken: (%d)\n"), InsertionTimeOut, TakenTimeOut));
	int nResult = m_pBna.AcceptCash(InsertionTimeOut, TakenTimeOut);
	NHDEBUG(1, (_T("DONE (%d)\n"), nResult));

	return nResult;
}

int CNHMWICtrl::DevBnaCancelAccept()
{
	NHDEBUG(1, (_T(" \n")));
	int nResult = m_pBna.CancelAccept();
	NHDEBUG(1, (_T(" DONE (%d)\n"), nResult));

	return nResult;
}

int CNHMWICtrl::DevBnaStoreCash()
{
	NHDEBUG(1, (_T(" \n")));
	int nResult = m_pBna.StoreCash();
	NHDEBUG(1, (_T(" DONE (%d)\n"), nResult));

	return nResult;
}

int CNHMWICtrl::DevBnaRollbackCash(long TakenTimeOut)
{
	NHDEBUG(1, (_T(" \n")));
	int nResult = m_pBna.RollbackCash(TakenTimeOut);
	NHDEBUG(1, (_T(" DONE (%d)\n"), nResult));

	return nResult;
}

//// 0: Retract to 'Retract Cash Unit'
//// 1: Retract to 'Transport'
//// 2: Retract to 'Stacker'
//// 3: Retract to 'Bill Cash Unit'
//int CNHMWICtrl::DevBnaRetract()
//{
//	NHDEBUG(1, (_T("DevBnaRetract() ... \n")));
//	int nResult = m_pBna.Retract(0);
//	NHDEBUG(1, (_T("DevBnaRetract() ... DONE (%d)\n"), nResult));
//
//	return nResult;
//}
//
//// FALSE: input position
//// TRUE: output position
//int CNHMWICtrl::DevBnaOpenShutter()
//{
//	NHDEBUG(1, (_T("DevBnaOpenShutter() ... \n")));
//	int nResult = m_pBna.OpenShutter(0);
//	NHDEBUG(1, (_T("DevBnaOpenShutter() ... DONE (%d)\n"), nResult));
//
//	return nResult;
//}
//
//// FALSE: input position
//// TRUE: output position
//int CNHMWICtrl::DevBnaCloseShutter()
//{
//	NHDEBUG(1, (_T("DevBnaCloseShutter() ... \n")));
//	int nResult = m_pBna.CloseShutter(0);
//	NHDEBUG(1, (_T("DevBnaCloseShutter() ... DONE (%d)\n"), nResult));
//
//	return nResult;
//}
//
//int CNHMWICtrl::DevBnaStartExchangeSync()
//{
//	NHDEBUG(1, (_T("DevBnaStartExchangeSync() ... \n")));
//
//	CArray<int, int> IArray;
//	IArray.RemoveAll();
//
//	// TODO: get proper parameters
//	//for ( int i = 0 ; i <= DevCduGetCSTCount() ; i++ )
//	//	IArray.Add(i+1);
//
//	int nResult = m_pBna.StartExchangeSync(IntArrayToSafeArray(IArray));
//	NHDEBUG(1, (_T("DevBnaStartExchangeSync() ... DONE (%d)\n"), nResult));
//
//	return nResult;
//}
//
//int CNHMWICtrl::DevBnaEndExchangeSync()
//{
//	NHDEBUG(1, (_T("DevBnaEndExchangeSync() ... \n")));
//	int nResult = m_pBna.EndExchangeSync();
//	NHDEBUG(1, (_T("DevBnaEndExchangeSync() ... DONE (%d)\n"), nResult));
//
//	return nResult;
//}
//
//int CNHMWICtrl::DevBnaOpenSafeDoor()
//{
//	NHDEBUG(1, (_T("DevBnaOpenSafeDoor() ... \n")));
//	int nResult = m_pBna.OpenSafeDoor();
//	NHDEBUG(1, (_T("DevBnaOpenSafeDoor() ... DONE (%d)\n"), nResult));
//
//	return nResult;
//}

int CNHMWICtrl::DevBnaReset(short UnitNumber)
{
	NHDEBUG(1, (_T("DevBnaReset(%d) ... \n"), UnitNumber));
	int nResult = m_pBna.Reset(UnitNumber);
	NHDEBUG(1, (_T("DevBnaReset() ... DONE (%d)\n"), nResult));

	return nResult;
}

int CNHMWICtrl::DevBnaConfigureNoteTypeSync()
{
	NHDEBUG(1, (_T("DevBnaConfigureNoteTypeSync() ... \n")));
	int nResult = m_pBna.ConfigureNoteTypeSync();
	NHDEBUG(1, (_T("DevBnaConfigureNoteTypeSync() ... DONE (%d)\n"), nResult));

	return nResult;
}

//int CNHMWICtrl::DevBnaCreateP6Signature()
//{
//	NHDEBUG(1, (_T("DevBnaCreateP6Signature() ... \n")));
//	int nResult = m_pBna.CreateP6Signature(0, 0);
//	NHDEBUG(1, (_T("DevBnaCreateP6Signature() ... DONE (%d)\n"), nResult));
//
//	return nResult;
//}
//
//int CNHMWICtrl::DevBnaGetP6Signature()
//{
//	NHDEBUG(1, (_T("DevBnaGetP6Signature() ... \n")));
//	int nResult = m_pBna.GetP6Signature(L"", 0);
//	NHDEBUG(1, (_T("DevBnaGetP6Signature() ... DONE (%d)\n"), nResult));
//
//	return nResult;
//}

int CNHMWICtrl::DevBnaSetCashUnitInfo()
{
	NHDEBUG(1, (_T(" \n")));
	int nResult = m_pBna.SetCashUnitInfo();
	NHDEBUG(1, (_T(" ... DONE (%d)\n"), nResult));

	return nResult;
}

//int CNHMWICtrl::DevBnaSetCashInLimit()
//{
//	NHDEBUG(1, (_T("DevBnaSetCashInLimit() ... \n")));
//	int nResult = m_pBna.SetCashInLimit(0, L"", 0);
//	NHDEBUG(1, (_T("DevBnaSetCashInLimit() ... DONE (%d)\n"), nResult));
//
//	return nResult;
//}

CString	CNHMWICtrl::DevBnaGetLastCashInStatus()
{
	NHDEBUG(1, (_T("\n")));
	CString strResult = m_pBna.GetLastCashInStatus();
	NHDEBUG(1, (_T("DevBnaGetLastCashInStatus() ... DONE (%s)\n"), strResult));

	return strResult;

}

int CNHMWICtrl::DevBnaGetNumberOfCashInStatus()
{
	NHDEBUG(1, (_T("\n")));
	int nResult = m_pBna.GetNumberOfCashInStatus();
	NHDEBUG(1, (_T("GetNumberOfCashInStatus() ... DONE (%d)\n"), nResult));

	return nResult;
}

int CNHMWICtrl::DevBnaGetNumberOfLogicalUnit()
{
	NHDEBUG(1, (_T("\n")));

	if (DevBnaGetDeviceStatus() == NODEVICE)
		return 0;

	int nResult = m_pBna.GetNumberOfLogicalUnit();
	NHDEBUG(1, (_T("... DONE (%d)\n"), nResult));

	return nResult;
}

LPDISPATCH CNHMWICtrl::DevBnaGetCashInStatus(long Index)
{
	NHDEBUG(1, (_T("Index: (%d) \n"), Index));
	LPDISPATCH data = m_pBna.GetCashInStatus(Index);
	NHDEBUG(1, (_T("... DONE ( )\n")));

	return data;
}


LPDISPATCH CNHMWICtrl::DevBnaGetLogicalUnit(long Index)
{
	NHDEBUG(1, (_T("Index: (%d) \n"), Index));

	if (DevBnaGetDeviceStatus() == NODEVICE)
	{
		NHDEBUG(1, (_T("Device not available! \n")));
		return NULL;
	}

	LPDISPATCH data = m_pBna.GetLogicalUnit(Index);
	//ILogicalUnit Unit(data);
	//
	//NHDEBUG(1, (_T("GetLogicalUnit [GetCashInCount=%d] \n"), Unit.GetCashInCount()));
	//NHDEBUG(1, (_T("GetLogicalUnit [GetTotalCount=%d] \n"), Unit.GetTotalCount()));
	//NHDEBUG(1, (_T("GetLogicalUnit [GetMaximumCount=%d] \n"), Unit.GetMaximumCount()));
	//NHDEBUG(1, (_T("GetLogicalUnit [GetCurrencyID=%s] \n"), Unit.GetCurrencyID()));
	//NHDEBUG(1, (_T("GetLogicalUnit [GetStatus=%s] \n"), Unit.GetStatus()));
	//NHDEBUG(1, (_T("GetLogicalUnit [GetNumberOfItem=%d] \n"), Unit.GetNumberOfItem()));

	//NHDEBUG(1, (_T("... DONE ( )\n")));

	return data;
}


// ----------------------------------------------------------------------------
// DESCRIPT : 장치디바이스 상태조회
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevBnaGetDeviceStatus()
{
	int nResult = NORMAL;
	CString strStatus = m_pDevCtrl->GetDeviceStatus(_T("BNAStDeviceStatus"));

	if		(strStatus.CompareNoCase(ST_DEVONLINE)	== 0)		nResult = NORMAL;
	else if	(strStatus.CompareNoCase(ST_DEVNODEVICE)== 0)		nResult = NODEVICE;
	else if	(strStatus.CompareNoCase(ST_DEVOFFLINE) == 0)		nResult = OFFLINE;
	else	nResult = DOWN;

	return nResult;
}

// ----------------------------------------------------------------------------
// DESCRIPT : Get the acceptor status
// ----------------------------------------------------------------------------
CString CNHMWICtrl::DevBnaGetAcceptorStatus()
{
	CString strResult = m_pBna.GetAcceptorStatus();

	return strResult;
}

// ----------------------------------------------------------------------------
// DESCRIPT : Get the stacker status
// ----------------------------------------------------------------------------
CString CNHMWICtrl::DevBnaGetStackerStatus()
{
	CString strResult = m_pBna.GetStackerStatus();

	return strResult;
}

// ----------------------------------------------------------------------------
// DESCRIPT : Get the max cash in item value
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevBnaGetMaxCashInItem()
{
	return m_pBna.GetMaxCashInItem();
}

// ----------------------------------------------------------------------------
// DESCRIPT : Get the max stacker item value
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevBnaGetMaxStackerItem()
{
	return m_pBna.GetMaxStackerItem();
}

// ----------------------------------------------------------------------------
// DESCRIPT : Get the last refused count value
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevBnaGetLastRefusedCount()
{
	return m_pBna.GetLastRefusedCount();
}

// ----------------------------------------------------------------------------
// DESCRIPT : Get the position status
// ----------------------------------------------------------------------------
CString CNHMWICtrl::DevBnaGetPositionStatus()
{
	CString strResult = m_pBna.GetPositionStatus();

	return strResult;
}

// ----------------------------------------------------------------------------
// DESCRIPT : Get the position status
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevBnaGetRefusedStatus()
{
	return m_bRefused;
}

// ----------------------------------------------------------------------------
// DESCRIPT : 장치디바이스 장애코드 삭제
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevBnaClearErrorCode()
{
	RegSetValueExt(_REGKEY_DEVERROR, _T("BNA"), REG_STR, 0, NULL);
	return NORMAL;
}

// ----------------------------------------------------------------------------
// DESCRIPT : 장애코드 조회
// ----------------------------------------------------------------------------
CString CNHMWICtrl::DevBnaGetErrorCode()
{
	return CString(m_pSPInform->SPInfo.BNA.ErrCode);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Event : Accepted From BNA
///////////////////////////////////////////////////////////////////////////////////////////////////
void CNHMWICtrl::OnFatalErrorNxCashAcceptor(LPCTSTR Action, long Result) 
{
	m_bRefused = FALSE;		// [RWC6-676] SKKim 2024.05.27

	NHDEBUG(1, (_T("Action: (%s) Result: (%d)\n"), Action, Result));
	ProcSetDeviceEvent(L"BNA", L"FatalError", NULL);
	DevBnaUpdateStatus();
}

void CNHMWICtrl::OnDeviceErrorNxCashAcceptor(LPCTSTR Action, long Result) 
{
	m_bRefused = FALSE;		// [RWC6-676] SKKim 2024.05.27

	NHDEBUG(1, (_T("Action: (%s) Result: (%d)\n"), Action, Result));
	ProcSetDeviceEvent(L"BNA", L"DeviceError", NULL);
	DevBnaUpdateStatus();
}

void CNHMWICtrl::OnLockGrantedNxCashAcceptor() 
{
	NHDEBUG(1, (_T("%s \n"), _T("")));
	// unnecessary event
}

void CNHMWICtrl::OnLockReleasedNxCashAcceptor() 
{
	NHDEBUG(1, (_T("%s \n"), _T("")));
	// unnecessary event
}

void CNHMWICtrl::OnLockTimeoutNxCashAcceptor() 
{
	NHDEBUG(1, (_T("%s \n"), _T("")));
	// unnecessary event
}

void CNHMWICtrl::OnTimeoutNxCashAcceptor() 
{
	m_bRefused = FALSE;		// [RWC6-676] SKKim 2024.05.27

	NHDEBUG(1, (_T("%s \n"), _T("")));

	ProcSetDeviceEvent(L"BNA", L"Timeout", NULL);
	DevBnaUpdateStatus();
}

void CNHMWICtrl::OnStartCashInCompleteNxCashAcceptor()
{
	m_bRefused = FALSE;		// [RWC6-676] SKKim 2024.05.27

	NHDEBUG(1, (_T("%s \n"), _T("")));

	ProcSetDeviceEvent(L"BNA", L"StartCashInComplete", NULL);
	DevBnaUpdateStatus();
}

void CNHMWICtrl::OnAcceptCashCompleteNxCashAcceptor()
{
	m_bRefused = FALSE;		// [RWC6-676] SKKim 2024.05.27

	NHDEBUG(1, (_T("%s \n"), _T("")));

	ProcSetDeviceEvent(L"BNA", L"AcceptCashComplete", NULL);
	DevBnaUpdateStatus();
}

void CNHMWICtrl::OnStoreCashCompleteNxCashAcceptor(long CashInItems)
{
	m_bRefused = FALSE;		// [RWC6-676] SKKim 2024.05.27

	NHDEBUG(1, (_T("cashInItems: (%ld)\n"), CashInItems));

	ProcSetDeviceEvent(L"BNA", L"StoreCashComplete", NULL);
	DevBnaUpdateStatus();
}

void CNHMWICtrl::OnRollbackCashCompleteNxCashAcceptor()
{
	m_bRefused = FALSE;		// [RWC6-676] SKKim 2024.05.27

	NHDEBUG(1, (_T("%s \n"), _T("")));

	ProcSetDeviceEvent(L"BNA", L"RollbackCashComplete", NULL);
	DevBnaUpdateStatus();
}

void CNHMWICtrl::OnOpenShutterCompleteNxCashAcceptor(LPCTSTR Position)
{
	NHDEBUG(1, (_T("Position: (%s)\n"), Position));
	ProcSetDeviceEvent(L"BNA", L"OpenShutterComplete", NULL);
	DevBnaUpdateStatus();
}

void CNHMWICtrl::OnCloseShutterCompleteNxCashAcceptor(LPCTSTR Position)
{
	NHDEBUG(1, (_T("Position: (%s)\n"), Position));
	ProcSetDeviceEvent(L"BNA", L"CloseShutterComplete", NULL);
	DevBnaUpdateStatus();
}

void CNHMWICtrl::OnRetractCompleteNxCashAcceptor()
{
	NHDEBUG(1, (_T("%s \n"), _T("")));
	ProcSetDeviceEvent(L"BNA", L"RetractComplete", NULL);
	DevBnaUpdateStatus();
}

void CNHMWICtrl::OnOpenSafeDoorCompleteNxCashAcceptor()
{
	NHDEBUG(1, (_T("%s \n"), _T("")));
	ProcSetDeviceEvent(L"BNA", L"OpenSafeDoorComplete", NULL);
	DevBnaUpdateStatus();
}

void CNHMWICtrl::OnResetCompleteNxCashAcceptor()
{
	m_bRefused = FALSE;		// [RWC6-676] SKKim 2024.05.27

	NHDEBUG(1, (_T("%s \n"), _T("")));
	ProcSetDeviceEvent(L"BNA", L"ResetComplete", NULL);
	DevBnaUpdateStatus();
}

void CNHMWICtrl::OnCashUnitThresholdNxCashAcceptor(short UnitNumber)
{
	NHDEBUG(1, (_T("UnitNumber: (%d)\n"), UnitNumber));
	ProcSetDeviceEvent(L"BNA", L"CashUnitThreshold", NULL);
	DevBnaUpdateStatus();
}

void CNHMWICtrl::OnCashUnitChangedNxCashAcceptor(short UnitNumber)
{
	NHDEBUG(1, (_T("UnitNumber: (%d)\n"), UnitNumber));
	ProcSetDeviceEvent(L"BNA", L"CashUnitChanged", NULL);
	DevBnaUpdateStatus();
}

void CNHMWICtrl::OnCashUnitErrorNxCashAcceptor(short UnitNumber)
{
	NHDEBUG(1, (_T("UnitNumber: (%d)\n"), UnitNumber));
	ProcSetDeviceEvent(L"BNA", L"CashUnitError", NULL);
	DevBnaUpdateStatus();
}

void CNHMWICtrl::OnItemsTakenNxCashAcceptor()
{
	m_bRefused = FALSE;		// [RWC6-676] SKKim 2024.05.27

	NHDEBUG(1, (_T("%s \n"), _T("")));
	ProcSetDeviceEvent(L"BNA", L"ItemsTaken", NULL);
	DevBnaUpdateStatus();
}

void CNHMWICtrl::OnItemsRefusedNxCashAcceptor()
{
	m_bRefused = TRUE;		// [RWC6-676] SKKim 2024.05.27

	NHDEBUG(1, (_T("%s \n"), _T("")));
	ProcSetDeviceEvent(L"BNA", L"ItemsRefused", NULL);
	DevBnaUpdateStatus();
}

void CNHMWICtrl::OnItemsInsertedNxCashAcceptor()
{
	m_bRefused = FALSE;		// [RWC6-676] SKKim 2024.05.27

	NHDEBUG(1, (_T("%s \n"), _T("")));
	ProcSetDeviceEvent(L"BNA", L"ItemsInserted", NULL);
	DevBnaUpdateStatus();
}

void CNHMWICtrl::OnMediaDetectedNxCashAcceptor(short UnitNumber)
{
	NHDEBUG(1, (_T("UnitNumber: (%d)\n"), UnitNumber));
	ProcSetDeviceEvent(L"BNA", L"MediaDetected", NULL);
	DevBnaUpdateStatus();
}

void CNHMWICtrl::OnStatusChangedNxCashAcceptor(LPCTSTR NewValue)
{
	NHDEBUG(1, (_T("NewValue: (%s)\n"), NewValue));
	//ProcSetDeviceEvent(L"BNA", L"StatusChanged", NULL);	// duplicated with 'DevBnaUpdateStatus()'
	DevBnaUpdateStatus();
}

void CNHMWICtrl::OnSafeDoorStatusChangedNxCashAcceptor(LPCTSTR NewValue)
{
	NHDEBUG(1, (_T("NewValue: (%s)\n"), NewValue));
	ProcSetDeviceEvent(L"BNA", L"SafeDoorStatusChanged", NULL);
	DevBnaUpdateStatus();
}

void CNHMWICtrl::OnAcceptorStatusChangedNxCashAcceptor(LPCTSTR NewValue)
{
	NHDEBUG(1, (_T("NewValue: (%s)\n"), NewValue));
	ProcSetDeviceEvent(L"BNA", L"AcceptorStatusChanged", NULL);
	DevBnaUpdateStatus();
}

void CNHMWICtrl::OnStackerStatusChangedNxCashAcceptor(LPCTSTR NewValue)
{
	NHDEBUG(1, (_T("NewValue: (%s)\n"), NewValue));
	ProcSetDeviceEvent(L"BNA", L"StackerStatusChanged", NULL);
	DevBnaUpdateStatus();
}

void CNHMWICtrl::OnReaderStatusChangedNxCashAcceptor(LPCTSTR NewValue)
{
	NHDEBUG(1, (_T("NewValue: (%s)\n"), NewValue));
	ProcSetDeviceEvent(L"BNA", L"ReaderStatusChanged", NULL);
	DevBnaUpdateStatus();
}

void CNHMWICtrl::OnShutterStatusChangedNxCashAcceptor(LPCTSTR Position, LPCTSTR NewValue)
{
	NHDEBUG(1, (_T("Position: (%s) NewValue: (%s)\n"), Position, NewValue));
	ProcSetDeviceEvent(L"BNA", L"ShutterStatusChanged", NULL);
	DevBnaUpdateStatus();
}

void CNHMWICtrl::OnPositionStatusChangedNxCashAcceptor(LPCTSTR Position, LPCTSTR NewValue)
{
	NHDEBUG(1, (_T("Position: (%s) NewValue: (%s)\n"), Position, NewValue));
	ProcSetDeviceEvent(L"BNA", L"PositionStatusChanged", NULL);
	DevBnaUpdateStatus();
}

void CNHMWICtrl::OnTransportStatusChangedNxCashAcceptor(LPCTSTR Position, LPCTSTR NewValue)
{
	NHDEBUG(1, (_T("Position: (%s) NewValue: (%s)\n"), Position, NewValue));
	ProcSetDeviceEvent(L"BNA", L"TransportStatusChanged", NULL);
	DevBnaUpdateStatus();
}

void CNHMWICtrl::OnAcceptCancelledNxCashAcceptor()
{
	m_bRefused = FALSE;		// [RWC6-676] SKKim 2024.05.27

	NHDEBUG(1, (_T("%s \n"), _T("")));
	ProcSetDeviceEvent(L"BNA", L"AcceptCancelled", NULL);
	DevBnaUpdateStatus();
}

void CNHMWICtrl::OnCreateP6SignatureCompleteNxCashAcceptor(short NoteID, LPCTSTR Orientation, LPCTSTR Signature)
{
	NHDEBUG(1, (_T("NoteId:(%d) Orientation: (%s) Signature: (%s)\n"), NoteID, Orientation, Signature));
	ProcSetDeviceEvent(L"BNA", L"CreateP6SignatureComplete", NULL);
	DevBnaUpdateStatus();
}

void CNHMWICtrl::OnGetP6SignatureCompleteNxCashAcceptor(short NoteID, LPCTSTR Orientation, LPCTSTR Signature)
{
	NHDEBUG(1, (_T("NoteId:(%d) Orientation: (%s) Signature: (%s)\n"), NoteID, Orientation, Signature));
	ProcSetDeviceEvent(L"BNA", L"GetP6SignatureComplete", NULL);
	DevBnaUpdateStatus();
}

void CNHMWICtrl::OnP6InputedNxCashAcceptor()
{
	NHDEBUG(1, (_T("%s \n"), _T("")));
	ProcSetDeviceEvent(L"BNA", L"P6Inputed", NULL);
	DevBnaUpdateStatus();
}

void CNHMWICtrl::OnItemsPresentedNxCashAcceptor()
{
	NHDEBUG(1, (_T("%s \n"), _T("")));
	ProcSetDeviceEvent(L"BNA", L"ItemsPresented", NULL);
	DevBnaUpdateStatus();
}

void CNHMWICtrl::OnNoteErrorNxCashAcceptor(LPCTSTR Reason)
{
	NHDEBUG(1, (_T("Reason: (%s) \n"), Reason));
	ProcSetDeviceEvent(L"BNA", L"NoteError", NULL);
	DevBnaUpdateStatus();
}

void CNHMWICtrl::OnSetCashUnitInfoCompleteNxCashAcceptor()
{
	NHDEBUG(1, (_T("%s \n"), _T("")));
	ProcSetDeviceEvent(L"BNA", L"SetCashUnitInfoComplete", NULL);
	DevBnaUpdateStatus();
}

void CNHMWICtrl::OnSetCashInLimitCompleteNxCashAcceptor()
{
	NHDEBUG(1, (_T("%s \n"), _T("")));
	ProcSetDeviceEvent(L"BNA", L"SetCashInLimitComplete", NULL);
	DevBnaUpdateStatus();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Method : Call From BNA to BNA
///////////////////////////////////////////////////////////////////////////////////////////////////

int CNHMWICtrl::DevBnaUpdateStatus()
{
	if (!m_bBnaOpened)											// 장치가Open되지 않았으면
	{
		NHDEBUG(1, (_T("Device not opened (%s)\n"), _T("")));
		return R_NORMAL;										// 상태갱신은 의미없음
	}

	CString strStatus = m_pBna.GetDeviceStatus();				// 장치상태조회 : 장치가 정상일경우에만 기타조회
	NHDEBUG(1, (_T("BNA - DeviceStatus(%s)\n"), strStatus));
	ProcSetDeviceStatus(L"BNA", L"StDeviceStatus", strStatus);

	return R_NORMAL;
}

