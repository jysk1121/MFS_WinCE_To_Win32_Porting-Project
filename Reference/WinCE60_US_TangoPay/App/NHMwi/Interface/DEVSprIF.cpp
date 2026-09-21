// DEVSPRIF.cpp

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
// Kalignite SPR ActiveX BUGREPORT
// ------------------------------------------------------------------------------------------------
// 1. 2003.06.09
//		-	용지 정지위치 관련하여 Media/paperStatus에 정보 필요
//			보충롤이 있을경우에 관련된 처리
// 2. 2003.06.09
//		-	Autoload실패시 장애처리에 관한건
// ------------------------------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// Method : Call to MCU
///////////////////////////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// 함 수 명 : int CNHMWICtrl::DevSprOpenConnection()
// DESCRIPT : 세션연결 시작
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevSprOpenConnection()
{
	NHDEBUG(1, (_T("OpenSessionSync() ...\n")));
	int nReturn = m_pSpr.OpenSessionSync(K_30_WAIT*1000);
	NHDEBUG(1, (L"\n"));
	NHDEBUG(1, (_T("OpenSessionSync() ... DONE (%d)\n"), nReturn));

	if (nReturn != R_NORMAL)
	{
		CString strErrorCode = "9792200";
		m_pDevCtrl->WriteMwiErrorCode(DEV_SPR, "SPR", strErrorCode.GetLength(), "9792200");
		ProcSetDeviceEvent(L"SPR", L"FatalError", NULL);
		return R_ERROR;
	}
	else
	{
		// This is to remove a fatal error generated after successful OpenSessionSync(), which was executed to open SP which was killed by force by GOMA.
		m_pDevCtrl->EvtQReset(DEV_SPR);

		ProcSetDeviceEvent(L"SPR", L"OpenComplete", NULL);
	}
	
	DevSprUpdateStatus();

	return nReturn;
}

// ----------------------------------------------------------------------------
// 함 수 명 : int CNHMWICtrl::DevSprCloseConnection()
// DESCRIPT : 세션종료
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevSprCloseConnection()
{
	m_pSpr.CloseSessionSync();
	return R_NORMAL;	
}

// ----------------------------------------------------------------------------
// 함 수 명 : int CNHMWICtrl::DevSprReset()
// DESCRIPT : 세션종료
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevSprReset()
{
	m_pSpr.Reset(_T("EJECT"), 0);
	return R_NORMAL;	
}

// ----------------------------------------------------------------------------
// 함 수 명 : int CNHMWICtrl::DevSprGetStatus(int nStatus)
// DESCRIPT : 상태조회(장치, 매체, 용지등)
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevSprGetStatus(int nStatus)
{
	int nResult = R_NORMAL;
	CString strStatus("");

	switch(nStatus)
	{
		case ST_DEVICEVSTATUS  :
			strStatus = m_pDevCtrl->GetDeviceStatus(_T("SPRStDeviceStatus"));
			if		(strStatus.CompareNoCase(ST_DEVONLINE)	== 0)	nResult = NORMAL;
			else if	(strStatus.CompareNoCase(ST_DEVNODEVICE)== 0)	nResult = NODEVICE;
			else	nResult = DOWN;
			break;
		case ST_MEDIASTATUS  :
			// 명세표부는 용지가 검지되면 어느위치건간에 SENSOR1으로 설정
			strStatus = m_pDevCtrl->GetDeviceStatus(_T("SPRStMediaStatus"));
			if		(strStatus.CompareNoCase(ST_NOTPRESENT) == 0)	nResult = ST_NOT_DETECT;
			else if (strStatus.CompareNoCase(ST_PRESENT)	== 0)	nResult = ST_SENSOR2;
			else if (strStatus.CompareNoCase(ST_ENTERING)	== 0)	nResult = ST_SENSOR1;
			else	nResult = ST_NOT_DETECT;					// 20030729_1.0_1055 : MaterialInfo
			break;
		case ST_PAPERSTATUS :
			strStatus = m_pDevCtrl->GetDeviceStatus(_T("SPRStPaperStatus"));
//			RETAILMSG(1,(L"MWI SPR PAPER STATUS (%s)", strStatus));	// test
			if		(strStatus.CompareNoCase(ST_PAPER_FULL) == 0)	nResult = SLIP_NORMAL;
			else if (strStatus.CompareNoCase(ST_PAPER_LOW)	== 0)	nResult = SLIP_LOW_END;
			else if (strStatus.CompareNoCase(ST_PAPER_OUT)	== 0)	nResult = SLIP_EMPTY_PAPER;
			else	nResult = SLIP_UNKNOWN;
		default :
			break;
	}

	return nResult;
}

