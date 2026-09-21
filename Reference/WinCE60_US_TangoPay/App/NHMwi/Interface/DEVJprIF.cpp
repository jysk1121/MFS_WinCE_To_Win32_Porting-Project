// DEVJPRIF.cpp

#include "stdafx.h"
#include "..\NHMWI.h"
#include "..\NHMWICtl.h"
#include "..\NHMWIPpg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// Kalignite JournalPrinter ActiveX BUGREPORT
// ------------------------------------------------------------------------------------------------
// 1. 2003.05.20
//		- 저널부 헤더열림 상태 감지 불가함
// ------------------------------------------------------------------------------------------------
// 2. 2003.08.06 : 초기화장애시 감지불가
//		- 원인 : 초기화동작과는 상관없이 SP/EP간 Session만 연결되면 정상이벤트가 발생된다.
//				 이때 AP(MWI)가 이 이벤트만을 확인하기 때문에 정상으로 확인된다.
//		- 대책 : 답답한 일이지만 SP는 대책이 불가하단다(남호현대리)
//				 AP(MWI) : 초기화후 정상이벤트 수신시에도 DeviceStatus를 확인하여 이벤트를 조작하도록
//						   처리하여 결과를 주시하고자 한다.
//		- 기타 : Kalignite SP를 사용하는 주요장치는 이러한 문제에 노출되어 있다
//				 (JPR/SPR/IDC/PBM)
//				 BRM과 UCM은 확인할 필요가 있다(정상처리로 알고있다)
// ------------------------------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// Method : Call to MCU
///////////////////////////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// 함 수 명 : int CNHMWICtrl::DevJprOpenConnection()
// DESCRIPT : 세션연결 시작
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevJprOpenConnection()
{
// AIREAT 2009.07.14
//-NOUSE	int nReturn = m_pJpr.OpenSessionSync(K_30_WAIT*1000);
//-NOUSE	if (nReturn != R_NORMAL)
//-NOUSE	{
//-NOUSE		CString strErrorCode = "9792100";
//-NOUSE		m_pDevCtrl->WriteMwiErrorCode(DEV_JPR, "JPR", strErrorCode.GetLength(), "9792100");
//-NOUSE		ProcSetDeviceEvent(L"JPR", L"FatalError", NULL);
//-NOUSE		return R_ERROR;
//-NOUSE	}
//-NOUSE
//-NOUSE	DevJprUpdateStatus();
//-NOUSE
	return R_NORMAL;
}

// ----------------------------------------------------------------------------
// 함 수 명 : int CNHMWICtrl::DevJprCloseConnection()
// DESCRIPT : 세션종료
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevJprCloseConnection()
{
//-NOUSE	m_pJpr.CloseSessionSync();			// AIREAT 2009.07.14
	return R_NORMAL;
}

// ----------------------------------------------------------------------------
// 함 수 명 : int CNHMWICtrl::DevJprGetStatus(int nStatus)
// DESCRIPT : 상태조회(장치, 매체, 용지등)
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevJprGetStatus(int nStatus)
{
	int nResult = R_NORMAL;
	CString strStatus("");

	switch(nStatus)
	{
		case ST_DEVICEVSTATUS  :
			strStatus = m_pDevCtrl->GetDeviceStatus(_T("JPRStDeviceStatus"));
			if		(strStatus.CompareNoCase(ST_DEVONLINE)	== 0)	nResult = NORMAL;
			else if	(strStatus.CompareNoCase(ST_DEVNODEVICE)== 0)	nResult = NODEVICE;
			else	nResult = DOWN;
			break;
		case ST_MEDIASTATUS  :
			// 저널부는 용지가 검지되면 어느위치건간에 SENSOR1으로 설정
			strStatus = m_pDevCtrl->GetDeviceStatus(_T("JPRStMediaStatus"));
			if		(strStatus.CompareNoCase(ST_NOTPRESENT) == 0)	nResult = ST_NOT_DETECT;
			else if (strStatus.CompareNoCase(ST_PRESENT)	== 0)	nResult = ST_SENSOR2;
			else if (strStatus.CompareNoCase(ST_ENTERING)	== 0)	nResult = ST_SENSOR1;
			else	nResult = ST_NOT_DETECT;					// 20030729_1.0_1055 : MaterialInfo
			break;
		case ST_PAPERSTATUS :
			strStatus = m_pDevCtrl->GetDeviceStatus(_T("JPRStPaperStatus"));
			if		(strStatus.CompareNoCase(ST_PAPER_FULL) == 0)	nResult = JNL_NORMAL;
			else if (strStatus.CompareNoCase(ST_PAPER_LOW)	== 0)	nResult = JNL_LOW_END;
			else if (strStatus.CompareNoCase(ST_PAPER_OUT)	== 0)	nResult = JNL_EMPTY_PAPER;
			else	nResult = JNL_EMPTY_PAPER;
		default :
			break;
	}

	return nResult;
}

