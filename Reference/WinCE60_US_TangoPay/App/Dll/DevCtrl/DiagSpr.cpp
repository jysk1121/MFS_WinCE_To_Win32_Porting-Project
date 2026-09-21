#include "stdafx.h"
#include ".\Dev\DiagSpr.h"

//#define NH_DEBUG
#include ".\Common\CmnLib.h"
#include ".\Common\NHDbgApi.h"

// 내부 Thread에게 동작을 명령하는 CMD

enum SPR_CMD
{
	CMD_NONE = 0,
	CDM_DEVICE_OPEN,
	CDM_DEVICE_CLOSE,
	CMD_INITIALIZE,
	CMD_GET_VERSION,
	CMD_GET_SENSOR,
	CMD_AUTOLOAD,
	CMD_TEST_PRINT,
	CMD_PRINT,
	CMD_CUT,
	CMD_EJECT,
	CMD_RETRACT,
	CMD_GET_LAST_ERROR,
};

CDiagSpr::CDiagSpr()
{
	m_bInit = FALSE;
	m_hSPR_DLL = NULL;
	m_bPortOpen = FALSE;
	m_bCDUInfo	= FALSE;
	m_bError	= FALSE;

	m_pSPRData	= NULL;

	// Load Dll
	m_hSPR_DLL = ::LoadLibrary(VDM_SPR_FULL_PATH);
	if (m_hSPR_DLL != NULL)
	{
		NHDEBUG(1, (_T(" CDiagSpr::CDiagSpr() LoadLibrary SUCCESS : 0x%x\n"), m_hSPR_DLL));
		/* SPR DLL Function Map */
		m_SPR_FUNC.SPR_OpenConnection				= (BOOL_FUNCTION_VOID*)GetProcAddress(m_hSPR_DLL, _LT("SPR_OpenConnection"));
		m_SPR_FUNC.SPR_GetVersion					= (VOID_FUNCTION_LPSTR*)GetProcAddress(m_hSPR_DLL, _LT("SPR_GetVersion"));
		m_SPR_FUNC.SPR_Initialize					= (INT_FUNCTION_VOID*)GetProcAddress(m_hSPR_DLL, _LT("SPR_Initialize"));
		m_SPR_FUNC.SPR_AutoLoad						= (INT_FUNCTION_VOID*)GetProcAddress(m_hSPR_DLL, _LT("SPR_AutoLoad"));
		m_SPR_FUNC.SPR_Print						= (INT_FUNCTION_VOID*)GetProcAddress(m_hSPR_DLL, _LT("SPR_Print"));
		m_SPR_FUNC.SPR_Print2						= (INT_FUNCTION_PCHAR_INT*)GetProcAddress(m_hSPR_DLL, _LT("SPR_Print2"));
		m_SPR_FUNC.SPR_Cut							= (INT_FUNCTION_VOID*)GetProcAddress(m_hSPR_DLL, _LT("SPR_Cut"));
		m_SPR_FUNC.SPR_Eject						= (INT_FUNCTION_VOID*)GetProcAddress(m_hSPR_DLL, _LT("SPR_Eject"));
		m_SPR_FUNC.SPR_Retract						= (INT_FUNCTION_VOID*)GetProcAddress(m_hSPR_DLL, _LT("SPR_Retract"));
		m_SPR_FUNC.SPR_GetSensor					= (INT_FUNCTION_LPSTR*)GetProcAddress(m_hSPR_DLL, _LT("SPR_GetSensor"));
		m_SPR_FUNC.SPR_GetLastError					= (VOID_FUNCTION_LPSTR*)GetProcAddress(m_hSPR_DLL, _LT("SPR_GetLastError"));
		m_SPR_FUNC.SPR_CloseConnection				= (VOID_FUNCTION_VOID*)GetProcAddress(m_hSPR_DLL, _LT("SPR_CloseConnection"));

		m_bInit = TRUE;
	}
	else
	{
		NHDEBUG(1, (_T(" CDiagSpr::CDiagSpr() LoadLibrary FAILE : 0x%x\n"), m_hSPR_DLL));
	}

	Resume();
}

CDiagSpr::~CDiagSpr()
{
	m_bDoingThread = FALSE;
	WaitForExitThread();

	// DLL Unload
	if (m_hSPR_DLL != NULL)
	{
		::FreeLibrary(m_hSPR_DLL);
		m_hSPR_DLL = NULL;
		
		memset(&m_SPR_FUNC, 0, sizeof(m_SPR_FUNC));
	}


}

