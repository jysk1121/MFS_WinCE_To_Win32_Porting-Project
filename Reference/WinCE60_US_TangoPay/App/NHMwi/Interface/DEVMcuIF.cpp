// DEVMCUIF.cpp
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
// Kalignite CardReader ActiveX BUGREPORT
// ------------------------------------------------------------------------------------------------
// 1. 2003.07.28
//		: 저널량 NORMAL->LOW->NORMAL로 잦은 변경에 대한 값의 잦은 변경발생
//		- 사용대기(중)에서 사용중지가 될때까지는 그 값을 유지하여야 하나,
//		  MWI에서는 기기의 모드를 알지 못한다.
//		- 따라서 모드를 알수있는 AP에서 이러한 값의 보정이 이루어져야 한다.
// ------------------------------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// Kalignite CardReader ActiveX BUGREPORT
// ------------------------------------------------------------------------------------------------
// 1. 2003.05.15
//		- 엔트리가 없는 상태에서 엔트리를 끄면 SP에서 응답없음
//		- Kalignite에서 무시되며, 이를 대비하기 위해서 플래그(m_nMcuEntry)를 사용함
// ------------------------------------------------------------------------------------------------
// 2. 2003.05.18
//		- AcceptAndReadAvailableTracks함수사용시 첫번째 인자는 읽을 트랙임
//		- "2,3" : 2트랙 데이타만 읽혀져 올라옴
//		- "1,2,3" : 2/3트랙 모두 데이타가 올라옴
// ------------------------------------------------------------------------------------------------
// 3. 2003.05.18
//		- OpenConnection처리이후 잔류매체가 회수된 경우 회수내역을 알수 없음(BRM제외)
//		- 초기화시 회수된 매체에 대한 정보 필요(SP검토필 및 BRM은 확인요망)
// ------------------------------------------------------------------------------------------------
// 4. 2003.05.19
//		- 복구처리시 카드부와 통장부는 회수할것인지 방출할 것인지 프로그램적으로 결정가능 검토
//		- 카드부는 가능한것으로 알며, 통장부도 확인필요(자동복구시는 방출, 수동시는 회수처리가능해야)
// ------------------------------------------------------------------------------------------------
// 5. 2003.06.09
//		배출이후 수취완료되면 다음 Entry발행시점까지 MediaStatus의 상태변화가 무의미하다.
//		- 1. EP의 역방향 방출
//		- 2. SP의 Event미발행
//		- 카드부/통장부 공동대응필요
// ------------------------------------------------------------------------------------------------
// 6. 2003.07.28 (검토사항)
//		카드회수시 카드미검지시 처리사양 확인(회수명령과 고객수취가 동시처리)
//		- 통장부/명세표부 마찬가지 확인필요
// ------------------------------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// Method : Call to MCU - Nextware Support 2005.11.23~
///////////////////////////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// PROTOTYPE : int CNHMWICtrl::DevMcuOpenConnection()
// DESCRIPT  : 초기 이니셜
// PARAMETER : 없음
// RETURN    : 0
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevMcuOpenConnection()
{
	NHDEBUG(1, (_T("OpenSessionSync() ... \n")));
	int nReturn = m_pMcu.OpenSessionSync(K_30_WAIT*1000);
	NHDEBUG(1, (L"\n"));
	NHDEBUG(1, (_T("OpenSessionSync() ... DONE (%d)\n"), nReturn));

	if (nReturn == R_NORMAL)
	{
		// This is to remove a fatal error generated after successful OpenSessionSync(), which was executed to open SP which was killed by force by GOMA.
		m_pDevCtrl->EvtQReset(DEV_MCU);

		ProcSetDeviceEvent(L"MCU", L"OpenComplete", NULL);
		DevMcuUpdateStatus();
	}
	else //error
	{
		ProcSetDeviceEvent(L"MCU", L"FatalError", NULL);
	}

	m_bMcuEntry = ENTRY_DISABLED;

	return nReturn;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : int CNHMWICtrl::DevMcuCloseConnection()
// DESCRIPT  : 세션종료
// PARAMETER : 없음
// RETURN    : 0
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevMcuCloseConnection()
{
	return m_pMcu.CloseSessionSync();
}

// ----------------------------------------------------------------------------
// PROTOTYPE : int CNHMWICtrl::DevMcuEject()
// DESCRIPT  : 카드부에 카드방출을 지시한다.
// PARAMETER : 없음
// RETURN    : 
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevMcuEject()
{
	m_bMcuWaitTaken = TRUE;										// 20030814_1.0_1066 : Pre-CancelWaitTaken Proc.
	return DevMcuEject(WTIME_MCUEJECT);
}

// ----------------------------------------------------------------------------
// PROTOTYPE : int CNHMWICtrl::DevMcuEject(int nWaitTakenSec)
// DESCRIPT  : 카드부에 카드방출을 지시한다  수취 대기시간 포함
// PARAMETER : 대기시간(초단위)
// RETURN    : 
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevMcuEject(int nWaitTakenSec)
{
	int nResult = R_NORMAL;

	if (nWaitTakenSec > 0) 
		nResult = m_pMcu.EjectMedia(nWaitTakenSec*1000);
	else 
		nResult = m_pMcu.EjectMedia(nWaitTakenSec);

	return nResult;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : 
// DESCRIPT  : 카드부엔트리 무한대기로 발행
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevMcuAcceptAndReadAvailableTracks(int nWaitTimeSec)
{
	// [#2250] US Justin 2014.01.27 Extract Track1 data
	//DevMcuAcceptAndReadAvailableTracks(_T("ISO1,ISO2,ISO3"), nWaitTimeSec);
	//DevMcuAcceptAndReadAvailableTracks(_T("ISO2"), nWaitTimeSec);			// 2Track Only
	DevMcuAcceptAndReadAvailableTracks(_T("ISO1,ISO2"), nWaitTimeSec);
	// End of [#2250]

	return 0;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : 
// DESCRIPT  : IC부엔트리 무한대기로 발행
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevMcuICAcceptAndReadAvailableTracks(int nWaitTimeSec)
{
NHDEBUG(1, (_T("CNHMWICtrl::DevMcuICAcceptAndReadAvailableTracks() m_bMcuEntry[%d]\n"), m_bMcuEntry));
	if (m_bMcuEntry == ENTRY_ENABLED)
	{
		ProcSetDeviceEvent(L"MCU", L"EntryEnabled", NULL);		// 20030828_1.0_1069 : EntryEnable 2중호출
		return R_NORMAL;
	}
	m_bMcuEntry = ENTRY_ENABLED;
	m_pMcu.ChipIO(0, _T("INIT"), ByteArrayToSafeArray(NULL, 0), nWaitTimeSec*1000);
	ProcSetDeviceEvent(L"MCU", L"EntryEnabled", NULL);			// 20030814_1.0_1066 : EntryEnable
	return 0;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : int CNHMWICtrl::DevMcuAcceptAndReadAvailableTracks(LPCTSTR szTracks, int nWaitTimeSec)
// DESCRIPT  : 엔트리 발행및 카드읽기
// PARAMETER : 읽을 트랙(예:"2,3"), 카드삽입대기시간(-1은 무한대기)
// RETURN    : 
// 기타사항 : 이 함수는 카드에 하나라도 유효한 데이타가 있어 읽기가 성공하면 CardAccepted를 이벤트로
//			  발생시킨다. 그러나 이와 비슷한 함수중 AcceptAndReadTracks는 지정된 트랙맵의 트랙중
//			  하나라도 유효하지 않으면 CardInvalid를 발행한다. 이점 유의하도록.....
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevMcuAcceptAndReadAvailableTracks(LPCTSTR szTracks, int nWaitTimeSec)
{
NHDEBUG(1, (_T("m_bMcuEntry [%d]\n"), m_bMcuEntry));

	if (m_bMcuEntry == ENTRY_ENABLED)							// 엔트리가 발행된 상태라면 처리하지 않는다
	{
		ProcSetDeviceEvent(L"MCU", L"EntryEnabled", NULL);
		return R_NORMAL;
	}

	m_bMcuEntry = ENTRY_ENABLED;

	int nReturn = 0;
	if (nWaitTimeSec == 0)
		nReturn = m_pMcu.ReadAvailableRawData(szTracks, nWaitTimeSec);
	else
		nReturn = m_pMcu.ReadAvailableRawData(szTracks, nWaitTimeSec*1000);

	ProcSetDeviceEvent(L"MCU", L"EntryEnabled", NULL);
	return nReturn;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : int CNHMWICtrl::DevMcuCancelAccept()
// DESCRIPT  : 카드부 엔트리발행을 취소한다.
// PARAMETER : 없음
// RETURN    : 
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevMcuCancelAccept()
{
	NHDEBUG(1, (_T("m_bMcuEntry[%d]\n"), m_bMcuEntry));

	// 엔트리가 발행되지 않은 상태라면 이벤트를 걍 발생시킨다.
	if (m_bMcuEntry == ENTRY_DISABLED)							// 엔트리가 발행되지 않은 상태라면 이벤트를 걍 발생시킨다.
	{
		ProcSetDeviceEvent(L"MCU", L"AcceptCancelled", NULL);
		return R_NORMAL;
	}

	m_bMcuEntry = ENTRY_DISABLED;

	return m_pMcu.CancelAccept();
}

// ----------------------------------------------------------------------------
// PROTOTYPE : CString CNHMWICtrl::DevMcuGetTrackData(int nTrack)
// DESCRIPT  : 카드의 데이타를 트랙별로 리턴한다.
// PARAMETER : 데이타를 알고자 하는 트랙맵
// RETURN    : 
// REMARK    : 20030917_1.0_1073 : MakeUnpack
// ----------------------------------------------------------------------------
CString CNHMWICtrl::DevMcuGetTrackData(int nTrack)
{
	CString strResult;
	char szData[2048];		// [#2022] NH KSK 2011.02.22

	memset(szData, 0x00, sizeof(szData));

	switch (nTrack)
	{
		case 1  :
			//[#2250] US Justin 2014.01.27 Read Track1 data
			if (m_pMcu.GetTrack1Status().CompareNoCase(_T("READ")) == 0)
			{
				CString strTrack1Data = m_pMcu.GetTrack1Data();

				Strcpy(szData, strTrack1Data.GetBuffer(0));
				strTrack1Data.ReleaseBuffer();
			}
			// End of [#2250]
			break;
		case 2  :
			if (m_pMcu.GetTrack2Status().CompareNoCase(_T("READ")) == 0)
			{
				CString strTrack2Data = m_pMcu.GetTrack2Data();

				Strcpy(szData, strTrack2Data.GetBuffer(0));
				strTrack2Data.ReleaseBuffer();
	
			}
			break;
		case 3  :
			if (m_pMcu.GetTrack3Status().CompareNoCase(_T("READ")) == 0)
			{
				CString strTrack3Data = m_pMcu.GetTrack3Data();

				Strcpy(szData, strTrack3Data.GetBuffer(0));
				strTrack3Data.ReleaseBuffer();

			}
			break;
		default :
			break;
	}

//	strResult = MakeUnPack(szData, strlen(szData), TRUE);
	strResult = MakeUnPack(szData, __min(strlen(szData), 1024), TRUE);		// [#2022] NH KSK 2011.02.21 Code Sonar 지적사항 대책

NHDEBUG(1, (_T("CNHMWICtrl::DevMcuGetTrackData()  nTrack(%d)  szData(%s)  Len(%d) \n"), nTrack, szData, strlen(szData)));

	return strResult;
}


// ----------------------------------------------------------------------------
// PROTOTYPE : int CNHMWICtrl::DevMcuGetStatus(int nStatus)
// DESCRIPT  : 카드디바이스의 장치상태(파라미터:1), 카드상태(파라미터:2)를 조회
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevMcuGetStatus(int nStatus)
{
	int nResult = R_NORMAL;
	CString strStatus("");

	switch(nStatus)
	{
		case ST_DEVICEVSTATUS  :
			strStatus = m_pDevCtrl->GetDeviceStatus(_T("MCUStDeviceStatus"));
			if		(strStatus.CompareNoCase(ST_DEVONLINE)	== 0)	nResult = NORMAL;
			else if	(strStatus.CompareNoCase(ST_DEVNODEVICE)== 0)	nResult = NODEVICE;
			else	nResult = DOWN;
			break;
		case ST_MEDIASTATUS  :
			strStatus = m_pDevCtrl->GetDeviceStatus(_T("MCUStMediaStatus"));
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
// PROTOTYPE : int CNHMWICtrl::DevMcuRetract()
// DESCRIPT  : 카드회수
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevMcuRetract()
{
	int nResult = R_NORMAL;

	if ((m_pMcu.GetRetainBinStatus() == "OK") ||
		(m_pMcu.GetRetainBinStatus() == "HIGH"))
		nResult = m_pMcu.RetainMedia();
	else
	{
		OnRetainCompleteNxcardreader();
	}

	return nResult;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : int CNHMWICtrl::DevMcuWriteTrack(LPCTSTR szTrackData)
// DESCRIPT  : 카드의 3트랙에 대이타를 기록한다.
// 기타사항 : Kalignite CardReader에서는 3Track만 기록이 가능하다.
// 20031029_1.1_1090 : 카드MS기록시 최대자릿수 확인
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevMcuWriteTrack(LPCTSTR szTrackData)
{
	int nResult = R_NORMAL;
	CStringArray strArray;
	CString strCardData("");

	strCardData.Format(_T(",,%s"), szTrackData);
	SplitString(strCardData, ",", strArray);

	if (m_pMcu.GetCanWriteISO3Track())
	{
		if (wcslen(szTrackData) <= 104)
			nResult = m_pMcu.WriteRawData(_T("ISO3"), StringArrayToSafeArray(strArray), _T("AUTO"));
		else
		{
			strArray[2].Format(_T("%104.104s"), szTrackData);
			nResult = m_pMcu.WriteRawData(_T("ISO3"), StringArrayToSafeArray(strArray), _T("AUTO"));
		}
	}
	else
	{
		OnWriteCompleteNxcardreader();
	}

	return nResult;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : DevMcuCancelWaitTaken()
// DESCRIPT  : 
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevMcuCancelWaitTaken()
{
	return R_NORMAL;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : int CNHMWICtrl::DevMcuEnableStatusEvents()
// DESCRIPT  : 상태변경 이벤트 발생을 가능하게 한다.
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevMcuEnableStatusEvents()
{
	return TRUE;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : int CNHMWICtrl::DevMcuDisableStatusEvents()
// DESCRIPT  : 상태변경 이벤트 발생을 불가하게 한다.
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevMcuDisableStatusEvents()
{
	return TRUE;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : int CNHMWICtrl::DevMcuResetBinCount()
// DESCRIPT  : SP에 카드회수정보 CLEAR를 요청한다.
// PARAMETER : 없음
// RETURN    : True if Success
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevMcuResetBinCount()
{
	int		nRetractCnt = 0;

	if (m_pMcu.GetRetainBinStatus() != ST_NOTSUPP)
		return m_pMcu.ResetRetainCount();
	else
	{
//		RegSetValueExt(_REGKEY_MCUSTATUS, _T("Retain_Count"), REG_INT, sizeof(nRetractCnt), &nRetractCnt);
		return 0;
	}
}

// ----------------------------------------------------------------------------
// PROTOTYPE : CString DevMcuGetErrorCode() 
// DESCRIPT  : 레지스트리에 저장된 장애코드를 확보한다.
// PARAMETER : 없음
// RETURN    : 장애코드
// ----------------------------------------------------------------------------
CString CNHMWICtrl::DevMcuGetErrorCode() 
{
	CString strResult("");

	char szTemp[10];
	memset(szTemp, NULL, sizeof(szTemp));
	memcpy(szTemp, m_pSPInform->SPErr.ErrCode_IDC, 7);
	strResult = CString(szTemp);

	return strResult;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long DevMcuClearErrorCode() 
// DESCRIPT  : 카드부의 장애코드를 클리어한다.
// PARAMETER : 없음
// RETURN    : NORMAL(0)
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevMcuClearErrorCode() 
{
//	RegSetValueExt(_REGKEY_DEVERROR, _T("IDC"), REG_STR, 0, NULL);	// 카드부
	return NORMAL;
}

// ----------------------------------------------------------------------------
// 함 수 명 : CString DevMcuGetSensorInfo
// DESCRIPT : 센서정보 조회 
// REMARK   : 20030719_1.0_1053 : GetSensorInfo
// ----------------------------------------------------------------------------
CString CNHMWICtrl::DevMcuGetSensorInfo()
{
	CString strResult("");
	TCHAR szCode[1024];		memset(szCode, 0x00, sizeof(szCode));
	int nResult = 0;

	// 레지스트리에서 장애코드를 확보한다.
//	nResult = RegQueryValueExt(_REGKEY_MCUSTATUS, _T("Sensor"), REG_STR, sizeof(szCode), (LPVOID)szCode);
//	if (nResult)
//	{
//		strResult.Format(_T("%S"), szCode);
//	}

	return strResult;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : int CNHMWICtrl::DevMcuICSendData(int ProtocolID, LPCTSTR szSendData, int nWaitSec)
// DESCRIPT  : 
// PARAMETER : 
// RETURN    : 
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevMcuICSendData(int ProtocolID, LPCTSTR szSendData, int nWaitSec)
{
	CString strToken("");
	int		nSendDataLen = 0;
	BYTE	bMakeSendData[1024];

	strToken = "SEND";
	memset(bMakeSendData, 0x00, sizeof(bMakeSendData));

	char szTemp[1024];
	memset(szTemp, NULL, sizeof(szTemp));
	WideToMulti(szTemp, szSendData, sizeof(szTemp));

	nSendDataLen = MakePack(szTemp, bMakeSendData, wcslen(szSendData));

	int nResult = m_pMcu.ChipIO(ProtocolID, 
								strToken.GetBuffer(0), 
								ByteArrayToSafeArray(bMakeSendData, nSendDataLen), 
								(nWaitSec > 0) ? (short)(nWaitSec * 1000) : (short)nWaitSec);
	strToken.ReleaseBuffer();

	return nResult;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : 
// DESCRIPT  : 
// PARAMETER : 
// RETURN    : 
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevMcuICChipInitialize()
{
	long nResult = 0;

	nResult = m_pMcu.ChipIO(0, _T("INIT"), ByteArrayToSafeArray(NULL, 0), K_15_WAIT * 1000);

	return nResult;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : 
// DESCRIPT  : 
// PARAMETER : 
// RETURN    : 
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevMcuICChipPower(long nActType)
{
	int nResult = FALSE;

// 	if ((nActType == IC_POWER_OFF) && 
// 		(DevMcuGetStatus(ST_MEDIASTATUS) != ST_SENSOR2))			// MEDIA PRESENT
// 	{
// 		ProcSetDeviceEvent(L"MCU", L"ChipPowerComplete", NULL);
// 		return TRUE;
// 	}

	if (nActType == IC_COLD_RESET)
		nResult = m_pMcu.ChipPower(_T("COLD"));
	else
	if (nActType == IC_WARM_RESET)
		nResult = m_pMcu.ChipPower(_T("WARM"));
	else
	if (nActType == IC_POWER_OFF)
		nResult = m_pMcu.ChipPower(_T("OFF"));

	return nResult;
}

/************************************************************************/
/* Event : Accepted From MCU											*/
/************************************************************************/
void CNHMWICtrl::OnFatalErrorNxcardreader(LPCTSTR Action, long Result) 
{
NHDEBUG(1, (_T("CNHMWICtrl::OnFatalErrorNxcardreader() \n")));

	m_bMcuEntry = ENTRY_DISABLED;			// [#460] [NH] KSK 2008.11.26 Disable Flag Set 시 AP에서 Disable Command에 대해 처리 못하는 Bug Fix

	DevMcuUpdateStatus();
	ProcSetDeviceEvent(L"MCU", L"FatalError", NULL);

	// [#2306] NH KSK MWI Add NVRAM Log
	m_strTemp.Empty();
	m_strTemp.Format(L"Result:%x", Result);
	NVDump('F', '1', "91", L"9999999", L"FATAL_ERR");
	NVDump('F', '1', "91", L"9999999", m_strTemp);
	// end of [#2306]
}

void CNHMWICtrl::OnDeviceErrorNxcardreader(LPCTSTR Action, long Result) 
{
NHDEBUG(1, (_T("CNHMWICtrl::OnDeviceErrorNxcardreader() \n")));

	m_bMcuEntry = ENTRY_DISABLED;								// 20030710_1.0_1050 : 엔트리없음 디버그
	m_bMcuWaitTaken = FALSE;									// 20030814_1.0_1066 : Pre-CancelWaitTaken Proc.
	m_nMcuExist = ST_NOT_DETECT;
	
	DevMcuUpdateStatus();
	ProcSetDeviceEvent(L"MCU", L"DeviceError", NULL);

	// [#2306] NH KSK MWI Add NVRAM Log
	m_strTemp.Empty();
	m_strTemp.Format(L"RESULT:%x", Result);
	NVDump('F', '1', "91", L"9999999", L"DEV_ERR");
	NVDump('F', '1', "91", L"9999999", m_strTemp);
	// end of [#2306]
}

void CNHMWICtrl::OnLockGrantedNxcardreader() 
{
}

void CNHMWICtrl::OnLockReleasedNxcardreader() 
{
}

void CNHMWICtrl::OnLockTimeoutNxcardreader() 
{
}

void CNHMWICtrl::OnDeviceStatusChangedNxcardreader(LPCTSTR Value) 
{
// [#272] NH AIREAT 2008.06.09 - 공통 루틴으로 적용
NHDEBUG(1, (_T("CNHMWICtrl::OnDeviceStatusChangedNxcardreader() Value(%s)\n"), Value));
	// [#68] UK HWANG 2008.04.02 EMV Level2 (source review 수정 - define)
 	DevMcuUpdateStatus();
	// end of [#68]
// [#272] NH AIREAT 2008.06.09 - 공통 루틴으로 적용

	// [#2306] NH KSK MWI Add NVRAM Log
	m_strTemp.Empty();
	m_strTemp.Format(L"VAL:%s", Value);
	NVDump('O', '1', "91", L"", L"DEV_C_STS");
	NVDump('O', '1', "91", L"", m_strTemp);
	// end of [#2306]
}

void CNHMWICtrl::OnMediaStatusChangedNxcardreader(LPCTSTR Value) 
{
// [#272] NH AIREAT 2008.06.09 - 공통 루틴으로 적용
NHDEBUG(1, (_T("CNHMWICtrl::OnMediaStatusChangedNxcardreader() Value(%s)\n"), Value));
	// [#68] UK HWANG 2008.04.02 EMV Level2 (source review 수정 - define)
	DevMcuUpdateStatus();
	// end of [#68]
// [#272] NH AIREAT 2008.06.09 - 공통 루틴으로 적용

	// [#2306] NH KSK MWI Add NVRAM Log
	m_strTemp.Format(L"MDA:%s", Value);
	NVDump('O', '1', "91", L"", L"MDA_C_STS");
	NVDump('O', '1', "91", L"", m_strTemp);
	// end of [#2306]
}

void CNHMWICtrl::OnRetainBinStatusChangedNxcardreader(LPCTSTR Value) 
{
NHDEBUG(1, (_T("CNHMWICtrl::OnRetainBinStatusChangedNxcardreader(%s) \n"), Value));
}

void CNHMWICtrl::OnSecurityStatusChangedNxcardreader(LPCTSTR Value) 
{
NHDEBUG(1, (_T("CNHMWICtrl::OnSecurityStatusChangedNxcardreader(%s) \n"), Value));
}

void CNHMWICtrl::OnCardsStatusChangedNxcardreader(short Value) 
{
NHDEBUG(1, (_T("CNHMWICtrl::OnCardsStatusChangedNxcardreader(%s) \n"), Value));

	// [#2306] NH KSK MWI Add NVRAM Log
	m_strTemp.Empty();
	m_strTemp.Format(L"VAL:%s", Value);
	NVDump('O', '1', "91", L"", L"C_S_C");
	NVDump('O', '1', "91", L"", m_strTemp);
	// end of [#2306]
}

void CNHMWICtrl::OnReadCompleteNxcardreader() 
{
NHDEBUG(1, (_T("CNHMWICtrl::OnReadCompleteNxcardreader() \n")));

	m_bMcuEntry = ENTRY_DISABLED;								// 20030710_1.0_1050 : 엔트리없음 디버그

	m_nMcuExist = ST_SENSOR2;

	DevMcuUpdateStatus();										// 모든 상태값을 업데이트한다.

	ProcSetDeviceEvent(L"MCU", L"ReadComplete", NULL);
}

void CNHMWICtrl::OnWriteCompleteNxcardreader() 
{
NHDEBUG(1, (_T("CNHMWICtrl::OnWriteCompleteNxcardreader() \n")));

	DevMcuUpdateStatus();

	ProcSetDeviceEvent(L"MCU", L"WriteComplete", NULL);
}

void CNHMWICtrl::OnEjectCompleteNxcardreader() 
{
NHDEBUG(1, (_T("CNHMWICtrl::OnEjectCompleteNxcardreader() \n")));

	DevMcuUpdateStatus();

	m_nMcuExist = ST_SENSOR1;

	ProcSetDeviceEvent(L"MCU", L"EjectComplete", NULL);
}

void CNHMWICtrl::OnRetainCompleteNxcardreader() 
{
NHDEBUG(1, (_T("CNHMWICtrl::OnRetainCompleteNxcardreader() \n")));

	DevMcuUpdateStatus();										// 모든 상태값을 업데이트한다.

	ProcSetDeviceEvent(L"MCU", L"RetainComplete", NULL);
}

void CNHMWICtrl::OnChipIOCompleteNxcardreader(LPCTSTR Token, const VARIANT FAR& Data) 
{
NHDEBUG(1, (_T("CNHMWICtrl::OnChipIOCompleteNxcardreader() \n")));

	int		nLen = 0;
	BYTE*	pICData;

	m_bMcuEntry = ENTRY_DISABLED;			// [#460] [NH] KSK 2008.11.26 Disable Flag Set 시 AP에서 Disable Command에 대해 처리 못하는 Bug Fix

	pICData = NULL;
	pICData = VarArrayToByteArray(Token, &Data, &nLen);

	VariantClear(&(VARIANT)Data);

	m_strICData = "";
	m_strICData.Format(_T("%s"), MakeUnPack(pICData, nLen));

	DevMcuUpdateStatus();										// 모든 상태값을 업데이트한다.
	ProcSetDeviceEvent(L"MCU", L"ChipIOComplete", NULL);
}

void CNHMWICtrl::OnChipIOFailureNxcardreader(LPCTSTR Token) 
{
NHDEBUG(1, (_T("CNHMWICtrl::OnChipIOFailureNxcardreader() \n")));

	m_bMcuEntry = ENTRY_DISABLED;			// [#460] [NH] KSK 2008.11.26 Disable Flag Set 시 AP에서 Disable Command에 대해 처리 못하는 Bug Fix

	DevMcuUpdateStatus();
	m_strICData = "";

	// [#68] HWANG 2008.04.02 EMV Level2 : 협의 필요
	ProcSetDeviceEvent(L"MCU", L"ChipIOComplete", NULL);
//	ProcSetDeviceEvent(L"MCU", L"FatalError", NULL);
	// end of [#68]
//	ProcSetDeviceEvent(L"MCU", L"ChipIOFailure", NULL);

	NVDump('F', '1', "91", L"9999999", L"CHIP_IO_FAIL");	// [#2306] NH KSK MWI Add NVRAM Log
}

void CNHMWICtrl::OnMediaInsertedNxcardreader() 
{
NHDEBUG(1, (_T("CNHMWICtrl::OnMediaInsertedNxcardreader() \n")));

//	m_bMcuEntry = ENTRY_DISABLED;								// 20030710_1.0_1050 : 엔트리없음 디버그	[#460] [NH] KSK 2008.11.26 Disable Flag Set 시 AP에서 Disable Command에 대해 처리 못하는 Bug Fix

	m_nMcuExist = ST_SENSOR2;

	DevMcuUpdateStatus();										// 모든 상태값을 업데이트한다.

	ProcSetDeviceEvent(L"MCU", L"MediaInserted", NULL);

	NVDump('O', '1', "91", L"", L"INSERTED");	// [#2306] NH KSK MWI Add NVRAM Log
}

void CNHMWICtrl::OnInvalidMediaNxcardreader() 
{
NHDEBUG(1, (_T("CNHMWICtrl::OnInvalidMediaNxcardreader() \n")));

	m_bMcuEntry = ENTRY_DISABLED;								// 20030710_1.0_1050 : 엔트리없음 디버그
	m_nMcuExist = ST_SENSOR2;

	DevMcuUpdateStatus();										// 모든 상태값을 업데이트한다.

	ProcSetDeviceEvent(L"MCU", L"InvalidMedia", NULL);

	NVDump('F', '1', "91", L"9999999", L"INVALID_MEA");	// [#2306] NH KSK MWI Add NVRAM Log
}

void CNHMWICtrl::OnInvalidTrackDataNxcardreader() 
{
NHDEBUG(1, (_T("CNHMWICtrl::OnInvalidTrackDataNxcardreader() \n")));

	m_bMcuEntry = ENTRY_DISABLED;								// 20030710_1.0_1050 : 엔트리없음 디버그
	m_nMcuExist = ST_SENSOR2;

	DevMcuUpdateStatus();										// 모든 상태값을 업데이트한다.

	ProcSetDeviceEvent(L"MCU", L"InvalidTrackData", NULL);

	NVDump('F', '1', "91", L"9999999", L"INVALID_TRACK");	// [#2306] NH KSK MWI Add NVRAM Log
}

void CNHMWICtrl::OnMediaRemovedNxcardreader() 
{
NHDEBUG(1, (_T("CNHMWICtrl::OnTimeoutNxcardreader() \n")));

	m_bMcuWaitTaken = FALSE;									// 20030814_1.0_1066 : Pre-CancelWaitTaken Proc.
	m_nMcuExist = ST_NOT_DETECT;

	DevMcuUpdateStatus();										// 모든 상태값을 업데이트한다.

	NVDump('O', '1', "91", L"", L"MEA_REMOVE");	// [#2306] NH KSK MWI Add NVRAM Log

//	ProcSetDeviceEvent(L"MCU", L"MediaRemoved", NULL);
}

void CNHMWICtrl::OnRetainBinThresholdNxcardreader() 
{
}

void CNHMWICtrl::OnTimeoutNxcardreader() 
{
NHDEBUG(1, (_T("CNHMWICtrl::OnTimeoutNxcardreader() \n")));

	m_bMcuEntry = ENTRY_DISABLED;
	m_bMcuWaitTaken = FALSE;

	DevMcuUpdateStatus();
	ProcSetDeviceEvent(L"MCU", L"Timeout", NULL);
}

void CNHMWICtrl::OnChipPowerStatusChangedNxcardreader(LPCTSTR Value) 
{
	NHDEBUG(1, (_T("[]: %s \n"), Value));

}

void CNHMWICtrl::OnMediaDetectedNxcardreader(LPCTSTR Value) 
{
	NHDEBUG(1, (_T("[]: %s \n"), Value));
	NVDump('O', '1', "91", L"", L"MEA_DETECT");	// [#2306] NH KSK MWI Add NVRAM Log
	
}

void CNHMWICtrl::OnResetCompleteNxcardreader() 
{
NHDEBUG(1, (_T("CNHMWICtrl::OnResetCompleteNxcardreader() \n")));

	DevMcuUpdateStatus();										// 모든 상태값을 업데이트한다.

	int nDeviceStatus = DevMcuGetStatus(ST_DEVICEVSTATUS);		// 장치의 상태값을 확인한다.

	if (nDeviceStatus != NORMAL)								// 장애검지시(FATAL/*NODEVICE*)
	{
		CString strErrorCode = "9792300";
		m_pDevCtrl->WriteMwiErrorCode(DEV_MCU, "IDC", strErrorCode.GetLength(), "9792300");
		ProcSetDeviceEvent(L"MCU", L"FatalError", NULL);
	}
	else /* NORMAL */
	{
		ProcSetDeviceEvent(L"MCU", L"ResetComplete", NULL);

		DevMcuEnableStatusEvents();								// 상태변경 이벤트를 Enable처리한다.
	}
}

void CNHMWICtrl::OnChipPowerCompleteNxcardreader() 
{
NHDEBUG(1, (_T("CNHMWICtrl::OnChipPowerCompleteNxcardreader() \n")));

	DevMcuUpdateStatus();
	ProcSetDeviceEvent(L"MCU", L"ChipPowerComplete", NULL);

	NVDump('O', '1', "91", L"", L"POWER_COMP");	// [#2306] NH KSK MWI Add NVRAM Log
}

void CNHMWICtrl::OnAcceptCancelledNxcardreader()				// 2005.12.21 Add
{
NHDEBUG(1, (_T("CNHMWICtrl::OnAcceptCancelledNxcardreader() \n")));

	m_bMcuEntry = ENTRY_DISABLED;								// 20030710_1.0_1050 : 엔트리없음 디버그
	
	DevMcuUpdateStatus();
	ProcSetDeviceEvent(L"MCU", L"AcceptCancelled", NULL);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Method : Call From MCU to MCU
///////////////////////////////////////////////////////////////////////////////////////////////////
int CNHMWICtrl::DevMcuUpdateStatus()
{
																// AP DOWN 장애현상 디버그 : 2003.12.20
	if (!m_bMcuOpened)											// 장치가Open되지 않았으면
		return R_NORMAL;										// 상태갱신은 의미없음

	CString strStatus = m_pMcu.GetDeviceStatus();				// 디바이스 상태와 매체 상태를 업데이트 한다.
	NHDEBUG(1, (_T("DeviceStatus(%s)\n"), strStatus));
	ProcSetDeviceStatus(L"MCU", L"StDeviceStatus", strStatus);
	
	if (strStatus.CompareNoCase(ST_DEVNODEVICE) != 0)
	{
		CString strMediaStatus = m_pMcu.GetMediaStatus();
		ProcSetDeviceStatus(L"MCU", L"StMediaStatus", strMediaStatus);
		NHDEBUG(1, (_T("MediaStatus(%s)\n"), strMediaStatus));
	}

//	int nRetractCnt = m_pMcu.GetRetainCount();
//	RegSetValueExt(_REGKEY_MCUSTATUS, _T("Retain_Count"), REG_INT, sizeof(nRetractCnt), &nRetractCnt);

	return R_NORMAL;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : int DevMcuPreCancelWaitTaken()
// DESCRIPT  : 매체수취대기 FLAG를 확인하여 대기중이면 수취대기를 취소한다
// PARAMETER : 없음
// RETURN    : 0
// ----------------------------------------------------------------------------
int CNHMWICtrl::DevMcuPreCancelWaitTaken()
{
	if (m_bMcuWaitTaken)
	{
		DevMcuCancelWaitTaken();
	}
	m_bMcuWaitTaken = FALSE;
	return R_NORMAL;
}
