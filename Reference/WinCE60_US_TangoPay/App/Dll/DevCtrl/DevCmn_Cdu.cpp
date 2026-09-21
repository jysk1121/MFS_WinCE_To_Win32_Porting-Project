/////////////////////////////////////////////////////////////////////////////
//	DevCdu.cpp : implementation file
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include ".\Common\CmnLib.h"
#include ".\Common\ConstDef.h"
#include ".\Common\ScreenDef.h"

#include ".\Dll\nhmwi.h"

#include ".\Dev\DevDefine.h"
#include ".\Dev\DevCmn.h"

#include ".\Scr\ScrCtrl.h"

#include ".\Tran\TranCmnDefine.h"	// [#463] [NH] KSK 2008.12.10

//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------

//#define NH_DEBUG		// Debug Message On/Off
#include ".\Common\NHDbgApi.h"

#define	DBG_CALL		1
#define DBG_INFO		1


/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnCDU_ClearErrorCode()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Clear Error Code
-------------------------------------------------------------------*/
int	CDevCmn::fnCDU_ClearErrorCode()
{
	NHDEBUG(1, (L"[CDevCmn::fnCDU_ClearErrorCode]\n"));

	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_CDU))
		nReturn = m_pMwi->CduClearErrorCode();

	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnCDU_ClearDispenseInfo()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Clear Dispense Information
-------------------------------------------------------------------*/
int	CDevCmn::fnCDU_ClearDispenseInfo()
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnCDU_ClearDispenseInfo]\n"));

	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_CDU))
	{
		fnCDU_SetCstInitialDispInfo(CDU_CST_ALL);
	}

	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnCDU_SetValueOfCash()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Set Value Of Cash(Cash Value Data)
-------------------------------------------------------------------*/
int	CDevCmn::fnCDU_SetValueOfCash(LPCTSTR szCashValueData)
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnCDU_SetValueOfCash]\n"));

	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_CDU))
		nReturn = m_pMwi->CduSetValueOfCash(szCashValueData);	// 2005.02.24

	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnCDU_SetMinMaxCST()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Set MinMax Of CST(Min Count Data, Max Count Data)
-------------------------------------------------------------------*/
int	CDevCmn::fnCDU_SetMinMaxCST(LPCTSTR szMinCntData, LPCTSTR szMaxCntData)
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnCDU_SetMinMaxCST]\n"));

	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_CDU))
		nReturn = m_pMwi->CduSetMinMaxCST(szMinCntData, szMaxCntData);

	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnCDU_SetNumberOfCash()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Set Number Of Cash(Cash Count Data)
-------------------------------------------------------------------*/
int	CDevCmn::fnCDU_SetNumberOfCash(LPCTSTR szCashCntData)
{
	NHDEBUG(1, (L"[CDevCmn::fnCDU_SetNumberOfCash]\n"));

	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_CDU))
		nReturn = m_pMwi->CduSetNumberOfCash(szCashCntData);

	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnCDU_AddNumberOfCash()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Add Number Of Cash(Cash Initial Data, Cash Add Count Data)
-------------------------------------------------------------------*/
int	CDevCmn::fnCDU_AddNumberOfCash(LPCTSTR szInitialCnt, LPCTSTR szAddCnt)
{
	NHDEBUG(1, (L"[CDevCmn::fnCDU_AddNumberOfCash]\n"));

	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_CDU))
		nReturn = m_pMwi->CduAddNumberOfCash(szInitialCnt, szAddCnt);

	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fstrCDU_GetErrorCode()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Get Error Code
-------------------------------------------------------------------*/
CString CDevCmn::fstrCDU_GetErrorCode()
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fstrCDU_GetErrorCode]\n"));

	CString strReturn("");

	if (fnAPL_GetDefineDevice(DEV_CDU))
		strReturn = m_pMwi->CduGetErrorCode();

	if(strReturn.GetLength() > 0)
	{
		NVDump('F', 'E', "98", L"", strReturn);		
	}
	return strReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fstrCDU_GetSensorInfo()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Get Sensor Information
-------------------------------------------------------------------*/
CString CDevCmn::fstrCDU_GetSensorInfo()	// 사용 안하는 함수임
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fstrCDU_GetSensorInfo]\n"));

	static	CString strSaveReturn("");
	CString			strReturn("");

	if (fnAPL_GetDefineDevice(DEV_CDU))
		strReturn = m_pMwi->CduGetSensorInfo();

	if (strReturn != strSaveReturn)
	{
		strSaveReturn = strReturn;
	}
	return strReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnCDU_GetDeviceStatus()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Get Device Status
-------------------------------------------------------------------*/
int	CDevCmn::fnCDU_GetDeviceStatus()
{
	//NHDEBUG(DBG_CALL, (L"[CDevCmn::fnCDU_GetDeviceStatus]\n"));

	static	int		nSaveReturn = FALSE;
	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_CDU))
		nReturn = m_pMwi->CduGetDeviceStatus();

	if (nReturn != nSaveReturn)
	{
		NHDEBUG(DBG_CALL, (L"[CDevCmn::fnCDU_GetDeviceStatus], Status Changed [%d] => [%d]\n", nSaveReturn, nReturn));

		nSaveReturn = nReturn;
		m_strNVLog.Format(L"1:CDUST(%d)", nReturn);
		NVDump('O', 'E', "0A", L"", m_strNVLog);
	}
	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnCDU_GetPosition()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Get Position
-------------------------------------------------------------------*/
int	CDevCmn::fnCDU_GetPosition()
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnCDU_GetPosition]\n"));

	static	int		nSaveReturn = 0;
	int		nReturn = 0;

	if (fnAPL_GetDefineDevice(DEV_CDU))
		nReturn = m_pMwi->CduGetPosition();

	if (nReturn != nSaveReturn)
		nSaveReturn = nReturn;

	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnCDU_GetRejectCSTStatus()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Get Reject CST Status
-------------------------------------------------------------------*/
int	CDevCmn::fnCDU_GetRejectCSTStatus()
{
	NHDEBUG(1, (L"[CDevCmn::fnCDU_GetRejectCSTStatus]\n"));

	static	int		nSaveReturn = 0;
	int		nReturn = 0;

	if (fnAPL_GetDefineDevice(DEV_CDU))
		nReturn = m_pMwi->CduGetRejectCSTStatus();

	if (nReturn != nSaveReturn)
		nSaveReturn = nReturn;

	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnCDU_GetRecycleBoxStatus()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Get Recycle Box Status
-------------------------------------------------------------------*/
int	CDevCmn::fnCDU_GetRecycleBoxStatus()
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnCDU_GetRecycleBoxStatus]\n"));

	static	int		nSaveReturn = FALSE;
	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_CDU))
		nReturn = m_pMwi->CduGetRecycleBoxStatus();

	if (nReturn != nSaveReturn)
	{
		nSaveReturn = nReturn;
	}
	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnCDU_GetMaterialInfo()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Get Material Information
-------------------------------------------------------------------*/
int	CDevCmn::fnCDU_GetMaterialInfo()
{
	//NHDEBUG(DBG_CALL, (L"[CDevCmn::fnCDU_GetMaterialInfo]\n"));

	static	int		nSaveReturn = FALSE;
	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_CDU))
		nReturn = m_pMwi->CduGetMaterialInfo();

	if (nReturn != nSaveReturn)
	{
		NHDEBUG(DBG_CALL, (L"[CDevCmn::fnCDU_GetMaterialInfo] Status Changed [%d] => [%d]\n", nSaveReturn, nReturn));
		nSaveReturn = nReturn;
	}
	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnCDU_GetAvailWithdraw()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Get Avail Withdraw
