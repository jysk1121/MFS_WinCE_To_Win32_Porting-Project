#include "stdafx.h"
#include "LineModem.h"
//#define NH_DEBUG
#include ".\Common\NHDbgApi.h"
#include "Queue.h"

//------------------------------------------------------------------
//	Define extern function
//------------------------------------------------------------------
extern int		put_data(char *recv_data, int recv_size);
extern int		put_event(char recv_evt);

//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------
#define DBG_ERR		1
#define DBG_CALL	1
#define DBG_INFO	1

//------------------------------------------------------------------
//	Define Return Value
//------------------------------------------------------------------
#define RET_OK	0
#define RET_NG	1

#define TAPI_VERSION_1_0		0x00010003
#define TAPI_VERSION_1_4		0x00010004
#define TAPI_VERSION_2_0		0x00020000
#define TAPI_VERSION_3_0		0x00030000
#define TAPI_USE_VERSION		TAPI_VERSION_3_0

#define	RING_EVENTCHECK_INTERVAL		2

#define _INTIALSTRING_REG	_T("Drivers\\Unimodem\\Init")

CLineModem::CLineModem() : m_eStartCheckSerial(FALSE, FALSE), m_eStartCheckLine(FALSE, FALSE)
{
	NHDEBUG(DBG_CALL, (L"CALL : [%s]\n", TEXT(__FUNCTION__)));

	CLineCtrl::Register(LINE_DIALUP, this);

	m_hLineApp = NULL;
	m_nLineID = -1;
	m_hCall = NULL;
	m_hComm = NULL;
	m_nMakeCallRequestID = 0;
	m_bCheckSerial = FALSE;
	m_bCheckLine = FALSE;
	m_hLineEvent = NULL;

	m_dwEvtMask = 0;
	memset(&m_CommTimeouts, 0, sizeof(m_CommTimeouts));
	memset(&m_dcb, 0, sizeof(m_dcb));

	memset(&m_CurLineInfo, 0, sizeof(m_CurLineInfo));
	memset(&m_TmpLineInfo, 0, sizeof(m_TmpLineInfo));

	// Create Thread.
	m_hLineEventThread = CreateThread(NULL,					/* Must be NULL */ 
							 0,								/* Stack size */ 
							 LineEventThreadProc,			/* Thread Start Routine */ 
							 (void*)this,					/* Thread Parameter */ 
							 0,								/* Creation Flag */ 
							 &m_nLineEventThread);			/* Thread ID */ 

	Resume();

}

CLineModem::~CLineModem()
{
	NHDEBUG(DBG_CALL, (L"CALL : [%s]\n", TEXT(__FUNCTION__)));

	m_bExitThread = TRUE;

	Sleep(50);	

	// 스레드를 종료한다.
	if (m_hLineEventThread != NULL)
	{
		::TerminateThread(m_hLineEventThread, 0);
		CloseHandle(m_hLineEventThread);
	}
}

int CLineModem::GetLineType()
{
	return LINE_DIALUP;
}


int	CLineModem::LineOpen(LPCTSTR Dest_Info,		/*ip:[xxx.xxx.xxx.xxx]*/
						 LPCTSTR dest_port,		/*port:[xxxxx]*/ 
						 LPCTSTR option,		/*option: "1" - rms init string, "0" - host init string*/	 
						 LPCTSTR mode)			/*socket mode: "1" - ready for Rms, "0" - host connect mode*/
{
	NHDEBUG(DBG_CALL, (L"CALL : [%s]\n", TEXT(__FUNCTION__)));

	// [#2016] NH KSK 2011.02.09
//	long	dwReturn;
//	BOOL	bRMSInitString = ((option[0] == '1') ? TRUE : FALSE);
//	BOOL	bDeviceTest = ((option[0] == '2') ? TRUE : FALSE);
//	BOOL	bOutgoingCall = ((mode[0] == '0') ? TRUE : FALSE);

	NVDump('O', 'A', "11", L"", L"MD_LINEOPEN"); // [#2024] NH KSK 2011.02.24

	long	dwReturn = 0;
	BOOL	bRMSInitString = FALSE;
	BOOL	bDeviceTest = FALSE;
	BOOL	bOutgoingCall = ((mode[0] == '0') ? TRUE : FALSE);

	if (option == L"1" || option == Int2Asc(MODEM_RMS_DEV_TEST) || option == Int2Asc(MODEM_RMS_LINE_TEST))
		bRMSInitString = TRUE;

	if (option == Int2Asc(MODEM_HOST_DEV_TEST) || option == Int2Asc(MODEM_RMS_DEV_TEST))
		bDeviceTest = TRUE;
	// end of [#2016]

	NHDEBUG(DBG_INFO, (L"[LineModem] Set Modem Initial String\n"));

	// SetInitialString
	SetModemInitString(bRMSInitString);

	NHDEBUG(DBG_INFO, (L"[LineModem] Initialize TAPI\n"));

	m_bConnectStatus = FALSE;	// [#2075] NH KSK 2011.06.27

	// Initialize TAPI
	if (InitializeTAPI() == FALSE)
	{
		ClosePhoneLine();
		NHDEBUG(DBG_ERR, (L"[LineModem] InitializeTAPI() failed. RMSInitString(%d), OutgoingCall(%d), DeviceTest(%d)\n", 
							bRMSInitString, bOutgoingCall, bDeviceTest));

		NVDump('F', 'A', "11", L"", L"MD_LINEOPEN_NG1"); // [#2024] NH KSK 2011.02.24

		return 12;		// Initialize fail
	}

	NHDEBUG(DBG_INFO, (L"[LineModem] Open Phone Line\n"));

	// Open the current line.
	if ((dwReturn = OpenPhoneLine()) != 0)
	{
		ClosePhoneLine();
		NHDEBUG(DBG_ERR, (L"[LineModem] lineOpen() failed. err(0x%08X) RMSInitString(%d), OutgoingCall(%d), DeviceTest(%d)\n", 
							dwReturn, bRMSInitString, bOutgoingCall, bDeviceTest));

		NVDump('F', 'A', "11", L"", L"MD_LINEOPEN_NG2"); // [#2024] NH KSK 2011.02.24

		return 13;	// lineOpen fail
	}

	m_bRMSMode = bRMSInitString;
	m_bOutgoingCall = bOutgoingCall;

	// Make Call
	if (bOutgoingCall == TRUE && bDeviceTest == FALSE)
	{
		// We want to be notified for everything
		if ((dwReturn = lineSetStatusMessages(m_CurLineInfo.hLine, 0x1FFFFFF, 0)) != 0)
		{
			ClosePhoneLine();
			NHDEBUG(DBG_ERR, (L"[LineModem] Failed in lineSetStatusMessage(). err(0x%08X)\n", dwReturn));

			NVDump('F', 'A', "11", L"", L"MD_LINEOPEN_NG3"); // [#2024] NH KSK 2011.02.24

			return 13;	// lineOpen fail
		}

		NHDEBUG(DBG_INFO, (L"[LineModem] Make Phone Call\n"));

		if ((dwReturn = MakePhoneCall(Dest_Info)) <= 0)
		{
			ClosePhoneLine();

			NHDEBUG(DBG_ERR, (L"[LineModem] MakePhoneCall() failed. err(0x%08X) RMSInitString(%d), OutgoingCall(%d), DeviceTest(%d)\n", 
							  dwReturn, bRMSInitString, bOutgoingCall, bDeviceTest));

			NVDump('F', 'A', "11", L"", L"MD_LINEOPEN_NG4"); // [#2024] NH KSK 2011.02.24

			/* failed in MakePhoneCall() */
			switch (dwReturn)
			{
			case LINEERR_ADDRESSBLOCKED:		return 15;		// D0301
			case LINEERR_INVALTERMINALID:		return 16;		// D0302
			case LINEERR_CALLUNAVAIL:			return 17;		// D0303

			case LINEERR_DIALBILLING:
			case LINEERR_DIALDIALTONE:
			case LINEERR_DIALPROMPT:
			case LINEERR_DIALQUIET:				return 18;		// D0304

			case LINEERR_INUSE:
			case LINEERR_INVALADDRESS:
			case LINEERR_INVALADDRESSID:
			case LINEERR_INVALADDRESSMODE:
			case LINEERR_INVALBEARERMODE:
			case LINEERR_INVALCALLPARAMS:
			case LINEERR_INVALCOUNTRYCODE:		return 19;		// D0305
			
			case LINEERR_OPERATIONFAILED:		return 20;		// D0306
			case LINEERR_RESOURCEUNAVAIL:		return 21;		// D0307

			default:							return 14;		// D0300
			}
		}

		// Start Thread to Check Line Event
		m_bCheckLine = TRUE;
		m_eStartCheckLine.Set();
	}
	else if (bOutgoingCall == FALSE && bDeviceTest == FALSE)
	{
		// We want to be notified for everything
		if ((dwReturn = lineSetStatusMessages(m_CurLineInfo.hLine, 0x1FFFFFF, 0)) != 0)
		{
			ClosePhoneLine();
			NHDEBUG(DBG_ERR, (L"[LineModem] Failed in lineSetStatusMessage(). err(0x%08X)\n", dwReturn));

			NVDump('F', 'A', "11", L"", L"MD_LINEOPEN_NG5"); // [#2024] NH KSK 2011.02.24

			return 13;	// lineOpen fail
		}

		// Start Thread to Check Line Event
		m_bCheckLine = TRUE;
		m_eStartCheckLine.Set();
	}

	NVDump('O', 'A', "11", L"", L"MD_LINEOPEN_OK"); // [#2024] NH KSK 2011.02.24

	return RET_OK;	// OK.
}

