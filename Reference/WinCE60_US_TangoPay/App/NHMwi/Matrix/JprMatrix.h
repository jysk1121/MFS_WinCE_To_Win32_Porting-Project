#ifndef _KALIGNITE_DEVICE_JPR_MATRIX_H_
#define _KALIGNITE_DEVICE_JPR_MATRIX_H_
///////////////////////////////////////////////////////////////////////////////

#include ".\Common\StructDef.h"

// ----------------------------------------------------------------------------
// KALIGNITE JOURNAL PRINTER ACITVEX METHOD
// ----------------------------------------------------------------------------
METHODLIST	JprMethod[] = 
{
	{"OpenSessionSync"				, "NO"  },					// 
	{"PrintForm"					, "YES" },					// 인자(인자만 수행)
	{"ControlMedia"					, "YES" },					// 매체 확장제어(개별움직임제어/명세표컷팅)
	{"EmbossPrint"					, "YES" }					// 엠보스인자
};

// ----------------------------------------------------------------------------
// KALIGNITE JOURNAL PRINTER ACITVEX EVENT
// ----------------------------------------------------------------------------
							
						
ITEMLIST	JprEvent[] =
{
	"OpenComplete"					,							// OpenSession 완료
	"PrintFormComplete"				,							// 인자완료
	"ControlComplete"				,							// 확장제어 완료
	"FiedWarning"					,							// 
	"FieldError"					,							// 
	"Timeout"						,							// 동작중 타임아웃
	"FatalError"					,							//
	"DeviceError"												// 동작중 장애발생
};

// ----------------------------------------------------------------------------
// KALIGNITE JOURNAL PRINTER ACITVEX METHOD/EVENT COUNT
// ----------------------------------------------------------------------------
static const int JprMethodCount = sizeof(JprMethod) / sizeof(METHODLIST);
static const int JprEventCount  = sizeof(JprEvent) / sizeof(ITEMLIST);

// ----------------------------------------------------------------------------
// KALIGNITE JOURNAL PRINTER ACITVEX METHOD/EVENT MATRIX
// ----------------------------------------------------------------------------
ITEMLIST	JprMatrix[JprMethodCount][JprEventCount] = 
{
/*	-----------------------------------------------------------------------------------------------------------------------------------------------------------
	"NEXTWARE JOURNAL PRINTER"		 /	"OpenComplete"
									 /			"PrintFormComplete"
									 /					"FiedWarning"
									 /							"FieldError"
									 /									"ControlComplete"
									 /											"Timeout"
									 /													"FatalError"
									 /															"DeviceError"
      -----------------------------------------------------------------------------------------------------------------------------------------------------------*/
	/*"OpenSessionSync"				*/{	"YES", ""   ,	""   ,  ""   ,	""   ,	""   ,	"YES",	"YES"},
	/*"PrintForm"					*/{	""   , "YES",	"YES",	"YES",	""   ,	"YES",	"YES",	"YES"},
	/*"ControlMedia"				*/{	""   , ""   ,	""   ,	""   ,	"YES",	"YES",	"YES",	"YES"},
	/*"EmbossPrint"					*/{	""   , "YES",	"YES",	"YES",	""   ,	"YES",	"YES",	"YES"}
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------
};

// ============================================================================
///////////////////////////////////////////////////////////////////////////////
#endif // _KALIGNITE_DEVICE_JPR_MATRIX_H_