-------------------------------------------------------------------*/
int	CDevCmn::fnCDU_GetAvailWithdraw()
{
	//NHDEBUG(DBG_CALL, (L"[CDevCmn::fnCDU_GetAvailWithdraw]\n"));

	static	int		nSaveReturn = FALSE;
	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_CDU))
		nReturn = m_pMwi->CduGetAvailWithdraw();
	if (nReturn != nSaveReturn)
	{
		NHDEBUG(DBG_CALL, (L"[CDevCmn::fnCDU_GetAvailWithdraw] Status Changed [%d] => [%d]\n", nSaveReturn, nReturn));
		nSaveReturn = nReturn;
	}
	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnCDU_GetNumberOfCST()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Get Number Of CST
-------------------------------------------------------------------*/
int	CDevCmn::fnCDU_GetNumberOfCST()
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnCDU_GetNumberOfCST]\n"));

	static	int		nSaveReturn = FALSE;
	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_CDU))
		nReturn = m_pMwi->CduGetNumberOfCST();

	if (nReturn != nSaveReturn)
	{
		nSaveReturn = nReturn;
	}
	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnCDU_WaitTaken()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Get CST Status(CST No)
-------------------------------------------------------------------*/
int	CDevCmn::fnCDU_GetCSTStatus(int nCSTNo)
{
	//NHDEBUG(DBG_CALL, (L"[CDevCmn::fnCDU_GetCSTStatus]\n"));

	static	int		nSaveReturn = FALSE;
	static	int		nSaveReturn0 = FALSE;
	static	int		nSaveReturn1 = FALSE;
	static	int		nSaveReturn2 = FALSE;
	static	int		nSaveReturn3 = FALSE;
	static	int		nSaveReturn4 = FALSE;
	int				nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_CDU))
		nReturn = m_pMwi->CduGetCSTStatus(nCSTNo);

	switch (nCSTNo)
	{
		case CDU_CST_REJECT:
			if (nReturn != nSaveReturn0)
			{
				NHDEBUG(DBG_CALL, (L"[CDevCmn::fnCDU_GetCSTStatus] (REJECT) Status Changed [%d] => [%d]\n", nSaveReturn0, nReturn));
				nSaveReturn0 = nReturn;
			}
			break;
		case CDU_CST_1:
			if (nReturn != nSaveReturn1)
			{
				NHDEBUG(DBG_CALL, (L"[CDevCmn::fnCDU_GetCSTStatus] (CST_1) Status Changed [%d] => [%d]\n", nSaveReturn1, nReturn));
				nSaveReturn1 = nReturn;
			}
			break;
		case CDU_CST_2:
			if (nReturn != nSaveReturn2)
			{
				NHDEBUG(DBG_CALL, (L"[CDevCmn::fnCDU_GetCSTStatus] (CST_2) Status Changed [%d] => [%d]\n", nSaveReturn2, nReturn));
				nSaveReturn2 = nReturn;
			}
			break;
		case CDU_CST_3:
			if (nReturn != nSaveReturn3)
			{
				NHDEBUG(DBG_CALL, (L"[CDevCmn::fnCDU_GetCSTStatus] (CST_3) Status Changed [%d] => [%d]\n", nSaveReturn3, nReturn));
				nSaveReturn3 = nReturn;
			}
			break;
		case CDU_CST_4:
			if (nReturn != nSaveReturn4)
			{
				NHDEBUG(DBG_CALL, (L"[CDevCmn::fnCDU_GetCSTStatus] (CST_4) Status Changed [%d] => [%d]\n", nSaveReturn4, nReturn));
				nSaveReturn4 = nReturn;
			}
			break;
	}

	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnCDU_GetAllCSTStatus()
 RETURN TYPE  : CST_NORMAL / CST_NEAR / CST_SET_NG
 PARAMETER    : NONE
 DESCRIPTION  : Cassette의 상태를 Check한다.
-------------------------------------------------------------------*/
int CDevCmn::fnCDU_GetAllCSTStatus()
{
	//NHDEBUG(DBG_CALL, (L"[CDevCmn::fnCDU_GetAllCSTStatus]\n"));

	int i;

	if (CSTCnt > 4)		return	CST_SET_NG;

	int nCSTStatus[4];

	for(i=0; i<CSTCnt; i++)
	{
		nCSTStatus[i] = fnCDU_GetCSTStatus(CDU_CST_1 + i);
	}

	int	nRet = CST_SET_NG;

	for(i=0; i<CSTCnt; i++)
	{
		if( nCSTStatus[i] == CST_NORMAL )
		{
			nRet = CST_NORMAL;		break;
		}
		else if( nCSTStatus[i] == CST_NEAR )
		{
			if( nRet != CST_NORMAL )
			{
				if (MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_LOWCRCYCHECK) == 1)
					nRet = CST_NEAR;
				else
				{
					nRet = CST_NORMAL;
					break;
				}
			}
		}
		else if( nCSTStatus[i] == CST_EMPTY )
		{
			if( nRet != CST_NORMAL && nRet != CST_NEAR)		nRet = CST_EMPTY;
		}
	}

	if (CSTCnt == 0)	nRet = CST_NORMAL;
	
	CSTLoc = nRet;
	
	return nRet;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnCDU_GetValueOfCash()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Get Value (=denomination) Of Cash(CST No)
-------------------------------------------------------------------*/
int	CDevCmn::fnCDU_GetValueOfCash(int nCSTNo)
{
	static	int		nSaveReturn = FALSE;
	static	int		nSaveReturn0 = FALSE;
	static	int		nSaveReturn1 = FALSE;
	static	int		nSaveReturn2 = FALSE;
	static	int		nSaveReturn3 = FALSE;
	static	int		nSaveReturn4 = FALSE;
	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_CDU))
		nReturn = m_pMwi->CduGetValueOfCash(nCSTNo);

	switch (nCSTNo)
	{
		case CDU_CST_REJECT:
			if (nReturn != nSaveReturn0)
			{
				nSaveReturn0 = nReturn;
			}
			break;
		case CDU_CST_1:
			if (nReturn != nSaveReturn1)
			{
				nSaveReturn1 = nReturn;
			}
			break;
		case CDU_CST_2:
			if (nReturn != nSaveReturn2)
			{
				nSaveReturn2 = nReturn;
			}
			break;
		case CDU_CST_3:
			if (nReturn != nSaveReturn3)
			{
				nSaveReturn3 = nReturn;
			}
			break;
		case CDU_CST_4:
			if (nReturn != nSaveReturn4)
			{
				nSaveReturn4 = nReturn;
			}
			break;
	}

	NHDEBUG(DBG_CALL, (L"CST%d:%d\n", nCSTNo, nReturn));
	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnCDU_GetCurrencyID()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Get CurrencyID(CST No) 2007.11.23 V01.02.19