/*-------------------------------------------------------------------
 CLASS    NAME: CDiagSpr
 FUNCTION NAME: CompleteAction()
 RETURN TYPE  : TRUE : 호출한 Method의 동작이 완료 되었다.
				FALSE : 호출한 Method가 동작 중이다.
 PARAMETER    : 
 DESCRIPTION  : ASync로 호출한 Method의 결과를 조회한다.
-------------------------------------------------------------------*/
BOOL CDiagSpr::CompleteAction(int nRequestID, DWORD dwWaitMs)
{
	CNHCtrlCmdInfo	Response;
	
	if (nRequestID == -1)
		return FALSE;
	
	if (m_ResponseQueue.Dequeue(Response, dwWaitMs) == TRUE)
	{
		if (Response.m_nRequestID == nRequestID)
		{
			m_nResponseCmd = Response.m_nResponseCmd;

			/* COMPLETE ASYNCHRONOUS CALL */
			if( dwWaitMs != INFINITE)
			{
				switch( nRequestID )
				{
				case CDM_DEVICE_OPEN:
					break;
				case CDM_DEVICE_CLOSE:
					break;
				case CMD_INITIALIZE:
					break;
				case CMD_GET_LAST_ERROR:
					break;
				}
			}
NHDEBUG(1, (_T(" CDiagSpr::CompleteAction() return : %d\n"), m_nResponseCmd));
			return m_nResponseCmd;
		}
	}

	return IO_PROCESSING;
}

BOOL CDiagSpr::Open(BOOL bWait, int *pRequestID)
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
		
		if( bResult == SPR_SUCCESS)
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

BOOL CDiagSpr::Close(BOOL bWait, int *pRequestID)
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
		
		if( bResult == SPR_SUCCESS)
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

BOOL CDiagSpr::Initialize(BOOL bWait, int *pRequestID)
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
		
		if( bResult == SPR_SUCCESS)
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

