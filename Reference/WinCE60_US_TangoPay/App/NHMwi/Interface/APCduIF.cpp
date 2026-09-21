// APCDUIF.cpp

#include "stdafx.h"

//#define NH_DEBUG
#include "..\NHMWI.h"
#include "..\NHMWICtl.h"
#include "..\NHMWIPpg.h"

#include ".\Common\NHDbgApi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


///////////////////////////////////////////////////////////////////////////////////////////////////
// Method : Called From AP
///////////////////////////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
long CNHMWICtrl::CduInitialize(long InitialType) 
{
NHTRACE((_T("CNHMWICtrl::CduInitialize()(%s) \n"), "  "));

	DevCduPreCancelWaitTaken();									

	int nResult = 0;

	if (m_bCduOpened)
	{
		m_pDevCtrl->EvtQReset(DEV_CDU);

		// [#507] [NH] KSK 2009.2.23 POS 사양을 지원하기 위해 Initial Flag를 AP에서 Control하도록 수정
		if (InitialType == 0)	// OpenSessionSync를 함
		{
			m_bCduOpened = TRUE;
			DevCduCloseConnection(); //CloseConnection은 무조건 R_NORMAL return임.
			
			SetEventMatrix(DEV_CDU, "OpenSessionSync");
			nResult = DevCduOpenConnection();
			
			if (nResult != R_NORMAL)		m_bCduOpened = FALSE;			
		}
		else	// Reset을 함
		{
			SetEventMatrix(DEV_CDU, "Reset");
			m_pCdu.Reset(0);
			nResult = R_NORMAL;
		}
		// end of [#507]
	}
	else		// Open Fail시에는 항상 OpenSessionSync를 하도록 한다.
	{
		// [#136] KSK 2008.04.18  ATM기동시 CDU 단선발생 후 OP에서 복구 안되는 문제 수정
		m_bCduOpened = TRUE;
		DevCduCloseConnection(); //CloseConnection은 무조건 R_NORMAL return임.

		SetEventMatrix(DEV_CDU, "OpenSessionSync");
		nResult = DevCduOpenConnection();

		if (nResult != R_NORMAL)		m_bCduOpened = FALSE;
		// end of [#136]
	}
NHTRACE((_T("CNHMWICtrl::CduInitialize()(%s) return \n"), "  "));

	return nResult;
}

// ----------------------------------------------------------------------------
long CNHMWICtrl::CduDeinitialize() 
{
NHTRACE((_T("CNHMWICtrl::CduDeinitialize()(%s) \n"), "  "));
	
	DevCduPreCancelWaitTaken();									

//	SetEventMatrix(DEV_CDU, "CloseConnection");
	
	int nResult = 0;
	if (m_bCduOpened)
	{
		m_bCduOpened = FALSE;										
		nResult = DevCduCloseConnection();						// 20030703_10_1047 : 함수호출수정
	}
	else
	{
		m_bCduOpened = FALSE;										
//		ProcSetDeviceEvent(DEVNM_CDU, "ConnectionClosed", "");	// 20030703_1.0_1048 : 이벤트 생성처리
	}

	return nResult;
}


// ----------------------------------------------------------------------------
long CNHMWICtrl::CduOpenShutter() 
{
NHTRACE((_T("CNHMWICtrl::CduOpenShutter()(%s) \n"), "  "));

	DevCduPreCancelWaitTaken();									

	SetEventMatrix(DEV_CDU, "OpenShutter");
	return DevCduOpenShutter();
}

// ----------------------------------------------------------------------------
long CNHMWICtrl::CduCloseShutter() 
{
NHTRACE((_T("CNHMWICtrl::CduCloseShutter()(%s) \n"), "  "));
	
	DevCduPreCancelWaitTaken();									

	SetEventMatrix(DEV_CDU, "CloseShutter");
	return DevCduCloseShutter();
}

// ----------------------------------------------------------------------------
long CNHMWICtrl::CduWaitTaken()
{
NHTRACE((_T("CNHMWICtrl::CduWaitTaken()(%s) \n"), "  "));

	SetEventMatrix(DEV_CDU, "WaitTaken");
	return R_NORMAL;
}

// ----------------------------------------------------------------------------
long CNHMWICtrl::CduCancelWaitTaken() 
{
NHTRACE((_T("CNHMWICtrl::CduCancelWaitTaken()(%s) \n"), "  "));

//	SetEventMatrix(DEV_CDU, "CancelWaitForCashTaken");
	return DevCduCancelWaitTaken();
}

