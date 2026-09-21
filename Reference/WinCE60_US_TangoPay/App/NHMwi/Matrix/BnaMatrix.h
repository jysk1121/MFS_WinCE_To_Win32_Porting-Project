#ifndef _DEVICE_BNA_MATRIX_H_
#define _DEVICE_BNA_MATRIX_H_
///////////////////////////////////////////////////////////////////////////////

#include ".\Common\StructDef.h"

// ----------------------------------------------------------------------------
// CASH ACCEPTOR ACITVEX METHOD
// ----------------------------------------------------------------------------
METHODLIST	BnaMethod[] = 
{
	// 21 methods, based on Event Table in 'Nextware30 ActiveX CashAcceptor Interface Programmer's Reference Guide(English).pdf'
	// The OCX file itself has more methods/events than the document, but I don't think we need them for now.
	{"OpenSessionSync"				, "NO"  },
	{"CloseSessionSync"				, "YES" },
	{"RequestLock"					, "YES" },
	{"ReleaseLock"					, "YES" },
	{"Reset"						, "YES" },
	{"StartCashIn"					, "YES" },
	{"AcceptCash"					, "YES" },
	{"CancelAccept"					, "YES" },
	{"StoreCash"					, "YES" },
	{"RollbackCash"					, "YES" },
	{"Retract"						, "YES" },
	{"OpenShutter"					, "YES" },
	{"CloseShutter"					, "YES" },
	{"StartExchangeSync"			, "YES" },
	{"EndExchangeSync"				, "YES" },
	{"OpenSafeDoor"					, "YES" },
	{"ConfigureNoteTypeSync"		, "YES" },
	{"CreateP6Signature"			, "YES" },
	{"GetP6Signature"				, "YES" },
	{"ConfigureCashUnitSync"		, "YES" },
	{"SetCashUnitInfo"				, "YES" }
	// a comment about "szMethodSPUse" field from somewhere:
	/*
	// USE Field가 "NO"인것은 대부분이 연속처리이므로 이때 버퍼를 비우면
	// 연속이벤트를 확인처리 하지 못하는 경우가 발생된다. 따라서 이때는 큐를 비우지 않는다
	*/
};

// ----------------------------------------------------------------------------
// CASH ACCEPTOR ACITVEX EVENT
// ----------------------------------------------------------------------------
ITEMLIST	BnaEvent[] =
{
	"OpenComplete"					,							// "OpenComplete" is raised by AP itself, not by NextWare.
	"LockGranted"					,
	"LockReleased"					,
	"LockTimeout"					,
	"StartCashInComplete"			,
	"AcceptCashComplete"			,
	"StoreCashComplete"				,
	"RollbackCashComplete"			,
	"OpenShutterComplete"			,
	"CloseShutterComplete"			,
	"AcceptCancelled"				,
	"RetractComplete"				,
	"OpenSafeDoorComplete"			,
	"ResetComplete"					,
	"ItemsPresented"				,
	"ItemsTaken"					,
	"ItemsRefused"					,
	"ItemsInserted"					,
	"MediaDetected"					,
	"CreateP6SignatureComplete"		,
	"GetP6SignatureComplete"		,
	"P6Inputed"						,
	"Timeout"						,
	"NoteError"						,
	"SetCashUnitInfoComplete"		,
	"CashUnitThreshold"				,
	"CashUnitChanged"				,
	"CashUnitError"					,
	"StatusChanged"					,
	"FatalError"					,
	"DeviceError"
};	// 31 events

// ----------------------------------------------------------------------------
// CASH ACCEPTOR ACITVEX METHOD/EVENT COUNT
// ----------------------------------------------------------------------------
static const int BnaMethodCount = sizeof(BnaMethod) / sizeof(METHODLIST);
static const int BnaEventCount  = sizeof(BnaEvent) / sizeof(ITEMLIST);

// ----------------------------------------------------------------------------
// CASH ACCEPTOR ACITVEX METHOD/EVENT MATRIX
// ----------------------------------------------------------------------------
ITEMLIST	BnaMatrix[BnaMethodCount][BnaEventCount] = 
{
	/*---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
	/*									1		2		3		4		5		6		7		8		9		10		11		12		13		14		15		16		17		18		19		20		21		22		23		24		25		26		27		28		29		30		31
	/									"OpenComplete"
	/											 "LockGranted"
	/													 "LockReleased"
	/														     "LockTimeout"
	/															  	     "StartCashInComplete"
	/							 										        "AcceptCashComplete"
	/																					"StoreCashComplete"
	/																							"RollbackCashComplete"
	/																									"OpenShutterComplete"
	/																											"CloseShutterComplete"
	/																													"AcceptCancelled"
	/																															"RetractComplete"
	/																																	"OpenSafeDoorComplete"
	/																																			"ResetComplete"
	/																																					"ItemsPresented"
	/																																							"ItemsTaken"
	/																																									"ItemsRefused"
	/																																											"ItemsInserted"
	/																																													"MediaDetected"
	/																																															"CreateP6SignatureComplete"
	/																																																	"GetP6SignatureComplete"
	/																																																			"P6Inputed"
	/																																																					"Timeout"
	/																																																							"NoteError"
	/																																																									"SetCashUnitInfoComplete"
	/																																																											"CashUnitThreshold"
	/																																																													"CashUnitChanged"
	/																																																															"CashUnitError"
	/																																																																	"StatusChanged"
	/																																																																			"FatalError"
	/																																																																					"DeviceError"
	/*---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
	/*									1		2		3		4		5		6		7		8		9		10		11		12		13		14		15		16		17		18		19		20		21		22		23		24		25		26		27		28		29		30		31
	/*"OpenSessionSync"				*/{	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",  "YES"},
	/*"CloseSessionSync"			*/{	""   ,	""   ,	""   ,	""	 ,	""	 ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES"},
	/*"RequestLock"					*/{	""   ,	"YES",	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   },
	/*"ReleaseLock"					*/{	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   },
	/*"Reset"						*/{	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	"YES",	"YES"},
	/*"StartCashIn"					*/{	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES"},
	/*"AcceptCash"					*/{	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES",	"YES",	""   ,	""   ,	""   ,	"YES",	"YES",	"YES",	""   ,	""   ,	""   ,	"YES",	""   ,	"YES",	"YES"},
	/*"CancelAccept"				*/{	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES",	""   ,	""   ,	""   ,	"YES",	""   ,	"YES",	""   ,	""   ,	""   ,	"YES",	""   ,	"YES",	"YES"},
	/*"StoreCash"					*/{	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	"YES",	"YES"},
	/*"RollbackCash"				*/{	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	"YES",	"YES"},
	/*"Retract"						*/{	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES",	""   ,	""   ,	""   ,	"YES",	""   ,	"YES",	"YES"},
	/*"OpenShutter"					*/{	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES"},
	/*"CloseShutter"				*/{	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES"},
	/*"StartExchangeSync"			*/{	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   },
	/*"EndExchangeSync"				*/{	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   },
	/*"OpenSafeDoor"				*/{	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES"},
	/*"ConfigureNoteTypeSync"		*/{	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   },
	/*"CreateP6Signature"			*/{	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	"YES",	"YES",	"YES",	"YES",	""   ,	"YES",	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES"},
	/*"GetP6Signature"				*/{	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   },
	/*"ConfigureCashUnitSync"		*/{	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   },
	/*"SetCashUnitInfo"				*/{	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	"YES",	"YES"}
	/*---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
};

// ============================================================================
///////////////////////////////////////////////////////////////////////////////
#endif // _DEVICE_BNA_MATRIX_H_