-------------------------------------------------------------------*/
CString	CDevCmn::fnCDU_GetCurrencyID(int nCSTNo)
{
	CString		strReturn = "";

	if (fnAPL_GetDefineDevice(DEV_CDU))
	{
		strReturn = m_pMwi->CduGetCurrencyID(nCSTNo);
	}

	NHDEBUG(DBG_CALL, (L"CST%d:%s\n", nCSTNo, strReturn));
	return strReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnCDU_SetCurrencyID()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Set CurrencyID [#514] [MX] KSK 2009.3.10
-------------------------------------------------------------------*/
int	CDevCmn::fnCDU_SetCurrencyID(LPCTSTR szCurrencyID)
{
	int nResult = DOWN;
	
	if (fnAPL_GetDefineDevice(DEV_CDU))
	{
		nResult = m_pMwi->CduSetCurrencyID(szCurrencyID);
		MemSetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST1_CURRENCYID, szCurrencyID);	// [#514] [MX] KSK 2009.3.11
	}
	
	NHDEBUG(DBG_CALL, (L"Set:%s Result:%d\n", szCurrencyID, nResult));
	return nResult;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnCDU_GetNumberOfCash()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Get Number Of Cash (=Bill Count) from cassette (CST No)
-------------------------------------------------------------------*/
int	CDevCmn::fnCDU_GetNumberOfCash(int nCSTNo)
{
	static	int		nSaveReturn = 0;
	static	int		nSaveReturn0 = 0;
	static	int		nSaveReturn1 = 0;
	static	int		nSaveReturn2 = 0;
	static	int		nSaveReturn3 = 0;
	static	int		nSaveReturn4 = 0;
	int		nReturn = 0;

	if (fnAPL_GetDefineDevice(DEV_CDU))
		nReturn = m_pMwi->CduGetNumberOfCash(nCSTNo);

	switch (nCSTNo)
	{
		case CDU_CST_REJECT:
			if (nReturn != nSaveReturn0)
			{
				nSaveReturn0 = nReturn;
			}
			break;
		case CDU_CST_1:
			if (nReturn != nSaveReturn1)
			{
				nSaveReturn1 = nReturn;
			}
			break;
		case CDU_CST_2:
			if (nReturn != nSaveReturn2)
			{
				nSaveReturn2 = nReturn;
			}
			break;
		case CDU_CST_3:
			if (nReturn != nSaveReturn3)
			{
				nSaveReturn3 = nReturn;
			}
			break;
		case CDU_CST_4:
			if (nReturn != nSaveReturn4)
			{
				nSaveReturn4 = nReturn;
			}
			break;
	}

	NHDEBUG(DBG_CALL, (L"CST%d: %d notes\n", nCSTNo, nReturn));

	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnCDU_GetNumberOfSetCash()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Get Number Of Set Cash(CST No)
-------------------------------------------------------------------*/
int	CDevCmn::fnCDU_GetNumberOfSetCash(int nCSTNo)
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnCDU_GetNumberOfSetCash]\n"));

	static	int		nSaveReturn = 0;
	static	int		nSaveReturn0 = 0;
	static	int		nSaveReturn1 = 0;
	static	int		nSaveReturn2 = 0;
	static	int		nSaveReturn3 = 0;
	static	int		nSaveReturn4 = 0;
	int		nReturn = 0;

	if (fnAPL_GetDefineDevice(DEV_CDU))
		nReturn = m_pMwi->CduGetNumberOfSetCash(nCSTNo);

	switch (nCSTNo)
	{
		case CDU_CST_REJECT:
		if (nReturn != nSaveReturn0)
		{
			nSaveReturn0 = nReturn;
		}
			break;
		case CDU_CST_1:
		if (nReturn != nSaveReturn1)
		{
			nSaveReturn1 = nReturn;
		}
			break;
		case CDU_CST_2:
		if (nReturn != nSaveReturn2)
		{
			nSaveReturn2 = nReturn;
		}
			break;
		case CDU_CST_3:
		if (nReturn != nSaveReturn3)
		{
			nSaveReturn3 = nReturn;
		}
			break;
		case CDU_CST_4:
		if (nReturn != nSaveReturn4)
		{
			nSaveReturn4 = nReturn;
		}
			break;
	}

	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fstrCDU_GetDispenseOfCST()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Get Dispense Of CST(Cash Amount)
-------------------------------------------------------------------*/
CString	CDevCmn::fstrCDU_GetDispenseOfCST(int nCashAmt)
{
	CString			strReturn("");

	if (fnAPL_GetDefineDevice(DEV_CDU))
		strReturn = m_pMwi->CduGetDispenseOfCST(nCashAmt);

	NHDEBUG(DBG_CALL, (L"Amt: %d, Ret: %s\n", nCashAmt, strReturn));

	return strReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnCDU_GetNumberOfReject()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Get Number Of Reject(CST No)
-------------------------------------------------------------------*/
int	CDevCmn::fnCDU_GetNumberOfReject(int nCSTNo)
{
	static	int		nSaveReturn = 0;
	static	int		nSaveReturn1 = 0;
	static	int		nSaveReturn2 = 0;
	static	int		nSaveReturn3 = 0;
	static	int		nSaveReturn4 = 0;
	CString strReturn("");
	CStringArray strReturnArray;
	int		nReturn = 0;

	if (fnAPL_GetDefineDevice(DEV_CDU))
	{
		if ((nCSTNo >= CDU_CST_1) && (nCSTNo <= CDU_CST_4))
		{
			if (nCSTNo == CDU_CST_1)
				nReturn = MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTCASSETTE1);
			else
			if (nCSTNo == CDU_CST_2)
				nReturn = MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTCASSETTE2);
			else
			if (nCSTNo == CDU_CST_3)
				nReturn = MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTCASSETTE3);
			else
			if (nCSTNo == CDU_CST_4)
				nReturn = MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_RJTCASSETTE4);

			if (nReturn < 0)
				nReturn = 0;
		}
	}

	switch (nCSTNo)
	{
		case CDU_CST_1:
			if (nReturn != nSaveReturn1)
			{
				nSaveReturn1 = nReturn;
			}
			break;
		case CDU_CST_2:
			if (nReturn != nSaveReturn2)
			{
				nSaveReturn2 = nReturn;
			}
			break;
		case CDU_CST_3:
			if (nReturn != nSaveReturn3)
			{
				nSaveReturn3 = nReturn;
			}
			break;
		case CDU_CST_4:
			if (nReturn != nSaveReturn4)
			{
				nSaveReturn4 = nReturn;
			}
			break;
	}

	NHDEBUG(DBG_CALL, (L"CST%d: %d\n", nCSTNo, nReturn));

	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnCDU_GetLastDispensedCount()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Get Last Dispensed Count of cassette
-------------------------------------------------------------------*/
int	CDevCmn::fnCDU_GetLastDispensedCount(int nCstNo)
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnCDU_GetLastDispensedCount]\n"));

	int		nReturn = 0;
	int		nNoofCst = fnCDU_GetNumberOfCST();

	// Customer가 실제 가지고 간 매수 구하기.
	if (nNoofCst >= nCstNo)
		nReturn = m_pMwi->CduGetLastDispensedCount(nCstNo);

	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnCDU_GetLastDispensedAmount()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Get Last Dispensed Amount
-------------------------------------------------------------------*/
int	CDevCmn::fnCDU_GetLastDispensedAmount()
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnCDU_GetLastDispensedAmount]\n"));

	int nReturn = 0;

	nReturn = m_pMwi->CduGetLastDispensedAmount();

	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnCDU_Initialize()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Initialize CDU Device