// ----------------------------------------------------------------------------
long CNHMWICtrl::CduRetract() 
{
NHTRACE((_T("CNHMWICtrl::CduRetract()(%s) \n"), "  "));

	DevCduPreCancelWaitTaken();
	SetEventMatrix(DEV_CDU, "Retract");
	return DevCduRetract();
}

// ----------------------------------------------------------------------------
long CNHMWICtrl::CduDispenseCount(long nCST1Cnt, long nCST2Cnt, long nCST3Cnt, long nCST4Cnt) 
{
NHTRACE((_T("CNHMWICtrl::CduDispenseCount(%d, %d, %d, %d) \n"), nCST1Cnt, nCST2Cnt, nCST3Cnt, nCST4Cnt));
	m_bDispenseSuccecded = FALSE; //2008-01-14 V01.02.25 SRC-8

	DevCduPreCancelWaitTaken();

	SetEventMatrix(DEV_CDU, "Dispense");
	int nResult = DevCduDispenseCount(nCST1Cnt, nCST2Cnt, nCST3Cnt, nCST4Cnt);
	if(nResult == R_NORMAL) //2008-01-14 V01.02.25 SRC-8
		m_bDispenseSuccecded = TRUE;
		
NHTRACE((_T("CNHMWICtrl::CduDispenseCount() Result(%d) \n"), nResult));
	return nResult;
}

// ----------------------------------------------------------------------------
// Mix결과조회
BSTR CNHMWICtrl::CduGetDispenseOfCST(long nAmount) 
{
NHTRACE((_T("CNHMWICtrl::CduGetDispenseOfCST(%d) \n"), nAmount));

	CString strResult("");
	CArray<int, int> IArray;
	IArray.RemoveAll();
	int nCSTSize	= 0;
	int nResult		= 0;

	DevCduPreCancelWaitTaken();

	// PJH 2006.01.05
	m_GetAvailableAmount = TRUE;

	SetEventMatrix(DEV_CDU, "Denominate");
	nResult = DevCduDispenseAmount(nAmount);
	if (nResult == NORMAL)
	{
		nResult = m_pDevCtrl->CheckDeviceAction(DEV_CDU, K_30_WAIT);
		if (nResult == NORMAL)
		{
			nCSTSize = VarArrayToIntArray(&m_vMixResult, IArray);

			for (int i = 1;i < 5;i++)
			{
				if (i != 1)	strResult += ",";					// Padding ","

				if (i < nCSTSize)								// CST Exist : Get Mix Result
					strResult += Int2Asc(IArray.GetAt(i));
				else											// CST Not-Exist : Padding "0"
					strResult += "0";
			}
		}
	}

	VariantClear(&m_vMixResult);	//-- SJKWONNOTE 2006-06-19 -- Add : Memory leak 수정//
//	VariantInit(&m_vMixResult);		// [#363] NH JSW 2008.07.15 불필요
	
NHTRACE((_T("CNHMWICtrl::CduGetDispenseOfCST strResult(%s) \n"), strResult));
	
	return strResult.AllocSysString();
}

// ----------------------------------------------------------------------------
long CNHMWICtrl::CduDispenseAmount(long nAmount) 
{
NHTRACE((_T("CNHMWICtrl::CduDispenseAmount nAmount(%d) \n"), nAmount));
	m_bDispenseSuccecded = FALSE; //2008-01-14 V01.02.25 SRC-8

	DevCduPreCancelWaitTaken();									

	// PJH 2006.01.05
	m_GetAvailableAmount = FALSE;

	SetEventMatrix(DEV_CDU, "Denominate");
	int nResult = DevCduDispenseAmount(nAmount);				// Mix
	if (nResult == NORMAL)
	{
		nResult = m_pDevCtrl->CheckDeviceAction(DEV_CDU, K_30_WAIT);
		if (nResult == NORMAL)
		{
			SetEventMatrix(DEV_CDU, "Dispense");
			nResult = DevCduDispense(m_vMixResult);
			if(nResult == R_NORMAL) //2008-01-14 V01.02.25 SRC-8
				m_bDispenseSuccecded = TRUE;
		}
	}
		
NHTRACE((_T("CNHMWICtrl::CduDispenseAmount nResult(%d) \n"), nResult));

	VariantClear(&m_vMixResult);	//-- SJKWONNOTE 2006-06-19 -- Add : Memory leak 수정//
//	VariantInit(&m_vMixResult);		// [#363] NH JSW 2008.07.15 불필요
	return nResult;
}