// ----------------------------------------------------------------------------
// 함 수 명 : int CNHMWICtrl::DevSprPrint(LPCTSTR szData)
// DESCRIPT : 명세표를 인자한다
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevSprPrint(LPCTSTR szData)
{
	CStringArray	arrPrintLines;
	CString			strPrintData, strTemp;
	int				nRequestLines, nPrintedLines;
	int				nIndex, nStartIndex, nEndIndex, nNextIndex, nCount;
	int				nResult = WFS_SUCCESS;

	SplitString(szData, FIELD_DELIMITER, arrPrintLines);
	nRequestLines = arrPrintLines.GetSize();
	nPrintedLines = 0;
	nNextIndex = 0;

	while (nPrintedLines < nRequestLines)
	{
		strPrintData = L"";

		// Make Print Data
		nStartIndex = nNextIndex;
		nEndIndex = nNextIndex + __min((nRequestLines-nPrintedLines), SLIP_MAX_LINE);
		for (nIndex = nStartIndex, nCount=1; nIndex < nEndIndex; nIndex++, nCount++)
		{
			if (arrPrintLines[nIndex].GetLength() > 0)
			{
				strTemp.Format(L"Line%d=%s", nCount, arrPrintLines[nIndex].Left(SLIP_MAX_COL));
				strTemp.Replace(L",", L",,");
			}
			else
				strTemp.Format(L"Line%d= ", nCount);

			// Add Data
			if (nCount > 1)
				strPrintData += L",";
			strPrintData += strTemp;

			nNextIndex = nIndex + 1;
			nPrintedLines++;
		}

		// Send Print Data
		if (strPrintData.GetLength() > 0)
		{
			m_pDevCtrl->SetEventMatrix(DEV_SPR, "PrintForm");

			nResult = m_pSpr.PrintForm(FORM_RECEIPT, MEDIA_RECEIPT, _T("USEFORMDEFN"), 0, 0, _T("MEDIUM"), strPrintData, K_30_WAIT*1000, _T("ANY"));
			if (nResult != WFS_SUCCESS)
				break;

			nResult = m_pDevCtrl->CheckDeviceAction(DEV_SPR, K_30_WAIT);
			if (nResult != NORMAL)
				break;
		}
	}

	return nResult;
}

