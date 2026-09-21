// DEVBCRIF.cpp

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
// BCR ActiveX 
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// Method : Call to BCR
///////////////////////////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// DESCRIPT : 장치디바이스 세션연결 및 초기화
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevBcrOpenConnection()
{
	int		nResult = R_ERROR;

	NHDEBUG(1, (_T("OpenSessionSync() ... \n")));
	DWORD curTickCount = GetTickCount();
	nResult = m_pBcr.OpenSessionSync(K_30_WAIT*1000);			// TODO
	NHDEBUG(1, (L"\n"));
	NHDEBUG(1, (_T("OpenSessionSync() ... DONE (%d) (%lu ms)\n"), nResult, GetTickCount() - curTickCount));

	if (nResult != R_NORMAL)
	{
		char chBcrOpenErrorCode[] = "9792A00";
		m_pDevCtrl->WriteMwiErrorCode(DEV_BCR, DEVNM_BCR, _countof(chBcrOpenErrorCode), chBcrOpenErrorCode);
		ProcSetDeviceEvent(L"BCR", L"FatalError", NULL);
	}
	else
	{
		// This is to remove a fatal error generated after successful OpenSessionSync(), which was executed to open SP which was killed by force by GOMA.
		m_pDevCtrl->EvtQReset(DEV_BCR);

		ProcSetDeviceEvent(L"BCR", L"OpenComplete", NULL);
	}

	return nResult;
}

// ----------------------------------------------------------------------------
// DESCRIPT : 장치디바이스 세션종료
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevBcrCloseConnection()
{
	NHDEBUG(1, (_T("DevBcrCloseConnection() ... \n")));
	int nResult = m_pBcr.CloseSessionSync();
	NHDEBUG(1, (_T("DevBcrCloseConnection() ... DONE (%d)\n"), nResult));

	return nResult;
}

// ----------------------------------------------------------------------------
// DESCRIPT : 장치디바이스 상태조회
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevBcrGetDeviceStatus()
{
	int nResult = NORMAL;
	CString strStatus = m_pDevCtrl->GetDeviceStatus(_T("BCRStDeviceStatus"));

	if		(strStatus.CompareNoCase(ST_DEVONLINE)	== 0)		nResult = NORMAL;
	else if	(strStatus.CompareNoCase(ST_DEVNODEVICE)== 0)		nResult = NODEVICE;
	else if	(strStatus.CompareNoCase(ST_DEVOFFLINE)== 0)		nResult = OFFLINE;
	else	nResult = DOWN;

	return nResult;
}

int CNHMWICtrl::DevBcrSendRawData()
{
	NHDEBUG(1, (_T("DevBcrSendRawData() ... \n")));

	m_strBarcodeData.Empty();
	int nResult = m_pBcr.SendRawData(TRUE, L"");
	
	NHDEBUG(1, (_T("DevBcrSendRawData() ... DONE (%d)\n"), nResult));

	return nResult;
}

int CNHMWICtrl::DevBcrCancelAccept()
{
	NHDEBUG(1, (_T("DevBcrCancelAccept() ... \n")));

	m_strBarcodeData.Empty();
	int nResult = m_pBcr.CancelAccept();
	NHDEBUG(1, (_T("DevBcrCancelAccept() ... DONE (%d)\n"), nResult));

	return nResult;
}

int CNHMWICtrl::DevBcrReset()
{
	NHDEBUG(1, (_T("DevBcrReset() ... \n")));
	int nResult = m_pBcr.Reset(L"EJECT", 0);
	NHDEBUG(1, (_T("DevBcrReset() ... DONE (%d)\n"), nResult));

	return nResult;
}

CString CNHMWICtrl::DevBcrGetBarcodeData()
{
	NHDEBUG(1, (_T("m_strBarcodeData: [%s]\n"), m_strBarcodeData));
	return m_strBarcodeData;
}

// ----------------------------------------------------------------------------
// DESCRIPT : 장치디바이스 장애코드 삭제
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevBcrClearErrorCode()
{
	RegSetValueExt(_REGKEY_DEVERROR, _T("BCR"), REG_STR, 0, NULL);
	return NORMAL;
}