int	CLineModem::LineClose()
{
	NHDEBUG(DBG_CALL, (L"CALL : [%s]\n", TEXT(__FUNCTION__)));

	m_bConnectStatus = FALSE;

	ClosePhoneLine();

	return RET_OK;	// OK.
}

int	CLineModem::LineSendData(BYTE *send_data, int len)
{
	//NHDEBUG(DBG_CALL, (L"CALL : [%s] - len(%d)\n", TEXT(__FUNCTION__), len));

	DWORD	dwErrorFlags;
	COMSTAT	comstate;
	DWORD	dwEvent;
	DWORD	dwNumberOfBytesWritten;

	if (m_hComm == NULL)
		return RET_NG;

	::PurgeComm(m_hComm, PURGE_TXABORT | PURGE_TXCLEAR);

   	ClearCommError(m_hComm, &dwErrorFlags , &comstate);

	if (m_bRMSMode == TRUE)
	{
		if (WriteFile(m_hComm, send_data, len, &dwNumberOfBytesWritten, NULL) == FALSE)
		{
			NHDEBUG(DBG_ERR, (L"[LineModem] Failed in WriteFile(). err(0x%08X)\n", GetLastError()));
			ClosePhoneLine();

			NVDump('F', 'A', "11", L"", L"MD_SND_NG1"); // [#2024] NH KSK 2011.02.24

			return RET_NG;
		}	
	}
	else
	{
#if (_WIN32_WCE < 0x600)
		for (int i = 0; i <len; i++)
		{
			if (WriteFile(m_hComm, &send_data[i], 1, &dwNumberOfBytesWritten, NULL) == FALSE)
			{
				NHDEBUG(DBG_ERR, (L"[LineModem] Failed in WriteFile(). err(0x%08X)\n", GetLastError()));
				ClosePhoneLine();
				return RET_NG;
			}	
			Sleep(3);	// [#554] KSK 2009.08.10 대용량 Data Send시 Abort가 나는 Bug Fix (거래 속도 차이 확인 필요)
		}
#else
		if (WriteFile(m_hComm, send_data, len, &dwNumberOfBytesWritten, NULL) == FALSE)
		{
			NHDEBUG(DBG_ERR, (L"[LineModem] Failed in WriteFile(). err(0x%08X)\n", GetLastError()));
			ClosePhoneLine();

			NVDump('F', 'A', "11", L"", L"MD_SND_NG2"); // [#2024] NH KSK 2011.02.24

			return RET_NG;
		}

#endif
	}

	WaitCommEvent(m_hComm, &dwEvent, NULL);
	ClearCommError(m_hComm, &dwErrorFlags , &comstate);

	NHDEBUG(DBG_INFO, (L"[LineModem] Send Data Successful. len(%d)-(0x%0X)\n", len, send_data[0]));
	return RET_OK;	// OK.
}

int CLineModem::LineAccept(BYTE opt)				/*accept option: '0x01' - call accept, '0x00' - call deny*/
{
	NHDEBUG(DBG_CALL, (L"CALL : [%s]\n", TEXT(__FUNCTION__)));

	if (opt && m_bNewCall)
	{
		if (Answer() == 0)
			return RET_OK;
	}

	return RET_NG;
}

