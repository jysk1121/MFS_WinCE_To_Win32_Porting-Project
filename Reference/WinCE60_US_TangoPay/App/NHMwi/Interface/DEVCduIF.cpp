// DEVCDUIF.cpp

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
// CDU ActiveX 
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// Method : Call to CDU
///////////////////////////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// DESCRIPT : 장치디바이스 세션연결 및 초기화
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevCduOpenConnection()
{
	TCHAR	szCode[256];
	int		nResult = 0;

	// nCSTMinimum4Mix6 보정매수 구하기?						// 2004.11.05
	memset(szCode, 0x00, sizeof(szCode));
	nResult = RegQueryValueExt(_REGKEY_CDUSTATUS, _T("CSTMinimum4Mix6"), REG_STR, sizeof(szCode), (LPVOID)szCode);
	if (nResult)
		m_nCSTMinimum4Mix6 = Asc2Int(szCode);//atoi(szCode); //Asc2Int((LPSTR)szCode);			// 2004.11.05

	NHDEBUG(1, (_T("OpenSessionSync() ... \n")));
	nResult = m_pCdu.OpenSessionSync(K_30_WAIT*1000);
	NHDEBUG(1, (L"\n"));
	NHDEBUG(1, (_T("OpenSessionSync() ... DONE (%d)\n"), nResult));

	if (nResult != R_NORMAL)									// 장애검지시(FATAL/*NODEVICE*)
	{
		// KSK_2007-08-16오전 9:43:14
//		CString strErrorCode = "9792500";
		CString strErrorCode = "9792400";

		m_pDevCtrl->WriteMwiErrorCode(DEV_CDU, "CDM", strErrorCode.GetLength(), "9792400");
		// end of KSK_2007-08-16오전 9:43:14
		ProcSetDeviceEvent(L"CDU", L"FatalError", NULL);
	}
	else
	{
		// This is to remove a fatal error generated after successful OpenSessionSync(), which was executed to open SP which was killed by force by GOMA.
		m_pDevCtrl->EvtQReset(DEV_CDU);

		ProcSetDeviceEvent(L"CDU", L"OpenComplete", NULL);
	}

	DevCduUpdateStatus();

//	return R_NORMAL;
	return nResult;	// [#136] KSK 2008.04.18  ATM기동시 CDU 단선발생 후 OP에서 복구 안되는 문제 수정
}

// ----------------------------------------------------------------------------
// DESCRIPT : 장치디바이스 세션종료
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevCduCloseConnection()
{
//	m_pCdu.DisableStatusEvents();								// 상태변경 이벤트를 Disable처리한다.

	m_pCdu.CloseSessionSync();
	return R_NORMAL;
}

// ----------------------------------------------------------------------------
// DESCRIPT : 장치디바이스 상태변경통보 가능처리
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevCduEnableStatusEvents()
{
//	m_pCdu.EnableStatusEvents();
//	m_pCdu.EnableCashUnitEvents();

	return R_NORMAL;
}

// ----------------------------------------------------------------------------
// DESCRIPT : 장치디바이스 상태변경통보 불가처리
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevCduDisableStatusEvents()
{
//	m_pCdu.DisableStatusEvents();
//	m_pCdu.DisableCashUnitEvents();

	return R_NORMAL;
}

// ----------------------------------------------------------------------------
// DESCRIPT : 셔터 열기
// REMARK	: 셔터를 닫고 열때 현재 셔터상태를 확인하지 않고 무조건 처리하도록 한다.
//			  SP에서 수취시 닫지 않는 경우가 있어 이를 대책하면서 모든 열고 닫는 동작을
//			  상태에 상관없이 AP/SP에 위임한다.(2004.04.01 teo)
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevCduOpenShutter()
{
	m_pCdu.OpenShutter();
	return R_NORMAL;
}

// ----------------------------------------------------------------------------
// DESCRIPT : 셔터 닫기
// REMARK	: 셔터를 닫고 열때 현재 셔터상태를 확인하지 않고 무조건 처리하도록 한다.
//			  SP에서 수취시 닫지 않는 경우가 있어 이를 대책하면서 모든 열고 닫는 동작을
//			  상태에 상관없이 AP/SP에 위임한다.(2004.04.01 teo)
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevCduCloseShutter()
{
	m_pCdu.CloseShutter();
	return R_NORMAL;
}

// ----------------------------------------------------------------------------
// DESCRIPT : MIX
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevCduDispenseAmount(int nAmount)
{
	m_nCduDispenseAmount = nAmount;
	
	VARIANT varTemp;
	VariantInit(&varTemp);
	varTemp = m_pCdu.GetUnitCount();

	// [#2459] AU KSK 2016.12.23
//	m_pCdu.Denominate(_T("1"), m_strCurrency, nAmount, varTemp);
#if (AU_A_VERSION)
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DISPENSE_MIX_OPTION) == CDM_MIX_MAXIMUM_NUMBER_OF_BILLS)
		m_pCdu.Denominate(_T("10"), m_strCurrency, nAmount, varTemp);
	else
		m_pCdu.Denominate(_T("1"), m_strCurrency, nAmount, varTemp);
#else
	if ((MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DISPENSE_STYLE) == 0) ||
		(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DISPENSE_STYLE) == NULL))	
	{
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DISPENSE_STYLE, DISPENSE_STYLE_LEAST);
	}

	CString sDispenseStyle;
	sDispenseStyle.Format(L"%d", MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DISPENSE_STYLE));

	m_pCdu.Denominate(sDispenseStyle, m_strCurrency, nAmount, varTemp);
#endif
	// end of [#2459]

	VariantClear(&varTemp);

	return R_NORMAL;
}

// ----------------------------------------------------------------------------
// DESCRIPT : Dispense and Present after Mix
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevCduDispenseAndPresentAmount(VARIANT NoteCnt, int nWaitSec)
{
	// [#2459] AU KSK 2016.12.23
//	m_pCdu.Dispense(_T("1"), m_strCurrency, m_nCduDispenseAmount, NoteCnt, TRUE, nWaitSec);
#if (AU_A_VERSION)
	if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DISPENSE_MIX_OPTION) == CDM_MIX_MAXIMUM_NUMBER_OF_BILLS)
		m_pCdu.Dispense(_T("10"), m_strCurrency, m_nCduDispenseAmount, NoteCnt, TRUE, nWaitSec);
	else
		m_pCdu.Dispense(_T("1"), m_strCurrency, m_nCduDispenseAmount, NoteCnt, TRUE, nWaitSec);
#else
	if ((MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DISPENSE_STYLE) == 0) ||
		(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DISPENSE_STYLE) == NULL))	
	{
		MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DISPENSE_STYLE, DISPENSE_STYLE_LEAST);
	}

	CString sDispenseStyle;
	sDispenseStyle.Format(L"%d", MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_DISPENSE_STYLE));

	m_pCdu.Dispense(sDispenseStyle, m_strCurrency, m_nCduDispenseAmount, NoteCnt, TRUE, nWaitSec);
#endif
	// end of [#2459]
	
	return R_NORMAL;
}

int CNHMWICtrl::DevCduDispenseAndPresentCount(int nCST1Cnt, int nCST2Cnt /* = 0 */, int nCST3Cnt /* = 0 */, int nCST4Cnt /* = 0 */, int nWaitSec)
{
	m_nCduDispenseAmount = DevCduGetValueOfCash(CDU_CST_1) * nCST1Cnt + 
						   DevCduGetValueOfCash(CDU_CST_2) * nCST2Cnt + 
						   DevCduGetValueOfCash(CDU_CST_3) * nCST3Cnt + 
						   DevCduGetValueOfCash(CDU_CST_4) * nCST4Cnt ;

	if (m_nCduDispenseAmount == 0)								// 금액검증
	{
		OnNotDispensableNxCashDispenser();
		return R_ERROR;
	}

	CArray<int, int> IArray;
	IArray.RemoveAll();

//	VARIANT	ChangeCST;
//    VariantInit( &ChangeCST );
	
	IArray.Add(0);	//Reject CST Count

	int nCSTCnt = DevCduGetCSTCount();

//	for ( int i = 0 ; i < DevCduGetCSTCount() ; i++ )
	for ( int i = 0 ; i < nCSTCnt ; i++ )
	{
		switch(i)
		{
		case 0:
			IArray.Add(nCST1Cnt);
			break;
		case 1:
			IArray.Add(nCST2Cnt);
			break;
		case 2:
			IArray.Add(nCST3Cnt);
			break;
		case 3:
			IArray.Add(nCST4Cnt);
			break;
		default:
			break;
		}
	}

	//-- SJKWONNOTE 2006-06-19 --Modify : Memory leak 수정//
	VARIANT varTemp;
	VariantInit(&varTemp);
	varTemp = IntArrayToSafeArray(IArray);
	//m_pCdu.Dispense(L"0", L"USD", 0, varTemp, TRUE, nWaitSec*1000);
	m_pCdu.Dispense(L"0", m_strCurrency, 0, varTemp, TRUE, nWaitSec*1000);
//	VariantClear(&varTemp);		// [#363] NH JSW 2008.07.18 IntArrayToSafeArray 내부에서 Clear됨
	//--
	return R_NORMAL;
}


// ----------------------------------------------------------------------------
// DESCRIPT : Dispense After Mix
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevCduDispense(VARIANT NoteCnt)
{
//	m_pCdu.Dispense(m_nCduDispenseAmount, NoteCnt, "CNY", "6"); // 2004.11.05
//	m_pCdu.Dispense("6", "CNY", m_nCduDispenseAmount, NoteCnt, FALSE, K_60_WAIT*1000);	// 2005.12.31
//	m_pCdu.Dispense("6", m_strCurrency, m_nCduDispenseAmount, NoteCnt, FALSE, K_60_WAIT*1000);
	m_pCdu.Dispense(_T("1"), m_strCurrency, m_nCduDispenseAmount, NoteCnt, FALSE, K_60_WAIT*1000);
	return R_NORMAL;
}