// ----------------------------------------------------------------------------
long CNHMWICtrl::CduDispenseAndPresentAmount(long nAmount, long nWaitSec) 
{
NHTRACE((_T("CNHMWICtrl::CduDispenseAndPresentAmount nAmount(%d) \n"), nAmount));
	m_bDispenseSuccecded = FALSE; //2008-01-14 V01.02.25 SRC-8

	DevCduPreCancelWaitTaken();									

	// PJH 2006.01.05
	m_GetAvailableAmount = FALSE;

	SetEventMatrix(DEV_CDU, "Denominate");
	int nResult = DevCduDispenseAmount(nAmount);				// Mix
	if (nResult == NORMAL)
	{
		nResult = m_pDevCtrl->CheckDeviceAction(DEV_CDU, K_30_WAIT);
		if (nResult == NORMAL)
		{
			SetEventMatrix(DEV_CDU, "Dispense");
			nResult = DevCduDispenseAndPresentAmount(m_vMixResult, nWaitSec);
			if(nResult == R_NORMAL) //2008-01-14 V01.02.25 SRC-8
				m_bDispenseSuccecded = TRUE;
		}
	}
		
NHTRACE((_T("CNHMWICtrl::CduDispenseAndPresentAmount nResult(%d) \n"), nResult));
	
	VariantClear(&m_vMixResult);	//-- SJKWONNOTE 2006-06-19 -- Add : Memory leak 수정//
//	VariantInit(&m_vMixResult);		// [#363] NH JSW 2008.07.15 불필요
	return nResult;
}

long CNHMWICtrl::CduDispenseAndPresentCount(long nCST1Cnt, long nCST2Cnt, long nCST3Cnt, long nCST4Cnt, long nWaitSec) 
{
NHTRACE((_T("CNHMWICtrl::CduDispenseAndPresentCount(%d, %d, %d, %d, %d) \n"), nCST1Cnt, nCST2Cnt, nCST3Cnt, nCST4Cnt, nWaitSec));
	m_bDispenseSuccecded = FALSE; //2008-01-14 V01.02.25 SRC-8
	DevCduPreCancelWaitTaken();									

	SetEventMatrix(DEV_CDU, "Dispense");

	int nResult = DevCduDispenseAndPresentCount(nCST1Cnt, nCST2Cnt, nCST3Cnt, nCST4Cnt, nWaitSec);
	if(nResult == FALSE) //2008-01-14 V01.02.25 SRC-8
		m_bDispenseSuccecded = TRUE;

NHTRACE((_T("CNHMWICtrl::CduDispenseAndPresentCount  nResult(%d) \n"), nResult));
	return nResult;
}

// ----------------------------------------------------------------------------
long CNHMWICtrl::CduPresent(long nWaitSec) 
{
NHTRACE((_T("CNHMWICtrl::CduPresent(%s) \n"), "  "));

	DevCduPreCancelWaitTaken();									
	SetEventMatrix(DEV_CDU, "Present");
	int nResult = DevCduPresent(nWaitSec);

NHTRACE((_T("CNHMWICtrl::CduPresent nResult(%d) \n"), nResult));
	return nResult;
}

// ----------------------------------------------------------------------------
BSTR CNHMWICtrl::CduGetErrorCode() 
{
NHTRACE((_T("CNHMWICtrl::CduGetErrorCode(%s) \n"), "  "));

	CString strResult("");
	strResult = DevCduGetErrorCode();

NHTRACE((_T("CNHMWICtrl::CduGetErrorCode strResult(%s) \n"), strResult));
	return strResult.AllocSysString();
}

// ----------------------------------------------------------------------------
BSTR CNHMWICtrl::CduGetSensorInfo() 
{
NHTRACE((_T("CNHMWICtrl::CduGetSensorInfo(%s) \n"), "  "));

	CString strResult("");
	strResult = DevCduGetSensorInfo();							// 20030719_1.0_1053 : GetSensorInfo

NHTRACE((_T("CNHMWICtrl::CduGetSensorInfo strResult(%s) \n"), strResult));
	return strResult.AllocSysString();
}

// ----------------------------------------------------------------------------
long CNHMWICtrl::CduGetDeviceStatus() 
{
	int nResult = DevCduGetDeviceStatus();

	return nResult;
}

// ----------------------------------------------------------------------------
long CNHMWICtrl::CduGetPosition() 
{
NHTRACE((_T("CNHMWICtrl::CduGetPosition(%s) \n"), "  "));

	int nResult = DevCduGetPosition();

NHTRACE((_T("CNHMWICtrl::CduGetPosition nResult(%d) \n"), nResult));
	return nResult;
}

