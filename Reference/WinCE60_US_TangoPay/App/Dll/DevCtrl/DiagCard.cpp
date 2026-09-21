#include "stdafx.h"
#include ".\Dev\DiagCard.h"

//#define NH_DEBUG
#include ".\Common\CmnLib.h"
#include ".\Common\NHDbgApi.h"

// 내부 Thread에게 동작을 명령하는 CMD
enum CARD_CMD
{
	CMD_NONE = 0,
	CDM_DEVICE_OPEN,
	CDM_DEVICE_CLOSE,
	CMD_INITIALIZE,
	CMD_ENABLE,
	CMD_DISABLE,
	CMD_EJECT,
	CMD_READ,
	CMD_READ_AGAIN,
	CMD_READ_TIMEOUT,
	CMD_GET_VERSION,
	CMD_GET_SENSOR,
	CMD_GET_LAST_ERROR,
	CMD_MAG_DEVICE_OPEN,
	CMD_MAG_DEVICE_CLOSE,
	CMD_MAG_INITIALIZE,
	CMD_MAG_POWERON,
	CMD_MAG_POWEROFF,
	CMD_MAG_LATCH,
	CMD_MAG_UNLATCH,
	CMD_MAG_ACCEPTCARD,
	CMD_MAG_READTRACK,
	CMD_MAG_REMOVECARD,
	CMD_SANKYO_DEVICE_OPEN,		// [#2078] NH KSK 2011.07.12
	CMD_SANKYO_DEVICE_CLOSE,
	CMD_SANKYO_INITIALIZE,
	CMD_SANKYO_ANTISKIMMINGSETUP,
	CMD_SANKYO_CHECK_ANTISKIMMINGSETUP,
	CMD_SANKYO_CHECK_ANTISKIMMINGSTATUS,
	CMD_SANKYO_POWERON,
	CMD_SANKYO_POWEROFF,
	CMD_SANKYO_READCARD			// end of [#2078]
};

CDiagCard::CDiagCard(int nMCUType)
{
	m_bInit		= FALSE;
	m_hMCU_DLL	= NULL;
	m_MCUType	= nMCUType;
	m_bPortOpen = FALSE;
	m_bPowerOn	= FALSE;
	m_bLatch	= FALSE;

	m_bError	= FALSE;	// [#2325] NH KSK 2015.01.25 Default 누락에 따른 초기화값 설정 (Open하지 않을 경우 NORMAL로 처리)

	/* Magtek Port Configuration */
	m_nPort			= 1;
	m_nBaud			= 1;
	m_nParity		= 1;
	m_nStop			= 1;
	m_nTraceLevel	= 1;

	// Load Dll
	if ( m_MCUType == 3 )
		m_hMCU_DLL = ::LoadLibrary(VDM_MCU_MAGTEK_FULL_PATH);
	else if (m_MCUType == 4)
		m_hMCU_DLL = ::LoadLibrary(VDM_MCU_SANKYO_FULL_PATH);	// [#2078] NH KSK 2011.07.12
	else
		m_hMCU_DLL = ::LoadLibrary(VDM_MCU_NHDIP_FULL_PATH);

	if (m_hMCU_DLL != NULL)
	{
		NHDEBUG(1, (_T(" CDiagCard::CDiagCard() LoadLibrary SUCCESS : 0x%x\n"), m_hMCU_DLL));
		/* Magtek */
		if(m_MCUType == 3)
		{
			m_MCU_FUNC.SetPropertyDLL		= (INT_FUNCTION_USHORT_ULONG_BYTE_BYTE_UINT*)GetProcAddress(m_hMCU_DLL, _LT("SetPropertyDLL"));
			m_MCU_FUNC.InitializeDLL		= (BOOL_FUNCTION_VOID*)GetProcAddress(m_hMCU_DLL, _LT("InitializeDLL"));
			m_MCU_FUNC.CloseDLL				= (BOOL_FUNCTION_VOID*)GetProcAddress(m_hMCU_DLL, _LT("CloseDLL"));
			m_MCU_FUNC.InsertCardDLL		= (INT_FUNCTION_VOID*)GetProcAddress(m_hMCU_DLL, _LT("InsertCardDLL"));
			m_MCU_FUNC.GetMediaStatusDLL	= (INT_FUNCTION_VOID*)GetProcAddress(m_hMCU_DLL, _LT("GetMediaStatusDLL"));
			m_MCU_FUNC.RemoveCardDLL		= (INT_FUNCTION_VOID*)GetProcAddress(m_hMCU_DLL, _LT("RemoveCardDLL"));
			m_MCU_FUNC.ReadTrackDLL			= (VOID_FUNCTION_PCHAR_PCHAR_PCHAR*)GetProcAddress(m_hMCU_DLL, _LT("ReadTrackDLL"));
			m_MCU_FUNC.ProcessUnLatch		= (BOOL_FUNCTION_VOID*)GetProcAddress(m_hMCU_DLL, _LT("ProcessUnLatch"));
			m_MCU_FUNC.ProcessLatch			= (BOOL_FUNCTION_VOID*)GetProcAddress(m_hMCU_DLL, _LT("ProcessLatch"));
			m_MCU_FUNC.ProcessChipSend		= (BOOL_FUNCTION_PBYTE_ULONG_PBYTE_PWORD*)GetProcAddress(m_hMCU_DLL, _LT("ProcessChipSend"));
			m_MCU_FUNC.ProcessPowerOn		= (BOOL_FUNCTION_PBYTE_PWORD*)GetProcAddress(m_hMCU_DLL, _LT("ProcessPowerOn"));
			m_MCU_FUNC.ProcessPowerOff		= (BOOL_FUNCTION_VOID*)GetProcAddress(m_hMCU_DLL, _LT("ProcessPowerOff"));
			m_MCU_FUNC.AcceptCardDLLForTimeout	= (INT_FUNCTION_DWORD*)GetProcAddress(m_hMCU_DLL, _LT("AcceptCardDLLForTimeout")); //[#581] SOOK 2009.11.15 MagTek CARD SCAN시 Timeout 적용
		}
		else if(m_MCUType == 4)		// [#2078] NH KSK 2011.07.12
		{
			m_MCU_FUNC.SANKYODIP_CARD_OpenConnection	= (DWORD_FUNCITON_VOID*)GetProcAddress(m_hMCU_DLL, _LT("CARD_OpenConnection"));
			m_MCU_FUNC.SANKYODIP_CARD_Initialize		= (DWORD_FUNCITON_VOID*)GetProcAddress(m_hMCU_DLL, _LT("CARD_Initialize"));
			m_MCU_FUNC.SANKYODIP_CARD_CloseConnection	= (DWORD_FUNCITON_VOID*)GetProcAddress(m_hMCU_DLL, _LT("CARD_CloseConnection"));
			m_MCU_FUNC.SANKYODIP_CARD_Sensor			= (DWORD_FUNCITON_LPBYTE_LPBYTE_LPBYTE*)GetProcAddress(m_hMCU_DLL, _LT("CARD_Sensor"));
			m_MCU_FUNC.SANKYODIP_CARD_MS_Read			= (DWORD_FUNCTION_PCARD_TRACK_DATA_DWORD*)GetProcAddress(m_hMCU_DLL, _LT("CARD_MS_Read"));
			m_MCU_FUNC.SANKYODIP_CARD_IC_PowerOn		= (DWORD_FUNCTION_LPBYTE_PWORD*)GetProcAddress(m_hMCU_DLL, _LT("CARD_IC_PowerOn"));
			m_MCU_FUNC.SANKYODIP_CARD_IC_PowerOff		= (DWORD_FUNCTION_VOID*)GetProcAddress(m_hMCU_DLL, _LT("CARD_IC_PowerOff"));
			m_MCU_FUNC.SANKYODIP_CARD_GetLastError		= (VOID_FUNCTION_PTCHAR*)GetProcAddress(m_hMCU_DLL, _LT("CARD_GetLastError"));
			m_MCU_FUNC.SANKYODIP_CARD_ESUSetup			= (DWORD_FUNCTION_LONG*)GetProcAddress(m_hMCU_DLL, _LT("CARD_ESUSetup"));
			m_MCU_FUNC.SANKYODIP_CARD_ESUCheck			= (DWORD_FUNCITON_LPBYTE_LPBYTE*)GetProcAddress(m_hMCU_DLL, _LT("CARD_ESUCheck"));
		}							// end of [#2078]
		else{
			m_MCU_FUNC.NHDIP_CARD_OpenConnection	= (BOOL_FUNCTION_VOID*)GetProcAddress(m_hMCU_DLL, _LT("NHDIP_CARD_OpenConnection"));
			m_MCU_FUNC.NHDIP_CARD_CloseConnection	= (VOID_FUNCTION_VOID*)GetProcAddress(m_hMCU_DLL, _LT("NHDIP_CARD_CloseConnection"));
			m_MCU_FUNC.NHDIP_CARD_GetLastError		= (VOID_FUNCTION_LPBYTE*)GetProcAddress(m_hMCU_DLL, _LT("NHDIP_CARD_GetLastError"));
			m_MCU_FUNC.NHDIP_CARD_GetVersion		= (VOID_FUNCTION_LPBYTE*)GetProcAddress(m_hMCU_DLL, _LT("NHDIP_CARD_GetVersion"));
			m_MCU_FUNC.NHDIP_CARD_Sensor			= (INT_FUNCTION_PBYTE*)GetProcAddress(m_hMCU_DLL, _LT("NHDIP_CARD_Sensor"));
			m_MCU_FUNC.NHDIP_CARD_Initialize		= (INT_FUNCTION_VOID*)GetProcAddress(m_hMCU_DLL, _LT("NHDIP_CARD_Initialize"));
			m_MCU_FUNC.NHDIP_CARD_Enable			= (INT_FUNCTION_VOID*)GetProcAddress(m_hMCU_DLL, _LT("NHDIP_CARD_Enable"));
			m_MCU_FUNC.NHDIP_CARD_Disable			= (INT_FUNCTION_VOID*)GetProcAddress(m_hMCU_DLL, _LT("NHDIP_CARD_Disable"));
			m_MCU_FUNC.NHDIP_CARD_Eject				= (INT_FUNCTION_VOID*)GetProcAddress(m_hMCU_DLL, _LT("NHDIP_CARD_Eject"));
			m_MCU_FUNC.NHDIP_CARD_Read				= (INT_FUNCTION_PCARD_TRACK_DATA*)GetProcAddress(m_hMCU_DLL, _LT("NHDIP_CARD_Read"));
			m_MCU_FUNC.NHDIP_CARD_Read_AGAIN		= (INT_FUNCTION_PCARD_TRACK_DATA*)GetProcAddress(m_hMCU_DLL, _LT("NHDIP_CARD_Read_AGAIN"));
			m_MCU_FUNC.NHDIP_CARD_Read_Read_Timeout	= (INT_FUNCTION_PCARD_TRACK_DATA_DWORD*)GetProcAddress(m_hMCU_DLL, _LT("NHDIP_CARD_Read_Read_Timeout"));
		}
		m_bInit = TRUE;
	}
	else
	{
		NHDEBUG(1, (_T(" CDiagCard::CDiagCard() LoadLibrary FAILE : 0x%x\n"), m_hMCU_DLL));
	}

	Resume();
}

