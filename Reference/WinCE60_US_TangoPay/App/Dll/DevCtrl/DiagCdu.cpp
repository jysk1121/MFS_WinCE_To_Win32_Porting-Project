#include "stdafx.h"
#include ".\Dev\DiagCdu.h"

//#define NH_DEBUG
#include ".\Common\CmnLib.h"
#include ".\Common\NHDbgApi.h"

// 내부 Thread에게 동작을 명령하는 CMD

enum CDU_CMD
{
	CMD_NONE = 0,
	CDM_DEVICE_OPEN,
	CDM_DEVICE_CLOSE,
	CMD_INITIALIZE,
	CMD_DISPENSE,
	CMD_CDUSETUP,
	CMD_GET_VERSION,
	CMD_GET_SENSOR,
	CMD_GET_AUTOADJUSTSENSOR,
	CMD_GET_LAST_ERROR,
};

//Country Type
// [#2048] NH KSK 2011.04.13 WINCE에서 사용하는 국가만 선택하도록 변경 및 뉴질랜드 국가 추가 (사양 협의 사항 2011.04.13)
// CONST UINT	COUNTRY_INFO_SIZE = 19;
// CONST CString COUNTRY_INFO[COUNTRY_INFO_SIZE][3] = {
// 	{	_T("USA"),			_T("U"),	_T("USD")	},
// 	{	_T("CANADA"),		_T("A"),	_T("CAD")	},
// 	{	_T("KOREA"),		_T("K"),	_T("KRW")	},
// 	{	_T("CHINA"),		_T("C"),	_T("CHD")	},
// 	{	_T("JAPAN"),		_T("J"),	_T("JPY")	},
// 	{	_T("MEXICO"),		_T("M"),	_T("MXN")	},
// 	{	_T("DEUTSCH"),		_T("D"),	_T("DEM")	},
// 	{	_T("INDONESIA"),	_T("i"),	_T("IDR")	},
// 	{	_T("ENGLAND"),		_T("B"),	_T("UKP")	},
// 	{	_T("TURKEY"),		_T("t"),	_T("TUL")	},
// 	{	_T("ROMANIA"),		_T("R"),	_T("RMR")	},
// 	{	_T("AUSTRALIA"),	_T("H"),	_T("AUD")	},
// 	{	_T("INDIA"),		_T("I"),	_T("IND")	},
// 	{	_T("TAIWAN"),		_T("T"),	_T("TWD")	},
// 	{	_T("EURO"),			_T("E"),	_T("EUE")	},
// 	{	_T("VIETNAM"),		_T("V"),	_T("VND")	},
// 	{	_T("UKRAINA"),		_T("u"),	_T("UKH")	},
// 	{	_T("ISRAEL"),		_T("S"),	_T("IRS")	},
// 	{	_T("OTHER"),		_T("O"),	_T("OOO")	},
// };
CONST UINT	COUNTRY_INFO_SIZE = 6;
CONST CString COUNTRY_INFO[COUNTRY_INFO_SIZE][3] = {
	{	_T("USA"),			_T("U"),	_T("USD")	},
	{	_T("CANADA"),		_T("A"),	_T("CAD")	},
	{	_T("MEXICO"),		_T("M"),	_T("MXN")	},
	{	_T("AUSTRALIA"),	_T("H"),	_T("AUD")	},
	{	_T("NEW ZEALAND"),	_T("N"),	_T("NZD")	},
	{	_T("OTHER"),		_T("O"),	_T("OOO")	}
};
// end of [#2048]

CDiagCdu::CDiagCdu()
{
	m_bInit = FALSE;
	m_hCDU_DLL = NULL;
	m_bPortOpen = FALSE;
	m_bCDUInfo	= FALSE;
	m_bError	= FALSE;

	/* CDU DEVICE INFORMATION */
	m_CSTNum		= 0;
	m_CSTType		= 0;
	memset(m_CSTIndex, 0x00, sizeof(m_CSTIndex));
	memset(m_CSTCountry, 0x00, sizeof(m_CSTCountry));

	// Load Dll
	m_hCDU_DLL = ::LoadLibrary(VDM_CDU_FULL_PATH);
	if (m_hCDU_DLL != NULL)
	{
NHDEBUG(1, (_T(" CDiagCdu::CDiagCdu() LoadLibrary SUCCESS : m_hCDU_DLL 0x%x\n"), m_hCDU_DLL));
		/* CDU DLL Function Map */
		m_CDU_FUNC.CDM_OpenDevice				= (BOOL_FUNCTION_VOID*)GetProcAddress(m_hCDU_DLL, _LT("CDM_OpenDevice"));
		m_CDU_FUNC.CDM_CloseDevice				= (BOOL_FUNCTION_VOID*)GetProcAddress(m_hCDU_DLL, _LT("CDM_CloseDevice"));
		m_CDU_FUNC.CDM_Initialize				= (DWORD_FUNCTION_VOID*)GetProcAddress(m_hCDU_DLL, _LT("CDM_Initialize"));
		m_CDU_FUNC.CDM_GetVersion				= (DWORD_FUNCTION_PUSHORT_4ULONG_4TCHAR_PUSHORT*)GetProcAddress(m_hCDU_DLL, _LT("CDM_GetVersion"));
		m_CDU_FUNC.CDM_Dispense					= (DWORD_FUNCTION_4ULONG_4ULONG_PULONG*)GetProcAddress(m_hCDU_DLL, _LT("CDM_Dispense"));
		m_CDU_FUNC.CDM_GetSensorInfo			= (DWORD_FUNCTION_LPBYTE_PWORD*)GetProcAddress(m_hCDU_DLL, _LT("CDM_GetSensorInfo"));
		m_CDU_FUNC.CDM_SetCdmInfo				= (DWORD_FUNCTION_USHORT_4ULONG_4TCHAR_USHORT*)GetProcAddress(m_hCDU_DLL, _LT("CDM_SetCdmInfo"));
		m_CDU_FUNC.CDM_GetErrorCode				= (VOID_FUNCTION_6BYTE*)GetProcAddress(m_hCDU_DLL, _LT("CDM_GetErrorCode"));
		m_CDU_FUNC.CDM_SetDenominationFlagSet	= (DWORD_FUNCTION_INT*)GetProcAddress(m_hCDU_DLL, _LT("CDM_SetDenominationFlagSet"));
		m_CDU_FUNC.CDM_GetAutoAdjustSensor		= (DWORD_FUNCTION_USHORT_PCONDIMMINGINGINFO_PVALUEREAD_PADVALUEREAD*)GetProcAddress(m_hCDU_DLL, _LT("CDM_GetAutoAdjustSensor"));
		m_bInit = TRUE;
	}
	else
	{
NHDEBUG(1, (_T(" CDiagCdu::CDiagCdu() LoadLibrary FAIL :  m_hCDU_DLL 0x%x\n"), m_hCDU_DLL));
	}

	Resume();
}

