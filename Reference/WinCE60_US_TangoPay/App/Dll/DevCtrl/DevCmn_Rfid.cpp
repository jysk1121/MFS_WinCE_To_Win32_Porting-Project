/////////////////////////////////////////////////////////////////////////////
//	DevRfid.cpp : implementation file
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include ".\Dev\DevCmn.h"

//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------

//#define NH_DEBUG		// Debug Message On/Off
#include ".\Common\NHDbgApi.h"

#define	DBG_CALL		1
#define DBG_INFO		1

// [#2325] NH KSK 2015.01.20
/*-------------------------------------------------------------------
CLASS    NAME: CDevCmn
FUNCTION NAME: fnRFID_Initialize()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : Initialize
-------------------------------------------------------------------*/
int	CDevCmn::fnRFID_Initialize()
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnRFID_Initialize]\n"));

	int		nReturn = FALSE;

	DeviceOperationFlag = TRUE;
	if (fnAPL_GetDefineDevice(DEV_RFID))
		nReturn = m_pMwi->RFIDInitialize();

	return nReturn;
}
// end of [#2325]

// [#2325] NH KSK 2015.01.20
/*-------------------------------------------------------------------
CLASS    NAME: CDevCmn
FUNCTION NAME: fnRFID_Deinitialize()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : Deinitialize
-------------------------------------------------------------------*/
int	CDevCmn::fnRFID_Deinitialize()
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnRFID_Deinitialize]\n"));

	int		nReturn = FALSE;

	DeviceOperationFlag = TRUE;
	if (fnAPL_GetDefineDevice(DEV_RFID))
		nReturn = m_pMwi->RFIDDeinitialize();

	return nReturn;
}
// end of [#2325]

// [#2325] NH KSK 2015.01.20
/*-------------------------------------------------------------------
CLASS    NAME: CDevCmn
FUNCTION NAME: fstrRFID_GetErrorCode()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : Get Error Code
-------------------------------------------------------------------*/
CString CDevCmn::fstrRFID_GetErrorCode()
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fstrRFID_GetErrorCode]\n"));

	CString strReturn("");

	if (fnAPL_GetDefineDevice(DEV_RFID))
		strReturn = m_pMwi->RFIDGetErrorCode();

	return strReturn;
}
//end of [#2325]

// [#2325] NH KSK 2015.01.20
/*-------------------------------------------------------------------
CLASS    NAME: CDevCmn
FUNCTION NAME: fnRFID_ClearErrorCode()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : Clear Error Code
-------------------------------------------------------------------*/
int	CDevCmn::fnRFID_ClearErrorCode()
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnRFID_ClearErrorCode]\n"));

	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_RFID))
		nReturn = m_pMwi->RFIDClearErrorCode();

	return nReturn;
}
// end of [#2325]

// [#2325] NH KSK 2015.01.20
/*-------------------------------------------------------------------
CLASS    NAME: CDevCmn
FUNCTION NAME: fnRFID_GetDeviceStatus()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : Get Device Status
-------------------------------------------------------------------*/
int	CDevCmn::fnRFID_GetDeviceStatus()
{
	//NHDEBUG(DBG_CALL, (L"[CDevCmn::fnRFID_GetDeviceStatus]\n"));

	static	int		nSaveReturn = FALSE;
	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_RFID))
		nReturn = m_pMwi->RFIDGetDeviceStatus();

	if (nReturn != nSaveReturn)
	{
		NHDEBUG(DBG_CALL, (L"[CDevCmn::fnRFID_GetDeviceStatus] (%d) -> (%d)\n", nSaveReturn, nReturn));
		nSaveReturn = nReturn;
		m_strNVLog.Format(L"1:RFIDST(%d)", nReturn);
		NVDump('O', 'E', "73", L"", m_strNVLog);
	}
	return nReturn;
}
// end of [#2325]

// [#2325] NH KSK 2015.01.20
/*-------------------------------------------------------------------
CLASS    NAME: CDevCmn
FUNCTION NAME: fstrRFID_GetCardData()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : Get Card Data
-------------------------------------------------------------------*/
CString CDevCmn::fstrRFID_GetCardData()
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fstrRFID_GetCardData]\n"));

	CString strReturn("");

	if (fnAPL_GetDefineDevice(DEV_RFID))
		strReturn = m_pMwi->RFIDGetCardData();

	return strReturn;
}
// end of [#2325]

// [#2325] NH KSK 2015.01.20
/*-------------------------------------------------------------------
CLASS    NAME: CDevCmn
FUNCTION NAME: fnRFID_EntryEnable()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : Entry Enable(Wait Time)
-------------------------------------------------------------------*/
int	CDevCmn::fnRFID_EntryEnable(int nWaitSec)
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnRFID_EntryEnable]\n"));

	int		nReturn = FALSE;

	DeviceOperationFlag = TRUE;
	if (fnAPL_GetDefineDevice(DEV_RFID))
		nReturn = m_pMwi->RFIDEntryEnable(nWaitSec);

	return nReturn;
}
// end of [#2325]

// [#2325] NH KSK 2015.01.20
/*-------------------------------------------------------------------
CLASS    NAME: CDevCmn
FUNCTION NAME: fnRFID_EntryDisable()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : Entry Disable
-------------------------------------------------------------------*/
int	CDevCmn::fnRFID_EntryDisable()
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnRFID_EntryDisable]\n"));

	int		nReturn = FALSE;

	DeviceOperationFlag = TRUE;
	if (fnAPL_GetDefineDevice(DEV_RFID))
		nReturn = m_pMwi->RFIDEntryDisable();

	return nReturn;
}
// end of [#2325]