// ----------------------------------------------------------------------------
// DESCRIPT : MIXANDDISPENSE
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevCduDispenseCount(int nCST1Cnt, int nCST2Cnt /* = 0 */, int nCST3Cnt /* = 0 */, int nCST4Cnt /* = 0 */)
{
	m_nCduDispenseAmount = DevCduGetValueOfCash(CDU_CST_1) * nCST1Cnt + 
						   DevCduGetValueOfCash(CDU_CST_2) * nCST2Cnt + 
						   DevCduGetValueOfCash(CDU_CST_3) * nCST3Cnt + 
						   DevCduGetValueOfCash(CDU_CST_4) * nCST4Cnt ;

	if (m_nCduDispenseAmount == 0)								// 금액검증
	{
		OnNotDispensableNxCashDispenser();
		return R_ERROR;
	}

	CArray<int, int> IArray;
	IArray.RemoveAll();

//	VARIANT	ChangeCST;
//    VariantInit( &ChangeCST );

	for ( int i = 0 ; i < DevCduGetCSTCount() ; i++ )
	{
		switch(i)
		{
		case 0:
			IArray.Add(nCST1Cnt);
			break;
		case 1:
			IArray.Add(nCST2Cnt);
			break;
		case 2:
			IArray.Add(nCST3Cnt);
			break;
		case 3:
			IArray.Add(nCST4Cnt);
			break;
		default:
			break;
		}
	}
																// 2004.11.05
//	m_pCdu.Dispense(m_nCduDispenseAmount, IntArrayToSafeArray(IArray), "CNY", "6");
//	m_pCdu.Dispense("6", "CNY", m_nCduDispenseAmount, IntArrayToSafeArray(IArray), FALSE, K_60_WAIT*1000);
//	m_pCdu.Dispense("6", m_strCurrency, m_nCduDispenseAmount, IntArrayToSafeArray(IArray), FALSE, K_60_WAIT*1000);
	
	//-- SJKWONNOTE 2006-06-19 --Modify : Memory leak 수정//
	VARIANT varTemp;
	VariantInit(&varTemp);
	varTemp = IntArrayToSafeArray(IArray);
	//m_pCdu.Dispense(_T("1"), m_strCurrency, m_nCduDispenseAmount, varTemp, FALSE, K_60_WAIT*1000); 
	m_pCdu.Dispense(_T("0"), m_strCurrency, 0, varTemp, FALSE, K_60_WAIT*1000); // m_nCduDispenseAmount>>0 (use count, not amount)// [#RWC6-12, #2584] US Brandon 2019.02.04 Denomination Selection demo version for ATMIA
//	VariantClear(&varTemp);		// [#363] NH JSW 2008.07.18 IntArrayToSafeArray 내부에서 Clear됨
	//--
	return R_NORMAL;
}

// ----------------------------------------------------------------------------
// DESCRIPT : 방출 및 수취대기
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevCduPresent(int nWaitSec)
{
	m_nCduDispenseAmount = 0;

	m_bCduWaitTaken = TRUE;										// 20030814_1.0_1066 : Pre-CancelWaitTaken Proc.
	m_pCdu.Present(nWaitSec*1000);

	return R_NORMAL;
}

// ----------------------------------------------------------------------------
// DESCRIPT : 회수처리
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevCduRetract()
{
	m_nCduDispenseAmount = 0;

	m_pCdu.Retract(3);											// Retract to REJECT_BIN
	return R_NORMAL;
}

// ----------------------------------------------------------------------------
// DESCRIPT : 수취대기취소
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevCduCancelWaitTaken()
{
//	return m_pCdu.CancelWaitForCashTaken();
	return R_NORMAL;
}

// ----------------------------------------------------------------------------
// DESCRIPT : 장치디바이스 상태조회
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevCduGetDeviceStatus()
{
	int nResult = NORMAL;
	CString strStatus = m_pDevCtrl->GetDeviceStatus(_T("CDUStDeviceStatus"));

	if		(strStatus.CompareNoCase(ST_DEVONLINE)	== 0)		nResult = NORMAL;
	else if	(strStatus.CompareNoCase(ST_DEVNODEVICE)== 0)		nResult = NODEVICE;
	else if	(strStatus.CompareNoCase(ST_DEVOFFLINE)== 0)		nResult = OFFLINE;
	else	nResult = DOWN;

	return nResult;
}

// ----------------------------------------------------------------------------
// DESCRIPT : 장치디바이스 장애코드 삭제
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevCduClearErrorCode()
{
	RegSetValueExt(_REGKEY_DEVERROR, _T("CDM"), REG_STR, 0, NULL);	// 현금부
	return NORMAL;
}

// ----------------------------------------------------------------------------
// DESCRIPT : 장애코드 조회
// ----------------------------------------------------------------------------
CString CNHMWICtrl::DevCduGetErrorCode()
{
	CString strResult("");
	
	char szTemp[10];
	memset(szTemp, NULL, sizeof(szTemp));
	memcpy(szTemp, m_pSPInform->SPErr.ErrCode_CDU, 7);
		
	strResult = CString(szTemp);

	return strResult;
}

// ----------------------------------------------------------------------------
// DESCRIPT : 현금부의 출금가능 권종정보 조회
// REMARK	: 중국현지화	2004.03.31
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevCduAvailWithdraw()
{
	int nResult = WITH_NOT_AVAILABLE;

	CString strStatus = m_pDevCtrl->GetDeviceStatus(_T("CDUAvailWithdraw"));
	BYTE btRet = 0;

	if (!strStatus.CompareNoCase(_T("WITH_NOT_AVAILABLE")))
		nResult = WITH_NOT_AVAILABLE;
	else
		nResult = Asc2Int(strStatus);

	return nResult;
/*
	int nResult = WITH_NOT_AVAILABLE;

	CString strStatus = m_pDevCtrl->GetDeviceStatus("CDUAvailWithdraw");
	if		(strStatus.CompareNoCase("WITH_TYPE1_ONLY")		== 0)	nResult = WITH_TYPE1_ONLY;
	else if (strStatus.CompareNoCase("WITH_TYPE2_ONLY")		== 0)	nResult = WITH_TYPE2_ONLY;
	else if (strStatus.CompareNoCase("WITH_BOTH_AVAILABLE") == 0)	nResult = WITH_BOTH_AVAILABLE;
	else nResult = WITH_NOT_AVAILABLE;
	
	return nResult;
*/
}

// ----------------------------------------------------------------------------
// DESCRIPT : 현금부의 현금권종 조회
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevCduGetValueOfCash(int nCSTNo)
{
	if (DevCduGetDeviceStatus() == NODEVICE)					// 장치비정상시 조회불가 : 2004.06.29
		return 0;

	CArray<int, int> IArray;
	IArray.RemoveAll();
	int nCSTSize	= 0;
	//-- SJKWONNOTE 2006-06-17 --Modify : Memory leak 수정//
	VARIANT varTemp;
	VariantInit(&varTemp);
	varTemp = m_pCdu.GetUnitValue();
	nCSTSize = VarArrayToIntArray(&varTemp, IArray);
	VariantClear(&varTemp);
	//--
	if (nCSTNo >= nCSTSize)										// Index오류
	{
		return 0;
	}

	return IArray.GetAt(nCSTNo);
}

// ----------------------------------------------------------------------------
// CDU Get Number Of Cash
// ----------------------------------------------------------------------------
int	CNHMWICtrl::DevCduGetNumberOfCash(int nType, int nCSTNo)
{
	if (DevCduGetDeviceStatus() == NODEVICE)					// 장치비정상시 조회불가 : 2004.06.29
		return 0;

	CArray<int, int> IArray;
	IArray.RemoveAll();
	int nCSTSize	= 0;

	//-- SJKWONNOTE 2006-06-17 --Modify : Memory leak 수정//
	VARIANT varTemp;
	VariantInit(&varTemp);

	switch(nType)
	{
	case 0:														// 설정매수조회
		varTemp = m_pCdu.GetPhysicalInitialCount();
		nCSTSize = VarArrayToIntArray(&varTemp, IArray);
		VariantClear(&varTemp);
		break;
	case 1:														// 현재매수조회
		varTemp = m_pCdu.GetPhysicalCount();
		nCSTSize = VarArrayToIntArray(&varTemp, IArray);
		VariantClear(&varTemp);
		break;
	default:
		return 0;
	}
	//--
	if (nCSTNo >= nCSTSize)										// Index오류
	{
		return 0;
	}

	return IArray.GetAt(nCSTNo);
}

// ----------------------------------------------------------------------------
// CDU Position Information
// ----------------------------------------------------------------------------
int	CNHMWICtrl::DevCduGetPosition()
{
	int nResult = NORMAL_POS;
	CString strStatus = m_pDevCtrl->GetDeviceStatus(_T("CDUPosition"));

	if (strStatus.CompareNoCase(_T("ABNORMAL_POS"))	== 0)	nResult = ABNORMAL_POS;
	else nResult = NORMAL_POS;

	return nResult;
}
// ----------------------------------------------------------------------------
// CDU Get RBBox Status
// ----------------------------------------------------------------------------
int	CNHMWICtrl::DevCduGetRecycleBoxStatus()
{
	int nResult = CST_NORMAL;
	CString strStatus = m_pDevCtrl->GetDeviceStatus(_T("CDURecycleBoxStatus"));

	if		(strStatus.CompareNoCase(_T("CST_EMPTY"))	== 0)	nResult = CST_EMPTY;
	else if (strStatus.CompareNoCase(_T("CST_SET_NG"))	== 0)	nResult = CST_SET_NG;
	else if (strStatus.CompareNoCase(_T("CST_NEAR"))	== 0)	nResult = CST_NEAR;
	else if (strStatus.CompareNoCase(_T("CST_FULL"))	== 0)	nResult = CST_FULL;
	else nResult = CST_NORMAL;

	return nResult;
}
// ----------------------------------------------------------------------------
// CDU Get RejectBox Status
// ----------------------------------------------------------------------------
int	CNHMWICtrl::DevCduGetRejectCSTStatus()
{
	int nResult = CST_NORMAL;
	CString strStatus = m_pDevCtrl->GetDeviceStatus(_T("CDURejectCSTStatus"));

	if		(strStatus.CompareNoCase(_T("CST_EMPTY"))	== 0)	nResult = CST_EMPTY;
	else if (strStatus.CompareNoCase(_T("CST_SET_NG"))	== 0)	nResult = CST_SET_NG;
	else if (strStatus.CompareNoCase(_T("CST_NEAR"))	== 0)	nResult = CST_NEAR;
	else if (strStatus.CompareNoCase(_T("CST_FULL"))	== 0)	nResult = CST_FULL;
	else nResult = CST_NORMAL;

	return nResult;
}

// ----------------------------------------------------------------------------
// CDU Get Device Status
// ----------------------------------------------------------------------------
int	CNHMWICtrl::DevCduGetStatus()
{
	return DevCduGetDeviceStatus();
}
// ----------------------------------------------------------------------------
// CDU Get Sensor Information
// 2004.04.01 teo	현재 레지스트리에 저장되어 있는 센서정보가 없습니다.
//					향후 버전업이 되어 센서정보가 올라오거나 경로가 확정되면
//					수정을 바랍니다.(현재 상태로도 NULL이 올라갑니다)
// ----------------------------------------------------------------------------
CString	CNHMWICtrl::DevCduGetSensorInfo()
{
	CString strResult("");
	TCHAR szCode[1024];		memset(szCode, 0x00, sizeof(szCode));
	int nResult = 0;

	// 레지스트리에서 장애코드를 확보한다.
	nResult = RegQueryValueExt(_REGKEY_CDUSTATUS, _T("Sensor"), REG_STR, sizeof(szCode), (LPVOID)szCode);
	if (nResult)
	{
		strResult.Format(_T("%S"), szCode);
	}

	return strResult;
}

// ----------------------------------------------------------------------------
// CDU 매체잔류정보
// ----------------------------------------------------------------------------
int	CNHMWICtrl::DevCduGetMaterialInfo()
{
	CString strStatus = m_pDevCtrl->GetDeviceStatus(_T("CDUMaterialInfo"));
//	return ::atoi((LPCSTR)strStatus.GetBuffer(0));	// AIREAT 20100211
	return _ttoi(strStatus);
}