-------------------------------------------------------------------*/
int	CDevCmn::fnCDU_Initialize(long InitialType)
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnCDU_Initialize]\n"));

	int		nReturn = FALSE;

	DeviceOperationFlag = TRUE;
	if (fnAPL_GetDefineDevice(DEV_CDU))
	{
		if(AtmStatus == ATM_INIT)
		{
			//-- SJKWONNOTE 2006-09-09 --방출중 Power Off 대책//
			//Enq 송신 Flag가 해제되어 있지 않으면(SP가 Write) 방출중 Power Off로 본다.
			//SJK_34-- SJKWONNOTE 2006-10-18 -- Modify 비교문 == 1로 수정함//
			//C0055 ==> 97455로 수정
			if(MemGetInt(_MEM_FLD_SP_CDM,_MEM_VAR_CDMSP_FLAGENQSEND) == 1) 
			{
				NVDump('F', 'E', "0R", L"97455", L"ENQFLGSET");
		
				// [#135] KSK 2008.04.18
				fnAPL_StackError(_T("9745500"), fstrSCR_GetStringFromTextID(T_APMSG_003002), DEV_CDU);	// [#419] [NH] KSK 2008.9.16
				// end of [#135]

				return R_ERROR;
			}
		}

		nReturn = m_pMwi->CduInitialize(InitialType);	// [#507] [NH] KSK 2009.2.23 함수 Parameter 추가
	}

	// [#RWC6-82] NH Kook 2021.12.27 Support CDU Auto Recovery (Jam Clear)
	BOOL bCDUSupportAR = MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_DEVINFO_CDU_SUPPORTAR);
	NHDEBUG(DBG_CALL, (L"_MEM_VAR_DEVINFO_CDU_SUPPORTAR: [%d] \n", bCDUSupportAR));
	CString strTemp;
	strTemp.Format(L"CduSuppAR:%d" ,bCDUSupportAR);
	NVDump('O', 'E', "0A", L"", strTemp);

	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnCDU_Deinitialize()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Deinitialize CDU Device
-------------------------------------------------------------------*/
int	CDevCmn::fnCDU_Deinitialize()
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnCDU_Deinitialize]\n"));

	int		nReturn = FALSE;

	DeviceOperationFlag = TRUE;
	if (fnAPL_GetDefineDevice(DEV_CDU))
		nReturn = m_pMwi->CduDeinitialize();

	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnCDU_Deinitialize : [0x%x]]\n", nReturn));

	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnCDU_WaitTaken()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Wait Taken
-------------------------------------------------------------------*/
int	CDevCmn::fnCDU_WaitTaken()
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnCDU_WaitTaken]\n"));

	int		nReturn = FALSE;

	DeviceOperationFlag = TRUE;
	if (fnAPL_GetDefineDevice(DEV_CDU))
		nReturn = m_pMwi->CduWaitTaken();

	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnCDU_Dispense()
 RETURN TYPE  : 
 PARAMETER    : nCashAmt : Cash Amount
 DESCRIPTION  : Dispense Cash.
-------------------------------------------------------------------*/
int	CDevCmn::fnCDU_Dispense(int nCashAmt)
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnCDU_Dispense]\n"));

	int		nReturn = FALSE;

	DeviceOperationFlag = TRUE;
	// 2008-02-03 CduDispenseAmount함수는 Shutter가 있는 Type만 사용해야함
	// WINCE는 Shutter가 없는 Type이므로 CduDispenseAndPresentAmount를 사용해야함
	if (fnAPL_GetDefineDevice(DEV_CDU))
		nReturn = m_pMwi->CduDispenseAndPresentAmount(nCashAmt, MID_DEVRSP_TIME);

	return nReturn;
}
// [#RWC6-12, #2584] US Brandon 2019.02.04 Denomination Selection demo version for ATMIA
/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnCDU_DispenseCount()
 RETURN TYPE  : 
 PARAMETER    : nCST1Cnt : CST1 Count, nCST2Cnt : CST2 Count, nCST3Cnt : CST3 Count, nCST4Cnt : CST4 Count, 
 DESCRIPTION  : Dispense Cash. (for spitter type)
-------------------------------------------------------------------*/
int	CDevCmn::fnCDU_DispenseCount(int nCST1Cnt, int nCST2Cnt, int nCST3Cnt, int nCST4Cnt) 
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnCDU_DispenseAndPresentCount]\n"));

	int		nReturn = FALSE;

	DeviceOperationFlag = TRUE;
	if (fnAPL_GetDefineDevice(DEV_CDU))
		nReturn = m_pMwi->CduDispenseCount(nCST1Cnt, nCST2Cnt, nCST3Cnt, nCST4Cnt);

	return nReturn;
}
/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnCDU_DispenseAndPresentCount()
 RETURN TYPE  : 
 PARAMETER    : nCST1Cnt : CST1 Count, nCST2Cnt : CST2 Count, nCST3Cnt : CST3 Count, nCST4Cnt : CST4 Count, 
 DESCRIPTION  : Dispense and Present Cash (for presenter type)
-------------------------------------------------------------------*/
int	CDevCmn::fnCDU_DispenseAndPresentCount(int nCST1Cnt, int nCST2Cnt, int nCST3Cnt, int nCST4Cnt, int nWaitSec)
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnCDU_DispenseAndPresentCount]\n"));

	int		nReturn = FALSE;

	DeviceOperationFlag = TRUE;
	if (fnAPL_GetDefineDevice(DEV_CDU))
		nReturn = m_pMwi->CduDispenseAndPresentCount(nCST1Cnt, nCST2Cnt, nCST3Cnt, nCST4Cnt, nWaitSec); 

	return nReturn;
}
// end of [#RWC6-12, #2584]
/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnCDU_GetCashDispenseInfo()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 카세트별로 체크하여 출금 정보를 계산한다.
-------------------------------------------------------------------*/
int CDevCmn::fnCDU_GetCashDispenseInfo(int nDispLimit)				// [#2350] US Justin 2015.06.18 Add Overwrite Dispense Limit (POP Money)
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnCDU_GetCashDispenseInfo]\n"));

	/////////////////////////////////////////////////////////////////////////////
	//	Limit Amount Variable
	/////////////////////////////////////////////////////////////////////////////
	m_MoneyMaxOut	= 0;		// Money Max Out
	m_MoneyMinOut	= 0;		// Money Min Out	// KSK 2008.10.16 m_MoneyMinOut 초기화
	m_FastCaseNo	= -1;		// Fast Cash Selected Case (Done on ATM_CUSTOM mode)

	// Sensor Data Update
	fnAPL_SetSensorInfo();	// [#1] NH JSW 2008.03.03 속도 향상을 위해 IsDispensible에서 위치 이동

	// 1. Calculate Lowest Possible Dispense Denom.
	m_MoneyMinOut = fnCDU_CalcMoneyMinOut();

	// [#2350] US Justin Add Transaction Dispense Limit (POP Money use independent dispense limit)
	if( nDispLimit >= m_MoneyMinOut)
		m_nTranDispLimit = nDispLimit*100;
	else 
		m_nTranDispLimit = Asc2Int(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_DISPENSELIMIT));
	// End of [#2350]

	// 2. Calculate Maximum Possible Cash out
	m_MoneyMaxOut = fnCDU_CalcMoneyMaxOut();

	// 3. Fast Cash 계산
	m_FastCaseNo = fnCDU_CalculateFastCash();
	
	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnCDU_CalcMoneyMaxOut()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Calculate Money Max Out