int CLineModem::LineCallState(int nMode)
{
	// [#2075] NH KSK 2011.06.27
//	if (m_bNewCall == TRUE)
//		return RET_OK;

	if (nMode == 0)
	{
		if (m_bNewCall == TRUE)
			return RET_OK;
		else
			return RET_NG;
	}

	return m_bConnectStatus;

//	return RET_NG;
	// end of [#2075]
}

BOOL CLineModem::SetupComPort(HANDLE hComm, BOOL bRMSMode)
{
	NHDEBUG(DBG_CALL, (L"CALL : [%s]\n", TEXT(__FUNCTION__)));
	
	DWORD			dwErrorFlags;
	COMSTAT			comstate;

	// only clear error state.
	ClearCommError(hComm, &dwErrorFlags , &comstate);

	if (::GetCommMask(hComm, &m_dwEvtMask) == FALSE)
	{
		NHDEBUG(DBG_ERR, (L"[LineModem] Failed in GetCommMask(). err(0x%08X)\n", GetLastError()));
		return FALSE;
	}

	if (::GetCommState(hComm, &m_dcb) == FALSE)
	{
		NHDEBUG(DBG_ERR, (L"[LineModem] Failed in GetCommState(). err(0x%08X)\n", GetLastError()));
		return FALSE;
	}

	if (::GetCommTimeouts(hComm, &m_CommTimeouts) == FALSE)
	{
		NHDEBUG(DBG_ERR, (L"[LineModem] Failed in GetCommTimeouts(). err(0x%08X)\n", GetLastError()));
		return FALSE;
	}

	// Set the comm mask so we receive error signals.
	if (::SetCommMask(hComm, EV_RXCHAR | EV_ERR |EV_BREAK |EV_TXEMPTY) == FALSE)
	{
		NHDEBUG(DBG_ERR, (L"[LineModem] Failed in SetCommMask(). err(0x%08X)\n", GetLastError()));
		return FALSE;
	}

#if defined(UNDER_CE) && (_WIN32_WCE < 0x600)
	// Set the size of InQueue, OutQueue	
	if (::SetupComm(hComm, QSize, QSize) == FALSE)
	{
		NHDEBUG(DBG_ERR, (L"[LineModem] Failed in SetupComm(). err(0x%08X)\n", GetLastError()));
		return FALSE;
	}
#endif

	m_dcb.DCBlength	= sizeof(DCB);
	m_dcb.fBinary		= TRUE;
	m_dcb.fParity		= TRUE;
	m_dcb.fErrorChar	= FALSE;
	m_dcb.fNull		= FALSE;
	m_dcb.fAbortOnError = FALSE;

	if (bRMSMode == FALSE)
	{
		// ATM position
		m_dcb.ByteSize	= 7;
		m_dcb.Parity	= EVENPARITY;
		m_dcb.StopBits	= ONESTOPBIT;
	}
	else
	{
		// RMS position
		m_dcb.ByteSize	= 8;
		m_dcb.Parity	= NOPARITY;
		m_dcb.StopBits	= ONESTOPBIT;
	}
	
	if (::SetCommState(hComm, &m_dcb) == FALSE)
	{
		NHDEBUG(DBG_ERR, (L"[LineModem] Failed in SetCommState(). err(0x%08X)\n", GetLastError()));
		return FALSE;
	}

	//SPR 대책 내용과 동일 적용
	m_CommTimeouts.ReadIntervalTimeout = 100;
	m_CommTimeouts.ReadTotalTimeoutMultiplier = 3;
	m_CommTimeouts.ReadTotalTimeoutConstant = 30;
	
	m_CommTimeouts.WriteTotalTimeoutMultiplier = 50;
	m_CommTimeouts.WriteTotalTimeoutConstant = 150;

	if (::SetCommTimeouts(hComm, &m_CommTimeouts) == FALSE)
	{
		NHDEBUG(DBG_ERR, (L"[LineModem] Failed in SetCommTimeouts(). err(0x%08X)\n", GetLastError()));
		return FALSE;
	}

	if (::PurgeComm(hComm, PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_RXCLEAR) == FALSE)
	{
		NHDEBUG(DBG_ERR, (L"[LineModem] Failed in PurgeComm(). err(0x%08X)\n", GetLastError()));
		return FALSE;
	}

	return TRUE;
}