// ----------------------------------------------------------------------------
// DESCRIPT  : 카세트별 정보설정전 초기화처리
// PARAMETER : 정보를 변경하고자 하는 CST의 인덱스
// RETURN    : 0-NORMAL
// REMARK    : 2004.04.01 
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevCduInitiateChange()
{
	// 2007.11.26 V01.02.19
	// DENOMINATION 변경장애(EP장애)시 STARTEXCHANGESYNC가 가능하도록 수정
//	if (DevCduGetDeviceStatus() != NORMAL)						// 장치비정상시 처리불가
//	{
//		ProcSetDeviceEvent(L"CDU", L"FatalError", NULL);
//		return DOWN;
//	}

	CArray<int, int> IArray;
	IArray.RemoveAll();

	VARIANT	ChangeCST;
    VariantInit( &ChangeCST );

	VarArrayToIntArray(&ChangeCST, IArray);

	for ( int i = 0 ; i <= DevCduGetCSTCount() ; i++ )
		IArray.Add(i+1);

	int nReturn = m_pCdu.StartExchangeSync(IntArrayToSafeArray(IArray), FALSE);

	return nReturn;
}

// ----------------------------------------------------------------------------
// DESCRIPT  : 카세트별 매수설정
// PARAMETER : 카세트인덱스, 매수
// RETURN    : 각 CST의 매수정보를 설정
// REMARK    : 2004.03.31 
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevCduSetNumberOfCash(LPCTSTR szCashCntData)
{
	CString	strCashCntData(szCashCntData);						// 2004.11.05
	CStringArray strCashCntDataArray;

	SplitString(strCashCntData, ",", strCashCntDataArray);

	// [#391] [NH] KSK 2008.8.5
//	if (DevCduGetDeviceStatus() != NORMAL)						// 장치비정상시 처리불가
//	{
//		ProcSetDeviceEvent(L"CDU", L"FatalError", NULL);
//		return 0;
//	}
	// end of [#391]

	CArray<int, int> IArray;
	IArray.RemoveAll();
	int nCSTSize	= 0;
	//-- SJKWONNOTE 2006-06-17 --Modify : Memory leak 수정//
	VARIANT varTemp;
	VariantInit(&varTemp);
	varTemp = m_pCdu.GetPhysicalCount();
	nCSTSize = VarArrayToIntArray(&varTemp, IArray);
	VariantClear(&varTemp);
	//--
	
	if ((nCSTSize > CDU_CST_REJECT) &&							// 2004.11.05
		(strCashCntDataArray.GetSize() > CDU_CST_REJECT))
	{
		IArray.SetAt(CDU_CST_REJECT, Asc2Int(strCashCntDataArray[CDU_CST_REJECT]));
	}
	if ((nCSTSize > CDU_CST_1) &&
		(strCashCntDataArray.GetSize() > CDU_CST_1))
	{
		IArray.SetAt(CDU_CST_1, Asc2Int(strCashCntDataArray[CDU_CST_1]));
	}
	if ((nCSTSize > CDU_CST_2) &&
		(strCashCntDataArray.GetSize() > CDU_CST_2))
	{
		IArray.SetAt(CDU_CST_2, Asc2Int(strCashCntDataArray[CDU_CST_2]));
	}
	if ((nCSTSize > CDU_CST_3) &&
		(strCashCntDataArray.GetSize() > CDU_CST_3))
	{
		IArray.SetAt(CDU_CST_3, Asc2Int(strCashCntDataArray[CDU_CST_3]));
	}
	if ((nCSTSize > CDU_CST_4) &&
		(strCashCntDataArray.GetSize() > CDU_CST_4))
	{
		IArray.SetAt(CDU_CST_4, Asc2Int(strCashCntDataArray[CDU_CST_4]));
	}
	m_pCdu.SetPhysicalInitialCount(IntArrayToSafeArray(IArray));
	m_pCdu.SetPhysicalCount(IntArrayToSafeArray(IArray));
	int nReturn = m_pCdu.EndExchangeSync();

	return nReturn;
}

// ----------------------------------------------------------------------------
// DESCRIPT  : 카세트별 Initial/Remain 매수설정
// PARAMETER : Initial Count, Remain Count
// RETURN    : ADD CASH 때 각 CST의 Initial/Remain 매수정보를 설정
// REMARK    : 2006.01.23
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevCduAddNumberOfCash(LPCTSTR szInitialCnt, LPCTSTR szAddCnt)
{
	CString	strInitialData(szInitialCnt);
	CStringArray strInitialCntArray;
	CString	strAddData(szAddCnt);
	CStringArray strAddCntArray;

	// Initial Count Split
	SplitString(strInitialData, ",", strInitialCntArray);
	// Add Count Split
	SplitString(strAddData, ",", strAddCntArray);

	// [#391] [NH] KSK 2008.8.5
//	if (DevCduGetDeviceStatus() != NORMAL)						// 장치비정상시 처리불가
//	{
//		ProcSetDeviceEvent(L"CDU", L"FatalError", NULL);
//		return 0;
//	}
	// end of [#391]

	// Initial Count Array
	CArray<int, int> IArray;
	IArray.RemoveAll();

	// Add Count Array
	CArray<int, int> AddArray;
	AddArray.RemoveAll();

	int nCSTSize	= 0;
	
	//-- SJKWONNOTE 2006-06-17 --Modify : Memory leak 수정//
	VARIANT varTemp;
	VariantInit(&varTemp);
	varTemp = m_pCdu.GetPhysicalCount();
	nCSTSize = VarArrayToIntArray(&varTemp, IArray);
	nCSTSize = VarArrayToIntArray(&varTemp, AddArray);
	VariantClear(&varTemp);
	if ((nCSTSize > CDU_CST_REJECT) &&							// 2004.11.05
		(strAddCntArray.GetSize() > CDU_CST_REJECT))
	{
		IArray.SetAt(CDU_CST_REJECT, Asc2Int(strInitialCntArray[CDU_CST_REJECT]));
		AddArray.SetAt(CDU_CST_REJECT, Asc2Int(strAddCntArray[CDU_CST_REJECT]));
	}
	if ((nCSTSize > CDU_CST_1) &&
		(strAddCntArray.GetSize() > CDU_CST_1))
	{
		IArray.SetAt(CDU_CST_1, Asc2Int(strInitialCntArray[CDU_CST_1]));
		AddArray.SetAt(CDU_CST_1, Asc2Int(strAddCntArray[CDU_CST_1]));
	}
	if ((nCSTSize > CDU_CST_2) &&
		(strAddCntArray.GetSize() > CDU_CST_2))
	{
		IArray.SetAt(CDU_CST_2, Asc2Int(strInitialCntArray[CDU_CST_2]));
		AddArray.SetAt(CDU_CST_2, Asc2Int(strAddCntArray[CDU_CST_2]));
	}
	if ((nCSTSize > CDU_CST_3) &&
		(strAddCntArray.GetSize() > CDU_CST_3))
	{
		IArray.SetAt(CDU_CST_3, Asc2Int(strInitialCntArray[CDU_CST_3]));
		AddArray.SetAt(CDU_CST_3, Asc2Int(strAddCntArray[CDU_CST_3]));
	}
	if ((nCSTSize > CDU_CST_4) &&
		(strAddCntArray.GetSize() > CDU_CST_4))
	{
		IArray.SetAt(CDU_CST_4, Asc2Int(strInitialCntArray[CDU_CST_4]));
		AddArray.SetAt(CDU_CST_4, Asc2Int(strAddCntArray[CDU_CST_4]));
	}
	m_pCdu.SetPhysicalInitialCount(IntArrayToSafeArray(IArray));
	m_pCdu.SetPhysicalCount(IntArrayToSafeArray(AddArray));
	int nReturn = m_pCdu.EndExchangeSync();

	return nReturn;
}

// ----------------------------------------------------------------------------
// DESCRIPT  : 카세트별 Threshold설정
// PARAMETER : 카세트별 Max/Min 매수
// REMARK    : 2004.03.31 
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevCduSetMinMaxThreshold(LPCTSTR szMinCntData, LPCTSTR szMaxCntData) 
{
	CString	strMinCntData(szMinCntData);						// 2004.11.05
	CString	strMaxCntData(szMaxCntData);
	CStringArray strMinCntDataArray;
	CStringArray strMaxCntDataArray;

	SplitString(strMinCntData, ",", strMinCntDataArray);
	SplitString(strMaxCntData, ",", strMaxCntDataArray);

	if (DevCduGetDeviceStatus() != NORMAL)						// 장치비정상시 처리불가
	{
		CString strErrorCode = "9792500";
		m_pDevCtrl->WriteMwiErrorCode(DEV_CDU, "CDM", strErrorCode.GetLength(), "9792500");
		ProcSetDeviceEvent(L"CDU", L"FatalError", NULL);
		return 0;
	}

	CArray<int, int> MaxArray;
	CArray<int, int> MinArray;
	MaxArray.RemoveAll();
	MinArray.RemoveAll();
	int nMaxCSTSize	= 0;
	int nMinCSTSize	= 0;

	//-- SJKWONNOTE 2006-06-17 --Modify : Memory leak 수정//
	VARIANT varTemp;
	VariantInit(&varTemp);
	varTemp = m_pCdu.GetUnitMaximumCount();
	nMaxCSTSize = VarArrayToIntArray(&varTemp, MaxArray);
	VariantClear(&varTemp);

//	VariantInit(&varTemp);	// [#363] NH JSW 2008.07.15 불필요
	varTemp = m_pCdu.GetUnitMinimumCount();
	nMinCSTSize = VarArrayToIntArray(&varTemp, MinArray);
	VariantClear(&varTemp);
	//--

//////////////////////////////////////////////////////////////////////////
	if ((nMaxCSTSize > CDU_CST_REJECT) &&						// 2004.11.05
		(strMaxCntDataArray.GetSize() > CDU_CST_REJECT))
	{
		MaxArray.SetAt(CDU_CST_REJECT, Asc2Int(strMaxCntDataArray[CDU_CST_REJECT]));
	}
	if ((nMaxCSTSize > CDU_CST_1) &&
		(strMaxCntDataArray.GetSize() > CDU_CST_1))
	{
		MaxArray.SetAt(CDU_CST_1, Asc2Int(strMaxCntDataArray[CDU_CST_1]));
	}
	if ((nMaxCSTSize > CDU_CST_2) &&
		(strMaxCntDataArray.GetSize() > CDU_CST_2))
	{
		MaxArray.SetAt(CDU_CST_2, Asc2Int(strMaxCntDataArray[CDU_CST_2]));
	}
	if ((nMaxCSTSize > CDU_CST_3) &&
		(strMaxCntDataArray.GetSize() > CDU_CST_3))
	{
		MaxArray.SetAt(CDU_CST_3, Asc2Int(strMaxCntDataArray[CDU_CST_3]));
	}
	if ((nMaxCSTSize > CDU_CST_4) &&
		(strMaxCntDataArray.GetSize() > CDU_CST_4))
	{
		MaxArray.SetAt(CDU_CST_4, Asc2Int(strMaxCntDataArray[CDU_CST_4]));
	}
//////////////////////////////////////////////////////////////////////////
	if ((nMinCSTSize > CDU_CST_REJECT) &&						// 2004.11.05
		(strMinCntDataArray.GetSize() > CDU_CST_REJECT))
	{
		MinArray.SetAt(CDU_CST_REJECT, Asc2Int(strMinCntDataArray[CDU_CST_REJECT]));
	}
	if ((nMinCSTSize > CDU_CST_1) &&
		(strMinCntDataArray.GetSize() > CDU_CST_1))
	{
		MinArray.SetAt(CDU_CST_1, Asc2Int(strMinCntDataArray[CDU_CST_1]));
	}
	if ((nMinCSTSize > CDU_CST_2) &&
		(strMinCntDataArray.GetSize() > CDU_CST_2))
	{
		MinArray.SetAt(CDU_CST_2, Asc2Int(strMinCntDataArray[CDU_CST_2]));
	}
	if ((nMinCSTSize > CDU_CST_3) &&
		(strMinCntDataArray.GetSize() > CDU_CST_3))
	{
		MinArray.SetAt(CDU_CST_3, Asc2Int(strMinCntDataArray[CDU_CST_3]));
	}
	if ((nMinCSTSize > CDU_CST_4) &&
		(strMinCntDataArray.GetSize() > CDU_CST_4))
	{
		MinArray.SetAt(CDU_CST_4, Asc2Int(strMinCntDataArray[CDU_CST_4]));
	}
//////////////////////////////////////////////////////////////////////////
	m_pCdu.SetPhysicalMaximum(IntArrayToSafeArray(MaxArray));
	m_pCdu.SetUnitMinimumCount(IntArrayToSafeArray(MinArray));
	int nReturn = m_pCdu.EndExchangeSync();

	return nReturn;
}

