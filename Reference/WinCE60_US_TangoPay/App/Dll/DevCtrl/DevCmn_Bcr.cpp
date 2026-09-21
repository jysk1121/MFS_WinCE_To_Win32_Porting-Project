/////////////////////////////////////////////////////////////////////////////
//	DevBcr.cpp : implementation file
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


/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnBCR_ClearErrorCode()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Clear Error Code
-------------------------------------------------------------------*/
int	CDevCmn::fnBCR_ClearErrorCode()
{
	NHDEBUG(DBG_CALL, (_T("fnBCR_ClearErrorCode() \n")));

	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_BCR))
		nReturn = m_pMwi->BcrClearErrorCode();

	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fstrBCR_GetErrorCode()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Get Error Code
-------------------------------------------------------------------*/
CString CDevCmn::fstrBCR_GetErrorCode()
{
	NHDEBUG(DBG_CALL, (_T("fstrBCR_GetErrorCode() \n")));

	CString strReturn("");

	if (fnAPL_GetDefineDevice(DEV_BCR))
		strReturn = m_pMwi->BcrGetErrorCode();

	if(strReturn.GetLength() > 0)
	{
		NVDump('F', 'E', "56", L"", strReturn);		
	}

	return strReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnBCR_GetDeviceStatus()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Get Device Status
-------------------------------------------------------------------*/
int	CDevCmn::fnBCR_GetDeviceStatus()
{
	//NHDEBUG(DBG_CALL, (_T("fnBCR_GetDeviceStatus() \n")));

	static	int		nSaveReturn = FALSE;
	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_BCR))
		nReturn = m_pMwi->BcrGetDeviceStatus();

	if (nReturn != nSaveReturn)
	{
		NHDEBUG(DBG_CALL, (L"'%S' => '%S'\n", GETSTR_DVCST(nSaveReturn), GETSTR_DVCST(nReturn)));
		nSaveReturn = nReturn;
		m_strNVLog.Format(L"1:BCRST(%d)", nReturn);
		NVDump('O', 'E', "58", L"", m_strNVLog);
	}
	return nReturn;
}


/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnBCR_Initialize()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Initialize
-------------------------------------------------------------------*/
int	CDevCmn::fnBCR_Initialize()
{
	NHDEBUG(DBG_CALL, (_T(" \n")));

	int		nReturn = FALSE;

	DeviceOperationFlag = TRUE;
	if (fnAPL_GetDefineDevice(DEV_BCR))
		nReturn = m_pMwi->BcrInitialize();

	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnBCR_Deinitialize()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : De-Initialize
-------------------------------------------------------------------*/
int	CDevCmn::fnBCR_Deinitialize()
{
	NHDEBUG(DBG_CALL, (_T("\n")));

	int		nReturn = FALSE;

	DeviceOperationFlag = TRUE;
	if (fnAPL_GetDefineDevice(DEV_BCR))
		nReturn = m_pMwi->BcrDeinitialize();

	return nReturn;
}


/*-------------------------------------------------------------------
CLASS    NAME: CDevCmn
FUNCTION NAME: fnBCR_SendRawData()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CDevCmn::fnBCR_SendRawData(BOOL bWait)
{
	NHDEBUG(DBG_CALL, (L"\n"));

	DeviceOperationFlag = TRUE;

	if (fnAPL_GetDefineDevice(DEV_BCR))
	{
		fnAPL_CheckDeviceAction(DEV_BCR);

		long result = m_pMwi->BcrSendRawData();
		NHDEBUG(DBG_CALL, (L"Result: %d \n", result));

		if (bWait)
		{
			fnAPL_CheckDeviceAction(DEV_BCR);

			fnAPL_CheckDevice();

			if (!fnAPL_GetDownErrorDevice(DEV_BCR))
				return TRUE;
		}

		return TRUE;
	}

	return FALSE;
}


/*-------------------------------------------------------------------
CLASS    NAME: CDevCmn
FUNCTION NAME: fnBCR_CancelAccept()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CDevCmn::fnBCR_CancelAccept()
{
	NHDEBUG(DBG_CALL, (L"fnBCR_CancelAccept() \n"));

	int nReturn = FALSE;
	DeviceOperationFlag = TRUE;

	if (fnAPL_GetDefineDevice(DEV_BCR))
	{
		//NHDEBUG(DBG_CALL, (L"fnAPL_CheckDeviceAction(DEV_BCR) #1... \n"));
		//fnAPL_CheckDeviceAction(DEV_BCR);

		nReturn = m_pMwi->BcrCancelAccept();
		NHDEBUG(DBG_CALL, (L"m_pMwi->BcrCancelAccept() returns: [%d] \n", nReturn));

		// wait 'AcceptCancelled' event
		fnAPL_CheckDeviceAction(DEV_BCR);
		fnAPL_CheckDevice();

		if (!fnAPL_GetDownErrorDevice(DEV_BCR))
			nReturn = TRUE;
	}

	NHDEBUG(DBG_CALL, (L"fnBCR_CancelAccept() returns: [%d] \n", nReturn));
	return nReturn;
}


/*-------------------------------------------------------------------
CLASS    NAME: CDevCmn
FUNCTION NAME: fnBCR_Reset()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CDevCmn::fnBCR_Reset()
{
	NHDEBUG(DBG_CALL, (L"fnBCR_Reset() \n"));

	int nReturn = FALSE;
	DeviceOperationFlag = TRUE;

	if (fnAPL_GetDefineDevice(DEV_BCR))
	{
		nReturn = m_pMwi->BcrReset();
		NHDEBUG(DBG_CALL, (L"m_pMwi->BcrReset() returns: [%d] \n", nReturn));

		// wait 'ResetComplete' event
		fnAPL_CheckDeviceAction(DEV_BCR);
		fnAPL_CheckDevice();

		if (!fnAPL_GetDownErrorDevice(DEV_BCR))
			nReturn = TRUE;
	}

	NHDEBUG(DBG_CALL, (L"fnBCR_Reset() returns: [%d] \n", nReturn));
	return nReturn;
}


/*-------------------------------------------------------------------
CLASS    NAME: CDevCmn
FUNCTION NAME: fstrBCR_GetBarcodeData()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : Get Barcode Data
-------------------------------------------------------------------*/
CString CDevCmn::fstrBCR_GetBarcodeData()
{
	NHDEBUG(DBG_CALL, (_T(" \n")));

	CString strReturn("");

	if (fnAPL_GetDefineDevice(DEV_BCR))
	{
		strReturn = m_pMwi->BcrGetBarcodeData();

		if(strReturn.GetLength() > 0)
			NVDump('F', 'E', "56", L"", strReturn);		
	}

	NHDEBUG(DBG_CALL, (_T("strReturn: [%s]\n"), strReturn));
	return strReturn;
}