// ----------------------------------------------------------------------------
// 함 수 명 : int CNHMWICtrl::DevJprPrint(LPCTSTR szData)
// DESCRIPT : 명세표를 인자한다
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevJprPrint(LPCTSTR szData)
{
// AIREAT 2009.07.14
//-NOUSE//	m_pJpr.SetDefaultMediaName(MEDIA_JOURNAL);
//-NOUSE
//-NOUSE	CStringArray	strDatas;
//-NOUSE	CString			strPrintData("");
//-NOUSE	CString			strLineData("");
//-NOUSE	CString			strData("");
//-NOUSE	char			szBuff[256];
//-NOUSE
//-NOUSE	strData.Format(_T("%s"), szData);
//-NOUSE	memset(szBuff, 0x00, sizeof(szBuff));
//-NOUSE	
//-NOUSE	int nLen = SplitString(strData, ",", strDatas);
//-NOUSE	strData.Empty();
//-NOUSE
//-NOUSE	char szTemp[1024];
//-NOUSE	
//-NOUSE	for(int i=0;i<nLen;i++)
//-NOUSE	{
//-NOUSE		memset(szBuff, 0x00, sizeof(szBuff));
//-NOUSE		strData = "";
//-NOUSE		memset(szTemp, NULL, sizeof(szTemp));
//-NOUSE		WideToMulti(szTemp, strDatas.GetAt(i), sizeof(szTemp));
//-NOUSE
//-NOUSE		MakePack(szTemp, szBuff, strDatas.GetAt(i).GetLength());
//-NOUSE		strData.Format(_T("%S"), szBuff);
//-NOUSE
//-NOUSE		strData.Replace(_T(","), _T(",,"));						// Add for NextWare
//-NOUSE
//-NOUSE		strLineData.Format(_T("Line%d=%s"), i+1, strData);		// for NextWare
//-NOUSE		strPrintData += strLineData;
//-NOUSE		if (i >= SLIP_MAX_LINE-1)	break;
//-NOUSE		if (i < nLen-1)	strPrintData += ",";
//-NOUSE		strLineData = "";
//-NOUSE	}
//-NOUSE
//-NOUSE	return m_pJpr.PrintForm(FORM_JOURNAL, MEDIA_JOURNAL, _T("USEFORMDEFN"), 0, 0, _T("MEDIUM"), strPrintData, K_30_WAIT*1000, _T("ANY"));

	return 0;
}

// ----------------------------------------------------------------------------
// 함 수 명 : int CNHMWICtrl::DevJprEmbossPrint()
// DESCRIPT : 명세표를 인자한다
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevJprEmbossPrint()
{
//	m_pJpr.SetDefaultMediaName(MEDIA_JOURNAL);
//	return m_pJpr.Print(FORM_JOURNAL, "Line1=<Emboss>");
	return R_NORMAL;
}

// ----------------------------------------------------------------------------
// 함 수 명 : int CNHMWICtrl::DevJprEnableStatusEvents()
// DESCRIPT : 상태변경 이벤트 발생을 가능하게 한다.
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevJprEnableStatusEvents()
{
//	return m_pJpr.EnableStatusEvents();
	return R_NORMAL;
}