// ----------------------------------------------------------------------------
// DESCRIPT  : 카세트별 권종설정
// PARAMETER : 카세트별 권종값
// RETURN    : 각 CST의 권종정보를 설정
// REMARK    : 2005.02.24 
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevCduSetValueOfCash(LPCTSTR szCashValueData)
{
	CString	strCashValueData(szCashValueData);					// 2005.02.24
	CStringArray strCashValueDataArray;

	SplitString(strCashValueData, ",", strCashValueDataArray);

	// 2007.11.26 V01.02.19
	// CDU 장애라도 DENOMINATION은 SET 가능하도록 수정(DENOMINATION 장애 보완)
//	if (DevCduGetDeviceStatus() != NORMAL)						// 장치비정상시 처리불가
//	{
//		ProcSetDeviceEvent(L"CDU", L"FatalError", NULL);
//		return 0;
//	}

	// 2007.11.27 V01.02.19
	// CURRENCY ID를 강제로 SET한 후 권종을 SET한다
	int nCSTSize	= 0;
	CStringArray SArray;
	VARIANT varTemp;

	VariantInit(&varTemp);
	SArray.RemoveAll();
	varTemp = m_pCdu.GetUnitCurrencyID();
	SArray.RemoveAll();
	nCSTSize = VarArrayToStringArray(&varTemp, SArray);

NHDEBUG(1, (_T("CNHMWICtrl::DevCduSetValueOfCash() CurrencyID Set \n")));

// [#514] [MX] KSK 2009.3.4 Mexico만 Multi Currency를 지원하도록 수정
	if(nCSTSize > CDU_CST_1)
		SArray.SetAt(CDU_CST_1, m_strCurrency);	// Set CurrecyID CST#1
	if(nCSTSize > CDU_CST_2)
		SArray.SetAt(CDU_CST_2, m_strCurrency);	// Set CurrecyID CST#2
	if(nCSTSize > CDU_CST_3)
		SArray.SetAt(CDU_CST_3, m_strCurrency);	// Set CurrecyID CST#3
	if(nCSTSize > CDU_CST_4)
		SArray.SetAt(CDU_CST_4, m_strCurrency);	// Set CurrecyID CST#4

	// 파라마터로 설정된 값으로 Currency ID 셋팅
	VariantClear(&varTemp);	// [#363] NH JSW 2008.07.15 추가
	varTemp = StringArrayToSafeArray(SArray);
	m_pCdu.SetUnitCurrencyID(varTemp);
//	VariantClear(&varTemp);	// [#363] NH JSW 2008.07.18 StringArrayToSafeArray 내부에서 Clear됨

NHDEBUG(1, (_T("CNHMWICtrl::DevCduSetValueOfCash() CurrencyID Set Complete \n")));
	//--------------------------------------------------------------

	CArray<int, int> IArray;

	IArray.RemoveAll();
	nCSTSize	= 0;
	//-- SJKWONNOTE 2006-06-17 --Modify : Memory leak 수정//
	// [#363] NH JSW 2008.07.21
	VARIANT varTemp2;
	VariantInit(&varTemp2);
	varTemp2 = m_pCdu.GetUnitValue();
	nCSTSize = VarArrayToIntArray(&varTemp2, IArray);
	VariantClear(&varTemp2);
	// end of [#363]
	//--

NHDEBUG(1, (_T("CNHMWICtrl::DevCduSetValueOfCash() UnitValue Set\n")));
	if ((nCSTSize > CDU_CST_1) &&								// 2005.02.24
		(strCashValueDataArray.GetSize() > CDU_CST_1))
	{
		IArray.SetAt(CDU_CST_1, Asc2Int(strCashValueDataArray[CDU_CST_1]));
	}
	if ((nCSTSize > CDU_CST_2) &&
		(strCashValueDataArray.GetSize() > CDU_CST_2))
	{
		IArray.SetAt(CDU_CST_2, Asc2Int(strCashValueDataArray[CDU_CST_2]));
	}
	if ((nCSTSize > CDU_CST_3) &&
		(strCashValueDataArray.GetSize() > CDU_CST_3))
	{
		IArray.SetAt(CDU_CST_3, Asc2Int(strCashValueDataArray[CDU_CST_3]));
	}
	if ((nCSTSize > CDU_CST_4) &&
		(strCashValueDataArray.GetSize() > CDU_CST_4))
	{
		IArray.SetAt(CDU_CST_4, Asc2Int(strCashValueDataArray[CDU_CST_4]));
	}
	m_pCdu.SetUnitValue(IntArrayToSafeArray(IArray));

	// [#587] NH KSK 2009.12.04 UnitType이 잘못된 경우 Denomination이 설정 안되는 장애 보완처리
	CStringArray saUnitTypes;
	saUnitTypes.Add("REJECTCASSETTE");
	for(int i = 0; i < nCSTSize - 1; i++)
	{
		saUnitTypes.Add("BILLCASSETTE");
	}
	m_pCdu.SetUnitType(StringArrayToSafeArray(saUnitTypes));
	// end of [#587]

	int nReturn = m_pCdu.EndExchangeSync();
NHDEBUG(1, (_T("CNHMWICtrl::DevCduSetValueOfCash() UnitValue Set Complete\n")));
NHDEBUG(1, (_T("CNHMWICtrl::EndExchangeSync() nResult : [%d] \n"), nReturn));
	return nReturn;
}

// ----------------------------------------------------------------------------
// DESCRIPT  : CDU Type구하기
// PARAMETER : 없음
// RETURN    : 
// ----------------------------------------------------------------------------
int	CNHMWICtrl::DevCduGetDispenserType()
{
	CStringArray	SArray;
	int				nCSTCount = 0;
	
	//-- SJKWONNOTE 2006-06-17 --Modify:VARIANT memory leak 수정//
	SArray.RemoveAll();
	VARIANT varTemp;
	VariantInit(&varTemp);
	varTemp = m_pCdu.GetUnitCurrencyID();
	nCSTCount = VarArrayToStringArray(&varTemp, SArray);
	VariantClear(&varTemp);																// 카세트별권종구하기
	//--

	if (!nCSTCount)												// 카세트없음
		return DISPENSER_NOT_SET;								// 출금부없음
	
	//SJK_42-- SJKWONNOTE 2006-10-26 -- Modify//
	if (nCSTCount >= 5)
		return DISPENSER_4CASH;									// 4CST
	else
	if (nCSTCount >= 4)
		return DISPENSER_3CASH;									// 3CST
	else
	if (nCSTCount >= 3)
		return DISPENSER_2CASH;									// 2CST
	else
	if (nCSTCount >= 2)
		return DISPENSER_1CASH;									// 1CST
	else
		return DISPENSER_NOT_SET;								// 출금부없음
	//SJK_42 end
}

// ----------------------------------------------------------------------------
// DESCRIPT  : CDU장탈착정보 조회
// PARAMETER : 
// RETURN    : REARDOOR 상태값조회
// REMARK    : CDU 정위치센서값을 Kal CashDispenser에서는 알려주지 않는다.
//             이 값을 REARDOOR쪽에서 관리한다.(이유는 CDU EP에서 관리하지 않는다나..)
// ----------------------------------------------------------------------------
CString CNHMWICtrl::DevCduGetStPosition()
{
	CString strReturn;
	strReturn = "NORMAL_POS";

	if (!m_pDoor)
		return strReturn;
	if (!m_bDoorOpened)
		return strReturn;

	if ((!m_pDoor.GetCanOpenShield())		||					// 2005.05.26
		(!m_pDoor.GetCanCloseShield()))
	{
		strReturn = "NORMAL_POS";
		return strReturn;
	}

	if (m_pDoor.GetShieldStatus().CompareNoCase(_T("OPEN")) == 0)	// 2005.05.26
		strReturn = "ABNORMAL_POS";
	else
		strReturn = "NORMAL_POS";

	return strReturn;
}

// ----------------------------------------------------------------------------
// DESCRIPT  : 카세트의 상태를 조회
// PARAMETER : 카세트번호(0~4, 0-Reject)
// RETURN    : 
// ----------------------------------------------------------------------------
CString CNHMWICtrl::DevCduGetCSTStatus(int nType)
{
	CString strReturn;

	switch(nType)
	{
		case CDU_CST_REJECT	:
			strReturn = m_pDevCtrl->GetDeviceStatus(_T("CDURejectCSTStatus"));
			break;
		case CDU_CST_1		: 
			strReturn = m_pDevCtrl->GetDeviceStatus(_T("CDUCST1STATUS"));
			break;
		case CDU_CST_2		: 
			strReturn = m_pDevCtrl->GetDeviceStatus(_T("CDUCST2STATUS"));
			break;
		case CDU_CST_3		: 
			strReturn = m_pDevCtrl->GetDeviceStatus(_T("CDUCST3STATUS"));
			break;
		case CDU_CST_4		: 
			strReturn = m_pDevCtrl->GetDeviceStatus(_T("CDUCST4STATUS"));
			break;
		default				: 
			return strReturn;
	}


/* -------------------------------------------------------------
// 속도개선 : 트레이스 최소화 및 속도 퍼포먼스를 개선하기 위해서
//            카세트 상태조회로직 변경처리함

	int	nCSTStatus	= CST_EMPTY;
	int nCSTSize	= 0;
	CStringArray SArray;

	strReturn = "CST_EMPTY";
	SArray.RemoveAll();

	switch(nType)
	{
		case CDU_CST_REJECT : 
		case CDU_CST_1		: 
		case CDU_CST_2		: 
		case CDU_CST_3		: 
		case CDU_CST_4		: 
			nCSTSize = VarArrayToStringArray(&(m_pCdu.GetCUStatus()), SArray);
			if (!nCSTSize)
				return strReturn;								// return CST_EMPTY
			if (nType >= nCSTSize)
				return strReturn;								// return CST_EMPTY
			strReturn = SArray.GetAt(nType);
			break;
		default				: 
			return strReturn;
	}

	strReturn.MakeUpper();
	if (nType == CDU_CST_REJECT)
	{
		if		(strReturn.CompareNoCase("HEALTHY"	) == 0)		strReturn = "CST_NORMAL";
		else if (strReturn.CompareNoCase("LOW"		) == 0)		strReturn = "CST_NEAR";
		else if (strReturn.CompareNoCase("EMPTY"	) == 0)		strReturn = "CST_EMPTY";
		else if (strReturn.CompareNoCase("MISSING"	) == 0)		strReturn = "CST_SET_NG";
		else if (strReturn.CompareNoCase("INOPERATIVE") == 0)	strReturn = "CST_FULL";
		else													strReturn = "CST_FULL";
	}
	else
	{
		if		(strReturn.CompareNoCase("HEALTHY"	) == 0)		strReturn = "CST_NORMAL";
		else if (strReturn.CompareNoCase("LOW"		) == 0)		strReturn = "CST_NEAR";
		else if (strReturn.CompareNoCase("EMPTY"	) == 0)		strReturn = "CST_EMPTY";
		else if (strReturn.CompareNoCase("MISSING"	) == 0)		strReturn = "CST_SET_NG";
		else if (strReturn.CompareNoCase("INOPERATIVE") == 0)	strReturn = "CST_EMPTY";
		else													strReturn = "CST_EMPTY";
	}
----------------------------------------------------------------*/
	return strReturn;
}

