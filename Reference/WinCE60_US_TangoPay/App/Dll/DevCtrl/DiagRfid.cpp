#include "stdafx.h"
#include ".\Dev\DiagRfid.h"

//#define NH_DEBUG
#include ".\Common\CmnLib.h"
#include ".\Common\NHDbgApi.h"

// 내부 Thread에게 동작을 명령하는 CMD
enum CARD_CMD
{
	CMD_NONE = 0,
	CDM_VIVOPAYKII_DEVICE_OPEN,
	CDM_VIVOPAYKII_DEVICE_CLOSE,
	CMD_VIVOPAYKII_ENABLE,
	CMD_VIVOPAYKII_DISABLE,
	CMD_VIVOPAYKII_READDATA,
	CMD_VIVOPAYKII_READDATA2,
	CMD_VIVOPAYKII_READRAWDATA,
	CMD_VIVOPAYKII_GETERRORCODE,
	CMD_VIVOPAYKII_CANCEL
};

// [#2325] NH KSK 2015.01.25
CDiagRfid::CDiagRfid(int nRFIDType)
{
	m_bInit     = FALSE;
	m_hRFID_DLL = NULL;
	m_RFIDType  = nRFIDType;
	m_bPortOpen = FALSE;

	m_bError    = FALSE;    // [#2325] NH KSK 2015.01.25 Default 누락에 따른 초기화값 설정 (Open하지 않을 경우 NORMAL로 처리)
	nRecvSize   = 0;

	memset(m_RFIDTrackData, 0, sizeof(m_RFIDTrackData));

	// Load Dll
	if ( m_RFIDType == 1 )
		m_hRFID_DLL = ::LoadLibrary(VDM_RFID_VIVOPAYKII_FULL_PATH);

	if (m_hRFID_DLL != NULL)
	{
		if ( m_RFIDType == 1 )
		{
			m_RFID_FUNC.RFID_READER_OpenSessionSync     = (HRESULT_FUNCTION_VOID*)GetProcAddress(m_hRFID_DLL, _LT("OpenSessionSync"));
			m_RFID_FUNC.RFID_READER_CloseSessionSync    = (VOID_FUNCTION_VOID*)GetProcAddress(m_hRFID_DLL, _LT("CloseSessionSync"));
			m_RFID_FUNC.RFID_READER_Enable              = (HRESULT_FUNCTION_VOID*)GetProcAddress(m_hRFID_DLL, _LT("Enable"));
			m_RFID_FUNC.RFID_READER_Disable             = (HRESULT_FUNCTION_VOID*)GetProcAddress(m_hRFID_DLL, _LT("Disable"));
			m_RFID_FUNC.RFID_READER_ReadData            = (HRESULT_FUNCITON_LPBYTE_PINT*)GetProcAddress(m_hRFID_DLL, _LT("ReadData"));
			m_RFID_FUNC.RFID_READER_ReadData2           = (HRESULT_FUNCITON_LPBYTE_PINT_DWORD*)GetProcAddress(m_hRFID_DLL, _LT("ReadData2"));
			m_RFID_FUNC.RFID_READER_ReadRawData         = (HRESULT_FUNCTION_WORD_PRFVDMCARDDATA_DWORD*)GetProcAddress(m_hRFID_DLL, _LT("ReadRawData"));
			m_RFID_FUNC.RFID_READER_GetErrorCode        = (VOID_FUNCTION_PCHAR*)GetProcAddress(m_hRFID_DLL, _LT("GetErrorCode"));
			m_RFID_FUNC.RFID_READER_GetDeviceType       = (INT_FUNCTION_VOID*)GetProcAddress(m_hRFID_DLL, _LT("GetDeviceType"));
			m_RFID_FUNC.RFID_READER_CancelCommand       = (VOID_FUNCTION_VOID*)GetProcAddress(m_hRFID_DLL, _LT("CancelCommand"));
		}
		m_bInit = TRUE;
	}

	Resume();
}

// [#2325] NH KSK 2015.01.25
CDiagRfid::~CDiagRfid()
{
	m_bDoingThread = FALSE;
	WaitForExitThread();

	// DLL Unload
	if (m_hRFID_DLL != NULL)
	{
		::FreeLibrary(m_hRFID_DLL);
		m_hRFID_DLL = NULL;

		memset(&m_RFID_FUNC, 0, sizeof(m_RFID_FUNC));
	}
}
// end of [#2325]

