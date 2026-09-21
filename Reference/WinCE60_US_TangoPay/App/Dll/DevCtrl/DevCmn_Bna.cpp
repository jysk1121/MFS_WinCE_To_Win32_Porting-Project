/////////////////////////////////////////////////////////////////////////////
//	DevBna.cpp : implementation file
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include ".\Dev\DevCmn.h"

//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------

//#define NH_DEBUG		// Debug Message On/Off

#include ".\Common\NHDbgApi.h"
#include "..\..\NHMwi\Axnext\nxcashacceptorx.h"			// [#GLDV-3005] US Kook 2022.01.05 to use ILogicalUnit class

#define	DBG_CALL		1
#define DBG_INFO		1

/*
 VARIANT FUNCTIONS
*/
VARIANT IntArrayToSafeArray(CArray<int, int>& IArray) 
{
	static	SAFEARRAY   *m_pSafeArray = NULL;					// 2004.05.10

	VARIANT Result;
	VariantInit(&Result);

	VARIANT *ElemVar;
	long    ArrayIndex[1]; // This is a 1D array, so it's index is a 1 vertex array

	SAFEARRAYBOUND Bounds[1];

	int i;

	HRESULT hRc ;

	// Setup the array boundrys
	Bounds[0].lLbound = 0;
	Bounds[0].cElements = IArray.GetSize();

	// If m_pSafeArray is not NULL, initialize this array 
	if (m_pSafeArray)											// 2004.05.10
	{
		SafeArrayDestroy(m_pSafeArray);	
		m_pSafeArray = NULL;
	}

	m_pSafeArray = SafeArrayCreate( VT_VARIANT, 1, Bounds);
	if ( m_pSafeArray == NULL) return Result;

	for( i = 0; i< IArray.GetSize(); i++ )
	{
		// Create a new Variant to put his element into
		ElemVar = new VARIANT;
		if( ElemVar == NULL) return Result;

		// Init variant
		VariantInit( ElemVar );

		// Set type and data
		ElemVar->vt = VT_I4;
		ElemVar->lVal = (long)(IArray.GetAt(i));

		// put elemtent into array
		ArrayIndex[0] = i;
		hRc = SafeArrayPutElement( m_pSafeArray, ArrayIndex, ElemVar ); 
		if( FAILED( hRc ) )										// 2004.05.10
		{
			VariantClear( ElemVar );	//SJK_51
			delete ElemVar;
			return Result;
		}

		// The element has now been copied, so we can delete it.
		VariantClear( ElemVar );
		delete ElemVar;
	}

	// Setup the array variant
	Result.vt = VT_ARRAY | VT_VARIANT;
	Result.parray = m_pSafeArray;

	return Result;
}

int VarArrayToIntArray(const VARIANT *Array, CArray<int, int>& IArray)
{
    HRESULT     hRc;
    SAFEARRAY   *ArrayPointer;
    long        ArrayIndex[1];
    int         ArrayMin, ArrayMax,ArraySize; // Min and Max in X dimention
    VARIANT     Element;
    VARIANT     ByteElement;
    int         i,j;
  
	// Initialise our temp variant structure
    VariantInit( &ByteElement );
	VariantInit( &Element );
	IArray.RemoveAll();

    ////////////////////////////////////////////////////////////////////////////
    //  Check type info, and get the actual array pointer.
    // It's posible to get a variant with nothing but a variant in it.
	while( Array->vt == (VT_BYREF | VT_VARIANT) )
	{
        Array = Array->pvarVal;
    }
    // Now the variant should be an array or dispatch ptr, if not, we've got a type error.
    if( Array->vt & VT_ARRAY )
    {
        if( Array->vt & VT_BYREF )
            ArrayPointer = *Array->pparray;
        else
            ArrayPointer = Array->parray;
    }
    else
	{
		return 0;
	}
        
    // Get array bounds.
    ArrayMin = ArrayPointer->rgsabound[0].lLbound;
	ArraySize= ArrayPointer->rgsabound[0].cElements;
    ArrayMax = ArrayMin + ArraySize; // Actualy last index +1 so we can do for( i=ArrayMin; i<ArrayMax;)

	// Loop through the array.
	for( i=ArrayMin, j=0; i< ArrayMax; i++,j++ )
	{
		ArrayIndex[0] = i;

		hRc = SafeArrayGetElement( ArrayPointer, &ArrayIndex[0], &Element);
		if( FAILED( hRc ) ) 
			break;

		hRc = VariantChangeType( &ByteElement, &Element, (USHORT)0, VT_I4 );
		if( FAILED( hRc ) )
			break;

		IArray.Add(ByteElement.lVal);

		VariantClear( &ByteElement );				// [#288] NH KGS 2008.06.12
		VariantClear( &Element );					// [#288] NH KGS 2008.06.12
	}
	ArrayPointer = NULL;

	//SJK_51
    VariantClear( &ByteElement );
	VariantClear( &Element );

    return( IArray.GetSize() );

}