BOOL CDiagSpr::Test_Print(BOOL bWait, int *pRequestID)
{
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
	CNHCtrlCmdInfo	CmdInfo(nRequestID, (int)CMD_TEST_PRINT);
	m_RequestQueue.Enqueue(CmdInfo);

	/* SYNCHRONOUS CALL */
	if (bWait == TRUE)
	{
		BOOL bResult;

		/* WAIT I/O COMPLETED */
		bResult = CompleteAction(nRequestID/*, INFINITE */);
		
		if( bResult == SPR_SUCCESS)
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

BOOL CDiagSpr::Print(BYTE* pPrintData, BOOL bWait, int *pRequestID)
{
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
	CNHCtrlCmdInfo	CmdInfo(nRequestID, (int)CMD_PRINT);
	m_RequestQueue.Enqueue(CmdInfo);

	/* SYNCHRONOUS CALL */
	if (bWait == TRUE)
	{
		BOOL bResult;

		/* WAIT I/O COMPLETED */
		bResult = CompleteAction(nRequestID/*, INFINITE */);
		
		if( bResult == SPR_SUCCESS)
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

BOOL CDiagSpr::Cutting(BOOL bWait, int *pRequestID)
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
	CNHCtrlCmdInfo	CmdInfo(nRequestID, (int)CMD_CUT);
	m_RequestQueue.Enqueue(CmdInfo);

	/* SYNCHRONOUS CALL */
	if (bWait == TRUE)
	{
		BOOL bResult;

		/* WAIT I/O COMPLETED */
		bResult = CompleteAction(nRequestID/*, INFINITE */);
		
		if( bResult == SPR_SUCCESS)
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

BOOL CDiagSpr::GetSensorInfo(LPBYTE lpbSensorData, DWORD dwSize, BOOL bWait, int *pRequestID)
{
	CNHAutoLock	nSyncObject(m_eMethodUse);
	
	/* SAVE REQUEST COMMAND ID */
	int nRequestID = GetRequestID();

	/* INITIALIZE RESPONSE RESULT */
	m_nResponseCmd = -1;

	/* INITIALIZE MEMBER VARIABLE CONCERNED THIS COMMAND, HERE */
	/////////////////////////////////////////////////////////////
	memset(m_Status, 0x00, sizeof(m_Status));

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
		
		if( bResult == SPR_SUCCESS)
		{
			/* OUTPUT PARAMETER SETTING HERE */
			memcpy(lpbSensorData, m_Status, dwSize);
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

BOOL CDiagSpr::GetLastError(BYTE* bErrorCode, BOOL bWait, int *pRequestID)
{
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

	/* SYNCHRONOUS CALL */
	if (bWait == TRUE)
	{
		BOOL bResult;

		/* WAIT I/O COMPLETED */
		bResult = CompleteAction(nRequestID/*, INFINITE */);
		
		if( bResult == IO_COMPLETED)
		{
			/* OUTPUT PARAMETER SETTING HERE */
			memcpy(bErrorCode, m_ErrorCode, sizeof(m_ErrorCode));
			m_strErroCode.Format(_T("%S"), m_ErrorCode);
NHDEBUG(1, (_T(" CDiagSpr::GetLastError() return : m_strErroCode [%s]\n"), m_strErroCode));

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

BOOL CDiagSpr::IsDeviceError()
{
	if( m_bError == TRUE)
		return TRUE;
	else
		return FALSE;
}

void CDiagSpr::ClearDeviceError()
{
	m_bError = FALSE;
}

CString CDiagSpr::GetErrorString()
{
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
	m_strErroCode.Format(_T("%S"), m_ErrorCode);
NHDEBUG(1, (_T(" CDiagSpr::GetErrorString() return : m_strErroCode [%s]\n"), m_strErroCode));

	return m_strErroCode;
}

unsigned CDiagSpr::ThreadHandlerProc()
{
	int			nReturn = FALSE;

	CNHCtrlCmdInfo	CmdInfo;
	m_bDoingThread = TRUE;

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
				nReturn = m_SPR_FUNC.SPR_OpenConnection();
				if( nReturn )
				{
					nReturn = SPR_SUCCESS;
					m_bError	= FALSE;
				}
				else
				{
					nReturn = SPR_FAIL;
					m_bError	= TRUE;
				}
				break;
			case CDM_DEVICE_CLOSE:
				m_SPR_FUNC.SPR_CloseConnection();
				nReturn = TRUE;
				break;
			case CMD_INITIALIZE:
				nReturn = m_SPR_FUNC.SPR_Initialize();
NHDEBUG(1, (_T("CDiagSpr::ThreadHandlerProc() : SPR_Initialize nReturn [%d] \n"), nReturn));
				if( nReturn == 0 )
				{
					nReturn		= SPR_SUCCESS;
					m_bError	= FALSE;
				}
				else
				{
					nReturn		= SPR_FAIL;
					m_bError	= TRUE;
				}
				break;

			case CMD_TEST_PRINT:
				nReturn = m_SPR_FUNC.SPR_Print();
NHDEBUG(1, (_T("CDiagSpr::ThreadHandlerProc() : SPR_Print nReturn [%d] \n"), nReturn));
				if( nReturn == 0 )
				{
					nReturn		= SPR_SUCCESS;
					m_bError	= FALSE;
				}
				else
				{
					nReturn		= SPR_FAIL;
					m_bError	= TRUE;
				}
				break;
			case CMD_CUT:
				nReturn = m_SPR_FUNC.SPR_Cut();
NHDEBUG(1, (_T("CDiagSpr::ThreadHandlerProc() : SPR_Cut nReturn [%d] \n"), nReturn));				
				if( nReturn == 0 )
				{
					nReturn		= SPR_SUCCESS;
					m_bError	= FALSE;
				}
				else
				{
					nReturn		= SPR_FAIL;
					m_bError	= TRUE;
				}
				break;

			case CMD_GET_VERSION:
//				nReturn	= m_SPR_FUNC.SPR_GetVersion(&m_CSTNum, m_CSTIndex, m_CSTCountry, &m_CSTType);
				break;
			case CMD_GET_SENSOR:
				nReturn	= m_SPR_FUNC.SPR_GetSensor((char*)m_Status);

				if( nReturn == 0 )
				{
					nReturn		= SPR_SUCCESS;
					m_bError	= FALSE;
				}
				else
				{
					nReturn		= SPR_FAIL;
					m_bError	= TRUE;
				}
				
				break;
			case CMD_GET_LAST_ERROR:
				CString Tmp;
				m_SPR_FUNC.SPR_GetLastError((char*)m_ErrorCode);

				Tmp.Format(_T("%S"), m_ErrorCode);
NHDEBUG(1, (_T("CDiagSpr::ThreadHandlerProc() : CMD_GET_LAST_ERROR Error [%s] \n"), Tmp));
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