// ----------------------------------------------------------------------------
// DESCRIPT  : 매체잔류상태 조회
// PARAMETER : 
// RETURN    : 잔류상태
// REMARK    : StackerStatus를 확인하기 전에 다른 값을 확인해야 StackerStatus값이
//			   갱신되어 정상적인 값을 알 수 있다.
// ----------------------------------------------------------------------------
CString CNHMWICtrl::DevCduGetStMediaStatus()
{
	CString strReturn("");

	// StackerStatus상태값 갱신처리
//	CString strDeviceStatus		= m_pCdu.GetStDeviceStatus();	// 2004.11.05
//	CString strShutterStatus	= m_pCdu.GetStShutterStatus();

	CString strMediaStatus		= m_pCdu.GetPositionStatus();	// 
	strMediaStatus.MakeUpper();
	if		(strMediaStatus.CompareNoCase(_T("EMPTY")	) == 0)		strReturn = "0";
	else if (strMediaStatus.CompareNoCase(_T("NOTEMPTY")) == 0)		strReturn = "1";
	else if (strMediaStatus.CompareNoCase(_T("UNKNOWN")	) == 0)		strReturn = "1";
	else if (strMediaStatus.CompareNoCase(_T("NOTSUPP")	) == 0)		strReturn = "0";
	else															strReturn = "1";
																
	if (strReturn == "0")										// 2004.06.19
	{
		CString strTransportStatus	= m_pCdu.GetTransportStateStatus();
		strTransportStatus.MakeUpper();
		if		(strTransportStatus.CompareNoCase(_T("OK")		) == 0)		strReturn = "0";
		else if (strTransportStatus.CompareNoCase(_T("FATAL")	) == 0)		strReturn = "1";
		else if (strTransportStatus.CompareNoCase(_T("UNKNOWN")	) == 0)		strReturn = "1";
		else if (strTransportStatus.CompareNoCase(_T("NOTSUPP")	) == 0)		strReturn = "0";
		else																strReturn = "1";
	}
	
	return strReturn;
}

// ----------------------------------------------------------------------------
// DESCRIPT  : 카세트 상태조회
// PARAMETER : 
// RETURN    : 카세트상태조회값
// REMARK    : 1) 반드시 현금함의 상태만을 확인한다.
//             2) 개별카세트의 상태값을 따로 저장한다.
// ----------------------------------------------------------------------------
CString CNHMWICtrl::DevCduGetStRecycleBoxStatus()
{
	CString strReturn("");

	CString strCST1 = DevCduGetCSTStatus(CDU_CST_1);			// 개별카세트 상태조회
	CString strCST2 = DevCduGetCSTStatus(CDU_CST_2);			// 개별카세트 상태조회
	CString strCST3 = DevCduGetCSTStatus(CDU_CST_3);			// 개별카세트 상태조회
	CString strCST4 = DevCduGetCSTStatus(CDU_CST_4);			// 개별카세트 상태조회
	
	//int DispenserType = Asc2Int(m_pDevCtrl->GetDeviceStatus(_T("CDUDispenserType")).GetBuffer(0));	// AIREAT 20100211
	int DispenserType = Asc2Int(m_pDevCtrl->GetDeviceStatus(_T("CDUDispenserType")));

	switch (DispenserType)
	{
	case DISPENSER_NOT_SET :									// 출금함미설정
		strCST1 = L"CST_EMPTY";
		strCST2 = L"CST_EMPTY";
		strCST3 = L"CST_EMPTY";
		strCST4 = L"CST_EMPTY";

		strReturn = L"CST_EMPTY";
		break;

	case DISPENSER_4CASH :										// 4CST 
		if ((strCST1.CompareNoCase(_T("CST_NORMAL")) == 0)	||
			(strCST2.CompareNoCase(_T("CST_NORMAL")) == 0)	||
			(strCST3.CompareNoCase(_T("CST_NORMAL")) == 0)	||
			(strCST4.CompareNoCase(_T("CST_NORMAL")) == 0)	)
			strReturn = L"CST_NORMAL";
		else
		if ((strCST1.CompareNoCase(_T("CST_FULL")) == 0)	||
			(strCST2.CompareNoCase(_T("CST_FULL")) == 0)	||
			(strCST3.CompareNoCase(_T("CST_FULL")) == 0)	||
			(strCST4.CompareNoCase(_T("CST_FULL")) == 0)	)
			strReturn = L"CST_NORMAL";
		else
		if ((strCST1.CompareNoCase(_T("CST_NEAR")) == 0)	||
			(strCST2.CompareNoCase(_T("CST_NEAR")) == 0)	||
			(strCST3.CompareNoCase(_T("CST_NEAR")) == 0)	||
			(strCST4.CompareNoCase(_T("CST_NEAR")) == 0)	)
			strReturn = L"CST_NEAR";
		else
		if ((strCST1.CompareNoCase(_T("CST_SET_NG")) == 0)	&&
			(strCST2.CompareNoCase(_T("CST_SET_NG")) == 0)	&&
			(strCST3.CompareNoCase(_T("CST_SET_NG")) == 0)	&&
			(strCST4.CompareNoCase(_T("CST_SET_NG")) == 0)	)
			strReturn = L"CST_SET_NG";
		else
			strReturn = L"CST_EMPTY";

		break;

	case DISPENSER_3CASH :										// 3CST 
		strCST4 = L"CST_EMPTY";

		if ((strCST1.CompareNoCase(_T("CST_NORMAL")) == 0)	||
			(strCST2.CompareNoCase(_T("CST_NORMAL")) == 0)	||
			(strCST3.CompareNoCase(_T("CST_NORMAL")) == 0)	)
			strReturn = L"CST_NORMAL";
		else
		if ((strCST1.CompareNoCase(_T("CST_FULL")) == 0)	||
			(strCST2.CompareNoCase(_T("CST_FULL")) == 0)	||
			(strCST3.CompareNoCase(_T("CST_FULL")) == 0)	)
			strReturn = L"CST_NORMAL";
		else
		if ((strCST1.CompareNoCase(_T("CST_NEAR")) == 0)	||
			(strCST2.CompareNoCase(_T("CST_NEAR")) == 0)	||
			(strCST3.CompareNoCase(_T("CST_NEAR")) == 0)	)
			strReturn = L"CST_NEAR";
		else
		if ((strCST1.CompareNoCase(_T("CST_SET_NG")) == 0)	&&
			(strCST2.CompareNoCase(_T("CST_SET_NG")) == 0)	&&
			(strCST3.CompareNoCase(_T("CST_SET_NG")) == 0)	)
			strReturn = L"CST_SET_NG";
		else
			strReturn = L"CST_EMPTY";

		break;

	case DISPENSER_2CASH :										// 2CST 
		strCST3 = L"CST_EMPTY";
		strCST4 = L"CST_EMPTY";

		if ((strCST1.CompareNoCase(_T("CST_NORMAL")) == 0)	||
			(strCST2.CompareNoCase(_T("CST_NORMAL")) == 0)	)
			strReturn = L"CST_NORMAL";
		else
		if ((strCST1.CompareNoCase(_T("CST_FULL")) == 0)	||
			(strCST2.CompareNoCase(_T("CST_FULL")) == 0)	)
			strReturn = L"CST_NORMAL";
		else
		if ((strCST1.CompareNoCase(_T("CST_NEAR")) == 0)	||
			(strCST2.CompareNoCase(_T("CST_NEAR")) == 0)	)
			strReturn = L"CST_NEAR";
		else
		if ((strCST1.CompareNoCase(_T("CST_SET_NG")) == 0)	&&
			(strCST2.CompareNoCase(_T("CST_SET_NG")) == 0)	)
			strReturn = L"CST_SET_NG";
		else
			strReturn = L"CST_EMPTY";

		break;

	case DISPENSER_1CASH :										// 1CST 
		strCST2 = L"CST_EMPTY";
		strCST3 = L"CST_EMPTY";
		strCST4 = L"CST_EMPTY";

		if (strCST1.CompareNoCase(_T("CST_NORMAL")) == 0)	
			strReturn = L"CST_NORMAL";
		else
		if (strCST1.CompareNoCase(_T("CST_FULL")) == 0)
			strReturn = L"CST_NORMAL";
		else
		if (strCST1.CompareNoCase(_T("CST_NEAR")) == 0)	
			strReturn = L"CST_NEAR";
		else
		if (strCST1.CompareNoCase(_T("CST_SET_NG")) == 0)
			strReturn = L"CST_SET_NG";
		else
			strReturn = L"CST_EMPTY";

		break;

	default:
		strCST1 = L"CST_EMPTY";
		strCST2 = L"CST_EMPTY";
		strCST3 = L"CST_EMPTY";
		strCST4 = L"CST_EMPTY";

		strReturn = L"CST_EMPTY";
		break;
	}

	return strReturn;
}

