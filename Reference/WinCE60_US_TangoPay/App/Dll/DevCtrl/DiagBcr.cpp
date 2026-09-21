#include "stdafx.h"
#include ".\Dev\DiagBcr.h"

//#define NH_DEBUG
#include ".\Common\CommDef.h"
#include ".\Common\CmnLib.h"
#include ".\Common\NHDbgApi.h"

// In Windows, there is no GetProcAddressW, so we have to force the string
// constants to multi-byte.
#ifdef UNDER_CE
#  define _LT(x) _T(x)
#else
#  define _LT(x) x
#endif

CDiagBcr::CDiagBcr()
{
	m_bInit     = FALSE;
	m_hDLL		= NULL;

	m_hDLL = ::LoadLibrary(VDM_BCR_FULL_PATH);
	if (m_hDLL == NULL)
	{
		NHDEBUG(1, (_T("Failed to load module [%s]!\n"), VDM_BCR_FULL_PATH));
		return;
	}

	m_pDllFunc.BCR_OpenConnection	= (BOOL_FUNCTION_VOID*)GetProcAddress(m_hDLL, _LT("BAR_OpenConnection"));
	m_pDllFunc.BCR_CloseConnection	= (VOID_FUNCTION_VOID*)GetProcAddress(m_hDLL, _LT("BAR_CloseConnection"));
	m_pDllFunc.BCR_Enable			= (BOOL_FUNCTION_VOID*)GetProcAddress(m_hDLL, _LT("BAR_Enable"));
	m_pDllFunc.BCR_Disable			= (BOOL_FUNCTION_VOID*)GetProcAddress(m_hDLL, _LT("BAR_Disable"));
	m_pDllFunc.BCR_GetEPVersion		= (BOOL_FUNCTION_PTCHAR_INT_PINT*)GetProcAddress(m_hDLL, _LT("BAR_GetEPVersion"));
	m_pDllFunc.BCR_GetData2			= (BOOL_FUNCTION_PTCHAR_INT_PINT*)GetProcAddress(m_hDLL, _LT("BAR_GetData2"));
	m_pDllFunc.BCR_ClearData		= (VOID_FUNCTION_VOID*)GetProcAddress(m_hDLL, _LT("BAR_ClearData"));
	m_pDllFunc.BCR_GetLastErrorCode	= (BOOL_FUNCTION_PTCHAR_PINT*)GetProcAddress(m_hDLL, _LT("BAR_GetLastErrorCode"));

	if (m_pDllFunc.BCR_OpenConnection == NULL)
		NHDEBUG(1, (_T("Failed to get address of [BCR_OpenConnection]!\n")));
	if (m_pDllFunc.BCR_CloseConnection == NULL)
		NHDEBUG(1, (_T("Failed to get address of [BCR_CloseConnection]!\n")));
	if (m_pDllFunc.BCR_Enable == NULL)
		NHDEBUG(1, (_T("Failed to get address of [BCR_Enable]!\n")));
	if (m_pDllFunc.BCR_Disable == NULL)
		NHDEBUG(1, (_T("Failed to get address of [BCR_Disable]!\n")));
	if (m_pDllFunc.BCR_GetEPVersion == NULL)
		NHDEBUG(1, (_T("Failed to get address of [BCR_GetEPVersion]!\n")));
	if (m_pDllFunc.BCR_GetData2 == NULL)
		NHDEBUG(1, (_T("Failed to get address of [BCR_GetData2]!\n")));
	if (m_pDllFunc.BCR_ClearData == NULL)
		NHDEBUG(1, (_T("Failed to get address of [BCR_ClearData]!\n")));
	if (m_pDllFunc.BCR_GetLastErrorCode == NULL)
		NHDEBUG(1, (_T("Failed to get address of [BCR_GetLastErrorCode]!\n")));

	if (m_pDllFunc.BCR_OpenConnection 
		&& m_pDllFunc.BCR_CloseConnection 
		&& m_pDllFunc.BCR_Enable 
		&& m_pDllFunc.BCR_Disable 
		&& m_pDllFunc.BCR_GetEPVersion 
		&& m_pDllFunc.BCR_GetData2 
		&& m_pDllFunc.BCR_ClearData 
		&& m_pDllFunc.BCR_GetLastErrorCode)
	{
		m_bInit = TRUE;
		NHDEBUG(1, (_T("DiagBcr Initialization Complete!\n")));
	}

	Resume();
}