VOID CLineModem::SetModemInitString(BOOL bRmsInitString)
{
	NHDEBUG(DBG_CALL, (L"CALL : [%s]\n", TEXT(__FUNCTION__)));

	// HOST를 위한 Initial string을 설정한다.
	if (bRmsInitString == FALSE)
	{
		CString strVal = MemGetStr(_MEM_FLD_INI_NETWORKINFO,_MEM_VAR_NETINFO_MODEMINITSTR);	// [#175] [NH] KSK 2008.04.29
		CString strSet = RegGetStr(_INTIALSTRING_REG, L"1");

		strVal.TrimLeft();
		strVal.TrimRight();

		NHDEBUG(DBG_INFO, (L"[LineModem] Initial String [%s]\n", strVal));

		if(strSet.GetLength() > 4 &&  strSet.Right(4) == "<cr>")
			strSet = strSet.Left(strSet.GetLength() - 4);
		
		//Modem Initial String
		//현재 Registry에 있는 값과 Update하려는 값이 다른 경우에만 Reg update한다.
		if(strSet != strVal)
		{
			strVal += "<cr>";
			RegSetStr(_INTIALSTRING_REG, L"1", strVal);
		}

		//S12 register 설정.
		strSet = RegGetStr(_INTIALSTRING_REG, L"2");
		if(strSet.GetLength() > 4 &&  strSet.Right(4) == "<cr>")
			strSet = strSet.Left(strSet.GetLength() - 4);
		
		if(strSet != "ATS12=1")
		{
			strVal = "ATS12=1<cr>";
			RegSetStr(_INTIALSTRING_REG, L"2", strVal);
		}

		//RegSetStr(_INTIALSTRING_REG, L"2", L"AT&G4%C0\\N0<cr>");
		//RegSetStr(_INTIALSTRING_REG, L"2", L"AT&G4%C1\\N4<cr>");
		//RegSetStr(_INTIALSTRING_REG, L"2", L"AT+MS=V22<cr>");
		
		//RegSetStr(_INTIALSTRING_REG, L"3", L"AT\\N0<cr>");
		//RegSetStr(_INTIALSTRING_REG, L"4", L"AT:U87,050A;+MS=V22;:U7A,3<cr>");
		//RegSetStr(_INTIALSTRING_REG, L"5", L"AT&H7<cr>");
		//RegSetStr(_INTIALSTRING_REG, L"3", L"AT+MS=V22B<cr>");
		

		//RegSetStr(_INTIALSTRING_REG, L"2", L"AT&D2X4\\V2%C0%V1+IFC=0,2<cr>");
		//RegSetStr(_INTIALSTRING_REG, L"3", L"AT\\N0+FCLASS=1;:UAA,8004<cr>");
		//RegSetStr(_INTIALSTRING_REG, L"4", L"AT+ES=6,,8;+ESA=0,0,0,,1<cr>");
		//RegSetStr(_INTIALSTRING_REG, L"5", L"AT:U87,050A;:U7A,1<cr>");
	}
	// RMS를 위한 Initial string을 설정한다.
	else
	{
		CString strRMSInit = MemGetStr(_MEM_FLD_OPTIONALSETTING_2,_MEM_VAR_OPT2_RMSMODEMINITIALSTRING);		// [#468] NH AIREAT 2008.12.19 NZ 장애대응
		CString strSet = RegGetStr(_INTIALSTRING_REG, L"1");

		strRMSInit.TrimLeft();
		strRMSInit.TrimRight();

		NHDEBUG(DBG_INFO, (L"[LineModem] Initial String [%s]\n", strRMSInit));

		if(strSet.GetLength() > 4 &&  strSet.Right(4) == "<cr>")
			strSet = strSet.Left(strSet.GetLength() - 4);

		//Registry update한다.
		if(strSet != strRMSInit)
		{
			strRMSInit += "<cr>";
			RegSetStr(_INTIALSTRING_REG, L"1", strRMSInit);
		}
	}
}

BOOL CLineModem::InitializeTAPI()
{
	NHDEBUG(DBG_CALL, (L"CALL : [%s]\n", TEXT(__FUNCTION__)));

	/*
#if (_WIN32_WCE >= 0x600)
	HANDLE	hModemPort = NULL;
	hModemPort = CreateFile(_T("COM5:"), GENERIC_READ|GENERIC_WRITE,NULL,NULL,OPEN_EXISTING,NULL,NULL);

	if (hModemPort != NULL)
	{
		DWORD	dwCode;
		BYTE	byOutBuffer[512];
		DWORD	dwBytes;
		DWORD	dwModemStatus = 0;

		dwCode = 0x1b0034;//IOCTL_SERIAL_GET_MODEMSTATUS
		DeviceIoControl(hModemPort, dwCode, NULL, 0, (LPVOID)byOutBuffer, sizeof(byOutBuffer), &dwBytes, NULL);
		DeviceIoControl(hModemPort, dwCode, NULL, 0, (LPVOID)byOutBuffer, sizeof(byOutBuffer), &dwBytes, NULL);

		// EV_CTS, EV_DSR 설정으로 현재 모뎀 상태 조회 코드 추가.
		GetCommModemStatus(hModemPort, &dwModemStatus);
		GetCommModemStatus(hModemPort, &dwModemStatus);

		CloseHandle(hModemPort);
	}
	else
	{
		NHDEBUG(DBG_CALL, (L"++++ MODEM COM5 OPEN FAILED(%08X)\n", GetLastError()));
	}
#endif
	*/


	DWORD			dwNumDevs	= 0;
	LONG			lReturn		= 0;
	DWORD			dwAPIVersion	= 0;
	BOOL			bFoundLineDev = FALSE;

	LINEINITIALIZEEXPARAMS	LineInitializeExParams;

	LineInitializeExParams.dwTotalSize = sizeof(LINEINITIALIZEEXPARAMS);
	LineInitializeExParams.dwNeededSize = sizeof(LINEINITIALIZEEXPARAMS);
	LineInitializeExParams.dwUsedSize = sizeof(LINEINITIALIZEEXPARAMS);
	LineInitializeExParams.dwOptions = LINEINITIALIZEEXOPTION_USEEVENT;

	m_TmpLineInfo.dwAPIVersion = TAPI_VERSION_3_0;
	// Initialize line
	lReturn = ::lineInitializeEx(&m_hLineApp,				// the application's handle for TAPI
							 NULL,							// instance handle of client application
							 NULL,							// the address of callback function
							 NULL,							// application-supplied name
							 &dwNumDevs,				
							 &m_TmpLineInfo.dwAPIVersion,
							 &LineInitializeExParams);

	// If function "lineInitialize" fails, then return.
	if (lReturn != 0)
	{
		NHDEBUG(DBG_ERR, (L"[LineModem] LineInitialize failed. err(0x%08X)\n", lReturn));

		NVDump('F', 'A', "11", L"", L"MD_TAPI_NG1"); // [#2024] NH KSK 2011.02.24

		return FALSE;
	}

	// If there is no device, then return.
    if (dwNumDevs == 0)
    {
		NHDEBUG(DBG_ERR, (L"[LineModem] There are no line devices available.\n"));

		NVDump('F', 'A', "11", L"", L"MD_TAPI_NG2"); // [#2024] NH KSK 2011.02.24

		return FALSE;
	}

	// Get Line Event Handle
	m_hLineEvent = LineInitializeExParams.Handles.hEvent;

	NHDEBUG(DBG_INFO, (L"[LineModem] m_hLineApp(%d), dwNumDevs(%d)\n", m_hLineApp, dwNumDevs));

	bFoundLineDev = FALSE;
	memset(&m_CurLineInfo, 0, sizeof(m_CurLineInfo));

    // Search line dev. to want to use
    for (DWORD dwLineID = 0; dwLineID < dwNumDevs; ++dwLineID)
    {
		memset(&m_TmpLineInfo, 0, sizeof(m_TmpLineInfo));
		if (IsNeedLineDev(dwLineID, &m_TmpLineInfo) == TRUE)
		{
			NHDEBUG(DBG_INFO, (L"[LineModem] Found Line Dev. to want to use. ID[%d]\n", dwLineID));
			m_CurLineInfo = m_TmpLineInfo;
			m_nLineID = dwLineID;
			bFoundLineDev = TRUE;
		}
    }

	if (bFoundLineDev == TRUE)
		return TRUE;

	NVDump('F', 'A', "11", L"", L"MD_TAPI_NG3"); // [#2024] NH KSK 2011.02.24

	return FALSE;
}