// ----------------------------------------------------------------------------
// 함 수 명 : int CNHMWICtrl::DevSprImagePrint()
// DESCRIPT : 명세표 이미지 헤더를 인자한다
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevSprImagePrint(int nImageType)					// [#2373] US Justin 2015.09.22 Support Coupon Image Printing
{
	CString strFieldName("");

	strFieldName.Format(_T(""));		// For WinCE

	// [#2373] US Justin 2015.09.22 Support Printing Coupon Image
	//return m_pSpr.PrintForm(FORM_RECEIPT_IMAGE, MEDIA_RECEIPT_IMAGE, _T("USEFORMDEFN"), 0, 0, _T("MEDIUM"), strFieldName, K_30_WAIT*1000, _T("ANY"));
	if(nImageType == IMAGETYPE_COUPON_QR)
		return m_pSpr.PrintForm(FORM_RECEIPT_COUPON_QR,		 MEDIA_RECEIPT_IMAGE,	   _T("USEFORMDEFN"), 0, 0, _T("MEDIUM"), strFieldName, K_30_WAIT*1000, _T("ANY"));
	else if(nImageType == IMAGETYPE_COUPON_LARGEQR)				// [#2503] US Justin 2017.09.15 Add Ethereum.. Large QR Image
		return m_pSpr.PrintForm(FORM_RECEIPT_COUPON_LARGEQR, MEDIA_RECEIPT_LARGEIMAGE, _T("USEFORMDEFN"), 0, 0, _T("MEDIUM"), strFieldName, K_30_WAIT*1000, _T("ANY"));
	else if(nImageType == IMAGETYPE_COUPON_BCD)
		return m_pSpr.PrintForm(FORM_RECEIPT_COUPON_BCD,	 MEDIA_RECEIPT_IMAGE,	   _T("USEFORMDEFN"), 0, 0, _T("MEDIUM"), strFieldName, K_30_WAIT*1000, _T("ANY"));
	else
		return m_pSpr.PrintForm(FORM_RECEIPT_HEADER_IMAGE,	 MEDIA_RECEIPT_IMAGE,	   _T("USEFORMDEFN"), 0, 0, _T("MEDIUM"), strFieldName, K_30_WAIT*1000, _T("ANY"));
	// End of [#2373]
}

// ----------------------------------------------------------------------------
// 함 수 명 : int CNHMWICtrl::DevSprEject(int nWaitSec)
// DESCRIPT : 명세표를 방출한다.
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevSprEject(int nWaitSec)
{
	m_bSprWaitTaken = TRUE;										// 20030814_1.0_1066 : Pre-CancelWaitTaken Proc.
//	return m_pSpr.Eject(nWaitSec*1000);
	return m_pSpr.ControlMedia(_T("EJECT"), nWaitSec*1000);
}

// ----------------------------------------------------------------------------
// 함 수 명 : int CNHMWICtrl::DevSprRetract()
// DESCRIPT : 명세표를 회수한다.
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevSprRetract()
{
	m_pSpr.CancelWaitForMediaTaken();
	if (m_pSpr.GetNumberOfRetractBins())								// 2005.11.11
		return m_pSpr.RetractMedia(0);
	else
	{
//		OnDevSprPrintCaptured();
		return 0;
	}
}

// ----------------------------------------------------------------------------
// 함 수 명 : int CNHMWICtrl::DevSprCancelWaitTaken()
// DESCRIPT : 명세표 수취 취소처리
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevSprCancelWaitTaken()
{
	return m_pSpr.CancelWaitForMediaTaken();
}

// ----------------------------------------------------------------------------
// 함 수 명 : int CNHMWICtrl::DevSprEnableStatusEvents()
// DESCRIPT : 상태변경 이벤트 발생을 가능하게 한다.
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevSprEnableStatusEvents()
{
//	return m_pSpr.EnableStatusEvents();
	return R_NORMAL;
}

