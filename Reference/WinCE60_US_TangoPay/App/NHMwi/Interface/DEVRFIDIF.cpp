// DEVRFIDIF.cpp
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
// Method : Call to RFID - Nextware Support 2015.01.20~
///////////////////////////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// PROTOTYPE : int CNHMWICtrl::DevRFIDOpenConnection()
// DESCRIPT  : 초기 이니셜
// PARAMETER : 없음
// RETURN    : 0
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevRFIDOpenConnection()
{
	int nReturn = m_pRFID.OpenSessionSync(K_30_WAIT*1000);

NHDEBUG(1, (_T("CNHMWICtrl::DevRFIDOpenConnection() result [%d]\n"), nReturn));

	if (nReturn == R_NORMAL)
	{
		ProcSetDeviceEvent(L"RFID", L"OpenComplete", NULL);
		DevRFIDUpdateStatus();
	}
	else //error
	{
		ProcSetDeviceEvent(L"RFID", L"FatalError", NULL);
	}

	m_bRFIDEntry = ENTRY_DISABLED;

	return nReturn;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : int CNHMWICtrl::DevRFIDCloseConnection()
// DESCRIPT  : 세션종료
// PARAMETER : 없음
// RETURN    : 0
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevRFIDCloseConnection()
{
	return m_pRFID.CloseSessionSync();
}

// ----------------------------------------------------------------------------
// PROTOTYPE : 
// DESCRIPT  : 카드부엔트리 무한대기로 발행
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevRFIDAcceptAndReadAvailableTracks(int nWaitTimeSec)
{
	// VivoPAy는 Track1,2만 지원됨
	DevRFIDAcceptAndReadAvailableTracks(_T("ISO1,ISO2"), nWaitTimeSec);

	return 0;
}
// ----------------------------------------------------------------------------
// PROTOTYPE : int CNHMWICtrl::DevRFIDAcceptAndReadAvailableTracks(LPCTSTR szTracks, int nWaitTimeSec)
// DESCRIPT  : 엔트리 발행및 카드읽기
// PARAMETER : 읽을 트랙(예:"2,3"), 카드삽입대기시간(-1은 무한대기)
// RETURN    : 
// 기타사항 : 이 함수는 카드에 하나라도 유효한 데이타가 있어 읽기가 성공하면 CardAccepted를 이벤트로
//			  발생시킨다. 그러나 이와 비슷한 함수중 AcceptAndReadTracks는 지정된 트랙맵의 트랙중
//			  하나라도 유효하지 않으면 CardInvalid를 발행한다. 이점 유의하도록.....
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevRFIDAcceptAndReadAvailableTracks(LPCTSTR szTracks, int nWaitTimeSec)
{
NHDEBUG(1, (_T("CNHMWICtrl::DevRFIDAcceptAndReadAvailableTracks() m_bRFIDEntry [%d]\n"), m_bRFIDEntry));

	if (m_bRFIDEntry == ENTRY_ENABLED)							// 엔트리가 발행된 상태라면 처리하지 않는다
	{
		ProcSetDeviceEvent(L"RFID", L"EntryEnabled", NULL);
		return R_NORMAL;
	}

	m_bRFIDEntry = ENTRY_ENABLED;

	int nReturn = 0;
	if (nWaitTimeSec == 0)
		nReturn = m_pRFID.ReadAvailableRawData(szTracks, nWaitTimeSec);
	else
		nReturn = m_pRFID.ReadAvailableRawData(szTracks, nWaitTimeSec*1000);

	ProcSetDeviceEvent(L"RFID", L"EntryEnabled", NULL);
	return nReturn;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : int CNHMWICtrl::DevRFIDCancelAccept()
// DESCRIPT  : 카드부 엔트리발행을 취소한다.
// PARAMETER : 없음
// RETURN    : 
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevRFIDCancelAccept()
{
NHDEBUG(1, (_T("CNHMWICtrl::DevRFIDCancelAccept() m_bRFIDEntry[%d]\n"), m_bRFIDEntry));
	// 엔트리가 발행되지 않은 상태라면 이벤트를 걍 발생시킨다.
	if (m_bRFIDEntry == ENTRY_DISABLED)							// 엔트리가 발행되지 않은 상태라면 이벤트를 걍 발생시킨다.
	{
		ProcSetDeviceEvent(L"RFID", L"AcceptCancelled", NULL);
		return R_NORMAL;
	}

	m_bRFIDEntry = ENTRY_DISABLED;

	return m_pRFID.CancelAccept();
}

// ----------------------------------------------------------------------------
// PROTOTYPE : CString CNHMWICtrl::DevRFIDGetTrackData(int nTrack)
// DESCRIPT  : 카드의 데이타를 트랙별로 리턴한다.
// PARAMETER : 데이타를 알고자 하는 트랙맵
// RETURN    : 
// REMARK    : 20030917_1.0_1073 : MakeUnpack
// ----------------------------------------------------------------------------
CString CNHMWICtrl::DevRFIDGetTrackData(int nTrack)
{
	CString strResult;
	char szData[2048];		// [#2022] NH KSK 2011.02.22

	memset(szData, 0x00, sizeof(szData));

	switch (nTrack)
	{
		case 1  :
			//[#2250] US Justin 2014.01.27 Read Track1 data
			if (m_pRFID.GetTrack1Status().CompareNoCase(_T("READ")) == 0)
			{
				CString strTrack1Data = m_pRFID.GetTrack1Data();

				Strcpy(szData, strTrack1Data.GetBuffer(0));
				strTrack1Data.ReleaseBuffer();
			}
			// End of [#2250]
			break;
		case 2  :
			if (m_pRFID.GetTrack2Status().CompareNoCase(_T("READ")) == 0)
			{
				CString strTrack2Data = m_pRFID.GetTrack2Data();

				Strcpy(szData, strTrack2Data.GetBuffer(0));
				strTrack2Data.ReleaseBuffer();
	
			}
			break;
		case 3  :
			if (m_pRFID.GetTrack3Status().CompareNoCase(_T("READ")) == 0)
			{
				CString strTrack3Data = m_pRFID.GetTrack3Data();

				Strcpy(szData, strTrack3Data.GetBuffer(0));
				strTrack3Data.ReleaseBuffer();

			}
			break;
		default :
			break;
	}

	strResult = MakeUnPack(szData, __min(strlen(szData), 1024), TRUE);		// [#2022] NH KSK 2011.02.21 Code Sonar 지적사항 대책

NHDEBUG(1, (_T("CNHMWICtrl::DevRFIDGetTrackData()  nTrack(%d)  szData(%S)  Len(%d) \n"), nTrack, szData, strlen(szData)));

	return strResult;
}


// ----------------------------------------------------------------------------
// PROTOTYPE : int CNHMWICtrl::DevRFIDGetStatus(int nStatus)
// DESCRIPT  : 카드디바이스의 장치상태(파라미터:1), 카드상태(파라미터:2)를 조회
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevRFIDGetStatus(int nStatus)
{
	int nResult = R_NORMAL;
	CString strStatus("");

	switch(nStatus)
	{
		case ST_DEVICEVSTATUS  :
			strStatus = m_pDevCtrl->GetDeviceStatus(_T("RFIDStDeviceStatus"));
			if		(strStatus.CompareNoCase(ST_DEVONLINE)	== 0)	nResult = NORMAL;
			else if	(strStatus.CompareNoCase(ST_DEVNODEVICE)== 0)	nResult = NODEVICE;
			else	nResult = DOWN;
			break;
		case ST_MEDIASTATUS  :
			strStatus = m_pDevCtrl->GetDeviceStatus(_T("RFIDStMediaStatus"));
			if		(strStatus.CompareNoCase(ST_NOTPRESENT) == 0)	nResult = ST_NOT_DETECT;
			else if (strStatus.CompareNoCase(ST_PRESENT)	== 0)	nResult = ST_SENSOR2;
			else if (strStatus.CompareNoCase(ST_ENTERING)	== 0)	nResult = ST_SENSOR1;
			else if (strStatus.CompareNoCase(ST_JAMMED)		== 0)	nResult = ST_SENSOR2;
			else	nResult = ST_NOT_DETECT;
			break;
		default :
			break;
	}

	return nResult;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : CString DevRFIDGetErrorCode() 
// DESCRIPT  : 레지스트리에 저장된 장애코드를 확보한다.
// PARAMETER : 없음
// RETURN    : 장애코드
// ----------------------------------------------------------------------------
CString CNHMWICtrl::DevRFIDGetErrorCode() 
{
	CString strResult("");

	char szTemp[10];
	memset(szTemp, NULL, sizeof(szTemp));
	memcpy(szTemp, m_pSPInform->SPInfo.RFD.ErrCode, 7);
	strResult = CString(szTemp);

	return strResult;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long DevRFIDClearErrorCode() 
// DESCRIPT  : 카드부의 장애코드를 클리어한다.
// PARAMETER : 없음
// RETURN    : NORMAL(0)
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevRFIDClearErrorCode() 
{
	return NORMAL;
}

// [#2325] NH KSK 2015.01.20
/************************************************************************/
/* Event : Accepted From RFID											*/
/************************************************************************/
void CNHMWICtrl::OnFatalErrorNxrfidreader(LPCTSTR Action, long Result) 
{
NHDEBUG(1, (_T("CNHMWICtrl::OnFatalErrorNxrfidreader() \n")));

	m_bRFIDEntry = ENTRY_DISABLED;			// [#460] [NH] KSK 2008.11.26 Disable Flag Set 시 AP에서 Disable Command에 대해 처리 못하는 Bug Fix

	DevRFIDUpdateStatus();
	ProcSetDeviceEvent(L"RFID", L"FatalError", NULL);

	// [#2306] NH KSK MWI Add NVRAM Log
	m_strTemp.Empty();
	m_strTemp.Format(L"Result:%x", Result);
	NVDump('F', '1', "95", L"9999999", L"FATAL_ERR");
	NVDump('F', '1', "95", L"9999999", m_strTemp);
	// end of [#2306]
}

void CNHMWICtrl::OnDeviceErrorNxrfidreader(LPCTSTR Action, long Result) 
{
NHDEBUG(1, (_T("CNHMWICtrl::OnDeviceErrorNxrfidreader() \n")));

	m_bMcuEntry = ENTRY_DISABLED;								// 20030710_1.0_1050 : 엔트리없음 디버그
	
	DevRFIDUpdateStatus();
	ProcSetDeviceEvent(L"RFID", L"DeviceError", NULL);

	// [#2306] NH KSK MWI Add NVRAM Log
	m_strTemp.Empty();
	m_strTemp.Format(L"RESULT:%x", Result);
	NVDump('F', '1', "95", L"9999999", L"DEV_ERR");
	NVDump('F', '1', "95", L"9999999", m_strTemp);
	// end of [#2306]
}

void CNHMWICtrl::OnLockGrantedNxrfidreader() 
{
}

void CNHMWICtrl::OnLockReleasedNxrfidreader() 
{
}

void CNHMWICtrl::OnLockTimeoutNxrfidreader() 
{
}

void CNHMWICtrl::OnDeviceStatusChangedNxrfidreader(LPCTSTR Value) 
{
NHDEBUG(1, (_T("CNHMWICtrl::OnDeviceStatusChangedNxrfidreader() Value(%s)\n"), Value));
 	DevRFIDUpdateStatus();

	// [#2306] NH KSK MWI Add NVRAM Log
	m_strTemp.Empty();
	m_strTemp.Format(L"VAL:%s", Value);
	NVDump('O', '1', "95", L"", L"DEV_C_STS");
	NVDump('O', '1', "95", L"", m_strTemp);
	// end of [#2306]
}

void CNHMWICtrl::OnMediaStatusChangedNxrfidreader(LPCTSTR Value) 
{
NHDEBUG(1, (_T("CNHMWICtrl::OnMediaStatusChangedNxrfidreader() Value(%s)\n"), Value));
//	DevRFIDUpdateStatus();

	// [#2306] NH KSK MWI Add NVRAM Log
	m_strTemp.Format(L"MDA:%s", Value);
	NVDump('O', '1', "95", L"", L"MDA_C_STS");
	NVDump('O', '1', "95", L"", m_strTemp);
	// end of [#2306]
}

void CNHMWICtrl::OnRetainBinStatusChangedNxrfidreader(LPCTSTR Value) 
{
NHDEBUG(1, (_T("CNHMWICtrl::OnRetainBinStatusChangedNxrfidreader(%s) \n"), Value));
}

void CNHMWICtrl::OnSecurityStatusChangedNxrfidreader(LPCTSTR Value) 
{
NHDEBUG(1, (_T("CNHMWICtrl::OnSecurityStatusChangedNxrfidreader(%s) \n"), Value));
}

void CNHMWICtrl::OnCardsStatusChangedNxrfidreader(short Value) 
{
NHDEBUG(1, (_T("CNHMWICtrl::OnCardsStatusChangedNxrfidreader(%s) \n"), Value));

	// [#2306] NH KSK MWI Add NVRAM Log
	m_strTemp.Empty();
	m_strTemp.Format(L"VAL:%s", Value);
	NVDump('O', '1', "91", L"", L"C_S_C");
	NVDump('O', '1', "91", L"", m_strTemp);
	// end of [#2306]
}

void CNHMWICtrl::OnReadCompleteNxrfidreader() 
{
NHDEBUG(1, (_T("CNHMWICtrl::OnReadCompleteNxrfidreader() \n")));

	m_bRFIDEntry = ENTRY_DISABLED;								// 20030710_1.0_1050 : 엔트리없음 디버그

	DevRFIDUpdateStatus();										// 모든 상태값을 업데이트한다.

	ProcSetDeviceEvent(L"RFID", L"ReadComplete", NULL);
}

void CNHMWICtrl::OnWriteCompleteNxrfidreader() 
{
NHDEBUG(1, (_T("CNHMWICtrl::OnWriteCompleteNxrfidreader() \n")));

	DevRFIDUpdateStatus();

	ProcSetDeviceEvent(L"RFID", L"WriteComplete", NULL);
}

void CNHMWICtrl::OnEjectCompleteNxrfidreader() 
{
NHDEBUG(1, (_T("CNHMWICtrl::OnEjectCompleteNxrfidreader() \n")));

	DevRFIDUpdateStatus();

	ProcSetDeviceEvent(L"RFID", L"EjectComplete", NULL);
}

void CNHMWICtrl::OnRetainCompleteNxrfidreader() 
{
NHDEBUG(1, (_T("CNHMWICtrl::OnRetainCompleteNxrfidreader() \n")));

	DevRFIDUpdateStatus();										// 모든 상태값을 업데이트한다.

	ProcSetDeviceEvent(L"RFID", L"RetainComplete", NULL);
}

void CNHMWICtrl::OnChipIOCompleteNxrfidreader(LPCTSTR Token, const VARIANT FAR& Data) 
{
NHDEBUG(1, (_T("CNHMWICtrl::OnChipIOCompleteNxrfidreader() \n")));
}

void CNHMWICtrl::OnChipIOFailureNxrfidreader(LPCTSTR Token) 
{
NHDEBUG(1, (_T("CNHMWICtrl::OnChipIOFailureNxrfidreader() \n")));
}

void CNHMWICtrl::OnMediaInsertedNxrfidreader() 
{
NHDEBUG(1, (_T("CNHMWICtrl::OnMediaInsertedNxrfidreader() \n")));

	DevRFIDUpdateStatus();	

	ProcSetDeviceEvent(L"RFID", L"MediaInserted", NULL);

	NVDump('O', '1', "95", L"", L"INSERTED");	// [#2306] NH KSK MWI Add NVRAM Log
}

void CNHMWICtrl::OnInvalidMediaNxrfidreader() 
{
NHDEBUG(1, (_T("CNHMWICtrl::OnInvalidMediaNxrfidreader() \n")));

	m_bRFIDEntry = ENTRY_DISABLED;

	DevRFIDUpdateStatus();										// 모든 상태값을 업데이트한다.

	ProcSetDeviceEvent(L"RFID", L"InvalidMedia", NULL);

	NVDump('F', '1', "95", L"9999999", L"INVALID_MEA");	// [#2306] NH KSK MWI Add NVRAM Log
}

void CNHMWICtrl::OnInvalidTrackDataNxrfidreader() 
{
NHDEBUG(1, (_T("CNHMWICtrl::OnInvalidTrackDataNxrfidreader() \n")));

	m_bRFIDEntry = ENTRY_DISABLED;

	DevRFIDUpdateStatus();										// 모든 상태값을 업데이트한다.

	ProcSetDeviceEvent(L"RFID", L"InvalidTrackData", NULL);

	NVDump('F', '1', "95", L"9999999", L"INVALID_TRACK");	// [#2306] NH KSK MWI Add NVRAM Log
}

void CNHMWICtrl::OnMediaRemovedNxrfidreader() 
{
NHDEBUG(1, (_T("CNHMWICtrl::OnTimeoutNxrfidreader() \n")));

//	DevRFIDUpdateStatus();										// 모든 상태값을 업데이트한다.

	NVDump('O', '1', "95", L"", L"MEA_REMOVE");	// [#2306] NH KSK MWI Add NVRAM Log
}

void CNHMWICtrl::OnRetainBinThresholdNxrfidreader() 
{
}

void CNHMWICtrl::OnTimeoutNxrfidreader() 
{
NHDEBUG(1, (_T("CNHMWICtrl::OnTimeoutNxrfidreader() \n")));

	m_bRFIDEntry = ENTRY_DISABLED;

	DevRFIDUpdateStatus();
	ProcSetDeviceEvent(L"RFID", L"Timeout", NULL);
}

void CNHMWICtrl::OnChipPowerStatusChangedNxrfidreader(LPCTSTR Value) 
{
NHDEBUG(1, (_T("CNHMWICtrl::OnChipPowerStatusChangedNxrfidreader() \n")));

}

void CNHMWICtrl::OnMediaDetectedNxrfidreader(LPCTSTR Value) 
{
NHDEBUG(1, (_T("CNHMWICtrl::OnMediaDetectedNxrfidreader() \n")));

}

void CNHMWICtrl::OnResetCompleteNxrfidreader() 
{
NHDEBUG(1, (_T("CNHMWICtrl::OnResetCompleteNxrfidreader() \n")));

	DevRFIDUpdateStatus();										// 모든 상태값을 업데이트한다.

	int nDeviceStatus = DevRFIDGetStatus(ST_DEVICEVSTATUS);		// 장치의 상태값을 확인한다.

	if (nDeviceStatus != NORMAL)								// 장애검지시(FATAL/*NODEVICE*)
	{
		CString strErrorCode = "9792900";
		m_pDevCtrl->WriteMwiErrorCode(DEV_RFID, "RFID", strErrorCode.GetLength(), "9792900");
		ProcSetDeviceEvent(L"MCU", L"FatalError", NULL);
	}
	else /* NORMAL */
	{
		ProcSetDeviceEvent(L"RFID", L"ResetComplete", NULL);
	}
}

void CNHMWICtrl::OnChipPowerCompleteNxrfidreader() 
{
NHDEBUG(1, (_T("CNHMWICtrl::OnChipPowerCompleteNxrfidreader \n")));
}

void CNHMWICtrl::OnAcceptCancelledNxrfidreader()				// 2005.12.21 Add
{
NHDEBUG(1, (_T("CNHMWICtrl::OnAcceptCancelledNxrfidreader() \n")));

	m_bRFIDEntry = ENTRY_DISABLED;	
	
	DevRFIDUpdateStatus();
	ProcSetDeviceEvent(L"RFID", L"AcceptCancelled", NULL);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Method : Call From MCU to MCU
///////////////////////////////////////////////////////////////////////////////////////////////////
int CNHMWICtrl::DevRFIDUpdateStatus()
{
	if (!m_bRFIDOpened)											// 장치가Open되지 않았으면
		return R_NORMAL;										// 상태갱신은 의미없음

	CString strStatus = m_pRFID.GetDeviceStatus();				// 디바이스 상태와 매체 상태를 업데이트 한다.
NHDEBUG(1, (_T("CNHMWICtrl::DevRFIDUpdateStatus() strStatus(%s)\n"), strStatus));
	ProcSetDeviceStatus(L"RFID", L"StDeviceStatus", strStatus);
	
	if (strStatus.CompareNoCase(ST_DEVNODEVICE) != 0)
	{
		CString strMediaStatus = m_pRFID.GetMediaStatus();
		ProcSetDeviceStatus(L"RFID", L"StMediaStatus", strMediaStatus);
	}

	return R_NORMAL;
}