BOOL CLineModem::IsNeedLineDev(DWORD dwLineID, LPLINEINFO lpLineInfo)
{
	NHDEBUG(DBG_CALL, (L"CALL : [%s]\n", TEXT(__FUNCTION__)));

	DWORD	dwRet;
    LPTSTR lpszLineName = NULL; 
    LPLINEDEVCAPS lpLineDevCaps = NULL;
    
    // Negotiate the API version number. If it fails, return to dwReturn.
    if (lineNegotiateAPIVersion (m_hLineApp,                   // TAPI registration handle
		                         dwLineID,                     // Line device to be queried
								 TAPI_VERSION_1_0,             // Least recent API version 
								 TAPI_USE_VERSION,			   // Most recent API version 
								 &(lpLineInfo->dwAPIVersion),  // Negotiated API version 
								 NULL) != 0)                        // Must be NULL; the provider-
    {
		NHDEBUG(DBG_INFO, (L"[LineModem] lineNegotiateAPIVersion failed. Line ID[%d]\n", dwLineID));
		return FALSE;
    }
    
	NHDEBUG(DBG_INFO, (L"[LineModem] TAPI API Version(%d)\n", lpLineInfo->dwAPIVersion));

	memset(&m_LineDevCaps, 0, sizeof(m_LineDevCaps));
	m_LineDevCaps.dwTotalSize = sizeof(LINEDEVCAPS);

	if ((dwRet = lineGetDevCaps (m_hLineApp,
		                dwLineID,
						lpLineInfo->dwAPIVersion,
						0,
						&m_LineDevCaps)) != 0)
	{
		NHDEBUG(DBG_INFO, (L"[LineModem] lineGetDevCaps failed. Line ID[%d], err[0x%08X]\n", dwLineID, dwRet));
		return FALSE;
	}


	if ((m_LineDevCaps.dwBearerModes & LINEBEARERMODE_VOICE) &&
		(m_LineDevCaps.dwMediaModes & LINEMEDIAMODE_DATAMODEM) &&
		(m_LineDevCaps.dwLineFeatures & LINEFEATURE_MAKECALL))
	{
		NHDEBUG(DBG_INFO, (L"[LineModem] we found the line device. Line ID[%d]\n", dwLineID));
		return TRUE;
	}

	NHDEBUG(DBG_INFO, (L"[LineModem] It's not device to want to use. Line ID[%d]\n", dwLineID));
	return FALSE;
}

DWORD CLineModem::OpenPhoneLine()
{
	NHDEBUG(DBG_CALL, (L"CALL : [%s], m_hLineApp(%d), m_nLineID(%d), APIVer(%d)\n", 
						TEXT(__FUNCTION__), m_hLineApp, m_nLineID, m_CurLineInfo.dwAPIVersion));

	DWORD dwReturn;

	// Open the current line.
	dwReturn = lineOpen(m_hLineApp,						// a handle to the application's registration with TAPI
						m_nLineID,						// identity of the line device
						&m_CurLineInfo.hLine,			// HLINE handle
						m_CurLineInfo.dwAPIVersion,		// the API version number
						0,								// the extension version number
						0,								// user-instance data
						LINECALLPRIVILEGE_OWNER,
						LINEMEDIAMODE_DATAMODEM,		// the media type
						NULL);							// Must set to NULL for Windows CE

	return dwReturn;
}

VOID CLineModem::ClosePhoneLine()
{
	CNHAutoLock	Sync(m_eMethodUse);

	NHDEBUG(DBG_CALL, (L"CALL : [%s]\n", TEXT(__FUNCTION__)));

	m_bCheckSerial = FALSE;
	m_bCheckLine = FALSE;

	/*
	// 50과 같은 상태로 Close
	Sleep(20);

	if (m_hComm != NULL)
	{
		CloseHandle(m_hComm);
		m_hComm = NULL;
	}

	Sleep(20);
	*/

//	/*
	// 50과 같은 상태로 Close
	// If lineMakeCall succeeded, then drop the call.
    if (m_hCall != NULL)
    {
		LINECALLSTATUS stLineStatus;
		LONG lRet;

		memset(&stLineStatus, 0, sizeof(LINECALLSTATUS));

		lRet = lineGetCallStatus(m_hCall, &stLineStatus);
		if (!lRet)
			::lineDrop (m_hCall, NULL, 0);
		m_hCall = NULL;
    }

	Sleep(20);
//	*/

	// close current line
	if (m_CurLineInfo.hLine != NULL)
	{
		::lineClose(m_CurLineInfo.hLine);
		m_CurLineInfo.hLine = NULL;
	}

	Sleep(20);

	if (m_hComm != NULL)
	{
		::PurgeComm(m_hComm,
					PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_RXCLEAR);
		::SetCommMask(m_hComm, m_dwEvtMask);
		::SetCommState(m_hComm, &m_dcb);
		::SetCommTimeouts(m_hComm, &m_CommTimeouts);
		::CloseHandle(m_hComm);
		m_hComm = NULL;
	}

	Sleep(20);

	// close handle for TAPI
	if (m_hLineApp != NULL)
	{
		::lineShutdown(m_hLineApp);
		m_hLineApp = NULL;
	}

	m_hLineEvent = NULL;
	m_bNewCall = FALSE;


	/*
	// Reset Modem.
#if (_WIN32_WCE >= 0x600)
	{
#define IOCTL_SYSTEM_RESET	1
#define IOCTL_MODEM_RESET	2

		HANDLE hReset = CreateFile(L"RST1:", GENERIC_READ | GENERIC_WRITE, NULL, NULL, OPEN_EXISTING, NULL, NULL);
		if (hReset != NULL)
		{
			DWORD dwBuffer = IOCTL_MODEM_RESET;
			DWORD dwWritten;
			WriteFile(hReset, &dwBuffer, 1, &dwWritten, NULL);
			CloseHandle(hReset);
			Sleep(100);
		}
	}
#endif
	*/
}