CDiagCard::~CDiagCard()
{
	m_bDoingThread = FALSE;
	WaitForExitThread();

	// DLL Unload
	if (m_hMCU_DLL != NULL)
	{
		::FreeLibrary(m_hMCU_DLL);
		m_hMCU_DLL = NULL;
		
		memset(&m_MCU_FUNC, 0, sizeof(m_MCU_FUNC));
	}
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDiagCard
 FUNCTION NAME: CompleteAction()
 RETURN TYPE  : TRUE : 호출한 Method의 동작이 완료 되었다.
				FALSE : 호출한 Method가 동작 중이다.
 PARAMETER    : 
 DESCRIPTION  : ASync로 호출한 Method의 결과를 조회한다.
-------------------------------------------------------------------*/
BOOL CDiagCard::CompleteAction(int nRequestID, DWORD dwWaitMs)
{
	CNHCtrlCmdInfo	Response;
	
	if (nRequestID == -1)
		return FALSE;
	
	if (m_ResponseQueue.Dequeue(Response, dwWaitMs) == TRUE)
	{
		if (Response.m_nRequestID == nRequestID)
		{
			m_nResponseCmd = Response.m_nResponseCmd;
			NHDEBUG(1, (_T(" !!!!!! CDiagCard::CompleteAction() return : [%d]\n"), m_nResponseCmd));
			return m_nResponseCmd;
		}
		NHDEBUG(1, (_T(" !!!!!! CDiagCard::CompleteAction() else : [%d]\n"), m_nResponseCmd));
	}

	NHDEBUG(1, (_T(" !!!!!! CDiagCard::CompleteAction() else : IO_PROCESSING [%d], [%d] \n"), m_nResponseCmd, IO_PROCESSING));
	return IO_PROCESSING;
}

BOOL CDiagCard::Open(BOOL bWait, int *pRequestID)
{
	NHDEBUG(1, (_T("CDiagCard::Open m_MCUType[%d] \n"), m_MCUType));
	CNHAutoLock	nSyncObject(m_eMethodUse);
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
	if( m_MCUType == 3 )
		nDeviceCommand = (int)CMD_MAG_DEVICE_OPEN;
	else if (m_MCUType == 4)	
		nDeviceCommand = (int)CMD_SANKYO_DEVICE_OPEN;	// [#2078] NH KSK 2011.07.12
	else
		nDeviceCommand = (int)CDM_DEVICE_OPEN;

	NHDEBUG(1, (_T("CDiagCard::Open nDeviceCommand[%d] \n"), nDeviceCommand));

	CNHCtrlCmdInfo	CmdInfo(nRequestID, nDeviceCommand);
	m_RequestQueue.Enqueue(CmdInfo);
	
	/* SYNCHRONOUS CALL */
	if (bWait == TRUE)
	{
		BOOL bResult;

		/* WAIT I/O COMPLETED */
		bResult = CompleteAction(nRequestID/*, INFINITE */);
		
		if( bResult == MCU_SUCCESS)
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

BOOL CDiagCard::Close(BOOL bWait, int *pRequestID)
{
	CNHAutoLock	nSyncObject(m_eMethodUse);
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
	if( m_MCUType == 3 )
		nDeviceCommand = (int)CMD_MAG_DEVICE_CLOSE;
	else if (m_MCUType == 4)
		nDeviceCommand = (int)CMD_SANKYO_DEVICE_CLOSE;	// [#2078] NH KSK 2011.07.12
	else
		nDeviceCommand = (int)CDM_DEVICE_CLOSE;

	CNHCtrlCmdInfo	CmdInfo(nRequestID, nDeviceCommand);
	m_RequestQueue.Enqueue(CmdInfo);

	/* SYNCHRONOUS CALL */
	if (bWait == TRUE)
	{
		BOOL bResult;

		/* WAIT I/O COMPLETED */
		bResult = CompleteAction(nRequestID/*, INFINITE */);
		
		if( bResult == MCU_IO_COMPLETED)
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

BOOL CDiagCard::Initialize(BOOL bWait, int *pRequestID)
{
	CNHAutoLock	nSyncObject(m_eMethodUse);
	
	int nDeviceCommand;
	
	// SAVE REQUEST COMMAND ID
	int nRequestID = GetRequestID();

	// INITIALIZE RESPONSE RESULT
	m_nResponseCmd = -1;

	/* INITIALIZE MEMBER VARIABLE CONCERNED THIS COMMAND, HERE */

	/* INPUT PARAMETER SETTING HERE */

	/* SET REQUEST COMMAND */
	if( m_MCUType == 3 )
		nDeviceCommand = (int)CMD_MAG_INITIALIZE;
	else if (m_MCUType == 4)
		nDeviceCommand = (int)CMD_SANKYO_INITIALIZE;	// [#2078] NH KSK 2011.07.12
	else
		nDeviceCommand = (int)CMD_INITIALIZE;

	CNHCtrlCmdInfo	CmdInfo(nRequestID, nDeviceCommand);
	m_RequestQueue.Enqueue(CmdInfo);

	if (bWait == TRUE)
	{
		return CompleteAction(nRequestID);
	}
	else if (pRequestID != NULL)
	{
		*pRequestID = nRequestID;
	}

	return TRUE;
}

BOOL CDiagCard::CancleIO()
{
	m_RequestQueue.Clear();
	m_ResponseQueue.Clear();

	return MCU_SUCCESS;
}

BOOL CDiagCard::Enable(BOOL bWait, int *pRequestID)
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
	CNHCtrlCmdInfo	CmdInfo(nRequestID, (int)CMD_ENABLE);
	m_RequestQueue.Enqueue(CmdInfo);

	/* SYNCHRONOUS CALL */
	if (bWait == TRUE)
	{
		BOOL bResult;

		/* WAIT I/O COMPLETED */
		bResult = CompleteAction(nRequestID/*, INFINITE */);
		
		if( bResult == MCU_IO_COMPLETED)
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

BOOL CDiagCard::Disable(BOOL bWait, int *pRequestID)
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
	CNHCtrlCmdInfo	CmdInfo(nRequestID, (int)CMD_DISABLE);
	m_RequestQueue.Enqueue(CmdInfo);

	/* SYNCHRONOUS CALL */
	if (bWait == TRUE)
	{
		BOOL bResult;

		/* WAIT I/O COMPLETED */
		bResult = CompleteAction(nRequestID/*, INFINITE */);
		
		if( bResult == MCU_IO_COMPLETED)
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

BOOL CDiagCard::GetSensorInfo(LPBYTE lpbSensorData, DWORD dwSize, BOOL bWait, int *pRequestID)
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
		
		if( bResult == MCU_IO_COMPLETED)
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

BOOL CDiagCard::Read(PCARD_TRACK_DATA pTrackData, BOOL bWait, int *pRequestID)
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
	CNHCtrlCmdInfo	CmdInfo(nRequestID, (int)CMD_READ);
	m_RequestQueue.Enqueue(CmdInfo);

	/* SYNCHRONOUS CALL */
	if (bWait == TRUE)
	{
		BOOL bResult;

		/* WAIT I/O COMPLETED */
		bResult = CompleteAction(nRequestID/*, INFINITE */);
		
		if( bResult == MCU_IO_COMPLETED)
		{
			/* OUTPUT PARAMETER SETTING HERE */
			memcpy(pTrackData, &m_TrackData, sizeof(m_TrackData));
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

BOOL CDiagCard::Read_Again(PCARD_TRACK_DATA pTrackData, BOOL bWait, int *pRequestID)
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
	CNHCtrlCmdInfo	CmdInfo(nRequestID, (int)CMD_READ_AGAIN);
	m_RequestQueue.Enqueue(CmdInfo);

	/* SYNCHRONOUS CALL */
	if (bWait == TRUE)
	{
		BOOL bResult;

		/* WAIT I/O COMPLETED */
		bResult = CompleteAction(nRequestID/*, INFINITE */);
		
		if( bResult == MCU_IO_COMPLETED)
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

BOOL CDiagCard::Read_TimeOut(PCARD_TRACK_DATA pTrackData, DWORD dwTimeOut, BOOL bWait, int *pRequestID)
{
	CNHAutoLock	nSyncObject(m_eMethodUse);
	
	// SAVE REQUEST COMMAND ID
	int nRequestID = GetRequestID();
	
	// INITIALIZE RESPONSE RESULT
	m_nResponseCmd = -1;

	/* INITIALIZE MEMBER VARIABLE CONCERNED THIS COMMAND, HERE */
	/////////////////////////////////////////////////////////////
	memset(&m_TrackData, 0, sizeof(m_TrackData));

	/* INPUT PARAMETER SETTING HERE */
	//////////////////////////////////
	m_dwTimeOut = dwTimeOut;


	// Store Request Command in the queue
	CNHCtrlCmdInfo	CmdInfo(nRequestID, (int)CMD_READ_TIMEOUT);
	m_RequestQueue.Enqueue(CmdInfo);

	/* SYNCHRONOUS CALL */
	if (bWait == TRUE)
	{
		BOOL bResult;

		/* WAIT I/O COMPLETED */
		bResult = CompleteAction(nRequestID/*, INFINITE */);
		
		if( bResult == MCU_SUCCESS)
		{
			/* OUTPUT PARAMETER SETTING HERE */
			memcpy(pTrackData, &m_TrackData, sizeof(m_TrackData));
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

BOOL CDiagCard::GetLastError(BYTE bErrorCode[6], BOOL bWait, int *pRequestID)
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
		
		if( bResult == MCU_IO_COMPLETED	)
		{
			/* OUTPUT PARAMETER SETTING HERE */
			memcpy(bErrorCode, m_ErrorCode, sizeof(m_ErrorCode));;
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

BOOL CDiagCard::GetTrackData(PCARD_TRACK_DATA pTrackData)
{
	memcpy(pTrackData, &m_TrackData, sizeof(m_TrackData));
	return TRUE;
}

/************************************************/
/*				MAGTEK MCU COMMAND				*/
/************************************************/
BOOL CDiagCard::ICCardPowerOn(BYTE byATRData[1024], WORD* wATRLength, BOOL bWait, int *pRequestID)
{
	CNHAutoLock	nSyncObject(m_eMethodUse);
	
	int nDeviceCommand;
	
	// SAVE REQUEST COMMAND ID
	int nRequestID = GetRequestID();

	// INITIALIZE RESPONSE RESULT
	m_nResponseCmd = -1;

	/* INITIALIZE MEMBER VARIABLE CONCERNED THIS COMMAND, HERE */
	

	/* INPUT PARAMETER SETTING HERE */
	m_wATRLength = 0;
	memset(m_byATRData, NULL, sizeof(m_byATRData));


	/* SET REQUEST COMMAND */
	if( m_MCUType == 3 )
		nDeviceCommand = (int)CMD_MAG_POWERON;
	else if (m_MCUType == 4)
		nDeviceCommand = (int)CMD_SANKYO_POWERON;	// [#2078] NH KSK 2011.07.12
	else
		nDeviceCommand = (int)CMD_INITIALIZE; /* DUMMY COMMAND */

	CNHCtrlCmdInfo	CmdInfo(nRequestID, nDeviceCommand);
	m_RequestQueue.Enqueue(CmdInfo);

	/* SYNCHRONOUS CALL */
	if (bWait == TRUE)
	{
		BOOL bResult;

		bResult = CompleteAction(nRequestID);
		if( bResult == MCU_SUCCESS)
		{
			*wATRLength = m_wATRLength;
			memcpy(byATRData, m_byATRData, m_wATRLength);
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

/************************************************/
/*				MAGTEK MCU COMMAND				*/
/************************************************/
BOOL CDiagCard::ICCardPowerOff(BOOL bWait, int *pRequestID)
{
	CNHAutoLock	nSyncObject(m_eMethodUse);
	
	int nDeviceCommand;
	
	// SAVE REQUEST COMMAND ID
	int nRequestID = GetRequestID();

	// INITIALIZE RESPONSE RESULT
	m_nResponseCmd = -1;

	/* INITIALIZE MEMBER VARIABLE CONCERNED THIS COMMAND, HERE */
	

	/* INPUT PARAMETER SETTING HERE */
	m_wATRLength = 0;
	memset(m_byATRData, NULL, sizeof(m_byATRData));


	/* SET REQUEST COMMAND */
	if( m_MCUType == 3 )
		nDeviceCommand = (int)CMD_MAG_POWEROFF;
	else if (m_MCUType == 4)
		nDeviceCommand = (int)CMD_SANKYO_POWEROFF;	// [#2078] NH KSK 2011.07.12
	else
		nDeviceCommand = (int)CMD_INITIALIZE; /* DUMMY COMMAND */

	CNHCtrlCmdInfo	CmdInfo(nRequestID, nDeviceCommand);
	m_RequestQueue.Enqueue(CmdInfo);

	/* SYNCHRONOUS CALL */
	if (bWait == TRUE)
	{
		return CompleteAction(nRequestID);
	}

	/* ASYNCHRONOUS CALL */
	else if (pRequestID != NULL)
	{
		*pRequestID = nRequestID;
	}

	return TRUE;
}

/************************************************/
/*				MAGTEK MCU COMMAND				*/
/************************************************/
BOOL CDiagCard::ICCardLatch(BOOL bWait, int *pRequestID)
{
	CNHAutoLock	nSyncObject(m_eMethodUse);
	
	int nDeviceCommand;
	
	// SAVE REQUEST COMMAND ID
	int nRequestID = GetRequestID();

	// INITIALIZE RESPONSE RESULT
	m_nResponseCmd = -1;

	/* INITIALIZE MEMBER VARIABLE CONCERNED THIS COMMAND, HERE */
	

	/* INPUT PARAMETER SETTING HERE */
	m_wATRLength = 0;
	memset(m_byATRData, NULL, sizeof(m_byATRData));


	/* SET REQUEST COMMAND */
	if( m_MCUType == 3 )
		nDeviceCommand = (int)CMD_MAG_LATCH;
	else
		nDeviceCommand = (int)CMD_INITIALIZE; /* DUMMY COMMAND */	// SANKYO는 Latch 제어 안됨

	CNHCtrlCmdInfo	CmdInfo(nRequestID, nDeviceCommand);
	m_RequestQueue.Enqueue(CmdInfo);

	/* SYNCHRONOUS CALL */
	if (bWait == TRUE)
	{
		return CompleteAction(nRequestID);
	}

	/* ASYNCHRONOUS CALL */
	else if (pRequestID != NULL)
	{
		*pRequestID = nRequestID;
	}

	return TRUE;
}

/************************************************/
/*				MAGTEK MCU COMMAND				*/
/************************************************/
BOOL CDiagCard::ICCardUnLatch(BOOL bWait, int *pRequestID)
{
	CNHAutoLock	nSyncObject(m_eMethodUse);
	
	int nDeviceCommand;
	
	// SAVE REQUEST COMMAND ID
	int nRequestID = GetRequestID();

	// INITIALIZE RESPONSE RESULT
	m_nResponseCmd = -1;

	/* INITIALIZE MEMBER VARIABLE CONCERNED THIS COMMAND, HERE */
	

	/* INPUT PARAMETER SETTING HERE */
	m_wATRLength = 0;
	memset(m_byATRData, NULL, sizeof(m_byATRData));


	/* SET REQUEST COMMAND */
	if( m_MCUType == 3 )
		nDeviceCommand = (int)CMD_MAG_UNLATCH;
	else
		nDeviceCommand = (int)CMD_INITIALIZE; /* DUMMY COMMAND */	// Sankyo는 Latch 제어 안됨

	CNHCtrlCmdInfo	CmdInfo(nRequestID, nDeviceCommand);
	m_RequestQueue.Enqueue(CmdInfo);

	/* SYNCHRONOUS CALL */
	if (bWait == TRUE)
	{
		return CompleteAction(nRequestID);
	}

	/* ASYNCHRONOUS CALL */
	else if (pRequestID != NULL)
	{
		*pRequestID = nRequestID;
	}

	return TRUE;
}

/************************************************/
/*				MAGTEK MCU COMMAND				*/
/************************************************/
BOOL CDiagCard::ICCardAccept(DWORD dwTimeOut, BOOL bWait, int *pRequestID)//[#581] SOOK 2009.11.15 MagTek CARD SCAN시 Timeout 적용
{
	CNHAutoLock	nSyncObject(m_eMethodUse);
	
	int nDeviceCommand;
	
	// SAVE REQUEST COMMAND ID
	int nRequestID = GetRequestID();

	// INITIALIZE RESPONSE RESULT
	m_nResponseCmd = -1;

	/* INITIALIZE MEMBER VARIABLE CONCERNED THIS COMMAND, HERE */
	

	/* INPUT PARAMETER SETTING HERE */
	m_wATRLength = 0;
	memset(m_byATRData, NULL, sizeof(m_byATRData));

	if (m_MCUType == 4)
		memset(&m_TrackData, 0, sizeof(m_TrackData));	// [#2078] NH KSK 2011.07.15

	m_dwTimeOut = dwTimeOut;//[#581] SOOK 2009.11.15 MagTek CARD SCAN시 Timeout 적용

	/* SET REQUEST COMMAND */
	if( m_MCUType == 3 )
		nDeviceCommand = (int)CMD_MAG_ACCEPTCARD;
	else if (m_MCUType == 4)
		nDeviceCommand = (int)CMD_SANKYO_READCARD;	// [#2078] NH KSK 2011.07.12
	else
		nDeviceCommand = (int)CMD_INITIALIZE; /* DUMMY COMMAND */

	CNHCtrlCmdInfo	CmdInfo(nRequestID, nDeviceCommand);
	m_RequestQueue.Enqueue(CmdInfo);

	/* SYNCHRONOUS CALL */
	if (bWait == TRUE)
	{
		return CompleteAction(nRequestID);
	}

	/* ASYNCHRONOUS CALL */
	else if (pRequestID != NULL)
	{
		*pRequestID = nRequestID;
	}

	return TRUE;
}

/************************************************/
/*				MAGTEK MCU COMMAND				*/
/************************************************/
BOOL CDiagCard::ICCardRemove(BOOL bWait, int *pRequestID)
{
	CNHAutoLock	nSyncObject(m_eMethodUse);
	
	int nDeviceCommand;
	
	// SAVE REQUEST COMMAND ID
	int nRequestID = GetRequestID();
	
	// INITIALIZE RESPONSE RESULT
	m_nResponseCmd = -1;
	
	/* INITIALIZE MEMBER VARIABLE CONCERNED THIS COMMAND, HERE */
	
	
	/* INPUT PARAMETER SETTING HERE */

	
	/* SET REQUEST COMMAND */
	if( m_MCUType == 3 )
		nDeviceCommand = (int)CMD_MAG_REMOVECARD;
	else
		nDeviceCommand = (int)CMD_INITIALIZE; /* DUMMY COMMAND */
	
	CNHCtrlCmdInfo	CmdInfo(nRequestID, nDeviceCommand);
	m_RequestQueue.Enqueue(CmdInfo);
	
	/* SYNCHRONOUS CALL */
	if (bWait == TRUE)
	{
		return CompleteAction(nRequestID);
	}
	
	/* ASYNCHRONOUS CALL */
	else if (pRequestID != NULL)
	{
		*pRequestID = nRequestID;
	}
	
	return TRUE;
}

// ICCardRead 사용 안함
/************************************************/
/*				MAGTEK MCU COMMAND				*/
/************************************************/
BOOL CDiagCard::ICCardRead(PCARD_TRACK_DATA pTrackData, BOOL bWait, int *pRequestID)
{
	CNHAutoLock	nSyncObject(m_eMethodUse);

	if(m_MCUType == 4)	// [#2078] NH KSK 2011.07.16
	{
		memcpy(pTrackData, &m_TrackData, sizeof(m_TrackData));
		return MCU_SUCCESS;
	}
	
	int nDeviceCommand;
	
	// SAVE REQUEST COMMAND ID
	int nRequestID = GetRequestID();

	// INITIALIZE RESPONSE RESULT
	m_nResponseCmd = -1;

	/* INITIALIZE MEMBER VARIABLE CONCERNED THIS COMMAND, HERE */
	memset(&m_TrackData, 0, sizeof(m_TrackData));

	/* INPUT PARAMETER SETTING HERE */


	/* SET REQUEST COMMAND */
	if( m_MCUType == 3 )
		nDeviceCommand = (int)CMD_MAG_READTRACK;
	else
		nDeviceCommand = (int)CMD_INITIALIZE; /* DUMMY COMMAND */

	CNHCtrlCmdInfo	CmdInfo(nRequestID, nDeviceCommand);
	m_RequestQueue.Enqueue(CmdInfo);

	/* SYNCHRONOUS CALL */
	if (bWait == TRUE)
	{
		BOOL bResult;
		
		/* WAIT I/O COMPLETED */
		bResult = CompleteAction(nRequestID/*, INFINITE */);
		
		if( bResult == MCU_SUCCESS)
		{
			/* OUTPUT PARAMETER SETTING HERE */
			memcpy(pTrackData, &m_TrackData, sizeof(m_TrackData));
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

BOOL CDiagCard::IsDeviceError()
{
	if( m_bError == TRUE)
		return TRUE;
	else
		return FALSE;
}

void CDiagCard::ClearDeviceError()
{
	m_bError = FALSE;
}

CString CDiagCard::GetErrorString()
{
	// MAGTEK일 경우에만 사용하나, DLL에서 GetLastError 함수가 없으므로 AP가 임의로 Error를 Set해야함
NHDEBUG(1, (_T("CDiagCard::GetErrorString()\n")));

	return m_strErroCode;
}

// [#2078] NH KSK 2011.07.15
BOOL CDiagCard::AntiSkimmingSetup(int nTimeThreshold, BOOL bWait, int *pRequestID)
{
	CNHAutoLock	nSyncObject(m_eMethodUse);

	int nDeviceCommand;

	// SAVE REQUEST COMMAND ID
	int nRequestID = GetRequestID();

	// INITIALIZE RESPONSE RESULT
	m_nResponseCmd = -1;
	m_nAntiSkimmingTimeThreshold = nTimeThreshold;

	/* INPUT PARAMETER SETTING HERE */


	/* SET REQUEST COMMAND */
	if( m_MCUType == 4 )
		nDeviceCommand = (int)CMD_SANKYO_ANTISKIMMINGSETUP;
	else
		return FALSE;

	CNHCtrlCmdInfo	CmdInfo(nRequestID, nDeviceCommand);
	m_RequestQueue.Enqueue(CmdInfo);

	/* SYNCHRONOUS CALL */
	if (bWait == TRUE)
	{
		BOOL bResult;

		/* WAIT I/O COMPLETED */
		bResult = CompleteAction(nRequestID/*, INFINITE */);

		if( bResult == MCU_SUCCESS)
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

BOOL CDiagCard::CheckAntiSkimmingSetup(BOOL bWait, int *pRequestID)	
{
	CNHAutoLock	nSyncObject(m_eMethodUse);

	int nDeviceCommand;

	// SAVE REQUEST COMMAND ID
	int nRequestID = GetRequestID();

	// INITIALIZE RESPONSE RESULT
	m_nResponseCmd = -1;
	memset(m_byVoltage, 0, sizeof(m_byVoltage));
	memset(m_byTemperature, 0, sizeof(m_byTemperature));

	/* INPUT PARAMETER SETTING HERE */


	/* SET REQUEST COMMAND */
	if( m_MCUType == 4 )
		nDeviceCommand = (int)CMD_SANKYO_CHECK_ANTISKIMMINGSETUP;
	else
		return FALSE;

	CNHCtrlCmdInfo	CmdInfo(nRequestID, nDeviceCommand);
	m_RequestQueue.Enqueue(CmdInfo);

	/* SYNCHRONOUS CALL */
	if (bWait == TRUE)
	{
		BOOL bResult;

		/* WAIT I/O COMPLETED */
		bResult = CompleteAction(nRequestID/*, INFINITE */);

		if( bResult == MCU_SUCCESS)
		{
			/* OUTPUT PARAMETER SETTING HERE */

		}
	}

	/* ASYNCHRONOUS CALL */
	else if (pRequestID != NULL)
	{
		*pRequestID = nRequestID;
	}

	return TRUE;
}

BOOL CDiagCard::CheckAntiSkimmingStatus(BOOL bWait, int *pRequestID)
{
	CNHAutoLock	nSyncObject(m_eMethodUse);

	int nDeviceCommand;

	// SAVE REQUEST COMMAND ID
	int nRequestID = GetRequestID();

	// INITIALIZE RESPONSE RESULT
	m_nResponseCmd = -1;
	memset(m_byAntiSkimmingStatus, 0, sizeof(m_byAntiSkimmingStatus));

	/* INPUT PARAMETER SETTING HERE */


	/* SET REQUEST COMMAND */
	if( m_MCUType == 4 )
		nDeviceCommand = (int)CMD_SANKYO_CHECK_ANTISKIMMINGSTATUS;
	else
		return FALSE;

	CNHCtrlCmdInfo	CmdInfo(nRequestID, nDeviceCommand);
	m_RequestQueue.Enqueue(CmdInfo);

	/* SYNCHRONOUS CALL */
	if (bWait == TRUE)
	{
		BOOL bResult;

		/* WAIT I/O COMPLETED */
		bResult = CompleteAction(nRequestID/*, INFINITE */);

		if( bResult == MCU_SUCCESS)
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

CString CDiagCard::GetAntiSkimmingVoltage()
{
	CString strVoltage;

	if (IsDeviceError())
		strVoltage = L"UNKNOWN";
	else
		strVoltage.Format(L"%d", Asc2Int(m_byVoltage, 4));
	return strVoltage;
}

CString CDiagCard::GetAntiSkimmingTemperature()
{
	CString strTemperature;
	if (IsDeviceError())
		strTemperature = L"UNKNOWN";
	else
		strTemperature.Format(L"%c%c%c.%c", m_byTemperature[0], m_byTemperature[1], m_byTemperature[2], m_byTemperature[3]);
	return strTemperature;
}

CString CDiagCard::GetAntiSkimmingStatus()
{
	CString strAntiSkimmingStatus;
//	if (IsDeviceError())
//		strAntiSkimmingStatus = L"UNKNOWN";
//	else
	{
		if (m_byAntiSkimmingStatus[2] == (BYTE)0x50)
			strAntiSkimmingStatus = L"NORMAL";
		else if (m_byAntiSkimmingStatus[2] == (BYTE)0x51)
		{
			if (IsDeviceError())
				strAntiSkimmingStatus = L"DETECTED";
			else
				strAntiSkimmingStatus = L"SKIMMING";
		}
		else if (m_byAntiSkimmingStatus[2] == (BYTE)0x40)
			strAntiSkimmingStatus = L"NO ESU";
		else
			strAntiSkimmingStatus = L"UNKNOWN";
	}

	// EP Firmware Bug로 로직 추가 추후 EP Firmware 수정 시 없어져야할 코드임
	if (m_byAntiSkimmingStatus[2] == (BYTE)0x50 && IsDeviceError())
		strAntiSkimmingStatus = L"DETECTED";

	return strAntiSkimmingStatus;
}

// end of [#2078]

unsigned CDiagCard::ThreadHandlerProc()
{
	int			nReturn = FALSE;

	CNHCtrlCmdInfo	CmdInfo;
	m_bDoingThread = TRUE;

	//NHDEBUG(1, (_T("CDiagCard::ThreadHandlerProc() Command execute Process Start()\n")));

	while(m_bDoingThread)
	{
		nReturn = FALSE;
		CmdInfo.m_nRequestCmd = (int)CMD_NONE;

		// Wait for Request Cmd
		if (m_RequestQueue.Dequeue(CmdInfo, 100) == TRUE)
		{
			NHDEBUG(1, (_T("CDiagCard::ThreadHandlerProc() Dequeue(CmdInfo.RequestID(%d)\n"), CmdInfo.m_nRequestCmd));
			switch ((int)CmdInfo.m_nRequestCmd)
			{
			case CDM_DEVICE_OPEN:
				/* OPEN SERIAL PORT */
				NHDEBUG(1, (_T("CDiagCard::ThreadHandlerProc() NHDIP_CARD_OpenConnection\n")));
				nReturn = m_MCU_FUNC.NHDIP_CARD_OpenConnection();
				if(nReturn)
				{
					m_bPortOpen = TRUE;
					nReturn = MCU_SUCCESS;
					NHDEBUG(1, (_T("CDiagCard::ThreadHandlerProc() NHDIP_CARD_OpenConnection SUCCESS nReturn[%d] \n"), nReturn));
				}
				else
				{
					m_bPortOpen = FALSE;
					nReturn = MCU_FAIL;
					NHDEBUG(1, (_T("CDiagCard::ThreadHandlerProc() NHDIP_CARD_OpenConnection SUCCESS nReturn[%d] \n"), nReturn));
				}
				break;

			case CDM_DEVICE_CLOSE:
				m_MCU_FUNC.NHDIP_CARD_CloseConnection();
				nReturn = MCU_SUCCESS;

				break;
			case CMD_INITIALIZE:
				m_MCU_FUNC.NHDIP_CARD_Initialize();
				nReturn = MCU_SUCCESS;
				break;
			case CMD_ENABLE:
				nReturn = m_MCU_FUNC.NHDIP_CARD_Enable();
				break;
			case CMD_DISABLE:
				nReturn = m_MCU_FUNC.NHDIP_CARD_Disable();
				break;
			case CMD_READ:
				nReturn = m_MCU_FUNC.NHDIP_CARD_Read(&m_TrackData);

				if( nReturn == 1 )
					nReturn = MCU_SUCCESS;
				break;
			case CMD_READ_AGAIN:
				nReturn = m_MCU_FUNC.NHDIP_CARD_Read_AGAIN(&m_TrackData);
				break;
			case CMD_READ_TIMEOUT:
				nReturn = m_MCU_FUNC.NHDIP_CARD_Read_Read_Timeout(&m_TrackData, m_dwTimeOut);

				NHDEBUG(1, (_T("!!!!!!!!!!!!!!!!!!!!!!!!! NHDIP_CARD_Read_Read_Timeout: nReturn [%d]\n"), nReturn));

				if( nReturn == 1 )
				{
					nReturn = MCU_SUCCESS;
					NHDEBUG(1, (_T("CDiagCard::ThreadHandlerProc() NHDIP_CARD_Read: MCU_SUCCESS\n")));
				}
				else if ( nReturn == 2 )
				{
					NHDEBUG(1, (_T("CDiagCard::ThreadHandlerProc() NHDIP_CARD_Read_Read_Timeout: TIME OUT !!!!!!\n")));
				}
				else if ( nReturn == 0 )
				{
					NHDEBUG(1, (_T("CDiagCard::ThreadHandlerProc() NHDIP_CARD_Read_Read_Timeout [%d]\n"), nReturn));
				}
					
				break;
			case CMD_GET_VERSION:
				break;
			case CMD_GET_SENSOR:
				nReturn	= m_MCU_FUNC.NHDIP_CARD_Sensor(m_Status);
				break;
			case CMD_MAG_DEVICE_OPEN:
				{
					// [#2022] NH KSK 2011.02.22 Code Sonar 지적사항 대책
					DWORD dwRes = 0, dwDisposition = 0;
					DWORD dwBufLen = 0;
					LONG lRes = 0;
					HKEY hkSub = NULL;
					// end of [#2022]

					lRes = RegCreateKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\ATM\\DEVINFO\\IDC"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ, NULL, &hkSub, &dwDisposition);

					if (lRes == MCU_SUCCESS)
					{
						lRes = RegQueryValueEx(hkSub, _T("PortNumber"), 0, NULL, (LPBYTE)&dwRes, &dwBufLen);
						if (lRes != ERROR_SUCCESS)
						{
							NHDEBUG(1, (_T("CDiagCard::ThreadHandlerProc() CMD_MAG_DEVICE_OPEN: RegQueryValueEx(PortNumber) FAIL !!!!!!\n")));
							m_nPort = 1;
						}
						else
						{
							m_nPort = dwRes;
							NHDEBUG(1, (_T("CDiagCard::ThreadHandlerProc() CMD_MAG_DEVICE_OPEN: RegQueryValueEx(PortNumber [%d] ) SUCCESS !!!!!!\n"), m_nPort));
						}

						lRes = RegQueryValueEx(hkSub, _T("Baudrate"), 0, NULL, (LPBYTE)&dwRes, &dwBufLen);
						if (lRes != MCU_SUCCESS)
						{
							NHDEBUG(1, (_T("CDiagCard::ThreadHandlerProc() CMD_MAG_DEVICE_OPEN: RegQueryValueEx(Baudrate) FAIL !!!!!!\n")));
							m_nBaud = 1;
						}
						else
						{
							m_nBaud = dwRes;
							NHDEBUG(1, (_T("CDiagCard::ThreadHandlerProc() CMD_MAG_DEVICE_OPEN: RegQueryValueEx(Baudrate [%d] ) SUCCESS !!!!!!\n"), m_nBaud));
						}

						lRes = RegQueryValueEx(hkSub, _T("ParityBits"), 0, NULL, (LPBYTE)&dwRes, &dwBufLen);
						if (lRes != MCU_SUCCESS)
						{
							NHDEBUG(1, (_T("CDiagCard::ThreadHandlerProc() CMD_MAG_DEVICE_OPEN: RegQueryValueEx(ParityBits) FAIL !!!!!!\n")));
							m_nParity = 1;
						}
						else
						{
							m_nParity = dwRes;
							NHDEBUG(1, (_T("CDiagCard::ThreadHandlerProc() CMD_MAG_DEVICE_OPEN: RegQueryValueEx(ParityBits [%d] ) SUCCESS !!!!!!\n"), m_nParity));
						}

						lRes = RegQueryValueEx(hkSub, _T("StopBits"), 0, NULL, (LPBYTE)&dwRes, &dwBufLen);
						if (lRes != MCU_SUCCESS)
						{
							NHDEBUG(1, (_T("CDiagCard::ThreadHandlerProc() CMD_MAG_DEVICE_OPEN: RegQueryValueEx(StopBits) FAIL !!!!!!\n")));
							m_nStop = 1;
						}
						else
						{
							m_nStop = dwRes;
							NHDEBUG(1, (_T("CDiagCard::ThreadHandlerProc() CMD_MAG_DEVICE_OPEN: RegQueryValueEx(StopBits [%d] ) SUCCESS !!!!!!\n"), m_nStop));
						}
						m_nTraceLevel = 1;
						RegCloseKey(hkSub);
					}
					else
					{
						m_nPort	= 1;
						m_nBaud	= 1;
						m_nParity = 1;
						m_nStop	= 1;
						m_nTraceLevel = 1;
					}
					/* SET PORT CONFIGUATION */
					nReturn = m_MCU_FUNC.SetPropertyDLL((USHORT)m_nPort, (ULONG)m_nBaud,(BYTE)m_nParity, (BYTE)m_nStop, (UINT)m_nTraceLevel);

					NHDEBUG(1, (_T("CDiagCard::ThreadHandlerProc() CMD_MAG_DEVICE_OPEN RESULT: [%d] !!!!!!\n"), nReturn));
					
					if( nReturn == MCU_SUCCESS)
					{
						m_bPortOpen = TRUE;
						m_bError	= FALSE;
					}
					else
					{
						m_bPortOpen = FALSE;
						m_bError	= TRUE;
						// ERROR CODE를 SET 해야함
					}
				}
				break;
			case CMD_MAG_DEVICE_CLOSE:
				{
					nReturn = m_MCU_FUNC.CloseDLL();

					NHDEBUG(1, (_T("CDiagCard::ThreadHandlerProc() CMD_MAG_DEVICE_CLOSE RESULT: [%d] !!!!!!\n"), nReturn));

					m_bPortOpen = FALSE;
				}
				break;

			case CMD_MAG_INITIALIZE:
				{
					m_strErroCode.Empty();
					m_bError	= FALSE;

					nReturn = m_MCU_FUNC.InitializeDLL();
					NHDEBUG(1, (_T("CDiagCard::ThreadHandlerProc() CMD_MAG_INITIALIZE RESULT: [%d] !!!!!!\n"), nReturn));
					
					if (nReturn == 1)	// KSK 2009.07.27 MAGTEK은 SUCCESS값이 1이다. (VDM SOURCE 참고)
					{
						nReturn = MCU_SUCCESS;
						m_bError	= FALSE;
					}
					else
					{
						nReturn = MCU_FAIL;
						m_bError	= TRUE;
						// ERROR CODE를 SET 해 (기존 VDM에 ERROR CODE가 미존재)
						m_strErroCode.Format(L"9723010");		// Device Open Fail로 Set
					}
				}
				break;

			case CMD_MAG_POWERON:
				{
					m_strErroCode.Empty();
					m_bError = FALSE;

					// Data Initialize
					memset(m_byATRData, 0, sizeof(m_byATRData));
					m_wATRLength = 0;

					nReturn = m_MCU_FUNC.ProcessPowerOn(m_byATRData, &m_wATRLength);
					NHDEBUG(1, (_T("CDiagCard::ThreadHandlerProc() CMD_MAG_POWERON RESULT: [%d] LEN[%d] !!!!!!\n"), nReturn, m_wATRLength));
					
					if (nReturn == MCU_SUCCESS && m_wATRLength)
					{
						// Power On Success
					}
					else
					{
						nReturn		= MCU_FAIL;
					}
				}
				break;
			case CMD_MAG_POWEROFF:
				{
					nReturn = m_MCU_FUNC.ProcessPowerOff();
					NHDEBUG(1, (_T("CDiagCard::ThreadHandlerProc() CMD_MAG_POWEROFF RESULT: [%d] !!!!!!\n"), nReturn));
				}
				break;
			case CMD_MAG_LATCH:
				{
					nReturn = m_MCU_FUNC.ProcessLatch();
					NHDEBUG(1, (_T("CDiagCard::ThreadHandlerProc() CMD_MAG_LATCH RESULT: [%d] !!!!!!\n"), nReturn));

					if (nReturn == 0)
						nReturn = MCU_FAIL;
					else
						nReturn = MCU_SUCCESS;
				}
				break;
			case CMD_MAG_UNLATCH:
				{
					nReturn = m_MCU_FUNC.ProcessUnLatch();
					NHDEBUG(1, (_T("CDiagCard::ThreadHandlerProc() CMD_MAG_UNLATCH RESULT: [%d] !!!!!!\n"), nReturn));

					if (nReturn == 0)
						nReturn = MCU_FAIL;
					else
						nReturn = MCU_SUCCESS;
				}
				break;
			case CMD_MAG_ACCEPTCARD:
				{
					m_strErroCode.Empty();
					m_bError = FALSE;

					nReturn = m_MCU_FUNC.AcceptCardDLLForTimeout(m_dwTimeOut); //[#581] SOOK 2009.11.15 MagTek CARD SCAN시 Timeout 적용
					NHDEBUG(1, (_T("CDiagCard::ThreadHandlerProc() CMD_MAG_ACCEPTCARD Timeout=[%d], RESULT: [%d] !!!!!!\n"),m_dwTimeOut, nReturn));

					if (nReturn == -2)
					{
						m_strErroCode.Format(L"9723019");
						m_bError = TRUE;
					}
					else if (nReturn == -1)
					{
						m_strErroCode.Format(L"9723011");
						m_bError = TRUE;
					}
				}
				break;
			case CMD_MAG_READTRACK:
				{
					m_MCU_FUNC.ReadTrackDLL(m_TrackData.Iso1_Data, m_TrackData.Iso2_Data, m_TrackData.Iso3_Data);
					
					m_TrackData.Iso1_Length = strlen(m_TrackData.Iso1_Data);
					m_TrackData.Iso2_Length = strlen(m_TrackData.Iso2_Data);
					m_TrackData.Iso3_Length = strlen(m_TrackData.Iso3_Data);
					
					nReturn = MCU_SUCCESS;
				}
				break;
			case CMD_MAG_REMOVECARD:
				{
					m_strErroCode.Empty();
					m_bError = FALSE;

					nReturn = m_MCU_FUNC.RemoveCardDLL();
 
					/* SUCCESS TO REMOVE CARD */
					if (nReturn == 1)
					{
						m_bError = FALSE;
					}
					/* FAIL TO REMOVE CARD */
					else if (nReturn == -1)
					{
						m_strErroCode.Format(L"9723011");		// Device Communication Fail로 Set
						m_bError = TRUE;
					}
				}
				break;

			// [#2078] NH KSK 2011.07.14
			case CMD_SANKYO_DEVICE_OPEN:
				{
					/* OPEN SERIAL PORT */
					NHDEBUG(1, (_T("CDiagCard::ThreadHandlerProc() SANKYODIP_CARD_OpenConnection\n")));
					nReturn = m_MCU_FUNC.SANKYODIP_CARD_OpenConnection();
					if(nReturn == MCU_SUCCESS)
					{
						m_bPortOpen = TRUE;
						nReturn = MCU_SUCCESS;
						NHDEBUG(1, (_T("CDiagCard::ThreadHandlerProc() SANKYODIP_CARD_OpenConnection SUCCESS nReturn[%d] \n"), nReturn));
					}
					else
					{
						m_bPortOpen = FALSE;
						nReturn = MCU_FAIL;
						NHDEBUG(1, (_T("CDiagCard::ThreadHandlerProc() SANKYODIP_CARD_OpenConnection SUCCESS nReturn[%d] \n"), nReturn));
					}
					break;
				}
				break;

			case CMD_SANKYO_DEVICE_CLOSE:
				{
					m_MCU_FUNC.SANKYODIP_CARD_CloseConnection();

					NHDEBUG(1, (_T("CDiagCard::ThreadHandlerProc() CMD_SANKYO_DEVICE_CLOSE RESULT: [%d] !!!!!!\n"), nReturn));

					m_bPortOpen = FALSE;
				}
				break;

			case CMD_SANKYO_INITIALIZE:
				{
					/* Initialize Device */
					m_strErroCode.Empty();
					m_bError	= FALSE;

					nReturn = m_MCU_FUNC.SANKYODIP_CARD_Initialize();
					NHDEBUG(1, (_T("CDiagCard::ThreadHandlerProc() CMD_MAG_INITIALIZE RESULT: [%d] !!!!!!\n"), nReturn));

					if (nReturn == MCU_SUCCESS)
					{
						nReturn = MCU_SUCCESS;
						m_bError	= FALSE;
					}
					else
					{
						nReturn = MCU_FAIL;
						m_bError	= TRUE;
						TCHAR ErrorCode[16] = { 0, };

						m_MCU_FUNC.SANKYODIP_CARD_GetLastError(ErrorCode);
						m_strErroCode = ErrorCode;
					}
				}
				break;

			case CMD_SANKYO_ANTISKIMMINGSETUP:
				{
					nReturn = m_MCU_FUNC.SANKYODIP_CARD_ESUSetup((LONG)m_nAntiSkimmingTimeThreshold);
					NHDEBUG(1, (_T("CDiagCard::ThreadHandlerProc() CMD_SANKYO_ANTISKIMMINGSETUP RESULT: [%d] !!!!!!\n"), nReturn));

					if (nReturn == MCU_SUCCESS)
					{
						nReturn = MCU_SUCCESS;
						m_bError	= FALSE;
					}
					else
					{
						nReturn = MCU_FAIL;
						m_bError	= TRUE;
						TCHAR ErrorCode[16] = { 0, };

						m_MCU_FUNC.SANKYODIP_CARD_GetLastError(ErrorCode);
						m_strErroCode = ErrorCode;
					}
				}
				break;

			case CMD_SANKYO_CHECK_ANTISKIMMINGSETUP:
				{
					memset(m_byVoltage, 0, sizeof(m_byVoltage));
					memset(m_byTemperature, 0, sizeof(m_byTemperature));
					nReturn = m_MCU_FUNC.SANKYODIP_CARD_ESUCheck(m_byVoltage, m_byTemperature);
					NHDEBUG(1, (_T("CDiagCard::ThreadHandlerProc() CMD_SANKYO_CHECK_ANTISKIMMINGSETUP RESULT: [%d] !!!!!!\n"), nReturn));
					NHDEBUG(1, (_T("CDiagCard::ThreadHandlerProc() CMD_SANKYO_CHECK_ANTISKIMMINGSETUP DATA: VolTage[%S] Temperature[%S] !!!!!!\n"), m_byVoltage, m_byTemperature));

					if (nReturn == MCU_SUCCESS)
					{
						nReturn = MCU_SUCCESS;
						m_bError = FALSE;
					}
					else
					{
						nReturn = MCU_FAIL;
						m_bError = TRUE;

						TCHAR ErrorCode[16] = { 0, };

						m_MCU_FUNC.SANKYODIP_CARD_GetLastError(ErrorCode);
						m_strErroCode = ErrorCode;
					}
				}
				break;

			case CMD_SANKYO_CHECK_ANTISKIMMINGSTATUS:
				{
					memset(m_byAntiSkimmingStatus, 0, sizeof(m_byAntiSkimmingStatus));
					nReturn = m_MCU_FUNC.SANKYODIP_CARD_Sensor(&m_byAntiSkimmingStatus[0], &m_byAntiSkimmingStatus[1], &m_byAntiSkimmingStatus[2]);
					NHDEBUG(1, (_T("CDiagCard::ThreadHandlerProc() CMD_SANKYO_CHECK_ANTISKIMMINGSTATUS RESULT: [%d] !!!!!!\n"), nReturn));
					NHDEBUG(1, (_T("CDiagCard::ThreadHandlerProc() CMD_SANKYO_CHECK_ANTISKIMMINGSETUP DATA: Status[%02X] !!!!!!\n"), m_byAntiSkimmingStatus[2]));

					if (nReturn == MCU_SUCCESS)
					{
						nReturn = MCU_SUCCESS;
						m_bError = FALSE;
					}
					else
					{
						nReturn = MCU_FAIL;
						m_bError = TRUE;

						TCHAR ErrorCode[16] = { 0, };

						m_MCU_FUNC.SANKYODIP_CARD_GetLastError(ErrorCode);
						m_strErroCode = ErrorCode;
					}
				}
				break;

			case CMD_SANKYO_POWERON:
				{
					m_strErroCode.Empty();
					m_bError = FALSE;

					// Data Initialize
					memset(m_byATRData, 0, sizeof(m_byATRData));
					m_wATRLength = 0;

					nReturn = m_MCU_FUNC.SANKYODIP_CARD_IC_PowerOn(m_byATRData, &m_wATRLength);

					NHDEBUG(1, (_T("CDiagCard::ThreadHandlerProc() CMD_MAG_POWERON RESULT: [%d] LEN[%d] !!!!!!\n"), nReturn, m_wATRLength));

					if (nReturn == MCU_SUCCESS && m_wATRLength)
					{
						// Power On Success
						nReturn = MCU_SUCCESS;
					}
					else
					{
						nReturn	= MCU_FAIL;
					}
				}
				break;

			case CMD_SANKYO_POWEROFF:
				{
					nReturn = m_MCU_FUNC.SANKYODIP_CARD_IC_PowerOff();

					if (nReturn == MCU_SUCCESS)
						nReturn = MCU_SUCCESS;
					else
						nReturn = MCU_FAIL;
				}
				break;

			case CMD_SANKYO_READCARD:
				{
					nReturn = m_MCU_FUNC.SANKYODIP_CARD_MS_Read(&m_TrackData, m_dwTimeOut);

					NHDEBUG(1, (_T("!!!!!!!!!!!!!!!!!!!!!!!!! CMD_SANKYO_READCARD: nReturn [%d]\n"), nReturn));

					if(nReturn == MCU_SUCCESS)
					{
						// Magtek과 맞추기 위해 상위국으로 Read Success시에는 1로 송부한다. (실제 VDM DLL은 0으로 RETURN함
						nReturn = 1;
						NHDEBUG(1, (_T("CDiagCard::ThreadHandlerProc() CMD_SANKYO_READCARD : MCU_SUCCESS\n")));
					}
					else
					{
						// Timeout과 Fail의 Error Code가 존재해야함. (문의 필요)
						nReturn = MCU_FAIL;
						NHDEBUG(1, (_T("CDiagCard::ThreadHandlerProc() CMD_SANKYO_READCARD : TIME OUT !!!!!!\n")));
					}
				}
				break;
			// end of [#2078]
			}

			// Response
			if (CmdInfo.m_nRequestCmd != CMD_NONE)
			{
				CmdInfo.m_nResponseCmd = (int)nReturn;
				m_ResponseQueue.Enqueue(CmdInfo);
			}
		}
	}

	NHDEBUG(1, (_T("CDiagCard::ThreadHandlerProc() Command execute Process End()\n")));

	return 0;
}