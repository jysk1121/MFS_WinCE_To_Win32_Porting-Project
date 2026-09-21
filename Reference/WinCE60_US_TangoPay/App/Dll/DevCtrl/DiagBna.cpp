#include "stdafx.h"
#include ".\Dev\DiagBna.h"

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

CDiagBna::CDiagBna(int nBnaType)
{
	NHDEBUG(1, (_T("given BNA type: [%d]\n"), nBnaType));

	m_bInit     = FALSE;
	m_hDLL		= NULL;

	switch (nBnaType)
	{
	case BNATYPE_JCM:
		m_hDLL = ::LoadLibrary(VDM_BNA_JCM_FULL_PATH);
		break;

	case BNATYPE_MEI:
	default:
		m_hDLL = ::LoadLibrary(VDM_BNA_MEI_FULL_PATH);
		break;
	}

	if (m_hDLL == NULL)
	{
		NHDEBUG(1, (_T("Failed to load module!\n")));
		return;
	}

	m_pDllFunc.BNA_OpenConnection		= (LONG_FUNCTION_LONG*)GetProcAddress(m_hDLL, _LT("MEI_OpenConnection"));
	m_pDllFunc.BNA_CloseConnection		= (VOID_FUNCTION_VOID*)GetProcAddress(m_hDLL, _LT("MEI_CloseConnection"));
	m_pDllFunc.BNA_Initialize			= (LONG_FUNCTION_VOID*)GetProcAddress(m_hDLL, _LT("MEI_Initialize"));
	m_pDllFunc.BNA_Accept				= (LONG_FUNCTION_LONG*)GetProcAddress(m_hDLL, _LT("MEI_Accept"));
	m_pDllFunc.BNA_CancelAccept			= (LONG_FUNCTION_VOID*)GetProcAddress(m_hDLL, _LT("MEI_CancelAccept"));
	m_pDllFunc.BNA_Stack				= (LONG_FUNCTION_VOID*)GetProcAddress(m_hDLL, _LT("MEI_Stack"));
	m_pDllFunc.BNA_Eject				= (INT_FUNCTION_VOID*)GetProcAddress(m_hDLL, _LT("MEI_Eject"));
	m_pDllFunc.BNA_Enable				= (LONG_FUNCTION_LONG_BOOL*)GetProcAddress(m_hDLL, _LT("MEI_Enable"));
	m_pDllFunc.BNA_GetCurrency			= (INT_FUNCTION_VOID*)GetProcAddress(m_hDLL, _LT("MEI_GetCurrency"));
	m_pDllFunc.BNA_GetEnabledDenom		= (INT_FUNCTION_LPTSTR_PBYTE*)GetProcAddress(m_hDLL, _LT("MEI_GetEnabledDenom"));
	m_pDllFunc.BNA_GetLastError			= (VOID_FUNCTION_LPTSTR*)GetProcAddress(m_hDLL, _LT("MEI_GetLastError"));
	m_pDllFunc.BNA_SetHandle			= (VOID_FUNCTION_HWND*)GetProcAddress(m_hDLL, _LT("MEI_SetHandle"));
	m_pDllFunc.BNA_Accept2				= (LONG_FUNCTION_PULONG_PINT_PINT_ULONG*)GetProcAddress(m_hDLL, _LT("MEI_Accept2"));
	m_pDllFunc.BNA_CancelAccept2		= (LONG_FUNCTION_VOID*)GetProcAddress(m_hDLL, _LT("MEI_CancelAccept2"));
	m_pDllFunc.BNA_Accept3				= (LONG_FUNCTION_PULONG_PINT_PINT_ULONG*)GetProcAddress(m_hDLL, _LT("MEI_Accept3"));
	m_pDllFunc.BNA_CancelAccept3		= (LONG_FUNCTION_VOID*)GetProcAddress(m_hDLL, _LT("MEI_CancelAccept3"));
	m_pDllFunc.BNA_QueryBarcodeData		= (BOOL_FUNCTION_PTCHAR*)GetProcAddress(m_hDLL, _LT("MEI_QueryBarcodeData"));
	m_pDllFunc.BNA_GetEPVersion_Bill	= (LONG_FUNCTION_PTCHAR*)GetProcAddress(m_hDLL, _LT("MEI_GetEPVersion_Bill"));
	m_pDllFunc.BNA_GetEPVersion_Variant	= (LONG_FUNCTION_PTCHAR*)GetProcAddress(m_hDLL, _LT("MEI_GetEPVersion_Variant"));

	if (m_pDllFunc.BNA_OpenConnection == NULL)
		NHDEBUG(1, (_T("Failed to get address of [BNA_OpenConnection]!\n")));
	if (m_pDllFunc.BNA_CloseConnection == NULL)
		NHDEBUG(1, (_T("Failed to get address of [BNA_CloseConnection]!\n")));
	if (m_pDllFunc.BNA_Initialize == NULL)
		NHDEBUG(1, (_T("Failed to get address of [BNA_Initialize]!\n")));
	if (m_pDllFunc.BNA_Accept == NULL)
		NHDEBUG(1, (_T("Failed to get address of [BNA_Accept]!\n")));
	if (m_pDllFunc.BNA_CancelAccept == NULL)
		NHDEBUG(1, (_T("Failed to get address of [BNA_CancelAccept]!\n")));
	if (m_pDllFunc.BNA_Stack == NULL)
		NHDEBUG(1, (_T("Failed to get address of [BNA_Stack]!\n")));
	if (m_pDllFunc.BNA_Eject == NULL)
		NHDEBUG(1, (_T("Failed to get address of [BNA_Eject]!\n")));
	if (m_pDllFunc.BNA_Enable == NULL)
		NHDEBUG(1, (_T("Failed to get address of [BNA_Enable]!\n")));
	if (m_pDllFunc.BNA_GetCurrency == NULL)
		NHDEBUG(1, (_T("Failed to get address of [BNA_GetCurrenty]!\n")));
	if (m_pDllFunc.BNA_GetEnabledDenom == NULL)
		NHDEBUG(1, (_T("Failed to get address of [BNA_GetEnabledDenom]!\n")));
	if (m_pDllFunc.BNA_GetLastError == NULL)
		NHDEBUG(1, (_T("Failed to get address of [BNA_GetLastError]!\n")));
	if (m_pDllFunc.BNA_SetHandle == NULL)
		NHDEBUG(1, (_T("Failed to get address of [BNA_SetHandle]!\n")));
	if (m_pDllFunc.BNA_Accept2 == NULL)
		NHDEBUG(1, (_T("Failed to get address of [BNA_Accept2]!\n")));
	if (m_pDllFunc.BNA_CancelAccept2 == NULL)
		NHDEBUG(1, (_T("Failed to get address of [BNA_CancelAccept2]!\n")));
	if (m_pDllFunc.BNA_Accept3 == NULL)
		NHDEBUG(1, (_T("Failed to get address of [BNA_Accept3]!\n")));
	if (m_pDllFunc.BNA_CancelAccept3 == NULL)
		NHDEBUG(1, (_T("Failed to get address of [BNA_CancelAccept3]!\n")));
	if (m_pDllFunc.BNA_QueryBarcodeData == NULL)
		NHDEBUG(1, (_T("Failed to get address of [BNA_QueryBarcodeData]!\n")));
	if (m_pDllFunc.BNA_GetEPVersion_Bill == NULL)
		NHDEBUG(1, (_T("Failed to get address of [BNA_GetEPVersion_Bill]!\n")));
	if (m_pDllFunc.BNA_GetEPVersion_Variant == NULL)
		NHDEBUG(1, (_T("Failed to get address of [BNA_GetEPVersion_Variant]!\n")));


	if (m_pDllFunc.BNA_OpenConnection 
		&& m_pDllFunc.BNA_CloseConnection 
		&& m_pDllFunc.BNA_Initialize 
		&& m_pDllFunc.BNA_Accept 
		&& m_pDllFunc.BNA_CancelAccept 
		&& m_pDllFunc.BNA_Stack 
		&& m_pDllFunc.BNA_Eject 
		&& m_pDllFunc.BNA_Enable
		&& m_pDllFunc.BNA_GetCurrency
		//&& m_pDllFunc.BNA_GetEnabledDenom
		&& m_pDllFunc.BNA_GetLastError
		&& m_pDllFunc.BNA_SetHandle
		&& m_pDllFunc.BNA_Accept2
		&& m_pDllFunc.BNA_CancelAccept2
		&& m_pDllFunc.BNA_Accept3
		&& m_pDllFunc.BNA_CancelAccept3
		&& m_pDllFunc.BNA_QueryBarcodeData
		&& m_pDllFunc.BNA_GetEPVersion_Bill
		&& m_pDllFunc.BNA_GetEPVersion_Variant)
	{
		m_bInit = TRUE;
		NHDEBUG(1, (_T("DiagBna Initialization Complete!\n")));
	}

	Resume();
}