CDiagBcr::~CDiagBcr()
{
	m_bDoingThread = FALSE;
	WaitForExitThread();

	// DLL Unload
	if (m_hDLL != NULL)
	{
		::FreeLibrary(m_hDLL);
		m_hDLL = NULL;

		memset(&m_pDllFunc, 0, sizeof(m_pDllFunc));
	}
}

BOOL CDiagBcr::ExecuteCommand(int pCmdNum, BOOL bWait, int *pRequestID)
{
	if (!m_bInit)
	{
		NHDEBUG(1, (_T("CDiagBcr is not initialized!\n")));
		return FALSE;
	}
	
	CNHAutoLock nSyncObject(m_eMethodUse);

	// SAVE REQUEST COMMAND ID
	int nRequestID = GetRequestID();

	// INITIALIZE RESPONSE RESULT
	m_nResponseCmd = -1;

	/* INITIALIZE MEMBER VARIABLE CONCERNED THIS COMMAND, HERE */
	/////////////////////////////////////////////////////////////

	/* INPUT PARAMETER SETTING HERE */
	//////////////////////////////////

	/* SET REQUEST COMMAND */
	NHDEBUG(1, (_T("Enqueuing Command #[%d]...\n"), pCmdNum));
	CNHCtrlCmdInfo  CmdInfo(nRequestID, pCmdNum);
	m_RequestQueue.Enqueue(CmdInfo);

	/* SYNCHRONOUS CALL */
	if (bWait == TRUE)
	{
		BOOL bResult;

		/* WAIT I/O COMPLETED */
		bResult = CompleteAction(nRequestID, (K_30_WAIT * 1000));

		if (bResult == ERROR_SUCCESS)
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

/**
* Inquiry result of the method which was called asynchronously.
* @param nRequestID an ID assigned to the method
* @param dwWaitMs timeout value
* @return Whether the method is completed or not.
*/
BOOL CDiagBcr::CompleteAction(int nRequestID, DWORD dwWaitMs)
{
	CNHCtrlCmdInfo	Response;

	if (nRequestID == -1)
		return FALSE;

	if (m_ResponseQueue.Dequeue(Response, dwWaitMs) == TRUE)
	{
		if (Response.m_nRequestID == nRequestID)
		{
			m_nResponseCmd = Response.m_nResponseCmd;
			NHDEBUG(1, (_T("CompleteAction() return : [%d]\n"), m_nResponseCmd));
			return m_nResponseCmd;
		}
	}

	NHDEBUG(1, (_T("CompleteAction() else : IO_PROCESSING [%d], [%d] \n"), m_nResponseCmd, IO_PROCESSING));
	return IO_PROCESSING;
}

BOOL CDiagBcr::ErrorExists()
{
	return m_bErrorExists;
}

CString CDiagBcr::GetErrorCode()
{
	if (ErrorExists())
		return CString(m_wchErrorCode);
	else
		return L"";
}

void CDiagBcr::ClearError()
{
	m_bErrorExists = FALSE;
	memset(m_wchErrorCode, 0, _countof(m_wchErrorCode));
}



CString CDiagBcr::GetCommandString(int pCmdNum)
{
	switch (pCmdNum)
	{
	case BCRCMD_OPEN: return "BCRCMD_OPEN";
	case BCRCMD_CLOSE: return "BCRCMD_CLOSE";
	case BCRCMD_ENABLE: return "BCRCMD_ENABLE";
	case BCRCMD_DISABLE: return "BCRCMD_DISABLE";
	case BCRCMD_GETEPVERSION: return "BCRCMD_GETEPVERSION";
	case BCRCMD_GETDATA2: return "BCRCMD_GETDATA2";
	case BCRCMD_CLEARDATA: return "BCRCMD_CLEARDATA";
	case BCRCMD_GETLASTERRORCODE: return "BCRCMD_GETLASTERRORCODE";
	default:
		break;
	}

	return "UNKNOWN";
}

unsigned CDiagBcr::ThreadHandlerProc()
{
	int nReturn = WFS_ERR_INTERNAL_ERROR;

	CNHCtrlCmdInfo	CmdInfo;
	m_bDoingThread = TRUE;

	NHDEBUG(1, (_T("CDiagBcr::ThreadHandlerProc() Command execute Process Start()\n")));
	NVDump('O', 'A', "11", L"", L"VDMTHREAD_BCR");	// adding NVDump() seems to be a workaround when ThreadHandlerProc() is not being called

	while(m_bDoingThread)
	{
		nReturn = WFS_ERR_INTERNAL_ERROR;
		CmdInfo.m_nRequestCmd = BCRCMD_NONE;

		// Wait for Request Cmd
		if (m_RequestQueue.Dequeue(CmdInfo, 100) == TRUE)
		{
			NHDEBUG(1, (_T("CDiagBcr::Dequeued CmdInfo: (%d: %s)\n"), CmdInfo.m_nRequestCmd, GetCommandString(CmdInfo.m_nRequestCmd)));
			switch (CmdInfo.m_nRequestCmd)
			{
			case BCRCMD_OPEN:	// return WFS_SUCCESS(0) when OpenConnection() returns TRUE(1)
				{
					ClearError();
					nReturn = m_pDllFunc.BCR_OpenConnection();
					if (m_pDllFunc.BCR_OpenConnection() == TRUE)
					{
						nReturn = WFS_SUCCESS;
					}
					else
					{
						int nErrorCodeLength = 0;
						m_pDllFunc.BCR_GetLastErrorCode(m_wchErrorCode, &nErrorCodeLength);
						m_bErrorExists = TRUE;
					}
				}
				break;

			case BCRCMD_CLOSE:
				{
					m_pDllFunc.BCR_CloseConnection();
					nReturn = WFS_SUCCESS;
				}
				break;

			case BCRCMD_ENABLE:
				{
					if (m_pDllFunc.BCR_Enable() == TRUE)
						nReturn = WFS_SUCCESS;
				}
				break;

			case BCRCMD_DISABLE:
				{
					if (m_pDllFunc.BCR_Disable() == TRUE)
						nReturn = WFS_SUCCESS;
				}
				break;

			case BCRCMD_GETEPVERSION:
				{
					ClearError();
					int versionLength = 0;
					memset(m_wchEPVersion, 0, _countof(m_wchEPVersion));

					nReturn = m_pDllFunc.BCR_GetEPVersion(m_wchEPVersion, _countof(m_wchEPVersion), &versionLength);
					NHDEBUG(1, (_T("m_chEPVersion: [%S] LEN[%d] \n"), m_wchEPVersion, versionLength));
				}

				break;

			case BCRCMD_GETDATA2:
				{
					ClearError();
					int barcodeLength = 0;
					memset(m_wchBarcodeData, 0, _countof(m_wchBarcodeData));

					nReturn = m_pDllFunc.BCR_GetData2(m_wchBarcodeData, _countof(m_wchBarcodeData), &barcodeLength);
					NHDEBUG(1, (_T("m_chBarcodeData: [%S] LEN[%d] \n"), m_wchBarcodeData, barcodeLength));
				}
				break;

			case BCRCMD_CLEARDATA:
				{
					m_pDllFunc.BCR_ClearData();
					nReturn = WFS_SUCCESS;
				}
				break;

			case BCRCMD_GETLASTERRORCODE:
				{
					ClearError();
					int errorCodeLength = 0;
					nReturn = m_pDllFunc.BCR_GetLastErrorCode(m_wchErrorCode, &errorCodeLength);
				}
				break;

			default:
				break;
			}

			NHDEBUG(1, (_T("Result: [%d][%s] \n"), nReturn, GetErrorCode()));

			// Response
			if (CmdInfo.m_nRequestCmd != BCRCMD_NONE)
			{
				CmdInfo.m_nResponseCmd = nReturn;
				m_ResponseQueue.Enqueue(CmdInfo);
			}
		}
	}

	NHDEBUG(1, (_T("ThreadHandlerProc() Command execute Process End()\n")));

	return 0;
}

// methods
BOOL CDiagBcr::Open(BOOL bWait, int *pRequestID)				{ return ExecuteCommand(BCRCMD_OPEN, bWait, pRequestID); }
BOOL CDiagBcr::Close(BOOL bWait, int *pRequestID)				{ return ExecuteCommand(BCRCMD_CLOSE, bWait, pRequestID); }
BOOL CDiagBcr::Enable(BOOL bWait, int *pRequestID)				{ return ExecuteCommand(BCRCMD_ENABLE, bWait, pRequestID); }
BOOL CDiagBcr::Disable(BOOL bWait, int *pRequestID)				{ return ExecuteCommand(BCRCMD_DISABLE, bWait, pRequestID); }
BOOL CDiagBcr::GetData2(BOOL bWait, int *pRequestID)			{ return ExecuteCommand(BCRCMD_GETDATA2, bWait, pRequestID); }
BOOL CDiagBcr::GetLastErrorCode(BOOL bWait, int *pRequestID)	{ return ExecuteCommand(BCRCMD_GETLASTERRORCODE, bWait, pRequestID); }

// properties
CString CDiagBcr::GetBarcodeData()			{ return CString(m_wchBarcodeData); }
CString CDiagBcr::GetEPVersion()			{ return CString(m_wchEPVersion); }
