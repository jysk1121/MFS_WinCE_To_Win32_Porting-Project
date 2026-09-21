#ifndef _KALIGNITE_DEVICE_CDU_MATRIX_H_
#define _KALIGNITE_DEVICE_CDU_MATRIX_H_
///////////////////////////////////////////////////////////////////////////////

#include ".\Common\StructDef.h"

// ----------------------------------------------------------------------------
// CASH DISPENSER ACITVEX METHOD
// ----------------------------------------------------------------------------
METHODLIST	CduMethod[] = 
{
	{"OpenSessionSync"				, "NO"  },					// 
	{"Reset"						, "YES" },					// Reset
	{"Denominate"					, "YES" },					// 출금권종/매수연산
	{"Dispense"						, "YES" },					// 계수
	{"Present"						, "YES" },					// 방출및수취대기
	{"Retract"						, "YES" },					// 회수
	{"Reject"						, "YES" },					// 반각
	{"OpenShutter"					, "YES" },					// 셔터열기
	{"CloseShutter"					, "YES" },					// 셔터닫기
	{"WaitTaken"					, "NO"  }					// 수취대기				2004.04.07
};

// ----------------------------------------------------------------------------
// CASH DISPENSER ACITVEX EVENT
// ----------------------------------------------------------------------------
ITEMLIST	CduEvent[] =
{
	"OpenComplete"					,							// OpenSession 완료
	"ResetComplete"					,							// Reset Complete
	"MediaDetected"					,							// Media Detected
	"DenominateComplete"			,							// 출금권종/매수연산완료
	"NotDispensable"				,							// 출금불가확인
	"DispenseComplete"				,							// 계수완료
	"PresentComplete"				,							// 방출완료
	"ItemsTaken"					,							// 수취완료
	"Timeout"						,							// 타임아웃
	"RetractComplete"				,							// 회수완료
	"RejectComplete"				,							// 반각완료
	"OpenShutterComplete"			,							// 셔터열기완료
	"CloseShutterComplete"			,							// 셔터닫기완료
	"CashUnitError"					,							// 장애
	"CashUnitChanged"				,							// 상태변경
	"CashUnitThreshold"				,							// 
	"DeviceError"					,							// 장애발생
	"FatalError"					 							// 장애발생
};

// ----------------------------------------------------------------------------
// CASH DISPENSER ACITVEX METHOD/EVENT COUNT
// ----------------------------------------------------------------------------
static const int CduMethodCount = sizeof(CduMethod) / sizeof(METHODLIST);
static const int CduEventCount  = sizeof(CduEvent) / sizeof(ITEMLIST);

// ----------------------------------------------------------------------------
// CASH DISPENSER ACITVEX METHOD/EVENT MATRIX
// ----------------------------------------------------------------------------
ITEMLIST	CduMatrix[CduMethodCount][CduEventCount] = 
{
/*	--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	"NEXTWARE CASH DISPENSER"		 /	"OpenComplete"
									 /		 "Resetcomplete"
									 /				 "MediaDetected"
									 /					     "DenominateComplete"
									 /						   	     "NotDispensable"
									 /							 	        "DispenseComplete"
									 /												"PresentComplete"
									 /														"ItemsTaken"
									 /																"Timeout"
									 /																		"RetractComplete"
									 /																				"RejectComplete"
									 /																						"OpenShutterComplete"
									 /																								"CloseShutterComplete"
									 /																										"CashUnitError"
									 /																												"CashUnitChanged"
									 /																														"CashUnitThreshold"
									 /																																"DeviceError"
									 /																																		"FatalError"
    ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
	/*"OpenSessionSync"				*/{	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",  "YES"},
	/*"Reset"						*/{	""   ,	"YES",	"YES",	""	 ,	""	 ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES"},
	/*"Denominate"					*/{	""   ,	""   ,	""   ,	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES"},
	/*"Dispense"					*/{	""   ,	""   ,	""   ,	""   ,	"YES",	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	"YES",	"YES"},
	/*"Present"						*/{	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES"},
	/*"Retract"						*/{	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES"},
	/*"Reject"						*/{	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES"},
	/*"OpenShutter"					*/{	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	"YES",	"YES"},
	/*"CloseShutter"				*/{	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	"YES",	"YES"},
	/*"WaitTaken"					*/{	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES"}
	//-------------------------------------------------------------------------------------------------------------------------------------------------------------
};

// ============================================================================
///////////////////////////////////////////////////////////////////////////////
#endif // _KALIGNITE_DEVICE_CDU_MATRIX_H_