// ----------------------------------------------------------------------------
long CNHMWICtrl::CduGetRejectCSTStatus() 
{
NHTRACE((_T("CNHMWICtrl::CduGetRejectCSTStatus(%s) \n"), "  "));

	int nResult = DevCduGetRejectCSTStatus();

NHTRACE((_T("CNHMWICtrl::CduGetRejectCSTStatus nResult(%d) \n"), nResult));
	return nResult;
}

// ----------------------------------------------------------------------------
long CNHMWICtrl::CduGetRecycleBoxStatus() 
{
NHTRACE((_T("CNHMWICtrl::CduGetRecycleBoxStatus(%s) \n"), "  "));

	int nResult = DevCduGetRecycleBoxStatus();

NHTRACE((_T("CNHMWICtrl::CduGetRecycleBoxStatus nResult(%d) \n"), nResult));
	return nResult;
}

// ----------------------------------------------------------------------------
long CNHMWICtrl::CduGetCSTStatus(long nType) 
{
NHTRACE((_T("CNHMWICtrl::CduGetCSTStatus(%s) \n"), "  "));

	int nResult = CST_NORMAL;
	CString szCSTStatus = DevCduGetCSTStatus(nType);

	if		(szCSTStatus.CompareNoCase(L"CST_NORMAL")	== 0)	nResult = CST_NORMAL;
	else if (szCSTStatus.CompareNoCase(L"CST_NEAR")		== 0)	nResult = CST_NEAR;
	else if	(szCSTStatus.CompareNoCase(L"CST_EMPTY")	== 0)	nResult = CST_EMPTY;
	else if (szCSTStatus.CompareNoCase(L"CST_SET_NG")	== 0)	nResult = CST_SET_NG;
	else														nResult = CST_EMPTY;

NHTRACE((_T("CNHMWICtrl::CduGetCSTStatus nResult(%d) \n"), nResult));
	return nResult;
}

// ----------------------------------------------------------------------------
long CNHMWICtrl::CduGetNumberOfCST() 
{
NHTRACE((_T("CNHMWICtrl::CduGetNumberOfCST(%s) \n"), "  "));

	int nCSTCount = DevCduGetCSTCount();

NHTRACE((_T("CNHMWICtrl::CduGetNumberOfCST nCSTCount(%d) \n"), nCSTCount));
	return nCSTCount;
}

// ----------------------------------------------------------------------------
long CNHMWICtrl::CduGetValueOfCash(long nCSTNo) 
{
NHTRACE((_T("CNHMWICtrl::CduGetValueOfCash(%s) \n"), "  "));

	int nResult = DevCduGetValueOfCash(nCSTNo);

NHTRACE((_T("CNHMWICtrl::CduGetNumberOfCash(%d) return [%d] \n"), nCSTNo, nResult));
	return nResult;
}

// ----------------------------------------------------------------------------
long CNHMWICtrl::CduGetNumberOfCash(long nCSTNo) 
{
NHTRACE((_T("CNHMWICtrl::CduGetNumberOfCash(%s) \n"), "  "));

	int nResult = DevCduGetNumberOfCash(1, nCSTNo);

NHTRACE((_T("CNHMWICtrl::CduGetNumberOfCash(%d) return [%d] \n"), nCSTNo, nResult));
	return nResult;
}

// ----------------------------------------------------------------------------
long CNHMWICtrl::CduGetNumberOfSetCash(long nCSTNo)
{
NHTRACE((_T("CNHMWICtrl::CduGetNumberOfSetCash(%s) \n"), "  "));

	int nResult = DevCduGetNumberOfCash(0, nCSTNo);

NHTRACE((_T("CNHMWICtrl::CduGetNumberOfSetCash(%d) return [%d] \n"), nCSTNo, nResult));
	return nResult;
}

// ----------------------------------------------------------------------------
long CNHMWICtrl::CduGetMaterialInfo() 
{
	int nResult = DevCduGetMaterialInfo();

	return nResult;
}

// ----------------------------------------------------------------------------
long CNHMWICtrl::CduGetAvailWithdraw() 
{
	int nResult = DevCduAvailWithdraw();
	
	return nResult;
}

// ----------------------------------------------------------------------------
long CNHMWICtrl::CduClearErrorCode() 
{
NHTRACE((_T("CNHMWICtrl::CduClearErrorCode(%s) \n"), "  "));

	return DevCduClearErrorCode();
}

