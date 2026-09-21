// MWIMEthod.cpp

#include "StdAfx.h"
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
// PROTOTYPE : BSTR CNHMWICtrl::TestMethod(LPCTSTR szParam1, LPCTSTR szParam2, LPCTSTR szParam3) 
// DESCRIPT  : 말그대로 테스트 메소드...더미함수이며, 
//		  	   테스트하고자 하는 처리를 넣어서 처리확인 인터페이스임
// PARAMETER : 문자열로 3개(필요한 파라미터 넘겨서 확인하도록)
// RETURN    : 더미로 문자열을 넘김
// FROm		 : AP
// ----------------------------------------------------------------------------
BSTR CNHMWICtrl::TestMethod(LPCTSTR szParam1, LPCTSTR szParam2, LPCTSTR szParam3) 
{
	CString strResult("");

	PinEntryEnable(2, 4, 12, FALSE, _T("NUMBERS,CANCEL,CLEAR,ENTER"), _T("ENTER"), _T("1234567891230"), -1);

	return strResult.AllocSysString();
}


// ----------------------------------------------------------------------------
// PROTOTYPE : long CNHMWICtrl::ClearEventDevice(long nDevID) 
// DESCRIPT  : 이벤트 디바이스를 초기화한다.
// PARAMETER : nDevID-장치ID
// RETURN    : 이벤트 디바이스의 장치ID
// ----------------------------------------------------------------------------
long CNHMWICtrl::ClearEventDevice(long nDevID) 
{
	m_nEventDevice &= nDevID;
	return (m_nEventDevice);
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long CNHMWICtrl::ClearStatusDevice(long nDevID) 
// DESCRIPT  : 이벤트 디바이스를 초기화한다.
// PARAMETER : nDevID-장치ID
// RETURN    : 이벤트 디바이스의 장치ID
// ----------------------------------------------------------------------------
long CNHMWICtrl::ClearStatusDevice(long nDevID) 
{
	m_nStatusDevice &= nDevID;
	return (m_nStatusDevice);
}


// ----------------------------------------------------------------------------
// PROTOTYPE : long CNHMWICtrl::GetErrorDevice(long nDevID) 
// DESCRIPT  : 장애디바이스를 리턴한다.
// PARAMETER : nDevID-장치ID
// RETURN    : 장애 디바이스의 장치ID
// ----------------------------------------------------------------------------
long CNHMWICtrl::GetErrorDevice(long nDevID) 
{
	return (nDevID & m_nErrorDevice);
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long CNHMWICtrl::GetTimeoutDevice(long nDevID) 
// DESCRIPT  : T/O 디바이스를 리턴한다.
// PARAMETER : nDevID-장치ID
// RETURN    : T/O 디바이스의 장치ID
// ----------------------------------------------------------------------------
long CNHMWICtrl::GetTimeoutDevice(long nDevID) 
{
	return (nDevID & m_nTimeoutDevice);
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long CNHMWICtrl::GetEventDevice(long nDevID) 
// DESCRIPT  : 이벤트 디바이스를 리턴한다.
// PARAMETER : nDevID-장치ID
// RETURN    : 이벤트 디바이스의 장치ID
// ----------------------------------------------------------------------------
long CNHMWICtrl::GetEventDevice(long nDevID) 
{
	return (nDevID & m_nEventDevice);
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long CNHMWICtrl::GetStatusDevice(long nDevID) 
// DESCRIPT  : 이벤트 디바이스를 리턴한다.
// PARAMETER : nDevID-장치ID
// RETURN    : 이벤트 디바이스의 장치ID
// ----------------------------------------------------------------------------
long CNHMWICtrl::GetStatusDevice(long nDevID) 
{
	return (nDevID & m_nStatusDevice);
}


// ----------------------------------------------------------------------------
// PROTOTYPE : BSTR CNHMWICtrl::CheckDeviceAction(long nDevID, long nWaitSec) 
// DESCRIPT  : 장치의 동작완료 상태를 확인한다.
// PARAMETER : nDevID-장치ID, nWaitSec-확인시간(초단위)
// RETURN    : NORMAL/TIMEOVER/ERROR/TIMEOUT
// ----------------------------------------------------------------------------
BSTR CNHMWICtrl::CheckDeviceAction(long nDevID, long nWaitSec) 
{
	CString		strResult("");
	int			nResult = R_NORMAL;
	CSingleLock sLock(&m_mutex);
	
	if (sLock.Lock((nWaitSec + K_10_WAIT) * 1000))				// Lock Ok : 2005.05.26
	{
		nResult = m_pDevCtrl->CheckDeviceAction(nDevID, nWaitSec);
	}
	else														// Lock Fail : 2005.05.26
	{
		m_nErrorDevice = DEV_NONE;
		m_nErrorDevice |= nDevID;								// ERROR-Device Set

		CString strErrorCode("");
		strErrorCode = "9791@99";								// LOCK TIMEOVER : 2005.05.26
		char szErrorCode[32];
		memset(szErrorCode, NULL, sizeof(szErrorCode));
		WideToMulti(szErrorCode, strErrorCode, sizeof(szErrorCode));
		
		if (m_nErrorDevice & DEV_SPR)
			m_pDevCtrl->WriteMwiErrorCode(DEV_SPR, "SPR", strErrorCode.GetLength(), szErrorCode);
		if (m_nErrorDevice & DEV_JPR)
			m_pDevCtrl->WriteMwiErrorCode(DEV_JPR, "JPR", strErrorCode.GetLength(), szErrorCode);
		if (m_nErrorDevice & DEV_MCU)
			m_pDevCtrl->WriteMwiErrorCode(DEV_MCU, "IDC", strErrorCode.GetLength(), szErrorCode);
		if (m_nErrorDevice & DEV_CDU)
			m_pDevCtrl->WriteMwiErrorCode(DEV_CDU, "CDM", strErrorCode.GetLength(), szErrorCode);
		if (m_nErrorDevice & DEV_PIN)
			m_pDevCtrl->WriteMwiErrorCode(DEV_PIN, "PINPAD", strErrorCode.GetLength(), szErrorCode);
		if (m_nErrorDevice & DEV_RFID)																	// [#2325] NH KSK 2015.01.20
			m_pDevCtrl->WriteMwiErrorCode(DEV_RFID, "RFID", strErrorCode.GetLength(), szErrorCode);		// end of [#2325]
		// [#GLDV-3005] US Kook 2021.12.21 Support Side Car
		if (m_nErrorDevice & DEV_BCR)
			m_pDevCtrl->WriteMwiErrorCode(DEV_BCR, "BCR", strErrorCode.GetLength(), szErrorCode);
		if (m_nErrorDevice & DEV_BNA)
			m_pDevCtrl->WriteMwiErrorCode(DEV_BNA, "BNA", strErrorCode.GetLength(), szErrorCode);
		// end of [#GLDV-3005]

		nResult = R_TIMEOVER;
	}

	if (sLock.IsLocked())										// Is Locked
		sLock.Unlock();											// Un Lock

	switch (nResult)
	{
		case R_NORMAL :
			strResult = "NORMAL";
			break;
		case R_TIMEOUT:
			strResult = "TIMEOUT";
			break;
		case R_TIMEOVER:
			strResult = "TIMEOVER";
			break;
		case R_ERROR:
			strResult = "ERROR";
			break;
		default:
			strResult = "TIMEOVER";
			break;
	}

	return strResult.AllocSysString();
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long CNHMWICtrl::ScanDeviceAction(long nDevID, long nWaitSec, long nEventKind) 
// DESCRIPT  : 장치별로 이벤트가 발생하였는지를 확인한다.
// PARAMETER : nDevID-장치ID, nWaitSec-확인시간(초단위), nEventKind-확인하고자 하는 이벤트종류
// RETURN    : 장치ID
// ----------------------------------------------------------------------------
long CNHMWICtrl::ScanDeviceAction(long nDevID, long nWaitSec, long nEventKind) 
{
	int nResult = m_pDevCtrl->ScanDeviceAction(nDevID, nWaitSec, nEventKind);

	return nResult;
}

// ----------------------------------------------------------------------------
// PROTOTYPE : long ProcBackupKalTrace(LPCTSTR szFileName, long nFileSize) 
// DESCRIPT  : Kalignite or Nextware Trace를 Backup한다.
// PARAMETER : 파일명, 파일사이즈(바이트단위)
// RETURN    : 0(if Success else return -2)
// REMARK    : 현재는 파라미터로 파일명과 그 사이즈만을 취하나, 향후에는 추출조건(Filter)을
//			   파라미터로 받아서 이를 활용하는 방안도 고려하여야 한다.
// ----------------------------------------------------------------------------
long CNHMWICtrl::ProcBackupTrace(LPCTSTR szFileName, long nFileSize)
{
	return R_NORMAL;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Method : Called From SP OR MWI
///////////////////////////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// PROTOTYPE : void CNHMWICtrl::ProcSetDeviceEvent(LPCTSTR szDeviceName, LPCTSTR szEventName, LPCTSTR szEventValue) 
// DESCRIPT  : 디바이스(혹은SP)에서 발생된 이벤트를 처리한다.(일단 큐에 저장)
// PARAMETER : 
// RETURN    : 없음
// FROM		: SP/MWI
// ----------------------------------------------------------------------------
void CNHMWICtrl::ProcSetDeviceEvent(LPCTSTR szDeviceName, LPCTSTR szEventName, LPCTSTR szEventValue) 
{
	char DeviceName[64] = {};
	WideToMulti(DeviceName, szDeviceName, sizeof(DeviceName));

	char EventName[128] = {};
	WideToMulti(EventName, szEventName, sizeof(EventName));

	char EventValue[128] = {};
	WideToMulti(EventValue, szEventValue, sizeof(EventValue));

	m_pDevCtrl->EvtQPutData(MWI_EVENT_DEVICE, DeviceName, EventName, EventValue);
																// 2004.07.05
	if (strncmp(DeviceName, DEVNM_MCU,		strlen(DeviceName)) == 0)
		m_nEventDevice |= DEV_MCU;								
	if (strncmp(DeviceName, DEVNM_RFID,		strlen(DeviceName)) == 0)	// [#2325] NH KSK 2015.01.20
		m_nEventDevice |= DEV_RFID;										// end of [#2325]
	if (strncmp(DeviceName, DEVNM_SPR,		strlen(DeviceName)) == 0)
		m_nEventDevice |= DEV_SPR;								
	if (strncmp(DeviceName, DEVNM_JPR,		strlen(DeviceName)) == 0)
		m_nEventDevice |= DEV_JPR;								
	if (strncmp(DeviceName, DEVNM_CDU,		strlen(DeviceName)) == 0)
		m_nEventDevice |= DEV_CDU;								
	if (strncmp(DeviceName, DEVNM_PIN,		strlen(DeviceName)) == 0)
		m_nEventDevice |= DEV_PIN;								
	if (strncmp(DeviceName, DEVNM_DOR,		strlen(DeviceName)) == 0)
		m_nEventDevice |= DEV_DOR;								
	if (strncmp(DeviceName, DEVNM_SIU,		strlen(DeviceName)) == 0)
		m_nEventDevice |= DEV_SIU;								
	if (strncmp(DeviceName, DEVNM_SCR,		strlen(DeviceName)) == 0)
		m_nEventDevice |= DEV_SCR;								
	if (strncmp(DeviceName, DEVNM_SNS,		strlen(DeviceName)) == 0)
		m_nEventDevice |= DEV_SNS;
	// [#GLDV-3005] US Kook 2021.10.21 Support Side Car
	if (strncmp(DeviceName, DEVNM_BCR,		strlen(DeviceName)) == 0)
		m_nEventDevice |= DEV_BCR;
	if (strncmp(DeviceName, DEVNM_BNA,		strlen(DeviceName)) == 0)
		m_nEventDevice |= DEV_BNA;
	// end of [#GLDV-3005]
}

// ----------------------------------------------------------------------------
// PROTOTYPE : void CNHMWICtrl::ProcSetDeviceStatus(LPCTSTR szDeviceName, LPCTSTR szProperty, LPCTSTR szStatus) 
// DESCRIPT  : 스크립트 혹은 SP에서 조회된 상태값을 저장하고 공유메모리 저장처리를 수행하여야 한다.
// PARAMETER : 장치명, 속성명, 상태값
// RETURN    : 없음
// 기타사항 : 디바이스 상태값을 MWI내부에서 저장하든 말든 크게 상관은 없다. 다만 
//			  확장개념 지원을 위해서 공유메모리에 저장하는 것을 잊지는 말아야 한다.
//			  2003.04.24 : 현재까지는 공유메모리 구조가 없다. 지원대는대로 저장처리를 하여야 한다.
// ----------------------------------------------------------------------------
void CNHMWICtrl::ProcSetDeviceStatus(LPCTSTR szDeviceName, LPCTSTR szProperty, LPCTSTR szStatus) 
{
	CString strTmp;
	strTmp = CString(szDeviceName);
	strTmp = strTmp + CString(szProperty);

	char DeviceName[64] = {};
	WideToMulti(DeviceName, szDeviceName, sizeof(DeviceName));

	m_pDevCtrl->AddDeviceStatus(strTmp, szStatus);
																// 2004.07.05
	if (strncmp(DeviceName, DEVNM_MCU,		strlen(DeviceName)) == 0)
		m_nStatusDevice |= DEV_MCU;								
	if (strncmp(DeviceName, DEVNM_RFID,		strlen(DeviceName)) == 0)	// [#2325] NH KSK 2015.01.20
		m_nStatusDevice |= DEV_RFID;									// end of [#2325]
	if (strncmp(DeviceName, DEVNM_SPR,		strlen(DeviceName)) == 0)
		m_nStatusDevice |= DEV_SPR;								
	if (strncmp(DeviceName, DEVNM_JPR,		strlen(DeviceName)) == 0)
		m_nStatusDevice |= DEV_JPR;								
	if (strncmp(DeviceName, DEVNM_CDU,		strlen(DeviceName)) == 0)
		m_nStatusDevice |= DEV_CDU;								
	if (strncmp(DeviceName, DEVNM_PIN,		strlen(DeviceName)) == 0)
		m_nStatusDevice |= DEV_PIN;								
	if (strncmp(DeviceName, DEVNM_DOR,		strlen(DeviceName)) == 0)
		m_nStatusDevice |= DEV_DOR;								
	if (strncmp(DeviceName, DEVNM_SIU,		strlen(DeviceName)) == 0)
		m_nStatusDevice |= DEV_SIU;								
	if (strncmp(DeviceName, DEVNM_SCR,		strlen(DeviceName)) == 0)
		m_nStatusDevice |= DEV_SCR;								
	if (strncmp(DeviceName, DEVNM_SNS,		strlen(DeviceName)) == 0)
		m_nStatusDevice |= DEV_SNS;			
	// [#GLDV-3005] US Kook 2021.10.21 Support Side Car
	if (strncmp(DeviceName, DEVNM_BCR,		strlen(DeviceName)) == 0)
		m_nStatusDevice |= DEV_BCR;
	if (strncmp(DeviceName, DEVNM_BNA,		strlen(DeviceName)) == 0)
		m_nStatusDevice |= DEV_BNA;
	// end of [#GLDV-3005]
}