// [#2325] NH KSK 2015.01.25
/*-------------------------------------------------------------------
CLASS    NAME: CDiagRfid
FUNCTION NAME: CompleteAction()
RETURN TYPE  : TRUE : 호출한 Method의 동작이 완료 되었다.
FALSE : 호출한 Method가 동작 중이다.
PARAMETER    : 
DESCRIPTION  : ASync로 호출한 Method의 결과를 조회한다.
-------------------------------------------------------------------*/
BOOL CDiagRfid::CompleteAction(int nRequestID, DWORD dwWaitMs)
{
	CNHCtrlCmdInfo  Response;

	if (nRequestID == -1)
		return FALSE;

	if (m_ResponseQueue.Dequeue(Response, dwWaitMs) == TRUE)
	{
		if (Response.m_nRequestID == nRequestID)
		{
			m_nResponseCmd = Response.m_nResponseCmd;
			NHDEBUG(1, (_T(" !!!!!! CDiagRfid::CompleteAction() return : [%d]\n"), m_nResponseCmd));
			return m_nResponseCmd;
		}
		NHDEBUG(1, (_T(" !!!!!! CDiagRfid::CompleteAction() else : [%d]\n"), m_nResponseCmd));
	}

	//  NHDEBUG(1, (_T(" !!!!!! CDiagRfid::CompleteAction() else : IO_PROCESSING [%d], [%d] \n"), m_nResponseCmd, IO_PROCESSING));
	return IO_PROCESSING;
}
// end of [#2325]