// ----------------------------------------------------------------------------
long CNHMWICtrl::CduSetNumberOfCash(LPCTSTR szCashCntData) 
{
NHTRACE((_T("CNHMWICtrl::CduSetNumberOfCash szCashCntData(%s) \n"), szCashCntData));

	DevCduPreCancelWaitTaken();									

	int nResult = DOWN;
	if (DevCduInitiateChange() == R_NORMAL)
	{
		nResult = DevCduSetNumberOfCash(szCashCntData);
	}

	SysFreeString((BSTR)szCashCntData);

NHTRACE((_T("CNHMWICtrl::CduSetNumberOfCash() nResult [%d] \n"), nResult));
	return nResult;
}

long CNHMWICtrl::CduAddNumberOfCash(LPCTSTR szInitialCnt, LPCTSTR szAddCnt) 
{
NHTRACE((_T("CNHMWICtrl::CduAddNumberOfCash  szInitialCnt(%s) szAddCnt(%s) \n"), szInitialCnt, szAddCnt));

	DevCduPreCancelWaitTaken();									

	int nResult = DOWN;
	if (DevCduInitiateChange() == R_NORMAL)
		nResult = DevCduAddNumberOfCash(szInitialCnt, szAddCnt);

	SysFreeString((BSTR)szInitialCnt);
	SysFreeString((BSTR)szAddCnt);

NHTRACE((_T("CNHMWICtrl::CduAddNumberOfCash() nResult [%d] \n"), nResult));
	return nResult;
}

// ----------------------------------------------------------------------------
long CNHMWICtrl::CduSetMinMaxCST(LPCTSTR szMinCntData, LPCTSTR szMaxCntData) 
{
NHTRACE((_T("CNHMWICtrl::CduSetMinMaxCST  szMinCntData(%s) szMaxCntData(%s) \n"), szMinCntData, szMaxCntData));

	DevCduPreCancelWaitTaken();									

	int nResult = DOWN;
	if (DevCduInitiateChange() == R_NORMAL)
	{
		nResult = DevCduSetMinMaxThreshold(szMinCntData, szMaxCntData);
	}

	SysFreeString((BSTR)szMinCntData);
	SysFreeString((BSTR)szMaxCntData);

NHTRACE((_T("CNHMWICtrl::CduSetMinMaxCST() nResult [%d] \n"), nResult));
	return nResult;
}

long CNHMWICtrl::CduSetValueOfCash(LPCTSTR szCashValueData) 
{
NHTRACE((_T("CNHMWICtrl::CduSetValueOfCash  szCashValueData(%s) \n"), szCashValueData));

	DevCduPreCancelWaitTaken();									

	int nResult = DOWN;
	if (DevCduInitiateChange() == NORMAL)
	{
		nResult = DevCduSetValueOfCash(szCashValueData);
	}

	SysFreeString((BSTR)szCashValueData);

NHTRACE((_T("CNHMWICtrl::CduSetValueOfCash() nResult [%d] \n"), nResult));
	return nResult;
}

long CNHMWICtrl::CduGetLastDispensedAmount() 
{
NHTRACE((_T("CNHMWICtrl::CduGetLastDispensedAmount(%s)  \n"), "  "));
	if(m_bDispenseSuccecded) //2008-01-14 V01.02.25 SRC-8
		return DevCduGetLastDispensedAmount();

	return 0;
}

long CNHMWICtrl::CduGetLastDispensedCount(long nCstNum)
{
NHTRACE((_T("CNHMWICtrl::CduGetLastDispensedCount(%s)  \n"), "  "));
	if(m_bDispenseSuccecded) //2008-01-14 V01.02.25 SRC-8
		return DevCduGetLastDispensedCount(nCstNum);

	return 0;
}

BOOL CNHMWICtrl::CduGetShutterStatus() 
{
NHTRACE((_T("CNHMWICtrl::CduGetShutterStatus(%s)  \n"), "  "));

	return DevCduGetShutterStatus();
}
// 2007.11.23 V01.02.19
// ----------------------------------------------------------------------------
BSTR CNHMWICtrl::CduGetCurrencyID(long nCSTNo)
{
	CString strResult = DevCduGetCurrencyID();
	return strResult.AllocSysString();
}

// [#514] [MX] KSK 2009.3.10
long CNHMWICtrl::CduSetCurrencyID(LPCTSTR CurrencyID)
{
NHTRACE((_T("CNHMWICtrl::CduSetCurrencyID  CurrencyID(%s) \n"), CurrencyID));
	
	m_strCurrency = CurrencyID;			// 실제 SP에 CurrencyID를 Setting하는게 아니라 MWI에서 가지고 있는 정보만 update한다.
	
	SysFreeString((BSTR)CurrencyID);

	return R_NORMAL;
}