long CLineModem::MakePhoneCall(LPCTSTR lpszPhoneNum)
{
	NHDEBUG(DBG_CALL, (L"CALL : [%s] - PHONE NUMBER[%s]\n", TEXT(__FUNCTION__), lpszPhoneNum));

	LONG				tr = ERROR_SUCCESS;
	LINECALLPARAMS		LineCallParams;

	// Initialize g_MakeCallRequestID.
    m_nMakeCallRequestID = 0;

	// Setup our CallParams for DATAMODEM settings.
	memset(&LineCallParams, 0, sizeof(LINECALLPARAMS));

    LineCallParams.dwTotalSize = sizeof(LINECALLPARAMS) + 1;

	LineCallParams.dwBearerMode = LINEBEARERMODE_VOICE;
	LineCallParams.dwMediaMode  = LINEMEDIAMODE_DATAMODEM;

	// [2ND] AIREAT 2009.04.18 : DOVE는 BLIND DIAL이 ENABLE 되어야 한다.
#if (_WIN32_WCE < 0x600)
	LineCallParams.dwCallParamFlags = LINECALLPARAMFLAGS_IDLE;
#endif
                                
	LineCallParams.dwAddressMode = LINEADDRESSMODE_ADDRESSID;
	LineCallParams.dwAddressID = 0;

	LineCallParams.dwOrigAddressSize = 0;
	LineCallParams.dwOrigAddressOffset = 0;

	LineCallParams.DialParams.dwDialSpeed = 0;
	LineCallParams.DialParams.dwDigitDuration = 0;
	LineCallParams.DialParams.dwDialPause = 0;
	LineCallParams.DialParams.dwWaitForDialtone = 0;

	LineCallParams.dwDisplayableAddressOffset = sizeof(LINECALLPARAMS);
	LineCallParams.dwDisplayableAddressSize = 1;


	// Make the phone call. lpCallParams should be NULL if the default 
    // call setup parameters are requested.
	m_nMakeCallRequestID = ::lineMakeCall(m_CurLineInfo.hLine,
										 &m_hCall,
										 lpszPhoneNum,
										 NULL,
										 &LineCallParams);

	if (m_nMakeCallRequestID > 0)
	{
		NHDEBUG(DBG_INFO, (L"[LineModem] LineMakeCall Success - RequestID(%d), Call(%d)\n", m_nMakeCallRequestID, m_hCall));
	}

	// If the make call did not succeed but the line was opened, 
    // then close it.
    if ((m_nMakeCallRequestID <= 0) && (m_CurLineInfo.hLine))
	{
		NHDEBUG(DBG_INFO, (L"[LineModem] m_nMakeCallRequestID(%d), m_CurLineInfo.hLine(%08X)\n", m_nMakeCallRequestID, m_CurLineInfo.hLine));
		ClosePhoneLine();
		return LINEERR_CALLUNAVAIL;
	}
	
	return m_nMakeCallRequestID;
}

HANDLE CLineModem::GetComportHandle(DWORD *lpRet)
{
	NHDEBUG(DBG_CALL, (L"CALL : [%s]\n", TEXT(__FUNCTION__)));

	char	szTemp[sizeof(VARSTRING)+255];

	VARSTRING	*pvarStrDevID = (VARSTRING *)szTemp;

	memset(pvarStrDevID, 0, sizeof(VARSTRING)+255);
	pvarStrDevID->dwTotalSize = sizeof(VARSTRING)+255;
	
	long lRet = lineGetID(m_CurLineInfo.hLine,
						  0,
						  m_hCall,
						  LINECALLSELECT_LINE,
						  pvarStrDevID,
						  L"comm/datamodem");
	if(lRet)
	{
		*lpRet = lRet;
		return NULL;
	}

	*lpRet = 0;

	return *((LPHANDLE)((char *)pvarStrDevID + pvarStrDevID->dwStringOffset));
}

DWORD CLineModem::Answer()
{
	NHDEBUG(DBG_CALL, (L"CALL : [%s]\n", TEXT(__FUNCTION__)));

	DWORD	tr = ERROR_SUCCESS;
	DWORD	nRet = 0;

	// [CS#82] NH AIREAT 2008.3.14
	// 추후 리턴값 상세분류를 위해 switch를 사용.
	tr = ::lineAnswer(m_hCall, NULL, 0);
	if (tr < 0)
	{
		//HangUp();

		switch (tr)
		{
		case LINEERR_INUSE:
		case LINEERR_OPERATIONUNAVAIL:
		case LINEERR_INVALCALLHANDLE:
		case LINEERR_OPERATIONFAILED:
		case LINEERR_INVALCALLSTATE:
		case LINEERR_RESOURCEUNAVAIL:
		case LINEERR_INVALPOINTER:
		case LINEERR_UNINITIALIZED:
		case LINEERR_NOMEM:
		case LINEERR_USERUSERINFOTOOBIG:
		case LINEERR_NOTOWNER:
			nRet = 1;
			break;
		default:
			nRet = 1;
			break;
		}
	}
	// end of [#82]

	return nRet;
}

unsigned CLineModem::ThreadHandlerProc(void)	// 스레드의 Handler.
{
	NHDEBUG(DBG_CALL, (L"CALL : [%s]\n", TEXT(__FUNCTION__)));

	DWORD			dwRead;
	BOOL			bCloseLine = FALSE;
	DWORD			dwCommEvent;

	m_bExitThread = FALSE;

	while(m_bExitThread != TRUE)
	{
		bCloseLine = FALSE;

		// wait for start signal
		if (m_eStartCheckSerial.Wait(10) != WAIT_OBJECT_0)
			continue;

		NHDEBUG(DBG_INFO, (L"[LineModem] Start THREAD to Check Serial. m_bCheckSerial(%d)\n", m_bCheckSerial));

		while(m_bCheckSerial == TRUE)
		{
			if (WaitCommEvent(m_hComm, &dwCommEvent, NULL) == TRUE)
			{
				// we receive EV_RXCHAR event.
				if (dwCommEvent & EV_RXCHAR)
				{
					NHDEBUG(DBG_ERR, (L"[LineModem] CHECKED RXCHAR\n"));
					do
					{
						if (ReadFile(m_hComm, m_RecvBuf, sizeof(m_RecvBuf), &dwRead, NULL) == TRUE)
						{
							if (dwRead > 0)
							{
								put_data(m_RecvBuf, dwRead);
								NHDEBUG(DBG_ERR, (L"[LineModem] Receive Data(%d) - (0x%0X)\n", dwRead, m_RecvBuf[0]));
							}
						}
						else
						{
							NHDEBUG(DBG_ERR, (L"[LineModem] Failed in ReadFile(). err(0x%08X)\n", GetLastError()));
							bCloseLine = TRUE;
							break;
						}

					} while(dwRead > 0);
				}
			}
			else
			{
				NHDEBUG(DBG_ERR, (L"[LineModem] Failed in WaitCommEvent(). err(0x%08X)\n", GetLastError()));
			}

			if (bCloseLine == TRUE)
				break;
		}

		NHDEBUG(DBG_INFO, (L"[LineModem] End THREAD to Check Serial. bCloseLine(%d)\n", bCloseLine));

		if (bCloseLine == TRUE)
		{
			m_bConnectStatus = FALSE;	// [#2075] NH KSK 2011.06.27
			put_event(DISCONNECT_EVT);
			ClosePhoneLine();
		}

		NHDEBUG(DBG_INFO, (L"[LineModem] go Wait For New Job\n"));

	} // end of while(m_bExitThread != TRUE)

	return 0;
}