CDiagCdu::~CDiagCdu()
{
	m_bDoingThread = FALSE;
	WaitForExitThread();

	// DLL Unload
	if (m_hCDU_DLL != NULL)
	{
		::FreeLibrary(m_hCDU_DLL);
		m_hCDU_DLL = NULL;
		
		memset(&m_CDU_FUNC, 0, sizeof(m_CDU_FUNC));
	}
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDiagCdu
 FUNCTION NAME: CompleteAction()
 RETURN TYPE  : TRUE : 호출한 Method의 동작이 완료 되었다.
				FALSE : 호출한 Method가 동작 중이다.
 PARAMETER    : 
 DESCRIPTION  : ASync로 호출한 Method의 결과를 조회한다.
-------------------------------------------------------------------*/
BOOL CDiagCdu::CompleteAction(int nRequestID, DWORD dwWaitMs)
{
	CNHCtrlCmdInfo	Response;
	
	if (nRequestID == -1)
		return FALSE;
	
	if (m_ResponseQueue.Dequeue(Response, dwWaitMs) == TRUE)
	{
		if (Response.m_nRequestID == nRequestID)
		{
			m_nResponseCmd = Response.m_nResponseCmd;

NHDEBUG(1, (_T(" CDiagCdu::CompleteAction() return : %d\n"), m_nResponseCmd));
			return m_nResponseCmd;
		}
	}

NHDEBUG(1, (_T(" CDiagCdu::CompleteAction() NOT WAIT : %d\n"), FALSE));
	return IO_PROCESSING;
}


BOOL CDiagCdu::Open(BOOL bWait, int *pRequestID)
{
	NHDEBUG(1, (_T(" \n") ));

	CNHAutoLock	nSyncObject(m_eMethodUse);
	
	// SAVE REQUEST COMMAND ID
	int nRequestID = GetRequestID();

	// INITIALIZE RESPONSE RESULT
	m_nResponseCmd = -1;

	/* INITIALIZE MEMBER VARIABLE CONCERNED THIS COMMAND, HERE */
	/////////////////////////////////////////////////////////////

	/* INPUT PARAMETER SETTING HERE */
	//////////////////////////////////

	// Request
	CNHCtrlCmdInfo	CmdInfo(nRequestID, (int)CDM_DEVICE_OPEN);
	m_RequestQueue.Enqueue(CmdInfo);

	/* SYNCHRONOUS CALL */
	if (bWait == TRUE)
	{
		BOOL bResult;

		/* WAIT I/O COMPLETED */
		bResult = CompleteAction(nRequestID/*, INFINITE */);
		
		if( bResult == CDU_SUCCESS )
		{
			/* OUTPUT PARAMETER SETTING HERE */
			;
		}

		return bResult;
	}

	/* ASYNCHRONOUS CALL */
	else if (pRequestID != NULL)
	{
		*pRequestID = nRequestID;
	}

	return TRUE;
}

BOOL CDiagCdu::Close(BOOL bWait, int *pRequestID)
{
	CNHAutoLock	nSyncObject(m_eMethodUse);
	
	// SAVE REQUEST COMMAND ID
	int nRequestID = GetRequestID();
	
	// INITIALIZE RESPONSE RESULT
	m_nResponseCmd = -1;

	/* INITIALIZE MEMBER VARIABLE CONCERNED THIS COMMAND, HERE */
	/////////////////////////////////////////////////////////////

	/* INPUT PARAMETER SETTING HERE */
	//////////////////////////////////

	// Store Request Command in the queue
	CNHCtrlCmdInfo	CmdInfo(nRequestID, (int)CDM_DEVICE_CLOSE);
	m_RequestQueue.Enqueue(CmdInfo);

	/* SYNCHRONOUS CALL */
	if (bWait == TRUE)
	{
		BOOL bResult;

		/* WAIT I/O COMPLETED */
		bResult = CompleteAction(nRequestID/*, INFINITE */);
		
		if( bResult == IO_COMPLETED)
		{
			/* OUTPUT PARAMETER SETTING HERE */
			;
		}

		return bResult;
	}
	/* ASYNCHRONOUS CALL */
	else if (pRequestID != NULL)
	{
		*pRequestID = nRequestID;
	}

	return TRUE;
}

BOOL CDiagCdu::Initialize(BOOL bWait, int *pRequestID)
{
	CNHAutoLock	nSyncObject(m_eMethodUse);
	
	// save Information
	int nRequestID = GetRequestID();
	// Clear Response Result
	m_nResponseCmd = -1;

	// Request
	CNHCtrlCmdInfo	CmdInfo(nRequestID, (int)CMD_INITIALIZE);
	m_RequestQueue.Enqueue(CmdInfo);

	/* SYNCHRONOUS CALL */
	if (bWait == TRUE)
	{
		BOOL bResult;

		/* WAIT I/O COMPLETED */
		bResult = CompleteAction(nRequestID/*, INFINITE */);
		
		if( bResult == IO_COMPLETED)
		{
			/* OUTPUT PARAMETER SETTING HERE */
			;
		}

		return bResult;
	}
	/* ASYNCHRONOUS CALL */
	else if (pRequestID != NULL)
	{
		*pRequestID = nRequestID;
	}

	return TRUE;
}

BOOL CDiagCdu::Test_Dispense(ULONG ulDispensedNotes[MAX_CST_NUMS], ULONG* ulTotalRejectedNote, ULONG ulRequestNotes[MAX_CST_NUMS], BOOL bWait, int *pRequestID)
{
NHDEBUG(1, (_T("CDiagCdu::Test_Dispense m_CSTNum [%d] \n"), m_CSTNum));
	DWORD	dwTemp = 0;
	CNHAutoLock	nSyncObject(m_eMethodUse);
	
	// SAVE REQUEST COMMAND ID
	int nRequestID = GetRequestID();

	// INITIALIZE RESPONSE RESULT
	m_nResponseCmd = -1;

	/* INITIALIZE MEMBER VARIABLE CONCERNED THIS COMMAND, HERE */
	memset(m_DispensNotes, 0x00, sizeof(m_DispensNotes));
	memset(m_RequestNotes, 0x00, sizeof(m_RequestNotes));
	m_TotalRejectedNote = 0;

	/* INPUT PARAMETER SETTING HERE */
//	if( ulRequestNotes == NULL )
//	{		
	if(m_CSTNum >= 1)
	{
		// [#RWC6-81] US William 2020.01.02 Multiple Test Notes
		m_RequestNotes[0] = ulRequestNotes[0];	// [#2357] NH KSK 2015.07.11
		SetRegDWORD(_T("DispenseNotesCst1"),_T("SOFTWARE\\ATM\\DevInfo\\NHVDM"),m_RequestNotes[0]);
	}
	else m_RequestNotes[0] = 0;

	if(m_CSTNum >= 2)
	{
		// [#RWC6-81] US William 2020.01.02 Multiple Test Notes
		m_RequestNotes[1] = ulRequestNotes[1];	// [#2357] NH KSK 2015.07.11
		SetRegDWORD(_T("DispenseNotesCst2"),_T("SOFTWARE\\ATM\\DevInfo\\NHVDM"),m_RequestNotes[1]);
	}
	else m_RequestNotes[1] = 0;

	if(m_CSTNum >= 3)
	{
		// [#RWC6-81] US William 2020.01.02 Multiple Test Notes
		m_RequestNotes[2] = ulRequestNotes[2];	// [#2357] NH KSK 2015.07.11
		SetRegDWORD(_T("DispenseNotesCst3"),_T("SOFTWARE\\ATM\\DevInfo\\NHVDM"),m_RequestNotes[2]);
	}
	else m_RequestNotes[2] = 0;

	if(m_CSTNum >= 4)
	{
		// [#RWC6-81] US William 2020.01.02 Multiple Test Notes
		m_RequestNotes[3] = ulRequestNotes[3];	// [#2357] NH KSK 2015.07.11
		SetRegDWORD(_T("DispenseNotesCst4"),_T("SOFTWARE\\ATM\\DevInfo\\NHVDM"),m_RequestNotes[3]);
	}
	else m_RequestNotes[3] = 0;

	// Request
	CNHCtrlCmdInfo	CmdInfo(nRequestID, (int)CMD_DISPENSE);
	m_RequestQueue.Enqueue(CmdInfo);

	/* SYNCHRONOUS CALL */
	if (bWait == TRUE)
	{
NHDEBUG(1, (_T("CDiagCdu::Test_Dispense : SYNCHRONOUS CALL\n")));
		BOOL bResult;

		/* WAIT I/O COMPLETED */
		bResult = CompleteAction(nRequestID/*, INFINITE */);
		
		if( bResult == IO_COMPLETED)
		{
			/* OUTPUT PARAMETER SETTING HERE */
			;
		}

		return bResult;
	}
	/* ASYNCHRONOUS CALL */
	else if (pRequestID != NULL)
	{
NHDEBUG(1, (_T("CDiagCdu::Test_Dispense : ASYNCHRONOUS CALL\n")));
		*pRequestID = nRequestID;
	}

	return TRUE;
}

BOOL CDiagCdu::GetSensorInfo(LPBYTE lpbSensorData, WORD* wSizeOfData, BOOL bWait, int *pRequestID)
{
	CNHAutoLock	nSyncObject(m_eMethodUse);
	
	/* SAVE REQUEST COMMAND ID */
	int nRequestID = GetRequestID();

	/* INITIALIZE RESPONSE RESULT */
	m_nResponseCmd = -1;

	/* INITIALIZE MEMBER VARIABLE CONCERNED THIS COMMAND, HERE */
	/////////////////////////////////////////////////////////////
	memset(m_Status, 0x00, sizeof(m_Status));
	m_wSizeOfData = 0;

	/* INPUT PARAMETER SETTING HERE */
	//////////////////////////////////


	// Request
	CNHCtrlCmdInfo	CmdInfo(nRequestID, (int)CMD_GET_SENSOR);
	m_RequestQueue.Enqueue(CmdInfo);

	/* SYNCHRONOUS CALL */
	if (bWait == TRUE)
	{
		BOOL bResult;

		/* WAIT I/O COMPLETED */
		bResult = CompleteAction(nRequestID/*, INFINITE */);
		
		if( bResult == CDU_SUCCESS )
		{
			/* OUTPUT PARAMETER SETTING HERE */
			memcpy(lpbSensorData, m_Status, m_wSizeOfData);
			*wSizeOfData = m_wSizeOfData;
		}

		return bResult;
	}
	/* ASYNCHRONOUS CALL */
	else if (pRequestID != NULL)
	{
		*pRequestID = nRequestID;
	}

	return TRUE;
}

BOOL CDiagCdu::GetVersion(USHORT *pusCstNum, ULONG ulCstIndex[MAX_CST_NUMS], TCHAR szCstCountry[4],  USHORT *pusCDUType, BOOL bWait, int *pRequestID)
{
	CNHAutoLock	nSyncObject(m_eMethodUse);
	
	/* SAVE REQUEST COMMAND ID */
	int nRequestID = GetRequestID();

	/* INITIALIZE RESPONSE RESULT */
	m_nResponseCmd = -1;

	/* INITIALIZE MEMBER VARIABLE CONCERNED THIS COMMAND, HERE */
	/////////////////////////////////////////////////////////////
	m_CSTNum		= 0;
	m_CSTType		= 0;
	memset(m_CSTIndex, 0x00, sizeof(m_CSTIndex));
	memset(m_CSTCountry, 0x00, sizeof(m_CSTCountry));

	/* INPUT PARAMETER SETTING HERE */
	//////////////////////////////////

	// Request
	CNHCtrlCmdInfo	CmdInfo(nRequestID, (int)CMD_GET_VERSION);
	m_RequestQueue.Enqueue(CmdInfo);

	/* SYNCHRONOUS CALL */
	if (bWait == TRUE)
	{
		BOOL bResult;

		/* WAIT I/O COMPLETED */
		bResult = CompleteAction(nRequestID/*, INFINITE */);
		
		if( bResult == CDU_SUCCESS)
		{
			/* OUTPUT PARAMETER SETTING HERE */
			*pusCstNum	= m_CSTNum;
			*pusCDUType	= m_CSTType;
			memcpy(ulCstIndex, m_CSTIndex, sizeof(m_CSTIndex));
			memcpy(szCstCountry, m_CSTCountry, sizeof(m_CSTCountry));

NHDEBUG(1, (_T("CDiagCdu::GetVersion() Success m_CSTNum[%d], m_CSTType 0x%x, m_CSTIndex[0] %d, m_CSTCountry %s\n"), m_CSTNum, m_CSTType, m_CSTIndex[0], m_CSTCountry));

			bResult = TRUE;
		}

		return bResult;
	}
	/* ASYNCHRONOUS CALL */
	else if (pRequestID != NULL)
	{
		*pRequestID = nRequestID;
	}

	return TRUE;
}

void CDiagCdu::GetCurrentCDUInform(USHORT *pusCstNum, ULONG ulCstIndex[MAX_CST_NUMS], TCHAR szCstCountry[4],  USHORT *pusCDUType)
{
	*pusCstNum	= m_CSTNum;
	*pusCDUType	= m_CSTType;
	memcpy(ulCstIndex, m_CSTIndex, sizeof(m_CSTIndex));
	memcpy(szCstCountry, m_CSTCountry, sizeof(m_CSTCountry));
}

CString CDiagCdu::GetCountryInfos(CString countryInfo, INT flag) 
{
	CString retVal;
	BOOL bCheckFind = FALSE;
	
	//값이 없을 경우는 넘어온 값을 그대로 출력하도록.
	//retVal.Format(_T("%s"),countryInfo);
	CString temp[3];
	int cnt = 0;
	
	for(int i=0; i<COUNTRY_INFO_SIZE; i++) {
		
		if(!COUNTRY_INFO[i][0].Compare(countryInfo) 
			|| !COUNTRY_INFO[i][1].Compare(countryInfo) 
			|| !COUNTRY_INFO[i][2].Compare(countryInfo)) 
		{
			
			retVal.Format(COUNTRY_INFO[i][flag]);
			bCheckFind = TRUE;
			break;
		}
		
		if(i == COUNTRY_INFO_SIZE-1 && bCheckFind == FALSE)
			retVal.Format(COUNTRY_INFO[COUNTRY_INFO_SIZE-1][flag]);
	}
	
//	return retVal.GetBuffer(retVal.GetLength());	AIREAT
	return retVal;
}

CString CDiagCdu::GetNextCountryInfos(CString countryInfo)
{
	BOOL bCheckFind = FALSE;
	int nCurrentIndex = 0;

	// Current Index Search
	for(int i=0; i<COUNTRY_INFO_SIZE; i++) {
		
		if(!COUNTRY_INFO[i][1].Compare(countryInfo))
		{
			nCurrentIndex = i;
			bCheckFind = TRUE;
			break;
		}
	}

	if (bCheckFind == FALSE)
		nCurrentIndex = 0;

	nCurrentIndex++;
	return COUNTRY_INFO[nCurrentIndex % COUNTRY_INFO_SIZE][1];
}

BOOL CDiagCdu::SetCDUSetupInform(USHORT usCstNum, ULONG ulCstIndex[MAX_CST_NUMS], TCHAR szCstCountry[4],  USHORT usCDUType)
{
	USHORT	tempCstNum;
	ULONG	tempCstIndex[MAX_CST_NUMS];
	TCHAR	tempCstCountry[4];
	USHORT	tempCDUType;

	CNHAutoLock	nSyncObject(m_eMethodUse);
	
	/* SAVE REQUEST COMMAND ID */
	int nRequestID = GetRequestID();
	
	/* INITIALIZE RESPONSE RESULT */
	m_nResponseCmd = -1;
	
	/* INITIALIZE MEMBER VARIABLE CONCERNED THIS COMMAND, HERE */
	/////////////////////////////////////////////////////////////
	m_CSTNum		= 0;
	m_CSTType		= 0;
	memset(m_CSTIndex, 0x00, sizeof(m_CSTIndex));
	memset(m_CSTCountry, 0x00, sizeof(m_CSTCountry));

	tempCstNum		= 0;
	tempCDUType		= 0;
	memset(tempCstIndex, 0x00, sizeof(tempCstIndex));
	memset(tempCstCountry, 0x00, sizeof(tempCstCountry));

	/* INPUT PARAMETER SETTING HERE */
	//////////////////////////////////
	m_CSTNum = usCstNum;
	m_CSTType = usCDUType;
	memcpy(m_CSTIndex, ulCstIndex, sizeof(m_CSTIndex));
	memcpy(m_CSTCountry, szCstCountry, sizeof(m_CSTCountry));

	tempCstNum = usCstNum;
	tempCDUType = usCDUType;
	memcpy(tempCstIndex, ulCstIndex, sizeof(m_CSTIndex));
	memcpy(tempCstCountry, szCstCountry, sizeof(m_CSTCountry));
	
	// Request
	CNHCtrlCmdInfo	CmdInfo(nRequestID, (int)CMD_CDUSETUP);
	m_RequestQueue.Enqueue(CmdInfo);
	
	/* SYNCHRONOUS CALL */
	BOOL bResult = FALSE;
	
	/* WAIT I/O COMPLETED */
	bResult = CompleteAction(nRequestID/*, INFINITE */);
	
NHDEBUG(1, (_T("CDiagCdu::CDU SETUP RESULT (%d)\n"), bResult));

	if( bResult == CDU_SUCCESS)
	{		
NHDEBUG(1, (_T("CDiagCdu::CDU SETUP SUCCESS\n")));
	}
NHDEBUG(1, (_T("CDiagCdu::CDU SETUP FAIL RESULT (%d)\n"), bResult));
		
	return bResult;
}

// [#RWC6-82] US Kook 2022.04.22 cdu ar auto dimming
BOOL CDiagCdu::GetAutoAdjustSensor(BOOL bWait, int *pRequestID)
{
	// CMD_GET_AUTOADJUSTSENSOR
	CNHAutoLock	nSyncObject(m_eMethodUse);

	/* SAVE REQUEST COMMAND ID */
	int nRequestID = GetRequestID();

	/* INITIALIZE RESPONSE RESULT */
	m_nResponseCmd = -1;

	/* INITIALIZE MEMBER VARIABLE CONCERNED THIS COMMAND, HERE */
	/////////////////////////////////////////////////////////////

	/* INPUT PARAMETER SETTING HERE */
	//////////////////////////////////

	// Request
	CNHCtrlCmdInfo	CmdInfo(nRequestID, (int)CMD_GET_AUTOADJUSTSENSOR);
	m_RequestQueue.Enqueue(CmdInfo);

	/* SYNCHRONOUS CALL */
	if (bWait == TRUE)
	{
		BOOL bResult;

		/* WAIT I/O COMPLETED */
		bResult = CompleteAction(nRequestID/*, INFINITE */);

//		if( bResult == IO_COMPLETED)
//		{
		/* OUTPUT PARAMETER SETTING HERE */

//		}

		return bResult;
	}
	/* ASYNCHRONOUS CALL */
	else if (pRequestID != NULL)
	{
		*pRequestID = nRequestID;
	}

	return TRUE;
}
// end of [#RWC6-82]

BOOL CDiagCdu::GetLastError(BYTE* bErrorCode, BOOL bWait, int *pRequestID)
{
	CNHAutoLock	nSyncObject(m_eMethodUse);
	
	/* SAVE REQUEST COMMAND ID */
	int nRequestID = GetRequestID();

	/* INITIALIZE RESPONSE RESULT */
	m_nResponseCmd = -1;

	/* INITIALIZE MEMBER VARIABLE CONCERNED THIS COMMAND, HERE */
	/////////////////////////////////////////////////////////////
	memset(m_ErrorCode, 0x00, sizeof(m_ErrorCode));

	/* INPUT PARAMETER SETTING HERE */
	//////////////////////////////////


	// Request
	CNHCtrlCmdInfo	CmdInfo(nRequestID, (int)CMD_GET_LAST_ERROR);
	m_RequestQueue.Enqueue(CmdInfo);

	/* SYNCHRONOUS CALL */
	if (bWait == TRUE)
	{
		BOOL bResult;

		/* WAIT I/O COMPLETED */
		bResult = CompleteAction(nRequestID/*, INFINITE */);
		
//		if( bResult == IO_COMPLETED)
//		{
			/* OUTPUT PARAMETER SETTING HERE */
			memcpy(bErrorCode, m_ErrorCode, sizeof(m_ErrorCode));
//		}

		return bResult;
	}
	/* ASYNCHRONOUS CALL */
	else if (pRequestID != NULL)
	{
		*pRequestID = nRequestID;
	}

	return TRUE;
}

BOOL CDiagCdu::IsDeviceError()
{
	if( m_bError == TRUE)
		return TRUE;
	else
		return FALSE;
}

void CDiagCdu::ClearDeviceError()
{
	m_bError = FALSE;
}

CString CDiagCdu::GetErrorString()
{
//NHDEBUG(1, (_T("CDiagCdu::GetErrorString()\n")));				

	CString szT;
	CNHAutoLock	nSyncObject(m_eMethodUse);
	
	/* SAVE REQUEST COMMAND ID */
	int nRequestID = GetRequestID();

	/* INITIALIZE RESPONSE RESULT */
	m_nResponseCmd = -1;

	/* INITIALIZE MEMBER VARIABLE CONCERNED THIS COMMAND, HERE */
	/////////////////////////////////////////////////////////////
	memset(m_ErrorCode, 0x00, sizeof(m_ErrorCode));
	m_strErroCode.Empty();

	/* INPUT PARAMETER SETTING HERE */
	//////////////////////////////////

	// Request
	CNHCtrlCmdInfo	CmdInfo(nRequestID, (int)CMD_GET_LAST_ERROR);
	m_RequestQueue.Enqueue(CmdInfo);

//	BOOL bResult;

	/* WAIT I/O COMPLETED */
	CompleteAction(nRequestID/*, INFINITE */);
		
	for(int i = 0; i < sizeof(m_ErrorCode); i++)
	{
		if(0 == m_ErrorCode[i])
			break;
		szT.Format(_T("%c"), m_ErrorCode[i]);
		m_strErroCode += szT;
	}

	return m_strErroCode;
}

unsigned CDiagCdu::ThreadHandlerProc()
{
	int			nReturn = FALSE;
	int			i;

	CNHCtrlCmdInfo	CmdInfo;
	m_bDoingThread = TRUE;

	CString szT = _T("");
	CString szErr = _T("");

	while(m_bDoingThread)
	{
		nReturn = FALSE;
		CmdInfo.m_nRequestCmd = (int)CMD_NONE;

		// Wait for Request Cmd
		if (m_RequestQueue.Dequeue(CmdInfo, 100) == TRUE)
		{
			switch ((int)CmdInfo.m_nRequestCmd)
			{
			case CDM_DEVICE_OPEN:
				/* OPEN SERIAL PORT */
				nReturn = m_CDU_FUNC.CDM_OpenDevice();
				NHDEBUG(1, (_T("CDiagCdu::ThreadHandlerProc : CDM_DEVICE_OPEN CALL nReturn %d\n"), nReturn));				
				
				/* SUCCESS TO OPEN */
				if(nReturn == TRUE)
				{
					m_bPortOpen = TRUE;
					m_bError	= FALSE;
					nReturn		= CDU_SUCCESS;

					if( (m_CDU_FUNC.CDM_GetVersion(&m_CSTNum, m_CSTIndex, m_CSTCountry, &m_CSTType) == CDU_SUCCESS) )
					{
						NHDEBUG(1, (_T("CDiagCdu::ThreadHandlerProc : CDM_GetVersion CALL m_CSTNum [%d] m_CSTIndex[0][%d] m_CSTCountry[%s] m_CSTType[0x%x]\n"), m_CSTNum, m_CSTIndex[0], m_CSTCountry, m_CSTType));				
						m_bCDUInfo = TRUE;

						// EP에서 0x00 , 0x01, 0x03, 0x0b일 경우에는 0x30, 0x31, 0x33, 0x3b값으로 올라옴
						// 기존 VDM과 POS BUG성을그대로 유지함
						if (m_CSTType == 0x30 || m_CSTType == 0x31 || m_CSTType == 0x32 || m_CSTType == 0x33 || 
							m_CSTType == 0x38 || m_CSTType == 0x39 || m_CSTType == 0x3a || m_CSTType == 0x3b) 			// [#2260] NH KSK 2014.03.13 Type 추가(0x02)에 따라 조건문 추가
							m_CSTType -= 0x30;
					}
				}
				else
				{
					m_bError	= TRUE;
					m_bPortOpen = FALSE;
					m_bCDUInfo	= FALSE;
					nReturn		= CDU_FAIL;
				}
				break;

			case CDM_DEVICE_CLOSE:
				nReturn = m_CDU_FUNC.CDM_CloseDevice();
				break;
			case CMD_INITIALIZE:
				nReturn = m_CDU_FUNC.CDM_Initialize();
				NHDEBUG(1, (_T("CDiagCdu::ThreadHandlerProc : CDM_Initialize CALL nReturn [%d]\n"), nReturn));
				if( nReturn == 0 )
				{
					m_bError	= FALSE;
					nReturn		= CDU_SUCCESS;
				}
				else
				{
					m_bError	= TRUE;
					nReturn		= CDU_FAIL;
				}

				break;
			case CMD_DISPENSE:
				nReturn	= m_CDU_FUNC.CDM_Dispense(m_RequestNotes, m_DispensNotes, &m_TotalRejectedNote);
				NHDEBUG(1, (_T("CDiagCdu::ThreadHandlerProc : CMD_DISPENSE CALL nReturn [%d] m_DispensNotes1 [%d] m_TotalRejectedNote[%d]\n"), nReturn, m_DispensNotes[0], m_TotalRejectedNote));

				if( nReturn == 0 )
				{
					m_bError	= FALSE;
					nReturn		= CDU_SUCCESS;

					// TEST DISPENSE가 성공할 경우 REJECT ANALYSIS를 UPDATE한다.
					for(int i=0; i<4; i++)
					{
						NHDEBUG(1, (_T("CDiagCdu::ThreadHandlerProc : CMD_DISPENSE CALL CST [%d] LastDispInfo.ulUsrCount[%d] LastDispInfo.ulRejCount[%d]\n"), i+1, MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_CST1_T_DISP_CNT + i), MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_CST1_T_REJ_CNT + i)));

						AddSumOfRejected(i+1, MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_CST1_T_REJ_CNT + i));
						AddSumOfTestRejected(i+1, MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_CST1_T_REJ_CNT + i));

						// [#2270] KSK 2014.06.11 Negative Count가 Enable인 경우 AP에서 관리하는 Remain Count를 Rejected Count만큼 감소시킨다.
						if (MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CDUM_NEGATIVE_COUNT_ENABLE) == ENABLE)
						{
							int nCurrentCount = MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST1_CURRENTCOUNT_AP + i);
							int nTempCount = 0;
							nTempCount = nCurrentCount - MemGetInt(_MEM_FLD_SP_CDM, _MEM_VAR_CDMSP_CST1_T_REJ_CNT + i);
							MemSetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_CST1_CURRENTCOUNT_AP + i, nTempCount);
						}
						// end of [#2270]
					}
				}
				else
				{
					m_bError	= TRUE;
					nReturn		= CDU_FAIL;
				}

				break;

			case CMD_CDUSETUP:
				nReturn	= m_CDU_FUNC.CDM_SetCdmInfo(m_CSTNum, m_CSTIndex, m_CSTCountry, m_CSTType);