// ----------------------------------------------------------------------------
// DESCRIPT  : 카세트 출금상태조회
// PARAMETER : 
// RETURN    : 
// ----------------------------------------------------------------------------
CString CNHMWICtrl::DevCduGetStAvailWithdraw()
{
	CString strReturn("WITH_NOT_AVAILABLE");
	CString strCSTR("CST_EMPTY");
	CString strCST1("CST_EMPTY");
	CString strCST2("CST_EMPTY");
	CString strCST3("CST_EMPTY");
	CString strCST4("CST_EMPTY");
	static CString strAvailWithSave("");

	// [#282] [NH] KSK 2008.6.11 국가별 권종 지원
	// TypeAvail setting하는 LOGIC 수정
	int		nCSTCount = DevCduGetCSTCount();
	int		Type1CashAvail	= FALSE;							// 제1권종 출금가능상태
	int		Type2CashAvail	= FALSE;							// 제2권종 출금가능상태
	
	strCSTR = DevCduGetCSTStatus(CDU_CST_REJECT);				// 회수카세트 상태조회
	switch(nCSTCount)
	{
	case 4 :		strCST4 = DevCduGetCSTStatus(CDU_CST_4);				// 개별카세트 상태조회
	case 3 :		strCST3 = DevCduGetCSTStatus(CDU_CST_3);				// 개별카세트 상태조회
	case 2 :		strCST2 = DevCduGetCSTStatus(CDU_CST_2);				// 개별카세트 상태조회
	case 1 :		strCST1 = DevCduGetCSTStatus(CDU_CST_1);	break;		// 개별카세트 상태조회
	default:													// 미정의 CST갯수
		return strReturn;										// 출금불가
	}

	if (DevCduGetStRecycleBoxStatus().CompareNoCase(_T("CST_EMPTY")) == 0)
	{
		strReturn = L"WITH_NOT_AVAILABLE";						// 출금불가
		return strReturn;
	}

	if ((strCSTR.CompareNoCase(_T("CST_FULL")) == 0)	||		// 회수함풀시출금불가
		(strCSTR.CompareNoCase(_T("CST_SET_NG")) == 0)	)		// 회수함탈착시출금불가	
	{
		strReturn = L"WITH_NOT_AVAILABLE";						// 출금불가
		return strReturn;
	}

	BYTE	TypeAvail = CST_WITH_NOT_AVAILABLE;
	int		CSTValue   = 0;
	int		nCashDeno[8];
	memset(nCashDeno, 0, sizeof(nCashDeno));

	// [#2115] [MX] KSK 2012.02.22
#if (MX_VERSION)
	CString strCurrencyID = DevCduGetCurrencyID();
	if (strCurrencyID == CURRENCY_TYPE)
	{
		// Mexico Denomination 지원
		nCashDeno[0] = CASH_DENOMINATION1;
		nCashDeno[1] = CASH_DENOMINATION2;
		nCashDeno[2] = CASH_DENOMINATION3;
		nCashDeno[3] = CASH_DENOMINATION4;
		nCashDeno[4] = CASH_DENOMINATION5;
		nCashDeno[5] = CASH_DENOMINATION6;
		nCashDeno[6] = CASH_DENOMINATION7;
		nCashDeno[7] = CASH_DENOMINATION8;
	}
	else
	{
		// USA Denomination 지원
		nCashDeno[0] = MULTI_CASH_DENOMINATION1;
		nCashDeno[1] = MULTI_CASH_DENOMINATION2;
		nCashDeno[2] = MULTI_CASH_DENOMINATION3;
		nCashDeno[3] = MULTI_CASH_DENOMINATION4;
		nCashDeno[4] = MULTI_CASH_DENOMINATION5;
		nCashDeno[5] = MULTI_CASH_DENOMINATION6;
		nCashDeno[6] = MULTI_CASH_DENOMINATION7;
		nCashDeno[7] = MULTI_CASH_DENOMINATION8;
	}
#else
	nCashDeno[0] = CASH_DENOMINATION1;
	nCashDeno[1] = CASH_DENOMINATION2;
	nCashDeno[2] = CASH_DENOMINATION3;
	nCashDeno[3] = CASH_DENOMINATION4;
	nCashDeno[4] = CASH_DENOMINATION5;
	nCashDeno[5] = CASH_DENOMINATION6;
	nCashDeno[6] = CASH_DENOMINATION7;
	nCashDeno[7] = CASH_DENOMINATION8;
#endif
	// end of [#2115]

	if ((strCST1.CompareNoCase(_T("CST_NORMAL")) == 0)	||		// 카세트상태 - 정상
		(strCST1.CompareNoCase(_T("CST_NEAR")) == 0)	)		// 카세트상태 - 잔량
	{															
		if (Asc2Int(m_pDevCtrl->GetDeviceStatus(_T("CDUCST1COUNT"))) > m_nCSTMinimum4Mix6)
		{														// 2004.11.05	
			CSTValue = DevCduGetValueOfCash(CDU_CST_1);

			for(int i=0; i<8; i++)
			{
				if (CSTValue == nCashDeno[i])
				{
					TypeAvail |= (DENOMINATION_BIT1<<i);
					break;
				}
			}
		}
	}

	if ((strCST2.CompareNoCase(_T("CST_NORMAL")) == 0)	||		// 카세트상태 - 정상
		(strCST2.CompareNoCase(_T("CST_NEAR")) == 0)	)		// 카세트상태 - 잔량	
	{
		if (Asc2Int(m_pDevCtrl->GetDeviceStatus(_T("CDUCST2COUNT"))) > m_nCSTMinimum4Mix6)
		{														// 2004.11.05
			CSTValue = DevCduGetValueOfCash(CDU_CST_2);
			for(int i=0; i<8; i++)
			{
				if (CSTValue == nCashDeno[i])
				{
					TypeAvail |= (DENOMINATION_BIT1<<i);
					break;
				}
			}
		}
	}

	if ((strCST3.CompareNoCase(_T("CST_NORMAL")) == 0)	||		// 카세트상태 - 정상
		(strCST3.CompareNoCase(_T("CST_NEAR")) == 0)	)		// 카세트상태 - 잔량	
	{
		if (Asc2Int(m_pDevCtrl->GetDeviceStatus(_T("CDUCST3COUNT"))) > m_nCSTMinimum4Mix6)
		{														// 2004.11.05
			CSTValue = DevCduGetValueOfCash(CDU_CST_3);
			for(int i=0; i<8; i++)
			{
				if (CSTValue == nCashDeno[i])
				{
					TypeAvail |= (DENOMINATION_BIT1<<i);
					break;
				}
			}
		}
	}

	if ((strCST4.CompareNoCase(_T("CST_NORMAL")) == 0)	||		// 카세트상태 - 정상
		(strCST4.CompareNoCase(_T("CST_NEAR")) == 0)	)		// 카세트상태 - 잔량	
	{
		if (Asc2Int(m_pDevCtrl->GetDeviceStatus(_T("CDUCST4COUNT"))) > m_nCSTMinimum4Mix6)
		{														// 2004.11.05
			CSTValue = DevCduGetValueOfCash(CDU_CST_4);
			for(int i=0; i<8; i++)
			{
				if (CSTValue == nCashDeno[i])
				{
					TypeAvail |= (DENOMINATION_BIT1<<i);
					break;
				}
			}
		}
	}
	// end of [#282]

	if (TypeAvail)
		strReturn.Format(_T("%d"), (int)TypeAvail);
	else
		strReturn = L"WITH_NOT_AVAILABLE";						// 출금불가

	if (strAvailWithSave.CompareNoCase(strReturn))
	{
		strAvailWithSave = strReturn;
	}

	return strReturn;
}

// ----------------------------------------------------------------------------
// DESCRIPT  : 카세트 갯수조회
// PARAMETER : 
// RETURN    : 각 CST의 권종값을 구해 그 ARRAY의 사이즈로 CST의 갯수를 짐작한다
// REMARK    : 2004.03.31 
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevCduGetCSTCount()
{
	if (DevCduGetDeviceStatus() == NODEVICE)					// 장치비정상시 조회불가 : 2004.06.29
		return 0;

	int nCSTCount = 0;

	//int nCDUType = Asc2Int(m_pDevCtrl->GetDeviceStatus(_T("CDUDispenserType")).GetBuffer(0));	AIREAT
	int nCDUType = Asc2Int(m_pDevCtrl->GetDeviceStatus(_T("CDUDispenserType")));
	switch(nCDUType)
	{
	case DISPENSER_4CASH : 
		nCSTCount = 4;
		break;

	case DISPENSER_3CASH : 
		nCSTCount = 3;
		break;

	case DISPENSER_2CASH : 
		nCSTCount = 2;
		break;
	
	case DISPENSER_1CASH : 
		nCSTCount = 1;
		break;

	default :
		nCSTCount = 0;
		break;
	}

	return nCSTCount;
}

// ----------------------------------------------------------------------------
// DESCRIPT  : 최종 출금액
// PARAMETER : 
// RETURN    : 각 CST의 최종 출금액을 계산하여 리턴한다.
// REMARK    : 2006.01.08 PJH
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevCduGetLastDispensedAmount()
{
//	CStringArray	SArray;
	int	nCSTCount = 0;
	int nCstCnt1 = 0;
	int nCstCnt2 = 0;
	int nCstCnt3 = 0;
	int nCstCnt4 = 0;

	int nCstDeno1 = 0;
	int nCstDeno2 = 0;
	int nCstDeno3 = 0;
	int nCstDeno4 = 0;
	int nTotalAmt = 0;
		
	nCstDeno1 = DevCduGetValueOfCash(CDU_CST_1);
	nCstDeno2 = DevCduGetValueOfCash(CDU_CST_2);
	nCstDeno3 = DevCduGetValueOfCash(CDU_CST_3);
	nCstDeno4 = DevCduGetValueOfCash(CDU_CST_4);

	VARIANT varTemp;
	VariantInit(&varTemp);
	varTemp = m_pCdu.GetLastDispenseDenomination();
	CArray<int, int> IArray;									
	IArray.RemoveAll();
	nCSTCount = VarArrayToIntArray(&varTemp, IArray);
	VariantClear(&varTemp);
																// 카세트별 최종 출금액
	if (!nCSTCount)												// 카세트없음
		return DISPENSER_NOT_SET;								// 출금부없음

	if (nCSTCount >= 5)
	{
//		nCstCnt1 = Asc2Int(SArray[1]);
//		nCstCnt2 = Asc2Int(SArray[2]);
//		nCstCnt3 = Asc2Int(SArray[3]);
//		nCstCnt4 = Asc2Int(SArray[4]);
		nCstCnt1 = IArray.GetAt(1);
		nCstCnt2 = IArray.GetAt(2);
		nCstCnt3 = IArray.GetAt(3);
		nCstCnt4 = IArray.GetAt(4);
	}
	else
	if (nCSTCount >= 4)
	{
//		nCstCnt1 = Asc2Int(SArray[1]);
//		nCstCnt2 = Asc2Int(SArray[2]);
//		nCstCnt3 = Asc2Int(SArray[3]);
		nCstCnt1 = IArray.GetAt(1);
		nCstCnt2 = IArray.GetAt(2);
		nCstCnt3 = IArray.GetAt(3);
	}
	else
	if (nCSTCount >= 3)
	{
//		nCstCnt1 = Asc2Int(SArray[1]);
//		nCstCnt2 = Asc2Int(SArray[2]);
		nCstCnt1 = IArray.GetAt(1);
		nCstCnt2 = IArray.GetAt(2);
	}
	else
	if (nCSTCount >= 2)
//		nCstCnt1 = Asc2Int(SArray[1]);
		nCstCnt1 = IArray.GetAt(1);
	else
		return 0;
//--

	//int nCDUType = Asc2Int(m_pDevCtrl->GetDeviceStatus(_T("CDUDispenserType")).GetBuffer(0));	// AIREAT 20100211
	int nCDUType = Asc2Int(m_pDevCtrl->GetDeviceStatus(_T("CDUDispenserType")));

	switch(nCDUType)
	{
	case DISPENSER_4CASH :

		nTotalAmt = nCstCnt1 * nCstDeno1+
					nCstCnt2 * nCstDeno2+
					nCstCnt3 * nCstDeno3+
					nCstCnt4 * nCstDeno4;
		break;

	case DISPENSER_3CASH : 

		nTotalAmt = nCstCnt1 * nCstDeno1+
					nCstCnt2 * nCstDeno2+
					nCstCnt3 * nCstDeno3;
		break;

	case DISPENSER_2CASH : 

		nTotalAmt = nCstCnt1 * nCstDeno1+
					nCstCnt2 * nCstDeno2;
		break;
	
	case DISPENSER_1CASH : 

		nTotalAmt = nCstCnt1 * nCstDeno1;
		break;

	default :
		break;
	}

	return nTotalAmt;
}
// ----------------------------------------------------------------------------
// DESCRIPT  : 카세트별 출금 매수
// PARAMETER : 
// RETURN    : 각 CST의 출금 매수를 리턴한다.
// REMARK    : 2006.02.26 PJH
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevCduGetLastDispensedCount(int nCstNum)
{
//	CStringArray	SArray;
	int	nCSTCount = 0;
	int nCstCnt1 = 0;
	int nCstCnt2 = 0;
	int nCstCnt3 = 0;
	int nCstCnt4 = 0;
	int nReturnCount = 0;
	
//-- SJKWONNOTE 2006-06-17 --Modify:VARIANT memory leak 수정, SArray->IArray로 변경//
//	SArray.RemoveAll();
//	nCSTCount = VarArrayToStringArray(&(m_pCdu.GetLastDispenseDenomination()), SArray);

	VARIANT varTemp;
	VariantInit(&varTemp);
	varTemp = m_pCdu.GetLastDispenseDenomination();
	CArray<int, int> IArray;									
	IArray.RemoveAll();
	nCSTCount = VarArrayToIntArray(&varTemp, IArray);
	VariantClear(&varTemp);

	if (!nCSTCount)												// 카세트없음
		return DISPENSER_NOT_SET;								// 출금부없음

	if (nCSTCount >= 5)
	{
		nCstCnt1 = IArray.GetAt(1);
		nCstCnt2 = IArray.GetAt(2);
		nCstCnt3 = IArray.GetAt(3);
		nCstCnt4 = IArray.GetAt(4);
	}
	else
	if (nCSTCount >= 4)
	{
		nCstCnt1 = IArray.GetAt(1);
		nCstCnt2 = IArray.GetAt(2);
		nCstCnt3 = IArray.GetAt(3);
	}
	else
	if (nCSTCount >= 3)
	{
		nCstCnt1 = IArray.GetAt(1);
		nCstCnt2 = IArray.GetAt(2);
	}
	else
	if (nCSTCount >= 2)
		nCstCnt1 = IArray.GetAt(1);
	else
		return 0;
	
//--

	if (nCstNum == DISPENSER_1CASH)
		nReturnCount = nCstCnt1;
	else
	if (nCstNum == DISPENSER_2CASH)
		nReturnCount = nCstCnt2;
	else
	if (nCstNum == DISPENSER_3CASH)
		nReturnCount = nCstCnt3;
	else
	if (nCstNum == DISPENSER_4CASH)
		nReturnCount = nCstCnt4;

	return nReturnCount;
}

