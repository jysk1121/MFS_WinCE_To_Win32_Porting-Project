#ifndef _KALIGNITE_DEVICE_MCU_MATRIX_H_
#define _KALIGNITE_DEVICE_MCU_MATRIX_H_
///////////////////////////////////////////////////////////////////////////////

#include ".\Common\StructDef.h"

// ----------------------------------------------------------------------------
// KALIGNITE CARD READER ACITVEX METHOD
// ----------------------------------------------------------------------------
METHODLIST	McuMethod[] = 
{
	{"OpenSessionSync"				,	"NO"  },					// 
	{"Reset"						,	"YES" } , 				// 초기화
	{"EntryEnable"					,	"NO"  }	,				// Entry처리를 위한 의사CMD : 20030814_1.0_1066 : EntryEnable
	{"ReadRawData"					,	"YES" } ,				// 엔트리 발행및 투입시 Read
	{"CancelAccept"					,	"YES" }	,				// 엔트리 중지
	{"EjectMedia"					,	"YES" } ,				// 카드 방출
	{"RetainMedia"					,	"YES" } ,				// 카드 회수
	{"WriteRawData"					,	"YES" } ,				// 지정트랙 기록
	{"ChipIO"						,	"YES" } ,				// IC Send & Receive
	{"ChipPower"					,	"YES" } , 				// IC POWER
	{"CardRead"						,	"NO"  } ,				// ReadComplete/Invalid를 대기하기 위한 의사CMD
	{"WaitTaken"					,	"NO"  } 				// MediaRemoved 대기하기 위한 의사CMD
};

// ----------------------------------------------------------------------------
// KALIGNITE CARD READER ACITVEX EVENT
// ----------------------------------------------------------------------------
ITEMLIST	McuEvent[] =
{
	"OpenComplete"					,							// OpenSession 완료
	"ResetComplete"					,							// 초기화 완료
	"MediaDetected"					,							// 초기화 완료 후 카드 상태
	"EntryEnabled"					,							// EntryEnable CMD처리를 위한 가상EVENT(MWI발생) : 20030814_1.0_1066 : EntryEnable
	"AcceptCancelled"				,							// 엔트리 중지완료
	"MediaInserted"					,							// 카드 삽입됨
	"ReadComplete"					,							// 카드 읽음
	"InvalidMedia"					,							// 카드 읽음-무효카드
	"InvalidTrackData"				,							// 카드 읽음-트랙이상
	"EjectComplete"					,							// 카드 방출됨
	"RetainComplete"				,							// 카드 회수됨
	"MediaRemoved"					,							// 카드 수취됨
	"WriteComplete"					,							// 카드 기록됨
	"ChipIOComplete"				,							// 
	"ChipPowerComplete"				,							//
	"ChipIOFailure"					,							// 
	"Timeout"						,							// 카드부 동작중 타임아웃발생
	"FatalError"					,							//
	"DeviceError"												// 
};

// ----------------------------------------------------------------------------
// KALIGNITE CARD READER ACITVEX METHOD/EVENT COUNT
// ----------------------------------------------------------------------------
static const int McuMethodCount = sizeof(McuMethod) / sizeof(METHODLIST);
static const int McuEventCount  = sizeof(McuEvent) / sizeof(ITEMLIST);

// ----------------------------------------------------------------------------
// KALIGNITE CARD READER ACITVEX METHOD/EVENT MATRIX
// ----------------------------------------------------------------------------
ITEMLIST	McuMatrix[McuMethodCount][McuEventCount] = 
{
/*	-----------------------------------------------------------------------------------------------------------------------------------------------------------
	"NEXTWARE CARD READER"	/	"OpenComplete"
							/			"ResetComplete"
							/					"MediaDetected"
							/							"EntryEnabled"
							/ 									"AcceptCancelled"
							/ 											"MediaInserted"
							/													"ReadComplete"
							/															"InvalidMedia"
							/																	"InvalidTrackData"
							/																			"EjectComplete"	
							/																					"RetainComplete"
							/																							"MediaRemoved"
							/																									"WriteComplete"	
							/																											"ChipIOComplete"
							/																													"ChipPowerComplete"
							/																															"ChipIOFailure"
							/																																	"Timeout"	
							/																																			"FatalError"
							/																																					"DeviceError"
    --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
	/*"OpenSessionSync"		*/{	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES"},
	/*"Reset"				*/{	""   ,	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES"},
	/*"EntryEnable"			*/{	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES",	"YES"},
	/*"ReadRawData"			*/{	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES",	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES",	"YES"},
	/*"CancelAccept"		*/{	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES"},
	/*"EjectMedia"			*/{	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES",	"YES"},
	/*"RetainMedia"			*/{	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES"},
	/*"WriteRawData"		*/{	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES",	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	"YES",	"YES"},
	/*"ChipIO"				*/{	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	"YES",	"YES",	"YES",	"YES"},
	/*"ChipPower"			*/{	""	 ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	"YES",	"YES"},
	/*"CardRead"			*/{	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES",	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	"YES",	"YES"},
	/*"WaitTaken"			*/{	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	""   ,	"YES",	""   ,	""   ,	""   ,	""   ,	"YES",	"YES",	"YES"}
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
};

// ============================================================================
///////////////////////////////////////////////////////////////////////////////
#endif // _KALIGNITE_DEVICE_MCU_MATRIX_H_