// ----------------------------------------------------------------------------
// 함 수 명 : int CNHMWICtrl::DevJprDisableStatusEvents()
// DESCRIPT : 상태변경 이벤트 발생을 불가하게 한다.
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevJprDisableStatusEvents()
{
//	return m_pJpr.DisableStatusEvents();
	return R_NORMAL;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : BSTR DevJprGetErrorCode() 
// DESCRIPT  : 장애코드를 얻어온다.
// PARAMETER : 없음
// RETURN    : 장애코드
// ----------------------------------------------------------------------------
CString CNHMWICtrl::DevJprGetErrorCode() 
{
	CString strResult("");

	char szTemp[10];
	memset(szTemp, NULL, sizeof(szTemp));
	memcpy(szTemp, m_pSPInform->SPErr.ErrCode_JPR, 7);
	strResult = CString(szTemp);

//	TCHAR szCode[256];		memset(szCode, 0x00, sizeof(szCode));
//	int nResult = 0;
//
//	// 레지스트리에서 장애코드를 확보한다.
//	nResult = RegQueryValueExt(_REGKEY_DEVERROR, _T("JPR"), REG_STR, sizeof(szCode), (LPVOID)szCode);
//	if (nResult)
//	{
//		strResult.Format(_T("%S0000000"), szCode);				// 에러코드보정 : 20030729_1.0_1055 : ErrorCode Revision
//		strResult = strResult.Left(7);							// 에러코드보정	: 20030729_1.0_1055 : ErrorCode Revision
//	}

	return strResult;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long DevJprClearErrorCode() 
// DESCRIPT  : 저널부의 장애코드를 클리어한다.
// PARAMETER : 없음
// RETURN    : NORMAL(0)
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevJprClearErrorCode() 
{
	RegSetValueExt(_REGKEY_DEVERROR, _T("JPR"), REG_STR, 0, NULL);	// 저널부
	return NORMAL;
}

// ----------------------------------------------------------------------------
// 함 수 명 : CString DevJprGetSensorInfo
// DESCRIPT : 센서정보 조회 
// REMARK   : 20030719_1.0_1053 : GetSensorInfo
// ----------------------------------------------------------------------------
CString CNHMWICtrl::DevJprGetSensorInfo()
{
	CString strResult("");
	TCHAR szCode[1024];		memset(szCode, 0x00, sizeof(szCode));
	int nResult = 0;

	// 레지스트리에서 장애코드를 확보한다.
	nResult = RegQueryValueExt(_REGKEY_JPRSTATUS, _T("Sensor"), REG_STR, sizeof(szCode), (LPVOID)szCode);
	if (nResult)
	{
		strResult.Format(_T("%S"), szCode);
	}

	return strResult;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Event : Accepted From SPR
///////////////////////////////////////////////////////////////////////////////////////////////////

void CNHMWICtrl::OnFatalErrorNxjournalprinter(LPCTSTR Action, long Result) 
{
	m_nJprExist = ST_NOT_DETECT;

	DevJprUpdateStatus();
	ProcSetDeviceEvent(L"JPR", L"FatalError", NULL);
}

void CNHMWICtrl::OnDeviceErrorNxjournalprinter(LPCTSTR Action, long Result) 
{
	m_nJprExist = ST_NOT_DETECT;

	DevJprUpdateStatus();
	ProcSetDeviceEvent(L"JPR", L"DeviceError", NULL);
}

void CNHMWICtrl::OnLockGrantedNxjournalprinter() 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnLockReleasedNxjournalprinter() 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnLockTimeoutNxjournalprinter() 
{
	// TODO: Add your control notification handler code here
}

void CNHMWICtrl::OnControlMediaCompleteNxjournalprinter(LPCTSTR Actions) 
{
	DevJprUpdateStatus();
	ProcSetDeviceEvent(L"JPR", L"ControlMediaComplete", NULL);
}

void CNHMWICtrl::OnPrintFormCompleteNxjournalprinter() 
{
	m_nJprExist = ST_NOT_DETECT;

	DevJprUpdateStatus();
	ProcSetDeviceEvent(L"JPR", L"PrintFormComplete", NULL);
}

void CNHMWICtrl::OnFieldWarningNxjournalprinter(LPCTSTR FormName, LPCTSTR FieldName, short FailureCode) 
{
	DevJprUpdateStatus();
	ProcSetDeviceEvent(L"JPR", L"FieldWarning", NULL);
}

void CNHMWICtrl::OnFieldErrorNxjournalprinter(LPCTSTR FormName, LPCTSTR FieldName, short FailureCode) 
{
	DevJprUpdateStatus();
	ProcSetDeviceEvent(L"JPR", L"FieldError", NULL);
}

void CNHMWICtrl::OnTimeoutNxjournalprinter() 
{
	DevJprUpdateStatus();
	ProcSetDeviceEvent(L"JPR", L"Timeout", NULL);
}

void CNHMWICtrl::OnSendRawDataCompleteNxjournalprinter(LPCTSTR ResponseData) 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnDeviceStatusChangedNxjournalprinter(LPCTSTR newValue) 
{
	DevJprUpdateStatus();
	ProcSetDeviceEvent(L"JPR", L"DeviceStatusChanged", NULL);
}

void CNHMWICtrl::OnMediaStatusChangedNxjournalprinter(LPCTSTR newValue) 
{
	DevJprUpdateStatus();
	ProcSetDeviceEvent(L"JPR", L"MediaStatusChanged", NULL);
}

void CNHMWICtrl::OnTonerStatusChangedNxjournalprinter(LPCTSTR newValue) 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnInkStatusChangedNxjournalprinter(LPCTSTR newValue) 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnLampStatusChangedNxjournalprinter(LPCTSTR newValue) 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnResetCompleteNxjournalprinter() 
{
	DevJprUpdateStatus();
	ProcSetDeviceEvent(L"JPR", L"ResetComplete", NULL);
}

void CNHMWICtrl::OnMediaDetectedNxjournalprinter(LPCTSTR Position, short BinNumber) 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnDispensePaperCompleteNxjournalprinter() 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnPaperStatusChangedNxjournalprinter(LPCTSTR PaperSource, LPCTSTR newValue) 
{
	DevJprUpdateStatus();
	ProcSetDeviceEvent(L"JPR", L"PaperStatusChanged", NULL);
}

void CNHMWICtrl::OnMediaInsertedNxjournalprinter() 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnAcceptCancelledNxjournalprinter() 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnMediaTakenNxjournalprinter() 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnWaitCancelledNxjournalprinter() 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnResetCountCompleteNxjournalprinter() 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnRetractMediaCompleteNxjournalprinter(short BinNumber) 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnRetractBinStatusChangedNxjournalprinter(short BinNumber, LPCTSTR newValue) 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnReadFormCompleteNxjournalprinter(LPCTSTR FieldValues) 
{
	// TODO: Add your control notification handler code here
	
}

void CNHMWICtrl::OnMediaExtentsCompleteNxjournalprinter(long SizeX, long SizeY) 
{
	// TODO: Add your control notification handler code here
	
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Method : Call From SPR to SPR
///////////////////////////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// 함 수 명 : 
// DESCRIPT : 
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevJprUpdateStatus()
{
																// AP DOWN 장애현상 디버그 : 2003.12.20
	if (!m_bJprOpened)											// 장치가Open되지 않았으면
		return R_NORMAL;										// 상태갱신은 의미없음

// AIREAT 2009.07.14
//-NOUSE	// 디바이스 상태와 매체 상태를 업데이트 한다.
//-NOUSE	CString strStatus = m_pJpr.GetDeviceStatus();
//-NOUSE	ProcSetDeviceStatus(L"JPR", L"StDeviceStatus", strStatus);
//-NOUSE	if (strStatus.CompareNoCase(ST_DEVNODEVICE) != 0)
//-NOUSE	{
//-NOUSE		ProcSetDeviceStatus(L"JPR", L"StMediaStatus", m_pJpr.GetMediaStatus());
//-NOUSE		ProcSetDeviceStatus(L"JPR", L"StPaperStatus", m_pJpr.GetPaperStatus(_T("UPPER")));
//-NOUSE	}

	return R_NORMAL;
}