-------------------------------------------------------------------*/
int CDevCmn::fnCDU_CalcMoneyMaxOut()
{
	NHDEBUG(DBG_CALL, (_T("[CDevCmn::fnCDU_CalcMoneyMaxOut]\n")));

	// 4. 남은 매수/카세트 상태로 체크하기
	m_MoneyMaxOut = fnCDU_GetMaxAmountOfAllCsts();

	// 5. 고객 SUPERVISOR MODE 세팅 값과 비교하여 크면 세팅함.
	// [#2350] US Justin 2015.06.18 Add Transactional Dispense Limit
	//int nOwnerMaxAmt = Asc2Int(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_DISPENSELIMIT)) / 100;	// KSK 2008.10.17 Default값 삭제
	int nOwnerMaxAmt = m_nTranDispLimit / 100;
	// End of [#2350]

	// 100 > Value
	if (nOwnerMaxAmt == 0)
	{
		// [#2350] US Justin 2015.06.18 Add Transactional Dispense Limit
		//nOwnerMaxAmt = Asc2Int(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_DISPENSELIMIT));	// KSK 2008.10.17 Default값 삭제
		nOwnerMaxAmt = m_nTranDispLimit;
	}

	if (m_MoneyMaxOut > nOwnerMaxAmt)
		m_MoneyMaxOut = nOwnerMaxAmt;

	return m_MoneyMaxOut;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnCDU_CalcMoneyMinOut()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Calculate Money Min Out
-------------------------------------------------------------------*/
int CDevCmn::fnCDU_CalcMoneyMinOut()
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnCDU_CalcMoneyMinOut]\n"));

	int nMinMoney = 0;
	int i;

#if (MX_VERSION)	// [#2115] MX KSK 2012.02.07
	if (m_strCurrencyID == CURRENCY_TYPE)
		nMinMoney = CDU_MCASH_MINOUT;
	else
		nMinMoney = CDU_MULTI_MCASH_MINOUT;
#else
	nMinMoney = CDU_MCASH_MINOUT;
#endif				// end of [#2115]

	// [#1] NH JSW 2008.03.26 WinXP와 동일하게 수정
	int nCstValue[4] = {};
	int nCstTempValue = 0;

	// 2. Get Cassette Values
	for (i = 0; i < CDU_MAX_CST_COUNT; i++)
		nCstValue[i] = fnCDU_GetValueOfCash(i+1);

	// 3. Sort in ascending order
	for (i = 0; i < CDU_MAX_CST_COUNT; i++)
	{
		for (int j = i+1; j < CDU_MAX_CST_COUNT; j++)
		{
			if (nCstValue[i] > nCstValue[j])
			{
				nCstTempValue = nCstValue[i];
				nCstValue[i] = nCstValue[j];
				nCstValue[j] = nCstTempValue;
			}
		}
	}

	// 4. 제일 작은 denomination부터 Dispense 가능한지 체크
	for (i = 0; i < CDU_MAX_CST_COUNT; i++)
	{
		if (nCstValue[i] == 0 ) continue;

		if (fbCDU_IsDispensible(nCstValue[i]))
		{
			nMinMoney = nCstValue[i];
			break;
		}
	}
	// end of [#1]

	return nMinMoney;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnCDU_CalculateFastCash()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Calculate FastCash Button Values
-------------------------------------------------------------------*/
int CDevCmn::fnCDU_CalculateFastCash()
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnCDU_CalculateFastCash]\n"));

	int		i;
	int		nCnt = 0;
	int		ScrNum = 0;
	CString strReturn;
	int		GetCase = 0;										// 2004.11.05
	CString	WithMsg;
	int		nDeno = 0;

	CString			strTemp;
	CStringArray	strArrAvailNote;
	int nTempMaxOut = m_MoneyMaxOut;

	if (!nTempMaxOut)		return -1;

	int nCashMoney = 0;
	bool bAvailable = false;
	memset(m_nFastCash, 0, sizeof(m_nFastCash));

	/////////////////////////////////////////////////////////////////////////////
	// 1.CURRENT FAST CASH AVAILABILITY CHECK
	for (i=0; i<CDU_CST_MAXFASTCASH; i++)
	{
		// [#2472] US Justin 2017.02.08 DCC Withdrawal Option
		// nCashMoney = MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_FASTCASH1 + i);	// [#268] [NH] KSK 2008.6.9
		if(m_nDCCLocalAvail == DCC_PRECHECK_DCCAVAIL)	nCashMoney = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DCC_OPTION_FASTCASH1 + i);
		else											nCashMoney = MemGetInt(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_FASTCASH1 + i);
		// End of [#2472]

		if ((nCashMoney > 0) && (nCashMoney <= nTempMaxOut))
		{
			if (fbCDU_IsDispensible(nCashMoney))
			{
				bool bfound = false;
				for (int j=0; j<i; j++)
				{
					if (nCashMoney == m_nFastCash[j])
						bfound = true;
				}
				// 같은 값이 이미 있다면 생략함.
				if (!bfound) m_nFastCash[i] = nCashMoney;

				bAvailable =  true;
			}
		}
	}

	// 2. FAST CASH 설정값이 있으면 임의로 정함.
	int nAvailCnt  = 0;
	int nLowestDen = 0;

#if (MX_VERSION)	// [#2115] MX KSK 2012.02.07
	if (m_strCurrencyID == CURRENCY_TYPE)
		nLowestDen = CDU_MCASH_MAXOUT;
	else
		nLowestDen = CDU_MULTI_MCASH_MAXOUT;
#else
	nLowestDen = CDU_MCASH_MAXOUT;	// 무의미의 Denomination 숫자 설정 // [#282] [NH] KSK 2008.6.11 // (1,2,5,10,20,50,100 중에 하나 설정 가능)
#endif				// end of [#2115]

	// Supervisor Menu에서 FAST CASH MENU Screen에 Display 가능하다면 FAST CASH로 간다.
	if (bAvailable)
	{
		GetCase = 0;
	}
	else
	{
		// 그렇지 않다면 임의로 만든다.(최소 Denom.을 찾은 후에)
		// 1.CURRENT FAST CASH AVAILABILITY CHECK
		for (i=0; i<CDU_MAX_CST_COUNT; i++)		// [#281] [NH] JSW 2008.6.10
		{
			// 최소 Denomination 값 지정
			if (((fnCDU_GetValueOfCash(i+1) < nLowestDen) &&
				(fnCDU_GetAmountOfCash(i+1) > 0)))
			{
				nAvailCnt++;
				nLowestDen = fnCDU_GetValueOfCash(i+1);
			}
		}

		// [#281] [NH] JSW 2008.6.10
		// [#2350] US Justin 2015.06.18 Add Transactional Dispense Limit
		//int nDispenseLimit = Asc2Int(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_DISPENSELIMIT))/100;
		int nDispenseLimit = m_nTranDispLimit / 100;
		// End of [#2350]

#if (MX_VERSION)	// [#2115] MX KSK 2012.02.07
		if (nDispenseLimit == 0)
		{
			if (m_strCurrencyID == CURRENCY_TYPE)
				nDispenseLimit = CDU_MCASH_MAXOUT;
			else
				nDispenseLimit = CDU_MULTI_MCASH_MAXOUT;
		}
#else
		if( nDispenseLimit == 0 )	nDispenseLimit = CDU_MCASH_MAXOUT;
#endif				// end of [#2115]

		if (nLowestDen > nDispenseLimit)
			return -1;	// Invalid Max Amount Setting 2006.02.26
		// end of [#281]

		// 2.Find Lowest Denomination Cassette
		//   찾은 Lowest Denom.에 1,2,3,... 값을 곱하여 Fast Cash 값을 만든다.
		int	nMultiDen_1[6]		= { 1,  5, 10, 15, 20, 25};
		int	nMultiDen_5[6]		= { 5, 10, 20, 30, 40, 50};
		memset(m_nFastCash, 0, sizeof(m_nFastCash));
		GetCase = 1;
		for (i=0; i<CDU_CST_MAXFASTCASH; i++)
		{
			// [#281] [NH] JSW 2008.6.10
			// 아래의 $1, $5는 미국 특이사항임
			if (nLowestDen == 1)		m_nFastCash[i] = nMultiDen_1[i];
			else if (nLowestDen == 5)	m_nFastCash[i] = nMultiDen_5[i];
			else						m_nFastCash[i] = nLowestDen * (i+1);
			// end of [#281]

			nCashMoney = m_nFastCash[i];

			if ((nCashMoney > 0) && (nCashMoney <= nTempMaxOut))
			{
				if (fbCDU_IsDispensible(nCashMoney))	GetCase = 0;
				else									m_nFastCash[i] = 0;
			}
			else	m_nFastCash[i] = 0;
		}
	}

	/////////////////////////////////////////////////////////////////////////////
	return GetCase;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnCDU_GetMaxAmountOfAllCsts()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Get Max Amount of All Cassette