NHDEBUG(1, (_T("CDiagCdu::ThreadHandlerProc : CMD_CDUSETUP CALL nReturn [%d] m_CSTNum [%d] m_CSTIndex[0][%d] m_CSTCountry[%s] m_CSTType[0x%x]\n"), nReturn, m_CSTNum, m_CSTIndex[0], m_CSTCountry, m_CSTType));

				if( nReturn == 0 )
				{
					m_bError	= FALSE;
					nReturn		= CDU_SUCCESS;
				}
				else
				{
					m_bError	= TRUE;
					nReturn		= CDU_FAIL;
				}
				break;

			case CMD_GET_VERSION:
				nReturn	= m_CDU_FUNC.CDM_GetVersion(&m_CSTNum, m_CSTIndex, m_CSTCountry, &m_CSTType);

				if( nReturn == 0 )
				{
					m_bError	= FALSE;
					nReturn		= CDU_SUCCESS;

					// 2009.09.30 KSK Bug Fix
					// EP에서 0x00 , 0x01, 0x03, 0x0b일 경우에는 0x30, 0x31, 0x33, 0x3b값으로 올라옴
					// 기존 VDM과 POS BUG성을그대로 유지함
//					if (m_CSTType == 0x30 || m_CSTType == 0x31 || m_CSTType == 0x33 || m_CSTType == 0x3b)
					if (m_CSTType == 0x30 || m_CSTType == 0x31 || m_CSTType == 0x32 || m_CSTType == 0x33 || 
						m_CSTType == 0x38 || m_CSTType == 0x39 || m_CSTType == 0x3a || m_CSTType == 0x3b) 			// [#2260] NH KSK 2014.03.13 Type 추가(0x02)에 따라 조건문 추가
						m_CSTType -= 0x30;
				}
				else
				{
					m_bError	= TRUE;
					nReturn		= CDU_FAIL;
				}

				break;

			case CMD_GET_SENSOR:
				nReturn	= m_CDU_FUNC.CDM_GetSensorInfo(m_Status, &m_wSizeOfData);

				if( nReturn == 0 )
				{
					m_bError	= FALSE;
					nReturn		= CDU_SUCCESS;
				}
				else
				{
					m_bError	= TRUE;
					nReturn		= CDU_FAIL;
				}

				break;

				// [#RWC6-82] US Kook 2022.04.22 cdu ar auto dimming
			case CMD_GET_AUTOADJUSTSENSOR:
				CONDIMMING_INFO		stSensorInfo;
				VALUE_READ			stDAValue;
				ADVALUE_READ		stADValue;

				// usSubMI : 0x10, Condimming Dark (Dark 자동조광)
				szT.Empty();
				nReturn = m_CDU_FUNC.CDM_GetAutoAdjustSensor(0x10, &stSensorInfo, &stDAValue, &stADValue);
				for (i=0; i<sizeof(stSensorInfo.SensorInfo) / sizeof(SENSOR_INFO_CHANNEL); i++)
					szT += Int2Asc(stSensorInfo.SensorInfo[i].AdjustResult);
				NHDEBUG(1, (_T("CDM_GetAutoAdjustSensor(usSubMI: 0x10): [%d], SensorInfo.AdjustResult: [%s]\n"), nReturn, szT));

				// usSubMI : 0x20, Condimming Light (Light 자동조광)
				szT.Empty();
				nReturn = m_CDU_FUNC.CDM_GetAutoAdjustSensor(0x20, &stSensorInfo, &stDAValue, &stADValue);
				for (i=0; i<sizeof(stSensorInfo.SensorInfo) / sizeof(SENSOR_INFO_CHANNEL); i++)
					szT += Int2Asc(stSensorInfo.SensorInfo[i].AdjustResult);
				NHDEBUG(1, (_T("CDM_GetAutoAdjustSensor(usSubMI: 0x20): [%d], SensorInfo.AdjustResult: [%s]\n"), nReturn, szT));

				// usSubMI : 0x30, Condimming Result Read (자동조광 결과 취득)
				szT.Empty();
				nReturn = m_CDU_FUNC.CDM_GetAutoAdjustSensor(0x30, &stSensorInfo, &stDAValue, &stADValue);
				for (i=0; i<sizeof(stSensorInfo.SensorInfo) / sizeof(SENSOR_INFO_CHANNEL); i++)
					szT += Int2Asc(stSensorInfo.SensorInfo[i].AdjustResult);
				NHDEBUG(1, (_T("CDM_GetAutoAdjustSensor(usSubMI: 0x30): [%d], SensorInfo.AdjustResult: [%s]\n"), nReturn, szT));

				szT.Format(L"GAAS:%s", szT);
				NVDump('O', 'E', "_", L"", szT);

				if (nReturn == 0)
				{
					m_bError	= FALSE;
					nReturn		= CDU_SUCCESS;
				}
				else
				{
					m_bError	= TRUE;
					nReturn		= CDU_FAIL;
				}
				break;
				// end of [#RWC6-82]

			case CMD_GET_LAST_ERROR:
				m_CDU_FUNC.CDM_GetErrorCode(m_ErrorCode);
				CString strDeviceError;
				CString szT;

				for(i = 0; i < sizeof(m_ErrorCode); i++)
				{
					if(0 == m_ErrorCode[i])
						break;
					szT.Format(_T("%c"), m_ErrorCode[i]);
					strDeviceError += szT;
				}
