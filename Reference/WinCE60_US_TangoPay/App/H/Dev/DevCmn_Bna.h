#ifndef __DEV_CMN_BNA_H__

#define __DEV_CMN_BNA_H__

/////////////////////////////////////////////////////////////////////////////
//	BILL ACCEPTOR FUNCTION(BNA)
/////////////////////////////////////////////////////////////////////////////
// BNA SET MODE
int		fnBNA_ClearErrorCode();
int		fnBNA_ClearCashInInfo();

// BNA GET MODE
CString	fstrBNA_GetErrorCode();
int		fnBNA_GetDeviceStatus();
CString	fstrBNA_GetExtraStatus(CString KeyName);

// BNA OPERATION MODE
int		fnBNA_Initialize();
int		fnBNA_Deinitialize();

int		fnBNA_StartCashIn();
int		fnBNA_AcceptCash(long InsertionTime, long TakenTime);
int		fnBNA_StoreCash();
int		fnBNA_RollbackCash(long TakenTimeOut);
int		fnBNA_CancelAccept();

//int		fnBNA_SetCashUnitInformation();
int		fnBNA_GetCashUnitInformation();
int		fnBNA_GetLastCashInValue();
CString	fstrBNA_GetLastCashInStatus();

CString	fstrBNA_GetAcceptorStatus();
CString	fstrBNA_GetStackerStatus();
int		fnBNA_GetMaxCashInItem();
int		fnBNA_GetMaxStackerItem();
int		fnBNA_GetLastRefusedCount();

int     fnBNA_GetAggregateDeviceStatus();
int     fnBNA_GetTotalCount();
CString fstrBNA_GetPositionStatus();	// [RWC6-676] SKKim 2024.05.17
BOOL	fbBNA_GetRefusedStatus();		// [RWC6-676] SKKim 2024.05.27

#endif __DEV_CMN_BNA_H__