-------------------------------------------------------------------*/
int CDevCmn::fnCDU_GetMaxAmountOfAllCsts()
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnCDU_GetMaxAmountOfAllCsts]\n"));

	int i;
	int nCstValue[5] = {0, 0, 0, 0, 0};
	int nCstCount[5] = {0, 0, 0, 0, 0};
	int nAvailCstCount  = 0;

#if (US_VERSION || CA_VERSION)					// [#2288] US Justin 2014.08.06 Apply 4000W Bill Max to Canadian Version
	CString strCDUEPVersion = MemGetVersion(_MEMKEY_EPVERSION, L"CDM");	// [#2098] US KSK 2011.11.28
#endif

	// 1. 유효 카세트 갯수 구하기
	if ((CSTCnt >= CDU_CST_1) &&
		(FirstCSTLoc == NORMAL_POS) &&
		((FirstCSTStatus == CST_NORMAL) || (FirstCSTStatus == CST_NEAR)))
	{
		nCstValue[1] = fnCDU_GetValueOfCash(CDU_CST_1);
		nCstCount[1] = fnCDU_GetNumberOfCash(CDU_CST_1);
		nAvailCstCount++;
	}

	if ((CSTCnt >= CDU_CST_2) &&
		(SecondCSTLoc == NORMAL_POS) &&
		((SecondCSTStatus == CST_NORMAL) || (SecondCSTStatus == CST_NEAR)))
	{
		nCstValue[2] = fnCDU_GetValueOfCash(CDU_CST_2);
		nCstCount[2] = fnCDU_GetNumberOfCash(CDU_CST_2);
		nAvailCstCount++;
	}

	if ((CSTCnt >= CDU_CST_3) &&
		(ThirdCSTLoc == NORMAL_POS) &&
		((ThirdCSTStatus == CST_NORMAL) || (ThirdCSTStatus == CST_NEAR)))
	{
		nCstValue[3] = fnCDU_GetValueOfCash(CDU_CST_3);
		nCstCount[3] = fnCDU_GetNumberOfCash(CDU_CST_3);
		nAvailCstCount++;
	}

	if ((CSTCnt >= CDU_CST_4) &&
		(FourthCSTLoc == NORMAL_POS) &&
		((FourthCSTStatus == CST_NORMAL) || (FourthCSTStatus == CST_NEAR)))
	{
		nCstValue[4] = fnCDU_GetValueOfCash(CDU_CST_4);
		nCstCount[4] = fnCDU_GetNumberOfCash(CDU_CST_4);
		nAvailCstCount++;
	}

	// 2. Available Cassette 가 없으면 최대값은 0으로 리턴
	if (!nAvailCstCount)	return 0;		

	// 3. Denomination 높은 순으로 정렬
	int nTmpCstValue = 0;
	int nTmpCstCount = 0;
	for( i=1; i<=4; i++)
	{
		for( int j=i+1; j<=4; j++ )
		{
			if( nCstValue[i] < nCstValue[j] )
			{
				nTmpCstValue = nCstValue[i];		nTmpCstCount = nCstCount[i];
				nCstValue[i] = nCstValue[j];		nCstCount[i] = nCstCount[j];
				nCstValue[j] = nTmpCstValue;		nCstCount[j] = nTmpCstCount;
			}
		}
	}

	// 4. 최대 출금 가능 금액 계산
	int nTotalAmount = 0;
	int nTotalCount = 0;
	int nReturnAmt = 0;

	// [#281] [NH] JSW 2008.6.10
	// [#2350] US Justin 2015.06.18 Add Transactional Dispense Limit
	//int nDispenseLimit = Asc2Int(MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_DISPENSELIMIT))/100;
	int nDispenseLimit = m_nTranDispLimit / 100;
	// End of [#2350]

#if (MX_VERSION)	// [#2115] MX KSK 2012.02.07
	if( nDispenseLimit == 0 )
	{
		if (m_strCurrencyID == CURRENCY_TYPE)
			nDispenseLimit = CDU_MCASH_MAXOUT;
		else
			nDispenseLimit = CDU_MULTI_MCASH_MAXOUT;
	}
#else
	if( nDispenseLimit == 0 )	nDispenseLimit = CDU_MCASH_MAXOUT;
#endif				// end of [#2115]

	for( i=1; i<=nAvailCstCount; i++)		
	{
		// [#2098] US KSK 2011.11.28
//		if( (nTotalCount + nCstCount[i]) > CST_MAXDISPENSE )	// 최대 40장 방출 가능
//			nCstCount[i] = CST_MAXDISPENSE - nTotalCount;
#if (US_VERSION || CA_VERSION)			// [#2288] US Justin 2014.08.06 Apply 4000W Bill Max to Canadian Version
		if (strCDUEPVersion.Left(3) == W_CDU_EP_VERSION)
		{
			if( (nTotalCount + nCstCount[i]) > CST_W_MAXDISPENSE )	// 최대 20장 방출 가능
				nCstCount[i] = CST_W_MAXDISPENSE - nTotalCount;
		}
		else
		{
			if( (nTotalCount + nCstCount[i]) > CST_MAXDISPENSE )	// 최대 40장 방출 가능
				nCstCount[i] = CST_MAXDISPENSE - nTotalCount;
		}
#else
		if( (nTotalCount + nCstCount[i]) > CST_MAXDISPENSE )	// 최대 40장 방출 가능
			nCstCount[i] = CST_MAXDISPENSE - nTotalCount;
#endif
		// end of [#2098]

		nTotalCount += nCstCount[i];
		nReturnAmt += nCstValue[i] * nCstCount[i];
		if( nReturnAmt > nDispenseLimit )						// 최대 $800 방출 가능
			nReturnAmt = nDispenseLimit;

		// [#2098] US KSK 2011.11.28
#if (US_VERSION || CA_VERSION)			// [#2288] US Justin 2014.08.06 Apply 4000W Bill Max to Canadian Version
		if (strCDUEPVersion.Left(3) == W_CDU_EP_VERSION)
		{
			if( nTotalCount >= CST_W_MAXDISPENSE || nReturnAmt >= nDispenseLimit )
				break;
		}
		else
		{
			if( nTotalCount >= CST_MAXDISPENSE || nReturnAmt >= nDispenseLimit )
				break;
		}
#else
		if( nTotalCount >= CST_MAXDISPENSE || nReturnAmt >= nDispenseLimit )
			break;
#endif
		// end of [#2098]
	}
	// end of [#281]

	return nReturnAmt;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fbCDU_IsDispensible()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