CDiagBna::~CDiagBna()
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

/**
 * Execute given device command.
 * @param pCmdNum command number
 * @param bWait a flag to wait the completion
 * @return The result for synchronous call, TRUE for asynchronous call
*/
BOOL CDiagBna::ExecuteCommand(int pCmdNum, BOOL bWait, int *pRequestID)
{
	if (!m_bInit)
	{
		NHDEBUG(1, (_T("CDiagBna is not initialized!\n")));
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
		bResult = CompleteAction(nRequestID/*, INFINITE */);

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
BOOL CDiagBna::CompleteAction(int nRequestID, DWORD dwWaitMs)
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

BOOL CDiagBna::ErrorExists()
{
	return m_bErrorExists;
}

CString CDiagBna::GetErrorCode()
{
	if (ErrorExists())
		return CString(m_wchErrorCode);
	else
		return L"";
}

void CDiagBna::ClearError()
{
	m_bErrorExists = FALSE;
	memset(m_wchErrorCode, 0, _countof(m_wchErrorCode));
}

CString CDiagBna::GetCommandString(int pCmdNum)
{
	switch (pCmdNum)
	{
	case BNACMD_OPEN: return "BNACMD_OPEN";
	case BNACMD_CLOSE: return "BNACMD_CLOSE";
	case BNACMD_INITIALIZE: return "BNACMD_INITIALIZE";
	case BNACMD_ACCEPT: return "BNACMD_ACCEPT";
	case BNACMD_CANCELACCEPT: return "BNACMD_CANCELACCEPT";
	case BNACMD_STACK: return "BNACMD_STACK";
	case BNACMD_EJECT: return "BNACMD_EJECT";
	case BNACMD_ENABLE: return "BNACMD_ENABLE";
	case BNACMD_GETCURRENCY: return "BNACMD_GETCURRENCY";
	case BNACMD_GETENABLEDDENOM: return "BNACMD_GETENABLEDDENOM";
	case BNACMD_GETLASTERROR: return "BNACMD_GETLASTERROR";
	case BNACMD_SETHANDLE: return "BNACMD_SETHANDLE";
	case BNACMD_ACCEPT2: return "BNACMD_ACCEPT2";
	case BNACMD_CANCELACCEPT2: return "BNACMD_CANCELACCEPT2";
	case BNACMD_ACCEPT3: return "BNACMD_ACCEPT3";
	case BNACMD_CANCELACCEPT3: return "BNACMD_CANCELACCEPT3";
	case BNACMD_QUERYBARCODEDATA: return "BNACMD_QUERYBARCODEDATA";
	case BNACMD_GETEPVERSION_BILL: return "BNACMD_GETEPVERSION_BILL";
	case BNACMD_GETEPVERSION_VARIANT: return "BNACMD_GETEPVERSION_VARIANT";
	default:
		break;
	}

	return "UNKNOWN";
}

unsigned CDiagBna::ThreadHandlerProc()
{
	int nReturn = FALSE;

	CNHCtrlCmdInfo	CmdInfo;
	m_bDoingThread = TRUE;

	NHDEBUG(1, (_T("CDiagBna::ThreadHandlerProc() Command execute Process Start()\n")));
	NVDump('O', 'A', "11", L"", L"VDMTHREAD_BNA");	// adding NVDump() seems to be a workaround when ThreadHandlerProc() is not being called

	while(m_bDoingThread)
	{
		nReturn = FALSE;
		CmdInfo.m_nRequestCmd = BNACMD_NONE;

		// Wait for Request Cmd
		if (m_RequestQueue.Dequeue(CmdInfo, 100) == TRUE)
		{
			NHDEBUG(1, (_T("CDiagBna::Dequeued CmdInfo: (%d: %s)\n"), CmdInfo.m_nRequestCmd, GetCommandString(CmdInfo.m_nRequestCmd)));
			switch (CmdInfo.m_nRequestCmd)
			{
			case BNACMD_OPEN:
				{
					ClearError();
					nReturn = m_pDllFunc.BNA_OpenConnection(K_30_WAIT * 1000);
					if (nReturn != WFS_SUCCESS)
					{
						m_pDllFunc.BNA_GetLastError(m_wchErrorCode);
						m_bErrorExists = TRUE;
					}
				}
				break;

			case BNACMD_CLOSE:
				{
					m_pDllFunc.BNA_CloseConnection();
					nReturn = WFS_SUCCESS;
				}
				break;

			case BNACMD_INITIALIZE:
				{
					ClearError();
					nReturn = m_pDllFunc.BNA_Initialize();
					if (nReturn != WFS_SUCCESS)
					{
						m_pDllFunc.BNA_GetLastError(m_wchErrorCode);
						m_bErrorExists = TRUE;
					}
				}
				break;

			case BNACMD_ACCEPT:
				{
					ClearError();
					nReturn = m_pDllFunc.BNA_Accept(K_30_WAIT * 1000);
					if (nReturn != WFS_SUCCESS)
					{
						m_pDllFunc.BNA_GetLastError(m_wchErrorCode);
						m_bErrorExists = TRUE;
					}
				}
				break;

			case BNACMD_CANCELACCEPT:
				{
					nReturn = m_pDllFunc.BNA_CancelAccept();
				}
				break;

			case BNACMD_STACK:
				{
					ClearError();
					nReturn = m_pDllFunc.BNA_Stack();
					if (nReturn != WFS_SUCCESS)
					{
						m_pDllFunc.BNA_GetLastError(m_wchErrorCode);
						m_bErrorExists = TRUE;
					}
				}
				break;

			case BNACMD_EJECT:
				{
					ClearError();
					nReturn = m_pDllFunc.BNA_Eject();
					if (nReturn != WFS_SUCCESS)
					{
						m_pDllFunc.BNA_GetLastError(m_wchErrorCode);
						m_bErrorExists = TRUE;
					}
				}
				break;

			case BNACMD_ENABLE:
				{
					// improper command for VDM
					NHDEBUG(1, (_T("Method not supported!\n")));
				}
				break;

			case BNACMD_GETCURRENCY:
				{
					nReturn = m_pDllFunc.BNA_GetCurrency();
				}
				break;

			case BNACMD_GETENABLEDDENOM:
				{
					// improper command for VDM
					NHDEBUG(1, (_T("Method not supported!\n")));
				}
				break;

			case BNACMD_GETLASTERROR:
				{
					ClearError();
					m_pDllFunc.BNA_GetLastError(m_wchErrorCode);
					nReturn = WFS_SUCCESS;
				}
				break;

			case BNACMD_SETHANDLE:
				{
					// improper command for VDM
					NHDEBUG(1, (_T("Method not supported!\n")));
				}
				break;

			case BNACMD_ACCEPT2:
				{
					ClearError();
					nReturn = m_pDllFunc.BNA_Accept2(&m_ulAcceptValue, &m_nAcceptCount, &m_nRejectCount, K_30_WAIT * 1000);
					if (nReturn != WFS_SUCCESS)
					{
						m_pDllFunc.BNA_GetLastError(m_wchErrorCode);
						m_bErrorExists = TRUE;
					}
				}
				break;

			case BNACMD_CANCELACCEPT2:
				{
					nReturn = m_pDllFunc.BNA_CancelAccept2();
				}
				break;

			case BNACMD_ACCEPT3:
				{
					ClearError();
					nReturn = m_pDllFunc.BNA_Accept3(&m_ulAcceptValue, &m_nAcceptCount, &m_nRejectCount, K_30_WAIT * 1000);
					if (nReturn != WFS_SUCCESS)
					{
						m_pDllFunc.BNA_GetLastError(m_wchErrorCode);
						m_bErrorExists = TRUE;
					}
				}
				break;

			case BNACMD_CANCELACCEPT3:
				{
					nReturn = m_pDllFunc.BNA_CancelAccept3();
				}
				break;

			case BNACMD_QUERYBARCODEDATA:
				{
					memset(m_wchBarcodeData, 0, _countof(m_wchBarcodeData));
					nReturn = m_pDllFunc.BNA_QueryBarcodeData(m_wchBarcodeData);
					NHDEBUG(1, (_T("BNA_QueryBarcodeData: (%s)\n"), m_wchBarcodeData));
				}
				break;

			case BNACMD_GETEPVERSION_BILL:
				{
					memset(m_wchEPVersionBill, 0, _countof(m_wchEPVersionBill));
					m_pDllFunc.BNA_GetEPVersion_Bill(m_wchEPVersionBill);
					NHDEBUG(1, (_T("BNA_GetEPVersion_Bill: (%s)\n"), m_wchEPVersionBill));
				}
				break;

			case BNACMD_GETEPVERSION_VARIANT:
				{
					memset(m_wchEPVersionVariant, 0, _countof(m_wchEPVersionVariant));
					m_pDllFunc.BNA_GetEPVersion_Variant(m_wchEPVersionVariant);
					NHDEBUG(1, (_T("BNA_GetEPVersion_Variant: (%s)\n"), m_wchEPVersionVariant));
				}
				break;

			default:
				break;
			}

			NHDEBUG(1, (_T("Result: [%d][%s] \n"), nReturn, GetErrorCode()));

			// Response
			if (CmdInfo.m_nRequestCmd != BNACMD_NONE)
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
BOOL CDiagBna::Open(BOOL bWait, int *pRequestID)				{ return ExecuteCommand(BNACMD_OPEN, bWait, pRequestID); }
BOOL CDiagBna::Close(BOOL bWait, int *pRequestID)				{ return ExecuteCommand(BNACMD_CLOSE, bWait, pRequestID);}
BOOL CDiagBna::Initialize(BOOL bWait, int *pRequestID)			{ return ExecuteCommand(BNACMD_INITIALIZE, bWait, pRequestID);}
BOOL CDiagBna::Accept(BOOL bWait, int *pRequestID)				{ return ExecuteCommand(BNACMD_ACCEPT, bWait, pRequestID);}
BOOL CDiagBna::CancelAccept(BOOL bWait, int *pRequestID)		{ return ExecuteCommand(BNACMD_CANCELACCEPT, bWait, pRequestID);}
BOOL CDiagBna::Stack(BOOL bWait, int *pRequestID)				{ return ExecuteCommand(BNACMD_STACK, bWait, pRequestID);}
BOOL CDiagBna::Eject(BOOL bWait, int *pRequestID)				{ return ExecuteCommand(BNACMD_EJECT, bWait, pRequestID);}
BOOL CDiagBna::Enable(BOOL bWait, int *pRequestID)				{ return ExecuteCommand(BNACMD_ENABLE, bWait, pRequestID);}
BOOL CDiagBna::GetCurrency(BOOL bWait, int *pRequestID)			{ return ExecuteCommand(BNACMD_GETCURRENCY, bWait, pRequestID);}
BOOL CDiagBna::GetEnabledDenom(BOOL bWait, int *pRequestID)		{ return ExecuteCommand(BNACMD_GETENABLEDDENOM, bWait, pRequestID);}
BOOL CDiagBna::GetLastError(BOOL bWait, int *pRequestID)		{ return ExecuteCommand(BNACMD_GETLASTERROR, bWait, pRequestID);}
BOOL CDiagBna::SetHandle(BOOL bWait, int *pRequestID)			{ return ExecuteCommand(BNACMD_SETHANDLE, bWait, pRequestID);}
BOOL CDiagBna::Accept2(BOOL bWait, int *pRequestID)				{ return ExecuteCommand(BNACMD_ACCEPT2, bWait, pRequestID);}
BOOL CDiagBna::CancelAccept2(BOOL bWait, int *pRequestID)		{ return ExecuteCommand(BNACMD_CANCELACCEPT2, bWait, pRequestID);}
BOOL CDiagBna::Accept3(BOOL bWait, int *pRequestID)				{ return ExecuteCommand(BNACMD_ACCEPT3, bWait, pRequestID);}
BOOL CDiagBna::CancelAccept3(BOOL bWait, int *pRequestID)		{ return ExecuteCommand(BNACMD_CANCELACCEPT3, bWait, pRequestID);}
BOOL CDiagBna::QueryBarcodeData(BOOL bWait, int *pRequestID)	{ return ExecuteCommand(BNACMD_QUERYBARCODEDATA, bWait, pRequestID);}
BOOL CDiagBna::GetEPVersion_Bill(BOOL bWait, int *pRequestID)	{ return ExecuteCommand(BNACMD_GETEPVERSION_BILL, bWait, pRequestID);}
BOOL CDiagBna::GetEPVersion_Variant(BOOL bWait, int *pRequestID){ return ExecuteCommand(BNACMD_GETEPVERSION_VARIANT, bWait, pRequestID);}

// properties
ULONG	CDiagBna::GetAcceptValue()			{ return m_ulAcceptValue; }
int		CDiagBna::GetAcceptCount()			{ return m_nAcceptCount; }
int		CDiagBna::GetRejectCount()			{ return m_nRejectCount; }
CString CDiagBna::GetBarcodeData()			{ return CString(m_wchBarcodeData); }
CString CDiagBna::GetEPVersion_Bill()		{ return CString(m_wchEPVersionBill); }
CString CDiagBna::GetEPVersion_Variant()	{ return CString(m_wchEPVersionVariant); }