//NHDEBUG(1, (_T("CDiagCdu::ThreadHandlerProc() : Error [%s] \n"), strDeviceError));

				break;
			}

			// Response
			if (CmdInfo.m_nRequestCmd != CMD_NONE)
			{
				CmdInfo.m_nResponseCmd = (int)nReturn;
				m_ResponseQueue.Enqueue(CmdInfo);
			}
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////
/* Diagnostics Register Function [Get Register Value]		   */
/////////////////////////////////////////////////////////////////
BOOL GetRegDWORD(LPCTSTR szValueName, DWORD *dwValue, LPCTSTR lpszPath)
{
	HKEY		hKey = NULL;
	LONG		lRet = 0;					// [#2022] NH KSK 2011.02.22 Code Sonar 지적사항 대책
	DWORD		dwType = 0, dwSize = 0;		// [#2022] NH KSK 2011.02.22 Code Sonar 지적사항 대책
	BYTE		lpByte[1024] = { 0, };
	CString		szKeyPath=_T("SOFTWARE\\ATM\\DevInfo\\NHVDM");
	BOOL		bReturn = FALSE;

	for(int i=0; i<3; i++)
	{
		if(lpszPath == NULL)	
			lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKeyPath, 0, 0, &hKey);
		else
			lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpszPath, 0, 0, &hKey);
		
		if(lRet != ERROR_SUCCESS) {
			continue;
		}

		lRet = RegQueryValueEx(hKey, szValueName, NULL, &dwType, lpByte, &dwSize);

		if(lRet != ERROR_SUCCESS) 
		{
			lRet = RegQueryValueEx(hKey, szValueName, NULL, &dwType, lpByte, &dwSize);
			if(lRet != ERROR_SUCCESS)
			{
				RegCloseKey(hKey);
			}
		}
		else
		{
			RegCloseKey(hKey);
			bReturn = TRUE;
			break;
		}
	}
	
	if(bReturn)
		*dwValue = *(DWORD *)lpByte;
	return bReturn;
}

/////////////////////////////////////////////////////////////////
/* Diagnostics Register Function [Set Register Value]		   */
/////////////////////////////////////////////////////////////////

BOOL SetRegDWORD(LPCTSTR lpszKey, LPCTSTR lpszPath, DWORD dwValue)
{
	DWORD	dwDisp;
	HKEY	hKey, hOptKey = HKEY_LOCAL_MACHINE;
	LONG	lRet = -1;
	int		nRetry = 0;
	
	while (lRet!=ERROR_SUCCESS) {
		if (nRetry>=3)
			return FALSE;

		lRet = ::RegCreateKeyEx(hOptKey, lpszPath, 0, NULL, REG_OPTION_NON_VOLATILE, 0, NULL, &hKey, &dwDisp);
		nRetry++;
	}

	lRet = -1;
	nRetry = 0;
	while (lRet!=ERROR_SUCCESS) {
		if (nRetry>=3)
			return FALSE;

		lRet = ::RegSetValueEx(hKey, lpszKey, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));
		nRetry++;
	}

	RegCloseKey(hKey);

	return TRUE;
}