BOOL CDevCmn::fbCDU_IsDispensible(int nCashAmount)
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fbAPD_IsDispensible]\n"));

	int nCstDispCnt1, nCstDispCnt2, nCstDispCnt3, nCstDispCnt4;
	int nCstDispCntTotal = 0;
	CString strTemp;
	CStringArray strArrAvailNote;
	
//#if (US_VERSION)			// [#2472] US Justin 2017.02.09 Support 4000W
	CString strCDUEPVersion = MemGetVersion(_MEMKEY_EPVERSION, L"CDM");	// [#2098] US KSK 2011.11.28
//#endif

	nCstDispCnt1 = 0; nCstDispCnt2 = 0;
	nCstDispCnt3 = 0; nCstDispCnt4 = 0;

	strTemp = fstrCDU_GetDispenseOfCST(nCashAmount);

	strArrAvailNote.RemoveAll();
	SplitString(strTemp, ",", strArrAvailNote);
	if (strArrAvailNote.GetSize() >= 1)
		nCstDispCnt1 = Asc2Int(strArrAvailNote[0]);
	if (strArrAvailNote.GetSize() >= 2)
		nCstDispCnt2 = Asc2Int(strArrAvailNote[1]);
	if (strArrAvailNote.GetSize() >= 3)
		nCstDispCnt3 = Asc2Int(strArrAvailNote[2]);
	if (strArrAvailNote.GetSize() >= 4)
		nCstDispCnt4 = Asc2Int(strArrAvailNote[3]);

	nCstDispCntTotal = nCstDispCnt1+nCstDispCnt2+nCstDispCnt3+nCstDispCnt4;

	// [#2472] US Justin 2017.02.09 Support 4000W
/*
#if (US_VERSION)
	if (strCDUEPVersion.Left(3) == W_CDU_EP_VERSION)
	{
		if (nCstDispCntTotal > CST_W_MAXDISPENSE)
			return FALSE;
	}
	else
	{
		if (nCstDispCntTotal > CST_MAXDISPENSE)
			return FALSE;
	}
#else
	// 최대 방출 매수 초과 시,
	if (nCstDispCntTotal > CST_MAXDISPENSE)
		return FALSE;
#endif
*/

	if (strCDUEPVersion.Left(3) == W_CDU_EP_VERSION)
	{
		if (nCstDispCntTotal > CST_W_MAXDISPENSE)
			return FALSE;
	}
	else
	{
		if (nCstDispCntTotal > CST_MAXDISPENSE)
			return FALSE;
	}
	// End of [#2472]

	if ((nCstDispCnt1) || (nCstDispCnt2) || (nCstDispCnt3) || (nCstDispCnt4))
		return TRUE;
	
	return FALSE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnCDU_SumProc()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
int CDevCmn::fnCDU_SumProc(int InputID)
{
	/*	사용 Index 정리
	1. _MEM_FLD_INI_WITHTOTALINFO
		1) 출금
			0 : _MEM_VAR_SUM_WITH_COUNT
			0 : _MEM_VAR_SUM_WITH_AMOUNT

		2) REVERSAL
			5 : _MEM_VAR_SUM_WITH_COUNT
			5 : _MEM_VAR_SUM_WITH_AMOUNT

		3) SURCHARGE
			11 : _MEM_VAR_SUM_WITH_COUNT
			11 : _MEM_VAR_SUM_WITH_AMOUNT

	2. _MEM_FLD_INI_INQTOTALINFO
		1) 조회
			0 : _MEM_VAR_SUM_INQ_COUNT

	3. _MEM_FLD_INI_TRANSTOTALINFO
		1) 이체
			0 : _MEM_VAR_SUM_INQ_COUNT
			0 : _MEM_VAR_SUM_TRANS_AMOUNT
	*/

	// [#374] [NH] KSK 2008.7.18
	// SUM LOGIC 단순화
	int		wAmount = 0;	// [#484] [NH] KSK 2009.1.14
	char	szTemp[128] = {0,};
	CString strTemp;
	int		nRequestedAmount = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSREQAMOUNT));
	int		nDispensedAmount = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_TRANSDISPAMOUNT));