// ----------------------------------------------------------------------------
// DESCRIPT : 장애코드 조회
// ----------------------------------------------------------------------------
CString CNHMWICtrl::DevBcrGetErrorCode()
{
	return CString(m_pSPInform->SPInfo.BCR.ErrCode);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Event
///////////////////////////////////////////////////////////////////////////////////////////////////
void CNHMWICtrl::OnFatalErrorNxBarcodeReader(LPCTSTR Action, long Result) 
{
	NHDEBUG(1, (_T("OnFatalErrorNxBarcodeReader() (%s)(%d)\n"), Action, Result));
	ProcSetDeviceEvent(L"BCR", L"FatalError", NULL);
	DevBcrUpdateStatus();
}

void CNHMWICtrl::OnDeviceErrorNxBarcodeReader(LPCTSTR Action, long Result) 
{
	NHDEBUG(1, (_T("OnDeviceErrorNxBarcodeReader() (%s)(%d)\n"), Action, Result));
	ProcSetDeviceEvent(L"BCR", L"DeviceError", NULL);
	DevBcrUpdateStatus();
}

void CNHMWICtrl::OnTimeoutNxBarcodeReader() 
{
	NHDEBUG(1, (_T("OnTimeoutNxBarcodeReader() (%s)\n"), _T("")));
	ProcSetDeviceEvent(L"BCR", L"Timeout", NULL);
	DevBcrUpdateStatus();
}


void CNHMWICtrl::OnResetCompleteNxBarcodeReader() 
{
	NHDEBUG(1, (_T("OnResetCompleteNxBarcodeReader() (%s)\n"), _T("")));
	ProcSetDeviceEvent(L"BCR", L"ResetComplete", NULL);
	DevBcrUpdateStatus();
}

void CNHMWICtrl::OnDeviceStatusChangedNxBarcodeReader(LPCTSTR newValue) 
{
	NHDEBUG(1, (_T("OnDeviceStatusChangedNxBarcodeReader() (%s)\n"), newValue));
	//ProcSetDeviceEvent(L"BCR", L"StatusChanged", NULL);	// duplicated with 'DevBcrUpdateStatus()'
	DevBcrUpdateStatus();
}

void CNHMWICtrl::OnSendRawDataCompleteNxBarcodeReader(LPCTSTR ResponseData)
{
	NHDEBUG(1, (_T("OnSendRawDataCompleteNxBarcodeReader() (%s)\n"), ResponseData));

	// 373236303030363633373233 => 726000663723
	CString strTemp = ResponseData;
	MakePack(strTemp, m_strBarcodeData, strTemp.GetLength());

	ProcSetDeviceEvent(L"BCR", L"SendRawDataComplete", NULL);
	DevBcrUpdateStatus();
}

void CNHMWICtrl::OnAcceptCancelledNxBarcodeReader()
{
	NHDEBUG(1, (_T("OnAcceptCancelledNxBarcodeReader() (%s)\n"), _T("")));

	//m_strBarcodeData.Empty();

	ProcSetDeviceEvent(L"BCR", L"AcceptCancelled", NULL);
	DevBcrUpdateStatus();
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Method : Call From BCR to BCR
///////////////////////////////////////////////////////////////////////////////////////////////////

int CNHMWICtrl::DevBcrUpdateStatus()
{
	if (!m_bBcrOpened)											// 장치가Open되지 않았으면
	{
		NHDEBUG(1, (_T("Device not opened (%s)\n"), _T("")));
		return R_NORMAL;										// 상태갱신은 의미없음
	}

	CString strStatus = m_pBcr.GetDeviceStatus();				// 장치상태조회 : 장치가 정상일경우에만 기타조회
	NHDEBUG(1, (_T("DeviceStatus(%s)\n"), strStatus));
	ProcSetDeviceStatus(L"BCR", L"StDeviceStatus", strStatus);

	return R_NORMAL;
}