// [#2325] NH KSK 2015.01.25
BOOL CDiagRfid::Open(BOOL bWait, int *pRequestID)
{
	NHDEBUG(1, (_T("CDiagRfid::Open Start \n")));
	CNHAutoLock nSyncObject(m_eMethodUse);
	int nDeviceCommand;

	// SAVE REQUEST COMMAND ID
	int nRequestID = GetRequestID();

	// INITIALIZE RESPONSE RESULT
	m_nResponseCmd = -1;

	/* INITIALIZE MEMBER VARIABLE CONCERNED THIS COMMAND, HERE */
	/////////////////////////////////////////////////////////////


	/* INPUT PARAMETER SETTING HERE */
	//////////////////////////////////


	/* SET REQUEST COMMAND */
	if( m_RFIDType == 1 )
		nDeviceCommand = (int)CDM_VIVOPAYKII_DEVICE_OPEN;

	NHDEBUG(1, (_T("CDiagRfid::Open nDeviceCommand[%d] \n"), nDeviceCommand));

	CNHCtrlCmdInfo  CmdInfo(nRequestID, nDeviceCommand);
	m_RequestQueue.Enqueue(CmdInfo);

	/* SYNCHRONOUS CALL */
	if (bWait == TRUE)
	{
		BOOL bResult;

		/* WAIT I/O COMPLETED */
		bResult = CompleteAction(nRequestID/*, INFINITE */);

		if( bResult == RFID_SUCCESS)
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
// end of [#2325]

// [#2325] NH KSK 2015.01.25
BOOL CDiagRfid::Close(BOOL bWait, int *pRequestID)
{
	CNHAutoLock nSyncObject(m_eMethodUse);
	int nDeviceCommand;

	// SAVE REQUEST COMMAND ID
	int nRequestID = GetRequestID();

	// INITIALIZE RESPONSE RESULT
	m_nResponseCmd = -1;

	/* INITIALIZE MEMBER VARIABLE CONCERNED THIS COMMAND, HERE */
	/////////////////////////////////////////////////////////////


	/* INPUT PARAMETER SETTING HERE */
	//////////////////////////////////


	/* SET REQUEST COMMAND */
	if( m_RFIDType == 1 )
		nDeviceCommand = (int)CDM_VIVOPAYKII_DEVICE_CLOSE;

	CNHCtrlCmdInfo  CmdInfo(nRequestID, nDeviceCommand);
	m_RequestQueue.Enqueue(CmdInfo);

	/* SYNCHRONOUS CALL */
	if (bWait == TRUE)
	{
		BOOL bResult;

		/* WAIT I/O COMPLETED */
		bResult = CompleteAction(nRequestID/*, INFINITE */);

		if( bResult == RFID_IO_COMPLETED)
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
// [#2325] NH KSK 2015.01.25

// [#2325] NH KSK 2015.01.25
BOOL CDiagRfid::Enable(BOOL bWait, int *pRequestID)
{
	CNHAutoLock nSyncObject(m_eMethodUse);

	// SAVE REQUEST COMMAND ID
	int nRequestID = GetRequestID();

	// INITIALIZE RESPONSE RESULT
	m_nResponseCmd = -1;

	/* INITIALIZE MEMBER VARIABLE CONCERNED THIS COMMAND, HERE */
	/////////////////////////////////////////////////////////////

	/* INPUT PARAMETER SETTING HERE */
	//////////////////////////////////

	// Store Request Command in the queue
	CNHCtrlCmdInfo  CmdInfo(nRequestID, (int)CMD_VIVOPAYKII_ENABLE);
	m_RequestQueue.Enqueue(CmdInfo);

	/* SYNCHRONOUS CALL */
	if (bWait == TRUE)
	{
		BOOL bResult;

		/* WAIT I/O COMPLETED */
		bResult = CompleteAction(nRequestID/*, INFINITE */);

		if( bResult == RFID_IO_COMPLETED)
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
// end of [#2325]

// [#2325] NH KSK 2015.01.25
BOOL CDiagRfid::Disable(BOOL bWait, int *pRequestID)
{
	CNHAutoLock nSyncObject(m_eMethodUse);

	// SAVE REQUEST COMMAND ID
	int nRequestID = GetRequestID();

	// INITIALIZE RESPONSE RESULT
	m_nResponseCmd = -1;

	/* INITIALIZE MEMBER VARIABLE CONCERNED THIS COMMAND, HERE */
	/////////////////////////////////////////////////////////////

	/* INPUT PARAMETER SETTING HERE */
	//////////////////////////////////

	// Store Request Command in the queue
	CNHCtrlCmdInfo  CmdInfo(nRequestID, (int)CMD_VIVOPAYKII_DISABLE);
	m_RequestQueue.Enqueue(CmdInfo);

	/* SYNCHRONOUS CALL */
	if (bWait == TRUE)
	{
		BOOL bResult;

		/* WAIT I/O COMPLETED */
		bResult = CompleteAction(nRequestID/*, INFINITE */);

		if( bResult == RFID_IO_COMPLETED)
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
// end of [#2325]

// [#2325] NH KSK 2015.01.25
BOOL CDiagRfid::ReadRawData(DWORD dwTimeout, BOOL bWait, int *pRequestID)
{
	CNHAutoLock nSyncObject(m_eMethodUse);

	// SAVE REQUEST COMMAND ID
	int nRequestID = GetRequestID();

	// INITIALIZE RESPONSE RESULT
	m_nResponseCmd = -1;

	/* INITIALIZE MEMBER VARIABLE CONCERNED THIS COMMAND, HERE */
	/////////////////////////////////////////////////////////////
	m_dwTimeOut = dwTimeout;
	memset(m_RFIDTrackData, 0, sizeof(m_RFIDTrackData));

	/* INPUT PARAMETER SETTING HERE */
	//////////////////////////////////

	// Store Request Command in the queue
	CNHCtrlCmdInfo  CmdInfo(nRequestID, (int)CMD_VIVOPAYKII_READRAWDATA);
	m_RequestQueue.Enqueue(CmdInfo);

	/* SYNCHRONOUS CALL */
	if (bWait == TRUE)
	{
		BOOL bResult;

		/* WAIT I/O COMPLETED */
		bResult = CompleteAction(nRequestID/*, INFINITE */);

		if( bResult == RFID_IO_COMPLETED)
		{
			/* OUTPUT PARAMETER SETTING HERE */
			// Sync 미사용함
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
// end of [#2325]

// [#2325] NH KSK 2015.01.25
BOOL CDiagRfid::CancelCommand(BOOL bWait, int *pRequestID)
{
	CNHAutoLock nSyncObject(m_eMethodUse);

	// SAVE REQUEST COMMAND ID
	int nRequestID = GetRequestID();

	// INITIALIZE RESPONSE RESULT
	m_nResponseCmd = -1;

	// Store Request Command in the queue
	CNHCtrlCmdInfo  CmdInfo(nRequestID, (int)CMD_VIVOPAYKII_CANCEL);
	m_RequestQueue.Enqueue(CmdInfo);

	/* SYNCHRONOUS CALL */
	if (bWait == TRUE)
	{
		BOOL bResult;

		/* WAIT I/O COMPLETED */
		bResult = CompleteAction(nRequestID/*, INFINITE */);

		if( bResult == RFID_SUCCESS)
		{
			/* OUTPUT PARAMETER SETTING HERE */
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

// [#2325] NH KSK 2015.01.25
BOOL CDiagRfid::GetErrorCode(BYTE bErrorCode[16], BOOL bWait, int *pRequestID)
{
	CNHAutoLock nSyncObject(m_eMethodUse);

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
	CNHCtrlCmdInfo  CmdInfo(nRequestID, (int)CMD_VIVOPAYKII_GETERRORCODE);
	m_RequestQueue.Enqueue(CmdInfo);

	/* SYNCHRONOUS CALL */
	if (bWait == TRUE)
	{
		BOOL bResult;

		/* WAIT I/O COMPLETED */
		bResult = CompleteAction(nRequestID/*, INFINITE */);

		if( bResult == RFID_IO_COMPLETED    )
		{
			/* OUTPUT PARAMETER SETTING HERE */
			memcpy(bErrorCode, m_ErrorCode, sizeof(m_ErrorCode));
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
// end of [#2325]

// [#2325] NH KSK 2015.01.25
BOOL CDiagRfid::IsDeviceError()
{
	if( m_bError == TRUE)
		return TRUE;
	else
		return FALSE;
}
// end of [#2325]

// [#2325] NH KSK 2015.01.25
void CDiagRfid::ClearDeviceError()
{
	m_bError = FALSE;
	m_strErroCode.Empty();
}
// end of [#2325]

// [#2325] NH KSK 2015.01.25
CString CDiagRfid::GetErrorString()
{
	NHDEBUG(1, (_T("CDiagRfid::GetErrorString()\n")));

	return m_strErroCode;
}
// end of [#2325]

// [#2325] NH KSK 2015.01.28
void CDiagRfid::RFID_GetTrackData(RFVDMCARDDATA sRFVDMCardData[5])
{
	memcpy(sRFVDMCardData, m_RFIDTrackData, sizeof(m_RFIDTrackData));
}
// end of [#2325]

// [#2325] NH KSK 2015.01.25
unsigned CDiagRfid::ThreadHandlerProc()
{
	int         nReturn = FALSE;
	char        chErrorCode[16] = { 0, };

	CNHCtrlCmdInfo  CmdInfo;
	m_bDoingThread = TRUE;

	NHDEBUG(1, (_T("CDiagRfid::ThreadHandlerProc() Command execute Process Start()\n")));

	while(m_bDoingThread)
	{
		nReturn = FALSE;
		CmdInfo.m_nRequestCmd = (int)CMD_NONE;

		// Wait for Request Cmd
		if (m_RequestQueue.Dequeue(CmdInfo, 100) == TRUE)
		{
			NHDEBUG(1, (_T("CDiagRfid::ThreadHandlerProc() Dequeue(CmdInfo.RequestID(%d)\n"), CmdInfo.m_nRequestCmd));
			switch ((int)CmdInfo.m_nRequestCmd)
			{
			case CDM_VIVOPAYKII_DEVICE_OPEN:
				/* OPEN SERIAL PORT */
				NHDEBUG(1, (_T("CDiagRfid::ThreadHandlerProc() RFID_READER_OpenSessionSync\n")));
				nReturn = m_RFID_FUNC.RFID_READER_OpenSessionSync();
				if(nReturn == RFID_SUCCESS)
				{
					m_bPortOpen = TRUE;
					m_bError = FALSE;
					nReturn = RFID_SUCCESS;
					m_strErroCode.Empty();
					NHDEBUG(1, (_T("CDiagRfid::ThreadHandlerProc() RFID_READER_OpenSessionSync SUCCESS nReturn[%d] \n"), nReturn));
				}
				else
				{
					m_bPortOpen = FALSE;
					m_bError = TRUE;
					nReturn = RFID_FAIL;
					memset(chErrorCode, 0, sizeof(chErrorCode));
					m_RFID_FUNC.RFID_READER_GetErrorCode(chErrorCode);
					m_strErroCode.Format(L"%S", chErrorCode);
					NHDEBUG(1, (_T("CDiagRfid::ThreadHandlerProc() RFID_READER_OpenSessionSync FAIL nReturn[%d][%s] \n"), nReturn, m_strErroCode));
				}
				break;

			case CDM_VIVOPAYKII_DEVICE_CLOSE:
				NHDEBUG(1, (_T("CDiagRfid::ThreadHandlerProc() RFID_READER_CloseSessionSync\n")));
				m_RFID_FUNC.RFID_READER_CloseSessionSync();
				nReturn = RFID_SUCCESS;
				break;

			case CMD_VIVOPAYKII_ENABLE:
				NHDEBUG(1, (_T("CDiagRfid::ThreadHandlerProc() RFID_READER_Enable\n")));
				nReturn = m_RFID_FUNC.RFID_READER_Enable();
				NHDEBUG(1, (_T("CDiagRfid::ThreadHandlerProc() RFID_READER_Enable (%d)\n"), nReturn));
				break;

			case CMD_VIVOPAYKII_DISABLE:
				NHDEBUG(1, (_T("CDiagRfid::ThreadHandlerProc() RFID_READER_Disable\n")));
				nReturn = m_RFID_FUNC.RFID_READER_Disable();
				NHDEBUG(1, (_T("CDiagRfid::ThreadHandlerProc() RFID_READER_Disable (%d)\n"), nReturn));
				break;

			case CMD_VIVOPAYKII_READRAWDATA:
				NHDEBUG(1, (_T("CDiagRfid::ThreadHandlerProc() RFID_READER_ReadRawData\n")));
				// to allow test normal IC cards, change DataSource from 0x03 (WFS_IDC_TRACK1 & WFS_IDC_TRACK2) to 0x08 (WFS_IDC_CHIP).
// 				nReturn = m_RFID_FUNC.RFID_READER_ReadRawData(0x03, &m_RFIDTrackData[0], m_dwTimeOut);
				nReturn = m_RFID_FUNC.RFID_READER_ReadRawData(0x08, &m_RFIDTrackData[0], m_dwTimeOut);
				NHDEBUG(1, (_T("CDiagRfid::ThreadHandlerProc() RFID_READER_ReadData2 (Track[%d][%S][Len:%d])\n"), m_RFIDTrackData[0].wDataSource, m_RFIDTrackData[0].byData, m_RFIDTrackData[0].ulDataLength));
				NHDEBUG(1, (_T("CDiagRfid::ThreadHandlerProc() RFID_READER_ReadData2 Track[%d][%S][Len:%d]\n"), m_RFIDTrackData[1].wDataSource, m_RFIDTrackData[1].byData, m_RFIDTrackData[1].ulDataLength));
				NHDEBUG(1, (_T("CDiagRfid::ThreadHandlerProc() RFID_READER_ReadData2 Return[%d]\n"), nReturn));

				if( nReturn == 0 )
					nReturn = RFID_SUCCESS;
				else if (nReturn == -13 || nReturn == -54)  // 단선인 경우 -13 or -54 return함
					nReturn = RFID_ERROR;
				else if (nReturn == -48)
					nReturn = RFID_IO_TIME_OUT;
				else
					nReturn = RFID_FAIL;
				break;

			case CMD_VIVOPAYKII_GETERRORCODE:
				break;

			case CMD_VIVOPAYKII_CANCEL:
				NHDEBUG(1, (_T("CDiagRfid::ThreadHandlerProc() RFID_READER_CancelCommand\n")));
				m_RFID_FUNC.RFID_READER_CancelCommand();
				nReturn = RFID_SUCCESS;
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

	NHDEBUG(1, (_T("CDiagRfid::ThreadHandlerProc() Command execute Process End()\n")));

	return 0;
}
// end of [#2325]