// [#2137] MX PCS 2012.07.17 환율 삭제 후 주석처리함.
// #if (MX_VERSION)	// [#2115] [MX] KSK 2012.02.07
// 	if (m_strCurrencyID == MULTI_CURRENCY_TYPE)
// 	{
// 		nRequestedAmount = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_CALC_REQ_MXDOLLAR));
// 		nDispensedAmount = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_CALC_DISP_MXDOLLAR));
// 	}
// #endif				// end of [#2115]

	switch (InputID) 
	{
		case SUM_OF_WITHDRAWAL:							// Withdrawal
			if (nDispensedAmount >= 0)					// [#412] [NH] KSK 2008.9.4
			{
				AddData(_MEM_FLD_INI_WITHTOTALINFO, _MEM_VAR_SUM_WITH_COUNT, 0, 1);
				///////////////////////////////////////////////////////////////////////////

#if (MX_VERSION)	// [#2115] MX KSK 2012.02.07
				// 멕시코는 환율로 계산되기 때문에 cent까지 포함해서 save해야한다.
				AddData(_MEM_FLD_INI_WITHTOTALINFO, _MEM_VAR_SUM_WITH_AMOUNT, 0, nDispensedAmount);
#else
				// cent를 제외한 값을 넣는다
				AddData(_MEM_FLD_INI_WITHTOTALINFO, _MEM_VAR_SUM_WITH_AMOUNT, 0, nDispensedAmount / 100); // [#484] [NH] KSK 2009.1.14
#endif				// end of [#2115]
				///////////////////////////////////////////////////////////////////////////
			}
			break;

		case SUM_OF_INQUIRY:							// Inquiry Host
			AddData(_MEM_FLD_INI_INQTOTALINFO, _MEM_VAR_SUM_INQ_COUNT, 0, 1);
			break;
		
		case SUM_OF_TRANSFER:							// Transfer Host
			{
				if (nRequestedAmount > 0)
				{
					AddData(_MEM_FLD_INI_TRANSTOTALINFO, _MEM_VAR_SUM_TRANS_COUNT, 0, 1);
					AddData(_MEM_FLD_INI_TRANSTOTALINFO, _MEM_VAR_SUM_TRANS_AMOUNT, 0, nRequestedAmount);
				}
			}
			break;
		
		case SUM_OF_REVERSAL:							// Reversal Withdraw
			wAmount = nRequestedAmount - nDispensedAmount;
			
			if (wAmount >= 0)	// 조회(호주) REVERSAL인 경우에도 COUNT를 증가하기 위해 0인경우에도 SUM함
			{
				AddData(_MEM_FLD_INI_WITHTOTALINFO, _MEM_VAR_SUM_WITH_COUNT, 5, 1);
				AddData(_MEM_FLD_INI_WITHTOTALINFO, _MEM_VAR_SUM_WITH_AMOUNT, 5, wAmount);
			}
			break;

		case SUM_OF_SURCHARGE:							// Surcharge Amount
#if (MX_VERSION)	// [#2147] MX KSK 2012.08.23 
			// Peso Mode는 그대로 유지함
			if (m_strCurrencyID == MULTI_CURRENCY_TYPE)
			{
				if (MemGetInt(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_CUSTOMER_TYPE) == MX_INTERNATIONAL_TYPE)
				{
					// International Card인 경우는 그대로 SUM
					// wAmount = Asc2Int(MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_LINE_USAGE_FEE));
					
					// [#2138] MX PCS 2012.08.28
					char	chTemp[1024] = { 0,};
					float	fTempAmount;
					CString strTemp;

					WideToMulti(chTemp, MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_LINE_USAGE_FEE), sizeof(chTemp));

					fTempAmount = (float)atof(chTemp);
					strTemp.Format(L"%.2f",fTempAmount);	// 소수점 3째 자리에서 반올림. 예를 들어 15.0123 ==> 15.01로 변환.
					wAmount = Asc2Int(strTemp);				
					if (wAmount > 0)
					{
						AddData(_MEM_FLD_INI_WITHTOTALINFO, _MEM_VAR_SUM_WITH_COUNT, 11, 1);
						AddData(_MEM_FLD_INI_WITHTOTALINFO, _MEM_VAR_SUM_WITH_AMOUNT, 11, wAmount);
					}
					// end of [#2138]
				}
				else
				{
					// Domestic Card인 경우는 Peso로 내려오므로 환율로 나눠서 SUM하도록 함
					char	chTemp[1024] = { 0,};
					CString strTemp;
					int nSurchargeAmt = Asc2Int(MemGetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEAMT));

					WideToMulti(chTemp, MemGetStr(_MEM_FLD_APP_ERRTRANS, _MEM_VAR_APP_EXCHANGE_RATE), sizeof(chTemp));

					float fExchangeRate = (float)atof(chTemp);
					float fSurchargeAmt = (float)(nSurchargeAmt * 0.01);
					float CalcSurchargeAmt = fSurchargeAmt / fExchangeRate;

					strTemp.Format(L"%.2f", CalcSurchargeAmt);	// 소수 2자리에서 반올림
					wAmount = Asc2Int(strTemp);

					if (wAmount > 0)
					{
						AddData(_MEM_FLD_INI_WITHTOTALINFO, _MEM_VAR_SUM_WITH_COUNT, 11, 1);
						AddData(_MEM_FLD_INI_WITHTOTALINFO, _MEM_VAR_SUM_WITH_AMOUNT, 11, wAmount);
					}
				}
			}
			else
			{
				wAmount = Asc2Int(MemGetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEAMT));	// [#484] [NH] KSK 2009.1.14
				if (wAmount > 0)
				{
					AddData(_MEM_FLD_INI_WITHTOTALINFO, _MEM_VAR_SUM_WITH_COUNT, 11, 1);
					AddData(_MEM_FLD_INI_WITHTOTALINFO, _MEM_VAR_SUM_WITH_AMOUNT, 11, wAmount);
				}
			}
#else
			wAmount = Asc2Int(MemGetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEAMT));	// [#484] [NH] KSK 2009.1.14
			if (wAmount > 0)
			{
				AddData(_MEM_FLD_INI_WITHTOTALINFO, _MEM_VAR_SUM_WITH_COUNT, 11, 1);
				AddData(_MEM_FLD_INI_WITHTOTALINFO, _MEM_VAR_SUM_WITH_AMOUNT, 11, wAmount);
			}
#endif				// end of [#2147]
			break;

		case SUM_OF_NON_CASH_WITHDRAWAL:				// Non-Cash Withdrawal	// 미국만 NON_CASH를 사용함
			if (nDispensedAmount >= 0)					// [#412] [NH] KSK 2008.9.4
			{
				AddData(_MEM_FLD_INI_WITHTOTALINFO, _MEM_VAR_SUM_WITH_COUNT, 12, 1);
				AddData(_MEM_FLD_INI_WITHTOTALINFO, _MEM_VAR_SUM_WITH_AMOUNT, 12, nDispensedAmount / 100);	// [#484] [NH] KSK 2009.1.14
			}
			break;

		// [#2185] US Justin 2013.05.14
		case SUM_OF_DHDCC:
			if (nDispensedAmount >= 0)
			{
				AddData(_MEM_FLD_INI_DHDCCINFO, _MEM_VAR_SUM_DHDCC_COUNT, 0, 1);
				AddData(_MEM_FLD_INI_DHDCCINFO, _MEM_VAR_SUM_DHDCC_AMOUNT, 0, nDispensedAmount / 100);

			}
			break;
		
		case SUM_OF_DHDCC_SURCHARGE:
			wAmount = Asc2Int(MemGetStr(_MEM_FLD_INI_TRANSINFO, _MEM_VAR_APP_SURCHARGEAMT));
			if (wAmount > 0)
			{
				AddData(_MEM_FLD_INI_DHDCCINFO, _MEM_VAR_SUM_DHDCC_COUNT, 1, 1);
				AddData(_MEM_FLD_INI_DHDCCINFO, _MEM_VAR_SUM_DHDCC_AMOUNT, 1, wAmount);
			}
			break;

		case SUM_OF_DHDCC_REVERSAL:
			wAmount = nRequestedAmount - nDispensedAmount;			
			if (wAmount >= 0)
			{
				AddData(_MEM_FLD_INI_DHDCCINFO, _MEM_VAR_SUM_DHDCC_COUNT, 2, 1);
			}
			break;
		// end of [#2185]

		default:
			break;
	}
	
	return T_OK;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnAPP_SumClear(int n3rdTranType)
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
int CDevCmn::fnCDU_SumClear(int n3rdTranType)
{
	if( n3rdTranType == 0 )
		MemClearTotal();
	else
		MemClearDualHostDCCTotal();	// [#2185] US Justin 2013.05.14 Dual Host DCC

	return T_OK;
}

// [#2504] NH Justin 2017.09.13
/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnCDU_TestDispense()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
int	CDevCmn::fnCDU_TestDispense()
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnCDU_TestDispense()]\n"));

	int		nReturn = FALSE;
	DeviceOperationFlag = TRUE;

	if (fnAPL_GetDefineDevice(DEV_CDU))
	{	
		// Dispense Note Count
		int nCST1Cnt, nCST2Cnt, nCST3Cnt, nCST4Cnt;
		nCST1Cnt = nCST2Cnt = nCST3Cnt = nCST4Cnt = 0;
		if( CSTCnt >= CDU_CST_1 )	 nCST1Cnt = 1;
		if( CSTCnt >= CDU_CST_2 )	 nCST2Cnt = 1;
		if( CSTCnt >= CDU_CST_3 )	 nCST3Cnt = 1;
		if( CSTCnt >= CDU_CST_4 )	 nCST4Cnt = 1;

		// Set Test Dispense Mode
		RegSetStr(L"SOFTWARE\\ATM\\DevInfo\\CDM", L"TestDispense", L"T");

		nReturn = m_pMwi->CduDispenseAndPresentCount(nCST1Cnt, nCST2Cnt, nCST3Cnt, nCST4Cnt, MID_DEVRSP_TIME);		
	}

	return nReturn;
}
// End of [#2504]
