#ifndef _KALIGNITE_DEVICE_BCR_MATRIX_H_
#define _KALIGNITE_DEVICE_BCR_MATRIX_H_
///////////////////////////////////////////////////////////////////////////////

#include ".\Common\StructDef.h"

// ----------------------------------------------------------------------------
// BARCODE READER ACTIVEX METHOD
// ----------------------------------------------------------------------------
METHODLIST	BcrMethod[] = 
{
	{"OpenSessionSync"				, "NO"  },					// 
	{"SendRawData"					, "YES" },					// 
	{"CancelAccept"					, "YES" },					// 
	{"Reset"						, "YES" },					// 
};

// ----------------------------------------------------------------------------
// BARCODE READER ACTIVEX EVENT
// ----------------------------------------------------------------------------
ITEMLIST	BcrEvent[] =
{
	"OpenComplete"					,							// OpenSession 완료
	"SendRawDataComplete"			,							//
	"AcceptCancelled"				,							//
	"ResetComplete"					,							//
	"Timeout"						,							// 동작중 타임아웃
	"FatalError"					,							//
	"DeviceError"												// 동작중 장애발생
};

// ----------------------------------------------------------------------------
// BARCODE READER ACTIVEX METHOD/EVENT COUNT
// ----------------------------------------------------------------------------
static const int BcrMethodCount = sizeof(BcrMethod) / sizeof(METHODLIST);
static const int BcrEventCount  = sizeof(BcrEvent) / sizeof(ITEMLIST);

// ----------------------------------------------------------------------------
// BARCODE READER ACTIVEX METHOD/EVENT MATRIX
// ----------------------------------------------------------------------------
ITEMLIST	BcrMatrix[BcrMethodCount][BcrEventCount] = 
{
/*	-----------------------------------------------------------------------------------------------------------------------------------------------------------
	"NEXTWARE SCANNER PRINTER"		 /	"OpenComplete"
									 /			"SendRawDataComplete"
									 /					"AcceptCancelled"
									 /							"ResetComplete"
									 /									"Timeout"
									 /											"FatalError"
									 /													"DeviceError"
      -----------------------------------------------------------------------------------------------------------------------------------------------------------*/
	/*"OpenSessionSync"				*/{	"YES", ""   ,	""   ,	""   ,	""   ,	"YES",	"YES"},
	/*"SendRawData"					*/{	""   , "YES",	""   ,	""   ,	""   ,	"YES",	"YES"},
	/*"CancelAccept"				*/{	""   , ""   ,	"YES",	""   ,	""   ,	"YES",	"YES"},
	/*"Reset"						*/{	""   , ""   ,	""   ,	"YES",	""   ,	"YES",	""},
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------
};

// ============================================================================
///////////////////////////////////////////////////////////////////////////////
#endif // _KALIGNITE_DEVICE_BCR_MATRIX_H_