// ----------------------------------------------------------------------------
// DESCRIPT  : Shutter 존재 유무 Property Check
// PARAMETER : 
// RETURN    : Shutter가 있으면 TRUE, 없으면 FALSE가 리턴됨.
// REMARK    : 2006.02.08 PJH
// ----------------------------------------------------------------------------
BOOL CNHMWICtrl::DevCduGetShutterStatus()
{
	return m_pCdu.GetHasShutter();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Event : Accepted From CDU
///////////////////////////////////////////////////////////////////////////////////////////////////
void CNHMWICtrl::OnFatalErrorNxCashDispenser(LPCTSTR Action, long Result) 
{
	m_nCduDispenseAmount = 0;
	m_bCduWaitTaken = FALSE;									// 20030814_1.0_1066 : Pre-CancelWaitTaken Proc.
//	VariantInit(&m_vMixResult);
	VariantClear(&m_vMixResult);		// [#363] NH JSW 2008.07.14

	ProcSetDeviceEvent(L"CDU", L"FatalError", NULL);
	DevCduUpdateStatus();										// 모든 상태값을 업데이트한다.

	// [#2306] NH KSK MWI Add NVRAM Log
	m_strTemp.Empty();
	m_strTemp.Format(L"Result:%x", Result);
	NVDump('F', '1', "94", L"9999999", L"FATAL_ERR");
	NVDump('F', '1', "94", L"9999999", m_strTemp);
	// end of [#2306]

}

void CNHMWICtrl::OnDeviceErrorNxCashDispenser(LPCTSTR Action, long Result) 
{
	m_nCduDispenseAmount = 0;
	m_bCduWaitTaken = FALSE;									// 20030814_1.0_1066 : Pre-CancelWaitTaken Proc.
//	VariantInit(&m_vMixResult);
	VariantClear(&m_vMixResult);		// [#363] NH JSW 2008.07.14

	ProcSetDeviceEvent(L"CDU", L"DeviceError", NULL);
	DevCduUpdateStatus();

	// [#2306] NH KSK MWI Add NVRAM Log
	m_strTemp.Empty();
	m_strTemp.Format(L"Result:%x", Result);
	NVDump('F', '1', "94", L"9999999", L"DEV_ERR");
	NVDump('F', '1', "94", L"9999999", m_strTemp);
	// end of [#2306]
}

void CNHMWICtrl::OnLockGrantedNxCashDispenser() 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnLockReleasedNxCashDispenser() 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnLockTimeoutNxCashDispenser() 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnTimeoutNxCashDispenser() 
{
	m_bCduWaitTaken = FALSE;									// 20030814_1.0_1066 : Pre-CancelWaitTaken Proc.
//	VariantInit(&m_vMixResult);
	VariantClear(&m_vMixResult);		// [#363] NH JSW 2008.07.14
	ProcSetDeviceEvent(L"CDU", L"Timeout", NULL);
	DevCduUpdateStatus();
}

void CNHMWICtrl::OnDenominateCompleteNxCashDispenser(const VARIANT FAR& Mix) 
{
	
	ProcSetDeviceEvent(L"CDU", L"DenominateComplete", NULL);
	VariantClear(&m_vMixResult);		// [#363] NH JSW 2008.07.14 추가
	VariantCopy(&m_vMixResult, &(VARIANT)Mix);
	VariantClear(&(VARIANT)Mix);		// SJK
}

void CNHMWICtrl::OnNotDispensableNxCashDispenser() 
{
	CString strErrorCode("");
	strErrorCode = L"9792401";									// 2005.03.08

	// PJH 2006.1.6
	// 장애가 발생하였지만, 정상으로 리턴함. FAST CASH 금액 검사용으로 사용함.
	if (!m_GetAvailableAmount)
		m_pDevCtrl->WriteMwiErrorCode(DEV_CDU, "CDM", strErrorCode.GetLength(), "9792401");

	m_nCduDispenseAmount = 0;
	m_bCduWaitTaken = FALSE;									// 20030814_1.0_1066 : Pre-CancelWaitTaken Proc.
//	VariantInit(&m_vMixResult);
	VariantClear(&m_vMixResult);		// [#363] NH JSW 2008.07.14

	// PJH 2006.1.6
	// 장애가 발생하였지만, 정상으로 리턴함. FAST CASH 금액 검사용으로 사용함.
	// DISPENSE 이전 DENOMINATION 명령이면, 장애로 리턴함.
	if (!m_GetAvailableAmount)
		ProcSetDeviceEvent(L"CDU", L"DeviceError", NULL);
	else
		ProcSetDeviceEvent(L"CDU", L"DenominateComplete", NULL);

	DevCduUpdateStatus();

	NVDump('O', '1', "94", L"", L"NOT_DISPENSE");	// [#2306] NH KSK MWI Add NVRAM Log
}

void CNHMWICtrl::OnNoteErrorNxCashDispenser(LPCTSTR Reason) 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnInCompleteDispenseNxCashDispenser() 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnDispenseCompleteNxCashDispenser() 
{
	CString strDeviceStatus		= m_pCdu.GetDeviceStatus();

	if (strDeviceStatus.CompareNoCase(ST_DEVONLINE) != 0)		// 2004.08.30
	{
		m_nCduDispenseAmount		= 0;
		m_bCduWaitTaken = FALSE;								// 20030814_1.0_1066 : Pre-CancelWaitTaken Proc.
//		VariantInit(&m_vMixResult);
		VariantClear(&m_vMixResult);		// [#363] NH JSW 2008.07.14

		ProcSetDeviceEvent(L"CDU", L"DeviceError", NULL);
		DevCduUpdateStatus();									// 모든 상태값을 업데이트한다.
	}
	else
	{
		ProcSetDeviceEvent(L"CDU", L"DispenseComplete", NULL);
	}
}

void CNHMWICtrl::OnCountCompleteNxCashDispenser() 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnItemsPresentedNxCashDispenser() 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnPresentCompleteNxCashDispenser() 
{
	ProcSetDeviceEvent(L"CDU", L"PresentComplete", NULL);
	DevCduUpdateStatus();
}

void CNHMWICtrl::OnItemsTakenNxCashDispenser() 
{
	m_bCduWaitTaken = FALSE;									// 20030814_1.0_1066 : Pre-CancelWaitTaken Proc.

	// Lifted Event가 생략될때를 대비하여 한번더 임의로 발생시켜 준다.
//y	ProcSetDeviceEvent(DEVNM_CDU, "CashLifted", "");
	
	ProcSetDeviceEvent(L"CDU", L"ItemsTaken", NULL);
	DevCduUpdateStatus();										// 모든 상태값을 업데이트한다.
}

void CNHMWICtrl::OnRetractCompleteNxCashDispenser() 
{
	m_bCduWaitTaken = FALSE;									// 20030814_1.0_1066 : Pre-CancelWaitTaken Proc.
	ProcSetDeviceEvent(L"CDU", L"RetractComplete", NULL);
	DevCduUpdateStatus();										// 장치 상태정보를 갱신한다
}

void CNHMWICtrl::OnRejectCompleteNxCashDispenser() 
{
	ProcSetDeviceEvent(L"CDU", L"RejectComplete", NULL);
	DevCduUpdateStatus();										// 장치 상태정보를 갱신한다
}

void CNHMWICtrl::OnOpenShutterCompleteNxCashDispenser() 
{
	m_bCduWaitTaken = FALSE;									// 20030814_1.0_1066 : Pre-CancelWaitTaken Proc.
	ProcSetDeviceEvent(L"CDU", L"OpenShutterComplete", NULL);
	DevCduUpdateStatus();										// 장치 상태정보를 갱신한다
}

void CNHMWICtrl::OnCloseShutterCompleteNxCashDispenser() 
{
	m_bCduWaitTaken = FALSE;									// 20030814_1.0_1066 : Pre-CancelWaitTaken Proc.
	ProcSetDeviceEvent(L"CDU", L"CloseShutterComplete", NULL);
	DevCduUpdateStatus();										// 장치 상태정보를 갱신한다
}

void CNHMWICtrl::OnCashUnitErrorNxCashDispenser(short UnitNumber) 
{
	DevCduUpdateStatus();										// 장치 상태정보를 갱신한다
	// CITI07-120 KJK 2007.04.30
//	ProcSetDeviceEvent(L"CDU", L"CashUnitError", NULL); //2008-01-14 V01.02.25
}

void CNHMWICtrl::OnOpenSafeDoorCompleteNxCashDispenser() 
{
	ProcSetDeviceEvent(L"CDU", L"OpenSafeDoorComplete", NULL);
	DevCduUpdateStatus();										// 장치 상태정보를 갱신한다
}

void CNHMWICtrl::OnCalibrateCompleteNxCashDispenser() 
{
	ProcSetDeviceEvent(L"CDU", L"CalibrateComplete", NULL);
	DevCduUpdateStatus();										// 장치 상태정보를 갱신한다
}

void CNHMWICtrl::OnCashUnitThresholdNxCashDispenser(short UnitNumber) 
{
	ProcSetDeviceEvent(L"CDU", L"CashUnitThreshold", NULL);
	DevCduUpdateStatus();										// 장치 상태정보를 갱신한다
}

void CNHMWICtrl::OnCashUnitChangedNxCashDispenser(short UnitNumber) 
{
	ProcSetDeviceEvent(L"CDU", L"CashUnitChanged", NULL);
	DevCduUpdateStatus();										// 장치 상태정보를 갱신한다
}

void CNHMWICtrl::OnResetCompleteNxCashDispenser() 
{
	ProcSetDeviceEvent(L"CDU", L"ResetComplete", NULL);
	DevCduUpdateStatus();										// 장치 상태정보를 갱신한다
}

void CNHMWICtrl::OnMediaDetectedNxCashDispenser(short UnitNumber) 
{
//	ProcSetDeviceEvent(DEVNM_CDU, "MediaDetected", "");			// 2006.10.08
	DevCduUpdateStatus();										// 장치 상태정보를 갱신한다
}

void CNHMWICtrl::OnDeviceStatusChangedNxCashDispenser(LPCTSTR Value) 
{
	DevCduUpdateStatus();

	// [#2306] NH KSK MWI Add NVRAM Log
	m_strTemp.Empty();
	m_strTemp.Format(L"Result:%s", Value);
	NVDump('O', '1', "94", L"", L"DEV_C_STS");
	NVDump('O', '1', "94", L"", m_strTemp);
	// end of [#2306]
}

void CNHMWICtrl::OnSafeDoorStatusChangedNxCashDispenser(LPCTSTR Value) 
{
	DevCduUpdateStatus();
}

void CNHMWICtrl::OnDispenserStatusChangedNxCashDispenser(LPCTSTR Value) 
{
	DevCduUpdateStatus();

	// [#2306] NH KSK MWI Add NVRAM Log
	m_strTemp.Empty();
	m_strTemp.Format(L"Result:%s", Value);
	NVDump('O', '1', "94", L"", L"DISPENSE_C_STS");
	NVDump('O', '1', "94", L"", m_strTemp);
	// end of [#2306]
}

void CNHMWICtrl::OnStackerStatusChangedNxCashDispenser(LPCTSTR Value) 
{
	DevCduUpdateStatus();
}

void CNHMWICtrl::OnShutterStatusChangedNxCashDispenser(LPCTSTR Value) 
{
	DevCduUpdateStatus();
}

void CNHMWICtrl::OnTransportStatusChangedNxCashDispenser(LPCTSTR Value) 
{
	DevCduUpdateStatus();
}

void CNHMWICtrl::OnTransportStateStatusChangedNxCashDispenser(LPCTSTR Value) 
{
	DevCduUpdateStatus();
}

void CNHMWICtrl::OnPositionStatusChangedNxCashDispenser(LPCTSTR Value) 
{
	DevCduUpdateStatus();
}

void CNHMWICtrl::OnCountChangedNxCashDispenser() 
{
	DevCduUpdateStatus();
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Method : Call From CDU to CDU
///////////////////////////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// PROTOTYPE : int DevCduPreCancelWaitTaken()
// DESCRIPT  : 매체수취대기 FLAG를 확인하여 대기중이면 수취대기를 취소한다
// PARAMETER : 없음
// RETURN    : 0
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevCduPreCancelWaitTaken()
{
	if (m_bCduWaitTaken)
	{
//		SetEventMatrix(DEV_CDU, "CancelWaitForCashTaken");
		DevCduCancelWaitTaken();
//		CheckDeviceAction(DEV_CDU, K_30_WAIT);
	}
	m_bCduWaitTaken = FALSE;
	return R_NORMAL;
}

// ----------------------------------------------------------------------------
// DESCRIPT  : 
// PARAMETER : 디바이스 상태와 매체 상태를 업데이트 한다.
// RETURN    : 
// REMARK	 : 정위치센서정보는 REARDOOR부에서도 처리를 하여야 한다.
//			   현금부(CDU)정위치 센서가 DOOR부에 연결되어 있다.
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevCduUpdateStatus()
{
																// AP DOWN 장애현상 디버그 : 2003.12.20
	if (!m_bCduOpened)											// 장치가Open되지 않았으면
		return R_NORMAL;										// 상태갱신은 의미없음
	
	CString strStatus = m_pCdu.GetDeviceStatus();				// 장치상태조회 : 장치가 정상일경우에만 기타조회
	ProcSetDeviceStatus(L"CDU", L"StDeviceStatus", strStatus);

	//SJK_41-- SJKWONNOTE 2006-10-26 --//
	//if (strStatus.CompareNoCase(ST_NODEVICE) != 0)				// 2004.11.05 FATAL상태에서도 UPDATE처리 : 2004.06.29
	if (strStatus.CompareNoCase(ST_DEVNODEVICE) != 0)
	{
		ProcSetDeviceStatus(L"CDU", L"Position", DevCduGetStPosition());
		ProcSetDeviceStatus(L"CDU", L"DispenserType", Int2Asc(DevCduGetDispenserType()));
		ProcSetDeviceStatus(L"CDU", L"CSTCount", Int2Asc(DevCduGetCSTCount()));
		DevCduUpdateCstStatus();								// CST갱신처리
		ProcSetDeviceStatus(L"CDU", L"RecycleBoxStatus", DevCduGetStRecycleBoxStatus());
		ProcSetDeviceStatus(L"CDU", L"AvailWithdraw", DevCduGetStAvailWithdraw());
		ProcSetDeviceStatus(L"CDU", L"MaterialInfo", DevCduGetStMediaStatus());
	}

	return R_NORMAL;
}

// ----------------------------------------------------------------------------
// DESCRIPT  : 
// PARAMETER : 
// RETURN    : 
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevCduUpdateCstStatus()
{
	CString		strReturn("");
	int	nCSTStatus	= CST_EMPTY;
	int nCSTSTatusSize	= 0;
	CStringArray SArray;

	strReturn = "CST_EMPTY";
	SArray.RemoveAll();

	int			nReturn = 0;
	CArray<int, int> IArray;									// 2004.11.05
	IArray.RemoveAll();
	int nCSTCountSize	= 0;

	//-- SJKWONNOTE 2006-06-17 --Modify : Memory leak 수정//
	VARIANT varTemp;
	VariantInit(&varTemp);
	varTemp = m_pCdu.GetPhysicalStatus();
	nCSTSTatusSize = VarArrayToStringArray(&varTemp, SArray);
	VariantClear(&varTemp);

//	VariantInit(&varTemp);	// [#363] NH JSW 2008.07.15 불필요
	varTemp = m_pCdu.GetPhysicalCount();
	nCSTCountSize = VarArrayToIntArray(&varTemp, IArray);
	VariantClear(&varTemp);
	//--

	if (nCSTSTatusSize)											// 2004.11.05
	{
		for (int i=0;i<nCSTSTatusSize;i++)
		{
			strReturn = SArray.GetAt(i);
			strReturn.MakeUpper();
			if (i == CDU_CST_REJECT)
			{
				if		(strReturn.CompareNoCase(_T("OK")	) == 0)			strReturn = L"CST_NORMAL";
				else if (strReturn.CompareNoCase(_T("LOW")		) == 0)		strReturn = L"CST_NEAR";
				else if (strReturn.CompareNoCase(_T("EMPTY")	) == 0)		strReturn = L"CST_EMPTY";
				else if (strReturn.CompareNoCase(_T("MISSING")	) == 0)		strReturn = L"CST_SET_NG";
				else if (strReturn.CompareNoCase(_T("INOPERATIVE")) == 0)	strReturn = L"CST_FULL";
				else													strReturn = L"CST_FULL";
			}
			else
			{
				if		(strReturn.CompareNoCase(_T("OK")	) == 0)			strReturn = L"CST_NORMAL";
				else if (strReturn.CompareNoCase(_T("LOW")		) == 0)		strReturn = L"CST_NEAR";
				else if (strReturn.CompareNoCase(_T("EMPTY")	) == 0)		strReturn = L"CST_EMPTY";
				else if (strReturn.CompareNoCase(_T("MISSING")	) == 0)		strReturn = L"CST_SET_NG";
				else if (strReturn.CompareNoCase(_T("INOPERATIVE")) == 0)	strReturn = L"CST_EMPTY";
				else													strReturn = L"CST_EMPTY";
			}

			switch(i)
			{
			case 0:												// 개별카세트 상태저장
				ProcSetDeviceStatus(L"CDU", L"RejectCSTStatus", strReturn);	
				break;	
			case 1:
				ProcSetDeviceStatus(L"CDU", L"CST1STATUS", strReturn);
				break;
			case 2:
				ProcSetDeviceStatus(L"CDU", L"CST2STATUS", strReturn);
				break;
			case 3:
				ProcSetDeviceStatus(L"CDU", L"CST3STATUS", strReturn);
				break;
			case 4:
				ProcSetDeviceStatus(L"CDU", L"CST4STATUS", strReturn);
				break;
			default:
				break;
			}
		}
	}

	if (nCSTCountSize)											// 2004.11.05
	{
		for (int i=0;i<nCSTCountSize;i++)
		{
			nReturn = IArray.GetAt(i);
			switch(i)
			{
			case 0:												// 개별카세트 상태저장
				ProcSetDeviceStatus(L"CDU", L"RejectCSTCount", Int2Asc(nReturn));	
				break;	
			case 1:
				ProcSetDeviceStatus(L"CDU", L"CST1COUNT", Int2Asc(nReturn));
				break;
			case 2:
				ProcSetDeviceStatus(L"CDU", L"CST2COUNT", Int2Asc(nReturn));
				break;
			case 3:
				ProcSetDeviceStatus(L"CDU", L"CST3COUNT", Int2Asc(nReturn));
				break;
			case 4:
				ProcSetDeviceStatus(L"CDU", L"CST4COUNT", Int2Asc(nReturn));
				break;
			default:
				break;
			}
		}
	}

	return 0;
}
// 2007.11.23 V01.02.19
// ----------------------------------------------------------------------------
// DESCRIPT : 현금부의 국가정보 조회 2007.10.22
// ----------------------------------------------------------------------------
CString CNHMWICtrl::DevCduGetCurrencyID()
{
	if (DevCduGetDeviceStatus() == NODEVICE)
		return L"UNKNOWN";

	CStringArray	SArray;
	int				nCSTCount = 0;

	SArray.RemoveAll();
	VARIANT varTemp;
	VariantInit(&varTemp);
	varTemp = m_pCdu.GetUnitCurrencyID();
	nCSTCount = VarArrayToStringArray(&varTemp, SArray);

	VariantClear(&varTemp);
	if(nCSTCount < 2)	// RJ CST 제외
		return L"UNKNOWN";

	return SArray.GetAt(CDU_CST_1);	// 무조건 첫번째 카세트만 참조하기로 함(유승현)
}