// ----------------------------------------------------------------------------
// 함 수 명 : int CNHMWICtrl::DevSprDisableStatusEvents()
// DESCRIPT : 상태변경 이벤트 발생을 불가하게 한다.
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevSprDisableStatusEvents()
{
//	return m_pSpr.DisableStatusEvents();
	return R_NORMAL;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : int CNHMWICtrl::DevSprResetBinCount()
// DESCRIPT  : SP에 명세표회수정보 CLEAR를 요청한다.
// PARAMETER : 없음
// RETURN    : True if Success
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevSprResetBinCount()
{
	int		nRetractCnt = 0;									// 2005.11.11
	if (m_pSpr.GetNumberOfRetractBins())
		return m_pSpr.ResetRetractCount(0);
	else
	{
		RegSetValueExt(_REGKEY_SPRSTATUS, _T("Retain_Count"), REG_INT, sizeof(nRetractCnt), &nRetractCnt);
		return 0;
	}
}

// ----------------------------------------------------------------------------
// PROTOTYPE : CString DevSprGetErrorCode() 
// DESCRIPT  : 장애코드를 조회한다.
// PARAMETER : 없음
// RETURN    : 조회된 SPR장애코드
// ----------------------------------------------------------------------------
CString CNHMWICtrl::DevSprGetErrorCode() 
{
	CString strResult("");

	// [2ND] NH AIREAT 2008.11.12
	//char szTemp[10];
	//memset(szTemp, NULL, sizeof(szTemp));
	//memcpy(szTemp, m_pSPInform->SPErr.ErrCode_SPR, 7);
	//strResult = CString(szTemp);
	m_pSPInform->SPErr.ErrCode_SPR[7] = 0;
	strResult.Format(L"%S", m_pSPInform->SPErr.ErrCode_SPR);
	// end of [2ND]

//	TCHAR szCode[256];		memset(szCode, 0x00, sizeof(szCode));
//	int nResult = 0;
//
//	// 레지스트리에서 장애코드를 확보한다.
//	nResult = RegQueryValueExt(_REGKEY_DEVERROR, _T("SPR"), REG_STR, sizeof(szCode), (LPVOID)szCode);
//	if (nResult)
//	{
//		strResult.Format(_T("%S0000000"), szCode);					// 2004.06.19
//		strResult = strResult.Left(7);							// 에러코드보정	: 2004.06.19
//	}

	return strResult;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long DevSprClearErrorCode() 
// DESCRIPT  : 명세표부의 장애코드를 클리어한다.
// PARAMETER : 없음
// RETURN    : NORMAL(0)
// ----------------------------------------------------------------------------
INT CNHMWICtrl::DevSprClearErrorCode() 
{
	RegSetValueExt(_REGKEY_DEVERROR, _T("SPR"), REG_STR, 0, NULL);	// 명세표부
	return NORMAL;
}

// ----------------------------------------------------------------------------
// 함 수 명 : CString DevSprGetSensorInfo
// DESCRIPT : 센서정보 조회 
// REMARK   : 20030719_1.0_1053 : GetSensorInfo
// ----------------------------------------------------------------------------
CString CNHMWICtrl::DevSprGetSensorInfo()
{
	CString strResult("");
	TCHAR szCode[1024];		memset(szCode, 0x00, sizeof(szCode));
	int nResult = 0;

	// 레지스트리에서 장애코드를 확보한다.
	nResult = RegQueryValueExt(_REGKEY_SPRSTATUS, _T("Sensor"), REG_STR, sizeof(szCode), (LPVOID)szCode);
	if (nResult)
	{
		strResult.Format(_T("%S"), szCode);
	}

	return strResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Event : Accepted From SPR
///////////////////////////////////////////////////////////////////////////////////////////////////
void CNHMWICtrl::OnFatalErrorNxreceiptprinter(LPCTSTR Action, long Result) 
{
	m_bSprWaitTaken = FALSE;									// 20030814_1.0_1066 : Pre-CancelWaitTaken Proc.
	m_nSprExist = ST_NOT_DETECT;

	DevSprUpdateStatus();
	ProcSetDeviceEvent(L"SPR", L"FatalError", NULL);

	// [#2306] NH KSK MWI Add NVRAM Log
	m_strTemp.Empty();
	m_strTemp.Format(L"Result:%x", Result);
	NVDump('F', '1', "92", L"9999999", L"FATAL_ERR");
	NVDump('F', '1', "92", L"9999999", m_strTemp);
	// end of [#2306]
}

void CNHMWICtrl::OnDeviceErrorNxreceiptprinter(LPCTSTR Action, long Result) 
{
	m_bSprWaitTaken = FALSE;									// 20030814_1.0_1066 : Pre-CancelWaitTaken Proc.
	m_nSprExist = ST_NOT_DETECT;

	DevSprUpdateStatus();
	ProcSetDeviceEvent(L"SPR", L"DeviceError", NULL);

	// [#2306] NH KSK MWI Add NVRAM Log
	m_strTemp.Empty();
	m_strTemp.Format(L"Result:%x", Result);
	NVDump('F', '1', "92", L"9999999", L"DEV_ERR");
	NVDump('F', '1', "92", L"9999999", m_strTemp);
	// end of [#2306]
}

void CNHMWICtrl::OnControlMediaCompleteNxreceiptprinter(LPCTSTR Actions) 
{
	DevSprUpdateStatus();
	ProcSetDeviceEvent(L"SPR", L"ControlMediaComplete", NULL);
}

void CNHMWICtrl::OnPrintFormCompleteNxreceiptprinter() 
{
	DevSprUpdateStatus();
	ProcSetDeviceEvent(L"SPR", L"PrintFormComplete", NULL);
}

void CNHMWICtrl::OnFieldWarningNxreceiptprinter(LPCTSTR FormName, LPCTSTR FieldName, short FailureCode) 
{
	DevSprUpdateStatus();
	ProcSetDeviceEvent(L"SPR", L"FieldWarning", NULL);
}

void CNHMWICtrl::OnFieldErrorNxreceiptprinter(LPCTSTR FormName, LPCTSTR FieldName, short FailureCode) 
{
	DevSprUpdateStatus();
	ProcSetDeviceEvent(L"SPR", L"FieldError", NULL);

	// [#2306] NH KSK MWI Add NVRAM Log
	NVDump('F', '1', "92", L"9999999", L"FIELD_ERR");
	m_strTemp.Empty();
	m_strTemp.Format(L"FN:%s", FieldName);
	NVDump('F', '1', "92", L"9999999", m_strTemp);
	m_strTemp.Format(L"Result:%x", FailureCode);
	NVDump('F', '1', "92", L"9999999", m_strTemp);
	// end of [#2306]
}

void CNHMWICtrl::OnTimeoutNxreceiptprinter() 
{
	m_bSprWaitTaken = FALSE;									// 20030814_1.0_1066 : Pre-CancelWaitTaken Proc.
	DevSprUpdateStatus();
	ProcSetDeviceEvent(L"SPR", L"Timeout", NULL);
}

void CNHMWICtrl::OnSendRawDataCompleteNxreceiptprinter(LPCTSTR ResponseData) 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnDeviceStatusChangedNxreceiptprinter(LPCTSTR newValue) 
{
	DevSprUpdateStatus();
	ProcSetDeviceEvent(L"SPR", L"DeviceStatusChanged", NULL);

	// [#2306] NH KSK MWI Add NVRAM Log
	m_strTemp.Empty();
	m_strTemp.Format(L"Result:%s", newValue);
	NVDump('O', '1', "92", L"", L"DEV_C_STS");
	NVDump('O', '1', "92", L"", m_strTemp);
	// end of [#2306]
}

void CNHMWICtrl::OnMediaStatusChangedNxreceiptprinter(LPCTSTR newValue) 
{
	DevSprUpdateStatus();
	ProcSetDeviceEvent(L"SPR", L"MediaStatusChanged", NULL);

	// [#2306] NH KSK MWI Add NVRAM Log
	m_strTemp.Empty();
	m_strTemp.Format(L"Result:%s", newValue);
	NVDump('O', '1', "92", L"", L"MDA_C_STS");
	NVDump('O', '1', "92", L"", m_strTemp);
	// end of [#2306]
}

void CNHMWICtrl::OnResetCompleteNxreceiptprinter() 
{
	DevSprUpdateStatus();
	ProcSetDeviceEvent(L"SPR", L"ResetComplete", NULL);
}

void CNHMWICtrl::OnPaperStatusChangedNxreceiptprinter(LPCTSTR PaperSource, LPCTSTR newValue) 
{
	DevSprUpdateStatus();
	ProcSetDeviceEvent(L"SPR", L"PaperStatusChanged", NULL);

	// [#2306] NH KSK MWI Add NVRAM Log
	m_strTemp.Empty();
	m_strTemp.Format(L"Result:%s", newValue);
	NVDump('O', '1', "92", L"", L"PAP_C_STS");
	NVDump('O', '1', "92", L"", m_strTemp);
	// end of [#2306]
}

void CNHMWICtrl::OnMediaTakenNxreceiptprinter() 
{
	m_bSprWaitTaken = FALSE;									// 20030814_1.0_1066 : Pre-CancelWaitTaken Proc.
	m_nSprExist = ST_NOT_DETECT;

	DevSprUpdateStatus();
	ProcSetDeviceEvent(L"SPR", L"ControlMediaComplete", NULL);
	ProcSetDeviceEvent(L"SPR", L"MediaTaken", NULL);
}

void CNHMWICtrl::OnWaitCancelledNxreceiptprinter() 
{
	m_bSprWaitTaken = FALSE;									// 20030814_1.0_1066 : Pre-CancelWaitTaken Proc.
	DevSprUpdateStatus();
	ProcSetDeviceEvent(L"SPR", L"WaitCancelled", NULL);
}

void CNHMWICtrl::OnResetCountCompleteNxreceiptprinter() 
{
	DevSprUpdateStatus();
	ProcSetDeviceEvent(L"SPR", L"ResetCountComplete", NULL);
}

void CNHMWICtrl::OnRetractMediaCompleteNxreceiptprinter(short BinNumber) 
{
	DevSprUpdateStatus();
	ProcSetDeviceEvent(L"SPR", L"RetractMediaComplete", NULL);
}

void CNHMWICtrl::OnRetractBinStatusChangedNxreceiptprinter(short BinNumber, LPCTSTR newValue) 
{
	DevSprUpdateStatus();
	ProcSetDeviceEvent(L"SPR", L"RetractBinStatusChanged", NULL);
}

void CNHMWICtrl::OnReadFormCompleteNxreceiptprinter(LPCTSTR FieldValues) 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnMediaExtentsCompleteNxreceiptprinter(long SizeX, long SizeY) 
{
	// TODO: Add your control notification handler code here
	
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Method : Call From SPR to SPR
///////////////////////////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// 함 수 명 : int CNHMWICtrl::DevSprUpdateStatus()
// DESCRIPT : 상태를 갱신한다.
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevSprUpdateStatus()
{
																// AP DOWN 장애현상 디버그 : 2003.12.20
	if (!m_bSprOpened)											// 장치가Open되지 않았으면
		return R_NORMAL;										// 상태갱신은 의미없음

	// 디바이스 상태와 매체 상태를 업데이트 한다.
	CString strStatus = m_pSpr.GetDeviceStatus();
	ProcSetDeviceStatus(L"SPR", L"StDeviceStatus", strStatus);
	if (strStatus.CompareNoCase(ST_DEVNODEVICE) != 0)			// 2004.11.05
	{
		ProcSetDeviceStatus(L"SPR", L"StMediaStatus", m_pSpr.GetMediaStatus());
		ProcSetDeviceStatus(L"SPR", L"StPaperStatus", m_pSpr.GetPaperStatus(_T("UPPER")));
	}

	return R_NORMAL;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : int DevSprPreCancelWaitTaken()
// DESCRIPT  : 매체수취대기 FLAG를 확인하여 대기중이면 수취대기를 취소한다
// PARAMETER : 없음
// RETURN    : 0
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevSprPreCancelWaitTaken()
{
	if (m_bSprWaitTaken)
	{
		SetEventMatrix(DEV_SPR, "CancelWaitForMediaTaken");
		DevSprCancelWaitTaken();
		CheckDeviceAction(DEV_SPR, K_30_WAIT);
	}
	m_bSprWaitTaken = FALSE;
	return R_NORMAL;
}