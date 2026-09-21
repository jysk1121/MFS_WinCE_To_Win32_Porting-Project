#ifndef _KALIGNITE_DEVICE_SPR_MATRIX_H_
#define _KALIGNITE_DEVICE_SPR_MATRIX_H_
///////////////////////////////////////////////////////////////////////////////

#include ".\Common\StructDef.h"

// ----------------------------------------------------------------------------
// KALIGNITE RECEIPT PRINTER ACITVEX METHOD
// ----------------------------------------------------------------------------
METHODLIST	SprMethod[] = 
{
	{"OpenSessionSync"				, "NO"  },					// 
	{"PrintForm"					, "YES" },					// 인자(인자만 수행)
	{"Reset"						, "YES" },					// 장치초기화
	{"ControlMedia"					, "YES" },					// 매체 확장제어(EJECT/CUT)
	{"CancelWaitForMediaTaken"		, "YES" },					// 매체 수취대기 취소
	{"RetractMedia"					, "YES" },					// 매체 회수처리
	{"ResetRetractCount"			, "YES" },					// 회수매수초기화
	{"WaitTaken"					, "NO"  }
};

// ----------------------------------------------------------------------------
// KALIGNITE RECEIPT PRINTER ACITVEX EVENT
// ----------------------------------------------------------------------------
ITEMLIST	SprEvent[] =
{
	"OpenComplete"					,							// OpenSession 완료
	"PrintFormComplete"				,							// 1 인자완료
	"FieldWarning"					,							// 2 인자오류
	"FieldError"					,							// 3 인자오류
	"ResetComplete"					,							// 4 장치초기화완료
	"ControlMediaComplete"			,							// 5 확장제어 완료
	"MediaTaken"					,							// 6 수취완료
	"WaitCancelled"					,							// 7 수취대기취소완료
	"RetractMediaComplete"			,							// 8 회수완료
	"ResetCountComplete"			,							// 9 회수매수초기화완료
	"Timeout"						,							// 10동작중 타임아웃
	"FatalError"					,							// 11동작중 장애발생
	"DeviceError"												// 12동작중 장애발생
};

// ----------------------------------------------------------------------------
// KALIGNITE RECEIPT PRINTER ACITVEX METHOD/EVENT COUNT
// ----------------------------------------------------------------------------
static const int SprMethodCount = sizeof(SprMethod) / sizeof(METHODLIST);
static const int SprEventCount  = sizeof(SprEvent) / sizeof(ITEMLIST);

// ----------------------------------------------------------------------------
// KALIGNITE RECEIPT PRINTER ACITVEX METHOD/EVENT MATRIX
// ----------------------------------------------------------------------------
ITEMLIST	SprMatrix[SprMethodCount][SprEventCount] = 
{
/*	-----------------------------------------------------------------------------------------------------------------------------------------------------------
	"NEXTWARE RECEIPT PRINTER"		 /	"OpenComplete"		
									 /			"PrintFormComplete"
									 /					"FieldWarning"	
									 /							"FieldError"	
									 /									"ResetComplete"	
									 /											"ControlMediaComplete"
									 /													"MediaTaken"
									 /															"WaitCancelled"
									 /																	"RetractMediaComplete"		
									 /																			"ResetCountComplete"		
									 /																					"Timeout"		
									 /																							"FatalError"	
									 /																									"DeviceError"	
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
	/*"OpenSessionSync"				*/{	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""	 ,	""   ,	""   ,	"YES",	"YES"},
	/*"PrintForm"					*/{	""   ,	"YES",	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	""	 ,	""   ,	"YES",	"YES",	"YES"},
	/*"Reset"						*/{	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""	 ,	""	 ,	""   ,	"YES",	"YES"},
	/*"ControlMedia"				*/{	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES",	""   ,	""   ,	""   ,	"YES",	"YES",	"YES"},
	/*"CancelWaitForMediaTaken"		*/{	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""	 ,	"YES",	""   ,	""   ,	"YES",	"YES",	"YES"},
	/*"RetractMedia"				*/{	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	"YES",	"YES",	"YES"},
	/*"ResetRetractCount"			*/{	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""	 ,	""	 ,	""	 ,	"YES",	""   ,	"YES",	"YES"},
	/*"WaitTaken"					*/{	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""	 ,	""	 ,	""   ,	"YES",	"YES",	"YES"}
	//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
};

// ============================================================================
///////////////////////////////////////////////////////////////////////////////
#endif // _KALIGNITE_DEVICE_SPR_MATRIX_H_