/*
 end of VARIANT FUNCTIONS
*/


/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnBNA_ClearErrorCode()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Clear Error Code
-------------------------------------------------------------------*/
int	CDevCmn::fnBNA_ClearErrorCode()
{
	NHDEBUG(DBG_CALL, (_T("\n")));

	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_BNA))
		nReturn = m_pMwi->BnaClearErrorCode();

	return nReturn;
}

int CDevCmn::fnBNA_ClearCashInInfo()
{
	NHDEBUG(DBG_CALL, (_T("\n")));

	int nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_BNA))
	{
		LPDISPATCH data = m_pMwi->BnaGetLogicalUnit(0);
		ILogicalUnit Unit(data);

		CArray<int, int> IItemArrary;
		int nNumOfPUnit;

		IItemArrary.RemoveAll();
		nNumOfPUnit = VarArrayToIntArray(&Unit.GetPCUTotalCount(), IItemArrary);

		IItemArrary.RemoveAll();
		for(int j=0; j<nNumOfPUnit; j++)
		{
			IItemArrary.Add(j);
			IItemArrary.SetAt(j, 0);
		}

		// LCU 매수 삭제
		// set(clear) TotalCount
		Unit.SetTotalCount(0);
		// set(clear) CashInCount
		Unit.SetCashInCount(0);

		// PCU 매수 삭제
		// set(clear) PCUTotalCount
		Unit.SetPCUTotalCount(IntArrayToSafeArray(IItemArrary));
		// set(clear) PCUCashInCount
		Unit.SetPCUCashInCount(IntArrayToSafeArray(IItemArrary));

		// set(clear) 권종별 매수 삭제
		IItemArrary.RemoveAll();
		int nNumOfItem = VarArrayToIntArray(&Unit.GetItemCount(), IItemArrary);

		IItemArrary.RemoveAll();
		for(int j=0; j<nNumOfItem; j++)
		{
			IItemArrary.Add(j);
			IItemArrary.SetAt(j, 0);
		}

		Unit.SetItemCount(IntArrayToSafeArray(IItemArrary));

		nReturn = m_pMwi->BnaSetCashUnitInfo();
	}

	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fstrBNA_GetErrorCode()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Get Error Code