DWORD _stdcall CLineModem::LineEventThreadProc( void *pThreadHandler )
{
	CLineModem *pcHandler = static_cast<CLineModem*> (pThreadHandler);

	return pcHandler->LineEventThread();
}

unsigned CLineModem::LineEventThread(void)
{
	DWORD			dwReturn;
	LINEMESSAGE		LineMsg;
	BOOL			bCloseLine;

	m_bExitThread = FALSE;

	while (m_bExitThread != TRUE)
	{
		// wait for start signal
		if (m_eStartCheckLine.Wait(10) != WAIT_OBJECT_0)
			continue;

		while (m_bCheckLine == TRUE)
		{
			bCloseLine = FALSE;

			// Get a TAPI event if available, wait for 10ms just enough to give up quontum.
			switch(WaitForSingleObject(m_hLineEvent, 10))
			{
			case WAIT_OBJECT_0:
				{
					//TAPI's got something
					if ((dwReturn = lineGetMessage(m_hLineApp, &LineMsg, 0)) != 0)
					{
						NHDEBUG(DBG_ERR, (L"[LineModem] failed in lineGetMessage(). err(0x%08X)\n", dwReturn));
						bCloseLine = TRUE;
						break;
					}

					// Process the retruned msg
					switch (LineMsg.dwMessageID)
					{
					case LINE_CALLSTATE:
						switch (LineMsg.dwParam1)
						{
#ifdef NH_DEBUG
						case LINECALLSTATE_ACCEPTED:
							NHDEBUG(DBG_INFO, (L"[LineModem] Receive State : [LINECALLSTATE]-[ACCEPTED]\n"));
							break;
						case LINECALLSTATE_OFFERING:
							NHDEBUG(DBG_INFO, (L"[LineModem] Receive State : [LINECALLSTATE]-[OFFERING]\n"));
							break;
#endif

						case LINECALLSTATE_IDLE:
							NHDEBUG(DBG_INFO, (L"[LineModem] Receive State : [LINECALLSTATE]-[IDLE]\n"));
							bCloseLine = TRUE;
							break;
						case LINECALLSTATE_BUSY:
							NHDEBUG(DBG_INFO, (L"[LineModem] Receive State : [LINECALLSTATE]-[BUSY]\n"));
							put_event(USER_LINEBUSY);
							bCloseLine = TRUE;
							break;
						case LINECALLSTATE_SPECIALINFO:
							NHDEBUG(DBG_INFO, (L"[LineModem] Receive State : [LINECALLSTATE]-[SPECIALINFO]\n"));
							bCloseLine = TRUE;
							break;

						case LINECALLSTATE_CONNECTED:
							{
								DWORD			lRet;

								NHDEBUG(DBG_INFO, (L"[LineModem] Receive State : [LINECALLSTATE]-[CONNECTED]\n"));

								// Get Handle
								m_hComm = GetComportHandle(&lRet);
								if (lRet != 0)
								{
									NHDEBUG(DBG_ERR, (L"[LineModem] Comport Open Failed. err(0x%08X)\n", lRet));
									bCloseLine = TRUE;
									break;
								}

								if (SetupComPort(m_hComm, m_bRMSMode) == FALSE)
								{
									NHDEBUG(DBG_ERR, (L"[LineModem] failed in SetupComPort()\n"));
									bCloseLine = TRUE;
									break;
								}

								// start Thread to check serial
								m_bCheckSerial = TRUE;
								m_eStartCheckSerial.Set();

								NHDEBUG(DBG_INFO, (L"[LineModem] PUT EVENT [CONNECT_EVT]\n"));

								m_bConnectStatus = TRUE;	// [#2075] NH KSK 2011.06.27
								put_event(CONNECT_EVT);
							}
							break;
						case LINECALLSTATE_DISCONNECTED:
							{
								NHDEBUG(DBG_INFO, (L"[LineModem] Receive State : [LINECALLSTATE]-[DISCONNECTED]\n"));
								bCloseLine = TRUE;

								m_bConnectStatus = FALSE;	// [#2075] NH KSK 2011.06.27

								switch (LineMsg.dwParam2)
								{
								case LINEDISCONNECTMODE_BUSY:			put_event(USER_LINEBUSY);	break;
								case LINEDISCONNECTMODE_NOANSWER:		put_event(USER_NOANSWER);	break;
								case LINEDISCONNECTMODE_NODIALTONE:		put_event(USER_NODIALTONE);	break;
								}
#ifdef NH_DEBUG
								switch (LineMsg.dwParam2)
								{
								case LINEDISCONNECTMODE_NORMAL:			NHDEBUG(DBG_INFO, (L"[LineModem] Remote party disconnected\n"));	break;
								case LINEDISCONNECTMODE_UNKNOWN:		NHDEBUG(DBG_INFO, (L"[LineModem] Unknown reason\n"));				break;
								case LINEDISCONNECTMODE_REJECT:			NHDEBUG(DBG_INFO, (L"[LineModem] Remote Party rejected call\n"));	break;
								case LINEDISCONNECTMODE_PICKUP:			NHDEBUG(DBG_INFO, (L"[LineModem] Local phone picked up\n"));		break;
								case LINEDISCONNECTMODE_FORWARDED:		NHDEBUG(DBG_INFO, (L"[LineModem] Forwarded\n"));					break;
								case LINEDISCONNECTMODE_BUSY:			NHDEBUG(DBG_INFO, (L"[LineModem] Busy\n"));							break;
								case LINEDISCONNECTMODE_NOANSWER:		NHDEBUG(DBG_INFO, (L"[LineModem] No Answer\n"));					break;
								case LINEDISCONNECTMODE_BADADDRESS:		NHDEBUG(DBG_INFO, (L"[LineModem] Bad address\n"));					break;
								case LINEDISCONNECTMODE_UNREACHABLE:	NHDEBUG(DBG_INFO, (L"[LineModem] Unreachable\n"));					break;
								case LINEDISCONNECTMODE_CONGESTION:		NHDEBUG(DBG_INFO, (L"[LineModem] Congestion\n"));					break;
								case LINEDISCONNECTMODE_INCOMPATIBLE:	NHDEBUG(DBG_INFO, (L"[LineModem] Incompatible\n"));					break;
								case LINEDISCONNECTMODE_UNAVAIL:		NHDEBUG(DBG_INFO, (L"[LineModem] Unavailable\n"));					break;
								case LINEDISCONNECTMODE_NODIALTONE:		NHDEBUG(DBG_INFO, (L"[LineModem] No dial tone\n"));					break;
								default:								NHDEBUG(DBG_INFO, (L"[LineModem] No dial tone\n"));					break;
								}
#endif
							}
							break;

#ifdef NH_DEBUG
							/* JUST INFORMATION */
						case LINECALLSTATE_DIALING:
							NHDEBUG(DBG_INFO, (L"[LineModem] Receive State : [LINECALLSTATE]-[DIALING]\n"));
							break;
						case LINECALLSTATE_DIALTONE:
							NHDEBUG(DBG_INFO, (L"[LineModem] Receive State : [LINECALLSTATE]-[DIALTONE]\n"));
							break;
						case LINECALLSTATE_ONHOLD:
							NHDEBUG(DBG_INFO, (L"[LineModem] Receive State : [LINECALLSTATE]-[ONHOLD]\n"));
							break;
						case LINECALLSTATE_ONHOLDPENDCONF:
							NHDEBUG(DBG_INFO, (L"[LineModem] Receive State : [LINECALLSTATE]-[ONHOLDPENDCONF]\n"));
							break;
						case LINECALLSTATE_PROCEEDING:
							NHDEBUG(DBG_INFO, (L"[LineModem] Receive State : [LINECALLSTATE]-[PROCEEDING]\n"));
							break;
						case LINECALLSTATE_RINGBACK:
							NHDEBUG(DBG_INFO, (L"[LineModem] Receive State : [LINECALLSTATE]-[RINGBACK]\n"));
							break;
						case LINECALLSTATE_UNKNOWN:
							NHDEBUG(DBG_INFO, (L"[LineModem] Receive State : [LINECALLSTATE]-[UNKNOWN]\n"));
							break;
#endif
						}
						break;	// case LINE_CALLSTATE:

					case LINE_LINEDEVSTATE:
						switch (LineMsg.dwParam1)
						{
						case LINEDEVSTATE_RINGING:
							{
								NHDEBUG(DBG_INFO, (L"[LineModem] Receive State : [LINE_LINEDEVSTATE]-[RINGING]-[%d]\n", LineMsg.dwParam3));
								if ((m_CheckRingEventTime + RING_EVENTCHECK_INTERVAL) <= CTime::GetCurrentTime() && m_bNewCall == FALSE)
								{
									m_CheckRingEventTime = CTime::GetCurrentTime();
									m_nRingCount++;
									if (MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSRINGCNT) <= m_nRingCount)
									{
										NHDEBUG(DBG_INFO, (L"[LineModem] New Call Detected\n"));
										m_nRingCount = 0;
										m_bNewCall = TRUE;
									}
								}
							}
							break;
							
						case LINEDEVSTATE_OUTOFSERVICE:
						case LINEDEVSTATE_DISCONNECTED:
						case LINEDEVSTATE_MAINTENANCE:
							NHDEBUG(DBG_INFO, (L"[LineModem] Recv Line State : [LINEDEVSTATE]-[%d]\n", LineMsg.dwParam1));
							bCloseLine = TRUE;
							break;
						}
						break;	// case LINE_LINEDEVSTATE:
					case LINE_REPLY:
						{
							NHDEBUG(DBG_INFO, (L"[LineModem] Receive State : [LINE_REPLY]-[ID(%d)]\n", LineMsg.dwParam1));
							// Reply from the lineMakeCall function.
							if (m_nMakeCallRequestID == LineMsg.dwParam1)
							{
								// If an error occurred on making the call.
								if (LineMsg.dwParam2 != 0)
								{
									NHDEBUG(DBG_ERR, (L"[LineModem] received to fail in lineMakeCall()\n"));
									bCloseLine = TRUE;
								}
							}
						}
						break;  // case LINE_REPLY:
					case LINE_APPNEWCALL:
						{
							NHDEBUG(DBG_INFO, (L"[LineModem] Receive State : [LINE_APPNEWCALL] - hCall[%d]\n", LineMsg.dwParam2));
							if (LineMsg.dwParam3 == LINECALLPRIVILEGE_OWNER && m_CurLineInfo.hLine == (HLINE)LineMsg.hDevice && m_bNewCall == FALSE)
							{
								NHDEBUG(DBG_INFO, (L"[LineModem] Receive State : [LINE_APPNEWCALL] - ACCEPT - hCall[%d]\n", LineMsg.dwParam2));
								m_CheckRingEventTime = CTime::GetCurrentTime();
								m_nRingCount = 1;
								m_hCall = (HCALL)LineMsg.dwParam2;
								if (MemGetInt(_MEM_FLD_INI_NETWORKINFO, _MEM_VAR_NETINFO_RMSRINGCNT) <= m_nRingCount)
								{
									NHDEBUG(DBG_INFO, (L"[LineModem] New Call Detected --(0x%08X)\n", LineMsg.dwParam2));
									m_nRingCount = 0;
									m_bNewCall = TRUE;
								}
							}
						}
						break;	// case LINE_APPNEWCALL:
					}
				}
				break;
			}

			if (bCloseLine == TRUE)
			{
				m_bConnectStatus = FALSE;	// [#2075] NH KSK 2011.06.27
				put_event(DISCONNECT_EVT);
				LineClose();
				break;
			}

		} // end of while (m_bCheckLine == TRUE)
		
	} // end of while (m_bExitThread != TRUE)

	return 0;
}