-------------------------------------------------------------------*/
CString CDevCmn::fstrBNA_GetErrorCode()
{
	NHDEBUG(DBG_CALL, (_T("fstrBNA_GetErrorCode() \n")));

	CString strReturn("");

	if (fnAPL_GetDefineDevice(DEV_BNA))
		strReturn = m_pMwi->BnaGetErrorCode();

	if(strReturn.GetLength() > 0)
	{
		NVDump('F', 'E', "56", L"", strReturn);		
	}

	return strReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnBNA_GetDeviceStatus()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Get Device Status
-------------------------------------------------------------------*/
int	CDevCmn::fnBNA_GetDeviceStatus()
{
	static	int		nSaveReturn = FALSE;
	int		nReturn = FALSE;

	if (fnAPL_GetDefineDevice(DEV_BNA))
		nReturn = m_pMwi->BnaGetDeviceStatus();

	if (nReturn != nSaveReturn)
	{
		NHDEBUG(DBG_CALL, (L"'%S' => '%S'\n", GETSTR_DVCST(nSaveReturn), GETSTR_DVCST(nReturn)));
		nSaveReturn = nReturn;
		m_strNVLog.Format(L"BNAST:%d", nReturn);
		NVDump('O', 'E', "58", L"", m_strNVLog);
	}
	return nReturn;
}


/*-------------------------------------------------------------------
CLASS    NAME: CDevCmn
FUNCTION NAME: fstrBNA_GetExtraStatus()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : Get Extra Status for given Key Name
-------------------------------------------------------------------*/
CString CDevCmn::fstrBNA_GetExtraStatus(CString KeyName)
{
	NHDEBUG(DBG_CALL, (_T("\n")));

	CString strReturn("");

	if (fnAPL_GetDefineDevice(DEV_BNA))
		strReturn = m_pMwi->BnaGetExtraStatus(KeyName);

	//if (strReturn.GetLength() > 0)
	{
		NVDump('O', 'E', "58", L"BNAeST:%s", strReturn);
	}

	return strReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnBNA_Initialize()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Initialize
-------------------------------------------------------------------*/
int	CDevCmn::fnBNA_Initialize()
{
	NHDEBUG(DBG_CALL, (_T(" \n")));

	int		nReturn = FALSE;

	DeviceOperationFlag = TRUE;
	if (fnAPL_GetDefineDevice(DEV_BNA))
		nReturn = m_pMwi->BnaInitialize();

	if (nReturn == R_NORMAL)
		fnBNA_GetCashUnitInformation();

	return nReturn;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnBNA_Deinitialize()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
int	CDevCmn::fnBNA_Deinitialize()
{
	NHDEBUG(DBG_CALL, (_T("fnBNA_Deinitialize() \n")));

	int		nReturn = FALSE;

	DeviceOperationFlag = TRUE;
	if (fnAPL_GetDefineDevice(DEV_BNA))
		nReturn = m_pMwi->BnaDeinitialize();

	return nReturn;
}


/*-------------------------------------------------------------------
CLASS    NAME: CDevCmn
FUNCTION NAME: fnBNA_StartCashIn()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
int	CDevCmn::fnBNA_StartCashIn()
{
	NHDEBUG(DBG_CALL, (_T("\n")));

	int		nReturn = FALSE;

	DeviceOperationFlag = TRUE;
	if (fnAPL_GetDefineDevice(DEV_BNA))
		nReturn = m_pMwi->BnaStartCashIn();

	return nReturn;
}


/*-------------------------------------------------------------------
CLASS    NAME: CDevCmn
FUNCTION NAME: fnBNA_AcceptCash()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
int	CDevCmn::fnBNA_AcceptCash(long InsertionTime, long TakenTime)
{
	NHDEBUG(DBG_CALL, (_T("InsertionTime: (%d), TakenTime: (%d)\n"), InsertionTime, TakenTime));

	int		nReturn = FALSE;

	DeviceOperationFlag = TRUE;
	if (fnAPL_GetDefineDevice(DEV_BNA))
		nReturn = m_pMwi->BnaAcceptCash(InsertionTime, TakenTime);

	return nReturn;
}


/*-------------------------------------------------------------------
CLASS    NAME: CDevCmn
FUNCTION NAME: fnBNA_StoreCash()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
int	CDevCmn::fnBNA_StoreCash()
{
	NHDEBUG(DBG_CALL, (_T("fnBNA_StoreCash() \n")));

	int		nReturn = FALSE;

	DeviceOperationFlag = TRUE;
	if (fnAPL_GetDefineDevice(DEV_BNA))
		nReturn = m_pMwi->BnaStoreCash();

	return nReturn;
}

/*-------------------------------------------------------------------
CLASS    NAME: CDevCmn
FUNCTION NAME: fnBNA_RollbackCash()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
int	CDevCmn::fnBNA_RollbackCash(long TakenTimeOut)
{
	NHDEBUG(DBG_CALL, (_T("takenTimeout: %d \n"), TakenTimeOut));

	int		nReturn = FALSE;

	DeviceOperationFlag = TRUE;
	if (fnAPL_GetDefineDevice(DEV_BNA))
		nReturn = m_pMwi->BnaRollbackCash(TakenTimeOut);

	return nReturn;
}

/*-------------------------------------------------------------------
CLASS    NAME: CDevCmn
FUNCTION NAME: fnBNA_CancelAccept()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
int	CDevCmn::fnBNA_CancelAccept()
{
	NHDEBUG(DBG_CALL, (_T("\n")));

	int		nReturn = FALSE;

	DeviceOperationFlag = TRUE;
	if (fnAPL_GetDefineDevice(DEV_BNA))
		nReturn = m_pMwi->BnaCancelAccept();

	return nReturn;
}


/*-------------------------------------------------------------------
CLASS    NAME: CDevCmn
FUNCTION NAME: fnBNA_GetCashUnitInformation()
RETURN TYPE  : 
PARAMETER    : 
DESCRIPTION  : 
-------------------------------------------------------------------*/
int	CDevCmn::fnBNA_GetCashUnitInformation()
{
	NHDEBUG(DBG_CALL, (_T("\n")));

	int		nReturn = FALSE;

	DeviceOperationFlag = TRUE;
	if (fnAPL_GetDefineDevice(DEV_BNA))
	{
		nReturn = m_pMwi->BnaGetNumberOfLogicalUnit();

		// BnaGetLogicalUnit() makes 0x80000002 (datatype misalignment) exception sometimes... applied workaround below
		// refer: https://stackoverflow.com/questions/23882798/unhandled-exception-0x80000002-datatype-misalignment-while-using-dynamic-cast
		LPDISPATCH data = m_pMwi->BnaGetLogicalUnit(0);

		if (data == NULL)
		{
			NHDEBUG(DBG_CALL, (_T("Failed to get logical unit!\n")));
			return FALSE;
		}

		ILogicalUnit Unit(data);

		NHDEBUG(DBG_CALL, (_T("GetLogicalUnit [GetCashInCount=%d] \n"), Unit.GetCashInCount()));
		NHDEBUG(DBG_CALL, (_T("GetLogicalUnit [GetTotalCount=%d] \n"), Unit.GetTotalCount()));
		NHDEBUG(DBG_CALL, (_T("GetLogicalUnit [GetMaximumCount=%d] \n"), Unit.GetMaximumCount()));
		NHDEBUG(DBG_CALL, (_T("GetLogicalUnit [GetCurrencyID=%s] \n"), Unit.GetCurrencyID()));
		NHDEBUG(DBG_CALL, (_T("GetLogicalUnit [GetStatus=%s] \n"), Unit.GetStatus()));
		NHDEBUG(DBG_CALL, (_T("GetLogicalUnit [GetNumberOfItem=%d] \n"), Unit.GetNumberOfItem()));

		VARIANT vItemID = Unit.GetItemID();
		VARIANT vItemCount = Unit.GetItemCount();
		VARIANT vItemValue = Unit.GetItemValue();

		SAFEARRAY* parray[3];
		parray[0] = vItemID.parray;
		parray[1] = vItemCount.parray;
		parray[2] = vItemValue.parray;

		for(int v=0;v<3;v++)
		{
			LONG lBound = 0;
			LONG uBound = 0;

			::SafeArrayGetLBound(parray[v], 1, &lBound);
			::SafeArrayGetUBound(parray[v], 1, &uBound);

			CString str = _T("");
			CString temp;
			for (LONG i = lBound; i <= uBound; i++)
			{
				VARIANT varElement;
				::SafeArrayGetElement(parray[v], &i, &varElement);
				::VariantChangeType(&varElement, &varElement, 0, VT_I4);
				temp.Format(_T("(%d)"), varElement.lVal);
				str += temp;
			}

			// -- !! CAUTION !! : It is the information of "CASH IN TOTAL"
			//   [GetNumberOfItem=8]
			//   [ItemId=(1)(2)(3)(4)(5)(6)(7)(8)] 
			//   [ItemCount=(0)(0)(0)(0)(0)(0)(1)(0)] 
			//   [ItemValue=(1)(2)(5)(10)(20)(50)(100)(0)] 
			if(v==0)
			{
				NHDEBUG(DBG_CALL, (_T("GetLogicalUnit [ItemId=%s] \n"), str));
			}
			else if(v==1)
			{
				NHDEBUG(DBG_CALL, (_T("GetLogicalUnit [ItemCount=%s] \n"), str));
			}
			else
			{
				NHDEBUG(DBG_CALL, (_T("GetLogicalUnit [ItemValue=%s] \n"), str));
			}
		}

		// clear variables
		m_nBnaNumberOfItem = 0;
		m_nBnaCSTStatus = BNA_CST_UNKNOWN;
		memset(m_nBnaItemCount, 0, sizeof(m_nBnaItemCount));
		memset(m_nBnaItemValue, 0, sizeof(m_nBnaItemValue));

		m_nBnaNumberOfItem = Unit.GetNumberOfItem();
		
		if (Unit.GetStatus() == L"OK" || Unit.GetStatus() == L"HIGH")
		{
			m_nBnaCSTStatus = BNA_CST_NORMAL;
		}
		else if (Unit.GetStatus() == L"MISSING")
		{
			m_nBnaCSTStatus = BNA_CST_MISSING;
		}
		else if (Unit.GetStatus() == L"FULL")
		{
			m_nBnaCSTStatus = BNA_CST_FULL;
		}

		for (LONG i=0; i<m_nBnaNumberOfItem; i++)
		{
			VARIANT varItemValue;
			::SafeArrayGetElement(parray[2], &i, &varItemValue);
			::VariantChangeType(&varItemValue, &varItemValue, 0, VT_I4);

			m_nBnaItemValue[i] = varItemValue.lVal;

			VARIANT varItemCount;
			::SafeArrayGetElement(parray[1], &i, &varItemCount);
			::VariantChangeType(&varItemCount, &varItemCount, 0, VT_I4);

			m_nBnaItemCount[i] = varItemCount.lVal;
		}
	}

	return nReturn;
}

		
int CDevCmn::fnBNA_GetLastCashInValue()
{
	NHDEBUG(DBG_CALL, (_T("\n")));

	int nCashInValue = 0;

	long lReturn = m_pMwi->BnaGetNumberOfCashInStatus();
	if (lReturn)
	{
		LPDISPATCH data = m_pMwi->BnaGetCashInStatus(0);
		ICashInStatus CashInStatus(data);

		NHDEBUG(1, (_T("GetCashInStatus [GetCurrency=%s] \n"), CashInStatus.GetCurrency()));
		NHDEBUG(1, (_T("GetCashInStatus [GetId=%d] \n"), CashInStatus.GetId()));
		NHDEBUG(1, (_T("GetCashInStatus [GetValue=%d] \n"), CashInStatus.GetValue()));
		NHDEBUG(1, (_T("GetCashInStatus [GetItemCount=%d] \n"), CashInStatus.GetItemCount()));

		nCashInValue = CashInStatus.GetValue();
	}

	return nCashInValue;
}

CString CDevCmn::fstrBNA_GetLastCashInStatus()
{
	NHDEBUG(DBG_CALL, (_T("\n")));

	CString strReturn;

	DeviceOperationFlag = TRUE;
	if (fnAPL_GetDefineDevice(DEV_BNA))
		strReturn = m_pMwi->BnaGetLastCashInStatus();

	return strReturn;
}

CString CDevCmn::fstrBNA_GetAcceptorStatus()
{
	// [#RWC6-465] US ryan.payton 2022.12.22 DigitalMint check cassette status
	// Removed to unclutter the logs
#if !(APP_DIGITALMINT)
	NHDEBUG(DBG_CALL, (_T("\n")));
#endif

	CString strReturn;

	DeviceOperationFlag = TRUE;
	if (fnAPL_GetDefineDevice(DEV_BNA))
		strReturn = m_pMwi->BnaGetAcceptorStatus();

	return strReturn;
}

CString CDevCmn::fstrBNA_GetStackerStatus()
{
	NHDEBUG(DBG_CALL, (_T("\n")));

	CString strReturn;

	DeviceOperationFlag = TRUE;
	if (fnAPL_GetDefineDevice(DEV_BNA))
		strReturn = m_pMwi->BnaGetStackerStatus();

	return strReturn;
}
int CDevCmn::fnBNA_GetMaxCashInItem()
{
	NHDEBUG(DBG_CALL, (_T("\n")));

	int		nReturn = FALSE;

	DeviceOperationFlag = TRUE;
	if (fnAPL_GetDefineDevice(DEV_BNA))
		nReturn = m_pMwi->BnaGetMaxCashInItem();

	return nReturn;
}

int CDevCmn::fnBNA_GetMaxStackerItem()
{
	NHDEBUG(DBG_CALL, (_T("\n")));

	int		nReturn = FALSE;

	DeviceOperationFlag = TRUE;
	if (fnAPL_GetDefineDevice(DEV_BNA))
		nReturn = m_pMwi->BnaGetMaxStackerItem();

	return nReturn;
}

int CDevCmn::fnBNA_GetLastRefusedCount()
{
	NHDEBUG(DBG_CALL, (_T("\n")));

	int		nReturn = FALSE;

	DeviceOperationFlag = TRUE;
	if (fnAPL_GetDefineDevice(DEV_BNA))
		nReturn = m_pMwi->BnaGetLastRefusedCount();

	return nReturn;
}

int CDevCmn::fnBNA_GetAggregateDeviceStatus()
{
	// get the device state from XFS
	int nReturn = fnBNA_GetDeviceStatus();

	if (nReturn == NORMAL && fstrBNA_GetAcceptorStatus() != ST_OK)
	{
		nReturn = DOWN;
	}

	return nReturn;
}

int CDevCmn::fnBNA_GetTotalCount()
{
	int total = 0;

	for (LONG i = 0; i < m_nBnaNumberOfItem; i++)
		total += m_nBnaItemValue[i] * m_nBnaItemCount[i];

	return total;
}

// [RWC6-676] Start SKKim 2024.05.17
CString CDevCmn::fstrBNA_GetPositionStatus()
{
	// "EMPTY", "NOTEMPTY","NOTSUPP", "UNKNOWN"
	CString strReturn;

	if (fnAPL_GetDefineDevice(DEV_BNA))
		strReturn = m_pMwi->BnaGetPositionStatus();

	return strReturn;
}
// [RWC6-676] End SKKim 2024.05.17

// [RWC6-676] Start SKKim 2024.05.27
BOOL CDevCmn::fbBNA_GetRefusedStatus()
{
	// "EMPTY", "NOTEMPTY","NOTSUPP", "UNKNOWN"
	int nReturn;

	if (fnAPL_GetDefineDevice(DEV_BNA))
		nReturn = m_pMwi->BnaGetRefusedStatus();

	if (nReturn == 1)
		return TRUE;

	return FALSE;
}
// [RWC6-676] End SKKim 2024.05.27